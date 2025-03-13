#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include "lib/nlohmann/json.hpp"
#include "lib/sha/sha1.hpp"
#include <curl/curl.h>
#include "./utils/getRequest.h"
#include <format>
#include <boost/asio.hpp>


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



string urlEncodeHex(string hex){
    string ret;
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        // Convert each pair of hex characters to a byte
        string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(stoi(byteString, nullptr, 16));

        // URL encode if necessary
        if (isalnum(byte) || byte == '-' || byte == '_' || byte == '.' || byte == '~') {
            ret += byte;  // Safe characters remain unchanged
        } else {
            ret += '%';
            ret += hex[i];
            ret += hex[i+1];
        }
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





string bencode_json(json info){
    // cerr<<"Bencoding\n" << info.dump(-1, ' ' , false, json::error_handler_t::ignore) << endl;
    string ret = "";
    if (info.is_array()){
        ret += "l";
        for (int i = 0; i < info.size(); i++){
            ret += bencode_json(info[i]);
        }
        ret += "e";
    }
    else if (info.is_object()){
        ret += "d";
        for (auto item: info.items()){
            ret += bencode_json(item.key());
            ret += bencode_json(item.value());
        }
        ret += "e";
    }
    else if (info.is_number()){
        ret += 'i';
        ret += info.dump(-1, ' ' , false, json::error_handler_t::ignore);
        ret += 'e';
    }
    else if (info.is_string()){
        string raw = info.get<string>();
        ret += to_string(raw.length());
        ret += ':';
        
        ret += raw;

    }
    return ret;

}

void listHashes(string pieces){
    for (int i = 0; i < pieces.length(); i++){
        cout << pieces[i];
        if (i % 40 == 39)
            cout << endl;
    }
}

string getHex(string s){
    string ret;
    char chrs[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    for (int i = 0; i < s.length(); i++){
        ret += chrs[((unsigned char) s[i])/16];
        ret += chrs[((unsigned char) s[i])%16];
    }
    return ret;
}

vector<string> extractPeers(string s){
    vector<string> ret;
    for (int i = 0; i < s.length(); i += 6){
        string peer = "";
        for (int j = 0; j < 4; j++){
            cerr << (uint8_t) s[i+j] << endl;
            peer += to_string((uint8_t) s[i+j]);
            if (j != 3)
                peer += '.';
        }
        // peer += to_string(s[i+4]*256 + s[i+5]);
        uint16_t num;
        num = (((uint8_t) s[i+4]) << 8) | (uint8_t)s[i+5]; // Ensures correct order

        peer += ':' + std::to_string(num);
        ret.push_back(peer);
    }
    return ret;
}

vector<uint8_t> decodeHex(string s){
    vector<uint8_t> ret;
    for (int i = 0; i < s.length(); i+=2){
        uint8_t num;
        if (s[i] <= '9')
            num += 16*(s[i] - '0');
        else{
            num += 16*(10 + s[i] - 'a');
        }
        if (s[i+1] <= '9')
            num += s[i] - '0';
        else
            num += s[i] - 'a' + 10;
        ret.push_back(num);
    }
}

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
        cerr << fileContents << endl;
        cerr << getHex(fileContents) << endl;
        // outputHex(fileContents);
        json decoded_value = decode_bencoded_value(fileContents);
        string trackerURL = decoded_value["announce"].dump();
        cout << "Tracker URL: " << trackerURL.substr(1, trackerURL.length()-2) << endl;
        cout << "Length: " << decoded_value["info"]["length"] << endl;
        cerr << "Bencoded json: ";
        outputHex(bencode_json(decoded_value["info"]));
        SHA1 sha1;
        sha1.update(bencode_json(decoded_value["info"]));
        cout << "Info Hash: " << sha1.final() << endl;
        cout << "Piece Length: " << decoded_value["info"]["piece length"].dump() << endl;
        cout << "Piece Hashes:\n";
        string pieceHashes = decoded_value["info"]["pieces"].get<string>();
        cerr << "pieceHashesOrig: " << pieceHashes << endl;
        string pieceHashesHex = getHex(pieceHashes);
        cerr << "pieceHashesHex: " << pieceHashesHex << endl;
        listHashes(pieceHashesHex);

    }

    else if (command == "peers"){
        if (argc < 3) {
            cerr << "Usage: " << argv[0] << " info <torrent_file>" << endl;
            return 1;
        }
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        cerr << "Logs from your program will appear here!" << endl;

        // Uncomment this block to pass the first stage
        string fileContents;
        fileContents = readFile(argv[2]);
        json decoded_value = decode_bencoded_value(fileContents);
        string trackerURL = decoded_value["announce"].get<string>();
        string peer_id = "adityahanjiteddybear";
        SHA1 sha1;
        sha1.update(bencode_json(decoded_value["info"]));
        string info_hash = urlEncodeHex(sha1.final());
        int port = 6881;
        int uploaded = 0;
        int downloaded = 0;
        int left = decoded_value["info"]["length"];
        int compact = 1;
        cerr << format("{}?info_hash={}&peer_id={}&port={}&uploaded={}&downloaded={}&left={}&compact={}", trackerURL, info_hash, peer_id, port, uploaded, downloaded, left, compact) << endl;
        string resp = getRequest(format("{}?info_hash={}&peer_id={}&port={}&uploaded={}&downloaded={}&left={}&compact={}", trackerURL, info_hash, peer_id, port, uploaded, downloaded, left, compact));
        cerr << resp << endl;
        json decoded_resp = decode_bencoded_value(resp);
        string peersRaw = decoded_resp["peers"].get<string>();
        
        vector<string> peers = extractPeers(peersRaw);
        for (int i = 0; i < peers.size(); i++){
            cout << peers[i] << endl;
        }



    }

    else if (command == "handshake"){
        string fileContents;
        fileContents = readFile(argv[2]);
        json decoded_value = decode_bencoded_value(fileContents);
        SHA1 sha1;
        sha1.update(bencode_json(decoded_value["info"]));
        vector<uint_8> rawInfoHash = decodeHex(sha1.final());
        string peer = argv[3];
        try {
            boost::asio::io_context io_context;
    
            // Connect to the server at 127.0.0.1, port 8000
            tcp::resolver resolver(io_context);
            tcp::resolver::results_type endpoints = resolver.resolve(peer.substr(0, peer.size()-5), peer.substr(peer.size()-4, 4));
            tcp::socket socket(io_context);
            boost::asio::connect(socket, endpoints);
    
            // Send data
            std::vector<uint8_t> message = {0x19, "B", "i", "t", "t", "o", "r", "r", "e", "n", "t", " ", "p", "r", "o", "t", "o", "c", "o", "l", 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
            message.insert(message.end(), rawInfoHash.begin(), rawInfoHash.end());
            string peerid = "adityahanjiteddybear";
            message.insert(message.end(), peerid.begin(), peerid.end());


            boost::asio::write(socket, boost::asio::buffer(message));
    
            // Listen for response
            char reply[1024];
            size_t reply_length = socket.read_some(boost::asio::buffer(reply));
            std::cout << std::string(reply+reply_length-20, reply_length) << std::endl;
            
        } catch (std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    
        return 0;
    }

    else {
        cerr << "unknown command: " << command << endl;
        return 1;
    }

    return 0;
}
