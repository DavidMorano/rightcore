/* sysinfo */

/* System-Information UNIX® System interposer */


#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a version of |sysinfo(2)| that is preloaded to over-ride the
	standard UNIX® system version.

	Q. Is this multi-thread safe?
	A. Since it is a knock-off of an existing UNIX® system LIBC (3c)
	   subroutine that is already multi-thread safe -- then of course
	   it is!

	Q. Is this much slower than the default system version?
	A. No, not really.

	Q. How are we smarter than the default system version?
	A. Let me count the ways:
		+ value optionally from environment
		+ value optionally from a configuration file
		+ customizable built-in compiled default
		+ value is cached!

	Q. Why are you so smart?
	A. I do not know.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<limits.h>
#include	<unistd.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<vsystem.h>
#include	<preload.h>
#include	<buffer.h>
#include	<vecstr.h>
#include	<filebuf.h>
#include	<ctdec.h>
#include	<cthex.h>
#include	<localmisc.h>


/* local defines */

#define	SYSINFO			struct sysinfo_head
#define	SYSINFO_HWSERIAL	SI_HW_SERIAL
#define	SYSINFO_SETDOMAIN	SI_SET_SRPC_DOMAIN
#define	SYSINFO_BUFLEN		(10*MAXPATHLEN)
#define	SYSINFO_TTL		(2*3600)	/* two (2) hours */
#define	SYSINFO_FNAME		"/etc/default/sysinfo"
#define	SYSINFO_LOGDOMAIN	"/var/tmp/setoncdomain"

#ifndef	VARHWSERIAL
#define	VARHWSERIAL	"HWSERIAL"
#endif

#ifndef	VARHOSTID
#define	VARHOSTID	"HOSTID"
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int129_t in decimal */
#endif

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	32		/* can hold 'int128_t' in hexadecimal */
#endif

#ifndef	UIDSYS
#define	UIDSYS		100
#endif

#define	NDF		"sysinfo.deb"


/* typedefs */

typedef	int (*sysinfo_func)(int,char *,long) ;


/* external subroutines */

extern int	cfdecui(const char *,int,uint *) ;
extern int	cfnumui(const char *,int,uint *) ;
extern int	cfhexui(const char *,int,uint *) ;
extern int	cfhexul(const char *,int,ulong *) ;
extern int	vecstr_envfile(VECSTR *,const char *) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* local structures */

struct sysinfo_head {
	sysinfo_func	func ;
} ;


/* forward references */

static int	sysinfo_hwserial(char *,long) ;
static int	sysinfo_setdomain(int,char *,long) ;
static int	sysinfo_setdomainlog(int,char *,long,long) ;
static int	sysinfo_next(int,char *,long) ;

static int	sysinfo_varhwserial(BUFFER *) ;
static int	sysinfo_varhostid(BUFFER *) ;
static int	sysinfo_var(BUFFER *,const char *) ;
static int	sysinfo_file(BUFFER *) ;
static int	sysinfo_def(BUFFER *) ;

#ifdef	COMMENT
static int	sysinfo_defer(BUFFER *,int,const char *) ;
#endif /* COMMENT */

static int	uload_hwserial(char *,const char *,long) ;


/* local variables */

static SYSINFO	sysinfo_data ; /* zero-initialized */

static int	(*sysinfos[])(BUFFER *) = {
	sysinfo_varhwserial,
	sysinfo_varhostid,
	sysinfo_file,
	sysinfo_def,
	NULL
} ;


/* exported subroutines */


int sysinfo(int name,char *ubuf,long len)
{
	int		rc = 0 ;
#if	CF_DEBUGN
	nprintf(NDF,"sysinfo: ent n=%u\n",name) ;
#endif
	ubuf[0] = '\0' ;
	if (name == SYSINFO_HWSERIAL) {
	    rc = sysinfo_hwserial(ubuf,len) ;
	} else if (name == SYSINFO_SETDOMAIN) {
	    rc = sysinfo_setdomain(name,ubuf,len) ;
	} else {
	    rc = sysinfo_next(name,ubuf,len) ;
	} /* end if */
#if	CF_DEBUGN
	nprintf(NDF,"sysinfo: ret rc=%d\n",rc) ;
	nprintf(NDF,"sysinfo: ret ubuf=>%s<\n",ubuf) ;
#endif
	return rc ;
}
/* end subroutine (sysinfo) */


