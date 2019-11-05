#ifndef _TYPE_CONVERT
#define _TYPE_CONVERT

#include <vector>
#include <stdint.h>
#include <stdbool.h>
#include <cstdlib>

// Union type whitch is used to convert wanted type to unsigned char and back.
template <typename Type>
union type_char_convert
{
  Type type;
  unsigned char c[sizeof(Type)];

  int get_size()
  {
    return sizeof(Type);
  }
};

std::vector<float> vectorUChar_to_vectorFloat(std::vector<unsigned char> source);
std::vector<unsigned char> vectorFloat_to_vectorUChar(std::vector<float> &source);

std::vector<unsigned int> vectorUChar_to_vectorUINT(std::vector<unsigned char> source);
std::vector<unsigned char> vectorUINT_to_vectorUChar(std::vector<unsigned int> &source);


// Convertin given variable to the std::vector<unsigned char>
template <typename Type>
std::vector<unsigned char> variable_to_uchar(Type value)
{
  std::vector<unsigned char> buffer;

  type_char_convert<Type> fc;
  fc.type = value;

  for(int counter = 0; counter <= fc.get_size() - 1; counter++)
  {
    buffer.push_back(fc.c[counter]);
  }

  if(buffer.size() == sizeof(Type))
  {
    return buffer;
  }

  return std::vector<unsigned char>();
}

// Convertin given std::vector back to the certain type
template <typename Type>
Type uchar_to_variable(std::vector<unsigned char> source)
{
  unsigned int size_of = sizeof(Type);

  if(source.size() < size_of || source.size() > size_of)
  {
    Type type = 0;

    return type;
  }

  type_char_convert<Type> fc;
  unsigned int i;

  for(i = 0; i <= size_of - 1; i++)
  {
    fc.c[i] = source.at(i);
  }

  return fc.type;
}

template <typename Variable_T>
bool variable_pop_back(std::vector<unsigned char> &table, Variable_T &variable, uint64_t &offset)
{
  uint64_t temp_offset = offset;

  type_char_convert<Variable_T> convert;

  if((offset + convert.get_size()) > table.size())
  {
    return EXIT_FAILURE;
  }

  for(int i = 0; i < convert.get_size(); i++)
  {
    convert.c[i] = table[temp_offset + i];
  }

  variable = convert.type;

  offset = temp_offset + convert.get_size();

  return EXIT_SUCCESS;
}

template <typename Variable_T>
void variable_push_back(std::vector<unsigned char> &table, Variable_T variable)
{
  type_char_convert<Variable_T> convert;

  convert.type = variable;

  for(int i = 0; i < convert.get_size(); i++)
  {
    table.push_back(convert.c[i]);
  }
}

#endif
