/*
 * License : BSD 3-Clause "New" or "Revised" License
 * File : libTemplates/source/Node_Info.cpp
 *
 * #Copyright 2023 Mika Manninen <mika.mannin@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *    (1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 *    (2) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.  
 *    
 *    (3)The name of the author may not be used to
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE. 
 */

#include <Common_Types.h>

#include <string>
#include <Sync_Table.h>

#ifdef _XML_SUPPORT
#include <helpers_tinyxml2.h>
#include <XML_Types.h>
#include <Base64.h>
#endif
#include <Common_Functions.h>

#include <Node_Info.h>

void Node_Info :: clear()
{
	this->info.clear();
	this->info.shrink_to_fit();
	this->memo.clear();
	this->memo.shrink_to_fit();
	this->name.clear();
	this->name.shrink_to_fit();
}

void Node_Info :: copy_to(Node_Info *to)
{
	to->info = this->info;
	to->memo = this->memo;
	to->name = this->name;
}

void Node_Info :: copy_from(Node_Info *from)
{
	this->info = from->info;
	this->memo = from->memo;
	this->name = from->name;
}


int Node_Info :: xml_parse(tinyxml2::XMLElement *element)
{
	std::string name = element->Value();

	if(name != XML_STRING_NODE XML_STRING_INFO)
	{
		return EXIT_FAILURE;
	}

	tinyxml2::XMLElement *element_child = element->FirstChildElement();

	while(element_child != nullptr)
	{
		name = element_child->Value();

		if(name == XML_STRING_NAME)
		{
			this->name = Base64_get_string(get_std_string(element_child));
		}

		else if(name == XML_STRING_INFO)
		{
			this->info = Base64_get_string(get_std_string(element_child));
		}

		else if(name == XML_STRING_MEMO)
		{
			this->memo = Base64_get_string(get_std_string(element_child));
		}

		name.clear();
		element_child = element_child->NextSiblingElement();
	}

	return EXIT_SUCCESS;
}

void Node_Info :: xml_create(tinyxml2::XMLPrinter *printer, XML_Options_Table &options)
{
	printer->OpenElement(XML_STRING_NODE XML_STRING_INFO, options.no_empty_space);

	if(!this->info.empty())
	{
		printer->OpenElement(XML_STRING_INFO, options.no_empty_space);
		printer->PushText(Base64_encode_string(this->info).c_str());
		printer->CloseElement(options.no_empty_space);
	}

	if(!this->name.empty())
	{
		printer->OpenElement(XML_STRING_NAME, options.no_empty_space);
		printer->PushText(Base64_encode_string(this->name).c_str());
		printer->CloseElement(options.no_empty_space);
	}

	if(!this->memo.empty())
	{
		printer->OpenElement(XML_STRING_MEMO, options.no_empty_space);
		printer->PushText(Base64_encode_string(this->memo).c_str());
		printer->CloseElement(options.no_empty_space);
	}

	printer->CloseElement(options.no_empty_space);
}
