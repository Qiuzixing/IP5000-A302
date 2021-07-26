/*
 * File     : $RCSfile$
 * Created  : January 2007
 * Updated  : $Date$
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : strcasecmp implementation
 *
 *  * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 *
 * Audinate Copyright Header Version 1
 *
 *	Implementation of strcasecmp for libc that are missing it.
 */

#include <ctype.h>
 
int
strcasecmp(const char * s1, const char * s2)
{
	char * p1 = (char *) s1;
	char * p2 = (char *) s2;
	while (*p1 && *p2 && (tolower(*p1)==tolower(*p2)))
	{
		p1++;
		p2++;
	}
	return tolower(*p1) - tolower(*p2);
}
