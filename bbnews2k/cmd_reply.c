/* cmd_reply */

/* lets the user type a reply to the specified message */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1994-01-01, Jishnu Mukerji
	Originally written.

	= 1994-02-01, David A­D­ Morano
        I wrote this from scratch when I took over the code. The previous code
        was a mess (still is in many places!).

	= 1998-11-22, David A­D­ Morano
        I wrote this from scratch when I took over the code. The previous code

	= 2018-01-22, David A­D­ Morano
	Refactored and cleaned up substantially.

*/

/* Copyright © 1994,1998,2018 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 			Jishnu Mukerji
 *                      David A.D. Morano


	This subroutine was modified several times by J.Mukerji between
	4.1.84 and 8.1.84 to do the following :
 *									   
 * (i)	Interface with PCSMAIL using a file rather than the cmdbuf line.   
 * (ii)	Pass a copy of the original message to PCSMAIL so that it is avail- 
 *	able for displaying in the +edit mode				   
 *	This is done only if the editor specified in the environment	   
 *	variable ED can make good use of it.				   


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ema.h>
#include	<localmisc.h>

#include	"artlist.h"
#include	"headerkeys.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define NOTFOUND	2
#define	CMDLEN		((2 * BUFSIZE) + 12)


/* external subroutines */

extern int	sfdirname(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	bufprintf(char *,int,cchar *,...) ;

extern int	getfield(cchar *,cchar *,char *) ;


extern int	uc_system(cchar *) ;


/* external variables */


/* exported subroutines */


int cmd_reply(pip,ap,ngdir,afname)
PROGINFO	*pip ;
ARTLIST_ENT	*ap ;
const char	ngdir[] ;
const char	afname[] ;
{
	EMA		a ;
	bfile		tfile, *tfp = &tfile ;
	const uid_t	uid = pip->uid ;
	const gid_t	gid = pip->gid ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ret = 0 ;
	int		l1, l2 ;
	cchar		*cp1, *cp2, *cp3 ;
	cchar		*tmp1 ;
	cchar		*fmt ;
	char		tempfile[BUFSIZE + 1] ;
	char		to[BUFSIZE + 1] ;
	char		subj[BUFSIZE + 1] ;
	char		oldsubj[BUFSIZE + 1] ;
	char		messid[BUFSIZE + 1], reference[BUFSIZE + 1] ;
	char		cmdbuf[CMDLEN + 1] ;

	to[0] = '\0' ;
	ret = getfield(afname,HK_REPLYTO,to) ;

	if (to[0] == '\0')
	    ret = getfield(afname,HK_FROM,to) ;

	getfield(afname,HK_MESSAGEID,messid) ;

	getfield(afname,HK_REFERENCES,reference) ;

	if (ret == 1)
	    goto ret0 ;

	if (ret == NOTFOUND) {
	    fmt = "%s: cannot find who the article was from -- aborting\n" ;
	    bprintf(pip->ofp,fmt,pip->progname) ;
	    goto ret0 ;
	}

	if ((rs = ema_starter(&a,to,-1)) >= 0) {
	    EMA_ENT	*ep ;
	    if ((rs = ema_get(&a,0,&ep)) >= 0) {

/* get subject */

	        oldsubj[0] = '\0' ;
	        getfield(afname,"subject",oldsubj) ;

	        if (oldsubj[0] == '\0')
	            getfield(afname,"TITLE:",oldsubj) ;

	        tmp1 = oldsubj+strspn(oldsubj," ") ;

	        if (strncmp ("re:", tmp1 ,3) != 0) {
	            strcpy (subj,"re: ") ;

	        } else
	            strcpy (subj,"");		/* already has re: */

	        strcat(subj, tmp1) ;

	        strcpy(tempfile,"/tmp/replyXXXXXX") ;

	        mktemp(tempfile) ;

	        if ((rs = bopen(tfp,tempfile,"wct",0664)) >= 0) {

	            uc_chown(tempfile,uid,gid) ;

	            uc_chmod(tempfile, 0640 ) ;

/* put reply message template in reply file */

	            if (ep->rp != NULL) {
	                bprintf(tfp,"TO: %s <%s>\n",ep->ap,ep->rp) ;
	            } else {
	                bprintf(tfp,"TO: %s\n",ep->ap) ;
	            }

	            if (*subj) {
	                bprintf(tfp,"SUBJECT: %s\n",subj) ;
	            }

	            if ((*reference != NULL) || (*messid != NULL)) {
	                bprintf(tfp,"REFERENCES: %s %s\n",reference, messid) ;
	            }

	            l1 = sfdirname(pip->prog_mailer,-1,&cp1) ;

	            l2 = sfbasename(pip->prog_mailer,-1,&cp2) ;

	            cp3 = "" ;
	            if ((l2 > 1) && (*cp2 != 'o') && (*cp2 != 'n')) {
	                if ((pip->prefix != NULL) && (pip->prefix[0] != '\0')) {
	                    if (pip->f.newprogram) {
	                        cp3 = "n" ;
	                    }
	                }
	            }

	            bufprintf(cmdbuf,CMDLEN,"%t%s%s%t -verify f=%s re=",
	                ((l1 > 0) ? cp1 : ""),l1,
	                ((l1 > 0) ? "/" : ""),
	                cp3,
	                cp2,l2,
	                tempfile) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {
	                debugprintf("reply: CMD> %s\n",cmdbuf) ;
	            }
#endif /* CF_DEBUG */

	            bclose(tfp) ;
	        } /* end if (bfile) */

	        if (rs >= 0) {
	            uc_system(cmdbuf) ;
	            uc_unlink(tempfile) ;
	        }

	    } /* end if (ema_get) */
	    rs1 = ema_finish(&a) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ema) */

ret0:
	return rs ;
}
/* end subroutine (cmd_reply) */


