//
//  timing.c
//  mOS
//
//  Created by Markus Axelsson on 2015-02-10.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#include "main.h"
#include "kernel.h"
#include "list_admin.h"
#include "kernel_hwdep.h"


exception wait(uint nTicks){
  volatile bool firstrun = TRUE;
   
    isr_off();
    SaveContext();
    if (firstrun) {
        firstrun = FALSE;
        listobj * tempObj;
        
        tempObj = pop_list(g_readylist);
        tempObj->nTCnt = nTicks + ticks();
        insert_timerlist(tempObj);
        Running = peek_list(g_readylist)->pTask;
        LoadContext();
        
        
    }
    else{
        if (ticks() > Running->DeadLine) {
            return DEADLINE_REACHED;
        }
        else{
           return OK;
        }
    }
    return OK;
}

void set_ticks(uint nTicks){
    TC = nTicks;
}

uint ticks(void){
    return TC;
}

uint deadline(void){
    return g_readylist->pHead->pTask->DeadLine;
}

void set_deadline(uint nNew){
    volatile int firstrun = TRUE;
    isr_off();
    SaveContext();
    if (firstrun) {
        firstrun = FALSE;
        listobj *temp = pop_list(g_readylist);
        temp->pTask->DeadLine = nNew;       
        push_list(g_readylist, temp);
        LoadContext();
    }
}

void TimerInt(void){
    TC++;
    listobj *obj = peek_list(g_timerlist);
    while (obj != NULL && ticks() > obj->nTCnt) {     
        push_list(g_readylist, pop_list(g_timerlist));
        obj = peek_list(g_timerlist);
    }
    
    
    obj = peek_list(g_waitinglist);
    while(obj != NULL && obj->pTask->DeadLine < ticks()){
            exception status = push_list(g_readylist, pop_list(g_waitinglist));
            
            obj = peek_list(g_waitinglist);
    }
    
    Running = peek_list(g_readylist)->pTask;
}



void     isr_off(void){

  //set_isr(ISR_OFF);

}
void     isr_on(void){

  //set_isr(ISR_ON);
}




