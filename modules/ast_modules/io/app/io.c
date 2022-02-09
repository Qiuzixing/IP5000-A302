/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ioaccess.h"

int  main (int argc, char **argv)
{
    int    temp = 0, test;
    char   *stop_at;
    IO_ACCESS_DATA   IO_Data;
    unsigned long Linear;
    

    memset (&IO_Data, 0, sizeof(IO_ACCESS_DATA));

    int    fd = open ("/dev/access", O_RDONLY);
    IO_Data.Type = strtoul (argv[1], &stop_at, 16);
    if (IO_Data.Type == 0) {
        IO_Data.Address = strtoul (argv[2], &stop_at, 16);
    }
    else if (IO_Data.Type == 1) {
        IO_Data.Address = strtoul (argv[2], &stop_at, 16);
        IO_Data.Data = strtoul (argv[3], &stop_at, 16);
    }
    else if (IO_Data.Type == 8) {
        IO_Data.Address = strtoul (argv[2], &stop_at, 16);
        IO_Data.Data = strtoul (argv[3], &stop_at, 16);
    }
    else if (IO_Data.Type == 9) {
        IO_Data.Address = strtoul (argv[2], &stop_at, 16);
        IO_Data.Data = strtoul (argv[3], &stop_at, 16);
        IO_Data.Value = strtoul (argv[4], &stop_at, 16);
    }

    if (IO_Data.Type == 0) {
        test = ioctl (fd, IOCTL_IO_READ, &IO_Data);
    }
    else if (IO_Data.Type == 1) {
        test = ioctl (fd, IOCTL_IO_WRITE, &IO_Data);
    }
    else if (IO_Data.Type == 8) {
        test = ioctl (fd, IOCTL_MEMORY_OVER_READ, &IO_Data);
    }
    else if (IO_Data.Type == 9) {
        test = ioctl (fd, IOCTL_MEMORY_OVER_WRITE, &IO_Data);
    }

    if (IO_Data.Type != 2) {
        if (IO_Data.Type == 0) {
            printf ("Data = %x\n", IO_Data.Data);
        }
    }
    close (fd);

    return 1;
}
