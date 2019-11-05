#include <helpers_tinyxml2.h>
#include <XML_Types.h>

bool string_to_bool(std::string value)
{
  if(value == XML_STRING_TRUE || value == XML_STRING_TRUE_SMALL)
  {
    return true;
  }

  return false;
}

bool get_bool(tinyxml2::XMLElement *element)
{
  return string_to_bool(get_std_string(element).c_str());
}

void print_bool(bool value, tinyxml2::XMLPrinter *printer)
{
  if(value == true)
  {
    printer->PushText(XML_STRING_TRUE);
  }

  else
  {
    printer->PushText(XML_STRING_FALSE);
  }
}

std::string get_std_string(tinyxml2::XMLElement *element)
{
  if(element == nullptr)
  {
    return std::string("");
  }

  const char *value = (const char*) element->GetText();

  if(value != nullptr)
  {
    return std::string(value);
  }

  return std::string();
}

