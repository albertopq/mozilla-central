/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

const {Cu, Cc, Ci} = require("chrome");
const events = require("sdk/event/core");
const protocol = require("devtools/server/protocol");
try {
    const { indexedDB } = require("sdk/indexed-db");
} catch (e) {
    // In xpcshell tests, we can't actually have indexedDB, which is OK:
    // we don't use it there anyway.
}
const {async} = require("devtools/async-utils");
const {Arg, Option, method, RetVal, types} = protocol;
const {LongStringActor, ShortLongString} = require("devtools/server/actors/string");

Cu.import("resource://gre/modules/Promise.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/devtools/LayoutHelpers.jsm");

XPCOMUtils.defineLazyModuleGetter(this, "Sqlite",
  "resource://gre/modules/Sqlite.jsm");

XPCOMUtils.defineLazyModuleGetter(this, "OS",
  "resource://gre/modules/osfile.jsm");

exports.register = function(handle) {
  handle.addTabActor(StorageActor, "storageActor");
};

exports.unregister = function(handle) {
  handle.removeTabActor(StorageActor);
};

// Global required for window less Indexed DB instantiation.
let global = this;

// Maximum number of cookies/local storage key-value-pairs that can be sent
// over the wire to the client in one request.
const MAX_STORE_OBJECT_COUNT = 30;
// Interval for the batch job that sends the accumilated update packets to the
// client.
const UPDATE_INTERVAL = 500; // ms

// A RegExp for characters that cannot appear in a file/directory name. This is
// used to sanitize the host name for indexed db to lookup whether the file is
// present in <profileDir>/storage/persistent/ location
let illegalFileNameCharacters = [
  "[",
  "\\x00-\\x25",     // Control characters \001 to \037
  "/:*?\\\"<>|\\\\", // Special characters
  "]"
].join("");
let ILLEGAL_CHAR_REGEX = new RegExp(illegalFileNameCharacters, "g");

// Holder for all the registered storage actors.
let storageTypePool = new Map();

/**
 * Gets an accumulated list of all storage actors registered to be used to
 * create a RetVal to define the return type of StorageActor.listStores method.
 */
function getRegisteredTypes() {
  let registeredTypes = {};
  for (let store of storageTypePool.keys()) {
    registeredTypes[store] = store;
  }
  return registeredTypes;
}

/**
 * An async method equivalent to setTimeout but using Promises
 *
 * @param {number} time
 *        The wait Ttme in milliseconds.
 */
function sleep(time) {
  let wait = Promise.defer();
  let updateTimer = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
  updateTimer.initWithCallback({
    notify: function() {
      updateTimer.cancel();
      updateTimer = null;
      wait.resolve(null);
    }
  } , time, Ci.nsITimer.TYPE_ONE_SHOT);
  return wait.promise;
}

// Cookies store object
types.addDictType("cookieobject", {
  name: "string",
  value: "longstring",
  path: "nullable:string",
  host: "string",
  isDomain: "boolean",
  isSecure: "boolean",
  isHttpOnly: "boolean",
  creationTime: "number",
  lastAccessed: "number",
  expires: "number"
});

// Array of cookie store objects
types.addDictType("cookiestoreobject", {
  total: "number",
  offset: "number",
  data: "array:nullable:cookieobject"
});

// Local Storage / Session Storage store object
types.addDictType("storageobject", {
  name: "string",
  value: "longstring"
});

// Array of Local Storage / Session Storage store objects
types.addDictType("storagestoreobject", {
  total: "number",
  offset: "number",
  data: "array:nullable:storageobject"
});

// Indexed DB store object
// This is a union on idb object, db metadata object and object store metadata
// object
types.addDictType("idbobject", {
  name: "nullable:string",
  db: "nullable:string",
  objectStore: "nullable:string",
  origin: "nullable:string",
  version: "nullable:number",
  objectStores: "nullable:number",
  keyPath: "nullable:string",
  autoIncrement: "nullable:boolean",
  indexes: "nullable:string",
  value: "nullable:longstring"
});

// Array of Indexed DB store objects
types.addDictType("idbstoreobject", {
  total: "number",
  offset: "number",
  data: "array:nullable:idbobject"
});

// Update notification object
types.addDictType("storeUpdateObject", {
  changed: "nullable:json",
  deleted: "nullable:json",
  added: "nullable:json"
});

// Helper methods to create a storage actor.
let StorageActors = {};

/**
 * Creates a default object with the common methods required by all storage
 * actors.
 *
 * This default object is missing a couple of required methods that should be
 * implemented seperately for each actor. They are namely:
 *   - observe : Method which gets triggered on the notificaiton of the watched
 *               topic.
 *   - getNamesForHost : Given a host, get list of all known store names.
 *   - getValuesForHost : Given a host (and optianally a name) get all known
 *                        store objects.
 *   - toStoreObject : Given a store object, convert it to the required format
 *                     so that it can be transferred over wire.
 *   - populateStoresForHost : Given a host, populate the map of all store
 *                             objects for it
 *
 * @param {string} typeName
 *        The typeName of the actor.
 * @param {string} observationTopic
 *        The topic which this actor listens to via Notification Observers.
 * @param {string} storeObjectType
 *        The RetVal type of the store object of this actor.
 */
