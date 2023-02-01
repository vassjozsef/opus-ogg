#include "rtp.h"

#include <iostream>

template<typename T>
T get(uint8_t* buffer) {
  T rv{0};
  const size_t bytes = sizeof(T);
  for (auto i = 0u; i < bytes; ++i) {
    rv = rv | ((T)buffer[i] << (bytes - i - 1) * 8);
  }
  return rv;
}

  Rtp::Rtp(uint8_t const* buf, size_t length) {
    buffer_ = std::make_unique<uint8_t[]>(length);
    memcpy(buffer_.get(), buf, length);
  }

  bool Rtp::Parse() {
    uint8_t version = (buffer_[0] & 0xc0) >> 6;
    if (version != 2) {
        return false;
    }

    extension_ = buffer_[0] & 0x10;
    uint8_t cc = buffer_[0] & 0xf;
    mark_ = buffer_[1] & 0x80;
    payloadType_ = buffer_[1] & 0x7f;
    sequenceNumber_ = get<uint16_t>(&buffer_[2]);
    timestamp_ = get<uint32_t>(&buffer_[4]);
    ssrc_ = get<uint32_t>(&buffer_[8]);

    if (extension_) {
        extensionHeaderLength_ = get<uint16_t>(&buffer_[12 + cc * 4 + 2]);
    }

    return true;
  }