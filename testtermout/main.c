/* main (C++98) */

/* test program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_FOLLOWFILES	0		/* follow sybolic links of files */
#define	CF_FTCASE	1		/* try a C-lang 'switch' */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a fairly generic front-end subroutine for a program.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<sigblock.h>
#include	<bwops.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"termout.h"

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	(MAXPATHLEN + 20)
#endif

#define	DMODE		0775


/* external subroutines */

extern "C" int	mainer(int,const char **,const char **) ;


int main(int argc,const char **argv,const char **envv) {
	return mainer(argc,argv,envv) ;
}



