/* main */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * David Hitz of Auspex Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n" ;
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)look.c	8.1 (Berkeley) 6/14/93" ;
#endif /* not lint */


/*
 * look -- find lines in a sorted list.
 *
 * The man page said that TABs and SPACEs participate in -d comparisons.
 * In fact, they were ignored.  This implements historic practice, not
 * the manual page.
 */


#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <limits.h>
#include <locale.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	__STDC__
#define	__STDC__	1
#endif

#define	__P(A)		A

#ifndef	SIZE_T_MAX
#define	SIZE_T_MAX	(4 * 1024 * 1024)
#endif

#ifndef	WORDF
#define	WORDF	"share/dict/words"
#endif

/*
 * FOLD and DICT convert characters to a normal form for comparison,
 * according to the user specified flags.
 *
 * DICT expects integers because it uses a non-character value to
 * indicate a character which should not participate in comparisons.
 */
#define	EQUAL		0
#define	GREATER		1
#define	LESS		(-1)
#define NO_COMPARE	(-2)

#define FOLD(c) (isupper(c) ? tolower(c) : (unsigned char) (c))
#define DICT(c) (isalnum(c) ? (c) & 0xFF /* int */ : NO_COMPARE)



/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;


/* forward references */

static int	strtlen(const char *) ;

static int	look(uchar *,uchar *,uchar *) ;

uchar    *binary_search 
__P((unsigned char *, unsigned char *, unsigned char *)) ;

uchar    *linear_search 
__P((unsigned char *, unsigned char *, unsigned char *)) ;

int      compare __P((unsigned char *, unsigned char *, unsigned char *)) ;
void	 err __P((const char *fmt, ...)) ;

void     print_from __P((unsigned char *, unsigned char *, unsigned char *)) ;

static void usage __P((void)) ;


/* variables */

int dflag, fflag ;

static const char	*progroots[] = {
	    "LOCAL",
	    "NCMP",
	    "GNU",
	    NULL
} ;

static const char	*wordfiles[] = {
	    "/usr/add-on/local/share/dict/words",
	    "/usr/add-on/ncmp/share/dict/words",
	    "/usr/add-on/gnu/share/dict/words",
	    "/usr/share/lib/dict/words",
	    "/usr/dict/words",
	    NULL
} ;






int main(argc, argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	sb ;

	int	ex ;
	int	fd_debug = -1 ;

	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	ex = b_look(argc,argv,NULL) ;

	return ex ;
}
/* end subroutine (main) */



