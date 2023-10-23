//
// Created by Kevin Rodrigues on 17/10/2023.
//

#include <lwip/init.h>
#include <pico/time.h>
#include "pico_http_client/lwip/socket_impl.h"

typedef struct {
    int available;
    ip_addr_t remote_addr;
} dns_result_t;

void on_dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    dns_result_t *dns_result = (dns_result_t *) callback_arg;
    if (ipaddr) {
        dns_result->remote_addr = *ipaddr;
        dns_result->available = 1;
    } else {
        dns_result->available = -1;
    }
}

err_t on_connect(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err == ERR_OK) {
#ifdef pico_http_DEBUG
        printf("Socket connected\n");
#endif
    } else {
        printf("Could not connect to socket err = %d\n", err);
    }
    return err;
}

void on_error(void *arg, err_t err) {
    tcp_client_t *tcp_client = (tcp_client_t *) arg;
    cyw43_arch_lwip_begin();
    if (tcp_client != NULL) {
        if (tcp_client->pcb) {
            if (err != ERR_ABRT) {
                printf("Error received from tcp socket. err = %s\n", lwip_strerr(err));
                tcp_abort(tcp_client->pcb);
            }
        }
    }
    cyw43_arch_lwip_end();
}

err_t on_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *pbuf, err_t err) {
    tcp_client_t *tcp_client = (tcp_client_t *) arg;
    cyw43_arch_lwip_begin();
    err_t result = ERR_OK;
    if (tcp_client == NULL) {
        tcp_abort(tpcb);
        result = ERR_ABRT;
    } else if (tpcb == NULL || tcp_client->pcb != tpcb) {
        if (tcp_client->rx_buffer) {
            free(tcp_client->rx_buffer);
            tcp_client->rx_buffer = NULL;
        }
        if (pbuf != NULL) {
            pbuf_free(pbuf);
        }
        tcp_client->pcb = NULL;
        tcp_abort(tpcb);
        result = ERR_ABRT;
    } else if (pbuf) {
        uint32_t new_len = tcp_client->rx_buffer_size + pbuf->tot_len;
        void *buffer = calloc(1, new_len);
        if (buffer == NULL) {
            printf("%s:%d: Could not allocate\n", __FUNCTION__, __LINE__);
            free(tcp_client->rx_buffer);
            tcp_client->rx_buffer = NULL;
            tcp_client->rx_buffer_size = 0;
            pbuf_free(pbuf);
            cyw43_arch_lwip_end();
            return ERR_ABRT;
        }
        // Already pending data in buffer
        // Need to concat new data with the old one
        if (tcp_client->rx_buffer) {
            memcpy(buffer, tcp_client->rx_buffer, tcp_client->rx_buffer_size);
            free(tcp_client->rx_buffer);
        }
        tcp_client->rx_buffer = buffer;
        pbuf_copy_partial(pbuf, tcp_client->rx_buffer + tcp_client->rx_buffer_size, pbuf->tot_len, 0);
        tcp_client->rx_buffer_size = new_len;
        pbuf_free(pbuf);
    } else {
#ifdef pico_http_DEBUG
        printf("Disconnected from remote\n");
#endif
        tcp_abort(tcp_client->pcb);
        tcp_client->pcb = NULL;
        result = ERR_ABRT;
    }
    cyw43_arch_lwip_end();
    return result;
}

err_t on_poll(void *arg, struct tcp_pcb *tpcb) {
    err_t err = ERR_OK;
    tcp_client_t *tcp_client = (tcp_client_t *) arg;
    cyw43_arch_lwip_begin();
    if (tcp_client != NULL && tpcb != NULL && tcp_client->pcb == tpcb) {
        err = tcp_output(tpcb);
    }
    cyw43_arch_lwip_end();
    return err;
}

tcp_client_t *pico_new_tcp_client() {
    return (tcp_client_t *) calloc(1, sizeof(tcp_client_t));
}

void pico_free_tcp_client(tcp_client_t *tcp_client) {
    cyw43_arch_lwip_begin();
    // do not free data since the pointer has been passed to the caller
    if (tcp_client->pcb) { tcp_abort(tcp_client->pcb); }
    free(tcp_client->rx_buffer);
    free(tcp_client);
    cyw43_arch_lwip_end();
}

