#include "connection_log.h"

#include <iostream>
#include <vector>

ConnectionLog::ConnectionLog(std::string const& outputPath) {
  fp_ = fopen(outputPath.c_str(), "rb+");
}

ConnectionLog::~ConnectionLog()
{
    if (fp_) {
        fclose(fp_);
    }
}

bool ConnectionLog::ReadNext(ConnectionLogVisitor* visitor)
{
    EventHeader header;
    std::vector<uint8_t> buf; // this is not intended to be fast...

    if (!fp_) {
        return false;
    }

    if (fread(&header, sizeof(EventHeader), 1, fp_) != 1) {
        return false;
    }

    buf.resize(header.payloadLength);

    if (fread(buf.data(), header.payloadLength, 1, fp_) != 1) {
        return false;
    }

    switch (static_cast<EventType>(header.eventType)) {
    case EventType::LocalUser:
    case EventType::UserInfo:
    case EventType::CodecInfo:
        return HandleJsonEvent(header, buf);
    case EventType::Receive:
        visitor->OnReceivedPacket(header.timestamp, buf.data(), buf.size());
        break;
    case EventType::Send:
        visitor->OnSentPacket(header.timestamp, buf.data(), buf.size());
        break;
    }

    return true;
}

bool ConnectionLog::HandleJsonEvent(EventHeader& header, std::vector<uint8_t>& buffer)
{
  char const* bufStart = reinterpret_cast<char const*>(buffer.data());
  char const* bufEnd = bufStart + buffer.size();
  std::string event(bufStart, bufEnd);

  switch (static_cast<EventType>(header.eventType)) {
  case EventType::LocalUser:
    std::cout << "LocalUser";
    break;
  case EventType::UserInfo:
    std::cout << "UserInfo";
    break;
  case EventType::CodecInfo:
    std::cout << "CodecInfo";
    break;
  default:
    return false;
  }
  
  std::cout << ": " << event;

  return true;
}