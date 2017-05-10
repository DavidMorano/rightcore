/* getpfopts */

/* get the PCS-wide options */


#define	CF_DEBUG	0


/* revision history:

	= 95/05/01, David A­D­ Morano

	This code module was completely rewritten to replace any
	original garbage that was here before.


*/


/******************************************************************************

	This subroutine parses out options from the main PCS
	configuration file.


	Synopsis:

	int getpfopts(gp,setsp)
	struct proginfo	*gp ;
	vecstr		*setsp ;




******************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<field.h>
#include	<vecstr.h>
#include	<pcsconf.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */

extern int	matstr3(char *const *,const char *,int) ;
extern int	headkeymat(char *,char *,int) ;

extern char	*strbasename(char *) ;


/* local forward references */


/* external variables */


/* global variables */


/* local data */

static char *const progopts[] = {
	    "mailername",
	    "progrbbpost",
	    "progmsgs",
	    "newsgroup",
	    "spooldir",
	    NULL
} ;


#define	PROGOPT_MAILERNAME	0
#define	PROGOPT_PROGRBBPOST	1
#define	PROGOPT_PROGMSGS	2
#define	PROGOPT_NEWSGROUP	3
#define	PROGOPT_SPOOLDIR	4





int getpfopts(gp,setsp)
struct proginfo	*gp ;
vecstr		*setsp ;
{
	int	i, oi ;

	char	*cp ;


/* system-wide options ? */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("getpfopts: scanning system options\n") ;
#endif

	for (i = 0 ; vecstr_get(setsp,i,&cp) >= 0 ; i += 1) {

	    char	*cp2 ;


	    if (cp == NULL) 
		continue ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("getpfopts: conf >%s<\n",cp) ;
#endif

	    if (! headkeymat(SEARCHNAME,cp,-1))
	        continue ;

/* we have one of ours, separate the keyname from the value */

	    cp += (strlen(SEARCHNAME) + 1) ;
	    if ((cp2 = strchr(cp,'=')) == NULL)
	        continue ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("getpfopts: opt >%s<\n",cp) ;
#endif

	    if ((oi = matstr3(progopts,cp,(cp2 - cp))) >= 0) {

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("getpfopts: system valid option, oi=%d\n",
	                oi) ;
#endif

	        cp2 += 1 ;
	        switch (oi) {

	        case PROGOPT_MAILERNAME:
	            if (*cp2 && (gp->mailername == NULL))
	                gp->mailername = mallocstr(cp2) ;

	            break ;

	        case PROGOPT_PROGRBBPOST:
	            if (*cp2 && (gp->prog_rbbpost == NULL))
	                gp->prog_rbbpost = mallocstr(cp2) ;

	            break ;

	        case PROGOPT_PROGMSGS:
	            if (*cp2 && (gp->prog_msgs == NULL))
	                gp->prog_msgs = mallocstr(cp2) ;

	            break ;

	        case PROGOPT_NEWSGROUP:
	            if (*cp2 && (gp->newsgroup == NULL))
	                gp->newsgroup = mallocstr(cp2) ;

	            break ;

	        case PROGOPT_SPOOLDIR:
	            if (*cp2 && (gp->spooldname == NULL))
	                gp->spooldname = mallocstr(cp2) ;

	            break ;

	        } /* end switch */

	    } /* end if (got a match) */

	} /* end for */

	return OK ;
}
/* end subroutine (getpfopts) */



