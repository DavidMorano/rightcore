/* bsd */


/* revision history:

	= 2017-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<string.h>
#include	<localmisc.h>



void bcopy(cchar *s1,char *s2,int slen)
{
	(void) memcpy(s2,s1,(size_t) slen) ;
}


int bcmp(cchar *s1,cchar *s2,int slen)
{
	return memcmp(s1,s2,(size_t) len) ;
}


void bzero(char *sp,int slen)
{
	(void) memset(sp,0,(size_t) slen) ;
}


