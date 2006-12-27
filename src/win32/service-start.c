/* @(#) $Id$ */

/* Copyright (C) 2006 Daniel B. Cid <dcid@ossec.net>
 * All rights reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */
       

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* Setup windows after install */
int main(int argc, char **argv)
{
    printf("%s: Attempting to start ossec.", argv[0]);

    system("net start OssecSvc");
    
    system("pause");
    return(0);
}
