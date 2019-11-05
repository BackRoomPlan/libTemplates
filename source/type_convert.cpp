#include <type_convert.h>

std::vector<unsigned char> vectorFloat_to_vectorUChar(std::vector<float> &source)
{
  if(source.size() == 0)
  {
    return std::vector<unsigned char>();
  }

  unsigned int count_of_vert = (unsigned int)source.size();
  std::vector<unsigned char> buffer;
  unsigned int i;

  for(i = 0; i <= count_of_vert - 1; i++)
  {
    type_char_convert<float> fc;
    fc.type = source.at(i);
    buffer.push_back(fc.c[0]);
    buffer.push_back(fc.c[1]);
    buffer.push_back(fc.c[2]);
    buffer.push_back(fc.c[3]);
  }

  return buffer;
}

std::vector<float> vectorUChar_to_vectorFloat(std::vector<unsigned char> source)
{
  if(source.size() < 4)
  {
    return std::vector<float>();
  }

  unsigned int count_of_vert = (unsigned int)source.size();
  std::vector<float> buffer;
  unsigned int i;

  for(i = 0; i <= count_of_vert - 4; i=i+4)
  {
    type_char_convert<float> fc;
    fc.c[0] = source.at(i);
    fc.c[1] = source.at(i+1);
    fc.c[2] = source.at(i+2);
    fc.c[3] = source.at(i+3);
    buffer.push_back(fc.type);
  }

  return buffer;
}

std::vector<unsigned char> vectorUINT_to_vectorUChar(std::vector<unsigned int> &source)
{
  unsigned int count_of_vert = (unsigned int)source.size();
  std::vector<unsigned char> buffer;
  unsigned int i;

  for(i = 0; i < count_of_vert; i++)
  {
    type_char_convert<unsigned int> fc;
    fc.type = source.at(i);
    buffer.push_back(fc.c[0]);
    buffer.push_back(fc.c[1]);
    buffer.push_back(fc.c[2]);
    buffer.push_back(fc.c[3]);
  }

  return buffer;
}

std::vector<unsigned int> vectorUChar_to_vectorUINT(std::vector<unsigned char> source)
{
  unsigned int count_of_vert = (unsigned int)source.size();
  std::vector<unsigned int> buffer;
  unsigned int i;

  for(i = 0; i < count_of_vert; i=i+4)
  {
    type_char_convert<unsigned int> fc;
    fc.c[0] = source.at(i);
    fc.c[1] = source.at(i+1);
    fc.c[2] = source.at(i+2);
    fc.c[3] = source.at(i+3);
    buffer.push_back(fc.type);
  }

  return buffer;
}
