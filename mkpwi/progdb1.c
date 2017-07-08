/* progdb */

/* process the system password GECOS fields and create associated DB */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGFILE	0		/* debug index file */
#define	CF_MKGECOSNAME	1		/* "should" be much faster? */


/* revision history:

	= 2001-09-01, David A­D­ Morano
	This subroutine was hacked from something else.  The other thing
	appears to have been a PCS cache file creator, but I'm not even sure
	now (and I even wrote the other thing also!).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads the system PASSWD database and creates an inverse
	password database file (ipasswd) that maps real name to usernames.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<getbufsize.h>
#include	<bfile.h>
#include	<pwfile.h>
#include	<setstr.h>
#include	<getax.h>
#include	<gecos.h>
#include	<realname.h>
#include	<strtab.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"

#include	"recorder.h"
#include	"ipasswd.h"


/* local defines */

#define	NREC_GUESS	100		/* guess of number of records */

#define	PWIFILE_MAGIC		"IPASSWD"
#define	PWIFILE_MAGICLEN	7
#define	PWIFILE_VERSION		0
#define	PWIFILE_TYPE		0

#ifndef	ITEMLEN
#define	ITEMLEN		100
#endif

#undef	USERNAMELEN
#ifdef	IPASSWD_USERNAME
#define	USERNAMELEN	IPASSWD_USERNAMELEN
#else
#define	USERNAMELEN	32
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;
extern int	snrealname(char *,int,cchar **,int) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	snwcpyhyphen(char *,int,cchar *,int) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	mkgecosname(char *,int,cchar *) ;
extern int	getgecosname(cchar *,int,cchar **) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	iceil(int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */

enum headers {
	header_writetime,
	header_writecount,
	header_rectab,
	header_rectablen,
	header_strtab,
	header_strtabsize,
	header_indexlen,
	header_indexl1,
	header_indexl3,
	header_indexf,
	header_indexfl3,
	header_indexun,
	header_overlast
} ;


/* forward references */

static int	progdber(PROGINFO *,bfile *,cchar *,int,int) ;
static int	procpwfile(PROGINFO *,STRTAB *,RECORDER *) ;
static int	procpwsys(PROGINFO *,STRTAB *,RECORDER *) ;

static int	procentry(PROGINFO *,STRTAB *,RECORDER *,cchar *,cchar *,int) ;
static int	wrcache(PROGINFO *,STRTAB *,RECORDER *,cchar *,int,time_t) ;
static int	wrstats(PROGINFO *,bfile *,RECORDER *) ;
static int	mkro(PROGINFO *) ;
static int	ensureperms(int,cchar *) ;


/* local variables */


/* exported subroutines */


int progdb(PROGINFO *pip,bfile *ofp,cchar *dbname)
{
	int		rs = SR_OK ;
	int		dnl ;
	int		n = NREC_GUESS ;
	int		c = 0 ;
	cchar		*dnp ;

	if (dbname == NULL) return SR_FAULT ;

/* check that the parent directory is writable by us */

	if ((dnl = sfdirname(dbname,-1,&dnp)) > 0) {
	    char	fname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath1w(fname,dnp,dnl)) >= 0) {
		const int	am = (X_OK|W_OK) ;
		if ((rs = perm(fname,-1,-1,NULL,am)) >= 0) {
		    if ((rs = mkfnamesuf1(fname,dbname,DBSUF)) >= 0) {
			if ((rs = mkro(pip)) >= 0) {
			    const int	ro = rs ;
	    		    rs = progdber(pip,ofp,fname,ro,n) ;
			    c = rs ;
			} /* end if (mkro) */
		    }
		}
	    }
	} /* end if (sfdirname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progdb: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progdb) */


/* local subroutines */


