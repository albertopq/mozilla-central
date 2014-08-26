/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// Original author: ekr@rtfm.com

#ifndef mediapipeline_h__
#define mediapipeline_h__

#include "sigslot.h"

#ifdef USE_FAKE_MEDIA_STREAMS
#include "FakeMediaStreams.h"
#else
#include "DOMMediaStream.h"
#include "MediaStreamGraph.h"
#include "VideoUtils.h"
#endif
#include "MediaConduitInterface.h"
#include "MediaPipelineFilter.h"
#include "AudioSegment.h"
#include "mozilla/ReentrantMonitor.h"
#include "mozilla/Atomics.h"
#include "SrtpFlow.h"
#include "databuffer.h"
#include "runnable_utils.h"
#include "transportflow.h"

#ifdef MOZILLA_INTERNAL_API
#include "VideoSegment.h"
#endif

#include "webrtc/modules/rtp_rtcp/interface/rtp_header_parser.h"

namespace mozilla {

class PeerIdentity;

// A class that represents the pipeline of audio and video
// The dataflow looks like:
//
// TRANSMIT
// CaptureDevice -> stream -> [us] -> conduit -> [us] -> transport -> network
//
// RECEIVE
// network -> transport -> [us] -> conduit -> [us] -> stream -> Playout
//
// The boxes labeled [us] are just bridge logic implemented in this class
//
// We have to deal with a number of threads:
//
// GSM:
//   * Assembles the pipeline
// SocketTransportService
//   * Receives notification that ICE and DTLS have completed
//   * Processes incoming network data and passes it to the conduit
//   * Processes outgoing RTP and RTCP
// MediaStreamGraph
//   * Receives outgoing data from the MediaStreamGraph
//   * Receives pull requests for more data from the
//     MediaStreamGraph
// One or another GIPS threads
//   * Receives RTCP messages to send to the other side
//   * Processes video frames GIPS wants to render
//
// For a transmitting conduit, "output" is RTP and "input" is RTCP.
// For a receiving conduit, "input" is RTP and "output" is RTCP.
//

class MediaPipeline;

template<>
struct HasDangerousPublicDestructor<MediaPipeline>
{
  static const bool value = true;
};

class MediaPipeline : public sigslot::has_slots<> {
 public:
  enum Direction { TRANSMIT, RECEIVE };
  enum State { MP_CONNECTING, MP_OPEN, MP_CLOSED };
  MediaPipeline(const std::string& pc,
                Direction direction,
                nsCOMPtr<nsIEventTarget> main_thread,
                nsCOMPtr<nsIEventTarget> sts_thread,
                MediaStream *stream,
                TrackID track_id,
                int level,
                RefPtr<MediaSessionConduit> conduit,
                RefPtr<TransportFlow> rtp_transport,
                RefPtr<TransportFlow> rtcp_transport)
      : direction_(direction),
        stream_(stream),
        track_id_(track_id),
        level_(level),
        conduit_(conduit),
        rtp_(rtp_transport, rtcp_transport ? RTP : MUX),
        rtcp_(rtcp_transport ? rtcp_transport : rtp_transport,
              rtcp_transport ? RTCP : MUX),
        main_thread_(main_thread),
        sts_thread_(sts_thread),
        rtp_packets_sent_(0),
        rtcp_packets_sent_(0),
        rtp_packets_received_(0),
        rtcp_packets_received_(0),
        rtp_bytes_sent_(0),
        rtp_bytes_received_(0),
        pc_(pc),
        description_() {
      // To indicate rtcp-mux rtcp_transport should be nullptr.
      // Therefore it's an error to send in the same flow for
      // both rtp and rtcp.
      MOZ_ASSERT(rtp_transport != rtcp_transport);

      // PipelineTransport() will access this->sts_thread_; moved here for safety
      transport_ = new PipelineTransport(this);
  }

  virtual ~MediaPipeline();

  // Must be called on the STS thread.  Must be called after ShutdownMedia_m().
  void ShutdownTransport_s();

