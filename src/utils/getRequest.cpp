#include <iostream>
#include <string>
#include <map>
#include "../lib/nlohmann/json.hpp"
#include <curl/curl.h>

using namespace std;
using json = nlohmann::json;

size_t writeCallback(char *contents, size_t size, size_t nmemb, string &response) {
    response.append(contents, size * nmemb);
    return size * nmemb;
}

// URL Encode function (Simple implementation)
string urlEncode(const string &value) {
    string encoded;
    char hex[4];
    
    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            snprintf(hex, sizeof(hex), "%%%02X", c);
            encoded += hex;
        }
    }
    
    return encoded;
}

// Function to construct a URL with query parameters
string buildUrl(const string &baseUrl, const map<string, string> &params) {
    if (params.empty()) return baseUrl;

    string fullUrl = baseUrl + "?";
    for (auto it = params.begin(); it != params.end(); ++it) {
        fullUrl += urlEncode(it->first) + "=" + urlEncode(it->second);
        if (next(it) != params.end()) fullUrl += "&";
    }

    return fullUrl;
}

// Function to perform GET request with query parameters
json fetchJson(const string &baseUrl, const map<string, string> &queryParams = {}) {
    string response;
    string url = buildUrl(baseUrl, queryParams);

    // Initialize curl with RAII wrapper
    unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);

    if (!curl) {
        throw runtime_error("Failed to initialize cURL");
    }

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, +[](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t {
        return writeCallback(ptr, size, nmemb, *static_cast<string *>(userdata));
    });
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "Mozilla/5.0"); // Some APIs require this

    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK) {
        throw runtime_error("cURL error: "s + curl_easy_strerror(res));
    }

    // Parse JSON response
    return json::parse(response);
}

int main(){
    map<string, string> queries;
    queries["info_hash"] = "70edcac2611a8829ebf467a6849f5d8408d9d8f4";
    queries["peer_id"] = "adityahanjiteddybear";
    queries["port"] = "6881";
    queries["uploaded"] = "0";
    queries["downloaded"] = "0";
    queries["left"] = "262144";
    queries["compact"] = "1";
    cout << fetchJson("http://bittorrent-test-tracker.codecrafters.io", queries).dump() << endl;
    return 0;
}