int pico_tcp_connect(tcp_client_t *tcp_client, const char *host, const char *port) {
    int ret = 0;

    dns_result_t dns_result;
    memset(&dns_result, 0, sizeof(dns_result_t));
    // We need to block until dns resolution.
    // Callback will fill the dns_result.
    if (dns_gethostbyname(host, &dns_result.remote_addr, on_dns_found, &dns_result) == ERR_INPROGRESS) {
        dns_result.available = 0;
        for (;;) {
            cyw43_arch_wait_for_work_until(make_timeout_time_us(1000));
            cyw43_arch_lwip_begin();
            if (dns_result.available == 1) {
                cyw43_arch_lwip_end();
                break;
            } else if (dns_result.available < 0) {
                printf("Could not find dns\n");
                cyw43_arch_lwip_end();
                return 1;
            }
            cyw43_arch_lwip_end();
        }
    }
    cyw43_arch_lwip_begin();
    tcp_client->pcb = tcp_new();
#ifdef pico_http_DEBUG
    printf("Connecting to %s port %u (0x%p)\n", ip4addr_ntoa(&dns_result.remote_addr), atoi(port), tcp_client);
#endif
    tcp_arg(tcp_client->pcb, tcp_client);
    tcp_poll(tcp_client->pcb, on_poll, 10);
    tcp_recv(tcp_client->pcb, on_received);
    tcp_err(tcp_client->pcb, on_error);
    err_t result = tcp_connect(tcp_client->pcb, &dns_result.remote_addr, atoi(port), on_connect);
    if (result != ERR_OK) {
        printf("Fail to connect to %s -> %d\n", ip4addr_ntoa(&dns_result.remote_addr), result);
        ret = 1;
    }
    cyw43_arch_lwip_end();
    return ret;
}

int pico_is_tcp_connected(tcp_client_t *tcp_client) {
    int ret = 0;
    if (tcp_client->rx_buffer) return 1;    // we want to caller to read the data even if the socket is closed
    if (tcp_client->pcb == NULL) return -1; // pcb has been released by lwip
    cyw43_arch_lwip_begin();
    switch (tcp_client->pcb->state) {
        case ESTABLISHED:
            ret = 2;
            break;
        case SYN_SENT:
        case SYN_RCVD:
            ret = 1;
            break;
        case CLOSED:
        case CLOSING:
        case CLOSE_WAIT:
            ret = tcp_client->rx_buffer == NULL ? -1
                                                : 1; // force caller to read the buffer even if the socket is closed
            break;
        default:
            ret = 1;
            break;
    }
    cyw43_arch_lwip_end();
    return ret;
}

int pico_tcp_poll(tcp_client_t *tcp_client, int timeout_us) {
    int ret;
    cyw43_arch_wait_for_work_until(make_timeout_time_us(timeout_us));
    cyw43_arch_lwip_begin();
    ret = tcp_client->rx_buffer == NULL ? 0 : 1;
    cyw43_arch_lwip_end();
    return ret;
}

int pico_tcp_write(tcp_client_t *tcp_client, void *data, size_t size) {
    err_t err;

    if (tcp_client->pcb == NULL || !tcp_sndbuf(tcp_client->pcb)) {
        printf("Could not send data through connection since its not ready yet\n");
        return 1;
    }

    cyw43_arch_lwip_begin();
    err = tcp_write(tcp_client->pcb, data, size, 0);
    if (err != ERR_OK) {
        printf("Fail to write socket\n");
        cyw43_arch_lwip_end();
        return 1;
    }

    cyw43_arch_lwip_end();
    return 0;
}

int pico_tcp_read(tcp_client_t *tcp_client) {
    cyw43_arch_lwip_begin();
    // we must do that protected by cyw43 mutex
    int readed = 0;
    if (tcp_client->rx_buffer != NULL) {
        if (tcp_client->data) {
            // data already read we have to concat it
            char *tmp = (char *) realloc(tcp_client->data, tcp_client->data_size + tcp_client->rx_buffer_size);
            if (tmp == NULL) {
                tcp_abort(tcp_client->pcb);
                free(tcp_client->data);
                tcp_client->data = NULL;
                tcp_client->data_size = 0;
                printf("%s:%d: Could not reallocate\n", __FUNCTION__, __LINE__);
                cyw43_arch_lwip_end();
                return -1;
            }
            memcpy(tmp + tcp_client->data_size, tcp_client->rx_buffer, tcp_client->rx_buffer_size);
            tcp_client->data = tmp;
            tcp_client->data_size += tcp_client->rx_buffer_size;
        } else {
            // swap between internal buffer and external buffer available for user
            tcp_client->data = tcp_client->rx_buffer;
            tcp_client->data_size = tcp_client->rx_buffer_size;
        }
        readed = tcp_client->rx_buffer_size;
        tcp_client->rx_buffer = NULL;
        tcp_client->rx_buffer_size = 0;
    }
    cyw43_arch_lwip_end();
    return readed;
}
