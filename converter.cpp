#include <iostream>

#include "connection_log.h"
#include "ogg_packer.h"

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
};

ConnectionLogReader::ConnectionLogReader(std::string const& logPath) :
  log_{std::make_unique<ConnectionLog>(logPath)}
{
}

bool ConnectionLogReader::Start() {
  if (!log_->IsOK()) {
    return false;
  }

  return true;
}

void ConnectionLogReader::OnSentPacket(uint64_t timestamp, uint8_t const* buf, size_t length) {
  std::cout << "Packet sent of size: " << length;
}

void ConnectionLogReader::OnReceivedPacket(uint64_t timestamp, uint8_t const* buf, size_t length) {
  std::cout << "Packet received of size: " << length;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " input_file output_file" << std::endl;
  }

  ConnectionLogReader reader(argv[1]);
  reader.Start();
}