  // Must be called on the main thread.
  void ShutdownMedia_m() {
    ASSERT_ON_THREAD(main_thread_);

    if (stream_) {
      DetachMediaStream();
    }
  }

  virtual nsresult Init();

  // When we have offered bundle, the MediaPipelines are created in an
  // indeterminate state; we do not know whether the answerer will take us
  // up on our offer. In the meantime, we need to behave in a manner that
  // errs on the side of packet loss when it is unclear whether an arriving
  // packet is meant for us. We want to get out of this indeterminate state
  // ASAP, which is what this function can be used for.
  void SetUsingBundle_s(bool decision);
  MediaPipelineFilter* UpdateFilterFromRemoteDescription_s(
      nsAutoPtr<MediaPipelineFilter> filter);

  virtual Direction direction() const { return direction_; }
  virtual TrackID trackid() const { return track_id_; }
  virtual int level() const { return level_; }
  virtual bool IsVideo() const { return false; }

  bool IsDoingRtcpMux() const {
    return (rtp_.type_ == MUX);
  }

  int32_t rtp_packets_sent() const { return rtp_packets_sent_; }
  int64_t rtp_bytes_sent() const { return rtp_bytes_sent_; }
  int32_t rtcp_packets_sent() const { return rtcp_packets_sent_; }
  int32_t rtp_packets_received() const { return rtp_packets_received_; }
  int64_t rtp_bytes_received() const { return rtp_bytes_received_; }
  int32_t rtcp_packets_received() const { return rtcp_packets_received_; }

  MediaSessionConduit *Conduit() const { return conduit_; }

  // Thread counting
  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(MediaPipeline)

  typedef enum {
    RTP,
    RTCP,
    MUX,
    MAX_RTP_TYPE
  } RtpType;

 protected:
  virtual void DetachMediaStream() {}

  // Separate class to allow ref counting
  class PipelineTransport : public TransportInterface {
   public:
    // Implement the TransportInterface functions
    explicit PipelineTransport(MediaPipeline *pipeline)
        : pipeline_(pipeline),
          sts_thread_(pipeline->sts_thread_) {}

    void Detach() { pipeline_ = nullptr; }
    MediaPipeline *pipeline() const { return pipeline_; }

    virtual nsresult SendRtpPacket(const void* data, int len);
    virtual nsresult SendRtcpPacket(const void* data, int len);

   private:
    virtual nsresult SendRtpPacket_s(nsAutoPtr<DataBuffer> data);
    virtual nsresult SendRtcpPacket_s(nsAutoPtr<DataBuffer> data);

    MediaPipeline *pipeline_;  // Raw pointer to avoid cycles
    nsCOMPtr<nsIEventTarget> sts_thread_;
  };
  friend class PipelineTransport;

  class TransportInfo {
    public:
      TransportInfo(RefPtr<TransportFlow> flow, RtpType type) :
        transport_(flow),
        state_(MP_CONNECTING),
        type_(type) {
        MOZ_ASSERT(flow);
      }

      RefPtr<TransportFlow> transport_;
      State state_;
      RefPtr<SrtpFlow> send_srtp_;
      RefPtr<SrtpFlow> recv_srtp_;
      RtpType type_;
  };

  // The transport is down
  virtual nsresult TransportFailed_s(TransportInfo &info);
  // The transport is ready
  virtual nsresult TransportReady_s(TransportInfo &info);
  void UpdateRtcpMuxState(TransportInfo &info);

  // Unhooks from signals
  void DisconnectTransport_s(TransportInfo &info);
  nsresult ConnectTransport_s(TransportInfo &info);

  TransportInfo* GetTransportInfo_s(TransportFlow *flow);

  void increment_rtp_packets_sent(int bytes);
  void increment_rtcp_packets_sent();
  void increment_rtp_packets_received(int bytes);
  void increment_rtcp_packets_received();

  virtual nsresult SendPacket(TransportFlow *flow, const void *data, int len);