StorageActors.defaults = function(typeName, observationTopic, storeObjectType) {
  return {
    typeName: typeName,

    get conn() {
      return this.storageActor.conn;
    },

    /**
     * Returns a list of currently knwon hosts for the target window. This list
     * contains unique hosts from the window + all inner windows.
     */
    get hosts() {
      let hosts = new Set();
      for (let {location} of this.storageActor.windows) {
        hosts.add(this.getHostName(location));
      }
      return hosts;
    },

    /**
     * Returns all the windows present on the page. Includes main window + inner
     * iframe windows.
     */
    get windows() {
      return this.storageActor.windows;
    },

    /**
     * Converts the window.location object into host.
     */
    getHostName: function(location) {
      return location.hostname || location.href;
    },

    initialize: function(storageActor) {
      protocol.Actor.prototype.initialize.call(this, null);

      this.storageActor = storageActor;

      this.populateStoresForHosts();
      if (observationTopic) {
        Services.obs.addObserver(this, observationTopic, false);
      }
      this.onWindowReady = this.onWindowReady.bind(this);
      this.onWindowDestroyed = this.onWindowDestroyed.bind(this);
      events.on(this.storageActor, "window-ready", this.onWindowReady);
      events.on(this.storageActor, "window-destroyed", this.onWindowDestroyed);
    },

    destroy: function() {
      this.hostVsStores = null;
      if (observationTopic) {
        Services.obs.removeObserver(this, observationTopic, false);
      }
      events.off(this.storageActor, "window-ready", this.onWindowReady);
      events.off(this.storageActor, "window-destroyed", this.onWindowDestroyed);
    },

    getNamesForHost: function(host) {
      return [...this.hostVsStores.get(host).keys()];
    },

    getValuesForHost: function(host, name) {
      if (name) {
        return [this.hostVsStores.get(host).get(name)];
      }
      return [...this.hostVsStores.get(host).values()];
    },

    getObjectsSize: function(host, names) {
      return names.length;
    },

    /**
     * When a new window is added to the page. This generally means that a new
     * iframe is created, or the current window is completely reloaded.
     *
     * @param {window} window
     *        The window which was added.
     */
    onWindowReady: async(function*(window) {
      let host = this.getHostName(window.location);
      if (!this.hostVsStores.has(host)) {
        yield this.populateStoresForHost(host, window);
        let data = {};
        data[host] = this.getNamesForHost(host);
        this.storageActor.update("added", typeName, data);
      }
    }),

    /**
     * When a window is removed from the page. This generally means that an
     * iframe was removed, or the current window reload is triggered.
     *
     * @param {window} window
     *        The window which was removed.
     */
    onWindowDestroyed: function(window) {
      let host = this.getHostName(window.location);
      if (!this.hosts.has(host)) {
        this.hostVsStores.delete(host);
        let data = {};
        data[host] = [];
        this.storageActor.update("deleted", typeName, data);
      }
    },

    form: function(form, detail) {
      if (detail === "actorid") {
        return this.actorID;
      }

      let hosts = {};
      for (let host of this.hosts) {
        hosts[host] = [];
      }

      return {
        actor: this.actorID,
        hosts: hosts
      };
    },

    /**
     * Populates a map of known hosts vs a map of stores vs value.
     */
    populateStoresForHosts: function() {
      this.hostVsStores = new Map();
      for (let host of this.hosts) {
        this.populateStoresForHost(host);
      }
    },

    /**
     * Returns a list of requested store objects. Maximum values returned are
     * MAX_STORE_OBJECT_COUNT. This method returns paginated values whose
     * starting index and total size can be controlled via the options object
     *
     * @param {string} host
     *        The host name for which the store values are required.
     * @param {array:string} names
     *        Array containing the names of required store objects. Empty if all
     *        items are required.
     * @param {object} options
     *        Additional options for the request containing following properties:
     *         - offset {number} : The begin index of the returned array amongst
     *                  the total values
     *         - size {number} : The number of values required.
     *         - sortOn {string} : The values should be sorted on this property.
     *         - index {string} : In case of indexed db, the IDBIndex to be used
     *                 for fetching the values.
     *
     * @return {object} An object containing following properties:
     *          - offset - The actual offset of the returned array. This might
     *                     be different from the requested offset if that was
     *                     invalid
     *          - total - The total number of entries possible.
     *          - data - The requested values.
     */
    getStoreObjects: method(async(function*(host, names, options = {}) {
      let offset = options.offset || 0;
      let size = options.size || MAX_STORE_OBJECT_COUNT;
      if (size > MAX_STORE_OBJECT_COUNT) {
        size = MAX_STORE_OBJECT_COUNT;
      }
      let sortOn = options.sortOn || "name";

      let toReturn = {
        offset: offset,
        total: 0,
        data: []
      };

      if (names) {
        for (let name of names) {
          toReturn.data.push(
            // yield because getValuesForHost is async for Indexed DB
            ...(yield this.getValuesForHost(host, name, options))
          );
        }
        toReturn.total = this.getObjectsSize(host, names, options);
        if (offset > toReturn.total) {
          // In this case, toReturn.data is an empty array.
          toReturn.offset = toReturn.total;
          toReturn.data = [];
        }
        else {
          toReturn.data = toReturn.data.sort((a,b) => {
            return a[sortOn] - b[sortOn];
          }).slice(offset, offset + size).map(a => this.toStoreObject(a));
        }
      }
      else {
        let total = yield this.getValuesForHost(host);
        toReturn.total = total.length;
        if (offset > toReturn.total) {
          // In this case, toReturn.data is an empty array.
          toReturn.offset = offset = toReturn.total;
          toReturn.data = [];
        }
        else {
          toReturn.data = total.sort((a,b) => {
            return a[sortOn] - b[sortOn];
          }).slice(offset, offset + size)
            .map(object => this.toStoreObject(object));
        }
      }

      return toReturn;
    }), {
      request: {
        host: Arg(0),
        names: Arg(1, "nullable:array:string"),
        options: Arg(2, "nullable:json")
      },
      response: RetVal(storeObjectType)
    })
  }
};

/**
 * Creates an actor and its corresponding front and registers it to the Storage
 * Actor.
 *
 * @See StorageActors.defaults()
 *
 * @param {object} options
 *        Options required by StorageActors.defaults method which are :
 *         - typeName {string}
 *                    The typeName of the actor.
 *         - observationTopic {string}
 *                            The topic which this actor listens to via
 *                            Notification Observers.
 *         - storeObjectType {string}
 *                           The RetVal type of the store object of this actor.
 * @param {object} overrides
 *        All the methods which you want to be differnt from the ones in
 *        StorageActors.defaults method plus the required ones described there.
 */
StorageActors.createActor = function(options = {}, overrides = {}) {
  let actorObject = StorageActors.defaults(
    options.typeName,
    options.observationTopic || null,
    options.storeObjectType
  );
  for (let key in overrides) {
    actorObject[key] = overrides[key];
  }

  let actor = protocol.ActorClass(actorObject);
  let front = protocol.FrontClass(actor, {
    form: function(form, detail) {
      if (detail === "actorid") {
        this.actorID = form;
        return null;
      }

      this.actorID = form.actor;
      this.hosts = form.hosts;
      return null;
    }
  });
  storageTypePool.set(actorObject.typeName, actor);
}

