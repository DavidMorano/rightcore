/* procfile */

/* process an input file to the output */


#define	CF_DEBUG	0
#define	CF_RBBPOST	1
#define	CF_MSGS		1


/* revistion history :

	= 1987-09-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This subroutine processes a file by writing the
	parsed addresses out to the output file.


*********************************************************************/


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

#include	<bfile.h>
#include	<baops.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LINELEN		200
#define	BUFLEN		(LINELEN + LINELEN)
#define	CMDLEN		((2 * MAXPATHLEN) + 20)


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;

extern char	*strshrink(char *) ;


/* local structures */

struct ustate {
	bfile	outfile ;
	int	facts ;
	int	f_active ;
	char	outfname[MAXPATHLEN + 1] ;
} ;


/* forward references */

static int	procfree() ;

static void	procinit() ;
static void	procopen(), procclose() ;
static void	procitem() ;


/* local variables */


/* exported subroutines */


int procfile(gp,ofp,infname,fn)
struct proginfo	*gp ;
bfile		*ofp ;
char		infname[] ;
int		fn ;
{
	bfile		infile, *ifp = &infile ;

	struct ustat	sb ;

	struct ustate	s ;

	int	len, rs ;
	int	facts ;
	int	f_stdinput = FALSE ;

	char	linebuf[LINELEN + 1], *lbp ;
	char	*cp ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile: processing file number %d\n",fn) ;
#endif

/* check the arguments */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile: file=%s\n",infname) ;
#endif

	if ((infname == NULL) || (infname[0] == '-')) {

	    f_stdinput = TRUE ;
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	} else
	    rs = bopen(ifp,infname,"r",0666) ;

	if (rs < 0)
	    goto badinfile ;


/* initialize the state data */

	procinit(gp,&s) ;


/* output the pages */

	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	    int	tlen ;


	    tlen = sfshrink(linebuf,len,&lbp) ;

	    if (tlen <= 0) {

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("procfile: tlen is zero\n") ;
#endif

	        if (s.f_active)
	            procclose(gp,&s) ;

	    } else {

	        lbp[tlen] = '\0' ;
	        if (*lbp == '-') {

	            lbp += 1 ;
	            if (s.f_active)
	                procclose(gp,&s) ;

	        }

	        while (CHAR_ISWHITE(*lbp)) lbp += 1 ;

	        cp = strshrink(lbp) ;

	        if (*cp != '\0') {

	            if (! s.f_active)
	                procopen(gp,&s) ;

	            procitem(gp,&s,lbp) ;

	        }

	    }

	} /* end while (reading file lines) */

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("procfile: finishing off\n") ;
#endif

	                procclose(gp,&s) ;

	facts = procfree(gp,&s) ;


done:
	bclose(ifp) ;

	return facts ;

badinfile:
	return BAD ;
}
/* end subroutine (procfile) */



/* LOCAL SUBROUTINES */



static void procinit(gp,sp)
struct proginfo	*gp ;
struct ustate	*sp ;
{


	sp->f_active = FALSE ;
	sp->facts = 0 ;
	bufprintf(sp->outfname,MAXPATHLEN,"%s/factsXXXXXXXXX",
	    gp->tmpdname) ;

}
/* end subroutine (procinit) */


static int procfree(gp,sp)
struct proginfo	*gp ;
struct ustate	*sp ;
{


	u_unlink(sp->outfname) ;

}
/* end subroutine (procfree) */


static void procopen(gp,sp)
struct proginfo	*gp ;
struct ustate	*sp ;
{
	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile/procopen: temporary file %s\n",sp->outfname) ;
#endif

	sp->f_active = TRUE ;
	(void) bopen(&sp->outfile,sp->outfname,"wct",0644) ;

	bprintf(&sp->outfile,"x-mailer: %s\n",gp->mailername) ;

	bprintf(&sp->outfile,"Date: %s\n",
	    timestr_hdate(gp->daytime,timebuf)) ;

	bprintf(&sp->outfile,"Subject: factoid\n") ;

	bprintf(&sp->outfile,"\n") ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile/procopen: exiting\n") ;
#endif

}
/* end subroutine (procopen) */


static void procclose(gp,sp)
struct proginfo	*gp ;
struct ustate	*sp ;
{
	char	cmdbuf[CMDLEN + 1] ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile/procclose: temporary file %s\n",sp->outfname) ;
#endif

	if (! sp->f_active)
	    return ;

	sp->f_active = FALSE ;
	sp->facts += 1 ;
	bclose(&sp->outfile) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile/procclose: about to post\n") ;
#endif

	if (! gp->f.no) {

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile/procclose: calling RBBPOST ?\n") ;
#endif

#if	CF_RBBPOST
	if (gp->prog_msgs != NULL) {

	    bufprintf(cmdbuf,CMDLEN,"%s %s < %s",
	        PROG_RBBPOST,gp->newsgroup,sp->outfname) ;

	    uc_system(cmdbuf) ;

	}
#endif /* CF_RBBPOST */


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile/procclose: calling MSGS ?\n") ;
#endif

#if	CF_MSGS
	if (gp->prog_msgs != NULL) {

	    bufprintf(cmdbuf,CMDLEN,"%s -s < %s",
	        gp->prog_msgs,sp->outfname) ;

	    uc_system(cmdbuf) ;

	}
#endif /* CF_MSGS */

	}

	u_unlink(sp->outfname) ;

}
/* end subroutine (procclose) */


static void procitem(gp,sp,linebuf)
struct proginfo	*gp ;
struct ustate	*sp ;
char		linebuf[] ;
{


#if	CF_DEBUG && 0
	if (gp->debuglevel > 1)
	    debugprintf("procfile/procitem: entered\n") ;
#endif

	bprintf(&sp->outfile,"%s\n",linebuf) ;

#if	CF_DEBUG && 0
	if (gp->debuglevel > 1)
	    debugprintf("procfile/procitem: exiting\n") ;
#endif

}
/* end subroutine (procitem) */



