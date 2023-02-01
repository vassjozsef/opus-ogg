#include <iostream>

#include "connection_log.h"
#include "ogg_packer.h"
#include "rtp.h"

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
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " input_file output_file" << std::endl;
  }

  ConnectionLogReader reader(argv[1]);
  reader.Start();
}
