#include "rtp.h"

#include <iostream>

const size_t RtpHeaderLength = 12;

template<typename T>
T get(uint8_t* buffer) {
  T rv{0};
  const size_t bytes = sizeof(T);
  for (auto i = 0u; i < bytes; ++i) {
    rv = rv | ((T)buffer[i] << (bytes - i - 1) * 8);
  }
  return rv;
}

Rtp::Rtp(uint8_t const* buf, size_t length)
  : length_{length}
{
  buffer_ = (uint8_t*)malloc(length);
  memcpy(buffer_, buf, length);
}

Rtp::~Rtp()
{
  if (buffer_) {
    free(buffer_);
  }
}

bool Rtp::Parse()
{
  uint8_t version = (buffer_[0] & 0xc0) >> 6;
  if (version != 2) {
    return false;
  }

  extension_ = buffer_[0] & 0x10;
  cc_ = buffer_[0] & 0xf;
  mark_ = buffer_[1] & 0x80;
  payloadType_ = buffer_[1] & 0x7f;
  sequenceNumber_ = get<uint16_t>(&buffer_[2]);
  timestamp_ = get<uint32_t>(&buffer_[4]);
  ssrc_ = get<uint32_t>(&buffer_[8]);

  if (extension_) {
    extensionHeaderLength_ = get<uint16_t>(&buffer_[RtpHeaderLength + cc_ * 4 + 2]);
  }

  return true;
}

uint8_t* Rtp::getPayload() const
{
  auto extensionHeaderLengthBytes = extension_ ? 4 * (extensionHeaderLength_ + 1): 0;
  return &buffer_[RtpHeaderLength + cc_ * 4 + extensionHeaderLengthBytes];
}

size_t Rtp::getPayloadSize() const
{
  auto extensionHeaderLengthBytes = extension_ ? 4 * (extensionHeaderLength_ + 1): 0;
  return length_ - (RtpHeaderLength + cc_ * 4 + extensionHeaderLengthBytes);
}