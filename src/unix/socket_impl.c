//
// Created by Kevin Rodrigues on 17/10/2023.
//

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "pico_http_client/unix/socket_impl.h"

tcp_client_t *pico_new_tcp_client() {
    return (tcp_client_t *) malloc(sizeof(tcp_client_t));
}

void pico_free_tcp_client(tcp_client_t *tcp_client) {
    if (tcp_client->fd > 0) {
        close(tcp_client->fd);
    }
    free(tcp_client);
}

int pico_tcp_connect(tcp_client_t *tcp_client, const char *host, const char *port) {
    struct addrinfo hints, *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int getaddrinfo_result = getaddrinfo(host, port, &hints, &result);
    if (getaddrinfo_result != 0) {
        return 1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        tcp_client->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (tcp_client->fd == -1) {
            continue;
        }
        int connect_result = connect(tcp_client->fd, rp->ai_addr, rp->ai_addrlen);
        if (connect_result == -1) {
            // successfully created a socket, but connection failed. close it!
            close(tcp_client->fd);
            tcp_client->fd = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    return 0;
}

int pico_is_tcp_connected(tcp_client_t *tcp_client) {
    return tcp_client->fd;
}

int pico_tcp_poll(tcp_client_t *tcp_client, int timeout_us) {
    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_SET(tcp_client->fd, &read_fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_us;
    int nb_fds_triggered = select(tcp_client->fd + 1, &read_fds, NULL, NULL, &timeout);
    if (nb_fds_triggered > 0) {
        if (FD_ISSET(tcp_client->fd, &read_fds)) {
            return 1;
        }
    } else if (nb_fds_triggered == -1) {
        perror("select()");
        return -1;
    }
    return 0;
}

int pico_tcp_write(tcp_client_t *tcp_client, void *data, size_t size) {
    return write(tcp_client->fd, data, size);
}

int pico_tcp_read(tcp_client_t *tcp_client) {
    int bytes_read = 0, len = 0;
    if (tcp_client->data == NULL) {
        tcp_client->buffer_capacity = BUFSIZ;
        tcp_client->data = calloc(1, tcp_client->buffer_capacity);
    }
    bytes_read = read(tcp_client->fd, tcp_client->data + tcp_client->data_size, tcp_client->buffer_capacity - tcp_client->data_size);
    len += bytes_read;
    tcp_client->data_size += bytes_read;
    if (bytes_read > 0 && tcp_client->data_size == tcp_client->buffer_capacity) {
        tcp_client->buffer_capacity += BUFSIZ;
        char *tmp = (char *) realloc(tcp_client->data, tcp_client->buffer_capacity);
        if (tmp == NULL) {
            free(tcp_client->data);
            tcp_client->data = NULL;
            return 0;
        }
        tcp_client->data = tmp;
    }
    return bytes_read;
}
