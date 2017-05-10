/* main (testourmsginfo) */

/* program subroutine (main and all) */


#define	CF_DEBUGS	0		/* compile-time */


/******************************************************************************

	This little program provides a tiny test of the MAILMSG object.


/******************************************************************************


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<logfile.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"msg.h"
#include	"msgheaders.h"
#include	"config.h"

#include	"ourmsginfo.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* exported subroutines */


int main()
{
	struct ourmsginfo_env	*mep ;

	struct ourmsginfo_header	*mhp ;

	struct ourmsginfo_instance	*mip ;

	struct ourmsginfo_line		*mlp ;

	OURMSGINFO	tmpmsg, *msgp = &tmpmsg ;

	MSGHEADERS	msghvalues ;

	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int	rs, i, len ;
	int	fd_debug = -1 ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	bopen(ifp,BFILE_STDIN,"r",0666) ;

	bopen(ofp,BFILE_STDOUT,"wct",0666) ;

	bprintf(ofp,"main: entered\n\n") ;


	ourmsginfo_init(msgp,ifp,0L,NULL,0) ;

#if	CF_DEBUGS
	debugprintf("main: called 'ourmsginfo_init'\n") ;
#endif

	for (i = 0 ; ourmsginfo_envget(msgp,i,&mep) >= 0 ; i += 1) {

	    if (mep == NULL) continue ;

	    bprintf(ofp,"E from=%s date=%s remote=%s\n",
	        mep->from,
	        mep->date,
	        mep->remote) ;

	}

	bprintf(ofp,"\n") ;

#if	CF_DEBUGS
	debugprintf("main: about to call 'ourmsginfo_headget'\n") ;
#endif

	for (i = 0 ; ourmsginfo_headget(msgp,i,&mhp) >= 0 ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("main: inside 'ourmsginfo_headget' loop\n") ;
#endif

	    if (mhp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("main: inside 'ourmsginfo_headget' loop 2, mhp=%08X\n",mhp) ;

	    debugprintf("main: nlen=%d vlen=%d\n",mhp->nlen,mhp->vlen) ;

	    debugprintf("main: name=%s value=%s\n",mhp->name,mhp->value) ;

#endif /* CF_DEBUGS */

	    bprintf(ofp,"H %t: %t\n",
	        mhp->name,mhp->nlen,
	        mhp->value,mhp->vlen) ;

	} /* end for */

	bprintf(ofp,"\n") ;

	while ((rs = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    bprintf(ofp,"B %t",lbuf,len) ;

	}

#if	CF_DEBUGS
	debugprintf("main: about to call 'msgheaders_init'\n") ;
#endif

	msgheaders_init(&msghvalues,msgp) ;

#if	CF_DEBUGS
	debugprintf("main: called 'msgheaders_init'\n") ;
#endif

	bprintf(ofp,"\nthe headers in this message were\n\n") ;

	for (i = 0 ; mailmsghdrs_names[i] != NULL ; i += 1) {

	    bprintf(ofp,"%s:\n",
	        mailmsghdrs_names[i]) ;

	    if (msghvalues.v[i] != NULL)
	        bprintf(ofp," %s\n", msghvalues.v[i]) ;

	    bprintf(ofp,"\n") ;

	} /* end for */

	msgheaders_free(&msghvalues) ;


	ourmsginfo_free(msgp) ;

	bclose(ifp) ;

	bclose(ofp) ;

	return EX_OK ;
}
/* end subroutine (main) */



