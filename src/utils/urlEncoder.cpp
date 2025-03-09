#include <iostream>
#include <string>

using namespace std;

int hexNum(string s){
    int ret = 0;
    if (s[0] <= '9')
        ret += (s[0] - '0')*16;
    else
        ret += (10+s[0] - 'a')*16;
    if (s[1] <= '9')
        ret += (s[1] - '0');
    else
        ret += (10+s[1] - 'a');
    return ret;
}

string urlEncode(const string &input) {
    string ret = "";
    for (int i = 0; i < input.length()-1; i+=2){
        if (isalnum(char(hexNum(input.substr(i, 2)))))
            ret += char(hexNum(input.substr(i, 2)));
        else{
            ret += '%';
            ret += input[i];
            ret += input[i+1];
        }
    }
    return ret;
}

// int main(){
//     cout << urlEncode("c77829d2a77d6516f88cd7a3de1a26abcbfab0db");
//     return 0;
// }