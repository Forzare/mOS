//
//  linkedlist.h
//  mOS DSTII
//
//  Created by Markus Axelsson and Oskar Lundgren  on 2015-02-06.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#ifndef __mOS_DSTII__linkedlist__
#define __mOS_DSTII__linkedlist__

#include <stdio.h>
#include "kernel.h"

/** Definitions **/
extern list *g_readylist;
extern list *g_timerlist;
extern list *g_waitinglist;

/** Functions **/
list* create_list(void);


/*Inserting objects in lists*/
int insert_timerlist(listobj *insert_object);
exception push_list(list *list, listobj *insert_object);
listobj* pop_list(list* target);

/*Extracting objects from lists*/
listobj* extract_timerlist(void);
exception extract_waitinglist(listobj *object);
listobj* extract_readylist(void);

listobj* peek_list(list *target);


#endif /* defined(__mOS_DSTII__linkedlist__) */
