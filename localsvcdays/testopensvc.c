/* testopensvc */
/* lang=C89 */

/* test opening a service */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<fsdir.h>
#include	<filebuf.h>
#include	<ucmallreg.h>
#include	<localmisc.h>

#ifndef FILEBUF_RCNET
#define	FILEBUF_RCNET	4		/* read-count for network */
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	VARDEBUGFNAME	"TESTOPENSVC_DEBUGFILE"

extern int	bufprintf(char *,int,const char *,...) ;
extern int	fbwrite(FILE *,const void *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar 	*getourenv(cchar **,cchar *) ;

extern char	*timestr_logz(time_t,char *) ;

/* forward references */

static int dumpfile(FILE *,int,int) ;
static int dumpdir(int,int) ;


/* exported subroutines */

/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	FILE		*ofp = stdout ;
#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif
	int		rs = SR_OK ;
	int		ex = 0 ;

#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting rs=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if (argv != NULL) {
	    int		ai ;
	    for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	        const int	of = O_RDONLY ;
	        cchar		*fn = argv[ai] ;
#if	CF_DEBUGS
	        debugprintf("main: fn=%s\n",fn) ;
#endif
	        if ((rs = uc_open(fn,of,0666)) >= 0) {
	            struct ustat	sb ;
	            int			fd = rs ;
#if	CF_DEBUGS
	            debugprintf("main: uc_open() rs=%d\n",rs) ;
#endif
	            if ((rs = u_fstat(fd,&sb)) >= 0) {
#if	CF_DEBUGS
	                debugprintf("main: mode=\\o%09o\n",sb.st_mode) ;
#endif
	                if (S_ISDIR(sb.st_mode)) {
	                    rs = dumpdir(fd,of) ;
	                } else {
	                    rs = dumpfile(ofp,fd,of) ;
	                }
	            } /* end if (fstat) */
#if	CF_DEBUGS
	            debugprintf("main: u_fstat-out rs=%d\n",rs) ;
#endif
	            u_close(fd) ;
	        } else if (rs == SR_NOTFOUND) {
#if	CF_DEBUGS
	        debugprintf("main: uc_open-bad rs=%d\n",rs) ;
#endif
	            printf("not_found fn=%s (%d)\n",fn,rs) ;
	            rs = SR_OK ;
	        } else {
	            printf("open-error fn=%s (%d)\n",fn,rs) ;
		}
#if	CF_DEBUGS
	        debugprintf("main: uc_open-out rs=%d\n",rs) ;
#endif
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (arguments) */

	if (rs < 0) {
	    printf("failure (%d)\n",rs) ;
	}

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
		UCMALLREG_CUR	cur ;
		UCMALLREG_REG	reg ;
		const int	size = (10*sizeof(uint)) ;
		const char	*ids = "main" ;
		uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
			mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
		ucmallreg_curbegin(&cur) ;
		while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
		    debugprinthexblock(ids,80,reg.addr,reg.size) ;
		}
		ucmallreg_curend(&cur) ;
	    }
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugclose() ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int dumpfile(FILE *ofp,int fd,int of)
{
	FILEBUF		b ;
	const int	to = 5 ;
	const int	fo = (of | O_NETWORK) ;
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("main/dumpfile: ent to=%d\n",to) ;
#endif
	if ((rs = filebuf_start(&b,fd,0L,0,fo)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		li ;
	    char	lbuf[LINEBUFLEN+1] ;
	    while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	        int	len = rs ;
		fbwrite(ofp,lbuf,len) ;
#if	CF_DEBUGS
		{
	            debugprintf("main/dumpfile: readline() len=%d\n",len) ;
	            li = strlinelen(lbuf,len,70) ;
	            lbuf[li] = '\0' ;
	            debugprintf("l=>%s<\n",lbuf) ;
		}
#endif /* CF_DEBUGS */
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = filebuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

#if	CF_DEBUGS
	debugprintf("main/dumpfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dumpfile) */


static int dumpdir(int fd,int of)
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	const int	dlen = MAXPATHLEN ;
	int		rs ;
	int		rs1 ;
	char		dbuf[USERNAMELEN+1] ;

#if	CF_DEBUGS
	debugprintf("main/dumpdir: ent\n") ;
#endif
	if ((rs = bufprintf(dbuf,dlen,"/dev/fd/%u",fd)) >= 0) {
	    if ((rs = fsdir_open(&d,dbuf)) >= 0) {
	        while ((rs = fsdir_read(&d,&de)) > 0) {
	            printf("e=%s\n",de.name) ;
	        } /* end while */
	        rs1 = fsdir_close(&d) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (fsdir) */
	} /* end if (bufprintf) */

#if	CF_DEBUGS
	debugprintf("main/dumpdir: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (dumpdir) */