/**
 * The Cookies actor and front.
 */
StorageActors.createActor({
  typeName: "cookies",
  storeObjectType: "cookiestoreobject"
}, {
  initialize: function(storageActor) {
    protocol.Actor.prototype.initialize.call(this, null);

    this.storageActor = storageActor;

    this.populateStoresForHosts();
    Services.obs.addObserver(this, "cookie-changed", false);
    Services.obs.addObserver(this, "http-on-response-set-cookie", false);
    this.onWindowReady = this.onWindowReady.bind(this);
    this.onWindowDestroyed = this.onWindowDestroyed.bind(this);
    events.on(this.storageActor, "window-ready", this.onWindowReady);
    events.on(this.storageActor, "window-destroyed", this.onWindowDestroyed);
  },

  destroy: function() {
    this.hostVsStores = null;
    Services.obs.removeObserver(this, "cookie-changed", false);
    Services.obs.removeObserver(this, "http-on-response-set-cookie", false);
    events.off(this.storageActor, "window-ready", this.onWindowReady);
    events.off(this.storageActor, "window-destroyed", this.onWindowDestroyed);
  },

  /**
   * Given a cookie object, figure out all the matching hosts from the page that
   * the cookie belong to.
   */
  getMatchingHosts: function(cookies) {
    if (!cookies.length) {
      cookies = [cookies];
    }
    let hosts = new Set();
    for (let host of this.hosts) {
      for (let cookie of cookies) {
        if (this.isCookieAtHost(cookie, host)) {
          hosts.add(host);
        }
      }
    }
    return [...hosts];
  },

  /**
   * Given a cookie object and a host, figure out if the cookie is valid for
   * that host.
   */
  isCookieAtHost: function(cookie, host) {
    try {
      cookie = cookie.QueryInterface(Ci.nsICookie)
                     .QueryInterface(Ci.nsICookie2);
    } catch(ex) {
      return false;
    }
    if (cookie.host == null) {
      return host == null;
    }
    if (cookie.host.startsWith(".")) {
      return host.endsWith(cookie.host);
    }
    else {
      return cookie.host == host;
    }
  },

  toStoreObject: function(cookie) {
    if (!cookie) {
      return null;
    }

    return {
      name: cookie.name,
      path: cookie.path || "",
      host: cookie.host || "",
      expires: (cookie.expires || 0) * 1000, // because expires is in seconds
      creationTime: cookie.creationTime / 1000, // because it is in micro seconds
      lastAccessed: cookie.lastAccessed / 1000, // - do -
      value: new LongStringActor(this.conn, cookie.value || ""),
      isDomain: cookie.isDomain,
      isSecure: cookie.isSecure,
      isHttpOnly: cookie.isHttpOnly
    }
  },

  populateStoresForHost: function(host) {
    this.hostVsStores.set(host, new Map());
    let cookies = Services.cookies.getCookiesFromHost(host);
    while (cookies.hasMoreElements()) {
      let cookie = cookies.getNext().QueryInterface(Ci.nsICookie)
                          .QueryInterface(Ci.nsICookie2);
      if (this.isCookieAtHost(cookie, host)) {
        this.hostVsStores.get(host).set(cookie.name, cookie);
      }
    }
  },

  /**
   * Converts the raw cookie string returned in http request's response header
   * to a nsICookie compatible object.
   *
   * @param {string} cookieString
   *        The raw cookie string coming from response header.
   * @param {string} domain
   *        The domain of the url of the nsiChannel the cookie corresponds to.
   *        This will be used when the cookie string does not have a domain.
   *
   * @returns {[object]}
   *          An array of nsICookie like objects representing the cookies.
   */
  parseCookieString: function(cookieString, domain) {
    /**
     * Takes a date string present in raw cookie string coming from http
     * request's response headers and returns the number of milliseconds elapsed
     * since epoch. If the date string is undefined, its probably a session
     * cookie so return 0.
     */
    let parseDateString = dateString => {
      return dateString ? new Date(dateString.replace(/-/g, " ")).getTime(): 0;
    };

    let cookies = [];
    for (let string of cookieString.split("\n")) {
      let keyVals = {}, name = null;
      for (let keyVal of string.split(/;\s*/)) {
        let tokens = keyVal.split(/\s*=\s*/);
        if (!name) {
          name = tokens[0];
        }
        else {
          tokens[0] = tokens[0].toLowerCase();
        }
        keyVals[tokens.splice(0, 1)[0]] = tokens.join("=");
      }
      let expiresTime = parseDateString(keyVals.expires);
      keyVals.domain = keyVals.domain || domain;
      cookies.push({
        name: name,
        value: keyVals[name] || "",
        path: keyVals.path,
        host: keyVals.domain,
        expires: expiresTime/1000, // seconds, to match with nsiCookie.expires
        lastAccessed: expiresTime * 1000,
        // microseconds, to match with nsiCookie.lastAccessed
        creationTime: expiresTime * 1000,
        // microseconds, to match with nsiCookie.creationTime
        isHttpOnly: true,
        isSecure: keyVals.secure != null,
        isDomain: keyVals.domain.startsWith("."),
      });
    }
    return cookies;
  },

  /**
   * Notification observer for topics "http-on-response-set-cookie" and
   * "cookie-change".
   *
   * @param subject
   *        {nsiChannel} The channel associated to the SET-COOKIE response
   *        header in case of "http-on-response-set-cookie" topic.
   *        {nsiCookie|[nsiCookie]} A single nsiCookie object or a list of it
   *        depending on the action. Array is only in case of "batch-deleted"
   *        action.
   * @param {string} topic
   *        The topic of the notification.
   * @param {string} action
   *        Additional data associated with the notification. Its the type of
   *        cookie change in case of "cookie-change" topic and the cookie string
   *        in case of "http-on-response-set-cookie" topic.
   */
  observe: function(subject, topic, action) {
    if (topic == "http-on-response-set-cookie") {
      // Some cookies got created as a result of http response header SET-COOKIE
      // subject here is an nsIChannel object referring to the http request.
      // We get the requestor of this channel and thus the content window.
      let channel = subject.QueryInterface(Ci.nsIChannel);
      let requestor = channel.notificationCallbacks ||
                      channel.loadGroup.notificationCallbacks;
      // requester can be null sometimes.
      let window = requestor ? requestor.getInterface(Ci.nsIDOMWindow): null;
      // Proceed only if this window is present on the currently targetted tab
      if (window && this.storageActor.isIncludedInTopLevelWindow(window)) {
        let host = this.getHostName(window.location);
        if (this.hostVsStores.has(host)) {
          let cookies = this.parseCookieString(action, channel.URI.host);
          let data = {};
          data[host] =  [];
          for (let cookie of cookies) {
            if (this.hostVsStores.get(host).has(cookie.name)) {
              continue;
            }
            this.hostVsStores.get(host).set(cookie.name, cookie);
            data[host].push(cookie.name);
          }
          if (data[host]) {
            this.storageActor.update("added", "cookies", data);
          }
        }
      }
      return null;
    }

    if (topic != "cookie-changed") {
      return null;
    }

    let hosts = this.getMatchingHosts(subject);
    let data = {};

    switch(action) {
      case "added":
      case "changed":
        if (hosts.length) {
          subject = subject.QueryInterface(Ci.nsICookie)
                           .QueryInterface(Ci.nsICookie2);
          for (let host of hosts) {
            this.hostVsStores.get(host).set(subject.name, subject);
            data[host] = [subject.name];
          }
          this.storageActor.update(action, "cookies", data);
        }
        break;

      case "deleted":
        if (hosts.length) {
          subject = subject.QueryInterface(Ci.nsICookie)
                           .QueryInterface(Ci.nsICookie2);
          for (let host of hosts) {
            this.hostVsStores.get(host).delete(subject.name);
            data[host] = [subject.name];
          }
          this.storageActor.update("deleted", "cookies", data);
        }
        break;

      case "batch-deleted":
        if (hosts.length) {
          for (let host of hosts) {
            let stores = [];
            for (let cookie of subject) {
              cookie = cookie.QueryInterface(Ci.nsICookie)
                             .QueryInterface(Ci.nsICookie2);
              this.hostVsStores.get(host).delete(cookie.name);
              stores.push(cookie.name);
            }
            data[host] = stores;
          }
          this.storageActor.update("deleted", "cookies", data);
        }
        break;

      case "cleared":
        this.storageActor.update("cleared", "cookies", hosts);
        break;

      case "reload":
        this.storageActor.update("reloaded", "cookies", hosts);
        break;
    }
    return null;
  },
});


