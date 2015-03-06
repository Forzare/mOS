//
//  interproc_com.c
//  mOS
//
//  Created by Markus Axelsson on 2015-02-13.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#include "interproc_com.h"

#include "main.h"
#include "kernel.h"
#include "list_admin.h"
#include <stdio.h>
#include <string.h>



//mailbox struct with FIFO-implementation

exception push_mailbox(mailbox *mBox, msg *message){
	
	if(mBox == NULL || message == NULL){
	
		return FAIL;
	}

	message->pPrevious = mBox->pHead;
	message->pNext = mBox->pHead->pNext;
	mBox->pHead->pNext->pPrevious = message;
	mBox->pHead->pNext = message;
	
	return OK;
}

msg* pop_mailbox(mailbox *mBox){
	
	msg* target_message = mBox->pTail->pPrevious;
	target_message->pPrevious->pNext = target_message->pNext;
	mBox->pTail->pPrevious = target_message->pPrevious;
	
	target_message->pNext = NULL;
	target_message->pPrevious = NULL;
	
	
	return target_message;
	
	
}

int no_messages( mailbox* mBox ){

	if(mBox == NULL){
	
		return -1;
	
	}
	
	return mBox->nMessages;
	
}



void remove_message(msg *new_message){
	
	new_message->pPrevious->pNext = new_message->pNext;
	new_message->pNext->pPrevious = new_message->pPrevious;
	new_message->pNext = new_message->pPrevious = NULL;
	free(new_message);
	
}



mailbox* create_mailbox( uint nMessages, uint nDataSize ){
	mailbox *newMb = malloc(sizeof(mailbox));
	newMb->pHead = malloc(sizeof(msg));
	newMb->pTail = malloc(sizeof(msg));
	if(newMb == NULL || newMb->pHead == NULL || newMb->pTail == NULL){
		free(newMb->pHead);
		free(newMb->pTail);
		free(newMb);
		return NULL;
	}
	
	else{
		newMb->nMaxMessages = nMessages;
		newMb->nDataSize = nDataSize;
		newMb->nMessages = 0;
		newMb->nBlockedMsg = 0;
		newMb->pHead->pNext = newMb->pTail;
		newMb->pTail->pNext = newMb->pHead;
		newMb->pTail->pPrevious = newMb->pHead;
		
		
		return newMb;
	}
}

exception remove_mailbox( mailbox * mBox){
	if (mBox->nMessages == 0) {
		free(mBox->pHead);
		free(mBox->pTail);
		free(mBox);
		return OK;
	}
	else{
		return NOT_EMPTY;
	}
}

exception send_wait(mailbox* mBox, void* pData){
	volatile int firstrun = TRUE;
	isr_off();
	SaveContext();
	if (firstrun){
		firstrun = FALSE;
		if (mBox->nBlockedMsg < 0 && mBox->pHead->pNext != mBox->pTail) {
			msg* reciever = pop_mailbox(mBox);
			memcpy(reciever->pData, pData, mBox->nDataSize);
			
			
			exception status = extract_waitinglist(reciever->pBlock);
                        assert(status == OK);
                        
                                               
			push_list(g_readylist,reciever->pBlock);
                        Running = peek_list(g_readylist)->pTask;
                        
                        
			mBox->nBlockedMsg++;
		}
		
		else{
			msg *message = malloc(sizeof(msg));
			if(message == NULL){
				free(message);
				return FAIL;
			}
			message->pData = malloc(mBox->nDataSize);
			if(message->pData == NULL){
			
				return FAIL;
			}
			
			memcpy(message->pData, pData, mBox->nDataSize);
			exception push_status = push_mailbox(mBox, message);
			
			if(push_status == FAIL){
				
				return push_status;
			
			}
			
			
			listobj *sending_task;
			sending_task = extract_readylist();
			push_list(g_waitinglist, sending_task);
			Running = peek_list(g_readylist)->pTask;
			message->pBlock = sending_task;
			sending_task->pMessage = message;
			mBox->nBlockedMsg++;
		}
		LoadContext();
	}
	
	else{
		
		if(g_waitinglist->pHead->pNext->pTask->DeadLine <= TC){
			isr_off();

			g_readylist->pHead->pNext->pMessage->pNext->pPrevious = g_readylist->pHead->pNext->pMessage->pPrevious;
			g_readylist->pHead->pNext->pMessage->pPrevious->pNext = g_readylist->pHead->pNext->pMessage->pNext;
			
			free(g_readylist->pHead->pNext->pMessage->pData);
			free(g_readylist->pHead->pNext->pMessage);
			
			mBox->nMessages--;
			mBox->nBlockedMsg--;
			isr_on();
			return DEADLINE_REACHED;
		}
		
		else{
			return OK;
			
		}
	}
	
	return OK;
}


