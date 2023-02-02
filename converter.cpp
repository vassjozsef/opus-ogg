#include <iostream>

#include "connection_log.h"
#include "rtp.h"

extern "C" {
#include "ogg_packer.h"
#include "opus_header.h"
}

class ConnectionLogReader: public ConnectionLogVisitor
{
public:
  ConnectionLogReader(std::string const& logPath);
  bool Start();

private:
  // ConnectionLogVisitor
  void OnSentPacket(uint64_t timestamp, uint8_t const* buf, size_t length) override;
  void OnReceivedPacket(uint64_t timestamp, uint8_t const* buf, size_t length) override;

  std::unique_ptr<ConnectionLog> log_;

  size_t packetsReceived_{0};
  size_t bytesReceived_{0};
};

ConnectionLogReader::ConnectionLogReader(std::string const& logPath) :
  log_{std::make_unique<ConnectionLog>(logPath)}
{
}

bool ConnectionLogReader::Start() {
  if (!log_->IsOK()) {
    return false;
  }

  bool eof{false};
  while (!eof) {
    eof = !log_->ReadNext(this);
  }

  std::cout << "Packets received: " << packetsReceived_ << std::endl;
  std::cout << "Bytes received: " << bytesReceived_ << std::endl;

  return true;
}

void ConnectionLogReader::OnSentPacket(uint64_t timestamp, uint8_t const* buf, size_t length) {
  std::cout << "Packet sent of size: " << length << std::endl;
}

void ConnectionLogReader::OnReceivedPacket(uint64_t timestamp, uint8_t const* buf, size_t length) {
  packetsReceived_++;
  bytesReceived_ += length;

  Rtp rtp(buf, length);
  if (!rtp.Parse()) {
    std::cout << "Failed to parse RTP packet" << std::endl;
  }

  std::cout << "RTP received, ssrc: " << (long)rtp.getSsrc() << ", pt: " << (int)rtp.getPayloadType()
      << ", ext: " << rtp.getExt() << "(" << rtp.getExtensionHeaderLength() <<  "), cc: "
      << (int)rtp.getCc() << ", sn: " << rtp.getSequenceNumber() << ", ts: " << rtp.getTimestamp()
      << ", size: " << length << std::endl;

  // auto toc = rtp.getPayload()[0];
  // std::cout << "toc: " << (int) toc << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " input_file output_file" << std::endl;
  }

  ConnectionLogReader reader(argv[1]);
  reader.Start();

  FILE* fp = fopen(argv[2], "wb");
  if (!fp) {
    std::cout << "Could not open file for writing: " << argv[2] << std::endl;
    return -1;
  }

  OpusHeader header;
  header.channels = 1;
  header.preskip = 0;
  header.input_sample_rate = 22500;
  header.gain = 0;
  header.channel_mapping = 0;

   oggpacker* oggp;
   auto serial = rand();
   oggp = oggp_create(serial);
   if (!oggp) {
    std::cout << "Failed to create ogg container" << std::endl;
   }
   oggp_set_muxing_delay(oggp, 48000);

   int headerSize = opeint_opus_header_get_size(&header);
   unsigned char* p = oggp_get_packet_buffer(oggp, headerSize);
   int packetSize = opeint_opus_header_to_packet(&header, p, headerSize);
   oggp_commit_packet(oggp, packetSize, 0, 0);
   oggp_flush_page(oggp);
   unsigned char* page;
   int len;
   oggp_get_next_page(oggp, &page, &len);
   auto written = fwrite(page, 1, len, fp);
   if (written != len) {
    std::cout << "Failed to write output file" << std::endl;
   }

  if (fp) {
    fclose(fp);
  }

}
