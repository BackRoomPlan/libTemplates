/*
 * License : BSD 3-Clause "New" or "Revised" License
 * File : libTemplates/include/Manager.h
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

#define __MANAGER

#include <Node.h>
#include <cstdlib>

#ifdef _SQL_DATABASE
#include <SQL/SQL_Database.h>
#endif

#include <File.h>
#include <XML_Types.h>
#include <string>
#include <Common_Functions.h>
#include <Base64.h>
#include <Sync_Table.h>
#include <Node_Info.h>

#ifndef ANDROID
#if __has_include(<filesystem>)
  #include <filesystem>
  namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
  #include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#elif __has_include(<boost/filesystem.hpp>)
	include <boost/filesystem.hpp>
#else
  error "Missing the <filesystem> header."
#endif
#else
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

#ifdef _FLOVER_
#include <Flover.h>
#include <Options.h>
#endif

#ifdef _DEBUG
#include <iostream>
#endif



template<class T>
class Manager
{
public:

	Manager()
	{
#ifdef _FLOVER_
		this->files = nullptr;
		this->flover = nullptr;
#endif

		this->is_child_manager = false;
		this->all_files_read = false;
		this->manager_init();
	}

	void manager_init()
	{
		this->nextID = 0;
		this->counter = 0;
		this->remove_unneeded = false;
		this->clear_pointers();
		this->clear_current_values();

#ifdef _FLOVER_

		if(this->files != nullptr)
		{
			delete this->files;
			this->files = nullptr;
		}

#endif

		this->xml_set_settings();
	}

	virtual ~Manager()
	{
		this->clear();

		this->delete_nodes();
		this->clear_pointers();
		this->clear_current_values();
		this->all_files_read = false;
		this->remove_unneeded = false;

#ifdef _FLOVER
		this->is_child_manager = false;
#endif
	}

	/*
	 * get pointer of node by counting from first to last
	 */
	T *get_pointer_count_from_first(Type_ID count)
	{
		Type_ID temp_count = 0;

		T *temp = this->first;

		while(temp != nullptr)
		{
			if(temp_count == count)
			{
				return temp;
			}

			temp_count++;
			temp = temp->next;

		}

		return nullptr;
	}

	void janitor_tick()
	{
		T *temp = nullptr;

		while(this->nodes_to_sync.empty() == false)
		{
			temp = this->nodes_to_sync.back();
			this->nodes_to_sync.pop_back();

			if(this->node_exists(temp))
			{
				temp->sync(this->flover->sync_table);
			}
		}

		temp = nullptr;

		while(this->nodes_to_delete.empty() == false)
		{
			temp = this->nodes_to_delete.back();
			this->nodes_to_delete.pop_back();

			if(this->node_exists(temp))
			{
				temp->sanitize();
				this->del(temp);
			}
		}

	}

	Type_ID get_ID_count_from_first(Type_ID count)
	{
		Type_ID temp_count = 0;

		T *temp = this->first;

		while(temp != nullptr)
		{
			if(temp_count == count)
			{
				return temp->id;
			}

			temp_count++;
			temp = temp->next;

		}

		return 0;
	}

	T *get_pointer_of(Type_ID id, bool use_privateID)
	{
		if(use_privateID)
		{
			return this->get_pointer_of_privateID(id);
		}

		return this->get_pointer_of_id(id);
	}

	Type_ID get_privateID_count_from_first(Type_ID count)
	{
		Type_ID temp_count = 0;

		T *temp = this->first;

		while(temp != nullptr)
		{
			if(temp_count == count)
			{
				return temp->privateID;
			}

			temp_count++;
			temp = temp->next;

		}

		return 0;
	}

	/*
	 * same as "get_count_from_fist(), but in reverse,
	 * by counting from last to first node of the manager's node listing
	 */
	T *get_pointer_count_from_last(Type_ID count)
	{
		Type_ID temp_count = 0;

		T *temp = this->last;

		while(temp != nullptr)
		{
			if(temp_count == count)
			{
				return temp;
			}

			temp_count++;
			temp = temp->prev;

		}

		return nullptr;
	}

	int set_current_by_id(Type_ID id, bool use_privateID)
	{
		if(use_privateID)
		{
			return this->set_current_by_privateID(id);
		}

		return this->set_current_by_id(id);
	}

	Type_ID get_id_of_next(Type_ID id, bool include_non_listing_nodes, bool use_privateID)
	{
		if(use_privateID)
		{
			return this->get_privateID_of_next(id, include_non_listing_nodes);
		}

		return this->get_id_of_next(id, include_non_listing_nodes);
	}

	Type_ID get_id_of_prev(Type_ID id,  bool include_non_listing_nodes,  bool use_privateID)
	{
		if(use_privateID)
		{
			return this->get_privateID_of_prev(id, include_non_listing_nodes);
		}

		return this->get_id_of_prev(id, include_non_listing_nodes);
	}

	Type_ID get_privateID_of_next(Type_ID id,  bool include_non_listing_nodes = false)
	{
		T *temp = this->_get_pointer_of_privateID(id);

		if(temp != nullptr)
		{
			if(temp->next != nullptr)
			{
				if(include_non_listing_nodes)
				{
					return temp->next->privateID;
				}

				else
				{
					T *_temp = temp->next;

					while(_temp != nullptr)
					{
						if(_temp->is_listing_item())
						{
							return temp->privateID;
						}

						temp = temp->next;
					}
				}
			}
		}

		return 0;
	}

	Type_ID get_privateID_of_prev(Type_ID id,  bool include_non_listing_nodes = false)
	{
		T *temp = this->_get_pointer_of_privateID(id);

		if(temp != nullptr)
		{
			if(temp->prev != nullptr)
			{
				if(include_non_listing_nodes)
				{
					return temp->prev->privateID;
				}

				else
				{
					T *_temp = temp->prev;

					while(_temp != nullptr)
					{
						if(_temp->is_listing_item())
						{
							return temp->privateID;
						}

						temp = temp->prev;
					}
				}
			}
		}

		return 0;
	}

	Type_ID get_id_of_next(Type_ID id,  bool include_non_listing_nodes = false)
	{
		T *temp = this->get_pointer_of_id(id);

		if(temp != nullptr)
		{
			if(temp->next != nullptr)
			{
				if(include_non_listing_nodes)
				{
					return temp->next->id;
				}

				else
				{
					T *_temp = temp->next;

					while(_temp != nullptr)
					{
						if(_temp->is_listing_item())
						{
							return temp->id;
						}

						temp = temp->next;
					}
				}
			}
		}

		return 0;
	}

	Type_ID get_id_of_prev(Type_ID id,  bool include_non_listing_nodes = false)
	{
		T *temp = this->get_pointer_of_id(id);

		if(temp != nullptr)
		{
			if(temp->prev != nullptr)
			{
				if(include_non_listing_nodes)
				{
					return temp->prev->id;
				}

				else
				{
					T *_temp = temp->prev;

					while(_temp != nullptr)
					{
						if(_temp->is_listing_item())
						{
							return temp->id;
						}

						temp = temp->prev;
					}
				}
			}
		}

		return 0;
	}

	virtual void clear()
	{
	}

	void clear_pointers()
	{
		this->first = nullptr;
		this->last = nullptr;
	}

	int delete_current( bool delete_file = false)
	{
		return this->del(this->current, delete_file);
	}

	void clear_current()
	{
		if(this->current != nullptr)
		{
			this->current_changed = true;
		}

		this->current = nullptr;
	}

	void clear_current_values()
	{
		this->current_prev = nullptr;
		this->current = nullptr;
		this->current_changed = false;
	}

	Type_ID get_next_id()
	{
		this->nextID = nextID + 1;
		return nextID;
	}

	Type_ID get_privateID_of_next(T *node)
	{
		if(this->node_exists(node) == EXIT_FAILURE)
		{
			if(this->first != nullptr)
			{
				return this->first->privateID;
			}
		}

		else if(node->next != nullptr)
		{
			return node->next->privateID;
		}

		return 0;
	}

	Type_ID get_privateID_of_prev(T *node)
	{
		if(this->node_exists(node) == EXIT_FAILURE)
		{
			if(this->last != nullptr)
			{
				return this->last->privateID;
			}
		}

		else if(node->prev != nullptr)
		{
			return node->prev->privateID;
		}

		return 0;
	}

	Type_ID seek_next_ongoing_privateID()
	{
		Type_ID highest_privateID = 0;
		T *node = this->first;

		while(node != nullptr)
		{
			if(node->privateID > highest_privateID)
			{
				highest_privateID = node->privateID;
			}

			node = node->next;
		}

		highest_privateID++; // Adding one to the highest ID to get next really free ID;
		return highest_privateID;
	}

	Type_ID get_privateID_of_id( Type_ID id)
	{
		if(id != 0)
		{
			T *temp = this->first;

			while(temp != nullptr)
			{
				if(temp->id == id)
				{
					return temp->privateID;
				}

				temp = temp->next;
			}
		}

		return 0;
	}

	T *seek_next_listing_node(T *node)
	{
		T *temp;
		temp = node;

		if(temp != nullptr)
		{
			if(temp->next != nullptr)
			{
				temp = temp->next;
			}
		}

		if(temp == nullptr)
		{
			temp = this->first;
		}

		while(temp != nullptr)
		{
			if(temp->is_listing_item())
			{
				return temp;
			}

			temp = temp->next;
		}

		return nullptr;
	}

	T *seek_prev_listing_node(T *node)
	{
		T *temp;
		temp = node;

		if(temp != nullptr)
		{
			if(temp->prev != nullptr)
			{
				temp = temp->prev;
			}
		}

		if(temp == nullptr)
		{
			temp = this->last;
		}

		while(temp != nullptr)
		{
			if(temp->is_listing_item())
			{
				return temp;
			}

			temp = temp->prev;
		}

		return nullptr;
	}

	T *_get_pointer_of_privateID(Type_ID id)
	{
		if(id != 0)
		{
			T *temp = this->first;

			while(temp != nullptr)
			{
				if(temp->privateID == id)
				{
					return temp;
				}

				temp = temp->next;
			}
		}

		return nullptr;
	}

	T *get_pointer_of_privateID(Type_ID id,  bool in_memory_only = false)
	{
		if(id != 0)
		{

			if(in_memory_only)
			{
				return this->_get_pointer_of_privateID(id);
			}

			if(in_memory_only == false){
				T *temp = this->_get_pointer_of_privateID(id);

				if(temp == nullptr)
				{
					temp = this->load_file(id);
				}

				/*
						  if(temp != nullptr && errorList != nullptr)
						  {
							 errorList->from_xml = true;
						  }
						  */


#ifdef _SQL_DATABASE

				if(temp == nullptr &&
		#ifdef _FLOVER_
						this->flover->options->database_location == Data_Location::SQL)
#else
						this->flover->sync_table.target_sql)
#endif
				{
					temp = this->database_get_by_privateID(id);
				}

#endif

				if(temp != nullptr)
				{
					temp->sync(this->flover->sync_table);
				}

				return temp;
			}
		}

		return nullptr;
	}

	T *get_by_filename(std::string filename)
	{
		std::string buffer = file_read_text(this->flover->sync_table, filename);

		if(buffer.empty() == false)
		{
			T *node = this->create();

			if(node->xml_parse(buffer) == EXIT_SUCCESS)
			{
				return node;
			}

			else
			{
				this->del(node);
			}
		}

		return nullptr;
	}

	T *get_pointer_of_id( Type_ID id)
	{
		T *temp = this->first;

		while(temp != nullptr)
		{
			if(temp->id == id)
			{
				return temp;
			}

			temp = temp->next;
		}

		return nullptr;
	}

	T *get_pointer_of_current()
	{

		if(this->current != nullptr)
		{
			if(this->node_exists(this->current) == EXIT_SUCCESS)
			{
				return this->current;
			}
		}

		return nullptr;
	}

	virtual T *get_pointer_of_file( Type_ID id)
	{
		if(id ==0)
		{}

		return nullptr;
	}

	int del(T *node,  bool delete_file = false)
	{
		if(node == nullptr)
		{
			return EXIT_FAILURE;
		}

		if(this->node_exists(node) == EXIT_FAILURE)
		{
			return EXIT_FAILURE;
		}

		if(delete_file)
		{
			this->xml_file_delete_by_privateID(node->privateID);
		}

		node->sanitize();

		// this->database_delete_by_privateID(node->sqlID);
		return this->_del(node);
	}

	int _del(T *node)
	{
		T *temp_find = this->first;

		while(temp_find != nullptr)
		{
			if(node == temp_find)
			{
				T *temp_prev = nullptr;
				T *temp_next = nullptr;
				T *temp = temp_find;

				if(temp->prev != nullptr && temp->next != nullptr)
				{
					temp_prev = temp->prev;
					temp_next = temp->next;
					temp_prev->next = temp_next;
					temp_next->prev = temp_prev;
				}

				else if(temp->prev == nullptr && temp->next != nullptr)
				{
					temp->next->prev = nullptr;
					this->first = temp->next;
					temp->next = nullptr;
				}

				else if(temp->prev != nullptr && temp->next == nullptr)
				{
					this->last = temp->prev;
					this->last->next = nullptr;
				}

				else
				{
					this->first = nullptr;
					this->last = nullptr;
				}

				this->counter--;
				delete temp;
				return EXIT_SUCCESS;
			}

			temp_find = temp_find->next;
		}

		return EXIT_FAILURE;
	}

	T *load_file(Type_ID privateID)
	{
		std::string filename;

#ifdef _FLOVER_
		filename = this->flover->options->get_xml_path() + this->xml_node_path + XML_STRING_SLASH + this->xml_node_name + XML_STRING_UNDERSCORE + base64Encode(variable_to_uchar<Type_ID>(privateID)) + XML_STRING_FILENAME_EXTENSION_XML;
#else
		filename = this->xml_node_path + XML_STRING_SLASH + this->xml_node_name + XML_STRING_UNDERSCORE + base64Encode(variable_to_uchar<Type_ID>(privateID)) + XML_STRING_FILENAME_EXTENSION_XML;
#endif

		return this->xml_node_parse(file_read_text(this->flover->sync_table, filename));
	}

	T *create(bool set_privateID = false)
	{
		T *new_node = new T(this->get_next_id());
		this->add_to_last(new_node);
		this->counter++;

		if(set_privateID)
		{
			new_node->privateID = this->seek_next_ongoing_privateID();
		}

		give_pointers(new_node);

		new_node->node_flags.bit_set(Node_Flags::Needed, true);
		new_node->node_flags.bit_set(Node_Flags::To_Delete, false);

		this->xml_set_node_settings(new_node);

		return new_node;
	}

	T *create_no_id()
	{
		T *new_object = new T(0);
		this->counter++;
		give_pointers(new_object);
		this->add_to_last(new_object);
		return new_object;
	}

	virtual void give_pointers(T *node)
	{
		if(node != nullptr)
		{
			return void();
		}
	}

	int delete_nodes()
	{
		T *temp = nullptr;
		T *temp_delete = nullptr;
		temp = this->first;

		if(temp != nullptr)
		{
			while(temp != nullptr)
			{
				temp_delete = temp;

#ifdef _FLOVER_
				temp_delete->unsync(this->flover->sync_table);
#endif

				temp = temp->next;
				delete temp_delete;
			}

			this->counter = 0;
			this->first = nullptr;
			this->last = nullptr;

			this->all_files_read = false;
			this->clear_current_values();

			return EXIT_SUCCESS;
		}


		return EXIT_FAILURE;
	}

	int delete_node_by_privateID(Type_ID id,  bool delete_file = false)
	{
		return this->del(this->_get_pointer_of_privateID(id), delete_file);
	}

	int delete_node_by_id( Type_ID id,  bool delete_file = false)
	{
		return this->del(this->get_pointer_of_id(id), delete_file);
	}

	void add_to_last(T *node)
	{
		if(this->first == nullptr && this->last == nullptr)
		{
			this->first = node;
			this->last = node;
			node->prev = nullptr;
			node->next = nullptr;
		}

		else
		{
			node->prev = this->last;
			this->last->next = node;
			this->last = this->last->next;
			this->last->next = nullptr;
		}
	}

	virtual void set_nodes_to_unneeded(bool value = false)
	{
		T *temp = this->first;

		while(temp != nullptr)
		{
			temp->set_needed(value);
			temp = temp->next;
		}
	}

	virtual void delete_unneeded()
	{
		T *temp = this->first;

		if(temp != nullptr)
		{
			while(temp != nullptr)
			{
				if(temp->node_flags.bit_get(Node_Flags::Needed) == false ||
						temp->node_flags.bit_get(Node_Flags::Delete_Node))
				{
					T *temp_delete = temp;
					temp->runtime_clear();
					temp = temp->next;

					this->_del(temp_delete);
				}

				else
				{
					temp->delete_unneeded_content();
					temp = temp->next;
				}
			}

			this->all_files_read = false;
		}
	}

	virtual int unsync_manager(Sync_Table &table)
	{
		T *node = this->first;

		if(node != nullptr)
		{
			while(node != nullptr)
			{
				node->unsync(table);
				node = node->next;
			}

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	virtual int unsync_manager()
	{
		T *node = this->first;

		if(node != nullptr)
		{
			while(node != nullptr)
			{
				node->unsync(this->flover->sync_table);
				node = node->next;
			}

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	int sync_all(Sync_Table &table)
	{
		T *node = this->first;

		if(node != nullptr)
		{
			while(node != nullptr)
			{
				node->sync(table);
				node = node->next;
			}

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	int sync_all()
	{
		T *node = this->first;

		if(node != nullptr)
		{
			while(node != nullptr)
			{
				node->sync(this->flover->sync_table);
				node = node->next;
			}

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	int node_exists(T *node)
	{
		if(node != nullptr)
		{
			T *temp = this->first;

			while(temp != nullptr)
			{
				if(node == temp)
				{
					return EXIT_SUCCESS;
				}

				temp = temp->next;
			}
		}

		return EXIT_FAILURE;
	}

#ifdef _SQL_DATABASE
	int database_delete(T *node)
	{
#ifdef _FLOVER_
		if(this->flover->sql_database != nullptr)
		{
			if(this->flover->sql_database->connected)
			{
				if(this->flover->sql_database->make_query(this->query_create_delete_by_id(node->sqlID)) == EXIT_SUCCESS)
				{
					return EXIT_SUCCESS;
				}
			}
		}
#endif
		return EXIT_FAILURE;
	}

	int database_delete_by_privateID( Type_ID privateID)
	{
		if(this->sql_database != nullptr)
		{
			if(this->sql_database->connected)
			{
				return this->sql_database->make_query(this->query_create_delete_by_privateID(privateID));
			}
		}

		return EXIT_FAILURE;
	}

	int database_save(T *node)
	{
#ifdef _FLOVER_
		if(this->flover->sql_database != nullptr)
		{
			if(this->flover->sql_database->connected)
			{
				if(this->flover->sql_database->make_query(this->query_create_save(node)) == EXIT_SUCCESS)
				{
					return EXIT_SUCCESS;
				}
			}
		}
#endif

		return EXIT_FAILURE;
	}

	int database_update(T *node)
	{
		if(this->sql_database != nullptr)
		{
			if(this->sql_database->connected)
			{
				if(this->sql_database->make_query(this->query_create_update(node)) == EXIT_SUCCESS)
				{
					return EXIT_SUCCESS;
				}
			}
		}

		return EXIT_FAILURE;
	}

	T *database_get_by_privateID( Type_ID id)
	{
#ifdef _FLOVER_
		if(this->flover->sql_database != nullptr)
		{
			if(this->flover->sql_database->connected)
			{
				if(id != 0)
				{
					//return this->get_node_from_query(this->sql_database->make_query(this->query_create_get_by_id(id)));

					std::string query = this->query_create_get_by_privateID(id);

					std::string data = this->flover->sql_database->make_query_get(query);

					if(data.empty())
					{
						return nullptr;
					}

					T *node = this->create();

					node->xml_parse(Base64_get_string(data));
				}
			}
		}
#endif

		return nullptr;
	}

	/*
	  T* database_get_by_privateID( Type_ID privateID)
	  {
		 if(this->sql_database != nullptr)
		 {
			if(this->sql_database->connected)
			{
			  if(this->sql_database->make_query(this->query_create_get_by_privateID()) == EXIT_SUCCESS)
			  {
				 MYSQL_RES* result =mysql_store_result(this->sql_database->connection);
				 if(result != nullptr)
				 {
					T *node = this->get_node_from_query(result);
					mysql_free_result(result);
					return node;
				 }
			  }
			}
		 }

		 return nullptr;
	  }
	*/

	int database_get_all()
	{
#ifdef _FLOVER_
		if(this->flover->sql_database != nullptr)
		{
			if(this->flover->sql_database->connected)
			{
				/*
				if(this->sql_database->make_query(this->query_create_get_all()) == EXIT_SUCCESS)
				{
				  return this->nodes_from_query(mysql_store_result(this->sql_database->connection));
				}
				*/
			}
		}
#endif

		return EXIT_FAILURE;
	}

	std::string query_create_get_by_privateID( Type_ID privateID)
	{
		if(privateID == 0)
		{
			return std::string();
		}

		std::string query;
		query = "select * from "+ this->xml_node_path + " where privateID = " + base64Encode(variable_to_uchar<unsigned int>(privateID));

		return query;
	}


	std::string query_create_delete_by_privateID( Type_ID id)
	{
		if(id != 0)
		{
		}

		return std::string();
	}

	std::string query_create_save(T *node)
	{
		if(node != nullptr)
		{
		}

		return std::string();
	}

	std::string query_create_update(T *node)
	{
		if(node != nullptr)
		{
		}

		return std::string();
	}

	std::string query_create_get_all()
	{
		return std::string();
	}
#endif

	std::string get_node_name()
	{
		return this->xml_node_name;
	}

	std::string get_node_path()
	{
		return this->xml_node_path;
	}

	int xml_files_read()
	{
		if(this->flover->sync_table.delete_on_memory_present)
		{
			this->delete_nodes();
		}

		this->all_files_read = true;

		std::string data_path =
#ifdef _FLOVER_
				this->flover->options->get_xml_path(this->flover->sync_table.sql_push) +
#endif
				this->get_node_path();
#ifndef ANDROID

		if(file_exits(data_path) == EXIT_FAILURE)
		{
			return EXIT_FAILURE;
		}

#ifdef __WASM__

		for(auto &p : std::__fs::filesystem::directory_iterator(data_path))
#else
		for(auto &p : std::filesystem::directory_iterator(data_path))
#endif
		{
			std::string name;
			name = p.path().string();

			if(name.find(std::string(".xml")) !=std::string::npos)
			{
				std::string buffer = file_read_text(this->flover->sync_table, name);

				if(buffer.empty() == false)
				{
					this->xml_node_parse(buffer);
				}
			}
		}

		return EXIT_SUCCESS;
#else
		AAssetDir *dir = AAssetManager_openDir(this->flover->options->engine->app->activity->assetManager, data_path.c_str());

		if(dir != nullptr)
		{
			std::string name;
			 char *c_name;
			c_name = AAssetDir_getNextFileName(dir);

			while(c_name != nullptr)
			{
				name = c_name;
				std::string file_path = data_path + XML_STRING_SLASH + name;

				this->xml_node_parse(file_read_text(this->flover->sync_table, file_path));


				name.clear();
				c_name = AAssetDir_getNextFileName(dir);
			}

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
#endif
	}

	int xml_file_listing_read()
	{
		std::string data_path;
#ifdef _FLOVER_
		data_path = this->flover->options->get_xml_path() + this->get_node_path();
#else
		data_path = this->get_node_path();
#endif
		/*
			 if(file_exits(data_path, this->flover->options) == EXIT_FAILURE)
			 {
				return EXIT_FAILURE;
			 }
		*/
		std::string path = data_path + XML_STRING_SLASH + this->xml_node_name + XML_STRING_UNDERSCORE XML_STRING_LISTING XML_STRING_FILENAME_EXTENSION_XML;

		return this->xml_listing_read(file_read_text(this->flover->sync_table, path));
	}

	int xml_files_write()
	{
#ifdef _SQL_DATABASE
#ifdef _FLOVER_
		if(this->flover->options->database_target == Data_Location::SQL)
		{
			if(this->flover->sql_database == nullptr)
			{
				return EXIT_FAILURE;
			}
		}
#endif
#endif

		T *node = this->first;

		if(node != nullptr)
		{
#ifdef _FLOVER_
			if(this->flover->options->database_target != Data_Location::SQL)
#else
			if(this->flover->sync_table.target_sql)
#endif
			{
				std::string directory = this->get_node_path();
#ifdef _FLOVER_
				std::string data_path;

				if(this->flover->xml_options.target_dataFinal == false)
				{
					data_path = this->flover->options->get_xml_path();
				}

				else
				{
					data_path = this->flover->options->get_dataFinal_path();
				}

				if(directory_exits_create(data_path + directory) == EXIT_FAILURE)
#else
				if(directory_exits_create(directory) == EXIT_FAILURE)
#endif
				{
					return EXIT_FAILURE;
				}

				do
				{
					if(node->privateID != 0)
					{
						std::string path =
#ifdef _FLOVER_
								data_path +
#endif
								directory + XML_STRING_SLASH + this->xml_node_name + XML_STRING_UNDERSCORE + base64Encode(variable_to_uchar<Type_ID>(node->privateID)) + XML_STRING_FILENAME_EXTENSION_XML;
						std::string xml_file = node->xml_get(this->flover->xml_options);

						file_write_text(path, xml_file);
					}

					node = node->next;
				}
				while(node != nullptr);

			}

#ifdef _SQL_DATABASE

			else
			{

				do
				{
					if(node->privateID != 0)
					{
						this->database_save(node);
					}

					node = node->next;
				}
				while(node != nullptr);
			}

#endif

			EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	void create_new_privateIDs(bool write_xml_files)
	{
		Type_ID temp_id = 1;

		T *node = this->first;

		while(node != nullptr)
		{
			node->privateID = temp_id;
			temp_id++;

			node = node->next;
		}

		if(write_xml_files)
		{
			this->xml_files_write();
		}
	}

	int xml_file_write_by_pointer(T *node)
	{
		if(this->node_exists(node) == EXIT_SUCCESS)
		{
#ifdef _FLOVER_
			std::string data_path = this->flover->options->get_xml_path() + this->xml_node_path;

			if(directory_exits_create(data_path) == EXIT_FAILURE)
			{
				return EXIT_FAILURE;
			}

			std::string path = data_path + XML_STRING_SLASH + this->xml_node_name + XML_STRING_UNDERSCORE + base64Encode(variable_to_uchar<Type_ID>(node->privateID)) + XML_STRING_FILENAME_EXTENSION_XML;
#else
			std::string path = this->xml_node_path + XML_STRING_SLASH + this->xml_node_name + XML_STRING_UNDERSCORE + base64Encode(variable_to_uchar<Type_ID>(node->privateID)) + XML_STRING_FILENAME_EXTENSION_XML;
#endif
			std::string xml_file;

			tinyxml2::XMLPrinter printer;
			node->xml_create(&printer, this->flover->xml_options);

#ifdef _ZLIB

			if(this->flover->xml_options.compress_node)
			{
				std::string buffer = printer.CStr();
				printer.ClearBuffer();
				xml_string_compress_print(buffer, &printer, this->flover->xml_options);
			}

#endif

			xml_file = printer.CStr();
			return file_write_text(path, xml_file);
		}

		return EXIT_FAILURE;
	}

	int xml_listing_file_write()
	{
#ifdef _FLOVER_
		if(this->flover == nullptr)
		{
			return EXIT_FAILURE;
		}
#endif

		std::string data_path;
#ifdef _FLOVER_
		if(this->flover->xml_options.target_dataFinal)
		{
			data_path = this->flover->options->get_dataFinal_path() + this->xml_node_path;
		}

		else
		{
			data_path = this->flover->options->get_xml_path() + this->xml_node_path;
		}
#else
		data_path = this->xml_node_path;
#endif

		if(directory_exits_create(data_path) == EXIT_FAILURE)
		{
			return EXIT_FAILURE;
		}

		std::string path = data_path + XML_STRING_SLASH + this->xml_node_name + XML_STRING_UNDERSCORE XML_STRING_LISTING XML_STRING_FILENAME_EXTENSION_XML;
		std::string xml_file;
		tinyxml2::XMLPrinter printer;
		this->xml_listing_write(&printer);
		xml_file = printer.CStr();
		return file_write_text(path, xml_file);
	}

	int xml_file_delete_by_privateID(Type_ID privateID)
	{
		if(privateID != 0)
		{
			std::string path = std::string(
#ifdef _FLOVER_
						this->flover->options->get_xml_path() +
#endif
											this->xml_node_path + XML_STRING_SLASH +
											this->xml_node_name + XML_STRING_UNDERSCORE + base64Encode(variable_to_uchar<Type_ID>(privateID)) + XML_STRING_FILENAME_EXTENSION_XML);
			return std::remove(path.c_str());
		}

		return EXIT_FAILURE;
	}

	void xml_set_node_settings(T *node)
	{
		node->xml_set_node_infos(this->xml_node_name);
	}

	T *xml_node_parse(std::string xml_file)
	{
		if(xml_file.empty() == false)
		{
			tinyxml2::XMLDocument document;
			document.Parse(xml_file.c_str(), xml_file.size());
			tinyxml2::XMLElement *root = document.RootElement();

			if(root == nullptr)
			{
				return nullptr;
			}

			tinyxml2::XMLElement *child = root->FirstChildElement();

#ifdef _ZLIB
			std::string xml_name = root->Value();

			if(xml_name == XML_STRING_COMPRESSED_ZLIB)
			{
				std::vector<uint8_t> data;
				data = base64Decode(get_std_string(root));

				if(data.empty())
				{
					return nullptr;
				}

				std::string xml_data = std_string_decompress(data);

				document.Parse(xml_data.c_str(), xml_data.size());
				root = document.RootElement();

				if(root == nullptr)
				{
					return nullptr;
				}

				child = root->FirstChildElement();
			}

#endif

			if(this->node_exists_by_privateID(root) == true)
			{
				return nullptr;
			}

			T *node = this->create();

			if(node->xml_parse_loop(root) == EXIT_FAILURE)
			{
				this->del(node);
				return nullptr;
			}

			return node;
		}

		return nullptr;
	}

	int node_exists_by_privateID(tinyxml2::XMLElement *root)
	{
		if(root == nullptr)
		{
			return 0;
		}
		tinyxml2::XMLElement *child = root->FirstChildElement();

		std::string text ;

		while(child != nullptr)
		{
			text = child->Value();
			if(text == XML_STRING_PRIVATE XML_STRING_ID)
			{
				Type_ID id = variable_read<Type_ID>(child);

				if(this->_get_pointer_of_privateID(id) != nullptr)
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			child = child->NextSiblingElement();
		}

	}

	/*
	T* xml_node_parse(std::string xml_file)
	{
	  if(xml_file.empty() == false)
	  {
		 T *node = this->create();

		 if(node->xml_parse(xml_file) == EXIT_FAILURE)
		 {
			this->del(node);
			return nullptr;
		 }

		 return node;
	  }

	  return nullptr;
	}
	*/

	T *xml_node_parse(tinyxml2::XMLElement *element)
	{
		if(element != nullptr)
		{
			T *node = this->create();
			node->xml_parse(element);
			return node;
		}

		return nullptr;
	}

	virtual void xml_set_settings()
	{

	}

	int xml_listing_read(tinyxml2::XMLElement *element)
	{
		std::string name = element->Value();

		if(name != this->xml_node_name + XML_STRING_LISTING)
		{
			return EXIT_FAILURE;
		}

		tinyxml2::XMLElement *child_element = element->FirstChildElement();

		while(child_element != nullptr)
		{
			name = child_element->Value();

			if(name == XML_STRING_MANAGER XML_STRING_VALUE XML_STRING_S)
			{
				this->xml_read_manager_values(child_element);
			}

			else if(name == this->xml_node_name)
			{
				T *node = this->create();
				node->xml_parse_loop(child_element);
			}

#ifdef _ZLIB
			else if(name == XML_STRING_COMPRESSED_ZLIB)
			{
				this->xml_node_parse(xml_string_read_decompress(child_element));
			}
#endif

			child_element = child_element->NextSiblingElement();
		}

		return EXIT_SUCCESS;
	}

	virtual int xml_write_manager_values(tinyxml2::XMLPrinter *printer)
	{
		if(printer != nullptr)
		{
			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	virtual void xml_read_manager_values( tinyxml2::XMLElement *element)
	{
#ifdef _FLOVER_
		if(this->flover != nullptr)
		{

		}
#endif

		if(element != nullptr)
		{
			return void();
		}
	}

	int xml_listing_read(std::string buffer)
	{
		if(buffer.empty())
		{
			return EXIT_FAILURE;
		}

		tinyxml2::XMLDocument document;
		document.Parse(buffer.c_str(), buffer.size());
		tinyxml2::XMLElement *element = document.RootElement();

		if(element != nullptr)
		{
			this->xml_listing_read(element);
		}

		document.Clear();

		if(this->first != nullptr)
		{
			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}
	int xml_listing_write(tinyxml2::XMLPrinter *printer, XML_Options_Table &options)
	{
		if(printer == nullptr)
		{
			return EXIT_FAILURE;
		}


		T *node;
		node = this->first;

		if(node != nullptr)
		{
			std::string listing_name = this->xml_node_name + XML_STRING_LISTING;

			this->xml_write_manager_values(printer);

			printer->OpenElement(listing_name.c_str(), options.no_empty_space);

			while(node != nullptr)
			{
				node->xml_set_node_infos(this->xml_node_name);
				node->xml_get(printer, options);

				node = node->next;
			}

			printer->CloseElement(options.no_empty_space);

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	int xml_listing_write(tinyxml2::XMLPrinter *printer)
	{
		if(printer == nullptr)
		{
			return EXIT_FAILURE;
		}


		T *node;
		node = this->first;

		if(node != nullptr)
		{
			std::string listing_name = this->xml_node_name + XML_STRING_LISTING;

			this->xml_write_manager_values(printer);

			printer->OpenElement(listing_name.c_str(), this->flover->xml_options.no_empty_space);

			while(node != nullptr)
			{
				node->xml_set_node_infos(this->xml_node_name);
				node->xml_get(printer, this->flover->xml_options);

				node = node->next;
			}

			printer->CloseElement(this->flover->xml_options.no_empty_space);

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	void xml_read_all_non_in_memory()
	{
		if(this->all_files_read)
		{
			return void();
		}

		std::vector<std::string> files;

		if(directory_listing_get(
#ifdef _FLOVER_
					this->flover->options->get_xml_path() +
#endif
					this->xml_node_name, &files) == EXIT_FAILURE)
		{
			return void();
		}

		for(std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
		{
			std::string base_ext = it->substr(it->find_first_of("_") + 1, it->find_first_of("."));
			std::string temp = base_ext.substr(0, base_ext.find("."));

			Type_ID id = uchar_to_variable<Type_ID>(base64Decode(temp));

			if(this->is_loaded_privateID(id) == EXIT_FAILURE)
			{
				this->load_file(id);
			}
		}

		this->all_files_read = true;
	}

	int is_loaded_privateID(Type_ID id)
	{
		T *temp = this->first;

		while(temp != nullptr)
		{
			if(temp->privateID == id)
			{
				return EXIT_SUCCESS;
			}

			temp = temp->next;
		}

		return EXIT_FAILURE;
	}

#ifdef _FLOVER_
	void prepare_for_objectPlacer()
	{
		if(this->all_files_read == false)
		{
			this->xml_read_all_non_in_memory();
		}
	}
#endif

	int current_set(T *node)
	{
		if(node == nullptr)
		{
			this->current_prev = this->current;
			this->tick_current_prev();
			this->current = nullptr;

			return EXIT_SUCCESS;
		}

		if(this->node_exists(node) == EXIT_SUCCESS)
		{
			this->current_prev = this->current;
			this->tick_current_prev();

			this->current = node;

			if(this->current != nullptr)
			{
				this->current->set_is_current();
			}

			this->current_changed = true;

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	int current_set_by_id(Type_ID id,  bool privateID)
	{
		if(privateID)
		{
			return this->current_set(this->_get_pointer_of_privateID(id));
		}

		return this->current_set(this->get_pointer_of_id(id));
	}

	int current_set_by_id( Type_ID id)
	{
		return this->current_set(this->get_pointer_of_id(id));
	}

	int current_set_by_privateID( Type_ID id)
	{
		return this->current_set(this->_get_pointer_of_privateID(id));
	}

	int current_to_next()
	{
		if(this->current != nullptr)
		{
			if(this->current->next != nullptr)
			{
				this->current = this->current->next;
				return EXIT_SUCCESS;
			}
		}

		return EXIT_FAILURE;
	}

	int current_to_prev()
	{
		if(this->current != nullptr)
		{
			if(this->current->prev != nullptr)
			{
				this->current = this->current->prev;
				return EXIT_SUCCESS;
			}
		}

		return EXIT_FAILURE;
	}

	int current_set_secondary_by_privateID( Type_ID id)
	{
		T *temp = this->_get_pointer_of_privateID(id);

		if(temp != nullptr)
		{
			this->current_secondary = temp;
			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	void tick_current_prev()
	{
		if(this->node_exists(this->current_prev) == EXIT_SUCCESS)
		{
			this->current_prev->set_not_current();
		}

		else
		{
			this->current_prev = nullptr;
		}
	}

	virtual int files_create(std::string path,  bool one_path = true)
	{
		if(path.empty() == false && one_path == false)
		{

		}

		return EXIT_FAILURE;
	}

	T *first;
	T *last;

	T *current;
	T *current_prev;
	bool current_changed;
	bool is_child_manager;

	Type_ID nextID;
	unsigned int counter;
	bool remove_unneeded;
	bool all_files_read;

	std::string xml_node_name;
	std::string xml_node_path;

#ifdef _FLOVER_

	int listing_create()
	{
#ifndef ANDROID

		if(this->files == nullptr)
		{
			this->files = new Manager<T>;
		}

		this->all_files_read = true;
		std::string data_path = this->flover->options->get_xml_path() + this->get_node_path();


		if(file_exits(data_path) == EXIT_FAILURE)
		{
			return EXIT_FAILURE;
		}

		std::string path;
		std::string buffer;

#ifdef __WASM__

		for(auto &p : std::__fs::filesystem::directory_iterator(data_path))
#else
		for(auto &p : std::filesystem::directory_iterator(data_path))
#endif
		{
			buffer.clear();
			path.clear();

			path = p.path().filename().string();

			T *node = this->files->create();

			if(this->flover->sync_table.peek_nodeInfo)
			{
				path = p.path().string();
				buffer = file_read_text(this->flover->sync_table, path);

				if(buffer.empty())
				{
					continue;
				}

				tinyxml2::XMLDocument document;

				if(document.Parse(buffer.c_str(), buffer.size()) != tinyxml2::XML_SUCCESS)
				{
					continue;
				}

				tinyxml2::XMLElement *element = document.RootElement()->FirstChildElement();

				while(element != nullptr)
				{
					if(node->info.xml_parse(element) == EXIT_SUCCESS)
					{
						if(node->info.name.empty())
						{
							node->info.name = p.path().stem().string();
						}

						break;
					}

					element = element->NextSiblingElement();
				}
			}

			else
			{
				std::string coded = p.path().stem().string().substr(p.path().stem().string().find_first_of("_")+1, p.path().stem().string().find_first_of("."));
				node->privateID = uchar_to_variable<Type_ID>(base64Decode(coded));
				node->info.name = p.path().stem().string();
			}

#ifdef _DEBUG
			std::cout << node->info.name << std::endl;
#endif
		}

		if(this->first != nullptr)
		{
			return EXIT_SUCCESS;
		}
#endif

		return EXIT_FAILURE;
	}

	int has_files()
	{
		if(this->files != nullptr)
		{
			return true;
		}

		return false;
	}

	int is_files_empty()
	{
		if(this->files != nullptr)
		{
			if(this->files->first != nullptr)
			{
				return false;
			}
		}

		return true;
	}
	Manager<T> *files;

#ifdef _FLOVER_
	void set_flover_pointer(Flover *flover_pointer)
	{
		this->flover = flover_pointer;
	}
	Flover *flover;
#else
	Storage *storage;
#endif

protected:

	std::vector<T *> nodes_to_sync;
	std::vector<T *> nodes_to_delete;

#endif

};
#endif
