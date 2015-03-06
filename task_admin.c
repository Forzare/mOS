//
//  task_admin.c
//  mOS
//
//  Created by Markus Axelsson on 2015-02-10.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#include "list_admin.h"
#include "timing.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "kernel.h"


bool State = INIT;


void idle(){

  for(;;){
        SaveContext();
        TimerInt();
        LoadContext();         
  }
	
}

exception init_kernel(void){
    set_ticks(0);
      g_readylist = create_list();
      
      
      g_readylist->pHead->pNext = g_readylist->pTail;
      g_readylist->pTail->pPrevious = g_readylist->pHead;
	if(g_readylist == NULL){
	
		free(g_readylist);
		return FAIL;
	}
        
        
	
      g_timerlist = create_list();
      g_timerlist->pHead->pNext = g_timerlist->pTail;
      g_timerlist->pTail->pPrevious = g_timerlist->pHead;
	if(g_timerlist == NULL){
		free(g_readylist);
		free(g_timerlist);
		return FAIL;
	}
	
      g_waitinglist = create_list();
      g_waitinglist->pHead->pNext = g_waitinglist->pTail;
      g_waitinglist->pTail->pPrevious = g_waitinglist->pHead;
	
	if(g_waitinglist == NULL){
		free(g_readylist);
		free(g_timerlist);
		free(g_waitinglist);
		return FAIL;
	}
	
	exception status = create_task(&idle, 0);
	
	if(status != OK){
		
		free(g_readylist);
		free(g_timerlist);
		free(g_waitinglist);
		
		return status;
		
	
	}
        
        
        
	State = INIT;
	return OK;
		
    
}

exception create_task(void(* body)(), uint d){
  volatile int firstrun = TRUE;
    int status;
    TCB *newTCB = malloc(sizeof(TCB));
    listobj *newObj = malloc(sizeof(listobj));
    if((newObj == NULL) || (newTCB == NULL)){
        free(newTCB);
        free(newObj);
        return FAIL;
    }
  
    if(d == 0){
      
      newTCB->DeadLine = 0xffffffff;
      
    }
      
    else{
      newTCB->DeadLine = ticks() + d;
    }
        newTCB->PC = body;
        newTCB->SP = &(newTCB->StackSeg[STACK_SIZE-1]);
        
        newObj->pTask = newTCB;
        newObj->nTCnt = ticks();
        if (!State) {
            status = push_list(g_readylist,newObj);

          
          if(status != OK){
          
            free(newTCB);
           
          }
           return status;
        }
          
       else{
            isr_off();
            
            SaveContext();
            if (firstrun) {
               firstrun = 0;

                status = push_list(g_readylist, newObj);
                
                if(status != OK){
                 
                  free(newTCB);
                  
                }
                  
                
                Running = peek_list(g_readylist)->pTask;
                LoadContext();
            }
        }
        
        
        return status;  
}


void run(void){

	//timer0_start();	
	State = RUNNING;
        Running = peek_list(g_readylist)->pTask;
	isr_on();
	LoadContext();
}



void terminate(void){
	
	
	listobj *remove_object;

	remove_object = extract_readylist();

        
	free(remove_object);
        Running = peek_list(g_readylist)->pTask;
	LoadContext();

}