  // Process slots on transports
  void StateChange(TransportFlow *flow, TransportLayer::State);
  void RtpPacketReceived(TransportLayer *layer, const unsigned char *data,
                         size_t len);
  void RtcpPacketReceived(TransportLayer *layer, const unsigned char *data,
                          size_t len);
  void PacketReceived(TransportLayer *layer, const unsigned char *data,
                      size_t len);

  Direction direction_;
  RefPtr<MediaStream> stream_;  // A pointer to the stream we are servicing.
                                // Written on the main thread.
                                // Used on STS and MediaStreamGraph threads.
                                // May be changed by rtpSender.replaceTrack()
  TrackID track_id_;            // The track on the stream.
                                // Written and used as with the stream_;
                                // Not used outside initialization in MediaPipelineTransmit
  int level_; // The m-line index (starting at 1, to match convention)
  RefPtr<MediaSessionConduit> conduit_;  // Our conduit. Written on the main
                                         // thread. Read on STS thread.

  // The transport objects. Read/written on STS thread.
  TransportInfo rtp_;
  TransportInfo rtcp_;
  // These are for bundle. We have a separate set because when we have offered
  // bundle, we do not know whether we will receive traffic on the transport
  // in this pipeline's m-line, or the transport in the "master" m-line for
  // the bundle. We need to be ready for either. Once this ambiguity is
  // resolved, the transport we know that we'll be using will be set in
  // rtp_transport_ and rtcp_transport_, and these will be unset.
  // TODO(bcampen@mozilla.com): I'm pretty sure this could be leveraged for
  // re-offer with a new address on an m-line too, with a little work.
  nsAutoPtr<TransportInfo> possible_bundle_rtp_;
  nsAutoPtr<TransportInfo> possible_bundle_rtcp_;

  // Pointers to the threads we need. Initialized at creation
  // and used all over the place.
  nsCOMPtr<nsIEventTarget> main_thread_;
  nsCOMPtr<nsIEventTarget> sts_thread_;

  // Created on Init. Referenced by the conduit and eventually
  // destroyed on the STS thread.
  RefPtr<PipelineTransport> transport_;

  // Only safe to access from STS thread.
  // Build into TransportInfo?
  int32_t rtp_packets_sent_;
  int32_t rtcp_packets_sent_;
  int32_t rtp_packets_received_;
  int32_t rtcp_packets_received_;
  int64_t rtp_bytes_sent_;
  int64_t rtp_bytes_received_;

  // Written on Init. Read on STS thread.
  std::string pc_;
  std::string description_;

  // Written on Init, all following accesses are on the STS thread.
  nsAutoPtr<MediaPipelineFilter> filter_;
  nsAutoPtr<webrtc::RtpHeaderParser> rtp_parser_;

 private:
  nsresult Init_s();

  bool IsRtp(const unsigned char *data, size_t len);
};

class GenericReceiveListener : public MediaStreamListener
{
 public:
  GenericReceiveListener(SourceMediaStream *source, TrackID track_id,
                         TrackRate track_rate)
    : source_(source),
      track_id_(track_id),
      track_rate_(track_rate),
      played_ticks_(0) {}

  virtual ~GenericReceiveListener() {}

  void AddSelf(MediaSegment* segment);

  void SetPlayedTicks(TrackTicks time) {
    played_ticks_ = time;
  }

  void EndTrack() {
    source_->EndTrack(track_id_);
  }

 protected:
  SourceMediaStream *source_;
  TrackID track_id_;
  TrackRate track_rate_;
  TrackTicks played_ticks_;
};

class TrackAddedCallback {
 public:
  virtual void TrackAdded(TrackTicks current_ticks) = 0;

  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(TrackAddedCallback);

 protected:
  virtual ~TrackAddedCallback() {}
};

class GenericReceiveListener;

class GenericReceiveCallback : public TrackAddedCallback
{
 public:
  GenericReceiveCallback(GenericReceiveListener* listener)
    : listener_(listener) {}

  void TrackAdded(TrackTicks time) {
    listener_->SetPlayedTicks(time);
  }