/**
 * Helper method to create the overriden object required in
 * StorageActors.createActor for Local Storage and Session Storage.
 * This method exists as both Local Storage and Session Storage have almost
 * identical actors.
 */
function getObjectForLocalOrSessionStorage(type) {
  return {
    getNamesForHost: function(host) {
      let storage = this.hostVsStores.get(host);
      return [key for (key in storage)];
    },

    getValuesForHost: function(host, name) {
      let storage = this.hostVsStores.get(host);
      if (name) {
        return [{name: name, value: storage.getItem(name)}];
      }
      return [{name: name, value: storage.getItem(name)} for (name in storage)];
    },

    getHostName: function(location) {
      if (!location.host) {
        return location.href;
      }
      return location.protocol + "//" + location.host;
    },

    populateStoresForHost: function(host, window) {
      try {
        this.hostVsStores.set(host, window[type]);
      } catch(ex) {
        // Exceptions happen when local or session storage is inaccessible
      }
      return null;
    },

    populateStoresForHosts: function() {
      this.hostVsStores = new Map();
      try {
        for (let window of this.windows) {
          this.hostVsStores.set(this.getHostName(window.location), window[type]);
        }
      } catch(ex) {
        // Exceptions happen when local or session storage is inaccessible
      }
      return null;
    },

    observe: function(subject, topic, data) {
      if (topic != "dom-storage2-changed" || data != type) {
        return null;
      }

      let host = this.getSchemaAndHost(subject.url);

      if (!this.hostVsStores.has(host)) {
        return null;
      }

      let action = "changed";
      if (subject.key == null) {
        return this.storageActor.update("cleared", type, [host]);
      }
      else if (subject.oldValue == null) {
        action = "added";
      }
      else if (subject.newValue == null) {
        action = "deleted";
      }
      let updateData = {};
      updateData[host] = [subject.key];
      return this.storageActor.update(action, type, updateData);
    },

    /**
     * Given a url, correctly determine its protocol + hostname part.
     */
    getSchemaAndHost: function(url) {
      let uri = Services.io.newURI(url, null, null);
      return uri.scheme + "://" + uri.hostPort;
    },

    toStoreObject: function(item) {
      if (!item) {
        return null;
      }

      return {
        name: item.name,
        value: new LongStringActor(this.conn, item.value || "")
      };
    },
  }
};

/**
 * The Local Storage actor and front.
 */
StorageActors.createActor({
  typeName: "localStorage",
  observationTopic: "dom-storage2-changed",
  storeObjectType: "storagestoreobject"
}, getObjectForLocalOrSessionStorage("localStorage"));

/**
 * The Session Storage actor and front.
 */
StorageActors.createActor({
  typeName: "sessionStorage",
  observationTopic: "dom-storage2-changed",
  storeObjectType: "storagestoreobject"
}, getObjectForLocalOrSessionStorage("sessionStorage"));


/**
 * Code related to the Indexed DB actor and front
 */

// Metadata holder objects for various components of Indexed DB

/**
 * Meta data object for a particular index in an object store
 *
 * @param {IDBIndex} index
 *        The particular index from the object store.
 */
function IndexMetadata(index) {
  this._name = index.name;
  this._keyPath = index.keyPath;
  this._unique = index.unique;
  this._multiEntry = index.multiEntry;
}
IndexMetadata.prototype = {
  toObject: function() {
    return {
      name: this._name,
      keyPath: this._keyPath,
      unique: this._unique,
      multiEntry: this._multiEntry
    };
  }
};

/**
 * Meta data object for a particular object store in a db
 *
 * @param {IDBObjectStore} objectStore
 *        The particular object store from the db.
 */
