/**
 * SPU_REGISTER
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_register.h - General purpose spu program unique id register
 *
 * Copyright (c) 2007, Kristian "unsolo" Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * Copyright (c) 2007, Steven "Sauce" Osman <sosman@terratron.com>
 * $Id:
 */

// Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>

// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The names of the authors may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef __SPU_EVENT_HANDLER_H
#define __SPU_EVENT_HANDLER_H

#include "spu_register.h"
#define PPU_MESSAGE_QUEUE_SIZE 16
#define MAX_NUM_CALLBACKS 128

typedef uint8_t message;
typedef uint8_t spu_callback_id;

struct ppu_bound_message {
  spu_program_unique_id spu_id; // 4 bytes (overkill)
  message message_type;            // 1 byte
  bool active;                  // 1 byte (overkill)
  char acDetails[122];          // Makes 128 byte message. This is up to the developer.
}__attribute__((alligned(128)));



typedef void (*ppu_side_event_callback)(spu_program_unique_id id, ppu_bound_message *msg, void *private); 

class ppu_event_handler {
	
	protected:
	struct handler_entry {
	//     bool active; covered in register.
		ppu_side_event_callback callbacks[MAX_NUM_CALLBACKS];
	//	void *private;
 	};
	
	ppu_event_handler() {
		free.resize(MAX_NUM_CALLBACKS);
		for (int i = 0; i<free.size();i++)
			free_callbacks.push_back(i);
		//memset(handlers, 0, sizeof(handlers));
		//memset(message_queue, 0, sizeof(message_queue));
		
		// different threading is better, ofc
		//pthreads..
		if (fork() == 0) {
			message_loop();
			exit(0);
		}
	}
	
	spu_register spureg;
	vector<int> free_callbacks;

	
 	handler_entry handlers[MAX_SPU_PROGRAM_ID];
	
	ppu_bound_message message_queue[PPU_MESSAGE_QUEUE_SIZE];

	ppu_bound_message *get_message_queue() {
   	 	return message_queue;
 	}

	spu_callback_id register_event_handler(spu_program_id id, ppu_side_event_callback callback) {

		if (spureg.is_in_use(id)) { //check that this is a active ID prior to registering callback
			if (free_callbacks.empty())
				return -1;
			
			spu_callback_id sci=free_callbacks.back()
			callbacks[sci] = callback;
			
			return sci;
			
		} else {

			printf("Bad SPU id!!!");

		}
  	}
};
