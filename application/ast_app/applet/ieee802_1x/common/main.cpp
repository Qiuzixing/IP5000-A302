
#include <stdio.h>

#include "Exec.h"

int main(int argc, char **argv)
{
	int i;
	Exec test;
	i = test.RunProgram("/home/longchanglin/work/8021_portting/SC009_802X_MODULE/app/802_1x/common/", "hello world ok fine", 1);
	printf("i=%d\n", i);
}


