/* mkgecosname */

/* fix the GECOS name as it comes out of the password file ('/etc/passwd') */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_GECOSHYPHEN	1		/* handle hyphen characters in name */


/* revision history:

	= 1998-08-12, David A­D­ Morano
        This subroutine was originally adopted from a similar subroutine that
        was a part of the PCS package distribution. However, the original was
        garbage so this version is essentially a complete rewrite of the
        original. Also the name of the subroutine is changed from the original.
        The original name of the subroutine was GECOSNAME.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
*									
*	FUNCTIONAL DESCRIPTION:						

	This subroutine gets the GECOS version of a user name (a type of a real
	name) from the fifth field of the system password file
	('/etc/passwd').

	Synopsis:

	int mkgecosname(rbuf,rflen,gf)
	char		rbuf[] ;
	int		rlen ;
	const char	gf[] ;

	Arguments:

	rbuf		user supplied result buffer to contain the full-name
	rlen		length of the user supplied result buffer
	gf		input buffer containing the GECOS field string

	Returns:

	>=0	length of GECOS name
	<0	failed 


*	SUBROUTINES CALLED:						
*		
		getgecosname(3dam)
*									
*	GLOBAL VARIABLES USED:						
*		None!  AS IT SHOULD BE!
*									

	Extra Notes:

	The GECOS field of the PASSWD database should be formatted in one of
	the following ways:

	    name,office,work_phone,home_phone
	    organization-name(account,bin)office,work_phone,home_phone,printer
	    name(account)office,work_phone,home_phone,printer
	    name

	Note also that an ampersand character ('&') that appears anywhere in
	the GCOS field is to be logically replaced by the corresponding
	username of the entry.

	The 'name' part of the GCOS entry may contain hyphen ('-') characters.

	The original AT&T GECOS field contained:

	    department-name(account,bin)

	and was further put into a 'struct comment' with fields:

	    c_dept
	    c_name
	    c_acct
	    c_bin

	Some modern possibilities for the GECOS field are:

	    org_dept-name(account,bin)office,work_phone,home_phone
	    org_dept-name(account,bin)office,work_phone,home_phone,printer

	Actual examples:

	    XNR64430-d.a.morano(126483,BIN8221)
	    rockwell-d.a.morano(126283,BIN8221)4B-411,5336,6175679484,hp0

	Design note:

	Note that in the 'getgecosname(3dam)' subroutine below, we use the
	subroutine 'strnlen(3dam)' to get the length of the supplied GECOS
	string length if one was not also given.  This is done in order to
	guard against the problem of an indefinitely long GECOS string being
	given to us (in order words: garbage) for whatever reason.

	I don't really know how an indefinitely long GECOS string -- and one
	that doesn't have traditional GECOS special characters in it -- could
	get passed to us from reasonable callers.  But then again, not all of
	our callers are always reasonable!

	Our goal is to prevent stray memory references -- in order to avoid
	core dumps!  Someone else can have core dumps if they want them -- but
	we do not!  Deal with it!

	If everyone guarded their subroutines the way that we try to do, maybe
	there wouldn't be so much crap out there -- like all of the core
	dumping that is going on!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	CH_LPAREN
#define	CH_LPAREN	0x28
#endif

#ifndef	MAXGECOSLEN
#define	MAXGECOSLEN	1024
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	snwcpyhyphen(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* local structures */


/* forward references */

int		getgecosname(const char *,int,const char **) ;


/* local variables */


/* exported subroutines */


int mkgecosname(char *rbuf,int rlen,cchar *gf)
{
	int		rs = SR_NOTFOUND ;
	int		cl ;
	const char	*cp ;

	if (rbuf == NULL) return SR_FAULT ;
	if (gf == NULL) return SR_FAULT ;

	if ((cl = getgecosname(gf,-1,&cp)) >= 0) {

#if	CF_GECOSHYPHEN
	    rs = snwcpyhyphen(rbuf,rlen,cp,cl) ;
#else
	    rs = snwcpy(rbuf,rlen,cp,cl) ;
#endif

	} /* end if (getgecosname) */

	return rs ;
}
/* end subroutine (mkgecosname) */


int getgecosname(cchar *gbuf,int glen,cchar **rpp)
{
	int		cl = 0 ;
	const char	*tp ;
	const char	*cp ;

	if (gbuf == NULL) return SR_FAULT ;

	if (glen < 0) glen = strnlen(gbuf,MAXGECOSLEN) ;

/* find the desired part (the realname) of the GECOS field */

	if (((cp = strnchr(gbuf,glen,'-')) != NULL) &&
	    ((tp = strnchr(cp,(glen - (cp - gbuf)),CH_LPAREN)) != NULL)) {

	    cp += 1 ;
	    cl = (tp - cp) ;

	} else if ((tp = strnchr(gbuf,glen,CH_LPAREN)) != NULL) {

	    cp = gbuf ;
	    cl = (tp - gbuf) ;

	} else if ((tp = strnchr(gbuf,glen,'-')) != NULL) {

	    cp = (tp + 1) ;
	    cl = glen - ((tp + 1) - gbuf) ;

	} else {

	    cp = gbuf ;
	    cl = glen ;

	} /* end if */

	if (rpp != NULL)
	    *rpp = cp ;

	return cl ;
}
/* end subroutine (getgecosname) */