static int progdber(PROGINFO *pip,bfile *ofp,cchar *fname,int ro,int n)
{
	STRTAB		st ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	    if ((rs = strtab_start(&st,(n * 25))) >= 0) {
	        RECORDER	rec ;
	        if ((rs = recorder_start(&rec,n,ro)) >= 0) {

	            if ((pip->pwfname != NULL) && (pip->pwfname[0] != '-')) {
	                rs = procpwfile(pip,&st,&rec) ;
	                c = rs ;
	            } else {
	                rs = procpwsys(pip,&st,&rec) ;
	                c = rs ;
	            }

/* where are we? */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progdb: mid rs=%d nrecs=%u\n",
	                    rs,c) ;
#endif /* CF_DEBUG */

	            if (rs >= 0) {
	                if ((rs = recorder_count(&rec)) >= 0) {
	                    const time_t	dt = pip->daytime ;
			    cchar		*fn = fname ;
	                    if ((rs = wrcache(pip,&st,&rec,fn,ro,dt)) >= 0) {
	                        if (pip->verboselevel > 1) {
				    if (pip->open.outfile) {
	                                rs = wrstats(pip,ofp,&rec) ;
				    }
	                        }

#if	CF_DEBUG && CF_DEBUGFILE
	                        if (DEBUGLEVEL(4)) {
	                            IPASSWD	pwi ;
	                            IPASSWD_CUR	cur ;
	                            const int	rlen = REALNAMELEN ;
	                            const int	nlen = REALNAMELEN ;
	                            int		rs1 ;
	                            cchar	*sa[6] ;
	                            char	ubuf[IPASSWD_USERNAMELEN + 1] ;
	                            char	rbuf[REALNAMELEN+1] ;
	                            char	nbuf[REALNAMELEN+1] ;

	                            rs1 = ipasswd_open(&pwi,fname) ;
	                            debugprintf("progdb: "
					"ipasswd_open() rs=%d\n", rs1) ;
	                        if ((rs = ipasswd_curbegin(&pwi,&cur)) >= 0) {
	                                while (TRUE) {
	                                    rs1 = ipasswd_enum(&pwi,&cur,
	                                        ubuf,sa,rbuf,rlen) ;
	                                    debugprintf("progdb: "
						"ipasswd_enum() rs=%d\n",rs1) ;
	                                    if (rs1 < 0) break ;
	                                    debugprintf("progdb: username=%s\n",
	                                        ubuf) ;
	                                    snrealname(nbuf,nlen,sa,-1) ;
	                                    debugprintf("main: name=%s\n",
	                                        nbuf) ;
	                                }
	                                ipasswd_curend(&pwi,&cur) ;
	                            } /* end if (enumerating) */
	                            ipasswd_close(&pwi) ;
	                        }
#endif /* CF_DEBUGFILE */
	                    } /* end if (wrstats) */
	                } /* end if (recorder_counts) */
	            } /* end if (ok) */

	            rs1 = recorder_finish(&rec) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (recorder) */
	        rs1 = strtab_finish(&st) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (strtab) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progdber) */


static int procpwfile(PROGINFO *pip,STRTAB *stp,RECORDER *rtp)
{
	PWFILE		pf ;
	PWFILE_CUR	cur ;
	PWFILE_ENT	pw ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = pwfile_open(&pf,pip->pwfname)) >= 0) {
	    if ((rs = pwfile_curbegin(&pf,&cur)) >= 0) {
	        const int	pwlen = getbufsize(getbufsize_pw) ;
	        int		nl ;
	        cchar		*np ;
	        char		*pwbuf ;
	        if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	            while ((rs = pwfile_enum(&pf,&cur,&pw,pwbuf,pwlen)) > 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progdb: username=%s\n",pw.username) ;
#endif

#if	CF_MKGECOSNAME
	                {
	                    if ((nl = getgecosname(pw.username,-1,&np)) > 0) {
	                        char	nbuf[nl + 1] ;

	                        if (strnchr(np,nl,'-') != NULL) {
	                            rs = snwcpyhyphen(nbuf,-1,np,nl) ;
	                            np = nbuf ;
	                        }

	                        if (rs >= 0) {
	                            cchar	*un = pw.username ;
	                            c += 1 ;
	                            rs = procentry(pip,stp,rtp,un,np,nl) ;
	                        }

	                    } /* end if */
	                } /* end block */
#else /* CF_MKGECOSNAME */
	                {
	                    GECOS	ge ;
	                    if ((rs = gecos_start(&ge,pw.gecos,-1)) >= 0) {
	                        const int	req = gecosval_realname ;
	                        if ((nl = gecos_getval(&ge,req,&np)) > 0) {
	                            char	nbuf[nl + 1] ;

	                            if (strnchr(np,nl,'-') != NULL) {
	                                rs = snwcpyhyphen(nbuf,-1,np,nl) ;
	                                np = nbuf ;
	                            }

	                            if (rs >= 0) {
	                                cchar	*un = pw.username ;
	                                c += 1 ;
	                                rs = procentry(pip,stp,rtp,un,np,nl) ;
	                            }

	                        } /* end block */
	                        gecos_finish(&ge) ;
	                    } /* end if (gecos) */
	                } /* end block */
#endif /* CF_MKGECOSNAME */

#if	CF_DEBUG
			if (DEBUGLEVEL(4))
			debugprintf("progdb/procpwfile: while-bot rs=%d\n",
				rs) ;
#endif

	                if (rs < 0) break ;
	            } /* end while (looping through entries) */
	            uc_free(pwbuf) ;
	        } /* end if (m-a) */
	        rs1 = pwfile_curend(&pf,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	    rs1 = pwfile_close(&pf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pwfile) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progdb/procpwfile: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpwfile) */


static int procpwsys(PROGINFO *pip,STRTAB *stp,RECORDER *rtp)
{
	SETSTR		u ;
	const int	n = 100 ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = setstr_start(&u,n)) >= 0) {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
		if ((rs = getpw_begin()) >= 0) {
	            while ((rs = getpw_ent(&pw,pwbuf,pwlen)) > 0) {
		        cchar	*un = pw.pw_name ;
		        if ((rs = setstr_add(&u,un,-1)) > 0) {
	    	            GECOS	ge ;
		            cchar	*gecos = pw.pw_gecos ;
	                    if ((rs = gecos_start(&ge,gecos,-1)) >= 0) {
	                        const int	req = gecosval_realname ;
	        	        int		nl ;
	        	        cchar		*np ;
	                        if ((nl = gecos_getval(&ge,req,&np)) > 0) {
	                            char	nbuf[nl+1] ;

	                    	    if (strnchr(np,nl,'-') != NULL) {
					rs = snwcpyhyphen(nbuf,-1,np,nl) ;
	                        	np = nbuf ;
	                    	    }

	                            if (rs >= 0) {
	                                c += 1 ;
	                                rs = procentry(pip,stp,rtp,un,np,nl) ;
	                            }

#if	CF_DEBUG
				if (DEBUGLEVEL(4))
				debugprintf("progdb/procpwsys: "
					"procentry() rs=%d\n",rs) ;
#endif

	                	} /* end if (gecos_getval) */
#if	CF_DEBUG
				if (DEBUGLEVEL(4))
				debugprintf("progdb/procpwsys: "
				   "gecos_getval-out rs=%d\n",rs) ;
#endif
	                        rs1 = gecos_finish(&ge) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (gecos) */
		        } /* end if (setstr_add) */
#if	CF_DEBUG
			if (DEBUGLEVEL(4))
			debugprintf("progdb/procpwsys: gecos-out rs=%d\n",rs) ;
#endif
	                if (rs < 0) break ;
	            } /* end while (looping through entries) */
		    rs1 = getpw_end() ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (getpw) */
	        uc_free(pwbuf) ;
	    } /* end if (m-a-f) */
	    rs1 = setstr_finish(&u) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (setstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progdb/procpwsys: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpwsys) */


static int procentry(pip,stp,rtp,username,nbuf,nlen)
PROGINFO	*pip ;
STRTAB		*stp ;
RECORDER	*rtp ;
cchar		username[] ;
cchar		nbuf[] ;
int		nlen ;
{
	REALNAME	rn ;
	int		rs ;

	if (username == NULL) return SR_FAULT ;
	if (nbuf == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procentry: username=%s\n",username) ;
	    debugprintf("procentry: rname=>%t<\n",nbuf,nlen) ;
	}
#endif

	if ((rs = realname_start(&rn,nbuf,nlen)) >= 0) {
	    RECORDER_ENT	re ;
	    int			len ;
	    int			i_username ;
	    int			i_last, i_first, i_m1, i_m2 ;
	    char		buf[ITEMLEN + 1], *bp ;

/* username */

	    len = strnlen(username,USERNAMELEN) ;

	    i_username = strtab_already(stp,username,len) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procentry: strtab_already() i_username=%d\n",
	            i_username) ;
#endif

	    if (i_username < 0) {
	        rs = strtab_add(stp,username,len) ;
	        i_username = rs ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procentry: strtab_add() rs=%d i_username=%d\n",
	                rs,i_username) ;
#endif
	    }

/* last */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procentry: last=%s\n",rn.last) ;
#endif

	    i_last = 0 ;
	    if ((rs >= 0) && (rn.last != NULL)) {

	        bp = strwcpylc(buf,rn.last,ITEMLEN) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procentry: lowered last=%s\n",buf) ;
#endif

	        i_last = strtab_already(stp,buf,(bp - buf)) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procentry: strtab_already() i_last=%d\n",
	                i_last) ;
#endif

	        if (i_last < 0) {
	            rs = strtab_add(stp,buf,(bp - buf)) ;
	            i_last = rs ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procentry: strtab_add() rs=%d i_last=%d\n",
	                    rs,i_last) ;
#endif
	        }
	    }

