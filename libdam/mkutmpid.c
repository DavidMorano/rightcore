/* mkutmpid */

/* make a UTMP ID value */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-10-09, David A­D­ Morano
        I finally made this because so many programs have difficulty in creating
        UTMP ID values.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to create UTMP ID values.  These values are
	(generally) four-character strings (without terminating NULs) that are
	used in the ID field of the UTMP record.  Many programs often struggle
	to create correct or valid IDs that follow historic UNIX conventions,
	so we are trying to help out with this subroutine!

	Special note: This subroutine will not NUL terminate the ID buffer if
	the created ID is the maximum length of the supplied buffer.  This is
	fine when the buffer is the exact buffer of the ID field in a UTMP
	record (for example) but this is not good if you want to try to print
	the buffer out as a normal string.  Supply a buffer one character
	larger than needed and NUL terminate it at the end if you want to treat
	the resulting buffer as a normal C-language string (NUL terminated).

	Note on IDs:

	Certain ID prefixes are reserved.  Generally, and in the "old" days,
	IDs without any leading alphabetic characters were taken to be terminal
	logins.  Other IDs with lower-case alphabetic letter prefixes were
	other IDs usually from the INITTAB file that were not associated with
	terminal logins.  However, with the advent of "pseudo" logins using
	pseudo terminals, other schemes for assigning IDs needed to be
	developed since most BSD-like pseudo terminal logins occurred without
	using any mechanism derived from the INIT program at all!

	The following have been in use (but are not necessarily endorsed
	here):

	r		RLOGIN
	t		TELNET

	The following are strongly recommended for all future use:

	P		any pseudo terminal with a device prefix 'pts'
	R		any pseudo terminal with a device prefix 'rx'
	X		any pseudo terminal with a device prefix 'xt'

	It is recommended that both RLOGIN and TELNET convert over to using the
	upper case prefixes as above.

	Also, note that lower case prefixes may be used by other services that
	use the old BSD-style pseudo terminals that have device names starting
	with a lower case letter.  For example, a pseudo terminal with a device
	file name of 'ttyp8' would get an ID of 'p8'.  See the problem!
	Services that use these terminal devices are naturally going to be
	using lower case letters between about 'p' and up to possibly 'z',
	depending on how many of these pseudo terminals are configured into the
	system.  It is strongly recommended that lower case letter IDs from
	INITAB not use lower case letters in this range.  This gives preference
	to the BSD-style pseudo terminals, but so be it.  They need IDs also
	and it is not clear what to do for them anyway.  Many services have
	algorithms that take the device name (like 'ttyp8') and strips the
	'tty' part off and then uses the remainder for the ID.  This seems
	reasonable to keep since at least the range of lower case letters used
	is manageable.

	It seems reasonable that ID usage with additional upper case alphabetic
	characters should be considered for future needs.

	Synopsis:

	int mkutmpid(idbuf,idlen,termbuf,tlen)
	char		idbuf[] ;
	int		idlen ;
	const char	termbuf[] ;
	int		tlen ;

	Arguments:

	idbuf		user supplied buffer to receive result
	idlen		length of user supplied buffer
	termbuf		buffer holding terminal device string
	tlen		length of terminal string

	Returns:

	>=0		OK, and length of resulting ID string
	<0		some error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct prefix {
	char		*name ;
	char		*prefix ;
} ;


/* forward references */

static int	idcpy(char [],int,const char [],int,const char [],int) ;


/* local variables */

static const struct prefix	prefixes[] = {
	{ "pts", "P" },
	{ "term", "" },
	{ "tty", "" },
	{ "rx", "R" },
	{ "xt", "X" },
	{ NULL, NULL }
} ;

static const struct prefix	specials[] = {
	{ "console", "co" },
	{ "syscon", "cs" },
	{ NULL, NULL }
} ;


/* exported subroutines */


