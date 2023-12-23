#ifndef MICRO_HTTP_CLIENT_H_
#define MICRO_HTTP_CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PICO_BOARD
#include "lwip/socket_impl.h"
#else
#include "unix/socket_impl.h"
#endif

/* Update also method2string accordingly */
enum HTTPMethod {
    OPTIONS = 0,
    GET,
//    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    CONNECT
};

typedef struct http_response {
    int code;
    char *body;
} http_response_t;

typedef struct http_client {
    char *headers;
    size_t header_len;
    char *url;
    char *post;
    void *data;
    int data_size;
} http_client_t;

typedef struct {
    char *scheme;
    char *domain;
    char *port;
    char *path;
    char *query;
} url_t;

http_client_t *new_http_client(const char *url);

int stringlog10(const char *s);

int add_post(http_client_t *http_client, const char *post);

int add_header(http_client_t *http_client, const char *key, const char *value);

void free_http_client(http_client_t *http_client);

http_response_t http_request(enum HTTPMethod method, http_client_t *http_client);

int handle_socket(http_client_t *http_client, url_t *url, void *data, int data_size);

#ifdef __cplusplus
}
#endif

#endif // MICRO_HTTP_CLIENT_H_