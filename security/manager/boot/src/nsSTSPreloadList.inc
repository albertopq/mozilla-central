/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*****************************************************************************/
/* This is an automatically generated file. If you're not                    */
/* nsSiteSecurityService.cpp, you shouldn't be #including it.     */
/*****************************************************************************/

#include <stdint.h>
const PRTime gPreloadListExpirationTime = INT64_C(1432984708039000);

class nsSTSPreload
{
  public:
    const char *mHost;
    const bool mIncludeSubdomains;
};

static const nsSTSPreload kSTSPreloadList[] = {
  { "007sascha.de", true },
  { "0x0a.net", true },
  { "17hats.com", true },
  { "18f.gsa.gov", true },
  { "1a-diamantscheiben.de", true },
  { "1a-vermessung.at", true },
  { "1a-werkstattgeraete.de", true },
  { "2048game.co.uk", true },
  { "302.nyc", true },
  { "accounts.firefox.com", true },
  { "accounts.google.com", true },
  { "aclu.org", false },
  { "activiti.alfresco.com", false },
  { "adamkostecki.de", true },
  { "addvocate.com", true },
  { "adlershop.ch", true },
  { "admin.google.com", true },
  { "adorai.tk", true },
  { "adsfund.org", true },
  { "ahoyconference.com", true },
  { "ahwatukeefoothillsmontessori.com", true },
  { "aie.de", true },
  { "aiticon.com", true },
  { "aladdinschools.appspot.com", false },
  { "alainwolf.net", true },
  { "alecvannoten.be", true },
  { "alexsexton.com", true },
  { "alexyang.me", true },
  { "alpha.irccloud.com", false },
  { "anadoluefessporkulubu.org", true },
  { "andreasbreitenlohner.de", true },
  { "anetaben.nl", true },
  { "angularjs.org", true },
  { "anime.my", false },
  { "animurecs.com", true },
  { "ankarakart.com.tr", true },
  { "annahmeschluss.de", true },
  { "annevankesteren.com", true },
  { "annevankesteren.nl", true },
  { "annevankesteren.org", true },
  { "ansdell.net", true },
  { "anycoin.me", true },
  { "apadvantage.com", true },
  { "api.intercom.io", false },
  { "api.lookout.com", false },
  { "api.simple.com", false },
  { "api.xero.com", false },
  { "apis.google.com", true },
  { "apn-einstellungen.de", true },
  { "app.lookout.com", false },
  { "app.manilla.com", true },
  { "app.recurly.com", true },
  { "app.simpletax.ca", false },
  { "app.yinxiang.com", false },
  { "appengine.google.com", true },
  { "aprz.de", true },
  { "archlinux.de", true },
  { "arendburgers.nl", true },
  { "arguggi.co.uk", true },
  { "arivo.com.br", true },
  { "arlen.io", true },
  { "atavio.at", true },
  { "atavio.ch", true },
  { "atavio.de", true },
  { "atlassian.net", true },
  { "atte.fi", true },
  { "auf-feindgebiet.de", true },
  { "authentication.io", true },
  { "azabani.com", true },
  { "baer.im", true },
  { "balcan-underground.net", true },
  { "baldwinkoo.com", true },
  { "balikonos.cz", true },
  { "bank.simple.com", false },
  { "barcodeberlin.com", true },
  { "barslecht.com", true },
  { "barslecht.nl", true },
  { "baruch.me", true },
  { "bassh.net", true },
  { "bautied.de", true },
  { "bayrisch-fuer-anfaenger.de", true },
  { "bccx.com", true },
  { "bcrook.com", false },
  { "beamitapp.com", true },
  { "beastowner.com", true },
  { "beastowner.li", true },
  { "bedeta.de", true },
  { "bedreid.dk", true },
  { "beneathvt.com", true },
  { "benjamins.com", true },
  { "best-wedding-quotes.com", true },
  { "bgneuesheim.de", true },
  { "bhatia.at", true },
  { "biathloncup.ru", true },
  { "big-andy.co.uk", true },
  { "bigbrownpromotions.com.au", true },
  { "bitbucket.org", false },
  { "bitfactory.ws", true },
  { "bitmex.com", true },
  { "bitmon.net", true },
  { "bitpod.de", true },
  { "bjornjohansen.no", true },
  { "bl4ckb0x.com", true },
  { "bl4ckb0x.de", true },
  { "bl4ckb0x.info", true },
  { "bl4ckb0x.net", true },
  { "bl4ckb0x.org", true },
  { "blacklane.com", true },
  { "blessnet.jp", true },
  { "blockchain.info", true },
  { "blocksatz-medien.de", true },
  { "blog.cyveillance.com", true },
  { "blog.gparent.org", true },
  { "blog.linode.com", false },
  { "blog.torproject.org", false },
  { "blubbablasen.de", true },
  { "bodo-wolff.de", true },
  { "bohramt.de", true },
  { "bonigo.de", true },
  { "bookingapp.nl", true },
  { "boxcryptor.com", true },
  { "brage.info", true },
  { "brainfork.ml", true },
  { "brakemanpro.com", true },
  { "broeselei.at", true },
  { "brossmanit.com", true },
  { "brunosouza.org", true },
  { "buddhistische-weisheiten.org", true },
  { "bugzil.la", true },
  { "bugzilla.mozilla.org", true },
  { "bulktrade.de", true },
  { "business.lookout.com", false },
  { "buttercoin.com", true },
  { "buzzconcert.com", true },
  { "bytepark.de", false },
  { "ca.gparent.org", false },
  { "cackette.com", true },
  { "call.me", true },
  { "calomel.org", true },
  { "camolist.com", true },
  { "caremad.io", true },
  { "carezone.com", false },
  { "cartouche24.eu", true },
  { "cartucce24.it", true },
  { "cdnb.co", true },
  { "celltek-server.de", false },
  { "certible.com", true },
  { "certly.io", true },
  { "chahub.com", true },
  { "chainmonitor.com", true },
  { "chatbot.me", true },
  { "check.torproject.org", false },
  { "checkout.google.com", true },
  { "chm.vn", true },
  { "chontalpa.pw", true },
  { "chrisjean.com", true },
  { "chrome-devtools-frontend.appspot.com", true },
  { "chrome.google.com", true },
  { "chromiumcodereview.appspot.com", false },
  { "chulado.com", true },
  { "cktennis.com", true },
  { "clan-ww.com", true },
  { "clapping-rhymes.com", true },
  { "clerkendweller.uk", true },
  { "clintwilson.technology", true },
  { "cloud.google.com", true },
  { "cloudcert.org", true },
  { "cloudns.com.au", true },
  { "cloudsecurityalliance.org", true },
  { "cloudstoragemaus.com", true },
  { "cloudup.com", true },
  { "code-poets.co.uk", true },
  { "code.google.com", true },
  { "codepref.com", true },
  { "codereview.appspot.com", false },
  { "codereview.chromium.org", true },
  { "coffeeetc.co.uk", true },
  { "coinapult.com", true },
  { "comdurav.com", true },
  { "comssa.org.au", true },
  { "config.schokokeks.org", true },
  { "conformal.com", true },
  { "conrad-kostecki.de", true },
  { "controlcenter.gigahost.dk", true },
  { "cor-ser.es", true },
  { "cordial-restaurant.com", true },
  { "cotonea.de", true },
  { "crm.onlime.ch", false },
  { "crowdjuris.com", true },
  { "crypto.cat", false },
  { "cryptopartyatx.org", true },
  { "cs50.harvard.edu", true },
  { "cspbuilder.info", true },
  { "cube.de", true },
  { "cupcake.io", true },
  { "cupcake.is", true },
  { "curiosity-driven.org", true },
  { "curlybracket.co.uk", true },
  { "cyanogenmod.xxx", true },
  { "cybershambles.com", true },
  { "cybozu.com", true },
  { "cyon.ch", true },
  { "cyphertite.com", true },
  { "czbix.com", true },
  { "daphne.informatik.uni-freiburg.de", true },
  { "darchoods.net", false },
  { "data-abundance.com", true },
  { "data.qld.gov.au", false },
  { "datenkeks.de", true },
  { "daveoc64.co.uk", true },
  { "davidlyness.com", true },
  { "deadbeef.ninja", true },
  { "dealcruiser.nl", true },
  { "debtkit.co.uk", true },
  { "dedimax.de", true },
  { "dee.pe", true },
  { "denh.am", true },
  { "depechemode-live.com", true },
  { "derevtsov.com", false },
  { "derhil.de", true },
  { "detectify.com", false },
  { "developer.mydigipass.com", false },
  { "devh.de", true },
  { "diamante.ro", true },
  { "die-besten-weisheiten.de", true },
  { "dillonkorman.com", true },
  { "dinamoelektrik.com", true },
  { "dist.torproject.org", false },
  { "dixmag.com", true },
  { "dl.google.com", true },
  { "dlc.viasinc.com", true },
  { "dm.lookout.com", false },
  { "dm.mylookout.com", false },
  { "doc.python.org", true },
  { "docs.google.com", true },
  { "docs.python.org", true },
  { "domains.google.com", true },
  { "donmez.ws", false },
  { "drive.google.com", true },
  { "dropbox.com", true },
  { "dzlibs.io", true },
  { "easysimplecrm.com", false },
  { "ebanking.indovinabank.com.vn", false },
  { "ecdn.cz", true },
  { "ecosystem.atlassian.net", true },
  { "edit.yahoo.com", false },
  { "eduroam.no", true },
  { "edyou.eu", true },
  { "ef.gy", true },
  { "eff.org", true },
  { "egfl.org.uk", true },
  { "egit.co", true },
  { "eksisozluk.com", true },
  { "electronic-ignition-system.com", true },
  { "elnutricionista.es", true },
  { "emailprivacytester.com", true },
  { "encircleapp.com", true },
  { "encryptallthethings.net", true },
  { "encrypted.google.com", true },
  { "energy-drink-magazin.de", true },
  { "enorekcah.com", true },
  { "entropia.de", false },
  { "errors.zenpayroll.com", false },
  { "espra.com", true },
  { "ethack.org", true },
  { "ethitter.com", true },
  { "eurotramp.com", true },
  { "everhome.de", true },
  { "evstatus.com", true },
  { "exiahost.com", true },
  { "explodie.org", true },
  { "f-droid.org", true },
  { "fabhub.io", true },
  { "fabianfischer.de", true },
  { "factor.cc", false },
  { "fairbill.com", true },
  { "fakturoid.cz", true },
  { "fant.dk", true },
  { "faq.lookout.com", false },
  { "fassadenverkleidung24.de", true },
  { "fastcomcorp.net", true },
  { "fedorapeople.org", true },
  { "feedbin.com", false },
  { "ferienhaus-polchow-ruegen.de", false },
  { "fewo-thueringer-wald.de", true },
  { "fiken.no", true },
  { "filedir.com", false },
  { "filip-prochazka.com", true },
  { "finn.io", true },
  { "firebaseio-demo.com", true },
  { "firebaseio.com", true },
  { "firefart.at", true },
  { "firemail.io", true },
  { "firstlook.org", true },
  { "fischer-its.com", true },
  { "fj.simple.com", false },
  { "flamer-scene.com", true },
  { "fleximus.org", false },
  { "floobits.com", true },
  { "flynn.io", true },
  { "fm83.nl", true },
  { "food4health.guide", true },
  { "forewordreviews.com", true },
  { "forodeespanol.com", true },
  { "forum.linode.com", false },
  { "forum.quantifiedself.com", true },
  { "fralef.me", false },
  { "frederik-braun.com", true },
  { "freenetproject.org", true },
  { "freeshell.de", true },
  { "freethought.org.au", true },
  { "fronteers.nl", true },
  { "fundingempire.com", true },
  { "futos.de", true },
  { "gambitnash.co.uk", true },
  { "gamercredo.com", true },
  { "garron.net", true },
  { "gavick.com", true },
  { "gemeinfreie-lieder.de", true },
  { "gerardozamudio.mx", true },
  { "gernert-server.de", true },
  { "get.zenpayroll.com", false },
  { "getcloak.com", false },
  { "getdigitized.net", true },
  { "getssl.uz", true },
  { "giacomopelagatti.it", true },
  { "github.com", true },
  { "gizzo.sk", true },
  { "glass.google.com", true },
  { "globuli-info.de", true },
  { "glossopnorthendafc.co.uk", true },
  { "gmail.com", false },
  { "gmantra.org", true },
  { "gmcd.co", true },
  { "go.xero.com", false },
  { "gocardless.com", true },
  { "googlemail.com", false },
  { "googleplex.com", true },
  { "goto.google.com", true },
  { "gplintegratedit.com", true },
  { "grc.com", false },
  { "greensolid.biz", true },
  { "grepular.com", true },
  { "groetzner.net", true },
  { "groups.google.com", true },
  { "gtraxapp.com", true },
  { "gunnarhafdal.com", true },
  { "guphi.net", true },
  { "guthabenkarten-billiger.de", true },
  { "gw2treasures.com", true },
  { "hack.li", true },
  { "hackerone.com", true },
  { "hansvaneijsden.com", true },
  { "harvestapp.com", true },
  { "hasilocke.de", true },
  { "haste.ch", true },
  { "haufschild.de", true },
  { "hausverbrauch.de", true },
  { "heartlandrentals.com", true },
  { "heha.co", false },
  { "heid.ws", true },
  { "heijblok.com", true },
  { "helichat.de", true },
  { "help.simpletax.ca", false },
  { "helpium.de", true },
  { "hemlockhillscabinrentals.com", true },
  { "henriknoerr.com", true },
  { "herocentral.de", true },
  { "hex2013.com", true },
  { "hexony.com", true },
  { "hg.python.org", true },
  { "hicoria.com", true },
  { "history.google.com", true },
  { "honeybadger.io", false },
  { "horseboners.xxx", true },
  { "horza.org", true },
  { "hostedtalkgadget.google.com", true },
  { "hostinginnederland.nl", true },
  { "hostix.de", true },
  { "howrandom.org", true },
  { "howsmyssl.com", true },
  { "howsmytls.com", true },
  { "hpac-portal.com", true },
  { "hrackydomino.cz", true },
  { "hsmr.cc", true },
  { "hstsfail.appspot.com", true },
  { "html5.org", true },
  { "i5y.co.uk", true },
  { "iamcarrico.com", true },
  { "ian.sh", true },
  { "iban.is", true },
  { "id-co.in", true },
  { "id-conf.com", true },
  { "id.atlassian.com", true },
  { "id.mayfirst.org", false },
  { "ideaweb.de", true },
  { "ihrlotto.de", true },
  { "ikkatsu-satei.jp", true },
  { "ilikerainbows.co.uk", true },
  { "imaginary.ca", true },
  { "imouto.my", false },
  { "in.xero.com", false },
  { "inb4.us", true },
  { "inbox.google.com", true },
  { "inkbunny.net", true },
  { "inleaked.com", true },
  { "innophate-security.com", true },
  { "innophate-security.nl", true },
  { "insouciant.org", true },
  { "instasex.ch", true },
  { "iranianlawschool.com", true },
  { "iridiumbrowser.de", true },
  { "irische-segenswuensche.info", true },
  { "ironfistdesign.com", true },
  { "isitchristmas.com", true },
  { "it-schwerin.de", true },
  { "itsamurai.ru", true },
  { "itshost.ru", true },
  { "jackyyf.com", false },
  { "jakub-boucek.cz", true },
  { "janoberst.com", true },
  { "janus-engineering.de", true },
  { "jelmer.co.uk", true },
  { "jelmer.uk", true },
  { "jettshome.org", true },
  { "jfreitag.de", true },
  { "jira.com", true },
  { "jitsi.org", false },
  { "jmedved.com", true },
  { "johners.me", true },
  { "jonas-keidel.de", true },
  { "jonaswitmer.ch", true },
  { "jonnybarnes.uk", true },
  { "julian-kipka.de", true },
  { "jwilsson.com", true },
  { "jwilsson.me", true },
  { "k-dev.de", true },
  { "kaheim.de", true },
  { "kardize24.pl", true },
  { "karmaspa.se", true },
  { "kartonmodellbau.org", true },
  { "kdex.de", true },
  { "kdyby.org", true },
  { "keepclean.me", true },
  { "keeperapp.com", true },
  { "keepersecurity.com", true },
  { "kernel-error.de", true },
  { "kevincox.ca", true },
  { "keycdn.com", true },
  { "keyerror.com", true },
  { "khanovaskola.cz", true },
  { "khmath.com", true },
  { "ki-on.net", true },
  { "kinderbuecher-kostenlos.de", true },
  { "kingmanhall.org", true },
  { "kinogb.net", false },
  { "kinsights.com", false },
  { "kirei.se", true },
  { "kitsta.com", true },
  { "klatschreime.de", true },
  { "klausbrinch.dk", true },
  { "klaxn.com", true },
  { "kleidertauschpartys.de", true },
  { "knowledgehook.com", true },
  { "koenvdheuvel.me", true },
  { "komandakovalchuk.com", true },
  { "konklone.com", true },
  { "koop-bremen.de", true },
  { "koordinate.net", true },
  { "kosho.org", true },
  { "kpebetka.net", true },
  { "kraken.io", true },
  { "kuppingercole.com", true },
  { "kupschke.net", true },
  { "kura.io", true },
  { "lagerauftrag.info", true },
  { "lasst-uns-beten.de", true },
  { "lastpass.com", false },
  { "launchkey.com", true },
  { "lavalite.de", true },
  { "lb-toner.de", true },
  { "leadbook.ru", true },
  { "leibniz-remscheid.de", true },
  { "leonardcamacho.me", true },
  { "les-corsaires.net", true },
  { "libraryfreedomproject.org", true },
  { "liebel.org", true },
  { "lighting-centres.co.uk", true },
  { "lilpwny.com", true },
  { "limpid.nl", true },
  { "lingolia.com", true },
  { "linode.com", false },
  { "linux-admin-california.com", true },
  { "linx.net", true },
  { "ljs.io", true },
  { "lobste.rs", true },
  { "lockify.com", true },
  { "lodash.com", true },
  { "loenshotel.de", true },
  { "loftboard.eu", true },
  { "logentries.com", false },
  { "login.corp.google.com", true },
  { "login.launchpad.net", true },
  { "login.persona.org", true },
  { "login.sapo.pt", true },
  { "login.ubuntu.com", true },
  { "login.xero.com", false },
  { "login.yahoo.com", false },
  { "lolicore.ch", true },
  { "lookout.com", false },
  { "ludwig.im", true },
  { "luelistan.net", true },
  { "lukonet.com", true },
  { "lumi.do", false },
  { "luneta.nearbuysystems.com", false },
  { "mach-politik.ch", true },
  { "maff.scot", false },
  { "mail.de", true },
  { "mail.google.com", true },
  { "mail.yahoo.com", false },
  { "mailbox.org", true },
  { "makeitdynamic.com", true },
  { "malnex.de", true },
  { "man3s.jp", true },
  { "manage.zenpayroll.com", false },
  { "manageprojects.com", true },
  { "manager.linode.com", false },
  { "mandala-ausmalbilder.de", true },
  { "manicode.com", true },
  { "market.android.com", true },
  { "markusueberallassetmanagement.de", true },
  { "marshut.net", true },
  { "matatall.com", true },
  { "mathiasbynens.be", true },
  { "matteomarescotti.it", true },
  { "mattmccutchen.net", true },
  { "mbp.banking.co.at", false },
  { "md5file.com", true },
  { "mdfnet.se", true },
  { "meamod.com", true },
  { "mediacru.sh", true },
  { "medium.com", true },
  { "meetfinch.com", true },
  { "mega.co.nz", false },
  { "meinebo.it", true },
  { "members.mayfirst.org", false },
  { "members.nearlyfreespeech.net", false },
  { "miasarafina.de", true },
  { "michalspacek.cz", true },
  { "mig5.net", true },
  { "mike-bland.com", true },
  { "mikewest.org", true },
  { "miku.hatsune.my", true },
  { "minecraftvoter.com", true },
  { "minez-nightswatch.com", true },
  { "minikneet.com", true },
  { "minnesotadata.com", true },
  { "mirrorx.com", true },
  { "miskatonic.org", true },
  { "mkcert.org", true },
  { "mkw.st", true },
  { "mnsure.org", true },
  { "mobile.usaa.com", false },
  { "mondwandler.de", true },
  { "moriz.de", true },
  { "mothereff.in", true },
  { "mountainmusicpromotions.com", true },
  { "mountainroseherbs.com", true },
  { "movlib.org", true },
  { "mpreserver.com", true },
  { "mqas.net", true },
  { "msc-seereisen.net", true },
  { "mths.be", true },
  { "mudcrab.us", true },
  { "mujadin.se", true },
  { "munich-rage.de", true },
  { "musicgamegalaxy.de", true },
  { "mutantmonkey.in", true },
  { "mutantmonkey.info", true },
  { "mutantmonkey.sexy", true },
  { "mvno.io", true },
  { "mwe.st", true },
  { "my.onlime.ch", false },
  { "my.usa.gov", true },
  { "my.xero.com", false },
  { "mygadgetguardian.lookout.com", false },
  { "mylookout.com", false },
  { "myni.io", true },
  { "mynigma.org", true },
  { "myplaceonline.com", true },
  { "myvirtualserver.com", true },
  { "nachsenden.info", true },
  { "nameid.org", true },
  { "namepros.com", true },
  { "nectarleaf.com", true },
  { "neg9.org", false },
  { "neilwynne.com", false },
  { "neko.li", true },
  { "net-safe.info", true },
  { "netera.se", true },
  { "netzbit.de", true },
  { "netztest.at", true },
  { "newstarnootropics.com", true },
  { "ng-security.com", true },
  { "nginxnudes.com", true },
  { "nieselregen.com", true },
  { "nmctest.net", true },
  { "nos-oignons.net", true },
  { "nouvelle-vague-saint-cast.fr", true },
  { "npw.net", true },
  { "nu3.at", true },
  { "nu3.ch", true },
  { "nu3.co.uk", true },
  { "nu3.com", true },
  { "nu3.de", true },
  { "nu3.dk", true },
  { "nu3.fi", true },
  { "nu3.fr", true },
  { "nu3.no", true },
  { "nu3.se", true },
  { "null-sec.ru", true },
  { "nwgh.org", true },
  { "oakslighting.co.uk", true },
  { "okmx.de", true },
  { "omitech.co.uk", true },
  { "onedot.nl", true },
  { "onedrive.com", true },
  { "onedrive.live.com", false },
  { "onsitemassageco.com", true },
  { "ooonja.de", true },
  { "opendesk.cc", true },
  { "oplop.appspot.com", true },
  { "opsmate.com", false },
  { "optimus.io", true },
  { "orbograph-hrcm.com", true },
  { "oscarvk.ch", true },
  { "osterkraenzchen.de", true },
  { "otakuworld.de", true },
  { "ouvirmusica.com.br", true },
  { "ovenapp.io", true },
  { "oversight.io", true },
  { "p.linode.com", false },
  { "packagist.org", false },
  { "pajonzeck.de", true },
  { "palava.tv", true },
  { "parent5446.us", true },
  { "partyvan.eu", true },
  { "partyvan.it", true },
  { "partyvan.nl", true },
  { "partyvan.se", true },
  { "passport.yandex.by", false },
  { "passport.yandex.com", false },
  { "passport.yandex.com.tr", false },
  { "passport.yandex.kz", false },
  { "passport.yandex.ru", false },
  { "passport.yandex.ua", false },
  { "passwd.io", true },
  { "password.codes", true },
  { "passwords.google.com", true },
  { "paste.linode.com", false },
  { "pastebin.linode.com", false },
  { "patt.us", true },
  { "pauladamsmith.com", true },
  { "pay.gigahost.dk", true },
  { "paymill.com", true },
  { "paymill.de", true },
  { "paypal.com", false },
  { "pdf.yt", true },
  { "peercraft.com", true },
  { "pentesterlab.com", true },
  { "personaldatabasen.no", true },
  { "pestici.de", true },
  { "petrolplus.ru", true },
  { "pharmaboard.de", true },
  { "phoenixlogan.com", true },
  { "phurl.de", true },
  { "picksin.club", true },
  { "pieperhome.de", true },
  { "pierre-schmitz.com", true },
  { "pixi.me", true },
  { "play.google.com", true },
  { "plothost.com", true },
  { "plus.google.com", false },
  { "plus.sandbox.google.com", false },
  { "portal.tirol.gv.at", true },
  { "posteo.de", false },
  { "powerplannerapp.com", true },
  { "prakharprasad.com", true },
  { "prefontaine.name", true },
  { "privategiant.com", true },
  { "profiles.google.com", true },
  { "progressiveplanning.com", true },
  { "projektzentrisch.de", true },
  { "propagandism.org", true },
  { "prowhisky.de", true },
  { "proximato.com", true },
  { "puac.de", true },
  { "pubkey.is", true },
  { "publications.qld.gov.au", false },
  { "pult.co", false },
  { "pwd.ovh", true },
  { "pypa.io", true },
  { "pypi.python.org", true },
  { "python.org", false },
  { "qetesh.de", true },
  { "quuz.org", true },
  { "r3s1stanc3.me", true },
  { "rad-route.de", true },
  { "rafaelcz.de", true },
  { "ragingserenity.com", true },
  { "raiseyourflag.com", true },
  { "rasing.me", true },
  { "raspass.me", true },
  { "ravchat.com", true },
  { "redports.org", true },
  { "redteam-pentesting.de", true },
  { "reedloden.com", true },
  { "reishunger.de", true },
  { "reliable-mail.de", true },
  { "reserve-online.net", true },
  { "residentsinsurance.co.uk", true },
  { "reviews.anime.my", true },
  { "riccy.org", true },
  { "riesenmagnete.de", true },
  { "rippleunion.com", true },
  { "rlalique.com", true },
  { "robteix.com", true },
  { "roland.io", true },
  { "romab.com", true },
  { "romans-place.me.uk", true },
  { "romulusapp.com", true },
  { "room-checkin24.de", true },
  { "rosenkeller.org", true },
  { "roundcube.mayfirst.org", false },
  { "ru-sprachstudio.ch", true },
  { "rudloff.pro", true },
  { "ruudkoot.nl", true },
  { "rws-vertriebsportal.de", true },
  { "s-c.se", true },
  { "saintsrobotics.com", true },
  { "sakaki.anime.my", true },
  { "salaervergleich.com", true },
  { "sale4ru.ru", true },
  { "salserocafe.com", true },
  { "samba.org", true },
  { "samizdat.cz", true },
  { "sandbox.mydigipass.com", false },
  { "savetheinternet.eu", true },
  { "savvytime.com", true },
  { "schachburg.de", true },
  { "schokokeks.org", true },
  { "schreiber-netzwerk.eu", true },
  { "schwarzer.it", true },
  { "sciencex.com", true },
  { "scotthelme.co.uk", true },
  { "scrambl.is", true },
  { "scribe.systems", true },
  { "script.google.com", true },
  { "sdsl-speedtest.de", true },
  { "securesuisse.ch", true },
  { "securify.nl", true },
  { "security-carpet.com", true },
  { "security.google.com", true },
  { "securityheaders.com", true },
  { "secuvera.de", true },
  { "seifried.org", true },
  { "servergno.me", true },
  { "servethecity-karlsruhe.de", true },
  { "shaaaaaaaaaaaaa.com", true },
  { "shakepeers.org", true },
  { "shellsec.pw", true },
  { "shenyuqi.com", true },
  { "sherbers.de", true },
  { "shiinko.com", false },
  { "shipard.com", true },
  { "shodan.io", true },
  { "shopontarget.com", true },
  { "shortdiary.me", true },
  { "silentcircle.com", false },
  { "simbolo.co.uk", false },
  { "simple.com", false },
  { "simpletax.ca", false },
  { "simplia.cz", true },
  { "simplystudio.com", true },
  { "siraweb.org", true },
  { "siriad.com", true },
  { "sites.google.com", true },
  { "sitesten.com", true },
  { "skhosting.eu", true },
  { "skogsbruket.fi", true },
  { "skogskultur.fi", true },
  { "skydrive.live.com", false },
  { "slack.com", true },
  { "slattery.co", true },
  { "slevomat.cz", true },
  { "slidebatch.com", true },
  { "smartcleaningcenter.nl", true },
  { "smartcoin.com.br", true },
  { "smartlend.se", true },
  { "smartship.co.jp", true },
  { "snakehosting.dk", true },
  { "sorz.org", true },
  { "sour.is", true },
  { "southside-crew.com", true },
  { "souvik.me", true },
  { "spartantheatre.org", true },
  { "spawn.cz", true },
  { "spencerbaer.com", true },
  { "spideroak.com", true },
  { "spreadsheets.google.com", true },
  { "spreed.me", true },
  { "sprueche-zum-valentinstag.de", true },
  { "sprueche-zur-geburt.info", true },
  { "sprueche-zur-hochzeit.de", true },
  { "sprueche-zur-konfirmation.de", true },
  { "squareup.com", false },
  { "srevilak.net", true },
  { "sro.center", true },
  { "ssl.google-analytics.com", true },
  { "sslmate.com", true },
  { "stage.wepay.com", false },
  { "standardssuck.org", true },
  { "static.wepay.com", false },
  { "stationary-traveller.eu", true },
  { "steventress.com", true },
  { "stocktrade.de", false },
  { "strasweb.fr", false },
  { "stretchmyan.us", true },
  { "stripe.com", true },
  { "strongest-privacy.com", true },
  { "studienportal.eu", true },
  { "studydrive.net", true },
  { "subrosa.io", true },
  { "suite73.org", true },
  { "sunjaydhama.com", true },
  { "supplies24.at", true },
  { "supplies24.es", true },
  { "support.mayfirst.org", false },
  { "surkatty.org", true },
  { "swehack.org", false },
  { "sylaps.com", true },
  { "sysctl.se", true },
  { "syss.de", true },
  { "tadigitalstore.com", true },
  { "tageau.com", true },
  { "talk.google.com", true },
  { "talkgadget.google.com", true },
  { "tapka.cz", true },
  { "tas2580.net", true },
  { "tatort-fanpage.de", true },
  { "tauchkater.de", true },
  { "tbspace.de", true },
  { "techhipster.net", true },
  { "tegelsensanitaironline.nl", true },
  { "tekshrek.com", true },
  { "tent.io", true },
  { "testsuite.org", true },
  { "texte-zur-taufe.de", true },
  { "the-sky-of-valkyries.com", true },
  { "thecustomizewindows.com", true },
  { "thepaymentscompany.com", true },
  { "therapynotes.com", false },
  { "theshadestore.com", true },
  { "thomastimepieces.com.au", true },
  { "thorncreek.net", false },
  { "thusoy.com", true },
  { "thyngster.com", true },
  { "tickopa.co.uk", true },
  { "tid.jp", true },
  { "timtaubert.de", true },
  { "tinfoilsecurity.com", false },
  { "tinte24.de", true },
  { "tintenfix.net", true },
  { "tipps-fuer-den-haushalt.de", true },
  { "tittelbach.at", true },
  { "tls.li", true },
  { "tno.io", true },
  { "tobias-kluge.de", true },
  { "todesschaf.org", true },
  { "tollmanz.com", true },
  { "tomfisher.eu", true },
  { "tomvote.com", true },
  { "toner24.at", true },
  { "toner24.co.uk", true },
  { "toner24.es", true },
  { "toner24.fr", true },
  { "toner24.it", true },
  { "toner24.nl", true },
  { "toner24.pl", true },
  { "tonerdepot.de", true },
  { "tonerjet.at", true },
  { "tonerjet.co.uk", true },
  { "tonerklick.de", true },
  { "tonerkurier.de", true },
  { "tonermaus.de", true },
  { "tonermonster.de", true },
  { "tonex.de", true },
  { "tonex.nl", true },
  { "topodin.com", true },
  { "tor2web.org", true },
  { "torproject.org", false },
  { "toshnix.com", true },
  { "translate.googleapis.com", true },
  { "trauertexte.info", true },
  { "tresorit.com", true },
  { "tribut.de", true },
  { "tucuxi.org", true },
  { "tunebitfm.de", true },
  { "twentymilliseconds.com", true },
  { "twisto.cz", true },
  { "twitter.com", false },
  { "typingrevolution.com", true },
  { "ub3rk1tten.com", true },
  { "ubertt.org", true },
  { "ukdefencejournal.org.uk", true },
  { "ukhas.net", true },
  { "ukrainians.ch", true },
  { "unison.com", true },
  { "unterfrankenclan.de", true },
  { "uonstaffhub.com", true },
  { "uptrends.com", true },
  { "usaa.com", false },
  { "vaddder.com", true },
  { "vhost.co.id", true },
  { "viasinc.com", false },
  { "visionless.me", false },
  { "vmoagents.com", false },
  { "vocaloid.my", true },
  { "vortexhobbies.com", true },
  { "vpnzoom.com", true },
  { "vrobert.fr", false },
  { "w-spotlight.appspot.com", true },
  { "wallet.google.com", true },
  { "warrencreative.com", false },
  { "watsonhall.uk", true },
  { "wbg-vs.de", true },
  { "webandmore.de", false },
  { "webandwords.com.au", true },
  { "webassadors.com", false },
  { "webcollect.org.uk", true },
  { "webfilings-eu-mirror.appspot.com", true },
  { "webfilings-eu.appspot.com", true },
  { "webfilings-mirror-hrd.appspot.com", true },
  { "webfilings.appspot.com", true },
  { "weblogzwolle.nl", true },
  { "webmail.gigahost.dk", false },
  { "webmail.onlime.ch", false },
  { "webmail.schokokeks.org", true },
  { "websenat.de", true },
  { "webtiles.co.uk", true },
  { "webtrh.cz", true },
  { "weggeweest.nl", true },
  { "welches-kinderfahrrad.de", true },
  { "wepay.com", false },
  { "wepay.in.th", true },
  { "wf-bigsky-master.appspot.com", true },
  { "wf-demo-eu.appspot.com", true },
  { "wf-demo-hrd.appspot.com", true },
  { "wf-dogfood-hrd.appspot.com", true },
  { "wf-pentest.appspot.com", true },
  { "wf-staging-hr.appspot.com", true },
  { "wf-training-hrd.appspot.com", true },
  { "wf-training-master.appspot.com", true },
  { "wf-trial-hrd.appspot.com", true },
  { "whatwg.org", true },
  { "whd-guide.de", true },
  { "when-release.ru", true },
  { "whocalld.com", true },
  { "wieninternational.at", true },
  { "wiki.python.org", true },
  { "wildbee.org", true },
  { "willnorris.com", true },
  { "winhistory-forum.net", true },
  { "wootton95.com", true },
  { "wpletter.de", true },
  { "writeapp.me", false },
  { "wubthecaptain.eu", true },
  { "wunderlist.com", true },
  { "wundi.net", true },
  { "www.aclu.org", false },
  { "www.airbnb.com", true },
  { "www.apollo-auto.com", true },
  { "www.banking.co.at", false },
  { "www.braintreepayments.com", false },
  { "www.capitainetrain.com", false },
  { "www.cyveillance.com", true },
  { "www.dropbox.com", true },
  { "www.dropcam.com", false },
  { "www.entropia.de", false },
  { "www.eternalgoth.co.uk", true },
  { "www.etsy.com", true },
  { "www.evernote.com", false },
  { "www.gamesdepartment.co.uk", false },
  { "www.getcloak.com", false },
  { "www.gmail.com", false },
  { "www.googlemail.com", false },
  { "www.gov.uk", false },
  { "www.grc.com", false },
  { "www.heliosnet.com", true },
  { "www.honeybadger.io", false },
  { "www.intercom.io", false },
  { "www.irccloud.com", false },
  { "www.lastpass.com", false },
  { "www.linode.com", false },
  { "www.lookout.com", false },
  { "www.makeyourlaws.org", true },
  { "www.mydigipass.com", false },
  { "www.mylookout.com", false },
  { "www.noisebridge.net", false },
  { "www.opsmate.com", true },
  { "www.paypal.com", false },
  { "www.python.org", true },
  { "www.roddis.net", true },
  { "www.schokokeks.org", true },
  { "www.simbolo.co.uk", false },
  { "www.simple.com", false },
  { "www.therapynotes.com", false },
  { "www.tinfoilsecurity.com", false },
  { "www.torproject.org", false },
  { "www.twitter.com", false },
  { "www.usaa.com", false },
  { "www.viasinc.com", true },
  { "www.wepay.com", false },
  { "www.zenpayroll.com", false },
  { "xbrlsuccess.appspot.com", true },
  { "xn--maraa-rta.org", true },
  { "xps2pdf.co.uk", true },
  { "y-o-w.com", true },
  { "yahvehyireh.com", true },
  { "yetii.net", true },
  { "yoursecondphone.co", true },
  { "ypart.eu", true },
  { "z.ai", true },
  { "zenpayroll.com", false },
  { "zeplin.io", false },
  { "zeropush.com", true },
  { "zixiao.wang", true },
  { "zlavomat.sk", true },
};