 private:
  RefPtr<GenericReceiveListener> listener_;
};

class ConduitDeleteEvent: public nsRunnable
{
public:
  ConduitDeleteEvent(TemporaryRef<MediaSessionConduit> aConduit) :
    mConduit(aConduit) {}

  /* we exist solely to proxy release of the conduit */
  NS_IMETHOD Run() { return NS_OK; }
private:
  RefPtr<MediaSessionConduit> mConduit;
};

// A specialization of pipeline for reading from an input device
// and transmitting to the network.
class MediaPipelineTransmit : public MediaPipeline {
public:
  // Set rtcp_transport to nullptr to use rtcp-mux
  MediaPipelineTransmit(const std::string& pc,
                        nsCOMPtr<nsIEventTarget> main_thread,
                        nsCOMPtr<nsIEventTarget> sts_thread,
                        DOMMediaStream *domstream,
                        int pipeline_index, // For PeerConnectionMedia/mPipelines
                        int level,
                        bool is_video,
                        RefPtr<MediaSessionConduit> conduit,
                        RefPtr<TransportFlow> rtp_transport,
                        RefPtr<TransportFlow> rtcp_transport) :
      MediaPipeline(pc, TRANSMIT, main_thread, sts_thread,
                    domstream->GetStream(), TRACK_INVALID, level,
                    conduit, rtp_transport, rtcp_transport),
      listener_(new PipelineListener(conduit)),
      domstream_(domstream),
      pipeline_index_(pipeline_index),
      is_video_(is_video)
  {}

  // Initialize (stuff here may fail)
  virtual nsresult Init() MOZ_OVERRIDE;

  virtual void AttachToTrack(TrackID track_id);

  // Index used to refer to this before we know the TrackID
  virtual TrackID pipeline_index() const { return pipeline_index_; }
  // Note: unlike MediaPipeline::trackid(), this is threadsafe
  // Not set until first media is received
  virtual TrackID const trackid_locked() { return listener_->trackid(); }
  // written and used from MainThread
  virtual bool IsVideo() const MOZ_OVERRIDE { return is_video_; }

#ifdef MOZILLA_INTERNAL_API
  // when the principal of the PeerConnection changes, it calls through to here
  // so that we can determine whether to enable stream transmission
  virtual void UpdateSinkIdentity_m(nsIPrincipal* principal,
                                    const PeerIdentity* sinkIdentity);
#endif

  // Called on the main thread.
  virtual void DetachMediaStream() {
    ASSERT_ON_THREAD(main_thread_);
    domstream_->RemoveDirectListener(listener_);
    domstream_ = nullptr;
    stream_->RemoveListener(listener_);
    // Let the listener be destroyed with the pipeline (or later).
    stream_ = nullptr;
  }

  // Override MediaPipeline::TransportReady.
  virtual nsresult TransportReady_s(TransportInfo &info);

  // Replace a track with a different one
  // In non-compliance with the likely final spec, allow the new
  // track to be part of a different stream (since we don't support
  // multiple tracks of a type in a stream yet).  bug 1056650
  virtual nsresult ReplaceTrack(DOMMediaStream *domstream,
                                TrackID track_id);


  // Separate class to allow ref counting
  class PipelineListener : public MediaStreamDirectListener {
   friend class MediaPipelineTransmit;
   public:
    PipelineListener(const RefPtr<MediaSessionConduit>& conduit)
      : conduit_(conduit),
        track_id_(TRACK_INVALID),
        mMutex("MediaPipelineTransmit::PipelineListener"),
        track_id_external_(TRACK_INVALID),
        active_(false),
        enabled_(false),
        direct_connect_(false),
        samples_10ms_buffer_(nullptr),
        buffer_current_(0),
        samplenum_10ms_(0)
#ifdef MOZILLA_INTERNAL_API
        , last_img_(-1)
#endif // MOZILLA_INTERNAL_API
    {
    }

