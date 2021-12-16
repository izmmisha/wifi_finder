#pragma once

typedef struct { // or this
  uint8_t mac[6];
} __attribute__((packed)) MacAddr;

typedef struct { // still dont know much about this
  int16_t fctl;
  int16_t duration;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

#define maxCh 13 //max Channel -> US = 11, EU = 13, Japan = 14

extern int curChannel;// = 1;
extern MacAddr tm;// = { .mac = {0x2e, 0xd2, 0x45, 0xca, 0x3d, 0x2c}};
extern int last_rssi;// = 0;
extern int64_t last_time;// = 0;
