/* strnndictcmp */

/* string compare using dictionary order */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is similar to the 'strcmp(3c)' subroutine except that
	with this subroutine all comparisions are done using "dictionary"
	order.  Dictionary order only compares characters that are:

		letters
		digits
	
	Also, upper and lower case are mostly ignored except that upper case
	still comes before lower case.

	Synopsis:

	int strnndictcmp(s1,s2)
	const char	s1[], s2[] ;

	Arguments:

	s1	one string
	s2	second string

	Returns:

	>0	the first string is bigger than the second
	0	both strings are equal (as compared)
	<0	first string is less than the second


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define	DCH	struct dictchar


/* external subroutines */

extern int	strwcmp(const char *,const char *,int) ;
extern int	isalnumlatin(int) ;
extern int	isdict(int) ;


/* external variables */


/* local structures */

struct dictchar {
	const uchar	*sp ;
	int		sl ;
} ;


/* forward references */

static int	dch_start(DCH *,const char *,int) ;
static int	dch_finish(DCH *) ;
static int	dch_get(DCH *) ;

static int	strdocmp(const char *,const char *,int) ;


/* exported subroutines */


int strnndictcmp(cchar *s1,int s1len,cchar *s2,int s2len)
{
	DCH		dc1, dc2 ;
	int		rs ;
	int		i ;
	int		ch1, ch2 ;
	int		fch1, fch2 ;
	int		rc = 0 ;

	if (s1len < 0) s1len = strlen(s1) ;
	if (s2len < 0) s2len = strlen(s2) ;

#if	CF_DEBUGS
	debugprintf("strnndictcmp: s1=>%t<\n",s1,s1len) ;
	debugprintf("strnndictcmp: s2=>%t<\n",s2,s2len) ;
#endif

	if ((rs = dch_start(&dc1,s1,s1len)) >= 0) {
	    if ((rs = dch_start(&dc2,s2,s2len)) >= 0) {

	        for (i = 0 ; rc == 0 ; i += 1) {

	            ch1 = dch_get(&dc1) ;
	            ch2 = dch_get(&dc2) ;

#if	CF_DEBUGS
	            debugprintf("strnndictcmp: ch1=%c(%02x) ch2=%c(%02x)\n",
	                ch1,ch1,ch2,ch2) ;
#endif

	            fch1 = CHAR_TOFC(ch1) ;
	            fch2 = CHAR_TOFC(ch2) ;

	            rc = (fch1 - fch2) ;
	            if ((ch1 == 0) && (ch2 == 0)) /* end-of-string for both */
	                break ;

	        } /* end while */

#if	CF_DEBUGS
	        debugprintf("strnndictcmp: 0 rc=%d\n",rc) ;
#endif

	        if (rc == 0) {
	            rc = (s1len - s2len) ; /* more determinism */
		}

	        if (rc == 0) {
	            rc = strdocmp(s1,s2,i) ; /* break ties so far */
		}

#if	CF_DEBUGS
	        debugprintf("strnndictcmp: 1 rc=%d\n",rc) ;
#endif

	        if (rc == 0) {
	            while (s1len && (! isdict(s1[s1len-1]))) s1len -= 1 ;
	            while (s2len && (! isdict(s2[s2len-1]))) s2len -= 1 ;
	            rc = (s1len - s2len) ; /* more determinism */
	        }

#if	CF_DEBUGS
	        debugprintf("strnndictcmp: 2 rc=%d\n",rc) ;
#endif

	        if (rc == 0) {
	            rc = strncmp(s1,s2,s2len) ; /* absolute determinism (?) */
		}

#if	CF_DEBUGS
	        debugprintf("strnndictcmp: 3 rc=%d\n",rc) ;
#endif

	        dch_finish(&dc2) ;
	    } /* end if (dc2) */
	    dch_finish(&dc1) ;
	} /* end if (dc1) */

#if	CF_DEBUGS
	debugprintf("strnndictcmp: ret rc=%d\n",rc) ;
#endif

	return rc ;
}
/* end subroutine (strnndictcmp) */


/* local subroutines */


static int dch_start(DCH *dcp,const char *sp,int sl)
{
	dcp->sp = (const uchar *) sp ;
	dcp->sl = sl ;
	return 0 ;
}
/* end subroutine (dch_start) */

static int dch_finish(DCH *dcp)
{
	int		sl = dcp->sl ;
	return sl ;
}
/* end subroutine (dch_finish) */

static int dch_get(DCH *dcp)
{
	int		doch ;
/* CONSTCOND */
	while (TRUE) {
	    doch = 0 ;
	    if (dcp->sl == 0)
	        break ;
	    doch = (dcp->sp[0] & UCHAR_MAX) ;
	    if (doch == 0)
	        break ;
	    dcp->sp += 1 ;
	    dcp->sl -= 1 ;
	    if (isdict(doch))
	        break ;
	} /* end while */
	return doch ;
}
/* end subroutine (dch_get) */


static int strdocmp(const char *s1,const char *s2,int slen)
{
	int		i ;
	int		do1 ;
	int		do2 ;
	int		rc = 0 ;

#if	CF_DEBUGS
	debugprintf("strnndictcmp/strdocmp: s1en=%u\n",slen) ;
#endif

	for (i = 0 ; (i < slen) && *s1 && *s2 ; i += 1) {
	    do1 = 0 ;
	    while (*s1 && ((do1 = CHAR_DICTORDER(*s1)) == 0)) s1 += 1 ;
	    do2 = 0 ;
	    while ((i < slen) && *s2 && ((do2 = CHAR_DICTORDER(*s2)) == 0)) {
	        s2 += 1 ;
	        i += 1 ;
	    }
	    rc = (do1 - do2) ;

#if	CF_DEBUGS
	    debugprintf("strnndictcmp/strdocmp: i=%u c1=%c c2=%c\n",
	        i,*s1,*s2) ;
	    debugprintf("strnndictcmp/strdocmp: do1=%u do2=%u\n",
	        do1,do2) ;
#endif

	    if (rc != 0)
	        break ;
	    if (*s1) s1 += 1 ;
	    if (*s2) s2 += 1 ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("strnndictcmp/strdocmp: ret rc=%d\n",rc) ;
#endif

	return rc ;
}
/* end subroutine (strdocmp) */


