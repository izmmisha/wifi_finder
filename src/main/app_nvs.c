#include "nvs_flash.h"
#include "nvs.h"
#include "sniffer.h"

int load_from_nvs()
{
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));

    if(ESP_OK != nvs_get_u8(my_handle, "m0", &tm.mac[0])
       || ESP_OK != nvs_get_u8(my_handle, "m1", &tm.mac[1])
       || ESP_OK != nvs_get_u8(my_handle, "m2", &tm.mac[2])
       || ESP_OK != nvs_get_u8(my_handle, "m3", &tm.mac[3])
       || ESP_OK != nvs_get_u8(my_handle, "m4", &tm.mac[4])
       || ESP_OK != nvs_get_u8(my_handle, "m5", &tm.mac[5]))
    {
        tm.mac[0] = 0x1f;
        tm.mac[1] = 0x1f;
        tm.mac[2] = 0xdf;
        tm.mac[3] = 0x3f;
        tm.mac[4] = 0xbf;
        tm.mac[5] = 0x5f;
    }

    if(ESP_OK != nvs_get_i32(my_handle, "ch", &curChannel))
        curChannel = 1;

    nvs_close(my_handle);
    return 0;
}

int save_to_nvs()
{
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));

    ESP_ERROR_CHECK(nvs_set_u8(my_handle, "m0", tm.mac[0]));
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, "m1", tm.mac[1]));
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, "m2", tm.mac[2]));
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, "m3", tm.mac[3]));
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, "m4", tm.mac[4]));
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, "m5", tm.mac[5]));

    ESP_ERROR_CHECK(nvs_set_i32(my_handle, "ch", curChannel));

    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    return 0;
}
