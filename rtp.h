#pragma once

#include <vector>

class Rtp {
public:
  Rtp(uint8_t const* buf, size_t length);
  bool Parse();

  uint16_t getSequenceNumber() const { return sequenceNumber_; }
  uint32_t getSsrc() const { return ssrc_; }
  uint32_t getTimestamp() const { return timestamp_; }
  uint8_t getPayloadType() const { return payloadType_; }
  uint8_t getCc() const { return cc; }
  bool getExt() const { return extension_; }
  uint32_t getExtensionHeaderLength() const { return extensionHeaderLength_; }

private:
  std::unique_ptr<uint8_t[]> buffer_;
  bool mark_;
  bool extension_;
  uint8_t cc;
  uint16_t sequenceNumber_;
  uint32_t ssrc_;
  uint8_t payloadType_;
  uint32_t timestamp_;
  uint16_t extensionHeaderLength_;
};