/* first */

	    i_first = 0 ;
	    if ((rs >= 0) && (rn.first != NULL)) {
	        bp = strwcpylc(buf,rn.first,ITEMLEN) ;
	        if ((i_first = strtab_already(stp,buf,(bp - buf))) < 0) {
	            rs = strtab_add(stp,buf,(bp - buf)) ;
	            i_first = rs ;
	        }
	    }

/* middle-1 */

	    i_m1 = 0 ;
	    if ((rs >= 0) && (rn.m1 != NULL)) {
	        bp = strwcpylc(buf,rn.m1,ITEMLEN) ;
	        if ((i_m1 = strtab_already(stp,buf,(bp - buf))) < 0) {
	            rs = strtab_add(stp,buf,(bp - buf)) ;
	            i_m1 = rs ;
	        }
	    }

/* middle-2 */

	    i_m2 = 0 ;
	    if ((rs >= 0) && (rn.m2 != NULL)) {
	        bp = strwcpylc(buf,rn.m2,ITEMLEN) ;
	        if ((i_m2 = strtab_already(stp,buf,(bp - buf))) < 0) {
	            rs = strtab_add(stp,buf,(bp - buf)) ;
	            i_m2 = rs ;
	        }
	    }

/* put the record together */

	    if (rs >= 0) {
	        re.username = i_username ;
	        re.last = i_last ;
	        re.first = i_first ;
	        re.m1 = i_m1 ;
	        re.m2 = i_m2 ;
	        rs = recorder_add(rtp,&re) ;
	    } /* end if (adding record to DB) */

	    realname_finish(&rn) ;
	} /* end if (realname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progdb/procentry: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procentry) */


