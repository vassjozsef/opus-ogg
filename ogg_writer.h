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
  bool WriteCommentHeader();
  bool WritePacket(const uint8_t* packet, size_t length);
  bool WriteEndStream();
  bool isOk() const { return fpOgg_; }

private:
  bool OutputPages();

private:
  oggpacker* oggp_{nullptr};
  FILE* fpOgg_{nullptr};
  int64_t granulePos_{0};
};