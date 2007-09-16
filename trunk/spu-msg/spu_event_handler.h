/**
 * SPU EVENT HANDLER
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_event_handler.h - General purpose spu program event handler
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

#include <pthread.h>
#include <libspe.h>

#include "spu_register.h"


#define PPU_MESSAGE_QUEUE_SIZE 16
#define MAX_NUM_CALLBACKS 128

typedef uint8_t message;
typedef uint8_t callback_id;

struct ppu_bound_message {
  spu_program_unique_id spu_id; // 4 bytes (overkill)
  callback_id call_id;            // 1 byte
  bool active;                  // 1 byte (overkill)
  char acDetails[122];          // Makes 128 byte message. This is up to the developer.
}__attribute__((aligned(128)));




typedef void (*ppu_side_event_callback)(spu_program_unique_id id, ppu_bound_message *msg); 

struct thread_data {
	bool run;
	ppu_side_event_callback callbacks[MAX_NUM_CALLBACKS];
	struct ppu_bound_message message_queue[PPU_MESSAGE_QUEUE_SIZE];
};



class spu_event_handler /*:protected spu_register*/ {
	
	public:


	spu_event_handler() {
		free_callbacks.resize(MAX_NUM_CALLBACKS);
		for (int i = 0; i<MAX_NUM_CALLBACKS;i++)
			free_callbacks.push_back(i);
		
		td.run=1;
		thread_id = pthread_create(&pts,NULL,&message_loop_thread,&td);
	}
	
	~spu_event_handler(){
		td.run=0;
		pthread_join(pts,NULL);
	};
	
	callback_id register_callback(spu_program_unique_id id, ppu_side_event_callback callback) 
	{

		printf("is_in_use %d\n",spureg.is_in_use(id));
		if (spureg.is_in_use(id)) { //check that this is a active ID prior to registering callback
		
			if (free_callbacks.empty())
				return -1;
			
			callback_id sci=free_callbacks.back();

			td.callbacks[sci] = callback;
			
			return sci;
			free_callbacks.pop_back();
			
		} else {

			printf("Bad SPU id!!! \t: %d\n",id);

		}
  	}

	
	void register_message(ppu_bound_message pbm) // Do not use this can cause problems.. only for testing purposes
	{
		bool unregistered =1;
		while (unregistered)
		{
			for (int i = 0 ; i < PPU_MESSAGE_QUEUE_SIZE ; i++)
			{
				if (!td.message_queue[i].active)
					{
					td.message_queue[i]=pbm;
					unregistered=0;
					break;
					}
			}

		}

	}
	
	ppu_bound_message *get_message_queue() {
   	 	return td.message_queue;
 	}
	
	
	static void * message_loop_thread(void* arg)
	{
		recv_spu_mbox_message(); // should this be interrupt perhaps ?
		struct thread_data * arg_ptr;
		arg_ptr=(struct thread_data *) arg;
		while (arg_ptr->run)
		{
			for (int i=0; i< PPU_MESSAGE_QUEUE_SIZE; i++) //do a loop.
			{
				if (arg_ptr->message_queue[i].active) //do we have a message to process ?
				{
					arg_ptr->callbacks[arg_ptr->message_queue[i].call_id](arg_ptr->message_queue[i].spu_id,&arg_ptr->message_queue[i]);
				
					arg_ptr->message_queue[i].active=0; //this message is now "deleted" from beeing active.
				}
			}
		}
		pthread_exit(NULL);
	}

	spu_register spureg;
	

	protected:
	int thread_id;

	pthread_t pts;
	
	thread_data td;

	vector<int> free_callbacks;
	
};

#endif