/* write out the cache file */
static int wrcache(pip,stp,rp,fname,ropts,daytime)
PROGINFO	*pip ;
STRTAB		*stp ;
RECORDER	*rp ;
cchar		fname[] ;
int		ropts ;
time_t		daytime ;
{
	RECORDER_ENT	*rectab ;
	uint		(*recind)[2] ;
	uint		header[header_overlast] ;
	uint		fto ;
	int		rs ;
	int		rs1 ;
	int		size ;
	int		rtsize, risize ;
	int		stsize ;
	int		rtlen, rilen ;
	int		oflags ;
	int		fd_pwi ;

	char		dbfname[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		vetu[16] ;
	char		*stab = NULL ;
	char		*bp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: ent fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;

	rs = snsd(dbfname,MAXPATHLEN,fname,ENDIAN) ;
	if (rs < 0)
	    goto ret0 ;

	recind = NULL ;
	rs = sncpy2(tmpfname,MAXPATHLEN,dbfname,"n") ;
	if (rs < 0)
	    goto ret0 ;

	oflags = (O_RDWR | O_CREAT | O_EXCL) ;
	rs = uc_open(tmpfname,oflags,0664) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: uc_open() rs=%d\n",rs) ;
#endif

	fd_pwi = rs ;
	if (rs < 0) {
	    if (rs == SR_EXIST) {
	        tmpfname[0] = '\0' ;
	        rs = SR_OK ;
	    }
	    goto ret0 ;
	}

/* ensure that the file has the correct permissions and group ownership */

	ensureperms(fd_pwi,tmpfname) ;

/* prepare and write the file magic */

	bp = strwcpy(vetu,PWIFILE_MAGIC,15) ;
	*bp++ = '\n' ;
	memset(bp,0,(vetu + 16) - bp) ;

	rs = u_write(fd_pwi,vetu,16) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: wrote magic rs=%d\n",rs) ;
#endif

	fto = 16 ;

/* prepare and write the version and encoding (VETU) */

	vetu[0] = PWIFILE_VERSION ;
	vetu[1] = ENDIAN ;
	vetu[2] = ropts ;
	vetu[3] = 0 ;

	if (rs >= 0)
	    rs = u_write(fd_pwi,vetu,4) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: wrote version-encoding rs=%d\n",
	        rs) ;
#endif

	fto += 4 ;

/* get the tables and write them out (along with table headers) */

/* handle the record table */

	if (rs >= 0)
	    rs = recorder_gettab(rp,&rectab) ;

	if (rs < 0)
	    goto ret1 ;

	rtsize = iceil(rs,sizeof(int)) ;

	rs = recorder_rtlen(rp) ;
	rtlen = rs ;
	if (rs < 0)
	    goto ret1 ;

/* handle the string table */

	rs = strtab_strsize(stp) ;
	stsize = rs ;
	if (rs >= 0)
	    rs = uc_malloc(stsize,&stab) ;

	if (rs < 0)
	    goto ret1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: stab=%p stsize=%u\n",
	        stab,stsize) ;
