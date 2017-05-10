/* setupmailbox */

/* setup something -- the mailbox? */


#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_GETMAIL	0		/* get new mail */
#define	CF_SPAWNPROC	0		/* spawn sub-process */


/* revision history:

	= 94-01-23, David A­D­ Morano
        This module was copied and modified from the VMAIL original. A variety
        of enhancements were made to prevent it from crashing due to short
        buffers. No checks were being made about whether a copy into a buffer
        was overflowing! Yes, this was one of the causes of the spread of the
        1988 Internet worm. Of course, nobody likes a program that crashes
        either (including myself). It was the crashing of this (and other)
        programs that lead me to fix this crap up in the first place!

	= 96-06-18, David A­D­ Morano
	I did:
		- remove old mail spool locks

	= 96-07-24, David A­D­ Morano
	I rewrote the "getnewmail" subroutine in part to :
		- lock user's "new" mailbox when fetching new mail
		  NOTE: This has to be removed when real proper
			mailbox handling is implemented.
		- guard against corrupted "new" mailbox on new mail
		  from file system full
		- added full binary compatibility for new mail

	= 2007-11-13, David A­D­ Morano
        Oh man! How long have I been toiling with this thing? I added the
        ability to grab mail from multiple users. I also rewrote from the
        original (before I started cleaning up this crap in 1994) much of the
        way that this process takes place.

*/

/* Copyright © 1994,1996,2007 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	sets up desired mailbox (calls sumessptr).  
	this initializes the globals: 
	messbeg,messend (byte beg/end of mess in file),  mb.total,
	messord (external->internal mn conversion (mess ordering)),
	curr.mbname (external name), curr.fp (internal file pointer),
	messdel (delete markers).

	Synopsis:

	int setupmailbox(pip,dsp,mbname)
	struct proginfo	*pip ;
	DS		*dsp ;
	const char	mbname[] ;

	Arguments:

	pip		program information pointer
	dsp		display pointer
	mbname		mailbox-name to setup

	Returns:

	>=0	successful with count of messages
	<0	can't be opened or other error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<curses.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<spawnproc.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mb.h"
#include	"ds.h"


/* local defines */

#define	O_FLAGS		(O_RDWR | O_APPEND)


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	mkmailbox(struct proginfo *,const char *) ;
extern int	search() ;


/* external variables */

extern struct mailbox	mb ;


/* local (forward) subroutines */

int		messplace(char *,int) ;
int		numinarray(int *,int,int) ;

static int	getnewmail(struct proginfo *) ;
static int	lockfileend(int) ;


/* local variables */

static int	tempmessord[MAXMESS + 1] ;

static int	ptord ;		/* to make it compile */





