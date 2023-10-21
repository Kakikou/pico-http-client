//
// Created by Kevin Rodrigues on 18/10/2023.
//

#ifndef INKYHOMEASSITANT_SOCKET_IMPL_H
#define INKYHOMEASSITANT_SOCKET_IMPL_H

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

typedef struct tcp_client {
    int fd;
    void *data;
    uint32_t data_size;
    uint32_t buffer_capacity;
} tcp_client_t;

tcp_client_t *pico_new_tcp_client();

void pico_free_tcp_client(tcp_client_t *);

int pico_tcp_connect(tcp_client_t *tcp_client, const char *host, const char *port);

int pico_is_tcp_connected(tcp_client_t *tcp_client);

int pico_tcp_poll(tcp_client_t *tcp_client, int timeout_us);

int pico_tcp_write(tcp_client_t *tcp_client, void *data, size_t size);

int pico_tcp_read(tcp_client_t *tcp_client);

#endif //INKYHOMEASSITANT_SOCKET_IMPL_H
