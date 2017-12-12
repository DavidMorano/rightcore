/* proglogout */

/* process the service names given us */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was adopted from the DWD program.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine prints out the standard-output or standard-error of a
        spawned process to the log-file service.

	Returns:

	OK	may not really matter in the current implementation!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<linefold.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	MAXOUTLEN
#define	MAXOUTLEN	64
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(char *,int,int *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	isprintlatin(int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* externals variables */


/* local structures */


/* forward references */

static int	procfile(PROGINFO *,cchar *,int,cchar *) ;
static int	procline(PROGINFO *,int,const char *,int) ;


/* local variables */

static cchar	blanks[] = "        " ;


/* exported subroutines */


int proglogout(PROGINFO *pip,cchar *msgstr,cchar *fname)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proglogout: fname=%s\n",fname) ;
#endif

	if (fname != NULL) {
	    if (fname[0] != '\0') {
	        if (pip->open.logprog) {
	    	    int		msglen = 0 ;
	            if (msgstr != NULL) {
	                msglen = strlen(msgstr) ;
	                while (msglen && (msgstr[msglen-1] == '>')) {
	                    msglen -= 1 ;
			}
	            } /* end if (non-null) */
	            rs = procfile(pip,msgstr,msglen,fname) ;
	            wlen = rs ;
	        } /* end if (logprog) */
	    } /* end if (non-nul) */
	} /* end if (fname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proglogout: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proglogout) */


/* local subroutines */


static int procfile(PROGINFO *pip,cchar *np,int nl,cchar *fname)
{
	bfile		ofile, *fp = &ofile ;
	const int	columns = LOGFILE_FMTLEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = bopen(fp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		line = 0 ;
	    char	lbuf[LINEBUFLEN + 2] ;

	    while ((rs = breadline(fp,lbuf,llen)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;

#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("proglogout: l=>%t<\n",
			lbuf,strlinelen(lbuf,len,50)) ;
#endif

	        if (len > 0) {

	            if ((line == 0) && (np != NULL)) {
	                proglog_printf(pip,"%t>",np,nl) ;
		    }

	            if (rs >= 0) {
	                rs = procline(pip,columns,lbuf,len) ;
	                wlen += rs ;
	            }

		} /* end if (non-zero) */

		line += 1 ;
		if (rs < 0) break ;
	    } /* end while (reading line) */

	    rs1 = bclose(fp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (server output-file) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procline(PROGINFO *pip,int columns,cchar *lp,int ll)
{
	const int	indent = 2 ;
	const int	leadlen = 4 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		textlen ;
	int		wlen = 0 ;

	textlen = (columns - leadlen) ;
	if (textlen > 0) {
	    LINEFOLD	lfd, *lfdp = &lfd ;
	    if ((rs = linefold_start(lfdp,textlen,indent,lp,ll)) >= 0) {
	        int	i ;
	        int	cl ;
	        int	ind = 0 ;
		cchar	*fmt = "| %t%t\n" ;
	        cchar	*cp ;
	        for (i = 0 ; (cl = linefold_get(lfdp,i,&cp)) >= 0 ; i += 1) {
		    wlen += cl ;
	            rs1 = proglog_printf(pip,fmt,blanks,ind,cp,cl) ;
	            ind = indent ;
		    if (rs1 < 0) break ;
	        } /* end for */
	        rs1 = linefold_finish(lfdp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (linefold) */
	} /* end if (non-zero) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


