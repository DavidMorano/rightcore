/* permsched */

/* find a file according to rules */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_BADNOFILE	0		/* is it bad if there was no file? */
#define	CF_SPERM	1		/* try 'sperm(3dam)' */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will form a file name according to some rules.

	Synopsis:

	int permsched(sched,nsp,rbuf,rlen,fname,am)
	const char	*sched[] ;
	VECSTR		*nsp ;
	const char	fname[] ;
	int		am ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	sched		array of strings forming the search schedule
	nsp		string-list of substitution keys
	rbuf		user supplied buffer to hold resuling filepath
	rlen		length of user supplied buffer
	fname		filename to search for
	am		access-mode needed to get a file hit

	Returns:

	>=0	length of found filepath
	<0	error


	A typical example is something like trying the following:

	programroot/etc/name/name.fname
	programroot/etc/name/fname
	programroot/etc/name.fname
	programroot/name.fname
	programroot/fname

	if (programroot != PWD) {

		etc/name/name.fname
		etc/name/fname
		etc/name.fname
		name.fname
		fname

	}

	%p/%e/%n/%n.%f
	%p/%e/%n/%f
	%p/%e/%n.%f
	%p/%n.%f
	%p/%f
	
	%e/%n/%n.%f
	%e/%n.%f
	%e/%n.%f
	%n.%f
	%f


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */

#define	KEYBUF(c)	(keybuf[0] = (c),keybuf[1] = '\0',keybuf)


/* external subroutines */

extern int	vstrkeycmp(char **,char **) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	schedexpand(const char *,VECSTR *,char *,int,const char *) ;


/* local variables */


/* exported subroutines */


int permsched(sched,nsp,rbuf,rlen,fname,am)
const char	*sched[] ;
VECSTR		*nsp ;
char		rbuf[] ;
int		rlen ;
const char	fname[] ;
int		am ;
{
	IDS	id ;

	int	rs ;
	int	i ;
	int	sl = 0 ;


#if	CF_DEBUGS
	debugprintf("permsched: entered fname=%s\n",fname) ;
#endif

	if ((sched == NULL) || (rbuf == NULL))
	    return SR_FAULT ;

	if (nsp == NULL)
	    return SR_FAULT ;

#if	CF_BADNOFILE
	if ((fname == NULL) || (fname[0] == '\0'))
	    return SR_NOEXIST ;
#endif

#if	CF_DEBUGS
	{
	    const char	*cp ;
	    debugprintf("permsched: key_values begin\n") ;
	    for (i = 0 ; vecstr_get(nsp,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;
	        debugprintf("permsched: >%s<\n",cp) ;
	    }
	    debugprintf("permsched: key_values end\n") ;
	}
#endif /* CF_DEBUGS */

#if	CF_SPERM
	rs = ids_load(&id) ;
	if (rs < 0) goto ret0 ;
#endif

/* loop through the schedules, expanding them as we go */

	rs = SR_NOEXIST ;
	for (i = 0 ; sched[i] ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("permsched: sched=%s\n",sched[i]) ;
#endif

	    sl = schedexpand(sched[i],nsp,rbuf,rlen,fname) ;

	    if (sl > 0) {

#if	CF_SPERM
	        {
	            struct ustat	sb ;

	            if ((rs = uc_stat(rbuf,&sb)) >= 0)
	                rs = sperm(&id,&sb,am) ;

	        } /* end block */
#else
	        rs = perm(rbuf,-1,-1,NULL,am) ;
#endif /* CF_SPERM */

#if	CF_DEBUGS
	        debugprintf("permsched: trying fname=%s rs=%d\n",rbuf,rs) ;
#endif

	    } /* end if (good expansion) */

	    if (rs >= 0) break ;
	} /* end for */

#if	CF_SPERM
	ids_release(&id) ;
#endif

ret0:

#if	CF_DEBUGS
	debugprintf("permsched: ret rs=%d sl=%u\n",rs,sl) ;
#endif

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (permsched) */


/* local subroutines */


static int schedexpand(fmt,nsp,rbuf,rlen,fname)
const char	*fmt ;
VECSTR		*nsp ;
char		rbuf[] ;
int		rlen ;
const char	fname[] ;
{
	SBUF	buffer ;

	int	rs ;
	int	len = 0 ;

	const char	*fp ;
	const char	*tp, *cp ;

	char	keybuf[2] ;


#if	CF_DEBUGS
	debugprintf("permsched/expand: entered fname=%s\n",fname) ;
#endif

	rbuf[0] = '\0' ;
	if (rlen <= 0)
	    return SR_TOOBIG ;

	if ((rs = sbuf_start(&buffer,rbuf,rlen)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("permsched/expand: about to while\n") ;
#endif

	    for (fp = fmt ; *fp && (rs >= 0) ; fp += 1) {

#if	CF_DEBUGS
	        debugprintf("permsched/expand: char=>%c<\n",*fp) ;
#endif

	        if (*fp == '%') {

	            fp += 1 ;
	            if (! *fp)
	                break ;

#if	CF_DEBUGS
	            debugprintf("permsched/expand: key=>%c<\n",*fp) ;
#endif

	            if (*fp == '%') {

	                rs = sbuf_char(&buffer,'%') ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("permsched/expand: got key=>%c<\n",*fp) ;
#endif

	                rs = vecstr_finder(nsp,KEYBUF(*fp),vstrkeycmp,&cp) ;

#if	CF_DEBUGS
	                debugprintf("permsched/expand: rs=%d key_value=%s\n",
	                    rs,cp) ;
#endif

	                if (rs >= 0) {

	                    rs = 0 ;
	                    if ((tp = strchr(cp,'=')) != NULL) {

	                        rs = sbuf_strw(&buffer,(tp + 1),-1) ;

	                    } /* end if (it had a value) */

	                } else if (*fp == 'f') {

	                    rs = SR_FAULT ;
	                    if (fname != NULL)
	                        rs = sbuf_strw(&buffer,fname,-1) ;

	                } else
	                    rs = SR_NOTFOUND ;

	            } /* end if (tried to expand a key) */

	        } else {

	            rs = sbuf_char(&buffer,*fp) ;

	        } /* end if (escape or regular character) */

	    } /* end for */

	    len = sbuf_finish(&buffer) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */


#if	CF_DEBUGS
	debugprintf("permsched/expand: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (schedexpand) */



