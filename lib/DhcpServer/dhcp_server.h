#pragma once

#include <Arduino.h>
#include <EthernetUdp.h>

class DhcpServer {
 public:
  void begin(IPAddress serverIp, IPAddress offeredIp, IPAddress subnetMask, uint32_t leaseSeconds = 86400);
  void loop();

 private:
  EthernetUDP _udp;
  IPAddress _serverIp;
  IPAddress _offeredIp;
  IPAddress _subnetMask;
  uint32_t _leaseSeconds = 86400;

  void sendReply(uint8_t messageType, const uint8_t *requestBuf);
};