/* local subroutines */


static int sysinfo_next(int name,char *rbuf,long len)
{
	SYSINFO		*sip = &sysinfo_data ;
	int		rc = 0 ;

	if (sip->func == NULL) {
	    void		*sp = dlsym(RTLD_NEXT,"sysinfo") ;
	    sip->func = (sysinfo_func) sp ;
	}

	if (sip->func != NULL) {
	    sysinfo_func	func = (sysinfo_func) sip->func ;
	    rc = (*func)(name,rbuf,len) ;
	}

	return rc ;
}
/* end subroutine (sysinfo_next) */


static int sysinfo_setdomain(int name,char *rbuf,long len)
{
	long		rc ;
	rc = sysinfo_next(name,rbuf,len) ;
	sysinfo_setdomainlog(name,rbuf,len,rc) ;
	return rc ;
}
/* end subroutine (sysinfo_setdomain) */


static int sysinfo_setdomainlog(int name,char *rbuf,long len,long rc)
{
	const mode_t	om = 0666 ;
	const int	of = (O_WRONLY|O_APPEND) ;
	int		rs ;
	int		rs1 ;
	cchar		*fn = SYSINFO_LOGDOMAIN ;
	if ((rs = u_open(fn,of,om)) >= 0) {
	    FILEBUF	b ;
	    const int	fd = rs ;
	    if ((rs = filebuf_start(&b,fd,0L,0,0)) >= 0) {
		const int	rl = (int) (len & INT_MAX) ;
		cchar		*fmt = "setoncdomain dn=%t (%ld)\n" ;
		filebuf_printf(&b,fmt,rbuf,rl,rc) ;
		rs1 = filebuf_finish(&b) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */
	    u_close(fd) ;
	} /* end if (file) */
	return rs ;
}
/* end subroutine (sysinfo_setdomainlog) */


static int sysinfo_hwserial(char *ubuf,long len)
{
	const int	rlen = SYSINFO_BUFLEN ;
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;
	char		*rbuf ;
#if	CF_DEBUGN
	nprintf(NDF,"sysinfo_hwserial: ent\n") ;
#endif
	if ((rs = uc_malloc((rlen+1),&rbuf)) >= 0) {
	    const int	di = PRELOAD_DHWSERIAL ;
	    if ((rs = preload_get(di,rbuf,rlen)) == 0) {
		BUFFER		b ;
		const int	blen = MAXNAMELEN ;
		if ((rs = buffer_start(&b,blen)) >= 0) {
		    int		i ;
		    for (i = 0 ; sysinfos[i] != NULL ; i += 1) {
			rs = (*sysinfos[i])(&b) ;
			if (rs != 0) break ;
		    } /* end for */
		    if (rs > 0) {
	    		cchar	*bp ;
	   		if ((rs = buffer_get(&b,&bp)) >= 0) {
			    const int	ttl = SYSINFO_TTL ;
			    if ((rs = preload_set(di,bp,rs,ttl)) >= 0) {
	    		        rc = uload_hwserial(ubuf,bp,len) ;
			    } /* end if (preload_set) */
			} /* end if (buffer_get) */
		    } /* end if (positive response) */
	    	    rs1 = buffer_finish(&b) ;
	    	    if (rs >= 0) rs = rs1 ;
		} /* end if (buffer) */
	    } else {
	  	rc = uload_hwserial(ubuf,rbuf,len) ;
	    } /* end if (cache) */
	    uc_free(rbuf) ;
	} /* end if (m-a) */

	if (rs < 0) errno = ENOMEM ; /* only real run-time error */
#if	CF_DEBUGN
	nprintf(NDF,"sysinfo_hwserial: ret rs=%d rc=%d\n",rs,rc) ;
#endif
	return (rs >= 0) ? rc : 0 ;
}
/* end subroutine (sysinfo_hwserial) */


static int sysinfo_varhwserial(BUFFER *bdp)
{
	cchar		*var = VARHWSERIAL ;
	return sysinfo_var(bdp,var) ;
}
/* end subroutine (sysinfo_varhwserial) */


static int sysinfo_varhostid(BUFFER *bdp)
{
	cchar		*var = VARHOSTID ;
	return sysinfo_var(bdp,var) ;
}
/* end subroutine (sysinfo_varhostid) */


static int sysinfo_var(BUFFER *bdp,cchar *var)
{
	int		rs = SR_OK ;
	cchar		*vp ;
	if ((vp = getenv(var)) != NULL) {
	    if (vp[0] != '\0') {
	        rs = buffer_strw(bdp,vp,-1) ;
	    }
	} /* end if (getenv) */
	return rs ;
}
/* end subroutine (sysinfo_var) */


static int sysinfo_file(BUFFER *bdp)
{
	VECSTR		env ;
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;
	if ((rs = vecstr_start(&env,0,0)) >= 0) {
	    cchar	*fname = SYSINFO_FNAME ;
	    if ((rs = vecstr_envfile(&env,fname)) >= 0) {
		cchar	*var = VARHOSTID ;
		cchar	*ep ;
		if ((rs = vecstr_finder(&env,var,vstrkeycmp,&ep)) >= 0) {
		    if (ep != NULL) {
		        cchar	*tp = strchr(ep,'=') ;
			if ((tp != NULL) && (tp[1] != '\0')) {
			    rs = buffer_strw(bdp,(tp+1),-1) ;
			    rl = rs ;
			} /* end if (have proper ENV variable) */
		    } /* end if (non-null) */
		} /* end if (vecstr_finder) */
	    } /* end if (vecstr_envfile) */
	    if (isNotPresent(rs)) rs = SR_OK ;
	    rs1 = vecstr_finish(&env) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (sysinfo_file) */


static int sysinfo_def(BUFFER *bdp)
{
	const int	name = SI_HW_SERIAL ;
	const int	dlen = DIGBUFLEN ;
	int		rs = SR_OK ;
	int		rc ;
	char		dbuf[DIGBUFLEN+1] ;
	if ((rc = sysinfo_next(name,dbuf,(dlen+1))) > 1) {
	    uint	uv ;
	    if ((rs = cfdecui(dbuf,(rc-1),&uv)) >= 0) {
		const int	hlen = HEXBUFLEN ;
		char		hbuf[HEXBUFLEN+1] ;
		if ((rs = cthexui(hbuf,hlen,uv)) >= 0) {
		    int	hl = rs ;
		    if ((rs = buffer_strw(bdp,"0x",2)) >= 0) {
		        rs = buffer_strw(bdp,hbuf,hl) ;
		    }
		} /* end if (cthexui) */
	    } /* end if (cfdecui) */
	} /* end if (sysinfo_next) */
	return rs ;
}
/* end subroutine (sysinfo_def) */


static int uload_hwserial(char *ubuf,cchar *rbuf,long len)
{
	uint	uv ;
	int	rs ;
	int	rl = 0 ;
	if ((rs = cfnumui(rbuf,-1,&uv)) >= 0) {
	    const int	dlen = DIGBUFLEN ;
	    char	dbuf[DIGBUFLEN+1] ;
	    if ((rs = ctdecui(dbuf,dlen,uv)) >= 0) {
		rl = (strlcpy(ubuf,dbuf,len)+1) ;
	    }
	} /* end if (cfnumui) */
	return (rs >= 0) ? rl : 0 ;
}
/* end subroutine (uload_hwserial) */


