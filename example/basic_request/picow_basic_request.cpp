//
// Created by Kevin Rodrigues on 21/10/2023.
//
#include <map>
#include <string>
#include <malloc.h>
#include <pico/time.h>

#include <pico/stdio.h>
#include <boards/pico_w.h>
#include "pico_http_client/pico_http_client.h"

std::string wifi_name = "";
std::string wifi_password = "";

int init_wifi() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_FRANCE)) {
        printf("Failed to initialise WiFi\n");
        return false;
    }
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_async(wifi_name.c_str(), wifi_password.c_str(), CYW43_AUTH_WPA2_AES_PSK)) {
        return false;
    }

    int flashrate = 500;
    int status = CYW43_LINK_UP + 1;
    while (status >= 0 && status != CYW43_LINK_UP) {
        int new_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        if (new_status != status) {
            status = new_status;
            flashrate = flashrate / (status + 1);
            printf("WiFi connection status: %d %d\n", status, flashrate);
        }
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(flashrate);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(flashrate);
    }
    printf("WiFi Connected\n");
    return 0;
}

void send_http_request(const char *url, const std::map <std::string, std::string> headers) {
    http_client_t *client = new_http_client(url);
    for (auto [key, value]: headers) {
        add_header(client, key.c_str(), value.c_str());
    }
    http_response_t response = http_request(HTTPMethod::GET, client);
    printf("%s\n", response.body);
    free_http_client(client);
    free(response.body);
}

int main() {
    stdio_init_all();
    init_wifi();

    std::map <std::string, std::string> headers{
            {"Content-Type", "application/json"},
    };
    malloc_stats();
    send_http_request("http://worldtimeapi.org/api/timezone/Europe/Paris", headers);
    send_http_request("http://worldtimeapi.org/api/timezone/Europe/London", headers);
    malloc_stats();

    return 0;
}