    ~PipelineListener()
    {
      // release conduit on mainthread.  Must use forget()!
      nsresult rv = NS_DispatchToMainThread(new
        ConduitDeleteEvent(conduit_.forget()));
      MOZ_ASSERT(!NS_FAILED(rv),"Could not dispatch conduit shutdown to main");
      if (NS_FAILED(rv)) {
        MOZ_CRASH();
      }
    }

    void SetActive(bool active) { active_ = active; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }
    TrackID trackid() {
      MutexAutoLock lock(mMutex);
      return track_id_external_;
    }

    // Implement MediaStreamListener
    virtual void NotifyQueuedTrackChanges(MediaStreamGraph* graph, TrackID tid,
                                          TrackRate rate,
                                          TrackTicks offset,
                                          uint32_t events,
                                          const MediaSegment& queued_media) MOZ_OVERRIDE;
    virtual void NotifyPull(MediaStreamGraph* aGraph, StreamTime aDesiredTime) MOZ_OVERRIDE {}

    // Implement MediaStreamDirectListener
    virtual void NotifyRealtimeData(MediaStreamGraph* graph, TrackID tid,
                                    TrackRate rate,
                                    TrackTicks offset,
                                    uint32_t events,
                                    const MediaSegment& media) MOZ_OVERRIDE;

   private:
    void NewData(MediaStreamGraph* graph, TrackID tid,
                 TrackRate rate,
                 TrackTicks offset,
                 uint32_t events,
                 const MediaSegment& media);

    virtual void ProcessAudioChunk(AudioSessionConduit *conduit,
                                   TrackRate rate, AudioChunk& chunk);
#ifdef MOZILLA_INTERNAL_API
    virtual void ProcessVideoChunk(VideoSessionConduit *conduit,
                                   TrackRate rate, VideoChunk& chunk);
#endif
    RefPtr<MediaSessionConduit> conduit_;

    // May be TRACK_INVALID until we see data from the track
    TrackID track_id_; // this is the current TrackID this listener is attached to
    Mutex mMutex;
    // protected by mMutex
    // May be TRACK_INVALID until we see data from the track
    TrackID track_id_external_; // this is queried from other threads

    // active is true if there is a transport to send on
    mozilla::Atomic<bool> active_;
    // enabled is true if the media access control permits sending
    // actual content; when false you get black/silence
    mozilla::Atomic<bool> enabled_;

    bool direct_connect_;


    // These vars handle breaking audio samples into exact 10ms chunks:
    // The buffer of 10ms audio samples that we will send once full
    // (can be carried over from one call to another).
    nsAutoArrayPtr<int16_t> samples_10ms_buffer_;
    // The location of the pointer within that buffer (in units of samples).
    int64_t buffer_current_;
    // The number of samples in a 10ms audio chunk.
    int64_t samplenum_10ms_;

#ifdef MOZILLA_INTERNAL_API
    int32_t last_img_; // serial number of last Image
#endif // MOZILLA_INTERNAL_API
  };

 private:
  RefPtr<PipelineListener> listener_;
  DOMMediaStream *domstream_;
  int pipeline_index_; // for lookups in LocalSourceStreamInfo::mPipelines;
  bool is_video_;
};


// A specialization of pipeline for reading from the network and
// rendering video.
class MediaPipelineReceive : public MediaPipeline {
 public:
  // Set rtcp_transport to nullptr to use rtcp-mux
  MediaPipelineReceive(const std::string& pc,
                       nsCOMPtr<nsIEventTarget> main_thread,
                       nsCOMPtr<nsIEventTarget> sts_thread,
                       MediaStream *stream,
                       TrackID track_id,
                       int level,
                       RefPtr<MediaSessionConduit> conduit,
                       RefPtr<TransportFlow> rtp_transport,
                       RefPtr<TransportFlow> rtcp_transport,
                       RefPtr<TransportFlow> bundle_rtp_transport,
                       RefPtr<TransportFlow> bundle_rtcp_transport,
                       nsAutoPtr<MediaPipelineFilter> filter) :
      MediaPipeline(pc, RECEIVE, main_thread, sts_thread,
                    stream, track_id, level, conduit, rtp_transport,
                    rtcp_transport),
      segments_added_(0) {
    filter_ = filter;
    rtp_parser_ = webrtc::RtpHeaderParser::Create();
    if (bundle_rtp_transport) {
      if (bundle_rtcp_transport) {
        MOZ_ASSERT(bundle_rtp_transport != bundle_rtcp_transport);
        possible_bundle_rtp_ = new TransportInfo(bundle_rtp_transport, RTP);
        possible_bundle_rtcp_ = new TransportInfo(bundle_rtcp_transport, RTCP);
      } else {
        possible_bundle_rtp_ = new TransportInfo(bundle_rtp_transport, MUX);
        possible_bundle_rtcp_ = new TransportInfo(bundle_rtp_transport, MUX);
      }
    }
  }

