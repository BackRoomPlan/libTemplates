#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <type_convert.h>

template <typename Vec_T, typename Variable_T>
bool glm_pop_back(std::vector<unsigned char> &buffer, Vec_T &vec, uint64_t &offset)
{
  uint64_t temp_offset = offset;

  for(int i = 0; i < vec.length(); i++)
  {
    if(variable_pop_back<Variable_T>(buffer, vec[i], temp_offset) == EXIT_FAILURE)
    {
      return EXIT_FAILURE;
    }
  }

  offset = temp_offset;
  return EXIT_SUCCESS;
}

template <typename Vec_T, typename Variable_T>
void glm_push_back(std::vector<unsigned char> &buffer, Vec_T &vec)
{
  for(int i = 0; i < vec.length(); i++)
  {
    variable_push_back<Variable_T>(buffer, vec[i]);
  }
}

template <typename Vec_T>
bool glm_is_empty(Vec_T table)
{
  for(int i = 0; i < table.length(); i++)
  {
    if(table[i] != 0.0f)
    {
      return false;
    }
  }

  return true;
}

template <typename Vec_T>
bool glm_is_all_numeric(Vec_T table)
{
  for(int i = 0; i < table.length(); i++)
  {
    if(std::isnan(table[i].x) == true)
    {
      return false;
    }
  }

  return true;
}
