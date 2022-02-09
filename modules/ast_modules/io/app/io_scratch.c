/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/socket.h>
#include "ioaccess.h"

int fd;

void WriteMMIOLong (unsigned long Offset, unsigned long Data)
{
    IO_ACCESS_DATA      IO_Data;
    
    memset (&IO_Data, 0, sizeof(IO_ACCESS_DATA));
    int    fd = open ("/dev/access", O_RDONLY);

    IO_Data.Address = Offset;
    IO_Data.Data = Data;
    ioctl (fd, IOCTL_IO_WRITE, &IO_Data);
    close (fd);    
}

unsigned long ReadMMIOLong (unsigned long Offset)
{
    IO_ACCESS_DATA      IO_Data;
    unsigned long       Status, Return_Data;
    
    memset (&IO_Data, 0, sizeof(IO_ACCESS_DATA));
    int    fd = open ("/dev/access", O_RDONLY);

    IO_Data.Address = Offset;
    Status = ioctl (fd, IOCTL_IO_READ, &IO_Data);
    close (fd);
    Return_Data = IO_Data.Data;
    
    return   Return_Data ;
}

int  main ()
{
    IO_ACCESS_DATA      IO_Data;
    unsigned long       temp, SCU40;

    memset (&IO_Data, 0, sizeof(IO_ACCESS_DATA));

    fd = open ("/dev/access", O_RDONLY);

//We use SCU offset 0x7C register to determine if it is AST1000/AST2000 series product
    temp = ReadMMIOLong (0x1E6E207C);
    if (temp == 0) { //AST1000/AST2000
	SCU40 = ReadMMIOLong (0x1E6E0140);
	WriteMMIOLong (0x1E6E0140, SCU40 | (0x5A << 24));
    }
    else { //AST1100/AST2050/AST2100
	SCU40 = ReadMMIOLong (0x1E6E2040);
	WriteMMIOLong (0x1E6E2040, SCU40 | (0x5A << 24));
    }

    close (fd);

    return 1;
}
