/*
 * License : BSD 3-Clause "New" or "Revised" License
 * File : libTemplates/include/Node_Depency.h
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

#ifndef _NODE_DEPENCY
#define _NODE_DEPENCY
#include <Common_Types.h>
#include <stdlib.h>
#include <Node.h>

#ifdef _DEBUG
#include <iostream>
#endif

template <class M, class N>
struct Node_Depency
{
	bool changed;
	bool give_up;
	bool node_synched;
	bool synched;
	N *node;
	Type_ID privateID;
	Type_ID new_privateID;

	Node_Depency()
	{
		this->node = nullptr;
		this->clear();
	}

	~Node_Depency()
	{
		this->clear();
	}

	void runtime_clear(bool set_give_up = false)
	{
		if(this->node != nullptr)
		{
			this->node->node_flags.bit_set(Node_Flags::Needed, false);
		}

		this->changed = false;
		this->give_up = set_give_up;
		this->node = nullptr;
		this->node_synched = false;
		this->synched = false;
		this->new_privateID = 0;
	}

	void clear()
	{
		this->node = nullptr;
		this->privateID = 0;
		this->new_privateID = 0;
		this->runtime_clear();
	}

	void set_give_up(bool value)
	{
		this->give_up = value;
	}

	int unsync(Sync_Table &table)
	{
		int err = EXIT_FAILURE;

		if(this->node != nullptr)
		{
			err = node->unsync(table);

			if(err == EXIT_SUCCESS)
			{
				this->synched = false;
				this->node_synched = false;
			}

		}

		return err;
	}

	void set_privateID(Type_ID id)
	{
		if(id == 0)
		{
			this->clear();
		}

		else if(this->node == nullptr && id != 0)
		{
			this->privateID = id;
			this->new_privateID = id;
			this->changed = true;
			this->give_up = false;
			this->node_synched = false;
			this->synched = false;
			this->node_synched = false;
		}

		else if(this->privateID != id && id != 0)
		{
			this->new_privateID = id;
			this->changed = true;
			this->give_up = false;
			this->node_synched = false;
			this->synched = false;
		}
	}

	int set_by_pointer(N *pointer, bool ignore_synched = false)
	{
		if(pointer != nullptr)
		{
			this->clear();

			if(pointer->node_flags.bit_get(Node_Flags::Synched) ||
					ignore_synched)
			{
				this->privateID = pointer->privateID;
				this->node = pointer;
				this->synched = pointer->node_flags.bit_get(Node_Flags::Synched);
				this->node_synched = true;
			}

			this->changed = false;
			this->give_up = false;

			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}

	void set_pointer_n_privateID(Node_Depency &pointer)
	{
		if(pointer.privateID != 0 && pointer.node != nullptr)
		{
			this->clear();
			this->privateID = pointer.privateID;

			this->node = pointer.node;
			this->synched = pointer.synched;
			this->node_synched = pointer.node_synched;
		}

		this->changed = false;
		this->give_up = false;

	}

	void copy_from(Node_Depency *from)
	{
		this->privateID = from->privateID;
	}

	void copy_to(Node_Depency *to)
	{
		to->privateID = this->privateID;
	}

	int set_prev(M *manager, bool ignore_synched = false)
	{
		if(this->node != nullptr)
		{
			if(this->node->prev != nullptr)
			{
				return this->set_by_pointer(this->node->prev, ignore_synched);
			}
			else
			{
				this->clear();

				return EXIT_SUCCESS;
			}
		}

		else
		{
			if(manager->last == nullptr)
			{
				return EXIT_FAILURE;
			}

			return this->set_by_pointer(manager->seek_prev_listing_node(nullptr), ignore_synched);
		}

		return EXIT_FAILURE;
	}

	int is_synched()
	{
		if(this->synched == true && this->give_up == false)
		{
			return true;
		}

		return false;
	}

	int set_next(M *manager, bool ignore_synched = false)
	{
		if(this->node != nullptr)
		{
			if(this->node->next != nullptr)
			{
				this->set_by_pointer(this->node->next, ignore_synched);

				return EXIT_SUCCESS;
			}
			else
			{
				this->clear();

				return EXIT_SUCCESS;
			}
		}

		else
		{
			if(manager->first == nullptr)
			{
				return EXIT_FAILURE;
			}

			this->set_by_pointer(manager->seek_next_listing_node(nullptr), ignore_synched);
		}

		return EXIT_FAILURE;
	}
};

template <class M, class N>
int sync_node_depency(M *manager, Node_Depency<M, N> &nodeDepency,  Sync_Table &table, bool forced = false)
{
#ifdef _DEBUG
	std::cout << "sync_node_depency()" << std::endl
			  << "depency privateID = " << nodeDepency.privateID << std::endl;
#endif

	if(forced == true)
	{
		nodeDepency.give_up = false;
	}

	if(table.content_unsync)
	{
		nodeDepency.unsync(table);
		nodeDepency.runtime_clear();
		nodeDepency.node = nullptr;

		return EXIT_SUCCESS;
	}

	else if(nodeDepency.synched &&
			table.resync == false &&
			table.runtime_clear == false &&
			table.depency_data_clear == false)
	{
		return EXIT_SUCCESS;
	}

	if(nodeDepency.privateID == 0 && nodeDepency.new_privateID == 0)
	{
		return EXIT_FAILURE;
	}

	if((nodeDepency.synched == false && nodeDepency.give_up == false) || (nodeDepency.changed && nodeDepency.give_up == false) || table.resync)
	{
		N *new_node = nullptr;

		if(nodeDepency.new_privateID != 0 && table.resync == false)
		{
			new_node = manager->get_pointer_of_privateID(nodeDepency.new_privateID);
		}

		else if(nodeDepency.privateID != 0 || (nodeDepency.privateID != 0 && table.resync))
		{
			new_node = manager->get_pointer_of_privateID(nodeDepency.privateID);
		}

		if(new_node != nullptr)
		{
			/*
			 * This whole class (Node_Depency) is a litle bit glichy,
			 * we wil need to set privateID from new_node.
			 *
			 * Don't ask me why, maby a glich on the C++ generator
			 */
			nodeDepency.privateID = new_node->privateID;
			nodeDepency.new_privateID = 0;

			if(nodeDepency.node != nullptr)
			{
				nodeDepency.node->node_flags.bit_set(Node_Flags::Needed, false);
			}

			nodeDepency.node = new_node;
			nodeDepency.node_synched = true;
			nodeDepency.node->node_flags.bit_set(Node_Flags::Needed, true);
			nodeDepency.changed = false;


#ifdef _DEBUG
			std::cout << "name : " << nodeDepency.node->info.name << std::endl;
#endif
		}

		else
		{
			nodeDepency.give_up = true;
			return EXIT_FAILURE;
		}
	}

	if (nodeDepency.node == nullptr)
	{
		return EXIT_FAILURE;
	}

	if(nodeDepency.node != nullptr)
	{
		if(nodeDepency.node->sync(table) == EXIT_SUCCESS)
		{
			nodeDepency.node->node_flags.bit_set(Node_Flags::Synched, true);
			nodeDepency.synched = true;
		}
	}


	if(nodeDepency.synched && nodeDepency.give_up == false)
	{
		return EXIT_SUCCESS;
	}

	nodeDepency.give_up = true;

	return EXIT_FAILURE;
}

#endif