  int segments_added() const { return segments_added_; }

 protected:
  int segments_added_;

 private:
};


// A specialization of pipeline for reading from the network and
// rendering audio.
class MediaPipelineReceiveAudio : public MediaPipelineReceive {
 public:
  MediaPipelineReceiveAudio(const std::string& pc,
                            nsCOMPtr<nsIEventTarget> main_thread,
                            nsCOMPtr<nsIEventTarget> sts_thread,
                            MediaStream *stream,
                            TrackID track_id,
                            int level,
                            RefPtr<AudioSessionConduit> conduit,
                            RefPtr<TransportFlow> rtp_transport,
                            RefPtr<TransportFlow> rtcp_transport,
                            RefPtr<TransportFlow> bundle_rtp_transport,
                            RefPtr<TransportFlow> bundle_rtcp_transport,
                            nsAutoPtr<MediaPipelineFilter> filter) :
      MediaPipelineReceive(pc, main_thread, sts_thread,
                           stream, track_id, level, conduit, rtp_transport,
                           rtcp_transport, bundle_rtp_transport,
                           bundle_rtcp_transport, filter),
      listener_(new PipelineListener(stream->AsSourceStream(),
                                     track_id, conduit)) {
  }

  virtual void DetachMediaStream() {
    ASSERT_ON_THREAD(main_thread_);
    listener_->EndTrack();
    stream_->RemoveListener(listener_);
    stream_ = nullptr;
  }

  virtual nsresult Init() MOZ_OVERRIDE;

 private:
  // Separate class to allow ref counting
  class PipelineListener : public GenericReceiveListener {
   public:
    PipelineListener(SourceMediaStream * source, TrackID track_id,
                     const RefPtr<MediaSessionConduit>& conduit);

    ~PipelineListener()
    {
      // release conduit on mainthread.  Must use forget()!
      nsresult rv = NS_DispatchToMainThread(new
        ConduitDeleteEvent(conduit_.forget()));
      MOZ_ASSERT(!NS_FAILED(rv),"Could not dispatch conduit shutdown to main");
      if (NS_FAILED(rv)) {
        MOZ_CRASH();
      }
    }

    // Implement MediaStreamListener
    virtual void NotifyQueuedTrackChanges(MediaStreamGraph* graph, TrackID tid,
                                          TrackRate rate,
                                          TrackTicks offset,
                                          uint32_t events,
                                          const MediaSegment& queued_media) MOZ_OVERRIDE {}
    virtual void NotifyPull(MediaStreamGraph* graph, StreamTime desired_time) MOZ_OVERRIDE;

   private:
    RefPtr<MediaSessionConduit> conduit_;
  };

