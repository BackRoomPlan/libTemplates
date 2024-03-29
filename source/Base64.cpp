#include <string>
#include <vector>
#include <stdexcept>
#include <Base64.h>

std::basic_string<char> base64Encode(std::vector<unsigned char> inputBuffer)
{
  if(inputBuffer.size() == 0)
  {
    return std::basic_string<char>();
  }

  std::basic_string<char> encodedString;
  encodedString.reserve(((inputBuffer.size()/3) + (inputBuffer.size() % 3 > 0)) * 4);
  long temp;
  std::vector<unsigned char>::iterator cursor = inputBuffer.begin();

  for(size_t idx = 0; idx < inputBuffer.size()/3; idx++)
  {
    temp  = (*cursor++) << 16; //Convert to big endian
    temp += (*cursor++) << 8;
    temp += (*cursor++);
    encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
    encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
    encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
    encodedString.append(1,encodeLookup[(temp & 0x0000003F)      ]);
  }

  switch(inputBuffer.size() % 3)
  {
  case 1:
    temp  = (*cursor++) << 16; //Convert to big endian
    encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
    encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
    encodedString.append(2,padCharacter);
    break;

  case 2:
    temp  = (*cursor++) << 16; //Convert to big endian
    temp += (*cursor++) << 8;
    encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
    encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
    encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
    encodedString.append(1,padCharacter);
    break;
  }

  return encodedString;
}

std::vector<unsigned char> base64Decode(const std::basic_string<char> input)
{
  if(input.length() % 4)  //Sanity check
    return std::vector<unsigned char>(0);

  size_t padding = 0;

  if(input.length())
  {
    if(input[input.length()-1] == padCharacter)
      padding++;

    if(input[input.length()-2] == padCharacter)
      padding++;
  }

  //Setup a vector to hold the result
  std::vector<unsigned char> decodedBytes;
  decodedBytes.reserve(((input.length()/4)*3) - padding);
  long temp=0; //Holds decoded quanta
  std::basic_string<char>::const_iterator cursor = input.begin();

  while(cursor < input.end())
  {
    for(size_t quantumPosition = 0; quantumPosition < 4; quantumPosition++)
    {
      temp <<= 6;

      if(*cursor >= 0x41 && *cursor <= 0x5A)        // This area will need tweaking if
        temp |= *cursor - 0x41;                 // you are using an alternate alphabet

      else if(*cursor >= 0x61 && *cursor <= 0x7A)
        temp |= *cursor - 0x47;

      else if(*cursor >= 0x30 && *cursor <= 0x39)
        temp |= *cursor + 0x04;

      else if(*cursor == 0x2B)
        temp |= 0x3E; //change to 0x2D for URL alphabet

      else if(*cursor == 0x2F)
        temp |= 0x3F; //change to 0x5F for URL alphabet

      else if(*cursor == padCharacter)   //pad
      {
        switch(input.end() - cursor)
        {
        case 1: //One pad character
          decodedBytes.push_back((temp >> 16) & 0x000000FF);
          decodedBytes.push_back((temp >> 8) & 0x000000FF);
          return decodedBytes;

        case 2: //Two pad characters
          decodedBytes.push_back((temp >> 10) & 0x000000FF);
          return decodedBytes;

        default:
          return std::vector<unsigned char>(0);
        }
      }

      else
      {
        return std::vector<unsigned char>(0);
      }

      cursor++;
    }

    decodedBytes.push_back((temp >> 16) & 0x000000FF);
    decodedBytes.push_back((temp >> 8) & 0x000000FF);
    decodedBytes.push_back((temp) & 0x000000FF);
  }

  return decodedBytes;
}

std::string Base64_get_string(const std::basic_string<char> input)
{
  std::vector<unsigned char> buffer = base64Decode(input);
  return std::string(buffer.begin(), buffer.end());
}

std::string Base64_encode_string(std::string input)
{
  std::vector<unsigned char> buffer;
  std::copy(input.begin(), input.end(), std::back_inserter(buffer));
  return base64Encode(buffer);
}
