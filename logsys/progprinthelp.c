/* progprinthelp */

/* print out a help file if we have one */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1997-11-01, David A­D­ Morano
        The subroutine was written to get some common code for the printing of
        help files.

*/

/* Copyright © 1997 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will search for a program helpfile and print it out (by
        default to STDOUT). A root filename is supplied (usually 'help') but
        along with a program root. The "standard" places within the program root
        directory tree are scanned for the help file.

	Synopsis:

	int progprinthelp(pip,fp,helpfname)
	PROGINFO	*pip ;
	void		*fp ;
	const char	helpfname[] ;

	Arguments:

	pip		program-information-pointer
	fp		open file pointer (BFILE or SFIO)
	helpfname	program help filename

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */

#undef	COMMENT

#ifndef	HELPSCHEDFNAME
#define	HELPSCHEDFNAME	"etc/progprinthelp.filesched"
#endif

#ifndef	HELPFNAME
#define	HELPFNAME	"help"
#endif

#ifndef	LIBCNAME
#define	LIBCNAME	"lib"
#endif

#ifndef	DEBUGFNAME
#define	DEBUGFNAME	"progprinthelp.deb"
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_loadfile(VECSTR *,int,const char *) ;
extern int	isNotPresent(int) ;


/* forward references */

static int	findhelpfile(const char *,const char *,char *,const char *) ;
static int	loadscheds(vecstr *,const char *,const char *) ;
static int	printout(void *,char *,int,const char *) ;


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


/* exported subroutines */


