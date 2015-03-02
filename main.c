//
//  main.c
//  mOS DSTII
//
//  Created by Markus Axelsson on 2015-02-06.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#include <stdio.h>
#include "list_admin.h"
#include "main.h"



int g_running_mode = FALSE;
int g_firstrun = TRUE;
TCB *Running = NULL;
uint TC;



void x(){

  int y = 5;

}

int main(void) {
    TC = 0;
    init_kernel();
    create_task(x,1000);
    run();
    
    
    return 0;
}
