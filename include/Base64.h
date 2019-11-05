#ifndef _BASE64
#define _BASE64
#include <string>
#include <vector>
//#define TEXT(x) x     //Not unicode


const static char encodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const static char padCharacter = '=';

std::basic_string<char> base64Encode(std::vector<unsigned char> inputBuffer);
std::vector<unsigned char> base64Decode(const std::basic_string<char> input);

std::string Base64_get_string(const std::basic_string<char> input);
std::string Base64_encode_string(std::string input);
#endif
