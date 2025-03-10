#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <curl/curl.h>

// Callback function to write response data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);

// Function to send a GET request and return the response as a string
std::string getRequest(std::string url);

#endif // HTTP_REQUEST_H