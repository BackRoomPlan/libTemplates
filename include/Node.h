/*
 * License : BSD 3-Clause "New" or "Revised" License
 * File : libTemplates/include/Node.h
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

#ifndef __NODE
#define __NODE

#include <Common_Types.h>

#include <string>
#include <Sync_Table.h>
#include <Common_Functions.h>
#include <Node_Info.h>
#include <BitField.h>
#ifdef _XML_SUPPORT
#include <helpers_tinyxml2.h>
#include <XML_Types.h>
#include <Common_Functions.h>
#include <Base64.h>
#include <Node_Info.h>
#endif

#ifdef _FLOVER_
class Flover;
#endif
struct Node_Info;


template <class T>
class Node
{
public:

	virtual std::string *get_name()
	{
		return &this->info.name;
	}

	virtual std::string get_name_string()
	{
		return this->info.name;
	}

	virtual std::string *get_info()
	{
		return &this->info.info;
	}

	virtual std::string *get_memo()
	{
		return &this->info.memo;
	}

	 /* if is listing item, then return >0
	  * if not, then <=0
	  */
	 virtual int is_listing_item()
	 {
		  return 1;
	}

	Node()
	{
		this->clear_node_variables();
	}

	virtual void runtime_clear()
	{

	}

	virtual ~Node()
	{
		this->clear_node_variables();
	}

	virtual int sanitize()
	{
		this->runtime_clear();
		this->clear();

		return EXIT_SUCCESS;
	}

	virtual void clear()
	{

	}

	virtual bool render_ready()
	{
		return false;
	}

	Type_ID get_id()
	{
		return this->id;
	}

	Type_ID get_privateID()
	{
		return this->privateID;
	}

	Node<T> *get_pointer()
	{
		return this;
	}

	/*
	 * If node uses some informatin, wich is initialy found from depencies, then
	 * this fuction is called, or used
	 *
	 * this is called anyhow when the sync function is called.
	 */
	virtual void sync_node_stuff(Sync_Table &table)
	{
		if(table.content_unsync == false)
		{

		}

	}

#ifdef _XML_SUPPORT
	std::string xml_get(XML_Options_Table &options)
	{
		tinyxml2::XMLPrinter printer;
		this->xml_create(&printer, options);

#ifdef _ZLIB

		if(options.compress_node)
		{
			std::string buffer = printer.CStr();
			printer.ClearBuffer();

			xml_string_compress_print(buffer, &printer, options);
		}

#endif

		return std::string(printer.CStr());
	}

	void xml_get(tinyxml2::XMLPrinter *printer, XML_Options_Table &options)
	{
#ifdef _ZLIB

		if(options.compress_node)
		{
			tinyxml2::XMLPrinter printer_t;
			this->xml_create(&printer_t, options);

			xml_string_compress_print(printer_t.CStr(), printer, options);
		}

		else
		{
#endif
			this->xml_create(printer, options);
#ifdef _ZLIB
		}

#endif
	}

	virtual void xml_create(tinyxml2::XMLPrinter *printer, XML_Options_Table &options)
	{
		printer->OpenElement(this->xml_name.c_str(), options.no_empty_space);

		this->xml_print_privateID(printer, options);
		this->info.xml_create(printer, options);
		this->flags.xml_flags_write(printer, options);
		this->xml_values_write(printer, options);

		printer->CloseElement(options.no_empty_space);
	}

	virtual void xml_values_read(tinyxml2::XMLElement *element)
	{
		if(element != nullptr)
		{
		}
	}

	virtual void xml_values_write(tinyxml2::XMLPrinter *printer, XML_Options_Table &options)
	{
		if(printer != nullptr)
		{
		}

		if(options.no_empty_space)
		{
		}
	}

	int xml_parse(std::string xml_file)
	{

		bool value = EXIT_FAILURE;

		tinyxml2::XMLDocument document;
		document.Parse(xml_file.c_str(), xml_file.size());
		tinyxml2::XMLElement *element = document.RootElement();

		if(element != nullptr)
		{
			value = this->xml_parse(element);
		}

		return value;
	}

	int xml_parse(tinyxml2::XMLElement *element)
	{
		if(element != nullptr)
		{
			if(element->Value() == this->xml_name)
			{
				return this->xml_parse_loop(element);
			}
		}

		return EXIT_FAILURE;
	}

	int xml_parse_privateID(tinyxml2::XMLElement *element)
	{
		if(element == nullptr)
		{
			return EXIT_FAILURE;
		}

		std::string name = element->Value();

		if(name == XML_STRING_PRIVATE XML_STRING_ID)
		{
			this->privateID = variable_read<Type_ID>(element);
			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	void xml_print_privateID(tinyxml2::XMLPrinter *printer, XML_Options_Table &options)
	{
		if(printer == nullptr)
		{
			return void();
		}

		printer->OpenElement(XML_STRING_PRIVATE XML_STRING_ID, options.no_empty_space);
		variable_write<Type_ID>(this->privateID, printer);
		printer->CloseElement(options.no_empty_space);
	}

	virtual int xml_parse_loop(tinyxml2::XMLElement *element)
	{
		if(element == nullptr)
		{
			return EXIT_FAILURE;
		}

		std::string name = element->Value();

		if(name != this->xml_name)
		{
			return EXIT_FAILURE;
		}

		tinyxml2::XMLElement *element_child = element->FirstChildElement();

		while(element_child != nullptr)
		{
			this->xml_parse_privateID(element_child);
			this->info.xml_parse(element_child);
			this->flags.xml_flags_read(element_child);
			this->xml_values_read(element_child);

			element_child = element_child->NextSiblingElement();
		}

		return EXIT_SUCCESS;
	}

	void xml_set_node_infos(std::string node_name)
	{
		this->xml_name = node_name;
	}

#endif

	void clear_node_variables()
	{
		this->privateID = 0;
		this->id = 0;
		this->sqlID = 0;
		this->node_flags.flags_clear();
		this->info.clear();
#ifdef _XML_SUPPORT
		this->xml_name.clear();
		this->xml_name.shrink_to_fit();
#endif
	}

	virtual int sync(Sync_Table &table)
	{
		if(table.dataFiles_read)
		{
		}

		this->sync_node_stuff(table);

		return EXIT_SUCCESS;
	}

	virtual int unsync(Sync_Table &table)
	{
		return EXIT_SUCCESS;
	}

	virtual void set_not_current()
	{

	}

	virtual void set_is_current()
	{

	}

	void set_needed(bool value)
	{
		this->node_flags.bit_set(Node_Flags::Needed, value);

		this->set_unneeded_content(value);
	}

	virtual void set_unneeded_content(bool value)
	{
		if(value)
		{
			return void();
		}
	}

	virtual void delete_unneeded_content()
	{
	}
#ifdef _QT_GUI_
	/*
	 * for QT_table_header() and QT_table_line()
	 * mode_long == false, prints only needed information
	 * mode_long, prints all informations
	 */
	virtual void QT_table_header(bool mode_long = false)
	{
		if(mode_long)
		{

		}
	}

	virtual void QT_table_line(bool mode_long = false)
	{
		if(mode_long)
		{

		}
	}
#endif
	Type_ID id;
	Type_ID privateID;
	Type_ID sqlID;
	_BitField node_flags;
	_BitField flags;


	T *prev;
	T *next;

	Node_Info info;

#ifdef _XML_SUPPORT
protected:
	std::string xml_name;
#endif

};
#endif
