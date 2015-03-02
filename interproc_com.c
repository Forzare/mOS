//
//  interproc_com.c
//  mOS
//
//  Created by Markus Axelsson on 2015-02-13.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#include <stdio.h>
#include "interproc_com.h"
#include <string.h>
#include "main.h"
#include "kernel.h"
#include "list_admin.h"
#include <assert.h>

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
		newMb->nMessages = nMessages;
		newMb->nDataSize = nDataSize;
		newMb->nMessages = 0;
		newMb->nBlockedMsg = 0;
		newMb->pHead->pNext = newMb->pTail;
		newMb->pTail->pNext = newMb->pHead;
		
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
	volatile bool firstrun = TRUE;
	isr_off();
	SaveContext();
	if (firstrun){
		firstrun = FALSE;
		if ( mBox->nBlockedMsg < 0 ) {
			memcpy(mBox->pHead->pNext->pData, pData, mBox->nDataSize); //??
			listobj *recieving_task;
			recieving_task = mBox->pHead->pNext->pBlock;
			remove_message(mBox->pHead->pNext);
			extract_waitinglist(recieving_task);
			insert_waiting_ready_list(g_readylist,recieving_task);
            mBox->nMessages--;
            mBox->nBlockedMsg++;
		}
		
		else{
			msg *message = malloc(sizeof(msg));
			if(message == NULL){
				free(message);
				return FAIL;
			}
			message->pData = pData;
			message->pNext = mBox->pTail;
			message->pPrevious = mBox->pTail->pPrevious;
			message->pBlock = g_readylist->pHead->pNext;
			mBox->pTail->pPrevious->pNext = message;
			mBox->pTail->pPrevious = message;
			mBox->nBlockedMsg++;
			mBox->nMessages++;
			listobj *sending_task;
			sending_task = extract_readylist();
			insert_waiting_ready_list(g_waitinglist, sending_task);
		}
		LoadContext();
	}
	
	else{
		if(g_readylist->pHead->pNext->pTask->DeadLine <= TC){
			isr_off();
			remove_message(mBox->pHead->pNext);
			isr_on();
			return DEADLINE_REACHED;
		}
		else{
			return OK;
		}
	}
	return OK;
}

exception recieve_wait(mailbox *mBox, void *data){
    
    volatile bool firstrun = TRUE;
    isr_off();
    SaveContext();
    
    if(firstrun){
        firstrun = FALSE;
        
        if( mBox->nBlockedMsg > 0 ){
            memcpy(mBox->pHead->pNext->pData, data, mBox->nDataSize);
            remove_message(mBox->pHead->pNext);
            mBox->nBlockedMsg--;
            mBox->nMessages--;
            
            if(mBox->nBlockedMsg < 0){
                listobj *sending_task;
                sending_task = mBox->pHead->pNext->pBlock;
                extract_waitinglist(sending_task);
                insert_waiting_ready_list(g_readylist, sending_task);
                mBox->nBlockedMsg--;
                mBox->nMessages++;
            }
            else{
                free(mBox->pHead->pNext->pData);
            }
        }
        else{
            listobj *recieving_task;
            msg *message = malloc(sizeof(msg));
            mBox->pHead->pNext = message;
            recieving_task = extract_readylist();
            insert_waiting_ready_list(g_waitinglist, recieving_task);
        }
        LoadContext();
    }
    else{
        if(g_waitinglist->pHead->pNext->pTask->DeadLine <= TC){
            isr_off();
            remove_message(mBox->pHead->pNext);
            mBox->nMessages--;
            mBox->nBlockedMsg++;
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
            
            status = insert_waiting_ready_list(g_readylist, receiver->pBlock);
            assert(status == OK);
            
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
                
                status = insert_waiting_ready_list(g_readylist, sender->pBlock);
                assert(status == OK);
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