function ObjectStoreMetadata(objectStore) {
  this._name = objectStore.name;
  this._keyPath = objectStore.keyPath;
  this._autoIncrement = objectStore.autoIncrement;
  this._indexes = new Map();

  for (let i = 0; i < objectStore.indexNames.length; i++) {
    let index = objectStore.index(objectStore.indexNames[i]);
    this._indexes.set(index, new IndexMetadata(index));
  }
}
ObjectStoreMetadata.prototype = {
  toObject: function() {
    return {
      name: this._name,
      keyPath: this._keyPath,
      autoIncrement: this._autoIncrement,
      indexes: JSON.stringify(
        [index.toObject() for (index of this._indexes.values())]
      )
    };
  }
};

/**
 * Meta data object for a particular indexed db in a host.
 *
 * @param {string} origin
 *        The host associated with this indexed db.
 * @param {IDBDatabase} db
 *        The particular indexed db.
 */
function DatabaseMetadata(origin, db) {
  this._origin = origin;
  this._name = db.name;
  this._version = db.version;
  this._objectStores = new Map();

  if (db.objectStoreNames.length) {
    let transaction = db.transaction(db.objectStoreNames, "readonly");

    for (let i = 0; i < transaction.objectStoreNames.length; i++) {
      let objectStore =
        transaction.objectStore(transaction.objectStoreNames[i]);
      this._objectStores.set(transaction.objectStoreNames[i],
                             new ObjectStoreMetadata(objectStore));
    }
  }
};
DatabaseMetadata.prototype = {
  get objectStores() {
    return this._objectStores;
  },

  toObject: function() {
    return {
      name: this._name,
      origin: this._origin,
      version: this._version,
      objectStores: this._objectStores.size
    };
  }
};

