/* xenv */

/* load environment-like strings from a file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-09-10, David A.D. Morano
	This program was originally written.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This set of subroutines are used to access an "alternate" environment
        space (according to some Solaris blurbs). This is mostly (as far as I
        can tell) used for mail-related programs.

        The principal subroutine to initialize this facility is 'xsetenv' and it
        will read (process) a file and put all of the unique strings (in the
        form of key-value pairs) found into a string space that can later be
        searched by a subroutine such as 'xgetenv'.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<field.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */

#undef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX((2 * MAXPATHLEN),LINE_MAX)
#else
#define	LINEBUFLEN	MAX((2 * MAXPATHLEN),2048)
#endif


/* external subroutines */

extern int	strkeycmp(const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	mktable(vecstr *) ;


/* local variables */

static char	**xenviron = NULL ;

static const uchar	fterms[32] = {
	0x00, 0x02, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int xsetenv(fname)
const char	fname[] ;
{
	FILE	*fp ;

	FIELD	fsb ;

	vecstr	list ;

	const int	llen = LINEBUFLEN ;
	int	rs = 0 ;
	int	rs1 ;
	int	i, n, len ;
	int	kl, vl ;
	int	opts ;

	const char	*kp, *vp ;

	char	lbuf[LINEBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("xsetenv: fname=%s\n",fname) ;
#endif

	if ((fname == NULL) || (fname[0] == '\0'))
	    return -1 ;

/* free-up any previous */

	if (xenviron != NULL) {
	    uc_free(xenviron) ;
	    xenviron = NULL ;
	}

/* try to open it */

	fp = fopen(fname,"r") ;
	if (fp == NULL) {
	    rs = -1 ;
	    goto ret0 ;
	}

	opts = VECSTR_OSTSIZE ;
	if ((rs = vecstr_start(&list,10,opts)) >= 0) {
	n = 0 ;
	while ((rs = freadline(fp,lbuf,llen)) > 0) {
	    len = rs ;

	    if (lbuf[len - 1] == '\n') len -= 1 ;

	    if ((len == 0) || (lbuf[0] == '#'))
	        continue ;

#if	CF_DEBUGS
	    debugprintf("xsetenv: line>%w<\n",lbuf,len) ;
#endif

	    if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	        if ((kl = field_get(&fsb,fterms,&kp)) > 0) {
	            kp[kl] = '\0' ;
	            vp = NULL ;
	            vl = 0 ;
	            if (fsb.term != '#') {
	                vl = field_get(&fsb,fterms,&vp) ;
		    }
	            n += 1 ;
	            rs = vecstr_envadd(&list,kp,vp,vl) ;
	        } /* end if (got one) */
	        field_finish(&fsb) ;
	    } /* end if (initialized) */

	            if (rs < 0) break ;
	} /* end while (reading lines) */
/* copy everything to a single buffer (memory allocations not freed) */
	mktable(&list) ;
	vecstr_finish(&list) ;
	} /* end if (vecstr) */

ret1:
	fclose(fp) ;

ret0:
	return (rs >= 0) ? 1 : 0 ;
}
/* end subroutine (xsetenv) */


/* return a x-environment variable value */
char *xgetenv(name)
const char	name[] ;
{
	int	i ;
	char	*cp = NULL ;


	if ((xenviron == NULL) || (name == NULL))
	    return NULL ;

#if	CF_DEBUGS
	debugprintf("xgetenv: name=%s\n",name) ;
#endif

	for (i = 0 ; xenviron[i] != NULL ; i += 1) {

	    if (strkeycmp(xenviron[i],name) == 0)
	        break ;

	} /* end for */

	if (xenviron[i] != NULL) {

	    cp = strchr(xenviron[i],'=') ;

	    if (cp != NULL)
	        cp += 1 ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("xgetenv: value=%s\n",cp) ;
#endif

	return cp ;
}
/* end subroutine (xgetenv) */


/* do this extra thing that is also in the library (just in case) */
char *Xgetenv(char *env)
{
	char *ret = NULL ;


	if (xenviron != NULL)
	    ret = xgetenv(env) ;

	return (ret ? ret : "") ;
}
/* end subroutine (Xgetenv) */


/* enumerate entries */
char *xgetentry(j)
int	j ;
{
	int	i ;
	char	*rp = NULL ;

	if (xenviron != NULL) {
	    for (i = 0 ; (xenviron[i] != NULL) && (i < j) ; i += 1) ;
	    rp = xenviron[i] ;
	}

	return rp ;
}
/* end subroutine (xgetentry) */


int xgetenvval(name)
const char	name[] ;
{
	int		rs = SR_NOENT ;
	int		rc = 0 ;
	const char	*cp ;

	if ((name == NULL) || (name[0] == '\0'))
	    return SR_FAULT ;

	if ((cp = xgetenv(name)) != NULL) {

	    if (isdigit(*cp)) {
	        rs = cfdeci(cp,-1,&rc) ;
	    } else {
	        rs = SR_OK ;
	        if ((tolower(*cp) == 'y') || 
	            (tolower(*cp) == 'n')) {
	            rc = (tolower(*cp) == 'y') ;
	        } else
	            rs = SR_INVALID ;
	    } /* end if */

	} /* end if (non-NULL value) */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (xgetenvval) */


/* local subroutines */


/* make a string table for handling the lookups */
static int mktable(vecstr *elp)
{
	int		rs ;
	int		n ;
	int		envsize ;
	int		c = 0 ;

	rs = vecstr_count(elp) ;
	n = rs ;
	if (rs < 0)
	    goto ret0 ;

	envsize = (n + 1) * sizeof(char *) ;
	if ((rs = vecstr_strsize(elp)) >= 0) {
	    const int	strsize = rs ;
	    int		size ;
	    size = 1 + envsize + strsize ;
	    if ((rs = uc_malloc(size,&xenviron)) >= 0) {
		char	*tab = ((char *) xenviron) + envsize ;

	        if ((rs = vecstr_strmk(elp,tab,strsize)) >= 0) {
		    const int	recsize = vecstr_recsize(elp) ;
		    int		*rec = NULL ;

	            if ((rs = uc_malloc(recsize,&rec)) >= 0) {
	                if ((rs = vecstr_recmk(elp,rec,recsize)) >= 0) {
			    int		i ;
			    n = rs ;
	                    for (i = 0 ; (i < n) && (rec[i] >= 0) ; i += 1) {
	                        if (rec[i] > 0) {
	                            xenviron[c++] = (tab + rec[i]) ;
			        }
	                    } /* end for */
	                } /* end if */
	                xenviron[c] = NULL ;
	                uc_free(rec) ;
	            } /* end if (record-table allocated) */

	        } /* end if (created string-table) */

	        if ((rs < 0) && (xenviron != NULL))
	            uc_free(xenviron) ;

	    } /* end if (allocated environment block) */
	} /* end if (vecstr_strsize) */
	if (rs < 0)
	    xenviron = NULL ;

ret0:

#if	CF_DEBUGS
	debugprintf("xenv/mktable: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mktable) */