#endif

	rs = strtab_strmk(stp,stab,stsize) ;
	if (rs < 0)
	    goto ret2 ;

/* get the index size and length */

	rs = recorder_indsize(rp) ;
	risize = rs ;
	if (rs < 0)
	    goto ret2 ;

	rs = recorder_indlen(rp) ;
	rilen = rs ;
	if (rs < 0)
	    goto ret2 ;

/* OK, we're ready to start writing stuff out! */

/* header itself */

	fto += (header_overlast * sizeof(uint)) ;

/* everything else */

	header[header_writetime] = (uint) daytime ;
	header[header_writecount] = 0 ;

	header[header_rectab] = fto ;
	fto += rtsize ;

	header[header_rectablen] = rtlen ;

	header[header_strtab] = fto ;
	fto += stsize ;			/* increment by its size not length */

	header[header_strtabsize] = stsize ;
	header[header_indexlen] = rilen ;

	header[header_indexl1] = fto ;
	fto += risize ;

	header[header_indexl3] = fto ;
	fto += risize ;

	header[header_indexf] = fto ;
	fto += risize ;

	header[header_indexfl3] = fto ;
	fto += risize ;

	header[header_indexun] = fto ;
	fto += risize ;

/* write everything out */

/* header */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    int	i ;
	    for (i = 0 ; i < header_overlast ; i += 1)
	        debugprintf("wrcache: header[%d]=%08x\n",i,header[i]) ;
	}
#endif /* CF_DEBUG */

	size = header_overlast * sizeof(int) ;
	rs = u_write(fd_pwi,header,size) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: u_write() header rs=%d\n",
	        rs) ;
#endif

	if (rs < 0)
	    goto ret2 ;

/* record table */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: u_write() record-table\n") ;
#endif

	rs = u_write(fd_pwi,rectab,rtsize) ;
	if (rs < 0)
	    goto ret2 ;

/* string table */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: u_write() string-table\n") ;
#endif

	rs = u_write(fd_pwi,stab,stsize) ;
	if (rs < 0)
	    goto ret2 ;

/* get the four indices that we want */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: uc_malloc() size=%u\n",risize) ;
#endif

	rs = uc_malloc(risize,&recind) ;
	if (rs < 0)
	    goto ret2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("wrcache: clearing memory\n") ;
	    memset(recind,0,risize) ;
	}
#endif

/* index L1 */

	rs = recorder_mkindl1(rp,stab,recind,risize) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: recorder_mkind() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret3 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: u_write() record-index L1\n") ;
