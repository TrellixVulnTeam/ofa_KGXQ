/*
 *  Copyright 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <memory>
#include <string>
#include <utility>

#include "webrtc/api/audiotrack.h"
#include "webrtc/api/fakemediacontroller.h"
#include "webrtc/api/localaudiosource.h"
#include "webrtc/api/mediastream.h"
#include "webrtc/api/remoteaudiosource.h"
#include "webrtc/api/rtpreceiver.h"
#include "webrtc/api/rtpsender.h"
#include "webrtc/api/streamcollection.h"
#include "webrtc/api/test/fakevideotracksource.h"
#include "webrtc/api/videotrack.h"
#include "webrtc/api/videotracksource.h"
#include "webrtc/base/gunit.h"
#include "webrtc/media/base/fakemediaengine.h"
#include "webrtc/media/base/mediachannel.h"
#include "webrtc/media/engine/fakewebrtccall.h"
#include "webrtc/p2p/base/faketransportcontroller.h"
#include "webrtc/pc/channelmanager.h"
#include "webrtc/test/gmock.h"
#include "webrtc/test/gtest.h"

using ::testing::_;
using ::testing::Exactly;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

static const char kStreamLabel1[] = "local_stream_1";
static const char kVideoTrackId[] = "video_1";
static const char kAudioTrackId[] = "audio_1";
static const uint32_t kVideoSsrc = 98;
static const uint32_t kVideoSsrc2 = 100;
static const uint32_t kAudioSsrc = 99;
static const uint32_t kAudioSsrc2 = 101;

namespace webrtc {

class RtpSenderReceiverTest : public testing::Test {
 public:
  RtpSenderReceiverTest()
      :  // Create fake media engine/etc. so we can create channels to use to
        // test RtpSenders/RtpReceivers.
        media_engine_(new cricket::FakeMediaEngine()),
        channel_manager_(media_engine_,
                         rtc::Thread::Current(),
                         rtc::Thread::Current()),
        fake_call_(webrtc::Call::Config()),
        fake_media_controller_(&channel_manager_, &fake_call_),
        stream_(MediaStream::Create(kStreamLabel1)) {
    // Create channels to be used by the RtpSenders and RtpReceivers.
    channel_manager_.Init();
    voice_channel_ = channel_manager_.CreateVoiceChannel(
        &fake_media_controller_, &fake_transport_controller_, cricket::CN_AUDIO,
        nullptr, false, cricket::AudioOptions());
    video_channel_ = channel_manager_.CreateVideoChannel(
        &fake_media_controller_, &fake_transport_controller_, cricket::CN_VIDEO,
        nullptr, false, cricket::VideoOptions());
    voice_media_channel_ = media_engine_->GetVoiceChannel(0);
    video_media_channel_ = media_engine_->GetVideoChannel(0);
    RTC_CHECK(voice_channel_);
    RTC_CHECK(video_channel_);
    RTC_CHECK(voice_media_channel_);
    RTC_CHECK(video_media_channel_);

    // Create streams for predefined SSRCs. Streams need to exist in order
    // for the senders and receievers to apply parameters to them.
    // Normally these would be created by SetLocalDescription and
    // SetRemoteDescription.
    voice_media_channel_->AddSendStream(
        cricket::StreamParams::CreateLegacy(kAudioSsrc));
    voice_media_channel_->AddRecvStream(
        cricket::StreamParams::CreateLegacy(kAudioSsrc));
    voice_media_channel_->AddSendStream(
        cricket::StreamParams::CreateLegacy(kAudioSsrc2));
    voice_media_channel_->AddRecvStream(
        cricket::StreamParams::CreateLegacy(kAudioSsrc2));
    video_media_channel_->AddSendStream(
        cricket::StreamParams::CreateLegacy(kVideoSsrc));
    video_media_channel_->AddRecvStream(
        cricket::StreamParams::CreateLegacy(kVideoSsrc));
    video_media_channel_->AddSendStream(
        cricket::StreamParams::CreateLegacy(kVideoSsrc2));
    video_media_channel_->AddRecvStream(
        cricket::StreamParams::CreateLegacy(kVideoSsrc2));
  }

  void TearDown() override { channel_manager_.Terminate(); }

  void AddVideoTrack() {
    rtc::scoped_refptr<VideoTrackSourceInterface> source(
        FakeVideoTrackSource::Create());
    video_track_ = VideoTrack::Create(kVideoTrackId, source);
    EXPECT_TRUE(stream_->AddTrack(video_track_));
  }

  void CreateAudioRtpSender() { CreateAudioRtpSender(nullptr); }

  void CreateAudioRtpSender(rtc::scoped_refptr<LocalAudioSource> source) {
    audio_track_ = AudioTrack::Create(kAudioTrackId, source);
    EXPECT_TRUE(stream_->AddTrack(audio_track_));
    audio_rtp_sender_ =
        new AudioRtpSender(stream_->GetAudioTracks()[0], stream_->label(),
                           voice_channel_, nullptr);
    audio_rtp_sender_->SetSsrc(kAudioSsrc);
    VerifyVoiceChannelInput();
  }

  void CreateVideoRtpSender() {
    AddVideoTrack();
    video_rtp_sender_ = new VideoRtpSender(stream_->GetVideoTracks()[0],
                                           stream_->label(), video_channel_);
    video_rtp_sender_->SetSsrc(kVideoSsrc);
    VerifyVideoChannelInput();
  }

  void DestroyAudioRtpSender() {
    audio_rtp_sender_ = nullptr;
    VerifyVoiceChannelNoInput();
  }

  void DestroyVideoRtpSender() {
    video_rtp_sender_ = nullptr;
    VerifyVideoChannelNoInput();
  }

  void CreateAudioRtpReceiver() {
    audio_track_ = AudioTrack::Create(
        kAudioTrackId, RemoteAudioSource::Create(kAudioSsrc, NULL));
    EXPECT_TRUE(stream_->AddTrack(audio_track_));
    audio_rtp_receiver_ = new AudioRtpReceiver(stream_, kAudioTrackId,
                                               kAudioSsrc, voice_channel_);
    audio_track_ = audio_rtp_receiver_->audio_track();
    VerifyVoiceChannelOutput();
  }

  void CreateVideoRtpReceiver() {
    video_rtp_receiver_ =
        new VideoRtpReceiver(stream_, kVideoTrackId, rtc::Thread::Current(),
                             kVideoSsrc, video_channel_);
    video_track_ = video_rtp_receiver_->video_track();
    VerifyVideoChannelOutput();
  }

  void DestroyAudioRtpReceiver() {
    audio_rtp_receiver_ = nullptr;
    VerifyVoiceChannelNoOutput();
  }

  void DestroyVideoRtpReceiver() {
    video_rtp_receiver_ = nullptr;
    VerifyVideoChannelNoOutput();
  }

  void VerifyVoiceChannelInput() { VerifyVoiceChannelInput(kAudioSsrc); }

  void VerifyVoiceChannelInput(uint32_t ssrc) {
    // Verify that the media channel has an audio source, and the stream isn't
    // muted.
    EXPECT_TRUE(voice_media_channel_->HasSource(ssrc));
    EXPECT_FALSE(voice_media_channel_->IsStreamMuted(ssrc));
  }

  void VerifyVideoChannelInput() { VerifyVideoChannelInput(kVideoSsrc); }

  void VerifyVideoChannelInput(uint32_t ssrc) {
    // Verify that the media channel has a video source,
    EXPECT_TRUE(video_media_channel_->HasSource(ssrc));
  }

  void VerifyVoiceChannelNoInput() { VerifyVoiceChannelNoInput(kAudioSsrc); }

  void VerifyVoiceChannelNoInput(uint32_t ssrc) {
    // Verify that the media channel's source is reset.
    EXPECT_FALSE(voice_media_channel_->HasSource(ssrc));
  }

  void VerifyVideoChannelNoInput() { VerifyVideoChannelNoInput(kVideoSsrc); }

  void VerifyVideoChannelNoInput(uint32_t ssrc) {
    // Verify that the media channel's source is reset.
    EXPECT_FALSE(video_media_channel_->HasSource(ssrc));
  }

  void VerifyVoiceChannelOutput() {
    // Verify that the volume is initialized to 1.
    double volume;
    EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
    EXPECT_EQ(1, volume);
  }

  void VerifyVideoChannelOutput() {
    // Verify that the media channel has a sink.
    EXPECT_TRUE(video_media_channel_->HasSink(kVideoSsrc));
  }

  void VerifyVoiceChannelNoOutput() {
    // Verify that the volume is reset to 0.
    double volume;
    EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
    EXPECT_EQ(0, volume);
  }

  void VerifyVideoChannelNoOutput() {
    // Verify that the media channel's sink is reset.
    EXPECT_FALSE(video_media_channel_->HasSink(kVideoSsrc));
  }

 protected:
  cricket::FakeMediaEngine* media_engine_;
  cricket::FakeTransportController fake_transport_controller_;
  cricket::ChannelManager channel_manager_;
  cricket::FakeCall fake_call_;
  cricket::FakeMediaController fake_media_controller_;
  cricket::VoiceChannel* voice_channel_;
  cricket::VideoChannel* video_channel_;
  cricket::FakeVoiceMediaChannel* voice_media_channel_;
  cricket::FakeVideoMediaChannel* video_media_channel_;
  rtc::scoped_refptr<AudioRtpSender> audio_rtp_sender_;
  rtc::scoped_refptr<VideoRtpSender> video_rtp_sender_;
  rtc::scoped_refptr<AudioRtpReceiver> audio_rtp_receiver_;
  rtc::scoped_refptr<VideoRtpReceiver> video_rtp_receiver_;
  rtc::scoped_refptr<MediaStreamInterface> stream_;
  rtc::scoped_refptr<VideoTrackInterface> video_track_;
  rtc::scoped_refptr<AudioTrackInterface> audio_track_;
};

// Test that |voice_channel_| is updated when an audio track is associated
// and disassociated with an AudioRtpSender.
TEST_F(RtpSenderReceiverTest, AddAndDestroyAudioRtpSender) {
  CreateAudioRtpSender();
  DestroyAudioRtpSender();
}

// Test that |video_channel_| is updated when a video track is associated and
// disassociated with a VideoRtpSender.
TEST_F(RtpSenderReceiverTest, AddAndDestroyVideoRtpSender) {
  CreateVideoRtpSender();
  DestroyVideoRtpSender();
}

// Test that |voice_channel_| is updated when a remote audio track is
// associated and disassociated with an AudioRtpReceiver.
TEST_F(RtpSenderReceiverTest, AddAndDestroyAudioRtpReceiver) {
  CreateAudioRtpReceiver();
  DestroyAudioRtpReceiver();
}

// Test that |video_channel_| is updated when a remote video track is
// associated and disassociated with a VideoRtpReceiver.
TEST_F(RtpSenderReceiverTest, AddAndDestroyVideoRtpReceiver) {
  CreateVideoRtpReceiver();
  DestroyVideoRtpReceiver();
}

// Test that the AudioRtpSender applies options from the local audio source.
TEST_F(RtpSenderReceiverTest, LocalAudioSourceOptionsApplied) {
  cricket::AudioOptions options;
  options.echo_cancellation = rtc::Optional<bool>(true);
  auto source = LocalAudioSource::Create(
      PeerConnectionFactoryInterface::Options(), &options);
  CreateAudioRtpSender(source.get());

  EXPECT_EQ(rtc::Optional<bool>(true),
            voice_media_channel_->options().echo_cancellation);

  DestroyAudioRtpSender();
}

// Test that the stream is muted when the track is disabled, and unmuted when
// the track is enabled.
TEST_F(RtpSenderReceiverTest, LocalAudioTrackDisable) {
  CreateAudioRtpSender();

  audio_track_->set_enabled(false);
  EXPECT_TRUE(voice_media_channel_->IsStreamMuted(kAudioSsrc));

  audio_track_->set_enabled(true);
  EXPECT_FALSE(voice_media_channel_->IsStreamMuted(kAudioSsrc));

  DestroyAudioRtpSender();
}

// Test that the volume is set to 0 when the track is disabled, and back to
// 1 when the track is enabled.
TEST_F(RtpSenderReceiverTest, RemoteAudioTrackDisable) {
  CreateAudioRtpReceiver();

  double volume;
  EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
  EXPECT_EQ(1, volume);

  audio_track_->set_enabled(false);
  EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
  EXPECT_EQ(0, volume);

  audio_track_->set_enabled(true);
  EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
  EXPECT_EQ(1, volume);

  DestroyAudioRtpReceiver();
}

// Currently no action is taken when a remote video track is disabled or
// enabled, so there's nothing to test here, other than what is normally
// verified in DestroyVideoRtpSender.
TEST_F(RtpSenderReceiverTest, LocalVideoTrackDisable) {
  CreateVideoRtpSender();

  video_track_->set_enabled(false);
  video_track_->set_enabled(true);

  DestroyVideoRtpSender();
}

// Test that the state of the video track created by the VideoRtpReceiver is
// updated when the receiver is destroyed.
TEST_F(RtpSenderReceiverTest, RemoteVideoTrackState) {
  CreateVideoRtpReceiver();

  EXPECT_EQ(webrtc::MediaStreamTrackInterface::kLive, video_track_->state());
  EXPECT_EQ(webrtc::MediaSourceInterface::kLive,
            video_track_->GetSource()->state());

  DestroyVideoRtpReceiver();

  EXPECT_EQ(webrtc::MediaStreamTrackInterface::kEnded, video_track_->state());
  EXPECT_EQ(webrtc::MediaSourceInterface::kEnded,
            video_track_->GetSource()->state());
}

// Currently no action is taken when a remote video track is disabled or
// enabled, so there's nothing to test here, other than what is normally
// verified in DestroyVideoRtpReceiver.
TEST_F(RtpSenderReceiverTest, RemoteVideoTrackDisable) {
  CreateVideoRtpReceiver();

  video_track_->set_enabled(false);
  video_track_->set_enabled(true);

  DestroyVideoRtpReceiver();
}

// Test that the AudioRtpReceiver applies volume changes from the track source
// to the media channel.
TEST_F(RtpSenderReceiverTest, RemoteAudioTrackSetVolume) {
  CreateAudioRtpReceiver();

  double volume;
  audio_track_->GetSource()->SetVolume(0.5);
  EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
  EXPECT_EQ(0.5, volume);

  // Disable the audio track, this should prevent setting the volume.
  audio_track_->set_enabled(false);
  audio_track_->GetSource()->SetVolume(0.8);
  EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
  EXPECT_EQ(0, volume);

  // When the track is enabled, the previously set volume should take effect.
  audio_track_->set_enabled(true);
  EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
  EXPECT_EQ(0.8, volume);

  // Try changing volume one more time.
  audio_track_->GetSource()->SetVolume(0.9);
  EXPECT_TRUE(voice_media_channel_->GetOutputVolume(kAudioSsrc, &volume));
  EXPECT_EQ(0.9, volume);

  DestroyAudioRtpReceiver();
}

// Test that the media channel isn't enabled for sending if the audio sender
// doesn't have both a track and SSRC.
TEST_F(RtpSenderReceiverTest, AudioSenderWithoutTrackAndSsrc) {
  audio_rtp_sender_ = new AudioRtpSender(voice_channel_, nullptr);
  rtc::scoped_refptr<AudioTrackInterface> track =
      AudioTrack::Create(kAudioTrackId, nullptr);

  // Track but no SSRC.
  EXPECT_TRUE(audio_rtp_sender_->SetTrack(track));
  VerifyVoiceChannelNoInput();

  // SSRC but no track.
  EXPECT_TRUE(audio_rtp_sender_->SetTrack(nullptr));
  audio_rtp_sender_->SetSsrc(kAudioSsrc);
  VerifyVoiceChannelNoInput();
}

// Test that the media channel isn't enabled for sending if the video sender
// doesn't have both a track and SSRC.
TEST_F(RtpSenderReceiverTest, VideoSenderWithoutTrackAndSsrc) {
  video_rtp_sender_ = new VideoRtpSender(video_channel_);

  // Track but no SSRC.
  EXPECT_TRUE(video_rtp_sender_->SetTrack(video_track_));
  VerifyVideoChannelNoInput();

  // SSRC but no track.
  EXPECT_TRUE(video_rtp_sender_->SetTrack(nullptr));
  video_rtp_sender_->SetSsrc(kVideoSsrc);
  VerifyVideoChannelNoInput();
}

// Test that the media channel is enabled for sending when the audio sender
// has a track and SSRC, when the SSRC is set first.
TEST_F(RtpSenderReceiverTest, AudioSenderEarlyWarmupSsrcThenTrack) {
  audio_rtp_sender_ = new AudioRtpSender(voice_channel_, nullptr);
  rtc::scoped_refptr<AudioTrackInterface> track =
      AudioTrack::Create(kAudioTrackId, nullptr);
  audio_rtp_sender_->SetSsrc(kAudioSsrc);
  audio_rtp_sender_->SetTrack(track);
  VerifyVoiceChannelInput();

  DestroyAudioRtpSender();
}

// Test that the media channel is enabled for sending when the audio sender
// has a track and SSRC, when the SSRC is set last.
TEST_F(RtpSenderReceiverTest, AudioSenderEarlyWarmupTrackThenSsrc) {
  audio_rtp_sender_ = new AudioRtpSender(voice_channel_, nullptr);
  rtc::scoped_refptr<AudioTrackInterface> track =
      AudioTrack::Create(kAudioTrackId, nullptr);
  audio_rtp_sender_->SetTrack(track);
  audio_rtp_sender_->SetSsrc(kAudioSsrc);
  VerifyVoiceChannelInput();

  DestroyAudioRtpSender();
}

// Test that the media channel is enabled for sending when the video sender
// has a track and SSRC, when the SSRC is set first.
TEST_F(RtpSenderReceiverTest, VideoSenderEarlyWarmupSsrcThenTrack) {
  AddVideoTrack();
  video_rtp_sender_ = new VideoRtpSender(video_channel_);
  video_rtp_sender_->SetSsrc(kVideoSsrc);
  video_rtp_sender_->SetTrack(video_track_);
  VerifyVideoChannelInput();

  DestroyVideoRtpSender();
}

// Test that the media channel is enabled for sending when the video sender
// has a track and SSRC, when the SSRC is set last.
TEST_F(RtpSenderReceiverTest, VideoSenderEarlyWarmupTrackThenSsrc) {
  AddVideoTrack();
  video_rtp_sender_ = new VideoRtpSender(video_channel_);
  video_rtp_sender_->SetTrack(video_track_);
  video_rtp_sender_->SetSsrc(kVideoSsrc);
  VerifyVideoChannelInput();

  DestroyVideoRtpSender();
}

// Test that the media channel stops sending when the audio sender's SSRC is set
// to 0.
TEST_F(RtpSenderReceiverTest, AudioSenderSsrcSetToZero) {
  CreateAudioRtpSender();

  audio_rtp_sender_->SetSsrc(0);
  VerifyVoiceChannelNoInput();
}

// Test that the media channel stops sending when the video sender's SSRC is set
// to 0.
TEST_F(RtpSenderReceiverTest, VideoSenderSsrcSetToZero) {
  CreateAudioRtpSender();

  audio_rtp_sender_->SetSsrc(0);
  VerifyVideoChannelNoInput();
}

// Test that the media channel stops sending when the audio sender's track is
// set to null.
TEST_F(RtpSenderReceiverTest, AudioSenderTrackSetToNull) {
  CreateAudioRtpSender();

  EXPECT_TRUE(audio_rtp_sender_->SetTrack(nullptr));
  VerifyVoiceChannelNoInput();
}

// Test that the media channel stops sending when the video sender's track is
// set to null.
TEST_F(RtpSenderReceiverTest, VideoSenderTrackSetToNull) {
  CreateVideoRtpSender();

  video_rtp_sender_->SetSsrc(0);
  VerifyVideoChannelNoInput();
}

// Test that when the audio sender's SSRC is changed, the media channel stops
// sending with the old SSRC and starts sending with the new one.
TEST_F(RtpSenderReceiverTest, AudioSenderSsrcChanged) {
  CreateAudioRtpSender();

  audio_rtp_sender_->SetSsrc(kAudioSsrc2);
  VerifyVoiceChannelNoInput(kAudioSsrc);
  VerifyVoiceChannelInput(kAudioSsrc2);

  audio_rtp_sender_ = nullptr;
  VerifyVoiceChannelNoInput(kAudioSsrc2);
}

// Test that when the audio sender's SSRC is changed, the media channel stops
// sending with the old SSRC and starts sending with the new one.
TEST_F(RtpSenderReceiverTest, VideoSenderSsrcChanged) {
  CreateVideoRtpSender();

  video_rtp_sender_->SetSsrc(kVideoSsrc2);
  VerifyVideoChannelNoInput(kVideoSsrc);
  VerifyVideoChannelInput(kVideoSsrc2);

  video_rtp_sender_ = nullptr;
  VerifyVideoChannelNoInput(kVideoSsrc2);
}

TEST_F(RtpSenderReceiverTest, AudioSenderCanSetParameters) {
  CreateAudioRtpSender();

  RtpParameters params = audio_rtp_sender_->GetParameters();
  EXPECT_EQ(1u, params.encodings.size());
  EXPECT_TRUE(audio_rtp_sender_->SetParameters(params));

  DestroyAudioRtpSender();
}

TEST_F(RtpSenderReceiverTest, SetAudioMaxSendBitrate) {
  CreateAudioRtpSender();

  EXPECT_EQ(-1, voice_media_channel_->max_bps());
  webrtc::RtpParameters params = audio_rtp_sender_->GetParameters();
  EXPECT_EQ(1, params.encodings.size());
  EXPECT_EQ(-1, params.encodings[0].max_bitrate_bps);
  params.encodings[0].max_bitrate_bps = 1000;
  EXPECT_TRUE(audio_rtp_sender_->SetParameters(params));

  // Read back the parameters and verify they have been changed.
  params = audio_rtp_sender_->GetParameters();
  EXPECT_EQ(1, params.encodings.size());
  EXPECT_EQ(1000, params.encodings[0].max_bitrate_bps);

  // Verify that the audio channel received the new parameters.
  params = voice_media_channel_->GetRtpSendParameters(kAudioSsrc);
  EXPECT_EQ(1, params.encodings.size());
  EXPECT_EQ(1000, params.encodings[0].max_bitrate_bps);

  // Verify that the global bitrate limit has not been changed.
  EXPECT_EQ(-1, voice_media_channel_->max_bps());

  DestroyAudioRtpSender();
}

TEST_F(RtpSenderReceiverTest, VideoSenderCanSetParameters) {
  CreateVideoRtpSender();

  RtpParameters params = video_rtp_sender_->GetParameters();
  EXPECT_EQ(1u, params.encodings.size());
  EXPECT_TRUE(video_rtp_sender_->SetParameters(params));

  DestroyVideoRtpSender();
}

TEST_F(RtpSenderReceiverTest, SetVideoMaxSendBitrate) {
  CreateVideoRtpSender();

  EXPECT_EQ(-1, video_media_channel_->max_bps());
  webrtc::RtpParameters params = video_rtp_sender_->GetParameters();
  EXPECT_EQ(1, params.encodings.size());
  EXPECT_EQ(-1, params.encodings[0].max_bitrate_bps);
  params.encodings[0].max_bitrate_bps = 1000;
  EXPECT_TRUE(video_rtp_sender_->SetParameters(params));

  // Read back the parameters and verify they have been changed.
  params = video_rtp_sender_->GetParameters();
  EXPECT_EQ(1, params.encodings.size());
  EXPECT_EQ(1000, params.encodings[0].max_bitrate_bps);

  // Verify that the video channel received the new parameters.
  params = video_media_channel_->GetRtpSendParameters(kVideoSsrc);
  EXPECT_EQ(1, params.encodings.size());
  EXPECT_EQ(1000, params.encodings[0].max_bitrate_bps);

  // Verify that the global bitrate limit has not been changed.
  EXPECT_EQ(-1, video_media_channel_->max_bps());

  DestroyVideoRtpSender();
}

TEST_F(RtpSenderReceiverTest, AudioReceiverCanSetParameters) {
  CreateAudioRtpReceiver();

  RtpParameters params = audio_rtp_receiver_->GetParameters();
  EXPECT_EQ(1u, params.encodings.size());
  EXPECT_TRUE(audio_rtp_receiver_->SetParameters(params));

  DestroyAudioRtpReceiver();
}

TEST_F(RtpSenderReceiverTest, VideoReceiverCanSetParameters) {
  CreateVideoRtpReceiver();

  RtpParameters params = video_rtp_receiver_->GetParameters();
  EXPECT_EQ(1u, params.encodings.size());
  EXPECT_TRUE(video_rtp_receiver_->SetParameters(params));

  DestroyVideoRtpReceiver();
}

}  // namespace webrtc
