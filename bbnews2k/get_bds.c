/* get_bds */

/* get the bulletin board names */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1		/* run-time debugging */
#define	CF_SUBSCRIBE	1		/* allow subscription from file */


/* revision history:

	= 1994-04-10, David A­D­ Morano
	The method of finding the newsgroups has been changed.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

 *   get_bds
 * read names and last modified times of newsgroups
 *
 *	Arguments:
 *
 

*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"bbnewsrc.h"
#include	"config.h"
#include	"mkdirlist.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	mktmpfile(char *,mode_t,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern int	bullbd_ct ;
extern int	namelen ;


/* forward references */

static int	dir_func() ;


/* global variables */

MKDIRLIST	*bull_bds ;

char		*namespace ;


/* exported subroutines */


int get_bds(pip,bb_dir,bb_bds,ufname,ubpp)
struct proginfo	*pip ;
char		bb_dir[], bb_bds[] ;
char		ufname[] ;
struct userstat	**ubpp ;
{
	struct ustat	sn, ss, stbuf ;
	struct userstat	us ;
	MKDIRLIST	*pdp ;
	BBNEWSRC	ung ;
	BBNEWSRC_ENT	unge ;
	bfile		sfile, *sfp = &sfile ;
	bfile		nfile, *nfp = &nfile ;
	bfile		lfile, *lfp = &lfile ;
	time_t		daytime ;
	time_t		mtime_user ;
	ulong		ulw ;
	size_t		msize ;
	int		rs, rs2, len ;
	int		sfd, nfd, lfd ;
	int		i, j, fd ;
	int		nng ;		/* number of newsgroups */
	int		nuboards ;
	int		mprot, mflags ;
	int		f_subscribe ;
	int		f_show ;
	int		f_first ;
	const char	*cp, *cp2 ;
	char		lfname[MAXPATHLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;
	char		*env ;
	void		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("get_bds: ent\n") ;
#endif

/* get all newsgroup names (that we can read) and the "stat"s on them */

	rs =  mkdirlist(bb_dir,sfp,nfp,dir_func) ;
	bullbd_ct = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("get_bds: mkdirlist() rs=%d\n",rs) ;
#endif

	if (rs < 0) goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("get_bds: mkdirlist() returned OK\n") ;
#endif

	if (bullbd_ct <= 0) 
		goto bad1 ;

/* get the file descriptors and file sizes */

	bcontrol(sfp,BC_FD,&sfd) ;

	bcontrol(nfp,BC_FD,&nfd) ;

	bcontrol(sfp,BC_STAT,&ss) ;

	bcontrol(nfp,BC_STAT,&sn) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("get_bds: nlen=%d slen=%d\n",sn.st_size,ss.st_size) ;
#endif

/* get the number of boards (directories) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: got to here\n") ;
#endif

#ifdef	COMMENT
	bullbd_ct = ss.st_size / sizeof(MKDIRLIST) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: about to try to map\n") ;
#endif

/* map the files into our memory space */

	mprot = (PROT_WRITE | PROT_READ) ;
	mflags = MAP_SHARED ;
	msize = (bullbd_ct * sizeof(MKDIRLIST)) ;
	rs = u_mmap(NULL,msize,mprot,mflags,sfd,0L,&p) ;
	bull_bds = p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: bull_bds=%08lX\n",bull_bds) ;
#endif

	if (rs < 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("get_bds: got a bad return from 1st mmap (rs=%d)\n",
	            rs) ;
#endif
	    goto bad1 ;
	}

	namelen = sn.st_size ;

	mprot = (PROT_WRITE | PROT_READ) ;
	mflags = MAP_SHARED ;
	rs = u_mmap(NULL,sn.st_size,mprot,mflags,nfd,0L,&p) ;
	namespace = p ;

	if (rs < 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("get_bds: got a bad return from 2nd mmap (rs=%d)\n",
	            rs) ;
#endif
	    goto bad2 ;
	}

/* close the directory name file */

	bclose(nfp) ;

/* close the directory status file */

	bclose(sfp) ;

/* relocate the "name" fields of the directory status structures */

	for (i = 0 ; i < bullbd_ct ; i += 1) {

/* important overhead maintenance */

	    bull_bds[i].name += (long) namespace ;
#ifdef	COMMENT
	    bull_bds[i].name[bull_bds[i].nlen] = '\0' ;
#endif

/* end of maintenance function */

	    bull_bds[i].link = NULL ;

	    bull_bds[i].f.link = FALSE ;
	    bull_bds[i].f.seen = FALSE ;
	    bull_bds[i].f.show = FALSE ;
	    bull_bds[i].f.new = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	        debugprintf("get_bds: %W\n",bull_bds[i].name,bull_bds[i].nlen) ;
#endif

	    if (bb_bds[0] == '\0') {
	        bull_bds[i].f.excl = 0 ;
	    } else 
	        bull_bds[i].f.excl = 1 ;

	} /* end for (overhead maintenance on mapped directory paths) */

/* loop through the directory path list linking like-entries together */

	for (i = 0 ; i < bullbd_ct ; i += 1) {

	    if (! bull_bds[i].f.link) {

	        pdp = bull_bds + i ;
	        for (j = i + 1 ; j < bullbd_ct ; j += 1) {

	            if ((! bull_bds[j].f.link) &&
	                (bbcmp(bull_bds[i].name,bull_bds[j].name) == 0)) {

	                pdp->link = bull_bds + j ;
	                pdp = bull_bds + j ;
	                bull_bds[j].f.link = TRUE ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("get_bds: linked a NG=%s\n",
	                        bull_bds[i].name) ;
#endif

	            } /* end if (board match) */

	        } /* end for (inner) */

	    } /* end if */

	} /* end for (linking like entries) */

/* OK, now we want to make a list file containing user-board structures */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: about to open TMP files\n") ;
#endif

	rs = mktmpfile(lfname,0644,"/tmp/lfileXXXXXXXXX") ;
	if (rs < 0)
	    goto bad3 ;

	rs = bopen(lfp,lfname,"rwct",0644) ;
	if (rs < 0)
		goto bad4 ;

	u_unlink(lfname) ;

/* open the user's BBNEWSRC file (if present) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("get_bds: about to loop through user's board list\n") ;
	    debugprintf("get_bds: uf=%s\n",ufname) ;
	}
#endif

	nuboards = 0 ;
	if ((rs = bbnewsrc_open(&ung,ufname,0)) >= 0) {

	    daytime = time(NULL) ;

	    mtime_user = daytime ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("get_bds: how are we doing, t1=%lu t2=%s\n",
	            daytime,timestr_log(mtime_user,timebuf)) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("get_bds: reading user NG list file\n") ;
#endif /* CF_DEBUG */

/* loop through the user's list file processing entries found */

	    while ((len = bbnewsrc_read(&ung,&unge)) != 0) {

	        if (len > 0) {

#if	 CF_DEBUG
	if (DEBUGLEVEL(4))
	                debugprintf("get_bds: user NG=%s umt=%s\n",
	                    unge.name,timestr_log(unge.mtime,timebuf)) ;
#endif

	            f_subscribe = unge.f_subscribed ;
	            mtime_user = unge.mtime ;
	            if (mtime_user == 0)
	                f_subscribe = FALSE ;

/* find this newsgroup name in the directory list */

	            for (i = 0 ; i < bullbd_ct ; i += 1) {

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(4))
	                    debugprintf("get_bds: scanning user list NG=%s\n",
	                        bull_bds[i].name) ;
#endif

	                if ((! bull_bds[i].f.link) && 
	                    (bbcmp(unge.name,bull_bds[i].name) == 0)) {

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(4))
	                        debugprintf("get_bds: search match NG=%s\n",
	                            bull_bds[i].name) ;
#endif

	                    bull_bds[i].f.subscribe = f_subscribe ;
#if	CF_SUBSCRIBE
	                    bull_bds[i].f.show = f_subscribe ;
#endif

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(4))
	                        debugprintf("get_bds: show=%d\n",
	                            bull_bds[i].f.show) ;
#endif

	                    if (! bull_bds[i].f.seen) {

	                        bull_bds[i].f.seen = TRUE ;
	                        us.mtime = (time_t) mtime_user ;
	                        us.dsp = bull_bds + i ;
	                        bwrite(lfp,
	                            &us,sizeof(struct userstat)) ;

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(4))
	                        debugprintf("get_bds: adding(a) NG=%s n=%d\n",
	                                bull_bds[i].name,nuboards) ;
#endif

	                        nuboards += 1 ;

	                    } /* end if (not seen) */

	                } /* end if (board name match) */

	            } /* end for (looping through newsgroup directory paths) */

	        } else {

	            bprintf(pip->efp,
	                "%s: bad line in user newsgroup file (line=%d)\n",
	                pip->progname,unge.badline) ;

	        }

	    } /* end while (looping through the user's list file) */

	    bbnewsrc_close(&ung) ;
	} /* end if (getting user board list) */

