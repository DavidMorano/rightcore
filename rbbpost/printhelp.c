/* printhelp */

/* print out a help file if we have one */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	The subroutine was written to get some common code for the printing of
	help files.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will search for a program helpfile and print it out (by
	default to STDOUT).  A root filename is supplied (usually 'help') but
	along with a program root.  The "standard" places within the program
	root directory tree are scanned for the help file.

	Synopsis:

	int printhelp(fp,pr,sn,hfname)
	void		*fp ;
	const char	pr[] ;
	const char	sn[] ;
	const char	hfname[] ;

	Arguments:

	fp		open file pointer (BFILE or SFIO)
	pr		program root directory path
	sn		program search name
	hfname		program help filename

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<expcook.h>
#include	<localmisc.h>


/* local defines */

#ifndef	KBUFLEN
#define	KBUFLEN		40
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(2*LINEBUFLEN)
#endif

#ifndef	HELPSCHEDFNAME
#define	HELPSCHEDFNAME	"etc/printhelp.filesched"
#endif

#ifndef	HELPFNAME
#define	HELPFNAME	"help"
#endif

#ifndef	LIBCNAME
#define	LIBCNAME	"lib"
#endif

#ifndef	NDF
#define	NDF		"printhelp.deb"
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_loadfile(VECSTR *,int,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;


/* forward references */

static int	findhelp(const char *,const char *,char *,const char *) ;
static int	loadscheds(vecstr *,const char *,const char *) ;
static int	procprint(cchar *,cchar *,void *,char *,int,cchar *) ;
static int	procexpload(const char *,EXPCOOK *,const char *) ;
static int	procout(EXPCOOK *,void *,char *,int,const char *) ;


/* local variables */

static const char	*schedule[] = {
	"%r/%l/%n/%n.%f",
	"%r/%l/%n/%f",
	"%r/share/help/%n.%f",
	"%r/share/help/%n",
	"/usr/extra/share/help/%n.%f",
	"/usr/extra/share/help/%n",
	"%r/%l/help/%n.%f",
	"%r/%l/help/%n",
	"%r/%l/%n.%f",
	"%r/%n.%f",
	"%n.%f",
	NULL
} ;

static const char	*expkeys[] = {
	"SN",
	"SS",
	"PR",
	"RN",
	NULL
} ;

enum expkeys {
	expkey_sn,
	expkey_ss,
	expkey_pr,
	expkey_rn,
	expkey_overlast
} ;


/* exported subroutines */


int printhelp(void *fp,cchar *pr,cchar *sn,cchar *hfname)
{
	const int	tlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	const char	*fname ;
	char		tbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("printhelp: SFIO=%u\n",CF_SFIO) ;
	debugprintf("printhelp: fp=%p\n",fp) ;
#endif

	if ((hfname == NULL) || (hfname[0] == '\0'))
	    hfname = HELPFNAME ;

#if	CF_SFIO
	if (fp == NULL) {
	    rs = SR_INVALID ;
#if	CF_DEBUGS
	    debugprintf("printhelp: no output file handle (SFIO mode)\n") ;
#endif
	}
#endif /* CF_SFIO */

#if	CF_DEBUGS
	debugprintf("printhelp: cont rs=%d\n",rs) ;
	debugprintf("printhelp: pr=%s\n",pr) ;
	debugprintf("printhelp: sn=%s\n",sn) ;
	debugprintf("printhelp: hfname=%s\n",hfname) ;
#endif

	if (rs >= 0) {

	    fname = hfname ;
	    if ((pr != NULL) && (hfname[0] != '/')) {

#if	CF_DEBUGS
	        debugprintf("printhelp: partial fname=%s\n",hfname) ;
#endif

	        rs = SR_NOTFOUND ;
	        if (strchr(hfname,'/') != NULL) {
	            fname = tbuf ;
	            if ((rs = mkpath2(tbuf,pr,hfname)) >= 0)
	                rs = u_access(tbuf,R_OK) ;

#if	CF_DEBUGS
	            debugprintf("printhelp: partial rs=%d hfname=%s\n",
	                rs,tbuf) ;
#endif

	        } /* end if */

	        if ((rs < 0) && isNotPresent(rs)) {
	            fname = tbuf ;
	            rs = findhelp(pr,sn,tbuf,hfname) ;
	        }

	    } /* end if (searching for file) */

#if	CF_DEBUGS
	    debugprintf("printhelp: mid rs=%d fname=%s\n",rs,fname) ;
#endif

	    if (rs >= 0) {
	        rs = procprint(pr,sn,fp,tbuf,tlen,fname) ;
	    }

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("printhelp: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (printhelp) */


/* local subroutines */


static int findhelp(cchar *pr,cchar *sn,char *tbuf,cchar *hfname)
{
	vecstr		hs ;
	const int	tlen = MAXPATHLEN ;
	int		rs ;
	int		f_hs = FALSE ;
	const char	**spp = schedule ;

#if	CF_DEBUGS
	debugprintf("printhelp/findhelp: hfname=%s\n",hfname) ;
#endif

/* first see if there is a "help schedule" in the ETC directory */

	if ((rs = mkpath2(tbuf,pr,HELPSCHEDFNAME)) >= 0) {

	    if ((rs = perm(tbuf,-1,-1,NULL,R_OK)) >= 0) {
		const int	opts = VECSTR_OCOMPACT ;

	        if ((rs = vecstr_start(&hs,15,opts)) >= 0) {
	            f_hs = TRUE ;
	            if ((rs = vecstr_loadfile(&hs,FALSE,tbuf)) >= 0) {
	                vecstr_getvec(&hs,&spp) ;
		    } else if (isNotPresent(rs)) {
			rs = SR_OK ;
		    }
	        }

	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }

	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("printhelp/findhelp: mid ret rs=%d\n",rs) ;
#endif

/* create the values for the file schedule searching and find the file */

	if (rs >= 0) {
	    vecstr	svars ;
	    if ((rs = vecstr_start(&svars,6,0)) >= 0) {

	        rs = loadscheds(&svars,pr,sn) ;

/* OK, do the look-up */

	        if (rs >= 0) {
	            rs = permsched(spp,&svars,tbuf,tlen,hfname,R_OK) ;
		}

	        if (isNotPresent(rs) && (spp != schedule)) {
	            rs = permsched(schedule,&svars,tbuf,tlen,hfname,R_OK) ;
		}

#if	CF_DEBUGS
	        debugprintf("printhelp/findhelp: permsched() rs=%d\n",rs) ;
	        debugprintf("printhelp/findhelp: tbuf=%s\n",tbuf) ;
#endif

	        vecstr_finish(&svars) ;
	    } /* end if (schedule variables) */
	} /* end if (ok) */

	if (f_hs)
	    vecstr_finish(&hs) ;

#if	CF_DEBUGS
	debugprintf("printhelp/findhelp: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (findhelp) */


static int loadscheds(vecstr *slp,const char *pr,const char *sn)
{
	int		rs = SR_OK ;

	if (pr != NULL) rs = vecstr_envadd(slp,"r",pr,-1) ;

	if (rs >= 0) rs = vecstr_envadd(slp,"l",LIBCNAME,-1) ;

	if ((rs >= 0) && (sn != NULL)) {
	    rs = vecstr_envadd(slp,"n",sn,-1) ;
	}

	return rs ;
}
/* end subroutine (loadscheds) */


static int procprint(cchar *pr,cchar *sn,void *fp,char *rp,int rl,cchar *fn)
{
	EXPCOOK		c ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = expcook_start(&c)) >= 0) {
	    if ((rs = procexpload(pr,&c,sn)) >= 0) {

	        rs = procout(&c,fp,rp,rl,fn) ;
	        wlen += rs ;

	    } /* end if (procexpload) */
	    rs1 = expcook_finish(&c) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (expcook) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprint) */


static int procexpload(cchar *pr,EXPCOOK *ecp,cchar *sn)
{
	int		rs ;
	char		nn[NODENAMELEN+1] ;
	char		dn[MAXHOSTNAMELEN+1] ;

	if ((rs = getnodedomain(nn,dn)) >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		i ;
	    int		vl ;
	    const char	*vp ;
	    const char	*ks = "SNDHPR" ;
	    char	hbuf[MAXHOSTNAMELEN + 1] ;
	    char	kbuf[KBUFLEN+1] ;
	    kbuf[1] = '\0' ;
	    for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	        int	kch = MKCHAR(ks[i]) ;
		vl = -1 ;
		vp = NULL ;
		switch (kch) {
	        case 'S':
		    vp = sn ;
		    break ;
	        case 'N':
		    vp = nn ;
		    break ;
	        case 'D':
		    vp = dn ;
		    break ;
	        case 'H':
		    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
			vl = rs ;
		        vp = hbuf ;
		    }
		    break ;
	        case 'P':
		    vp = pr ;
		    break ;
	        case 'R':
		    vl = sfbasename(pr,-1,&vp) ;
		    break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
		    kbuf[0] = kch ;
		    rs = expcook_add(ecp,kbuf,vp,vl) ;
	        }
	    } /* end for */
	    if (rs >= 0) {
		for (i = 0 ; (rs >= 0) && (i < expkey_overlast) ; i += 1) {
		    vl = -1 ;
		    vp = NULL ;
		    switch (i) {
		    case expkey_sn:
			vp = sn ;
			break ;
		    case expkey_ss:
			vp = hbuf ;
			vl = strwcpyuc(hbuf,sn,-1) - hbuf ;
			break ;
		    case expkey_pr:
			vp = pr ;
			break ;
		    case expkey_rn:
		        vl = sfbasename(pr,-1,&vp) ;
			break ;
		    } /* end switch */
	            if ((rs >= 0) && (vp != NULL)) {
		        rs = expcook_add(ecp,expkeys[i],vp,vl) ;
	            }
		} /* end for */
	    } /* end if (ok) */
	} /* end if (getnodedomain) */

	return rs ;
}
/* end subroutine (procexpload) */


static int procout(EXPCOOK *ecp,void *fp,char lbuf[],int llen,cchar *fname)
{
#if	CF_SFIO
#else
	bfile		outfile ;
#endif /* CF_SFIO */
	bfile		helpfile, *hfp = &helpfile ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_SFIO
#else
	int		f_open = FALSE ;
#endif /* CF_SFIO */

	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SFIO
	if (fp == NULL) return SR_FAULT ;
#else /* CF_SFIO */
	if (fp == NULL) {
	    fp = &outfile ;
	    rs = bopen(fp,BFILE_STDOUT,"w",0666) ;
	    f_open = (rs >= 0) ;
	}
#endif /* CF_SFIO */

	if ((rs >= 0) && ((rs = bopen(hfp,fname,"r",0666)) >= 0)) {
	    const int	elen = EBUFLEN ;
	    char	*ebuf ;
	    if ((rs = uc_malloc((elen+1),&ebuf)) >= 0) {
	        int	len ;

	        while ((rs = breadline(hfp,lbuf,llen)) > 0) {
	            len = rs ;

		    if ((rs = expcook_exp(ecp,0,ebuf,elen,lbuf,len)) > 0) {
#if	CF_SFIO
	                rs = sfwrite(fp,ebuf,rs) ;
	                wlen += rs ;
#else /* CF_SFIO */
	                rs = bwrite(fp,ebuf,rs) ;
	                wlen += rs ;
#endif /* CF_SFIO */
		    } /* end if (expansion) */

#if	CF_DEBUGS
	            debugprintf("printhelp: sfwrite() rs=%d\n",rs) ;
#endif

	            if (rs < 0) break ;
	        } /* end while */

		uc_free(ebuf) ;
	    } /* end if (memory-allocation) */
	    bclose(hfp) ;
	} /* end if (opened helpfile) */

#if	CF_SFIO
	sfsync(fp) ;
#else
	if (f_open)
	    bclose(fp) ;
#endif /* CF_SFIO */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