int mkutmpid(char *idbuf,int idlen,cchar *devbuf,int devlen)
{
	int		rs = SR_OK ;
	int		i ;
	int		sl, cl, pl, ll ;
	const char	*sp, *cp, *lp, *pp ;

#if	CF_DEBUGS
	debugprintf("mkutmpid: ent termdev=%t\n",
	    devbuf,devlen) ;
#endif

	if (idlen < 0)
	    idlen = 4 ;

	memset(idbuf,0,idlen) ;

	lp = devbuf ;
	ll = strnlen(devbuf,devlen) ;

	if (strncmp(devbuf,"/dev/",5) == 0) {
	    lp += 5 ;
	    ll -= 5 ;
	}

	if (ll < 1) {
	    rs = SR_INVALID ;
	}

/* handle subdirectories first */

#if	CF_DEBUGS
	debugprintf("mkutmpid: directory match line=%t\n",lp,ll) ;
#endif

	if (rs >= 0) {
	    if ((cp = strnchr(lp,ll,'/')) != NULL) {

	        sp = lp ;
	        sl = (cp - lp) ;

#if	CF_DEBUGS
	        debugprintf("mkutmpid: dir=%t\n",
	            sp,strnlen(sp,sl)) ;
#endif

	        cp += 1 ;
	        cl = ll - (cp - lp) ;

#if	CF_DEBUGS
	        debugprintf("mkutmpid: remainder=%t\n",
	            cp,strnlen(cp,cl)) ;
#endif

	        for (i = 0 ; prefixes[i].name != NULL ; i += 1) {
	            pp = prefixes[i].name ;
	            pl = strlen(pp) ;
	            if ((pl == sl) && (strncmp(pp,sp,pl) == 0)) break ;
	        } /* end for */

	        if (prefixes[i].name != NULL) {
	            pp = prefixes[i].prefix ;
	            pl = strlen(pp) ;
	            rs = idcpy(idbuf,idlen,pp,pl,cp,cl) ;
	        } /* end if */

	    } /* end if (tried for a directory match) */
	} /* end if (ok) */

#if	CF_DEBUGS
	if (idbuf[0] == '\0')
	    debugprintf("mkutmpid: prefix match\n") ;
#endif

	if ((rs >= 0) && (idbuf[0] == '\0')) {

	    cp = lp ;
	    cl = ll ;

#if	CF_DEBUGS
	    debugprintf("mkutmpid: remainder=%t\n",
	        cp,strnlen(cp,cl)) ;
#endif

	    for (i = 0 ; prefixes[i].name != NULL ; i += 1) {

	        pp = prefixes[i].name ;
	        pl = strlen(pp) ;

	        if ((pl <= cl) && (strncmp(pp,lp,pl) == 0))
	            break ;

	    } /* end for */

	    if (prefixes[i].name != NULL) {
	        pp = prefixes[i].prefix ;
	        pl = strlen(pp) ;
	        rs = idcpy(idbuf,idlen,pp,pl,cp,cl) ;
	    } /* end if */

	} /* end if (raw prefix match) */

/* are we a "special"? */

	if ((rs >= 0) && (idbuf[0] == '\0')) {

	    cp = lp ;
	    cl = ll ;

	    for (i = 0 ; specials[i].name != NULL ; i += 1) {
	        pp = specials[i].name ;
	        if (strcmp(pp,lp) == 0) break ;
	    } /* end for */

	    if (specials[i].name != NULL) {
	        pp = specials[i].prefix ;
	        pl = strlen(pp) ;
	        rs = idcpy(idbuf,idlen,pp,pl,NULL,0) ;
	    } /* end if */

	} /* end if (special terminal devices) */

/* if we still have failed, take the last TMPX_LID characters */

#if	CF_DEBUGS
	if (idbuf[0] == '\0')
	    debugprintf("mkutmpid: trailing characters\n") ;
#endif

	if ((rs >= 0) && (idbuf[0] == '\0')) {

#if	CF_DEBUGS
	    debugprintf("mkutmpid: trailing characters, ll=%u lp=%t\n",
	        ll,
	        lp,strnlen(lp,ll)) ;
#endif

	    if ((cp = strnchr(lp,ll,'/')) != NULL) {
	        cp += 1 ;
	        cl = ll - (cp - lp) ;
	    } else {
	        cp = lp ;
	        cl = ll ;
	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("mkutmpid: cl=%u cp=%t\n",cl,
	        cp,strnlen(cp,cl)) ;
#endif

	    if (cl > idlen) {
	        rs = idcpy(idbuf,idlen,NULL,0,(cp+(cl-idlen)),idlen) ;
	    } else {
	        rs = idcpy(idbuf,idlen,NULL,0,cp,cl) ;
	    }

	} /* end if (taking last several characters in deperation) */

#if	CF_DEBUGS
	debugprintf("mkutmpid: idbuf=%t\n",
	    idbuf,strnlen(idbuf,rs)) ;
#endif

	return rs ;
}
/* end subroutine (mkutmpid) */


/* local subroutines */


static int idcpy(char *idbuf,int idlen,cchar *pp,int pl,cchar *cp,int cl)
{
	int		rs ;
	int		k ;
	int		j = 0 ;

#if	CF_DEBUGS
	debugprintf("idcpy: prefix=>%t<\n",
	    pp,strnlen(pp,pl)) ;
	debugprintf("idcpy: remainder=>%t<\n",
	    cp,strnlen(cp,cl)) ;
#endif

	for (k = 0 ; (j < idlen) && (k < pl) ; j += 1) {
	    idbuf[j] = pp[k++] ;
	}

	for (k = 0 ; (j < idlen) && (k < cl) ; j += 1) {
	    idbuf[j] = cp[k++] ;
	}

	rs = j ;
	while (j < idlen) {
	    idbuf[j++] = '\0' ;
	}

	return rs ;
}
/* end subroutine (idcpy) */


