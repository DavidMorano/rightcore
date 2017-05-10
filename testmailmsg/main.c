/* main (testmailmsg) */
/* language (C89) */

/* program subroutine (main and all) */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little program provides a tiny test of the MSG object.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"mailmsg.h"
#include	"mailmsghdrs.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	mailmsg_loadfile(MAILMSG *,bfile *) ;
extern int	mailmsg_envaddrfold(MAILMSG *,char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	MAILMSG		tmpmsg, *msgp = &tmpmsg ;

	MAILMSGHDRS	msghvalues ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	const int	llen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	len ;
	int	c ;
	int	al, dl, rl ;
	int	cl ;
	int	i, j ;
	int	ll ;
	int	ex = EX_INFO ;

	const char	*ap, *dp, *rp ;
	const char	*cp ;

	char	lbuf[LINEBUFLEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	        cp = getourenv(envv,VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("b_la: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif


	ex = EX_OK ;

	rs = bopen(ofp,BFILE_STDOUT,"wct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto badoutopen ;
	}

	if ((rs = bopen(ifp,BFILE_STDIN,"r",0666)) >= 0) {

	    if ((rs = mailmsg_start(msgp)) >= 0) {

	        rs = mailmsg_loadfile(msgp,ifp) ;

#if	CF_DEBUGS
	        debugprintf("main: mailmsg_loadfile() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	        debugprintf("main: msg_envget\n") ;
#endif

	        bprintf(ofp,"MAILMSG envs:\n\n") ;

	        for (i = 0 ; 
	            (al = mailmsg_envaddress(msgp,i,&ap)) >= 0 ; 
	            i += 1) {

	            dl = mailmsg_envdate(msgp,i,&dp) ;
	            rl = mailmsg_envremote(msgp,i,&rp) ;

	            bprintf(ofp,"env %u a=%t\n",i,
	                ap,strlinelen(ap,al,40)) ;
	            bprintf(ofp,"env %u d=>%t<\n",i,
	                dp,strlinelen(dp,dl,40)) ;
	            bprintf(ofp,"env %u r=>%t<\n",i,
	                rp,strlinelen(rp,rl,40)) ;

	        } /* end for */

	        bprintf(ofp,"\n") ;

/* envelope folding */

	        if ((ll = mailmsg_envaddrfold(msgp,lbuf,llen)) > 0) {
	            bprintf(ofp,"MAILMSG env-addr >%t<\n",
		        lbuf,strlinelen(lbuf,ll,50)) ;
	            bprintf(ofp,"\n") ;
		}

/* headers */

#if	CF_DEBUGS
	        debugprintf("main: mailmsg_hdrival\n") ;
#endif

	        bprintf(ofp,"MAILMSG hdrs:\n\n") ;

	        for (i = 0 ; (cl = mailmsg_hdrikey(msgp,i,&cp)) >= 0 ; i += 1) {
	            if (cp == NULL) continue ;

	            bprintf(ofp,"hdrkey=%t\n",cp,cl) ;

	        } /* end for */

	        bprintf(ofp,"\n") ;

#if	CF_DEBUGS
	        debugprintf("main: msg_hdrival\n") ;
#endif

	        bprintf(ofp,"MAILMSG hdr-i-values:\n\n") ;

	        for (i = 0 ; mailmsghdrs_names[i] != NULL ; i += 1) {
		    const char	*hn = mailmsghdrs_names[i] ;

	            for (j = 0 ; 
	                (cl = mailmsg_hdrival(msgp,hn,j,&cp)) >= 0 ; 
	                j += 1) {

	                if (cp == NULL) continue ;

	                bprintf(ofp,"H %s %u: >%t<\n",
	                    mailmsghdrs_names[i],j,
	                    cp,strlinelen(cp,cl,40)) ;

	            } /* end for */

	        } /* end for */

	        bprintf(ofp,"\n") ;

#if	CF_DEBUGS
	        debugprintf("main: mailmsg_hdrval\n") ;
#endif

	        bprintf(ofp,"MAILMSG hdr-values:\n\n") ;

	        for (i = 0 ; mailmsghdrs_names[i] != NULL ; i += 1) {
		    const char	*hn = mailmsghdrs_names[i] ;

	            cl = mailmsg_hdrval(msgp,hn,&cp) ;

	            if ((cp != NULL) && (cl >= 0))
	                bprintf(ofp,"H %s >%t<\n",mailmsghdrs_names[i],
	                    cp,strlinelen(cp,cl,40)) ;

	        } /* end for (header key-names) */

	        bprintf(ofp,"\n") ;

	        while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	            len = rs ;

	            bprintf(ofp,"B %t",lbuf,len) ;

	        }

#ifdef	COMMENT

#if	CF_DEBUGS
	        debugprintf("main: about to call 'msgheaders_init'\n") ;
#endif

	        rs = mailmsghdrs_start(&msghvalues,msgp) ;

#if	CF_DEBUGS
	        debugprintf("main: called 'msgheaders_init'\n") ;
#endif

	        if (rs < 0) {
	            ex = EX_SOFTWARE ;
	            goto badmsgheadersinit ;
	        }

	        bprintf(ofp,"\nthe headers in this message were\n\n") ;

	        for (i = 0 ; mailmailmsghdrs_names[i] != NULL ; i += 1) {

	            bprintf(ofp,"%s:\n",
	                mailmsghdrs_names[i]) ;

	            if (msghvalues.v[i] != NULL)
	                bprintf(ofp," %s\n", msghvalues.v[i]) ;

	            bprintf(ofp,"\n") ;

	        } /* end for */

	        mailmsghdrs_finish(&msghvalues) ;

#endif /* COMMENT */

	        mailmsg_finish(msgp) ;
	    } /* end if (mailmsg) */

	    bclose(ifp) ;
	} else {
	    ex = EX_NOINPUT ;
	    bprintf(ofp,"testmsg: input inaccessible (%d)\n",rs) ;
	}

	bclose(ofp) ;

badoutopen:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("b_la: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */



