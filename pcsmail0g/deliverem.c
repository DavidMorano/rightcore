/* deliverem */


#define	DEBUG	0


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
#include	<bfile.h>

#include	"config.h"
#include	"smail.h"
#include	"localmisc.h"



/* external subroutines */

extern struct passwd	*getpwuid() ;

extern int		logfile_printf() ;

extern			deliv_mbag() ;



/* external variables */

extern struct global	g ;


/* struct for system name */

extern struct utsname utsname ;


struct sys_tbl sname[MAXMBAGS] ;


deliverem()
{
	struct table	*aname ;

	int   		i ;

	char		*realname ;
	char		templist[BUFLEN] ;


/* advise of wait */

	if ((iswait > 0) && f_interactive && (ex_mode == EM_PCSMAIL))
	    printf("please wait while the mail is sent ...\n") ;

/* convert raw names to system user names */

	tot_sys = 0 ;
	if (tonames > 0) {

	    strcpy( templist, recipient ) ;

	    realname = strtok( templist, ",") ;

	    while (realname != NULL) {

	        cookname(realname,1) ;

	        realname = strtok( 0, ",") ;

	    }
	}

	if (ccnames > 0) {

	    strcpy( templist, copyto ) ;

	    realname = strtok( templist, ",") ;

	    while (realname != NULL) {

	        cookname(realname,1) ;

	        realname = strtok( 0, ",") ;

	    }
	}

	if (bccnames > 0) {

	    strcpy(templist, bcopyto) ;

	    realname = strtok( templist, ",") ;

	    while (realname != NULL) {

	        cookname(realname,1) ;

	        realname = strtok( 0, ",") ;

	    }
	}

/* remove myname from list  */

	if ((myname >= 0) && (name[myname]->mail > 10)) {

	    name[myname]->mail = 0 ;
	}

/* first make sure that the "from" field has not been forged */

	getfield(tempfile,"From:",syscom) ;

	if (strcmp(from,syscom)) {

#ifdef	COMMENT
	    fprintf(stderr,"the from field has been forged\n") ;
#endif

	    if (*sentby == '\0') strcpy(sentby, from) ;

	    strcpy( from, syscom ) ;

	}

/* append file if it exists */

	if (isappend) {

	    fappend(appfile,tempfile) ;

	}

/* forward file if it exist */

	if (isforward) {

	    fappend(forwfile,tempfile) ;

	}

/* send copy to originator */

	if (copy && ((myname < 0) || (name[myname]->mail == 0))) {

	    deliv_filecopy(tempfile) ;

	}

/* set up messages */

#ifdef	COMMENT
/* open utmp */

	if (verify && ((fwho = fopen("/etc/utmp","r")) == NULL) && 
	    (ex_mode == EM_PCSMAIL))

	    fprintf(stderr,"%s: sorry, cannot verify recipients\n",
	        g.progname) ;
#endif

	goodmail = 0 ;
	while (tot_sys-- > 0) {

	    deliv_mbag(&sname[tot_sys]) ;

	}	/* end of recipient's loop */

	unlink(tmpf) ;		/* remove notify file if it exists */

	unlink(tempfile) ;	/* remove stored message */

/*	if forwfile exists remove that too		*/

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
	                fprintf(stderr,
	                    "%s: cannot send mail to '%s'\n", 
	                    g.progname,aname->realname) ;
#endif

#ifdef	LOGFILE
	            logfile_printf("weird to \"%s\" <%s>\n",
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

	    if (verify || ((iswait > 0) && f_interactive)) {

	        fprintf(stdout, "%s: %d message%s been sent\n",
	            g.progname,goodmail,(goodmail != 1) ? "s have" : " has") ;

	    }
	}

}
/* end subroutine (deliverem) */


int deliv_filecopy(mfname)
char	*mfname ;
{
	bfile	mfile, *mfp = &mfile ;
	bfile	cfile, *cfp = &cfile ;

	int	l, rs ;

	char	buf[BUFLEN + 1] ;


/* check for $HOME/mail/copy file */

	sprintf(buf,"%s/mail/copy", g.homedir) ;

	if ((rs = bopen(cfp,buf,"a")) >= 0) {

	    if ((rs = bopen(mfp,mfname,"r",0666)) >= 0) {

	        bprintf(cfp,"From %s %s\n",g.username,g.datetime) ;

	            while ((l = bread(mfp,buf,BUFLEN)) > 0) {

	            if ((rs = bwrite(cfp,buf,l)) < l) {

	                bprintf(g.efp,
	                    "%s: could not write a file copy\n",
	                    g.progname) ;

	                    bprintf(g.efp,
	                    " - possible full file system\n") ;

	                    break ;
	            }

	        } /* end while */

	    }
	}

	if (rs < 0) {

	    sprintf(buf,"rmail %s < %s",g.username,mfname) ;

	        rs = system(buf) ;

	}

	return rs ;
}
/* end subroutine (deliv_filecopy) */


