/* bopensched */

/* open a file name according to rules */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will form a file name according to some rules.

	We try in order:

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

#define	BFILE_MASTER	0

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<vecstr.h>
#include	<outbuf.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	MODELEN		12
#define	KEYBUF(c)	(keybuf[0] = c,keybuf[1] = '\0',keybuf)


/* external subroutines */

extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	vstrkeycmp(char **,char **) ;


/* external variables */


/* local structures */


/* forward references */

static int	schedexpand(const char *,VECSTR *,const char *,char *,int) ;


/* local global variabes */


/* local variables */


/* exported subroutines */


int bopensched(fp,sched,nsp,fname,outname,mode,permission)
bfile		*fp ;
const char	*sched[] ;
VECSTR		*nsp ;
const char	fname[] ;
char		outname[] ;
const char	mode[] ;
int		permission ;
{
	OUTBUF	ob ;

	int	rs = SR_OK ;
	int	i, sl ;
	int	imode ;
	int	f_create = FALSE ;

	const char	*mp ;
	const char	*onp = NULL ;

	char	openmode[MODELEN + 1], *omp = openmode ;
	char	*tmpfname ;


	if ((fp == NULL) || (sched == NULL))
	    return SR_FAULT ;

	if (nsp == NULL)
	    return SR_NOEXIST ;

/* no check on 'fname' because that may be OK! */

#if	CF_DEBUGS
	    debugprintf("bopensched: entered fname=%s\n",fname) ;
#endif

	imode = 0 ;
	for (mp = mode ; *mp && (mp < (mode + MODELEN)) ; mp += 1) {
	    int	ch = (*mp & 0xff) ;

	    switch (ch) {

	    case 'r':
	        *omp++ = *mp ;
	        imode += R_OK ;
	        break ;

	    case 'w':
	        *omp++ = *mp ;
	        imode += W_OK ;
	        break ;

	    case 'x':
	        *omp++ = *mp ;
	        imode += X_OK ;
	        break ;

	    case 'c':
	        f_create = TRUE ;
	        break ;

	    default:
	        *omp++ = *mp ;
		break ;

	    } /* end switch */

	} /* end for */

	*omp = '\0' ;

/* get ready */

	rs = outbuf_start(&ob,outname,-1) ;
	if (rs < 0)
	    goto ret0 ;

	rs = outbuf_get(&ob,&tmpfname) ;
	if (rs < 0)
	    goto ret1 ;

/* loop through the schedules, expanding them as we go */

	rs = SR_NOEXIST ;
	for (i = 0 ; sched[i] ; i += 1) {

	    sl = schedexpand(sched[i],nsp,fname,tmpfname,MAXPATHLEN) ;
	    if (sl < 0) continue ;

	    if ((rs = perm(tmpfname,-1,-1,NULL,R_OK)) >= 0)
	        break ;

	} /* end for */

	if (rs < 0) {

	    rs = SR_NOEXIST ;
	    if (f_create) {

/* continue, but this time we create the file as we go! */

	        for (i = 0 ; sched[i] ; i += 1) {

	            sl = schedexpand(sched[i],nsp,fname,tmpfname,MAXPATHLEN) ;
	            if (sl < 0) continue ;

	            if ((rs = bopen(fp,tmpfname,mode,permission)) >= 0)
	                break ;

	        } /* end for */

	    } /* end if (creating) */

	} else
	    rs = bopen(fp,tmpfname,mode,permission) ;

/* we are done */
ret1:
	outbuf_finish(&ob) ;

ret0:
	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (bopensched) */


/* local subroutines */


static int schedexpand(fmt,nsp,fname,buf,buflen)
const char	*fmt ;
VECSTR		*nsp ;
const char	fname[] ;
char		buf[] ;
int		buflen ;
{
	SBUF	buffer ;

	int	rs = SR_OK ;
	int	rs1 ;

	const char	*fp ;
	const char	*tp, *cp ;

	char	keybuf[2] ;
	char	*bp = buf ;


#if	CF_DEBUGS
	debugprintf("bopensched/expand: entered fname=%s\n",fname) ;
#endif

	buf[0] = '\0' ;
	if (buflen <= 0)
	    return SR_TOOBIG ;

	rs = sbuf_start(&buffer,buf,buflen) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("bopensched/expand: about to while\n") ;
#endif

	for (fp = fmt ; *fp && (rs >= 0) ; fp += 1) {

#if	CF_DEBUGS
	    debugprintf("bopensched/expand: char=>%c<\n",*fp) ;
#endif

	    if (*fp == '%') {

	        fp += 1 ;
	        if (! *fp) 
		    break ;

#if	CF_DEBUGS
	        debugprintf("bopensched/expand: key=>%c<\n",*fp) ;
#endif

	        if (*fp == '%') {

	            rs = sbuf_char(&buffer,'%') ;

	        } else if (*fp == 'f') {

	            rs = SR_FAULT ;
	            if (fname != NULL)
	                rs = sbuf_strw(&buffer,fname,-1) ;

	        } else {

#if	CF_DEBUGS
	            debugprintf("bopensched/expand: got key=>%c<\n",*fp) ;
#endif

	            rs = vecstr_finder(nsp,KEYBUF(*fp),vstrkeycmp,&cp) ;

#if	CF_DEBUGS
	            debugprintf("bopensched/expand: rs=%d key_value=%s\n",
			rs,cp) ;
#endif

	            if (rs >= 0) {

	                rs = 0 ;
	                if ((tp = strchr(cp,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("bopensched/expand: storing value=%s\n",
	                        (tp+1)) ;
#endif

	                    rs = sbuf_strw(&buffer,(tp+1),-1) ;

#if	CF_DEBUGS
	                    debugprintf("bopensched/expand: store rs=%d\n",rs) ;
#endif

	                }
	            }

	        }

	    } else {

#if	CF_DEBUGS
	        debugprintf("bopensched/expand: storing regular char=>%c<\n",
			*fp) ;
#endif

	        rs = sbuf_char(&buffer,*fp) ;

	    } /* end if */

#if	CF_DEBUGS
	    {
	        int	len = sbuf_getlen(&buffer) ;

	        debugprintf("bopensched/expand: bottom 'for', rs=%d\n",rs) ;
	        debugprintf("bopensched/expand: buflen=%d buf=%s\n",len,buf) ;
	    }
#endif

	} /* end for */

	rs1 = sbuf_finish(&buffer) ;
	if (rs >= 0) rs = rs1 ;

ret0:

#if	CF_DEBUGS
	debugprintf("bopensched/expand: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (schedexpand) */