int setupmailbox(pip,dsp,mbname)
struct proginfo	*pip ;
DS		*dsp ;
const char	mbname[] ;
{
	FILE	*fk ;

	int	rs = SR_OK, rs1 ;
	int	k, mn, nexttry ;
	int	pv ;
	int	c ;

	char	mbfname[MAXNAMELEN + 1] ;
	char	fullname[MAXPATHLEN + 1] ;
	char	sexp[LINEBUFLEN + 1] ;
	char	dummy[LINEBUFLEN + 1] ;
	char	*bn = (char *) mbname ;


	if (dsp == NULL)
	    return SR_FAULT ;

	if (mbname == NULL)
	    return SR_FAULT ;

	if (mbname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("setupmailbox: mbname=%s\n",mbname) ;
#endif

	c = 0 ;
	pages[0] = -1 ;

/* checking for new mail on the "in" mailbox */

	if (strcmp(bn,pip->mbname_in) == 0) {

	    rs1 = getnewmail(pip) ;

	} /* end if (tried to get new mail) */

/* make a mailfile name */

	rs = mkpath2(mbfname,pip->folderdname,mbname) ;
	if (rs < 0)
	    goto ret0 ;

/* open it */

	rs = mb_open(&mb,pip,mbfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("setupmailbox: mb_open() rs=%d\n",rs) ;
#endif

	c = rs ;
	if (rs < 0)
	    goto ret0 ;

/* open the old way also */

	curr.fp = fopen(mbfname,"rw") ;

/* record the new mailbox name */

	if (bn != NULL)
	    sncpy1(curr.mbname,MAXNAMELEN,bn) ;

/* initial ordering of the messages */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("setupmailbox: ordering of messages\n") ;
#endif

	pv = profile("fifo") ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("setupmailbox: fifo value=%d\n",pv) ;
#endif

	if ((pv < 0) || (pv > 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("setupmailbox: FIFO order\n") ;
#endif

	    for (k = 0 ; k < mb.total ; k += 1)
	        messord[k] = k ;

	} else {

/* want Last In First Out (reverse arrival) order */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("setupmailbox: LIFI order\n") ;
#endif

	    for (k = 0 ; k < mb.total ; k += 1)
	        messord[k] = mb.total - k - 1 ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("setupmailbox: done w/ ordering\n") ;
#endif


/* see if wish to sort messages (nosortall -> only if newbox) */

#ifdef	COMMENT
	if (profile("sort_all") || (strcmp(bn,pip->imbox) == 0)) {

/* ***

All message references will be done via "messord" array 
so change the message ordering in that.
logical expressions are contained in  '.priority' file.
		
****/

	    mkpath2(fullname,pip->folder,".PRIORITY") ;

	    if ((fk = fopen(fullname,"r")) != NULL) {

	        ptord = 0 ;
	        while (freadline(fk,sexp,LINEBUFLEN) > 0) {

/* matches of sexp get priority */

	            sexp[strlen(sexp)-1] = '\0' ;   /* rm linefeed */
	            if (leparse(sexp) == 0)
	                search(pip,dsp,&mb,messplace,dummy) ;

	        }

	        fclose(fk) ;

/* copy sorted messages into real messord */

	        for (k = 0 ; k < ptord ; k += 1)
	            messord[k] = tempmessord[k] ;

/* unmatched mess go in FIFO order */

	        nexttry = 0 ;
	        for (k = ptord ; k < mb.total ; k += 1) {

	            for (mn = nexttry ; mn < mb.total ; mn += 1)

/* found unused mn */

	                if (! numinarray(messord,ptord,mn)) {

	                    messord[k] = mn ;
	                    break ;
	                }

	            nexttry = mn + 1 ;
	        }

	    } /* end if (reading priority stuff) */

	}
#endif /* COMMENT */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("setupmailbox: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (setupmailbox) */


/****

place mn as the next message in expression-sorted order.
This is called by search which uses messord so it will temporarily
place mns in tempmessord.

****/

int messplace(dummy,mn)
char	dummy[] ;
int	mn ;
{


	if (! (numinarray(tempmessord,ptord,mn))) {

	    ptord += 1 ;
	    tempmessord[ptord] = mn ;
	}

	return 0 ;
}
/* end subroutine (messplace) */


/* searches for num in array. return 1 if found, 0 otherwise */
int numinarray(array,len,num)
int	array[] ;
int	num ;
int	len ;	/* length of given array */
{
	int	i ;


	for (i = 0 ; i < len ; i += 1) {

	    if (array[i] == num)
	        return BAD ;

	}

	return OK ;
}
/* end subroutine (numinarray) */


/* local subroutines */


/* get new mail from the spool area and append to the "incoming" mailbox */
static int getnewmail(pip)
struct proginfo	*pip ;
{
	struct ustat	sf ;
	struct ustat	sb ;

	struct flock	fl ;

	time_t		daytime ;

	offset_t		offset ;

	pid_t	pid ;

	int	rs = SR_OK, rs1 ;
	int	len ;
	int	i, k, l ;
	int	mfd ;
	int	child_stat ;
	int	c = 0 ;
	int	f_exit ;
	int	f_failed ;

	char	command[CMDBUFLEN + 1] ;
	char	infname[MAXPATHLEN + 1] ;
	char	msfname[MAXPATHLEN + 1] ;
	char	*mup = NULL ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getnewmail: ent\n") ;
#endif

/* is there any new mail? */

	if (pip->prog_getmail == NULL)
	    goto ret0 ;

	if ((pip->maildname == NULL) || (pip->maildname[0] == '\0'))
	    goto ret0 ;

/* is the incoming mailbox really a directory? if so => get out */

	if ((pip->mbname_in == NULL) || (pip->mbname_in[0] == '\0'))
	    goto ret0 ;

	rs = mkpath2(infname,pip->folderdname,pip->mbname_in) ;

	if (rs < 0)
	    goto ret0 ;

/* open the mailbox file */

	rs = uc_open(infname,O_FLAGS,0666) ;
	mfd = rs ;
	if (rs < 0) {

	    rs = uc_open(infname,(O_FLAGS | O_CREAT),0666) ;
	    mfd = rs ;
	    if ((rs >= 0) && (pip->gid_mail > 0))
	        u_fchown(mfd,-1,pip->gid_mail) ;

	} /* end if (created mailbox file) */

	if (rs < 0)
	    goto ret0 ;

	if ((rs = u_fstat(mfd,&sb)) < 0)
	    goto ret1 ;

	if (! S_ISREG(sb.st_mode)) {
	    rs = SR_ISDIR ;
	    goto ret1 ;
	}

/* lock the incoming mailbox */

	rs = lockfile(mfd,F_WLOCK,0L,-1,2) ;
	if (rs < 0)
	    goto ret1 ;

/* get any new mail */

#if	CF_GETMAIL
	for (i = 0 ; vecstr_get(&pip->mailusers,i,&mup) >= 0 ; i += 1) {

	    if (mup == NULL) continue ;

	    mkpath2(msfname,pip->maildname,mup) ;

	    if ((u_stat(msfname,&sb) >= 0) && (sb.st_size > 0) &&
	        (! S_ISDIR(sb.stmode)) &&
	        (sperm(&pip->id,&sb,(R_OK | W_OK)) >= 0)) {


#if	CF_SPAWNPROC
	        {
	            struct spawnproc	disp ;

	            char	*av[5] ;


	            disp.disp[0] = SPAWNPROC_DCLOSE ;
	            disp.disp[1] = SPAWNPROC_DDUP ;
	            disp.fd[1] = mfd ;
	            disp.disp[2] = SPAWNPROC_DCLOSE ;

	            av[0] = "GETMAIL" ;
	            av[1] = "-u" ;
	            av[2] = mup ;
	            av[3] = "-Q" ;
	            av[4] = NULL ;
	            rs1 = spawnproc(&disp,pip->prog_getmail,av,pip->envv) ;

	            pid = rs1 ;
	            if (rs1 >= 0)
	                rs1 = u_waitpid(pid,&childstat,0) ;

	        }
#endif /* CF_PROGSPAWB */

	    } /* end if (mail-spool file is available) */

	} /* end for */
#endif /* CF_GETMAIL */

/* unlock the incoming mailbox */


/* did we get anything? */

#ifdef	COMMENT
	f_newmail = FALSE ;
	if (u_fstat(mfd,&sb) > 0) {

	    if (sb.st_size > offset)
	        f_newmail = TRUE ;

	}
#endif

/* cleanup */
ret1:
	if (mfd >= 0)
	    u_close(mfd) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getnewmail) */


static int lockfileend(fd)
int	fd ;
{
	int	rs = SR_OK ;


	return rs ;
}
/* end subroutine (lockfileend) */



