/* deliverem */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		J.Mukerji						
		David A.D. Morano
 *									
*
 * deliverem() delivers the message to all the addresses to which it is
 * addressed ... der !


************************************************************************/




#include	<sys/utsname.h>
#include	<string.h>
#include	<signal.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	CMDBUFLEN
#define	CMDBUFLEN	((3 * MAXPATHLEN) + 50)
#endif



/* external subroutines */

extern int		logfile_printf() ;

extern			deliv_mbag() ;



/* external variables */

extern struct global	g ;


/* struct for system name */

struct sys_tbl		sname[MAXMBAGS + 1] ;




void deliverem()
{
	struct table	*aname ;

	int   		i ;
	int		rs ;

	char		*realname ;
	char		templist[BUFLEN + 1] ;


/* advise of wait */

	if ((iswait > 0) && g.f.interactive && (ex_mode == EM_PCSMAIL))
	    bprintf(g.ofp,
	        "please wait while the mail is sent ...\n") ;

/* convert raw names to system user names */

	tot_sys = 0 ;
	if (tonames > 0) {

	    strcpy( templist, recipient ) ;

	    realname = strtok( templist, ",") ;

	    while (realname != NULL) {

	        cookname(realname,1) ;

	        realname = strtok( 0, ",") ;

	    } /* end while */

	}

	if (ccnames > 0) {

	    strcpy( templist, copyto ) ;

	    realname = strtok( templist, ",") ;

	    while (realname != NULL) {

	        cookname(realname,1) ;

	        realname = strtok( 0, ",") ;

	    } /* end while */

	}

	if (bccnames > 0) {

	    strcpy(templist, bcopyto) ;

	    realname = strtok( templist, ",") ;

	    while (realname != NULL) {

	        cookname(realname,1) ;

	        realname = strtok( 0, ",") ;

	    } /* end while */

	}

/* remove myname from list  */

	if ((myname >= 0) && (name[myname]->mail > 10))
	    name[myname]->mail = 0 ;

/* first make sure that the "from" field has not been forged */

	getfield(tempfile,"From:",syscom) ;

	if (strcmp(from,syscom) != 0) {

#ifdef	COMMENT
	    bprintf(g.efp,"the from field has been forged\n") ;
#endif

	    if (*sentby == '\0') strcpy(sentby, from) ;

	    strcpy( from, syscom ) ;

	}

/* append file if it exists */

	if (isappend)
	    fappend(appfile,tempfile) ;

/* forward file if it exist */

	if (isforward)
	    fappend(forwfile,tempfile) ;

/* send copy to originator */

#ifdef	COMMENT
	if (copy && ((myname < 0) || (name[myname]->mail == 0)))
	    deliv_filecopy(tempfile) ;
#else
	if (copy && ((myname < 0) || (name[myname]->mail == 0))) {

	    logfile_printf(&g.lh,"creating a file copy, myname=%d mail=%d\n",
	        myname,(myname >= 0) ? name[myname]->mail : -1) ;

	    if ((rs = deliv_filecopy(tempfile)) < 0) {

	        logfile_printf(&g.lh,"file copy failed, rs=%d\n",
	            rs) ;

	        logfile_printf(&g.eh,"deliverem: file copy failed, rs=%d\n",
	            rs) ;

	    } 

	}
#endif

/* set up messages */

#ifdef	COMMENT
/* open utmp */

	if (verify && ((fwho = fopen("/etc/utmp","r")) == NULL) && 
	    (ex_mode == EM_PCSMAIL))

	    bprintf(g.efp,"%s: sorry, cannot verify recipients\n",
	        g.progname) ;
#endif

	goodmail = 0 ;
	while (tot_sys-- > 0) {

	    deliv_mbag(&sname[tot_sys]) ;

	} /* end while (recipient's loop) */

	unlink(tmpf) ;		/* remove notify file if it exists */

#if	F_CF_DEBUG
	if (g.f.debug)
	fileinfo(tempfile,"deliverem") ;
#endif

	unlink(tempfile) ;	/* remove stored message */

/* if forwfile exists remove that too		*/

	if (isforward > 0) unlink(forwfile) ;

/* notify of undeliverable mail */

	if (ex_mode == EM_PCSMAIL) {

/*
	The idea here is that any address left in the local mail list
	is an address that the message failed to be sent to.
*/

	    int	c_funny = 0 ;


	    for (i = 0 ; i < tablelen ; i += 1) {

	        aname = name[i] ;
	        if (aname->mail == 0) continue ;

	        if (strncmp(aname->mailaddress,"NO",2) == 0) continue ;

	        if ((! verify) && ((aname->mail/10) > 0)) continue ;

/* verify mail failure */

	        c_funny += 1 ;
	        if (*(aname->realname) != '\0') {

#ifdef	COMMENT
	            if (verify)
	                bprintf(g.efp,
	                    "%s: cannot send mail to '%s'\n", 
	                    g.progname,aname->realname) ;
#endif

#ifdef	LOGFILE
	            logfile_printf(&g.lh,"weird to \"%s\" <%s>\n",
	                aname->realname,aname->mailaddress) ;
#endif

	        }

	    } /* end for */

#ifdef	COMMENT
	    if (verify && (c_funny > 0)) {

	        fprintf(stdout,
	            "%s: %d weird type mail message%s sent\n", 
	            g.progname,c_funny,
	            (c_funny > 1) ? "s were" : " was") ;

	    }
#endif

	    if (verify || ((iswait > 0) && g.f.interactive)) {

	        bprintf(g.ofp,"%s: %d message%s been sent\n",
	            g.progname,goodmail,(goodmail != 1) ? "s have" : " has") ;

	    }
	}

}
/* end subroutine (deliverem) */


