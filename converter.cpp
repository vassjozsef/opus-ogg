#include <iostream>

#include "connection_log.h"
#include "rtp.h"

extern "C" {
#include "ogg_packer.h"
#include "opus_header.h"
}

constexpr int OpusSsrc = 1387;
constexpr int SampleRate = 48000;
constexpr int Channels = 1;
constexpr int SamplesPerFrame = 960;
constexpr int CommentPad = 512;
constexpr uint8_t SilentPacket[]{248, 255, 254};

class OggWriter: public ConnectionLogVisitor
{
public:
  OggWriter(std::string const& logPath, std::string const& oggPath);
  ~OggWriter();

  bool Start();

private:
  bool WriteIdHeader(int sampleRate, int channels);
  bool WriteCommentHeader();
  bool WriteEndStream();
  bool OutputPages();

  // ConnectionLogVisitor
  void OnSentPacket(uint64_t timestamp, uint8_t const* buf, size_t length) override;
  void OnReceivedPacket(uint64_t timestamp, uint8_t const* buf, size_t length) override;

  std::unique_ptr<ConnectionLog> log_;

  size_t packetsReceived_{0};
  size_t bytesReceived_{0};

  oggpacker* oggp_{nullptr};
  FILE* fpOgg_{nullptr};
  int64_t granulePos_{0};
};

OggWriter::OggWriter(std::string const& logPath, std::string const& oggPath) :
  log_{std::make_unique<ConnectionLog>(logPath)}
{
  fpOgg_ = fopen(oggPath.c_str(), "wb");
}

OggWriter::~OggWriter()
{
  oggp_destroy(oggp_);
  fclose(fpOgg_);
}

bool OggWriter::OutputPages() {
  unsigned char* page;
  int len;
  while (oggp_get_next_page(oggp_, &page, &len)) {
    std::cout << "oggp next page size: " << len << std::endl;
    auto written = fwrite(page, 1, len, fpOgg_);
    if (written != len) {
      std::cout << "Failed to write output file" << std::endl;
      return false;
    }
  }
  return true;
}

bool OggWriter::WriteIdHeader(int sampleRate, int channels) 
{
  OpusHeader header;
  header.channels = channels;
  header.preskip = 0;
  header.input_sample_rate = sampleRate;
  header.gain = 0;
  header.channel_mapping = 0;

   auto serial = rand();
   oggp_ = oggp_create(serial);
   if (!oggp_) {
    std::cout << "Failed to create ogg container" << std::endl;
    return false;
   }
   oggp_set_muxing_delay(oggp_, 48000);

   int headerSize = opeint_opus_header_get_size(&header);
   unsigned char* p = oggp_get_packet_buffer(oggp_, headerSize);
   int packetSize = opeint_opus_header_to_packet(&header, p, headerSize);
   oggp_commit_packet(oggp_, packetSize, 0, 0);
   oggp_flush_page(oggp_);
   return OutputPages();
}

bool OggWriter::WriteCommentHeader()
{
  char* comment;
  int length;
  auto vendorString = "Opus provided by WebRTC b2228fe20";
  opeint_comment_init(&comment, &length, vendorString);
  opeint_comment_add(&comment, &length, "ENCODER", "Discord Client");
  opeint_comment_pad(&comment, &length, CommentPad);
  unsigned char* p = oggp_get_packet_buffer(oggp_, length);
  memcpy(p, comment, length);
  oggp_commit_packet(oggp_, length, 0, 0);
  oggp_flush_page(oggp_);
  return OutputPages();
}

bool OggWriter::WriteEndStream()
{
  OutputPages();
  auto payloadSize = sizeof(SilentPacket);
  unsigned char* p = oggp_get_packet_buffer(oggp_, payloadSize);
  memcpy(p, SilentPacket, payloadSize);
  granulePos_ += SamplesPerFrame;
  oggp_commit_packet(oggp_, payloadSize, granulePos_, 1);
  return OutputPages();
}

bool OggWriter::Start() {
  if (!log_->IsOK()) {
    return false;
  }

  if (!fpOgg_) {
    return false;
  }

  if (!WriteIdHeader(SampleRate, Channels)) {
    std::cout << "Failed to write id header" << std::endl;
    return false;
  }

  if (!WriteCommentHeader()) {
    std::cout << "Failed to write comment header" << std::endl;
    return false;
  }

  bool eof{false};
  while (!eof) {
    eof = !log_->ReadNext(this);
  }

  std::cout << "Packets received: " << packetsReceived_ << std::endl;
  std::cout << "Bytes received: " << bytesReceived_ << std::endl;

  if (!WriteEndStream()) {
    std::cout << "Faield to write end of stream" << std::endl;
  }

  return true;
}

void OggWriter::OnSentPacket(uint64_t timestamp, uint8_t const* buf, size_t length) {
  std::cout << "Packet sent of size: " << length << std::endl;
}

void OggWriter::OnReceivedPacket(uint64_t timestamp, uint8_t const* buf, size_t length) {
  packetsReceived_++;
  bytesReceived_ += length;

  Rtp rtp(buf, length);
  if (!rtp.Parse()) {
    std::cout << "Failed to parse RTP packet" << std::endl;
  }

  std::cout << "RTP received, ssrc: " << (long)rtp.getSsrc() << ", pt: " << (int)rtp.getPayloadType()
      << ", ext: " << rtp.getExt() << "(" << rtp.getExtensionHeaderLength() <<  "), cc: "
      << (int)rtp.getCc() << ", sn: " << rtp.getSequenceNumber() << ", ts: " << rtp.getTimestamp()
      << ", payload size: " << (int)rtp.getPayloadSize() << ", size: " << length << std::endl;
  
  if (rtp.getSsrc() != OpusSsrc) {
    return;
  }

  auto payloadSize = rtp.getPayloadSize();
  unsigned char* p = oggp_get_packet_buffer(oggp_, payloadSize);
  memcpy(p, rtp.getPayload(), payloadSize);
  granulePos_ += SamplesPerFrame;
  oggp_commit_packet(oggp_, payloadSize, granulePos_, 0);
  if (granulePos_ >= 48000) {
    OutputPages();
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " input_file output_file" << std::endl;
  }

  OggWriter reader(argv[1], argv[2]);
  reader.Start();
}
