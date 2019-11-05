#ifndef _HELPERS_TINYXML2
#define _HELPERS_TINYXML2

#include <type_convert.h>
#include <tinyxml2.h>
#include <Base64.h>

bool get_bool(tinyxml2::XMLElement *element);
void print_bool(bool value, tinyxml2::XMLPrinter *printer);
bool string_to_bool(std::string value);
std::string get_std_string(tinyxml2::XMLElement *element);

template <typename Type>
Type variable_read(tinyxml2::XMLElement *element)
{
  return uchar_to_variable<Type>(base64Decode(get_std_string(element)));
}

template <typename Type>
void variable_write(Type variable, tinyxml2::XMLPrinter *printer)
{
  printer->PushText(base64Encode(variable_to_uchar<Type>(variable)).c_str());
}


#endif
