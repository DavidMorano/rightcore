/* update */



#define	CF_DEBUG	0



/****************************************************************************

 *   update
 *	writes user_bds structure to file USER_BDS
 *	in user's HOME directory
 *
 *	arguments
 *		board		board[s] to update
 *		board_ct	number of boards
 *		user_bds	structure of user boards and times
 

****************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<string.h>
#include	<dirent.h>
#include	<time.h>

#include	<bfile.h>

#include	"config.h"
#include	"dirlist.h"
#include	"bb.h"
#include	"localmisc.h"



/* external subroutines */

extern char		*julstr() ;


/* external variables */

extern struct global	g ;





int update()
{
	struct dirstat	*dsp ;

	struct tm	*timep, ts ;

	offset_t	uflen ;

	int	outcount ;
	int	i, fd, len, l, rs ;
	int	f_sublist ;

	char	home_bds[MAXPATHLEN + 1] ;
	char	nufname[MAXPATHLEN + 1] ;
	char	boardname[MAXPATHLEN + 1] ;
	char	buf[BUFSIZE + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("update: entered\n") ;
#endif

	g.daytime = time(NULL) ;

/* rewind the user's newsgroup list file */

	if (bseek(g.ufp,0L,SEEK_SET) < 0) {

#if	CF_DEBUG
	    if (g.f.debug) {

	        debugprintf("update: could not rewind USER_BDS\n",
	            g.progname) ;

	        debugprintf("update: opening \"%s\"\n",
	            g.ufname) ;

	    }
#endif

	    bclose(g.ufp) ;

	    if ((rs = bopen(g.ufp,g.ufname,"rwct",0666)) < 0) {

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("update: cannot open USER_BDS\n",
	                g.progname) ;
#endif

	        return BAD ;
	    }

	} /* end if (rewind or re-open file) */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("update: 3\n") ;
#endif

/* output the newsgroup list file header comments */

	bprintf(g.ufp,"# PCS user newsgroup list file\n") ;

	bprintf(g.ufp,"#\n") ;

#ifdef	SYSV
	timep = (struct tm *) localtime_r((time_t *) &g.daytime,&ts) ;
#else
	timep = (struct tm *) localtime((time_t *) &g.daytime) ;
#endif

	bprintf(g.ufp,"# updated %02d/%02d/%02d %02d%02d:%02d\n",
	    timep->tm_year,
	    timep->tm_mon + 1,
	    timep->tm_mday,
	    timep->tm_hour,
	    timep->tm_min,
	    timep->tm_sec) ;

	bprintf(g.ufp,"#\n\n") ;


/* go through the loops for each newsgroup */

	for (i = 0 ; i < nuboards ; i += 1) {

	    dsp = user_bds[i].dsp ;
#if	COMMENT
		f_sublist = dsp->f.subscribe ;
#else
		f_sublist = FALSE ;
#endif

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("update: loop b=%s %d\n",
	            dsp->name,dsp->f.subscribe) ;
#endif

/* make the board name (with the dots - not slashes) */

	    bbcpy(boardname,dsp->name) ;

	    if (user_bds[i].mtime > 0) {

	        if (ULT(user_bds[i].mtime,DATE1970))
	            user_bds[i].mtime = DATE1970 ;

#ifdef	SYSV
	        timep = 
	            (struct tm *) localtime_r((time_t *) &user_bds[i].mtime,
	            &ts) ;
#else
	        timep = 
	            (struct tm *) localtime((time_t *) &user_bds[i].mtime) ;
#endif

	        if ((rs = bprintf(g.ufp,"%s%c %02d%02d%02d%02d%02d%02d\n",
	            boardname,
	            (f_sublist ? ':' : '!'),
	            timep->tm_year,
	            timep->tm_mon + 1,
	            timep->tm_mday,
	            timep->tm_hour,
	            timep->tm_min,
	            timep->tm_sec)) < 0) goto badret ;

	    } else {

	        if ((rs = bprintf(g.ufp,"%s%c %d\n", boardname,
	            (f_sublist ? ':' : '!'),0)) < 0)
	            goto badret ;

	    }

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("update: board=%s mtime=%s rs=%d\n",
	            boardname,julstr(user_bds[i].mtime,buf),rs) ;
#endif

	} /* end for */

/* finish off with some trailing comments */

	bprintf(g.ufp,"\n\n") ;

/* close and let's get out */

	bflush(g.ufp) ;

	uflen = bseek(g.ufp,0L,SEEK_CUR) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("update: file offset=%ld \n",
	        uflen) ;
#endif

	if (uflen > 0) bcontrol(g.ufp,BC_TRUNCATE,uflen) ;

#ifdef	COMMENT
	bclose(g.ufp) ;

	rename(nufname,g.ufname) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("update: exiting\n") ;
#endif

	return OK ;

badret:
	bclose(g.ufp) ;

	return BAD ;
}
/* end subroutine (update) */


