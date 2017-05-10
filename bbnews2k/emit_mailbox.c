/* emit_mailbox */

/* emit (process) an article */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1994-11-01, David A­D­ Morano

	- added a mode to intercept for mailbox use


	= 1994-12-01, David A­D­ Morano

	- modified to only print out header fields that a user
	  is normally interested in


	= 1995-07-01, David A­D­ Morano

	- extensively modified to add:
		article follow-up capability
		article previous
		article printing
		article piping & redirecting


*/

/* Copyright © 1994,1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is one of the "EMIT" subroutines used for
	"emitting" articles in different ways.

	Synopsis:

	int emit_mailbox(pip,dsp,ai,aep,ngdir,af)
	struct proginfo	*pip ;
	MKDIRLIST_ENT	*dsp ;
	int		ai ;
	ARTLIST_ENT	*aep ;
	char		ngdir[] ;
	char		af[] ;

	Arguments:

	pip		program information pointer
	dsp		user structure pointer
	ai		article index within newsgroup
	aep		article ARTLIST_ENT pointer
	ngdir		directory (relative to spool directory) of article
	af		article base file name

	Returns:

	<0		error
	>=0		EMIT-code


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<string.h>
#include	<time.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mkdirlist.h"
#include	"artlist.h"
#include	"headerkeys.h"


/* local defines */

#define	MSGENVDATELEN	100
#define	MSGENVLEN	(MSGENVDATELEN+100)


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	bufprintf(char *,int,const char *,...) ;

extern int	progmsgenv_begin(struct proginfo *) ;
extern int	progmsgenv_envstr(struct proginfo *,char *,int) ;
extern int	progmsgenv_end(struct proginfo *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* forward references */

static int procenv(struct proginfo *,bfile *,bfile *) ;


/* local variables */


/* exported subroutines */


int emit_mailbox(pip,dsp,ai,ap,ngdir,af)
struct proginfo	*pip ;
MKDIRLIST_ENT	*dsp ;
int		ai ;
ARTLIST_ENT	*ap ;
const char	ngdir[] ;
const char	af[] ;
{
	bfile	*ofp = pip->ofp ;

	int	rs = SR_OK ;
	int	wlen = 0 ;

	const char	*nd = pip->newsdname ;

	char	afname[MAXPATHLEN + 1] ;


	if (ngdir == NULL) return EMIT_DONE ;
	if (af == NULL) return EMIT_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("emit_mailbox: ng=%s\n",ngdir) ;
	    debugprintf("emit_mailbox: af=%s\n",af) ;
	}
#endif

	if ((rs = mkpath3(afname,nd,ngdir,af)) >= 0) {
	    bfile	afile, *afp = &afile ;
	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
		int		len ;
		int		line = 0 ;
		char		lbuf[LINEBUFLEN+1] ;

		while ((rs = breadline(afp,lbuf,llen)) > 0) {
		    len = rs ;

		    if (line++ == 0) {
			if (strncmp(lbuf,"From ",5) != 0) {
			    rs = procenv(pip,ofp,afp) ;
			    wlen += rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("emit_mailbox: procenv() rs=%d\n",rs) ;
#endif
			}
		    }
		    if (rs >= 0) {
	                rs = bwrite(ofp,lbuf,len) ;
			wlen += rs ;
		    }

		    if (rs < 0) break ;
		} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("emit_mailbox: while-out rs=%d\n",rs) ;
#endif

		bclose(afp) ;
	    } /* end if (bfile) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("emit_mailbox: bfile-out rs=%d\n",rs) ;
#endif
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("emit_mailbox: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (emit_mailbox) */


/* local subroutines */


static int procenv(pip,ofp,afp)
struct proginfo	*pip ;
bfile		*ofp ;
bfile		*afp ;
{
	bfile_stat	sb ;
	int	rs ;
	int	wlen = 0 ;

	if ((rs = bstat(afp,&sb)) >= 0) {
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;
	    if ((rs = getusername(ubuf,ulen,sb.st_uid)) >= 0) {
		const int	dlen = MSGENVDATELEN ;
		char		dbuf[MSGENVDATELEN+1] ;
		if ((rs = progmsgenv_envstr(pip,dbuf,dlen)) >= 0) {
		    const int	elen = MSGENVLEN ;
		    const char	*fmt = "From %s %s" ;
		    char	ebuf[MSGENVLEN+1] ;
		    if ((rs = bufprintf(ebuf,elen,fmt,ubuf,dbuf)) >= 0) {
			rs = bprintline(ofp,ebuf,rs) ;
			wlen += rs ;
		    }
		}
	    } /* end if (getusername) */
	} /* end if (bstat) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procenv) */


