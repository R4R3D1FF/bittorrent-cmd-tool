#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include "lib/nlohmann/json.hpp"
#include "lib/sha/sha1.h"


using json = nlohmann::json;
using namespace std;

void outputHex(string str) {
    
    for (unsigned char c : str) {
        std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
    }
    std::cerr << std::endl;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Error reading file: " + filename);
    }

    return std::string(buffer.begin(), buffer.end());
}

string printVector(vector<string> v){
    if (v.size() == 0)
        return "[]";
    string ret = "[";
    for (int i = 0; i < v.size(); i++){
        ret += v[i];
        if (i != v.size()-1)
            ret += ',';
        else
            ret += ']';
    }
    return ret;
}

string printVector(vector<pair<string, string>> v){
    if (v.size() == 0)
        return "{}";
    string ret = "{";
    for (int i = 0; i < v.size(); i++){
        ret += v[i].first;
        ret += ':';
        ret += v[i].second;
        if (i != v.size()-1)
            ret += ',';
        else
            ret += '}';
    }
    return ret;
}


pair<json, int> decode_bencoded_value_pair(const string& encoded_value, int init = 0) {
    int i = init;

    if (isdigit(encoded_value[i])) {
        // string ret = "";
        // int i = 0;
        // while (encoded_value[i] != ':')
        //     i++;
        // return json(encoded_value.substr(i+1, encoded_value.length()-i+1));
        
        // Example: "5:hello" -> "hello"
        size_t colon_index = encoded_value.find(':', i);
        if (colon_index != string::npos) {
            string number_string = encoded_value.substr(i, colon_index);
            int64_t number = atoll(number_string.c_str());
            string str = encoded_value.substr(colon_index + 1, number);
            return {json(str), colon_index - init + 1 + number};
        } else {
            throw runtime_error("Invalid encoded value: " + encoded_value);
        }
    } 
    else if (encoded_value[i] == 'i'){
        long long total = 0;
        i++;
        if (encoded_value[i] == '-')
            i++;
        while (encoded_value[i] != 'e' && i < encoded_value.length()){
            total *= 10;
            total += encoded_value[i]-'0';
            i++;
        }
        if (i >= encoded_value.length())
            throw runtime_error("Invalid encoded value: " + encoded_value);
        if (encoded_value[1] == '-')
            return {json(-total), i - init + 1};
        else
            return {json(total), i - init + 1};
    }

    else if (encoded_value[i] == 'l'){
        vector<json> ret;
        i++;
        while (encoded_value[i] != 'e'){
            pair<json, int> listItem = decode_bencoded_value_pair(encoded_value, i);
            ret.push_back(listItem.first);
            i += listItem.second;
            if (i >= encoded_value.length())
                throw runtime_error("Invalid encoded value: " + encoded_value);
        }
        return {json(ret), i - init + 1};
    }

    else if (encoded_value[i] == 'd'){
        map<json, json> ret;
        i++;
        while (encoded_value[i] != 'e'){
            pair<json, int> listItem1 = decode_bencoded_value_pair(encoded_value, i);
            cerr << listItem1.first << endl;
            i += listItem1.second;
            pair<json, int> listItem2 = decode_bencoded_value_pair(encoded_value, i);
            i += listItem2.second;
            ret[listItem1.first] = listItem2.first;
            if (i >= encoded_value.length())
                throw runtime_error("Invalid encoded value: " + encoded_value);
        }
        return {json(ret), i - init + 1};
    }

    else {
        throw runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

json decode_bencoded_value(const string& encoded_value){
    return decode_bencoded_value_pair(encoded_value).first;
}



string bencode_json(const json& info) {
    string ret = "";

    if (info.is_array()) {
        ret += "l";
        for (const auto& item : info) {
            ret += bencode_json(item);
        }
        ret += "e";
    } 
    else if (info.is_object()) {
        ret += "d";

        // Ensure lexicographic order of keys
        map<json, json> sorted_dict(info.begin(), info.end());

        for (const auto& [key, value] : sorted_dict) {
            ret += bencode_json(key);
            ret += bencode_json(value);
        }

        ret += "e";
    } 
    else if (info.is_number_integer()) {
        ret += "i" + to_string(info.get<int64_t>()) + "e";
    } 
    else if (info.is_number_float()) {
        ret += "i" + to_string(static_cast<int64_t>(info.get<double>())) + "e";  // Bencode does not support floats, truncate
    } 
    else if (info.is_string()) {
        string raw_str = info.get<string>();  // Get the actual string
        ret += to_string(raw_str.length()) + ":" + raw_str;
    }

    return ret;
}


// string bencode_json(json info){
//     // cerr<<"Bencoding\n" << info.dump(-1, ' ' , false, json::error_handler_t::ignore) << endl;
//     string ret = "";
//     if (info.is_array()){
//         ret += "l";
//         for (int i = 0; i < info.size(); i++){
//             ret += bencode_json(info[i]);
//         }
//         ret += "e";
//     }
//     else if (info.is_object()){
//         ret += "d";
//         for (auto item: info.items()){
//             ret += bencode_json(item.key());
//             ret += bencode_json(item.value());
//         }
//         ret += "e";
//     }
//     else if (info.is_number()){
//         ret += 'i';
//         ret += info.dump(-1, ' ' , false, json::error_handler_t::ignore);
//         ret += 'e';
//     }
//     else if (info.is_string()){
//         string unstripped = info.dump(-1, ' ' , false, json::error_handler_t::ignore);
//         ret += to_string(unstripped.length()-2);
//         ret += ':';
        
//         ret += unstripped.substr(1, unstripped.length()-2);

//     }
//     return ret;

// }



int main(int argc, char* argv[]) {
    // Flush after every cout / cerr
    cout << unitbuf;
    cerr << unitbuf;

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " decode <encoded_value>" << endl;
        return 1;
    }

    string command = argv[1];

    if (command == "decode") {
        if (argc < 3) {
            cerr << "Usage: " << argv[0] << " decode <encoded_value>" << endl;
            return 1;
        }
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        cerr << "Logs from your program will appear here!" << endl;

        // Uncomment this block to pass the first stage
        string encoded_value = argv[2];
        json decoded_value = decode_bencoded_value(encoded_value);
        cout << decoded_value.dump() << endl;
    } 

    else if (command == "info"){
        if (argc < 3) {
            cerr << "Usage: " << argv[0] << " info <torrent_file>" << endl;
            return 1;
        }
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        cerr << "Logs from your program will appear here!" << endl;

        // Uncomment this block to pass the first stage
        string fileContents;
        fileContents = readFile(argv[2]);
        outputHex(fileContents);
        json decoded_value = decode_bencoded_value(fileContents);
        string trackerURL = decoded_value["announce"].dump();
        cout << "Tracker URL: " << trackerURL.substr(1, trackerURL.length()-2) << endl;
        cout << "Length: " << decoded_value["info"]["length"] << endl;
        cerr << "Bencoded json: ";
        outputHex(bencode_json(decoded_value["info"]));
        
        cout << "Info: " << sha1(bencode_json(decoded_value["info"])) << endl;
    }

    else {
        cerr << "unknown command: " << command << endl;
        return 1;
    }

    return 0;
}