StorageActors.createActor({
  typeName: "indexedDB",
  storeObjectType: "idbstoreobject"
}, {
  initialize: function(storageActor) {
    protocol.Actor.prototype.initialize.call(this, null);
    this.objectsSize = {};
    this.storageActor = storageActor;
    this.onWindowReady = this.onWindowReady.bind(this);
    this.onWindowDestroyed = this.onWindowDestroyed.bind(this);
    events.on(this.storageActor, "window-ready", this.onWindowReady);
    events.on(this.storageActor, "window-destroyed", this.onWindowDestroyed);
  },

  destroy: function() {
    this.hostVsStores = null;
    this.objectsSize = null;
    events.off(this.storageActor, "window-ready", this.onWindowReady);
    events.off(this.storageActor, "window-destroyed", this.onWindowDestroyed);
  },

  getHostName: function(location) {
    if (!location.host) {
      return location.href;
    }
    return location.protocol + "//" + location.host;
  },

  /**
   * This method is overriden and left blank as for indexedDB, this operation
   * cannot be performed synchronously. Thus, the preListStores method exists to
   * do the same task asynchronously.
   */
  populateStoresForHosts: function() {
  },

  getNamesForHost: function(host) {
    let names = [];
    for (let [dbName, metaData] of this.hostVsStores.get(host)) {
      for (let objectStore of metaData.objectStores.keys()) {
        names.push(JSON.stringify([dbName, objectStore]));
      }
    }
    return names;
  },

  /**
   * Returns all or requested entries from a particular objectStore from the db
   * in the given host.
   *
   * @param {string} host
   *        The given host.
   * @param {string} dbName
   *        The name of the indexed db from the above host.
   * @param {string} objectStore
   *        The name of the object store from the above db.
   * @param {string} id
   *        id of the requested entry from the above object store.
   *        null if all entries from the above object store are requested.
   * @param {string} index
   *        name of the IDBIndex to be iterated on while fetching entries.
   *        null or "name" if no index is to be iterated.
   * @param {number} offset
   *        ofsset of the entries to be fetched.
   * @param {number} size
   *        The intended size of the entries to be fetched.
   */
  getObjectStoreData:
  function(host, dbName, objectStore, id, index, offset, size) {
    let request = this.openWithOrigin(host, dbName);
    let success = Promise.defer();
    let data = [];
    if (!size || size > MAX_STORE_OBJECT_COUNT) {
      size = MAX_STORE_OBJECT_COUNT;
    }

    request.onsuccess = event => {
      let db = event.target.result;

      let transaction = db.transaction(objectStore, "readonly");
      let source = transaction.objectStore(objectStore);
      if (index && index != "name") {
        source = source.index(index);
      }

      source.count().onsuccess = event => {
        let count = event.target.result;
        this.objectsSize[host + dbName + objectStore + index] = count;

        if (!offset) {
          offset = 0;
        }
        else if (offset > count) {
          db.close();
          success.resolve([]);
          return;
        }

        if (id) {
          source.get(id).onsuccess = event => {
            db.close();
            success.resolve([{name: id, value: event.target.result}]);
          };
        }
        else {
          source.openCursor().onsuccess = event => {
            let cursor = event.target.result;

            if (!cursor || data.length >= size) {
              db.close();
              success.resolve(data);
              return;
            }
            if (offset-- <= 0) {
              data.push({name: cursor.key, value: cursor.value});
            }
            cursor.continue();
          };
        }
      };
    };
    request.onerror = () => {
      db.close();
      success.resolve([]);
    };
    return success.promise;
  },

  /**
   * Returns the total number of entries for various types of requests to
   * getStoreObjects for Indexed DB actor.
   *
   * @param {string} host
   *        The host for the request.
   * @param {array:string} names
   *        Array of stringified name objects for indexed db actor.
   *        The request type depends on the length of any parsed entry from this
   *        array. 0 length refers to request for the whole host. 1 length
   *        refers to request for a particular db in the host. 2 length refers
   *        to a particular object store in a db in a host. 3 length refers to
   *        particular items of an object store in a db in a host.
   * @param {object} options
   *        An options object containing following properties:
   *         - index {string} The IDBIndex for the object store in the db.
   */
  getObjectsSize: function(host, names, options) {
    // In Indexed DB, we are interested in only the first name, as the pattern
    // should follow in all entries.
    let name = names[0];
    let parsedName = JSON.parse(name);

    if (parsedName.length == 3) {
      // This is the case where specific entries from an object store were
      // requested
      return names.length;
    }
    else if (parsedName.length == 2) {
      // This is the case where all entries from an object store are requested.
      let index = options.index;
      let [db, objectStore] = parsedName;
      if (this.objectsSize[host + db + objectStore + index]) {
        return this.objectsSize[host + db + objectStore + index];
      }
    }
    else if (parsedName.length == 1) {
      // This is the case where details of all object stores in a db are
      // requested.
      if (this.hostVsStores.has(host) &&
          this.hostVsStores.get(host).has(parsedName[0])) {
        return this.hostVsStores.get(host).get(parsedName[0]).objectStores.size;
      }
    }
    else if (!parsedName || !parsedName.length) {
      // This is the case were details of all dbs in a host are requested.
      if (this.hostVsStores.has(host)) {
        return this.hostVsStores.get(host).size;
      }
    }
    return 0;
  },

  getValuesForHost: async(function*(host, name = "null", options) {
    name = JSON.parse(name);
    if (!name || !name.length) {
      // This means that details about the db in this particular host are
      // requested.
      let dbs = [];
      if (this.hostVsStores.has(host)) {
        for (let [dbName, db] of this.hostVsStores.get(host)) {
          dbs.push(db.toObject());
        }
      }
      return dbs;
    }
    let [db, objectStore, id] = name;
    if (!objectStore) {
      // This means that details about all the object stores in this db are
      // requested.
      let objectStores = [];
      if (this.hostVsStores.has(host) && this.hostVsStores.get(host).has(db)) {
        for (let objectStore of this.hostVsStores.get(host).get(db).objectStores) {
          objectStores.push(objectStore[1].toObject());
        }
      }
      return objectStores;
    }
    // Get either all entries from the object store, or a particular id
    return yield this.getObjectStoreData(host, db, objectStore, id,
                                         options.index, options.size);
  }),

  /**
   * Purpose of this method is same as populateStoresForHosts but this is async.
   * This exact same operation cannot be performed in populateStoresForHosts
   * method, as that method is called in initialize method of the actor, which
   * cannot be asynchronous.
   */
  preListStores: async(function*() {
    this.hostVsStores = new Map();
    for (let host of this.hosts) {
      yield this.populateStoresForHost(host);
    }
  }),

  populateStoresForHost: async(function*(host) {
    let storeMap = new Map();
    for (let name of (yield this.getDBNamesForHost(host))) {
      storeMap.set(name, yield this.getDBMetaData(host, name));
    }
    this.hostVsStores.set(host, storeMap);
  }),

  /**
   * Removes any illegal characters from the host name to make it a valid file
   * name.
   */
  getSanitizedHost: function(host) {
    return host.replace(ILLEGAL_CHAR_REGEX, "+");
  },

  /**
   * Opens an indexed db connection for the given `host` and database `name`.
   */
  openWithOrigin: function(host, name) {
    let principal;

    if (/^(about:|chrome:)/.test(host)) {
      principal = Services.scriptSecurityManager.getSystemPrincipal();
    }
    else {
      let uri = Services.io.newURI(host, null, null);
      principal = Services.scriptSecurityManager.getCodebasePrincipal(uri);
    }

    return indexedDB.openForPrincipal(principal, name);
  },

  /**
   * Fetches and stores all the metadata information for the given database
   * `name` for the given `host`. The stored metadata information is of
   * `DatabaseMetadata` type.
   */
  getDBMetaData: function(host, name) {
    let request = this.openWithOrigin(host, name);
    let success = Promise.defer();
    request.onsuccess = event => {
      let db = event.target.result;

      let dbData = new DatabaseMetadata(host, db);
      db.close();
      success.resolve(dbData);
    };
    request.onerror = event => {
      console.error("Error opening indexeddb database " + name + " for host " +
                    host);
      success.resolve(null);
    };
    return success.promise;
  },

  /**
   * Retrives the proper indexed db database name from the provided .sqlite file
   * location.
   */
  getNameFromDatabaseFile: async(function*(path) {
    let connection = null;
    let retryCount = 0;

    // Content pages might be having an open transaction for the same indexed db
    // which this sqlite file belongs to. In that case, sqlite.openConnection
    // will throw. Thus we retey for some time to see if lock is removed.
    while (!connection && retryCount++ < 25) {
      try {
        connection = yield Sqlite.openConnection({ path: path });
      }
      catch (ex) {
        // Continuously retrying is overkill. Waiting for 100ms before next try
        yield sleep(100);
      }
    }

    if (!connection) {
      return null;
    }

    let rows = yield connection.execute("SELECT name FROM database");
    if (rows.length != 1) {
      return null;
    }

    let name = rows[0].getResultByName("name");

    yield connection.close();

    return name;
  }),

  /**
   * Fetches all the databases and their metadata for the given `host`.
   */
  getDBNamesForHost: async(function*(host) {
    let sanitizedHost = this.getSanitizedHost(host);
    let directory = OS.Path.join(OS.Constants.Path.profileDir, "storage",
                                 "persistent", sanitizedHost, "idb");

    let exists = yield OS.File.exists(directory);
    if (!exists && host.startsWith("about:")) {
      // try for moz-safe-about directory
      sanitizedHost = this.getSanitizedHost("moz-safe-" + host);
      directory = OS.Path.join(OS.Constants.Path.profileDir, "storage",
                               "persistent", sanitizedHost, "idb");
      exists = yield OS.File.exists(directory);
    }
    if (!exists) {
      return [];
    }

    let names = [];
    let dirIterator = new OS.File.DirectoryIterator(directory);
    try {
      yield dirIterator.forEach(file => {
        // Skip directories.
        if (file.isDir) {
          return null;
        }

        // Skip any non-sqlite files.
        if (!file.name.endsWith(".sqlite")) {
          return null;
        }

        return this.getNameFromDatabaseFile(file.path).then(name => {
          if (name) {
            names.push(name);
          }
          return null;
        });
      });
    }
    finally {
      dirIterator.close();
    }
    return names;
  }),

  /**
   * Returns the over-the-wire implementation of the indexed db entity.
   */
  toStoreObject: function(item) {
    if (!item) {
      return null;
    }

    if (item.indexes) {
      // Object store meta data
      return {
        objectStore: item.name,
        keyPath: item.keyPath,
        autoIncrement: item.autoIncrement,
        indexes: item.indexes
      };
    }
    if (item.objectStores) {
      // DB meta data
      return {
        db: item.name,
        origin: item.origin,
        version: item.version,
        objectStores: item.objectStores
      };
    }
    // Indexed db entry
    return {
      name: item.name,
      value: new LongStringActor(this.conn, JSON.stringify(item.value))
    };
  },

  form: function(form, detail) {
    if (detail === "actorid") {
      return this.actorID;
    }

    let hosts = {};
    for (let host of this.hosts) {
      hosts[host] = this.getNamesForHost(host);
    }

    return {
      actor: this.actorID,
      hosts: hosts
    };
  },
});

