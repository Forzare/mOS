//
//  main.c
//  mOS DSTII
//
//  Created by Markus Axelsson on 2015-02-06.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

/*
#define MEMWATCH

#include <stdio.h>
#include "list_admin.h"
#include "main.h"
#include "memwatch.h"

int g_running_mode = FALSE;
int g_firstrun = TRUE;
unsigned int TC;

int main(void) {
    TC = 0;
    init_kernel();
    
    return 0;
}*/

#include <stdio.h>
#include "main.h"

#define TEST_PATTERN_1 0xAA
#define TEST_PATTERN_2 0x55
mailbox *mb;
void task1(void);
void task2(void);
int nTest1=0, nTest2=0, nTest3=0;
TCB *Running = NULL;

int main(void){
    
    if (init_kernel() != OK ) {
        /* Memory allocation problems */
        while(1); }
    if (create_task( task1, 2000 ) != OK ) {
        /* Memory allocation problems */
        while(1); }
    if (create_task( task2, 4000 ) != OK ) {
        /* Memory allocation problems */
        while(1); }
    if ( (mb=create_mailbox(1,sizeof(int))
          ) == NULL) {
        /* Memory allocation problems */
        while(1); }
    run(); /* First in readylist is task1 */
    return 0;
}

void task1(void){
    
    int nData_t1 = TEST_PATTERN_1; wait1(10); /* task2 borjar kora */
    if( no_messages(mb) != 1 ) terminate(); /* ERROR */
    if(send_wait(mb,&nData_t1) == DEADLINE_REACHED) terminate(); /* ERROR */
    wait1(10); /* task2 börjar köra */ /* start test 2 */
    nData_t1 = TEST_PATTERN_2;
    if(send_wait(mb,&nData_t1) == DEADLINE_REACHED)
        terminate(); /* ERROR */ wait1(10); /* task2 börjar köra */
    /* start test 3 */
    if(send_wait(mb,&nData_t1)==DEADLINE_REACHED) {
        if( no_messages(mb) != 0 )
            terminate(); /* ERROR */
        nTest3 = 1;
        if (nTest1*nTest2*nTest3) {
            /* Blinka lilla lysdiod */
            /* Test ok! */
        }
        terminate(); /* PASS, no receiver */ }
    else
    {
        terminate(); /* ERROR */ }
}
void task2(void){
    int nData_t2 = 0;
    if(receive_wait(mb,&nData_t2) ==
       DEADLINE_REACHED) /* t1 kör nu */ terminate(); /* ERROR */
    if( no_messages(mb) != 0 ) terminate(); /* ERROR */
    if (nData_t2 == TEST_PATTERN_1) nTest1 = 1; wait1(20); /* t1 kör nu */
    /* start test 2 */
    if( no_messages(mb) != 1 ) terminate(); /* ERROR */
    if(receive_wait(mb,&nData_t2) == DEADLINE_REACHED) /* t1 kör nu */
        terminate(); /* ERROR */ if( no_messages(mb) != 0 )
            terminate(); /* ERROR */
    if (nData_t2 == TEST_PATTERN_2) nTest2 = 1;
    /* Start test 3 */
    terminate();
}