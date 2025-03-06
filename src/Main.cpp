#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

string decode_bencoded_value(const std::string& encoded_value) {

    if (std::isdigit(encoded_value[0])) {
        // std::string ret = "";
        // int i = 0;
        // while (encoded_value[i] != ':')
        //     i++;
        // return json(encoded_value.substr(i+1, encoded_value.length()-i+1));
        
        // Example: "5:hello" -> "hello"
        size_t colon_index = encoded_value.find(':');
        if (colon_index != std::string::npos) {
            std::string number_string = encoded_value.substr(0, colon_index);
            int64_t number = std::atoll(number_string.c_str());
            std::string str = encoded_value.substr(colon_index + 1, number);
            return json(str);
        } else {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }
    } 
    else if (encoded_value[0] == 'i'){
        long long total = 0;
        int i = 1;
        if (encoded_value[i] == '-')
            i++;
        while (encoded_value[i] != 'e'){
            total *= 10;
            total += encoded_value[i]-'0';
            i++;
        }
        if (i == encoded_value.length()-1){
            if (encoded_value[1] == '-')
                return json(-total);
            else
                return json(total);
        }
        else throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }
    else {
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}



int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    if (command == "decode") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
            return 1;
        }
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        std::cerr << "Logs from your program will appear here!" << std::endl;

        // Uncomment this block to pass the first stage
        std::string encoded_value = argv[2];
        json decoded_value = decode_bencoded_value(encoded_value);
        std::cout << decoded_value << std::endl;
        // std::cout << decoded_value.dump() << std::endl;
    } else {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}