exception receive_wait(mailbox *mBox, void *data){

	volatile int firstrun = TRUE;
	isr_off();
	SaveContext();
	
	if(firstrun){
		firstrun = FALSE;
		
		if(mBox->nBlockedMsg > 0){
                  
                        msg* sender = pop_mailbox(mBox);
                        
			memcpy(mBox->pHead->pNext->pData, data, mBox->nDataSize);
                        free(sender->pData);
			

		
		
                  if(mBox->nBlockedMsg != NULL){


			exception status = extract_waitinglist(sender->pBlock);
                        assert(status = OK);
                        push_list(g_readylist, sender->pBlock);
                        Running = peek_list(g_readylist)->pTask;

		
                  }
                
                        mBox->nBlockedMsg--;
			free(sender);
               }
        
              else{
		listobj *receiving_task;
		msg *message = malloc(sizeof(msg));
                message->pData = data;
		exception status = push_mailbox(mBox,message);
                assert(status == OK);
		receiving_task = pop_list(g_readylist);
		push_list(g_waitinglist, receiving_task);
                
                
                message->pBlock = receiving_task;
                receiving_task->pMessage = message;
                mBox->nBlockedMsg--;
                Running = peek_list(g_readylist)->pTask;
                
                
            }
        
        
	LoadContext();
	
        }
	
	else{
		if(ticks() > Running->DeadLine){
			isr_off();
			
			g_readylist->pHead->pNext->pMessage->pNext->pPrevious = g_readylist->pHead->pNext->pMessage->pPrevious;
			g_readylist->pHead->pNext->pMessage->pPrevious->pNext = g_readylist->pHead->pNext->pMessage->pNext;
			
			free(g_readylist->pHead->pNext->pMessage);
			
			mBox->nMessages--;
			mBox->nBlockedMsg++;
			
			
			isr_on();
			return DEADLINE_REACHED;
                        
                        
                        
		
		}
               else{
			return OK;
		}
		

	
	}
	return OK;
}

exception send_no_wait(mailbox *mBox, void *data){
    
    volatile bool first_run = TRUE;
    isr_off();
    SaveContext();
    
    if(first_run){
        first_run = FALSE;
        
        if( mBox->nBlockedMsg < 0 ){
            msg* receiver = pop_mailbox(mBox);
            assert(receiver);
            memcpy(receiver->pData, data, mBox->nDataSize);
            
            exception status = extract_waitinglist(receiver->pBlock);
            assert(status == OK);
            
            status = push_list(g_readylist, receiver->pBlock);
            assert(status == OK);
            Running = peek_list(g_readylist)->pTask;
            
            free(receiver);
            mBox->nBlockedMsg++;
            LoadContext();
        }
        else{
            msg *message = malloc(sizeof(msg));
            if (!message) {
                return FAIL;
            }
            message->Status = SENDER;
            message->pData = malloc(mBox->nDataSize);
            if (!message->pData) {
                free(message);
                return FAIL;
            }
            memcpy(message->pData, data, mBox->nDataSize);
            if (mBox->nMessages == mBox->nMaxMessages) {
                msg* old = pop_mailbox(mBox);
            }
            exception status = push_mailbox(mBox, message);
            if(status != OK){
                free(message->pData);
                free(message);
                return status;
            }
            message->pBlock = NULL;
            mBox->nBlockedMsg++;
        }
    }
    isr_on();
    return OK;
}

//nMessage => # meddelande i mailen
//nBlockedMsg => (+) # meddelande som skall skickas
//               (-) # meddelande som skall tas emot
//nMessage = |nBlockedMsg|


int receive_no_wait( mailbox* mBox, void* pData ){
    volatile  bool first_run = TRUE;
    volatile exception status = FAIL;
    isr_off();
    SaveContext();
    if (first_run) {
        first_run = FALSE;
        if (mBox->nBlockedMsg > 0) {
            msg* sender = pop_mailbox(mBox);
            assert(sender);
            
            memcpy(pData, sender->pData, mBox->nDataSize);
            
            if (sender->pBlock != NULL) {
                
                status = extract_waitinglist(sender->pBlock);
                assert(status == OK);
                
                status = push_list(g_readylist, sender->pBlock);
                assert(status == OK);
                Running = peek_list(g_readylist)->pTask;
                
            }
            free(sender->pData);
            free(sender);
            mBox->nBlockedMsg--;
            status = OK;
        }
        else{
            status = FAIL;
        }
        LoadContext();
    }
    return status;
}


























