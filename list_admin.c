//
//  linkedlist.c
//  mOS DSTII
//
//  Created by Markus Axelsson and Oskar Lundgren on 2015-02-06.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#include "list_admin.h"

#include <assert.h>
#include "main.h"
#include "memwatch.h"

/** Variables **/
list *g_readylist;
list *g_timerlist;
list *g_waitinglist;

/** Functions **/

list* create_list(void){
  list *new_list;
    new_list = malloc(sizeof(list));
    new_list->pHead = malloc( sizeof(listobj) );
    new_list->pTail = malloc( sizeof(listobj) );
    
    if(new_list == 0 || new_list->pHead == 0 || new_list->pTail == 0){
        free(new_list);
        free(new_list->pHead);
        free(new_list->pTail);
        return NULL;
    }
    else{
        new_list->pHead->pNext = new_list->pTail;
        new_list->pTail->pPrevious = new_list->pHead;
        return new_list;
    }
}

exception insert_timerlist(listobj *insert_object){

    listobj *temp = g_timerlist->pHead;
    while(temp->pNext != g_timerlist->pTail && temp->pNext->nTCnt < insert_object->nTCnt) {
          temp = temp->pNext;
  }

	
	insert_object->pNext = temp->pNext;
	insert_object->pPrevious = temp;
	temp->pNext = insert_object;
	insert_object->pNext->pPrevious = insert_object;
  
        
        return OK;
}

listobj * extract_timerlist(void){
    listobj * returnObject;
    if(g_timerlist->pHead->pNext == g_timerlist->pTail){
        exit(0);
    }
    else{
        returnObject = g_timerlist->pHead->pNext;
        g_timerlist->pHead->pNext = g_timerlist->pHead->pNext->pNext;
        return returnObject;
    }
}

exception push_list(list *list, listobj * object){

        listobj *tempObject;
        tempObject = list->pHead;
        while ((tempObject->pNext != list->pTail) && (tempObject->pNext->pTask->DeadLine < object->pTask->DeadLine)) {
            tempObject = tempObject->pNext;
        }
        object->pNext = tempObject->pNext;
        object->pPrevious = tempObject;
        tempObject->pNext = object;
        object->pNext->pPrevious = object;
        
	return OK;
}


listobj* pop_list(list *target){

  if(target == NULL){
          return NULL;
  }
  if(target->pHead->pNext == target->pTail){
		return NULL;
  }
	listobj* returnObject = target->pHead->pNext;
  
	target->pHead->pNext = returnObject->pNext;
	returnObject->pNext->pPrevious = target->pHead;
	returnObject->pNext = NULL;
	returnObject->pPrevious = NULL;
	return returnObject;

}


exception extract_waitinglist(listobj *object){
  
	
  if(object==NULL||object->pNext==NULL||object->pPrevious==NULL){
  
      return FAIL;
  }
    
	object->pNext->pPrevious = object->pPrevious;
        object->pPrevious->pNext = object->pNext;
        object->pNext = NULL;
	object->pPrevious = NULL;

    return OK;

}

listobj* extract_readylist(void){
    listobj * returnObject;
    if (g_readylist->pHead->pNext == g_readylist->pTail) {
        return NULL;
    }
    else{
        returnObject = g_readylist->pHead->pNext;
        
        g_readylist->pHead->pNext = returnObject->pNext;
        returnObject->pNext->pPrevious = g_readylist->pHead;
	returnObject->pNext = NULL;
	returnObject->pPrevious = NULL;
        return returnObject;
    }
}

void delete_list(list **target){
	
	listobj *temp = (*target)->pHead;
	
	while(temp->pNext != (*target)->pTail){
	
		temp = temp->pNext;
		
		free(temp->pPrevious);
		
	}
	
	free((*target)->pTail);
	free(*target);
	*target = NULL;
	
}

listobj* peek_list(list *target){
  
  if(target == NULL){
  
    return NULL;
  }
  
  if(target->pHead->pNext == target->pTail){
  
    return NULL;
  }
  
  
	return target->pHead->pNext;

}


