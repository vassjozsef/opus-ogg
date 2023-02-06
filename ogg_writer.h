#pragma once

#include <string>

extern "C" {
#include "ogg_packer.h"
}

class OggWriter {
public:
  OggWriter(std::string const& oggPath);
  ~OggWriter();

  bool WriteIdHeader(int sampleRate, int channels);
  bool WriteCommentHeader(const std::string& vendor, const std::string& encoder);
  bool WritePacket(const uint8_t* packet, size_t length);
  bool WriteEndStream();
  bool IsOk() const { return fpOgg_; }
  int64_t GetGranulePos() const { return granulePos_; }

private:
  bool OutputPages();

private:
  oggpacker* oggp_{nullptr};
  FILE* fpOgg_{nullptr};
  int64_t granulePos_{0};
};