/* check for user specified boards as given with the invocation options */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: loop through user's BBBDS varaible list\n") ;
#endif

	env = strtok(bb_bds,":") ;

	mtime_user = 0 ;
	while (env != NULL) {

	    f_show = TRUE ;
	    if (env[0] == '-') {

	        env += 1 ;
	        f_show = FALSE ;

	    } else if (env[0] == '+')
	        env += 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("get_bds: BBBDS NG=%s\n",env) ;
#endif

/* find this/these newsgroup(s) in the system's newsgroup spool list */

	    pdp = NULL ;
	    f_first = TRUE ;
	    for (i = 0 ; i < bullbd_ct ; i += 1) {

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	            debugprintf("get_bds: scanning specified NG=%s\n",
	                bull_bds[i].name) ;
#endif

	        if ((! bull_bds[i].f.link) &&
	            (bbcmp(env,bull_bds[i].name) == 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	                debugprintf("get_bds: match on NG=%s\n",
	                    bull_bds[i].name) ;
#endif

	            bull_bds[i].f.excl = 0 ;
	            bull_bds[i].f.show = f_show ;
#ifdef	COMMENT
	            bull_bds[i].f.subscribe = f_subscribe ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	                debugprintf("get_bds: show=%d\n",
	                    bull_bds[i].f.show) ;
#endif

	            if (! bull_bds[i].f.seen) {

	                bull_bds[i].f.seen = TRUE ;
	                us.mtime = (time_t) mtime_user ;
	                us.dsp = bull_bds + i ;
	                bwrite(lfp,
	                    &us,sizeof(struct userstat)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	                    debugprintf("get_bds: adding(b)  NG=%s n=%d\n",
	                        bull_bds[i].name,nuboards) ;
#endif

	                nuboards += 1 ;

	            } /* end if (not seen) */

	        } /* end if (board name match) */

	    } /* end for (looping through newsgroups) */

	    env = strtok(0,":") ;

	} /* end while (looping through BBBDS) */

/* OK, if we didn't get any newsgroups so far take this as user wants all */

	nng = 0 ;
	for (i = 0 ; i < bullbd_ct ; i += 1) {

	    if ((! bull_bds[i].f.link) && bull_bds[i].f.show &&
	        bull_bds[i].f.seen)
	        nng += 1 ;

	} /* end for */

	if (nng <= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("get_bds: no user specified newsgroups, EVERY\n") ;
#endif

	    pip->f.every = TRUE ;

#if	CF_DEBUG
	} else {

	if (DEBUGLEVEL(4))
	        debugprintf("get_bds: newsgroups to be shown %d\n",nng) ;
#endif

	}

/* write out user structures for all remaining unseen boards */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: write out remaing structures, c=%d\n",
	        bullbd_ct) ;
#endif

	for (i = 0 ; i < bullbd_ct ; i += 1) {

	    if ((! bull_bds[i].f.link) &&
	        (! bull_bds[i].f.seen)) {

	        bull_bds[i].f.new = TRUE ;
	        bull_bds[i].f.show = FALSE ;
	        bull_bds[i].f.subscribe = FALSE ;
	        us.mtime = (time_t) 0 ;
	        us.dsp = bull_bds + i ;
	        bwrite(lfp,&us,sizeof(struct userstat)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	            debugprintf("get_bds: C NG=%s n=%d\n",
	                bull_bds[i].name,nuboards) ;
#endif

	        nuboards += 1 ;

	    } /* end if (not seen) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("get_bds: bottom of for i=%d\n",i) ;
#endif

	} /* end for (writing out remaining entries) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: out of for loop\n") ;
#endif

/* map in the user list structure file */

	bflush(lfp) ;

	bseek(lfp,0L,SEEK_SET) ;

	if ((rs = bcontrol(lfp,BC_FD,&lfd)) < 0) {

	    bprintf(pip->efp,
	        "%s: could not get FD on user list file (rs %d)\n",
	        pip->progname,rs) ;

	    goto bad5 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: number of user boards %d\n",nuboards) ;
#endif

	mprot = (PROT_WRITE | PROT_READ) ;
	mflags = MAP_SHARED ;
	msize = (nuboards * sizeof(struct userstat)) ;
	rs = u_mmap(NULL,msize,mprot,mflags,lfd,0L,(void *) ubpp) ;

	if (rs < 0) 
		goto bad5 ;

	bclose(lfp) ;

/* return OK */

	if (rs >= 0)
	    rs = nuboards ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("get_bds: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* handle the bad stuff */
bad5:
	bclose(lfp) ;

bad4:
	if ((lfname != NULL) && (*lfname != '\0'))
	    u_unlink(lfname) ;

bad3:
	u_munmap((char *) namespace,sn.st_size) ;

bad2:
	u_munmap((char *) bull_bds,
	    (bullbd_ct * sizeof(MKDIRLIST))) ;

	bull_bds = NULL ;

bad1:
	bclose(sfp) ;

	bclose(nfp) ;

	goto ret0 ;
}
/* end subroutine (get_bds) */


/* local subroutines */


static int dir_func(fnp,fsp,dsp)
char		*fnp ;
struct ustat	*fsp ;
MKDIRLIST	*dsp ;
{


	if (strcmp(fnp,".options") == 0)
	    dsp->f.options = TRUE ;

	if (strchr(fnp,' ') != NULL) 
		return BAD ;

	return OK ;
}
/* end subroutine (dir_func) */