  RefPtr<PipelineListener> listener_;
};


// A specialization of pipeline for reading from the network and
// rendering video.
class MediaPipelineReceiveVideo : public MediaPipelineReceive {
 public:
  MediaPipelineReceiveVideo(const std::string& pc,
                            nsCOMPtr<nsIEventTarget> main_thread,
                            nsCOMPtr<nsIEventTarget> sts_thread,
                            MediaStream *stream,
                            TrackID track_id,
                            int level,
                            RefPtr<VideoSessionConduit> conduit,
                            RefPtr<TransportFlow> rtp_transport,
                            RefPtr<TransportFlow> rtcp_transport,
                            RefPtr<TransportFlow> bundle_rtp_transport,
                            RefPtr<TransportFlow> bundle_rtcp_transport,
                            nsAutoPtr<MediaPipelineFilter> filter) :
      MediaPipelineReceive(pc, main_thread, sts_thread,
                           stream, track_id, level, conduit, rtp_transport,
                           rtcp_transport, bundle_rtp_transport,
                           bundle_rtcp_transport, filter),
      renderer_(new PipelineRenderer(MOZ_THIS_IN_INITIALIZER_LIST())),
      listener_(new PipelineListener(stream->AsSourceStream(), track_id)) {
  }

  // Called on the main thread.
  virtual void DetachMediaStream() {
    ASSERT_ON_THREAD(main_thread_);

    listener_->EndTrack();
    // stop generating video and thus stop invoking the PipelineRenderer
    // and PipelineListener - the renderer has a raw ptr to the Pipeline to
    // avoid cycles, and the render callbacks are invoked from a different
    // thread so simple null-checks would cause TSAN bugs without locks.
    static_cast<VideoSessionConduit*>(conduit_.get())->DetachRenderer();
    stream_->RemoveListener(listener_);
    stream_ = nullptr;
  }

  virtual nsresult Init() MOZ_OVERRIDE;

 private:
  class PipelineRenderer : public VideoRenderer {
   public:
    PipelineRenderer(MediaPipelineReceiveVideo *pipeline) :
      pipeline_(pipeline) {}

    void Detach() { pipeline_ = nullptr; }

    // Implement VideoRenderer
    virtual void FrameSizeChange(unsigned int width,
                                 unsigned int height,
                                 unsigned int number_of_streams) {
      pipeline_->listener_->FrameSizeChange(width, height, number_of_streams);
    }

    virtual void RenderVideoFrame(const unsigned char* buffer,
                                  unsigned int buffer_size,
                                  uint32_t time_stamp,
                                  int64_t render_time,
                                  const ImageHandle& handle) {
      pipeline_->listener_->RenderVideoFrame(buffer, buffer_size, time_stamp,
                                             render_time,
                                             handle.GetImage());
    }

   private:
    MediaPipelineReceiveVideo *pipeline_;  // Raw pointer to avoid cycles
  };

  // Separate class to allow ref counting
  class PipelineListener : public GenericReceiveListener {
   public:
    PipelineListener(SourceMediaStream * source, TrackID track_id);

    // Implement MediaStreamListener
    virtual void NotifyQueuedTrackChanges(MediaStreamGraph* graph, TrackID tid,
                                          TrackRate rate,
                                          TrackTicks offset,
                                          uint32_t events,
                                          const MediaSegment& queued_media) MOZ_OVERRIDE {}
    virtual void NotifyPull(MediaStreamGraph* graph, StreamTime desired_time) MOZ_OVERRIDE;

    // Accessors for external writes from the renderer
    void FrameSizeChange(unsigned int width,
                         unsigned int height,
                         unsigned int number_of_streams) {
      ReentrantMonitorAutoEnter enter(monitor_);

      width_ = width;
      height_ = height;
    }

    void RenderVideoFrame(const unsigned char* buffer,
                          unsigned int buffer_size,
                          uint32_t time_stamp,
                          int64_t render_time,
                          const RefPtr<layers::Image>& video_image);

   private:
    int width_;
    int height_;
#ifdef MOZILLA_INTERNAL_API
    nsRefPtr<layers::ImageContainer> image_container_;
    nsRefPtr<layers::Image> image_;
#endif
    mozilla::ReentrantMonitor monitor_; // Monitor for processing WebRTC frames.
                                        // Protects image_ against:
                                        // - Writing from the GIPS thread
                                        // - Reading from the MSG thread
  };

  friend class PipelineRenderer;

  RefPtr<PipelineRenderer> renderer_;
  RefPtr<PipelineListener> listener_;
};


}  // end namespace
#endif
