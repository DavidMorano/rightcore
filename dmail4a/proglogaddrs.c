/* proglogaddrs */

/* log messages addresses */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-02-01, David A­D­ Morano
        I added a little code to "post" articles that do not have a valid
        newsgroup to a special "dead article" directory in the BB spool area.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module processes one or more mail messages (in appropriate mailbox
        format if more than one) on STDIN. The output is a single file that is
        ready to be added to each individual mailbox in the spool area.

	Things to do:

	Change use of 'sfsubstance()'.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<char.h>
#include	<mailmsg.h>
#include	<mailmsghdrs.h>
#include	<ema.h>
#include	<emainfo.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	MSGLINELEN
#define	MSGLINELEN	(2 * 1024)
#endif

#define	LOGLINELEN	(80 - 16)

#define	DATEBUFLEN	80
#define	STACKADDRBUFLEN	(2 * 1024)

#undef	BUFLEN
#define	BUFLEN		(2 * 1024)

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif

#ifndef	TABLEN
#define	TABLEN		8
#endif

#ifndef	HN_XMAILER
#define	HN_XMAILER	"x-mailer"
#endif

#ifndef	HN_RECEIVED
#define	HN_RECEIVED	"received"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isprintlatin(int) ;

extern int	sfsubstance(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int proglogaddr(struct proginfo *,MAILMSG *,int) ;

#if	CF_DEBUG || CF_DEBUGS
static int debug_fsize(bfile *) ;
#endif


/* local variables */

static const int	hikeys[] = {
	HI_ERRORSTO,
	HI_SENDER,
	HI_REPLYTO,
	HI_FROM,
	HI_TO,
	HI_CC,
	HI_BCC,
	-1
} ;


/* exported subroutines */


int proglogaddrs(pip,msgp)
struct proginfo	*pip ;
MAILMSG		*msgp ;
{
	int		rs = SR_OK ;

	if (pip->f.logmsg) {
	    int		i ;
	    for (i = 0 ; hikeys[i] >= 0 ; i += 1) {
	        rs = proglogaddr(pip,msgp,i) ;
	        if (rs < 0) break ;
	    } /* end for */
	}

	return rs ;
}
/* end subroutine (proglogaddrs) */


/* local subroutines */


static int proglogaddr(pip,msgp,hi)
struct proginfo	*pip ;
MAILMSG		*msgp ;
int		hi ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl ;
	cchar		*sp ;
	const char	*hkey ;

	hkey = mailmsghdrs_names[hi] ;

	if ((sl = mailmsg_hdrval(msgp,hkey,&sp)) >= 0) {
	    EMA		a ;
	    EMA_ENT	*ep ;
	    if ((rs = ema_start(&a)) >= 0) {
	        if ((rs = ema_parse(&a,sp,sl)) >= 0) {
	            int		j ;
	            int		c = 0 ;
	            int		cl ;
	            int		f_route ;
	            const char	*cp ;
	            for (j = 0 ; ema_get(&a,j,&ep) >= 0 ; j += 1) {

	                f_route = TRUE ;
	                if ((cp = ep->rp) == NULL) {
	                    f_route = FALSE ;
	                    cp = ep->ap ;
	                }

	                if (cp != NULL) {

	                    if (pip->f.logmsg) {

	                        logfile_printf(&pip->lh,"  from=%t",
	                            cp,strnlen(cp,(LOGLINELEN - 7))) ;

	                        sp = ep->cp ;
	                        if (sp != NULL) {

	                            cl = sfsubstance(sp,ep->cl,&cp) ;

#if	CF_DEBUG
	                            if (DEBUGLEVEL(4) && (ep->cl > 0))
	                                debugprintf("procmsgs/procmsg: "
	                                    "comment=>%t<\n",
	                                    sp,ep->cl) ;
#endif

	                            if (cl > 0)
	                                logfile_printf(&pip->lh,"       (%t)",
	                                    cp,MIN(cl,(LOGLINELEN - 9))) ;

	                        }

	                        if (f_route && (ep->cp == NULL) &&
	                            (ep->ap != NULL)) {

	                            sp = ep->ap ;
	                            cl = sfsubstance(sp,ep->al,&cp) ;

	                            if (cl > 0)
	                                logfile_printf(&pip->lh,"       (%t)",
	                                    cp,MIN(cl,(LOGLINELEN - 9))) ;

	                        }

	                    } /* end if (logging enabled) */

	                    c += 1 ;
	                    if ((! pip->f.logmsg) && (c > 0))
	                        break ;

	                } /* end if (got an address) */

	            } /* end for */
	        } /* end if (ema_parse) */
	        rs1 = ema_finish(&a) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (ema) */
	} /* end if (have header) */

	return rs ;
}
/* end subroutine (proglogaddr) */