/**
 * The main Storage Actor.
 */
let StorageActor = exports.StorageActor = protocol.ActorClass({
  typeName: "storage",

  get window() {
    return this.parentActor.window;
  },

  get document() {
    return this.parentActor.window.document;
  },

  get windows() {
    return this.childWindowPool;
  },

  /**
   * List of event notifications that the server can send to the client.
   *
   *  - stores-update : When any store object in any storage type changes.
   *  - stores-cleared : When all the store objects are removed.
   *  - stores-reloaded : When all stores are reloaded. This generally mean that
   *                      we should refetch everything again.
   */
  events: {
    "stores-update": {
      type: "storesUpdate",
      data: Arg(0, "storeUpdateObject")
    },
    "stores-cleared": {
      type: "storesCleared",
      data: Arg(0, "json")
    },
    "stores-reloaded": {
      type: "storesRelaoded",
      data: Arg(0, "json")
    }
  },

  initialize: function (conn, tabActor) {
    protocol.Actor.prototype.initialize.call(this, null);

    this.conn = conn;
    this.parentActor = tabActor;

    this.childActorPool = new Map();
    this.childWindowPool = new Set();

    // Fetch all the inner iframe windows in this tab.
    this.fetchChildWindows(this.parentActor.docShell);

    // Initialize the registered store types
    for (let [store, actor] of storageTypePool) {
      this.childActorPool.set(store, new actor(this));
    }

    // Notifications that help us keep track of newly added windows and windows
    // that got removed
    Services.obs.addObserver(this, "content-document-global-created", false);
    Services.obs.addObserver(this, "inner-window-destroyed", false);
    this.onPageChange = this.onPageChange.bind(this);
    tabActor.browser.addEventListener("pageshow", this.onPageChange, true);
    tabActor.browser.addEventListener("pagehide", this.onPageChange, true);

    this.destroyed = false;
    this.boundUpdate = {};
    // The time which periodically flushes and transfers the updated store
    // objects.
    this.updateTimer = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
    this.updateTimer.initWithCallback(this , UPDATE_INTERVAL,
      Ci.nsITimer.TYPE_REPEATING_PRECISE_CAN_SKIP);

    // Layout helper for window.parent and window.top helper methods that work
    // accross devices.
    this.layoutHelper = new LayoutHelpers(this.window);
  },

  destroy: function() {
    this.updateTimer.cancel();
    this.updateTimer = null;
    this.layoutHelper = null;
    // Remove observers
    Services.obs.removeObserver(this, "content-document-global-created", false);
    Services.obs.removeObserver(this, "inner-window-destroyed", false);
    this.destroyed = true;
    if (this.parentActor.browser) {
      this.parentActor.browser.removeEventListener(
        "pageshow", this.onPageChange, true);
      this.parentActor.browser.removeEventListener(
        "pagehide", this.onPageChange, true);
    }
    // Destroy the registered store types
    for (let actor of this.childActorPool.values()) {
      actor.destroy();
    }
    this.childActorPool.clear();
    this.childWindowPool.clear();
    this.childWindowPool = this.childActorPool = null;
  },

  /**
   * Given a docshell, recursively find otu all the child windows from it.
   *
   * @param {nsIDocShell} item
   *        The docshell from which all inner windows need to be extracted.
   */
  fetchChildWindows: function(item) {
    let docShell = item.QueryInterface(Ci.nsIDocShell)
                       .QueryInterface(Ci.nsIDocShellTreeItem);
    if (!docShell.contentViewer) {
      return null;
    }
    let window = docShell.contentViewer.DOMDocument.defaultView;
    if (window.location.href == "about:blank") {
      // Skip out about:blank windows as Gecko creates them multiple times while
      // creating any global.
      return null;
    }
    this.childWindowPool.add(window);
    for (let i = 0; i < docShell.childCount; i++) {
      let child = docShell.getChildAt(i);
      this.fetchChildWindows(child);
    }
    return null;
  },

  isIncludedInTopLevelWindow: function(window) {
    return this.layoutHelper.isIncludedInTopLevelWindow(window);
  },

  getWindowFromInnerWindowID: function(innerID) {
    innerID = innerID.QueryInterface(Ci.nsISupportsPRUint64).data;
    for (let win of this.childWindowPool.values()) {
      let id = win.QueryInterface(Ci.nsIInterfaceRequestor)
                   .getInterface(Ci.nsIDOMWindowUtils)
                   .currentInnerWindowID;
      if (id == innerID) {
        return win;
      }
    }
    return null;
  },

  /**
   * Event handler for any docshell update. This lets us figure out whenever
   * any new window is added, or an existing window is removed.
   */
  observe: function(subject, topic, data) {
    if (subject.location &&
        (!subject.location.href || subject.location.href == "about:blank")) {
      return null;
    }
    if (topic == "content-document-global-created" &&
        this.isIncludedInTopLevelWindow(subject)) {
      this.childWindowPool.add(subject);
      events.emit(this, "window-ready", subject);
    }
    else if (topic == "inner-window-destroyed") {
      let window = this.getWindowFromInnerWindowID(subject);
      if (window) {
        this.childWindowPool.delete(window);
        events.emit(this, "window-destroyed", window);
      }
    }
    return null;
  },

  /**
   * Called on "pageshow" or "pagehide" event on the chromeEventHandler of
   * current tab.
   *
   * @param {event} The event object passed to the handler. We are using these
   *        three properties from the event:
   *         - target {document} The document corresponding to the event.
   *         - type {string} Name of the event - "pageshow" or "pagehide".
   *         - persisted {boolean} true if there was no
   *                     "content-document-global-created" notification along
   *                     this event.
   */
  onPageChange: function({target, type, persisted}) {
    if (this.destroyed) {
      return;
    }
    let window = target.defaultView;
    if (type == "pagehide" && this.childWindowPool.delete(window)) {
      events.emit(this, "window-destroyed", window)
    }
    else if (type == "pageshow" && persisted  && window.location.href &&
             window.location.href != "about:blank" &&
             this.isIncludedInTopLevelWindow(window)) {
      this.childWindowPool.add(window);
      events.emit(this, "window-ready", window);
    }
  },

  /**
   * Lists the available hosts for all the registered storage types.
   *
   * @returns {object} An object containing with the following structure:
   *  - <storageType> : [{
   *      actor: <actorId>,
   *      host: <hostname>
   *    }]
   */
  listStores: method(async(function*() {
    let toReturn = {};
    for (let [name, value] of this.childActorPool) {
      if (value.preListStores) {
        yield value.preListStores();
      }
      toReturn[name] = value;
    }
    return toReturn;
  }), {
    response: RetVal(types.addDictType("storelist", getRegisteredTypes()))
  }),

  /**
   * Notifies the client front with the updates in stores at regular intervals.
   */
  notify: function() {
    if (!this.updatePending || this.updatingUpdateObject) {
      return null;
    }
    events.emit(this, "stores-update", this.boundUpdate);
    this.boundUpdate = {};
    this.updatePending = false;
    return null;
  },

  /**
   * This method is called by the registered storage types so as to tell the
   * Storage Actor that there are some changes in the stores. Storage Actor then
   * notifies the client front about these changes at regular (UPDATE_INTERVAL)
   * interval.
   *
   * @param {string} action
   *        The type of change. One of "added", "changed" or "deleted"
   * @param {string} storeType
   *        The storage actor in which this change has occurred.
   * @param {object} data
   *        The update object. This object is of the following format:
   *         - {
   *             <host1>: [<store_names1>, <store_name2>...],
   *             <host2>: [<store_names34>...],
   *           }
   *           Where host1, host2 are the host in which this change happened and
   *           [<store_namesX] is an array of the names of the changed store
   *           objects. Leave it empty if the host was completely removed.
   *        When the action is "reloaded" or "cleared", `data` is an array of
   *        hosts for which the stores were cleared or reloaded.
   */
  update: function(action, storeType, data) {
    if (action == "cleared" || action == "reloaded") {
      let toSend = {};
      toSend[storeType] = data
      events.emit(this, "stores-" + action, toSend);
      return null;
    }

    this.updatingUpdateObject = true;
    if (!this.boundUpdate[action]) {
      this.boundUpdate[action] = {};
    }
    if (!this.boundUpdate[action][storeType]) {
      this.boundUpdate[action][storeType] = {};
    }
    this.updatePending = true;
    for (let host in data) {
      if (!this.boundUpdate[action][storeType][host] || action == "deleted") {
        this.boundUpdate[action][storeType][host] = data[host];
      }
      else {
        this.boundUpdate[action][storeType][host] =
        this.boundUpdate[action][storeType][host].concat(data[host]);
      }
    }
    if (action == "added") {
      // If the same store name was previously deleted or changed, but now is
      // added somehow, dont send the deleted or changed update.
      this.removeNamesFromUpdateList("deleted", storeType, data);
      this.removeNamesFromUpdateList("changed", storeType, data);
    }
    else if (action == "changed" && this.boundUpdate.added &&
             this.boundUpdate.added[storeType]) {
      // If something got added and changed at the same time, then remove those
      // items from changed instead.
      this.removeNamesFromUpdateList("changed", storeType,
                                     this.boundUpdate.added[storeType]);
    }
    else if (action == "deleted") {
      // If any item got delete, or a host got delete, no point in sending
      // added or changed update
      this.removeNamesFromUpdateList("added", storeType, data);
      this.removeNamesFromUpdateList("changed", storeType, data);
      for (let host in data) {
        if (data[host].length == 0 && this.boundUpdate.added &&
            this.boundUpdate.added[storeType] &&
            this.boundUpdate.added[storeType][host]) {
          delete this.boundUpdate.added[storeType][host];
        }
        if (data[host].length == 0 && this.boundUpdate.changed &&
            this.boundUpdate.changed[storeType] &&
            this.boundUpdate.changed[storeType][host]) {
          delete this.boundUpdate.changed[storeType][host];
        }
      }
    }
    this.updatingUpdateObject = false;
    return null;
  },

  /**
   * This method removes data from the this.boundUpdate object in the same
   * manner like this.update() adds data to it.
   *
   * @param {string} action
   *        The type of change. One of "added", "changed" or "deleted"
   * @param {string} storeType
   *        The storage actor for which you want to remove the updates data.
   * @param {object} data
   *        The update object. This object is of the following format:
   *         - {
   *             <host1>: [<store_names1>, <store_name2>...],
   *             <host2>: [<store_names34>...],
   *           }
   *           Where host1, host2 are the hosts which you want to remove and
   *           [<store_namesX] is an array of the names of the store objects.
   */
  removeNamesFromUpdateList: function(action, storeType, data) {
    for (let host in data) {
      if (this.boundUpdate[action] && this.boundUpdate[action][storeType] &&
          this.boundUpdate[action][storeType][host]) {
        for (let name in data[host]) {
          let index = this.boundUpdate[action][storeType][host].indexOf(name);
          if (index > -1) {
            this.boundUpdate[action][storeType][host].splice(index, 1);
          }
        }
        if (!this.boundUpdate[action][storeType][host].length) {
          delete this.boundUpdate[action][storeType][host];
        }
      }
    }
    return null;
  }
});

/**
 * Front for the Storage Actor.
 */
let StorageFront = exports.StorageFront = protocol.FrontClass(StorageActor, {
  initialize: function(client, tabForm) {
    protocol.Front.prototype.initialize.call(this, client);
    this.actorID = tabForm.storageActor;

    client.addActorPool(this);
    this.manage(this);
  }
});
