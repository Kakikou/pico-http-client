//
// Created by Kevin Rodrigues on 21/10/2023.
//

#include <map>
#include <string>

#include "pico_http_client/pico_http_client.h"

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
    std::map <std::string, std::string> headers{
            {"Content-Type", "application/json"},
    };
    send_http_request("http://worldtimeapi.org/api/timezone/Europe/Paris", headers);

    return 0;
}