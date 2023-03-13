/*
 * License : BSD 3-Clause "New" or "Revised" License
 * File : libTemplates/include/Node_Info.h
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

#ifndef __NODE_INFO
#define __NODE_INFO

#include <Common_Types.h>
#include <string>

struct Sync_Table;
/*
 * Node_Info table contains some information of the node,
 * priveteID : unique identification of the node
 * name : given name of the node
 * info : info about the node, if multiple same named
 * memo : longer written information of the node, may be not written on Data_Final(Flover)
 * flags : optional flags for the node
 */

struct Node_Info
{
	std::string name;
	std::string info;
	std::string memo;

	void clear();
	void copy_to(Node_Info *to);
	void copy_from(Node_Info *from);
	int xml_parse(tinyxml2::XMLElement *element);
	void xml_create(tinyxml2::XMLPrinter *printer, XML_Options_Table &options);
};
#endif
