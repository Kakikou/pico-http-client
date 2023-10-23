//
// Created by Kevin Rodrigues on 20/10/2023.
//

#include "pico_http_client/pico_http_client.h"

int handle_socket(http_client_t *http_client, url_t *url, void *data, int data_size) {
    tcp_client_t *tcp_client = pico_new_tcp_client();
    if (pico_tcp_connect(tcp_client, url->domain, url->port) > 0) {
        pico_free_tcp_client(tcp_client);
        return 0;
    }
    // Waiting tcp to connect
    int tcp_state = pico_is_tcp_connected(tcp_client);
    for (; tcp_state == 0; tcp_state = pico_is_tcp_connected(tcp_client)) sleep(100);
    if (tcp_state < 0) {
        // There is an issue with the tcp connection.
        // Logs has been pushed by the api
        printf("Not connected\n");
        pico_free_tcp_client(tcp_client);
        return 0;
    }
    pico_tcp_write(tcp_client, data, data_size);
    int total_readed = 0;
    do {
        if (pico_tcp_poll(tcp_client, 1000) > 0) {
            int readed = pico_tcp_read(tcp_client);
            total_readed += readed;
            if (readed == -1) {
                // error happened during the read.
                // we must quit
                total_readed = 0;
                break;
            } else if (readed == 0) {
                // seems socket has been disconnected
                break;
            }
        }
    } while (pico_is_tcp_connected(tcp_client) > 0);
    if (total_readed > 0) {
        http_client->data = tcp_client->data;
        http_client->data_size = tcp_client->data_size;
    } else {
        printf("Read nothing\n");
    }
    pico_free_tcp_client(tcp_client);
    return http_client->data_size;
}
