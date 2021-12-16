#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "example";

const wifi_promiscuous_filter_t filt={ //Idk what this does
    .filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT|WIFI_PROMIS_FILTER_MASK_DATA
};

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


int curChannel = 1;
MacAddr tm = { .mac = {0x2f, 0xdf, 0x4f, 0xcf, 0x3f, 0x2f}};
int last_rssi = 0;
int64_t last_time = 0;


void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) { //This is where packets end up after they get sniffed
  wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf; // Dont know what these 3 lines do
  int len = p->rx_ctrl.sig_len;
  WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
  len -= sizeof(WifiMgmtHdr);
  if (len < 0){
    return;
  }
  MacAddr sa = wh->sa;
  if (sa.mac[0] == tm.mac[0] && sa.mac[1] == tm.mac[1] && sa.mac[2] == tm.mac[2]
      && sa.mac[3] == tm.mac[3] && sa.mac[4] == tm.mac[4] && sa.mac[5] == tm.mac[5]) {
//  printf("sa: %02X:%02X:%02X:%02X:%02X:%02X %i\n", wh->sa.mac[0], wh->sa.mac[1], wh->sa.mac[2], wh->sa.mac[3], wh->sa.mac[4], wh->sa.mac[5],
//         p->rx_ctrl.rssi);
    last_rssi = p->rx_ctrl.rssi;
    last_time = esp_timer_get_time();
  }
}

static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

/* Initialize wifi with tcp/ip adapter */
static void initialize_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));

    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE));
}

void app_main(void)
{
    initialize_nvs();

    /* Initialize WiFi */
    initialize_wifi();

    printf("Ready to go!\n");

}


