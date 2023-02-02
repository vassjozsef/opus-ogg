#pragma once

#include <vector>

class Rtp {
public:
  Rtp(uint8_t const* buf, size_t length);
  ~Rtp();
  bool Parse();

  uint16_t getSequenceNumber() const { return sequenceNumber_; }
  uint32_t getSsrc() const { return ssrc_; }
  uint32_t getTimestamp() const { return timestamp_; }
  uint8_t getPayloadType() const { return payloadType_; }
  uint8_t getCc() const { return cc_; }
  bool getExt() const { return extension_; }
  uint32_t getExtensionHeaderLength() const { return extensionHeaderLength_; }
  uint8_t* getPayload() const;
  size_t getPayloadSize() const;

private:
  uint8_t* buffer_{nullptr};
  size_t length_;
  bool mark_;
  bool extension_;
  uint8_t cc_;
  uint16_t sequenceNumber_;
  uint32_t ssrc_;
  uint8_t payloadType_;
  uint32_t timestamp_;
  uint16_t extensionHeaderLength_;
};