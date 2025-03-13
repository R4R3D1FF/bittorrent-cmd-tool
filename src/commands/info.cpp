// #include <iostream>
// #include <string>
// #include "../lib/nlohmann/json.hpp"
// #include "utils/readFile.hpp"
// using json = nlohmann::json;
// using namespace std;

// void info(string torrentFile){
//     string fileContents;
//     fileContents = readFile(argv[2]);
//     // cerr << fileContents << endl;
//     // cerr << getHex(fileContents) << endl;
//     // outputHex(fileContents);
//     json decoded_value = decode_bencoded_value(fileContents);
//     string trackerURL = decoded_value["announce"].dump();
//     cout << "Tracker URL: " << trackerURL.substr(1, trackerURL.length()-2) << endl;
//     cout << "Length: " << decoded_value["info"]["length"] << endl;
//     // cerr << "Bencoded json: ";
//     // outputHex(bencode_json(decoded_value["info"]));
//     SHA1 sha1;
//     sha1.update(bencode_json(decoded_value["info"]));
//     cout << "Info Hash: " << sha1.final() << endl;
//     cout << "Piece Length: " << decoded_value["info"]["piece length"].dump() << endl;
//     cout << "Piece Hashes:\n";
//     string pieceHashes = decoded_value["info"]["pieces"].get<string>();
//     // cerr << "pieceHashesOrig: " << pieceHashes << endl;
//     string pieceHashesHex = getHex(pieceHashes);
//     // cerr << "pieceHashesHex: " << pieceHashesHex << endl;
//     listHashes(pieceHashesHex);
// }