#include "connection_log.h"
#include "ogg_writer.h"
#include "rtp.h"

constexpr int OpusSsrc = 1387;
constexpr int SampleRate = 48000;
constexpr int Channels = 1;

class Converter : public ConnectionLogVisitor {
public:
  Converter(std::string const& logPath, std::string const& oggPath);

  bool Start();

private:
  // ConnectionLogVisitor
  void OnSentPacket(uint64_t timestamp, uint8_t const* buf, size_t length) override;
  void OnReceivedPacket(uint64_t timestamp, uint8_t const* buf, size_t length) override;

  ConnectionLog connectionLog_;
  OggWriter oggWriter_;

  size_t packetsReceived_{0};
  size_t bytesReceived_{0};
};

Converter::Converter(std::string const& logPath, std::string const& oggPath)
  : connectionLog_(logPath)
  , oggWriter_(oggPath)
{
}

bool Converter::Start() {
  if (!connectionLog_.IsOK() || !oggWriter_.IsOk()) {
    return false;
  }

  if (!oggWriter_.WriteIdHeader(SampleRate, Channels)) {
    std::cout << "Failed to write id header" << std::endl;
    return false;
  }

  auto version = "Opus provided by WebRTC M89";
  auto encoder = "Discord Client";
  if (!oggWriter_.WriteCommentHeader(version, encoder)) {
    std::cout << "Failed to write comment header" << std::endl;
    return false;
  }

  bool eof{false};
  while (!eof) {
    eof = !connectionLog_.ReadNext(this);
  }

  std::cout << "Packets received: " << packetsReceived_ << std::endl;
  std::cout << "Bytes received: " << bytesReceived_ << std::endl;

  if (!oggWriter_.WriteEndStream()) {
    std::cout << "Faield to write end of stream" << std::endl;
  }

  return true;
}

void Converter::OnSentPacket(uint64_t timestamp, uint8_t const* buf, size_t length) {
  std::cout << "Packet sent of size: " << length << std::endl;
}

void Converter::OnReceivedPacket(uint64_t timestamp, uint8_t const* buf, size_t length) {
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

  oggWriter_.WritePacket(rtp.getPayload(), rtp.getPayloadSize()); 
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " input_file output_file" << std::endl;
  }

  Converter converter(argv[1], argv[2]);
  converter.Start();
}