int progprinthelp(pip,fp,helpfname)
PROGINFO	*pip ;
void		*fp ;
const char	helpfname[] ;
{
	int		rs = SR_OK ;
	const char	*fname ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("progprinthelp: SFIO=%u\n",CF_SFIO) ;
#endif

	if ((helpfname == NULL) || (helpfname[0] == '\0'))
	    helpfname = HELPFNAME ;

#if	CF_SFIO
	if (fp == NULL) {

#if	CF_DEBUGS
	    debugprintf("progprinthelp: no output file handle (SFIO mode)\n") ;
#endif

	    rs = SR_INVALID ;
	    goto ret0 ;
	}
#endif /* CF_SFIO */

#if	CF_DEBUGS
	debugprintf("progprinthelp: pr=%s\n",pr) ;
	debugprintf("progprinthelp: sn=%s\n",pip->searchname) ;
	debugprintf("progprinthelp: helpfname=%s\n",helpfname) ;
#endif

	fname = helpfname ;
	rs = SR_NOTFOUND ;
	if ((pip->pr != NULL) && (helpfname[0] != '/')) {

#if	CF_DEBUGS
	    debugprintf("progprinthelp: partial fname=%s\n",helpfname) ;
#endif

	    if (strchr(helpfname,'/') != NULL) {
	        fname = tmpfname ;
	        rs = mkpath2(tmpfname,pip->pr,helpfname) ;
	        if (rs >= 0)
	            rs = u_access(tmpfname,R_OK) ;

#if	CF_DEBUGS
	        debugprintf("progprinthelp: partial rs=%d helpfname=%s\n",
	            rs,tmpfname) ;
#endif

	    } /* end if */

	    if ((rs < 0) && isNotPresent(rs)) {
	        fname = tmpfname ;
	        rs = findhelpfile(pip->pr,pip->searchname,tmpfname,helpfname) ;
	    }

	} /* end if (searching for file) */

#if	CF_DEBUGS
	debugprintf("progprinthelp: attempt_open rs=%d fname=%s\n",rs,fname) ;
#endif

	if (rs >= 0)
	    rs = printout(fp,tmpfname,MAXPATHLEN,fname) ;

ret0:

#if	CF_DEBUGS
	debugprintf("progprinthelp: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progprinthelp) */


/* local subroutines */


static int findhelpfile(pr,sn,tmpfname,helpfname)
const char	*pr ;
const char	*sn ;
char		tmpfname[] ;
const char	*helpfname ;
{
	vecstr		svars ;
	vecstr		hs ;
	int		rs ;
	int		rs1 ;
	int		opts ;
	int		f_hs = FALSE ;
	const char	**spp ;

#if	CF_DEBUGS
	debugprintf("progprinthelp: full fname=%s\n",helpfname) ;
#endif

/* first see if there is a "help schedule" in the ETC directory */

	spp = schedule ;
	rs = mkpath2(tmpfname,pr,HELPSCHEDFNAME) ;

	if (rs >= 0)
	    rs = perm(tmpfname,-1,-1,NULL,R_OK) ;

	if (rs >= 0) {

	    opts = VECSTR_OCOMPACT ;
	    rs = vecstr_start(&hs,15,opts) ;
	    f_hs = (rs >= 0) ;
	    if (rs >= 0) {
	        rs1 = vecstr_loadfile(&hs,FALSE,tmpfname) ;
	        if (rs1 >= 0)
	            vecstr_getvec(&hs,&spp) ;
	    }

	} /* end if (could access) */

/* create the values for the file schedule searching and find the file */

	if (rs >= 0) {

	    if ((rs = vecstr_start(&svars,6,0)) >= 0) {

	        rs = loadscheds(&svars,pr,sn) ;

/* OK, do the look-up */

	        if (rs >= 0)
	            rs = permsched(spp,&svars,
	                tmpfname,MAXPATHLEN, helpfname,R_OK) ;

	        if (isNotPresent(rs) && (spp != schedule))
	            rs1 = permsched(schedule,&svars,
	                tmpfname,MAXPATHLEN, helpfname,R_OK) ;

#if	CF_DEBUGS
	        debugprintf("progprinthelp: permsched() rs=%d tmpfname=%s\n",
	            rs,tmpfname) ;
#endif

	        vecstr_finish(&svars) ;
	    } /* end if (schedule variables) */

	} /* end if */

	if (f_hs)
	    vecstr_finish(&hs) ;

	return rs ;
}
/* end subroutine (findhelpfile) */


static int loadscheds(vecstr *slp,const char *pr,const char *sn)
{
	int		rs = SR_OK ;

	if (pr != NULL) rs = vecstr_envadd(slp,"r",pr,-1) ;

	if (rs >= 0) rs = vecstr_envadd(slp,"l",LIBCNAME,-1) ;

	if ((rs >= 0) && (sn != NULL))
	    rs = vecstr_envadd(slp,"n",sn,-1) ;

	return rs ;
}
/* end subroutine (loadscheds) */


static int printout(fp,lbuf,llen,fname)
void		*fp ;
char		lbuf[] ;
int		llen ;
const char	*fname ;
{
	bfile		outfile ;
	bfile		helpfile, *hfp = &helpfile ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		f_open = FALSE ;

	if (lbuf == NULL) return SR_FAULT ;

	if (fp == NULL) {
	    fp = &outfile ;
	    rs = bopen(fp,BFILE_STDOUT,"w",0666) ;
	    f_open = (rs >= 0) ;
	}

	if ((rs >= 0) &&
	    ((rs = bopen(hfp,fname,"r",0666)) >= 0)) {

#if	CF_SFIO
	    {
	        int	len ;

	        while ((rs = bread(hfp,lbuf,llen)) > 0) {
	            len = rs ;

	            rs = sfwrite(fp,lbuf,len) ;

#if	CF_DEBUGS
	            debugprintf("progprinthelp: sfwrite() rs=%d\n",rs) ;
#endif

	            if (rs < 0) break ;
		    wlen += rs ;

	        } /* end while */

	    } /* end block */

#else /* CF_SFIO */

	    if (llen) {
	        rs = bcopyblock(hfp,fp,-1) ;
		wlen = rs ;
	    }

#endif /* CF_SFIO */

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
/* end subroutine (printout) */


