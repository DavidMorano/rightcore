/* update */

/* update the user's BBNEWSRC type file */


#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_SUBSCRIBE	1		/* re-record subscruption state */


/* revision history:

	= 1995-11-13, David A­D­ Morano

	This is a revised subroutine to match the new 'bbnewsrc' object.


*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

 *   update
 *	writes user_bds structure to file USER_BDS
 *	in user's HOME directory
 *
 *	arguments:
 *		board		board[s] to update
 *		board_ct	number of boards
 *		user_bds	structure of user boards and times
 

*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<string.h>
#include	<dirent.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"bbnewsrc.h"
#include	"config.h"
#include	"artlist.h"
#include	"defs.h"


/* external subroutines */

extern int	bbcpy(char *,const char *) ;

extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* forward references */


/* exported subroutines */


int update(gp,ufname,user_bds,nuboards)
struct proginfo	*gp ;
char		ufname[] ;
struct userstat	*user_bds ;
int		nuboards ;
{
	struct tm	*timep ;

	MKDIRLIST	*dsp ;

	BBNEWSRC	ung ;

	int	rs = SR_OK ;
	int	i ;
	int	f_subscribe ;

	char	ngname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("update: entered\n") ;
#endif

	gp->now.time = time(NULL) ;

/* update the user's newsgroup list file */

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("update: opening user NG file=%s\n",ufname) ;
#endif

	if (bbnewsrc_open(&ung,ufname,gp->f.readtime) >= 0) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("update: opened user NG file\n") ;
#endif

/* go through the loops for each newsgroup */

	    for (i = 0 ; i < nuboards ; i += 1) {

	        dsp = user_bds[i].dsp ;
#if	CF_SUBSCRIBE
	        f_subscribe = dsp->f.subscribe ;
#else
	        f_subscribe = FALSE ;
#endif

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("update: loop ng=%s f_subscribe=%d\n",
	                dsp->name,dsp->f.subscribe) ;
#endif

/* make the board name (with the dots - not slashes) */

	        bbcpy(ngname,dsp->name) ;

#if	CF_DEBUG
	        if (gp->debuglevel > 1) {
	            debugprintf("update: ngname=%s\n", ngname) ;
	            debugprintf("update: sf=%d mtime=%s\n",
			f_subscribe,
			timestr_log(user_bds[i].mtime,timebuf)) ;
		}
#endif

	        bbnewsrc_write(&ung,ngname,f_subscribe,user_bds[i].mtime) ;

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("update: bottom for\n") ;
#endif

	    } /* end for */

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("update: out of loop\n") ;
#endif

	    bbnewsrc_close(&ung) ;

	} /* end if (writing user NG file) */


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("update: exiting\n") ;
#endif

	return OK ;
}
/* end subroutine (update) */



