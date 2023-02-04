#include "ogg_writer.h"

extern "C" {
#include "opus_header.h"
}

constexpr int SamplesPerFrame = 960;
constexpr int CommentPad = 512;
constexpr uint8_t SilentPacket[]{248, 255, 254};

OggWriter::OggWriter(std::string const& oggPath)
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
    auto written = fwrite(page, 1, len, fpOgg_);
    if (written != len) {
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

bool OggWriter::WritePacket(const uint8_t* packet, size_t length) {
  unsigned char* p = oggp_get_packet_buffer(oggp_, length);
  memcpy(p, packet, length);
  granulePos_ += SamplesPerFrame;
  oggp_commit_packet(oggp_, length, granulePos_, 0);
  if (granulePos_ >= 48000) {
    OutputPages();
  }
}