int deliv_filecopy(mfname)
char	mfname[] ;
{
	bfile	mfile, *mfp = &mfile ;
	bfile	cfile, *cfp = &cfile ;

	int	l, rs ;

	char	fnamebuf[MAXPATHLEN + 1] ;
	char	cmdbuf[CMDBUFLEN + 1] ;


/* check for $HOME/mail/copy file */

	bufprintf(fnamebuf,MAXPATHLEN,"%s/mail/copy", g.homedir) ;

#ifdef	COMMENT
	if ((rs = bopen(cfp,fnamebuf,"wca",0666)) >= 0) {

	    if ((rs = bopen(mfp,mfname,"r",0666)) >= 0) {

	        bprintf(cfp,"From %s %s\n",g.username,g.envdate) ;

	        while ((l = bread(mfp,fnamebuf,MAXPATHLEN)) > 0) {

	            if ((rs = bwrite(cfp,fnamebuf,l)) < l) {

	                bprintf(g.efp,
	                    "%s: could not write a file copy (rs %d)\n",
	                    g.progname,rs) ;

	                bprintf(g.efp,
	                    " - possible full file system\n") ;

	                break ;
	            }

	        } /* end while */

		rs = bclose(mfp) ;

	    } /* end if (open of mail message file) */

		if (rs >= 0) rs = bclose(cfp) ;

		else bclose(cfp) ;

	} /* end if (open of "copy" mailbox) */

	if (rs < 0) {

	    logfile_printf(&g.lh,"error on making a user file copy (rs %d)\n",
	        rs) ;

	    logfile_printf(&g.eh,
	        "deliv_filecopy: error on file copy (rs %d)\n",
	        rs) ;

	    sprintf(fnamebuf,"rmail %s < %s",g.username,mfname) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"deliv_filecopy: SYSTEM> %s\n",buf) ;
#endif

	    rs = system(buf) ;

	}
#else
	bufprintf(cmdbuf,CMDBUFLEN,"%s -e %s -o %s",g.prog_pcscl,
		mfname,fnamebuf) ;

	{
		bfile	*fpa[3] ;

		pid_t	pid ;

		int	i, childstat ;


		for (i = 0 ; i < 3 ; i += 1)
			fpa[i] = (bfile *) NULL ;

		if ((rs = bopencmd(fpa,cmdbuf)) >= 0) {

		pid = (pid_t) rs ;
	        for (i = 0 ; i < 3 ; i += 1) 
			bclose(fpa[i]) ;

	        u_waitpid(pid,&childstat,0) ;

	    } /* end if */

	} /* end block */
#endif

	return (rs < 0) ? rs : OK ;
}
/* end subroutine (deliv_filecopy) */



