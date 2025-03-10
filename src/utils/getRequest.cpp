#include "getRequest.h"
#include <iostream>

using namespace std;

// Callback function to write response data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

// Function to send a GET request and return the response as a string
std::string getRequest(std::string url) {
    string ret;
    CURL *curl;
    CURLcode res;
    string response_data;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());  // Set URL
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        
        // Set callback function to capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            ret = "curl_easy_perform() failed: " + string(curl_easy_strerror(res)) + '\n';
        } else {
            ret = response_data;
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    // Cleanup global libcurl state
    curl_global_cleanup();

    return ret;
}
