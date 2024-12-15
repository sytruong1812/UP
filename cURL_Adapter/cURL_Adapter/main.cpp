#include <iostream>
#include "curl.h"

int main() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        printf("Failed to initialize CURL.\n");
        return 1;
    }

    // Set options
    char url[] = "http://example.com";
    long timeout = 30;
    char headers[] = "User-Agent: CustomAgent";

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, &timeout);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURL_OK) {
        printf("CURL perform error: %d\n", res);
    }

    // Cleanup
    curl_easy_cleanup(curl);
    return 0;
}
