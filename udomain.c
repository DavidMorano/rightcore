/* udomain */

/* get user domain */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-10-01, David A­D­ Morano
        I made up this idea for supporting multiple domains on the same machine
        so that each user could have a different domain name. This idea of
        multiplexing a single machine to appear to be multiple different
        machines is becoming very important in the Internet age!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine looks up the given username and returns the domainname
	for that user.  Per-user domain names are optional and are administered
	through the file 'etc/udomain' located relative to the programroot
	directory that is optionally supplied.  If no programroot is supplied,
	then '/' is used.

	This subroutine uses two different ways to read the UDOMAIN DB based on
	the file it is in.  This is so that in addition to the file being
	"regular" it can also be (secretly) a network file or portal of some
	kind.

	Synopsis:

	int udomain(pr,dbuf,dlen,username)
	const char	pr[] ;
	char		dbuf[] ;
	int		dlen ;
	const char	username[] ;

	Arguments:

	pr		program-root
	dbuf		buffer to receive results (must be MAXPATHLEN in size)
	dlen		length of supplied buffer
	username	specified username to lookup

	Returns:

	<0		error
	>=0		length of returned domain-name


	Format of file entries:

		username	domainname

	Note that this strategy of reading the "udomain" file is obsoleted by
	the use of the 'id' (Internet Domain) key-value pair in the 'user_attr'
	DB of the system!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<filemap.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	UDOMASTDINFNAME
#define	UDOMASTDINFNAME	"/etc/udomain"
#endif

#define	MAXFILESIZE	(2 * 1024 * 1024)


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	strwcmp(const char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct uargs {
	const char	*username ;
	char		*dbuf ;
	int		dlen ;
	int		ul ;
} ;


/* forward references */

static int	udomainer(struct uargs *,cchar *) ;
static int	parseline(struct uargs *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int udomain(cchar *pr,char *dbuf,int dlen,cchar *username)
{
	int		rs = SR_OK ;
	const char	*fname ;
	char		udfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("udomain: username=>%s<\n",username) ;
#endif

	if ((username == NULL) || (dbuf == NULL))
	    return SR_FAULT ;

	dbuf[0] = '\0' ;
	if ((pr != NULL) && (pr[0] != '\0') && (strcmp(pr,"/") != 0)) {
	    fname = udfname ;
	    rs = mkpath2(udfname,pr,UDOMASTDINFNAME) ;
	} else {
	    fname = UDOMASTDINFNAME ;
	}

	if (rs >= 0) {
	    struct uargs	a ; /* fill in the arguments */

	    a.username = username ;
	    a.dbuf = dbuf ;
	    a.dlen = dlen ;
	    a.ul = strlen(username) ;

#if	CF_DEBUGS
	    debugprintf("udomain: ul=%u username=>%s<\n",
	        a.ul,a.username) ;
#endif

	    rs = udomainer(&a,fname) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (udomain) */


/* local subroutines */


static int udomainer(struct uargs *uap,cchar *fname)
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;
	int		ml = 0 ;
	if ((rs = u_stat(fname,&sb)) >= 0) {
	    int		ll ;
	    cchar	*lp ;
	    if (S_ISREG(sb.st_mode) && (sb.st_size < MAXFILESIZE)) {
	        FILEMAP		udfile ;
	        const size_t	mfsize = MAXFILESIZE ;
	        const int	of = O_RDONLY ;

	        if ((rs = filemap_open(&udfile,fname,of,mfsize)) >= 0) {

	            while ((rs = filemap_getline(&udfile,&lp)) > 0) {
	                ll = rs ;

	                rs = parseline(uap,lp,ll) ;
	                ml = rs ;

	                if (ml > 0) break ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = filemap_close(&udfile) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (opened file) */

	    } else {
	        FILEBUF		b ;
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        if ((rs = uc_open(fname,O_RDONLY,0666)) >= 0) {
	            const int	fd = rs ;
	            if ((rs = filebuf_start(&b,fd,0L,0,0)) >= 0) {

	                ml = 0 ;
	                while ((rs = filebuf_readline(&b,lbuf,llen,-1)) > 0) {
	                    ll = rs ;
	                    lp = lbuf ;

	                    rs = parseline(uap,lp,ll) ;
	                    ml = rs ;

	                    if (ml > 0) break ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = filebuf_finish(&b) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (filebuf) */
	            u_close(fd) ;
	        } /* end if (opened file) */

	    } /* end if (type of file) */
	} /* end if (stat) */
	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (udomainer) */


static int parseline(struct uargs *uap,cchar *lbuf,int len)
{
	int		rs = SR_OK ;
	int		sl = len ;
	int		cl ;
	int		ml = 0 ;
	const char	*tp ;
	const char	*sp = lbuf ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("udomain/parseline: len=%d\n",len) ;
#endif

	if ((sl > 0) && (sp[sl-1] == '\n')) {
	    sl -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("udomain/parseline: s=>%t<\n",sp,sl) ;
#endif

	if ((tp = strnchr(sp,sl,'#')) != NULL) {
	    sl = (tp - sp) ;
	}

/* process */

	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    if ((cl == uap->ul) &&
	        (strncmp(uap->username,cp,cl) == 0)) {

#if	CF_DEBUGS
	        debugprintf("udomain/parseline: got match\n") ;
#endif

	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;

	        if ((cl = nextfield(sp,sl,&cp)) > 0) {
	            rs = snwcpy(uap->dbuf,uap->dlen,cp,cl) ;
	        }

	    } /* end if (username match) */
	} /* end if (nextfield) */

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (parseline) */


