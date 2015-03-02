//
//  task_admin.c
//  mOS
//
//  Created by Markus Axelsson on 2015-02-10.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#include <stdio.h>
#include "list_admin.h"
#include "timing.h"
#include "main.h"
#include "kernel.h"

int g_running_mode = FALSE;
int g_firstrun = TRUE;

exception init_kernel(void){
    set_ticks(0);
	
	g_readylist = create_list();
	
	if(g_readylist == NULL){
	
		return FAIL;
	}
	
    g_timerlist = create_list();
	
	if(g_timerlist == NULL){
		return FAIL;
	}
	
    g_waitinglist = create_list();
	
	if(g_waitinglist == NULL){
		
		return FAIL;
	}
	return OK;
}

exception create_task( void(*body)(), uint d ){
    int status;
    TCB *newTCB = malloc(sizeof(TCB));
    listobj *newObj = malloc(sizeof(listobj));
    if((newObj == 0) && (newTCB == 0)){
        free(newTCB);
        free(newObj);
        return FAIL;
    }
    else{
        
        newTCB->DeadLine = d;
        newTCB->PC = body;
        newTCB->SP = &(newTCB->StackSeg[99]);
        if (g_running_mode == 0) {
            status = insert_waiting_ready_list(g_readylist,newObj);
            
            newTCB->DeadLine = d;
            newTCB->PC = body;
            newTCB->SP = &(newTCB->StackSeg[99]);
            if (g_running_mode == 0) {
                status = insert_waiting_ready_list(g_readylist,newObj);
                return status;
            }
            else{
                isr_off();
                SaveContext();
                if (g_firstrun == 1) {
                    g_firstrun = 0;
                    status = insert_waiting_ready_list(g_readylist, newObj);
                    LoadContext();
                }
            }
            return status;
        }
    }
    return status;
}

void run(void){

	//Initialize interrupts
	
	g_running_mode = TRUE;
	isr_on();
	LoadContext();
}

void terminate(void){
	
	listobj *remove_object;
	remove_object = extract_readylist();
	remove_object = extract_readylist();
	free(remove_object);
	LoadContext();

}