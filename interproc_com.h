//
//  interproc_com.h
//  mOS
//
//  Created by Markus Axelsson on 2015-02-13.
//  Copyright (c) 2015 Markus & Oskar. All rights reserved.
//

#ifndef __mOS__interproc_com__
#define __mOS__interproc_com__

#include <stdio.h>
#include "kernel.h"

exception push_mailbox(mailbox *mBox, msg *message);
msg* pop_mailbox(mailbox *mBox);
void remove_message(msg *new_message);
mailbox* create_mailbox( uint nMessages, uint nDataSize );
exception remove_mailbox( mailbox * mBox);
exception send_wait(mailbox* mBox, void* pData);
exception recieve_wait(mailbox *mBox, void *data);
exception send_no_wait(mailbox *mBox, void *data);
int receive_no_wait( mailbox* mBox, void* pData );

#endif /* defined(__mOS__interproc_com__) */