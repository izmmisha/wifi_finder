#include <string.h>
#include <esp_console.h>
#include "argtable3/argtable3.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/uart.h>

#include "sniffer.h"

int save_to_nvs();

static TaskHandle_t cli_task;
static int stop = 0;

static int ping_handler(int argc, char *argv[])
{
    printf("Pong!\n");
    return 0;
}

static int show_handler(int argc, char *argv[])
{
    printf("target: %02X:%02X:%02X:%02X:%02X:%02X\n", tm.mac[0], tm.mac[1], tm.mac[2], tm.mac[3], tm.mac[4], tm.mac[5]);
    printf("channel: %i\n", curChannel);
    return 0;
}

static struct {
  struct arg_int *m0;
  struct arg_int *m1;
  struct arg_int *m2;
  struct arg_int *m3;
  struct arg_int *m4;
  struct arg_int *m5;
  struct arg_end *end;
} target_args;

static int set_target_handler(int argc, char *argv[])
{
    int nerrors = arg_parse(argc, argv, (void **) &target_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, target_args.end, argv[0]);
        return 1;
    }

    tm.mac[0] = target_args.m0->ival[0];
    tm.mac[1] = target_args.m1->ival[0];
    tm.mac[2] = target_args.m2->ival[0];
    tm.mac[3] = target_args.m3->ival[0];
    tm.mac[4] = target_args.m4->ival[0];
    tm.mac[5] = target_args.m5->ival[0];

    save_to_nvs();
    printf("new target: %02X:%02X:%02X:%02X:%02X:%02X\n", tm.mac[0], tm.mac[1], tm.mac[2], tm.mac[3], tm.mac[4], tm.mac[5]);

    return 0;
}

static struct  {
  struct arg_int *channel;
  struct arg_end *end;
} channel_args;

static int set_channel_handler(int argc, char *argv[])
{
    int nerrors = arg_parse(argc, argv, (void **) &channel_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, channel_args.end, argv[0]);
        return 1;
    }

    int new_channel = channel_args.channel->ival[0];
    if(new_channel < 1 || new_channel > maxCh)
        return 1;

    curChannel = new_channel;

    save_to_nvs();
    printf("settting to channel: %i\n", curChannel);

    return 0;
}

static esp_console_cmd_t cmds[] = {
    {
        .command = "ping",
        .help = "",
        .func = ping_handler,
    },
    {
        .command = "show",
        .help = "",
        .func = show_handler,
    },
    {
        .command = "target",
        .help = "",
        .func = set_target_handler,
        .argtable = &target_args,
    },
    {
        .command = "channel",
        .help = "",
        .func = set_channel_handler,
        .argtable = &channel_args,
    },
};

static int register_cli(void)
{
    target_args.m0 = arg_int1(NULL, NULL, "<m0>", "");
    target_args.m1 = arg_int1(NULL, NULL, "<m1>", "");
    target_args.m2 = arg_int1(NULL, NULL, "<m2>", "");
    target_args.m3 = arg_int1(NULL, NULL, "<m3>", "");
    target_args.m4 = arg_int1(NULL, NULL, "<m4>", "");
    target_args.m5 = arg_int1(NULL, NULL, "<m5>", "");
    target_args.end = arg_end(1);

    channel_args.channel = arg_int1(NULL, NULL, "<channel>", "channel number");
    channel_args.end = arg_end(1);

    int cmds_num = sizeof(cmds) / sizeof(esp_console_cmd_t);
    int i;
    for (i = 0; i < cmds_num; i++) {
        esp_console_cmd_register(&cmds[i]);
    }
    return 0;
}

static void scli_task(void *arg)
{
    int uart_num = (int) arg;
    uint8_t linebuf[256];
    int i, cmd_ret;
    esp_err_t ret;
    QueueHandle_t uart_queue;
    uart_event_t event;

    uart_driver_install(uart_num, 256, 0, 8, &uart_queue, 0);
    /* Initialize the console */
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
    };

    esp_console_init(&console_config);

    while (!stop) {
        i = 0;
        memset(linebuf, 0, sizeof(linebuf));
        do {
            ret = xQueueReceive(uart_queue, (void * )&event, (portTickType)portMAX_DELAY);
            if (ret != pdPASS) {
                if (stop == 1) {
                    break;
                } else {
                    continue;
                }
            }
            if (event.type == UART_DATA) {
                while (uart_read_bytes(uart_num, (uint8_t *) &linebuf[i], 1, 0)) {
                    if (linebuf[i] == '\r') {
                        uart_write_bytes(uart_num, "\r\n", 2);
                    } else {
                        uart_write_bytes(uart_num, (char *) &linebuf[i], 1);
                    }
                    i++;
                }
            }
        } while ((i < 255) && linebuf[i - 1] != '\r');
        if (stop) {
            break;
        }
        /* Remove the truncating \r\n */
        linebuf[strlen((char *)linebuf) - 1] = '\0';
        ret = esp_console_run((char *) linebuf, &cmd_ret);
        if (ret < 0) {
            break;
        }
    }
    vTaskDelete(NULL);
}

int scli_init(void)
{
    register_cli();
    xTaskCreate(scli_task, "scli_cli", 4096, (void *) 0, 3, &cli_task);
    if (cli_task == NULL) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