#endif

	rs = u_write(fd_pwi,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

/* index L3 */

	rs = recorder_mkindl3(rp,stab,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: u_write() record-index L3\n") ;
#endif

	rs = u_write(fd_pwi,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

/* index F */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: u_write() record-index F\n") ;
#endif

	rs = recorder_mkindf(rp,stab,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

	rs = u_write(fd_pwi,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

/* index FL3 */

	rs = recorder_mkindfl3(rp,stab,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: u_write() record-index FL3\n") ;
#endif

	rs = u_write(fd_pwi,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

/* index UN */

	rs = recorder_mkindun(rp,stab,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: u_write() record-index UN\n") ;
#endif

	rs = u_write(fd_pwi,recind,risize) ;
	if (rs < 0)
	    goto ret3 ;

/* we're out of here */
ret3:
	if (recind != NULL)
	    uc_free(recind) ;

ret2:
	if (stab != NULL)
	    uc_free(stab) ;

ret1:
	if (rs >= 0) {
	    if (pip->egid != pip->gid_tools) {
	        rs = u_fchown(fd_pwi,-1,pip->gid_tools) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("wrcache: u_fchown() rs=%d\n",rs) ;
#endif
	    }
	}

	u_close(fd_pwi) ;

	if (rs >= 0) {
	    rs = u_rename(tmpfname,dbfname) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("wrcache: u_rename() rs=%d\n",rs) ;
#endif
	} /* end if (rename) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("wrcache: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (wrcache) */


static int wrstats(PROGINFO *pip,bfile *ofp,RECORDER *recp)
{
	RECORDER_INFO	s ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
	if ((rs = recorder_info(recp,&s)) >= 0) {
	    uint	sum ;
	    int		i, j ;

	    bprintf(ofp,"       records %6u\n",rs) ;
	    bprintf(ofp,"  index length %6u\n",s.ilen) ;
	    bprintf(ofp,"L1  collisions %6u\n",s.c_l1) ;
	    bprintf(ofp,"L3  collisions %6u\n",s.c_l3) ;
	    bprintf(ofp,"F   collisions %6u\n",s.c_f) ;
	    bprintf(ofp,"FL3 collisions %6u\n",s.c_fl3) ;
	    bprintf(ofp,"UN  collisions %6u\n",s.c_un) ;

	    sum = s.c_l1 + s.c_l3 + s.c_f + s.c_fl3 + s.c_un ;
	    bprintf(ofp,"T   collisions %6u\n",sum) ;

	    bprintf(ofp,"index\t %5s %5s %5s %5s %5s\n",
	        "l1","l3","f","fl3","un") ;

	    for (j = 0 ; j < RECORDER_NCOLLISIONS ; j += 1) {
	        bprintf(ofp,"\t") ;
	        for (i = 0 ; i < 5 ; i += 1) {
	            bprintf(ofp," %5u",s.cden[i][j]) ;
	        } /* end for */
	        bprintf(ofp,"\n") ;
	    } /* end for */

	} /* end if */

	return rs ;
}
/* end subroutine (wrstats) */


static int mkro(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		ro = (RECORDER_OSEC | RECORDER_ORANDLC) ;
	if ((pip->typespec != NULL) && (pip->typespec[0] != '\0')) {
	    int	ft = 0 ;
	    if ((rs = cfdeci(pip->typespec,-1,&ft)) >= 0) {
	        if (ft <= 2) {
	            switch (ft) {
	            case 1:
	                ro |= RECORDER_OSEC ;
	                break ;
	            case 2:
	                ro |= (RECORDER_OSEC | RECORDER_ORANDLC) ;
	                break ;
	            } /* end switch */
		} else {
	            rs = SR_INVALID ;
		}
	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if (file options) */
	return (rs >= 0) ? ro : rs ;
}
/* end subroutine (mkro) */


static int ensureperms(int fd,cchar *fname)
{
	int		rs = SR_OK ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if (fd >= 0) {
	    struct ustat	fsb ;
	    int			dnl ;
	    cchar		*dnp ;
	    if ((rs = u_fstat(fd,&fsb)) >= 0) {
		mode_t fm = (S_IREAD | S_IWRITE | S_IRGRP | S_IWGRP | S_IROTH) ;
		if ((fsb.st_mode & fm) != fm) {
	    	    rs = u_fchmod(fd,(fsb.st_mode | fm)) ;
		}
		if (rs >= 0) {
	    	    char	dname[MAXPATHLEN + 1] ;
	    	    if ((dnl = sfdirname(fname,-1,&dnp)) > 0) {
	    	        struct ustat	dsb ;
			snwcpy(dname,MAXPATHLEN,dnp,dnl) ;
	        	if ((rs = u_stat(dname,&dsb)) >= 0) {
	            	    if (fsb.st_gid != dsb.st_gid) {
	                	rs = u_fchown(fd,-1,dsb.st_gid) ;
		    	    }
			}
	    	    }
		} /* end if (ok) */
	    } /* end if (u_fstat) */
	} else {
	    rs = SR_BADF ;
	}

	return rs ;
}
/* end subroutine (ensureperms) */


