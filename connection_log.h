#pragma once

#include <iostream>
#include <type_traits>
#include <cstdio>
#include <string>

class ConnectionLogVisitor;

class ConnectionLog {
public:
  ConnectionLog(std::string const& outputPath);
   ~ConnectionLog();

  bool IsOK() { return fp_; }
  bool ReadNext(ConnectionLogVisitor& visitor);

private:
  struct EventHeader {
    uint64_t timestamp;
    uint32_t eventType;
    uint32_t payloadLength;
  };

  enum class EventType { LocalUser, UserInfo, CodecInfo, Send, Receive };

  bool HandleJsonEvent(ConnectionLogVisitor& visitor, EventHeader& header, std::vector<uint8_t>& buffer);

  FILE* fp_{};
};

class ConnectionLogVisitor {
public:
  virtual void OnSentPacket(uint64_t timestamp, uint8_t const* buf, size_t length) = 0;
  virtual void OnReceivedPacket(uint64_t timestamp, uint8_t const* buf, size_t length) = 0;
};