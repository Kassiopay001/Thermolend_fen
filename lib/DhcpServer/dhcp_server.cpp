#include "dhcp_server.h"

namespace {
constexpr uint16_t DHCP_SERVER_PORT = 67;
constexpr uint16_t DHCP_CLIENT_PORT = 68;
constexpr uint8_t DHCP_DISCOVER = 1;
constexpr uint8_t DHCP_OFFER = 2;
constexpr uint8_t DHCP_REQUEST = 3;
constexpr uint8_t DHCP_ACK = 5;
constexpr size_t DHCP_OPTIONS_OFFSET = 240;
constexpr size_t PACKET_BUF_SIZE = 576;
const uint8_t MAGIC_COOKIE[4] = {99, 130, 83, 99};

uint8_t findMessageType(const uint8_t *buf, int len) {
  size_t pos = DHCP_OPTIONS_OFFSET;
  while (pos + 1 < (size_t)len) {
    uint8_t code = buf[pos];
    if (code == 255) break;
    if (code == 0) {
      pos++;
      continue;
    }
    uint8_t optLen = buf[pos + 1];
    if (code == 53 && optLen >= 1 && pos + 2 < (size_t)len) {
      return buf[pos + 2];
    }
    pos += 2 + optLen;
  }
  return 0;
}
}  // namespace

void DhcpServer::begin(IPAddress serverIp, IPAddress offeredIp, IPAddress subnetMask, uint32_t leaseSeconds) {
  _serverIp = serverIp;
  _offeredIp = offeredIp;
  _subnetMask = subnetMask;
  _leaseSeconds = leaseSeconds;
  _udp.begin(DHCP_SERVER_PORT);
}

void DhcpServer::loop() {
  int packetSize = _udp.parsePacket();
  if (packetSize <= 0) return;

  uint8_t buf[PACKET_BUF_SIZE];
  int len = _udp.read(buf, sizeof(buf));
  if (len < (int)DHCP_OPTIONS_OFFSET + 4 || buf[0] != 1) return;

  uint8_t msgType = findMessageType(buf, len);
  if (msgType == DHCP_DISCOVER) {
    sendReply(DHCP_OFFER, buf);
  } else if (msgType == DHCP_REQUEST) {
    sendReply(DHCP_ACK, buf);
  }
}

void DhcpServer::sendReply(uint8_t messageType, const uint8_t *requestBuf) {
  uint8_t reply[PACKET_BUF_SIZE];
  memset(reply, 0, sizeof(reply));

  reply[0] = 2;  // BOOTREPLY
  reply[1] = requestBuf[1];
  reply[2] = requestBuf[2];
  reply[3] = 0;

  memcpy(reply + 4, requestBuf + 4, 4);   // xid
  memcpy(reply + 8, requestBuf + 8, 2);   // secs
  memcpy(reply + 10, requestBuf + 10, 2); // flags

  reply[16] = _offeredIp[0];
  reply[17] = _offeredIp[1];
  reply[18] = _offeredIp[2];
  reply[19] = _offeredIp[3];

  memcpy(reply + 28, requestBuf + 28, 16);  // chaddr

  memcpy(reply + 236, MAGIC_COOKIE, 4);

  size_t pos = DHCP_OPTIONS_OFFSET;

  reply[pos++] = 53;
  reply[pos++] = 1;
  reply[pos++] = messageType;

  reply[pos++] = 54;  // server identifier
  reply[pos++] = 4;
  reply[pos++] = _serverIp[0];
  reply[pos++] = _serverIp[1];
  reply[pos++] = _serverIp[2];
  reply[pos++] = _serverIp[3];

  reply[pos++] = 51;  // lease time
  reply[pos++] = 4;
  reply[pos++] = (_leaseSeconds >> 24) & 0xFF;
  reply[pos++] = (_leaseSeconds >> 16) & 0xFF;
  reply[pos++] = (_leaseSeconds >> 8) & 0xFF;
  reply[pos++] = _leaseSeconds & 0xFF;

  reply[pos++] = 1;  // subnet mask
  reply[pos++] = 4;
  reply[pos++] = _subnetMask[0];
  reply[pos++] = _subnetMask[1];
  reply[pos++] = _subnetMask[2];
  reply[pos++] = _subnetMask[3];

  reply[pos++] = 3;  // router
  reply[pos++] = 4;
  reply[pos++] = _serverIp[0];
  reply[pos++] = _serverIp[1];
  reply[pos++] = _serverIp[2];
  reply[pos++] = _serverIp[3];

  reply[pos++] = 6;  // DNS
  reply[pos++] = 4;
  reply[pos++] = _serverIp[0];
  reply[pos++] = _serverIp[1];
  reply[pos++] = _serverIp[2];
  reply[pos++] = _serverIp[3];

  reply[pos++] = 255;  // end

  _udp.beginPacket(IPAddress(255, 255, 255, 255), DHCP_CLIENT_PORT);
  _udp.write(reply, pos);
  _udp.endPacket();
}
