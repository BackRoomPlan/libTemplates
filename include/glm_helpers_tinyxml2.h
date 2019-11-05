#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <Sync_Table.h>
#include <tinyxml2.h>

#ifdef _FLOVER_
#include <Common/Move_View.h>
#endif

template <class GLM_Type, typename Type>
GLM_Type glm_vec_read(tinyxml2::XMLElement *element)
{
  std::vector<unsigned char> buffer;
  buffer = base64Decode(get_std_string(element));

  if(buffer.size() < 8 || buffer.size() > 8)
  {
    return GLM_Type();
  }

  GLM_Type temp;
  uint64_t offset = 0;

  glm_pop_back<GLM_Type, Type>(buffer, temp, offset);

  return temp;
}


template <class GLM_Type, typename Type>
void glm_vec_write(GLM_Type vector, tinyxml2::XMLPrinter *printer)
{
  if(glm_is_empty(vector) == false)
  {
    std::vector<unsigned char> buffer;

    glm_push_back<GLM_Type, Type>(buffer, vector);

    printer->PushText(std::string(base64Encode(buffer)).c_str());
  }
}

#ifdef _FLOVER_
void dimensions_write(glm::vec3 *dimensions_min, glm::vec3 *dimensions_max, tinyxml2::XMLPrinter *printer);
void dimensions_read(tinyxml2::XMLElement *element, glm::vec3 *dimensions_min, glm::vec3 *dimensions_max);

void model_position_read(tinyxml2::XMLElement *element, model_position *mPosition);
void model_position_write(model_position *mPosition, tinyxml2::XMLPrinter *printer, XML_Options_Table options);
#endif
