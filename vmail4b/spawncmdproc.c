/* spawncmdproc */

/* program spawn-command */


#define	CF_DEBUGS	0		/* compile-time debugging print-outs */


/* revision history:

	= 2002-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine spawns a SHELL that executes a specified command.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<spawnproc.h>
#include	<filebuf.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#define	CMDBUFLEN	(5 * MAXPATHLEN)


/* external subroutines */

extern int	snsdd(char *,int,const char *,uint) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	getpwd(char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	isproc(pid_t) ;


/* local structures */


/* forward references */

static int	mkcmdfname(char *,const char *,const char *) ;


/* exported subroutines */


int spawncmdproc(SPAWNPROC *psp,cchar *shell,cchar *cmd)
{
	const int	am = (R_OK|X_OK) ;
	int		rs ;
	int		pid = 0 ;
	const char	*shprog = NULL ;
	char		shbuf[MAXPATHLEN + 1] ;

	if (psp == NULL) return SR_FAULT ;
	if (shell == NULL) return SR_FAULT ;
	if (cmd == NULL) return SR_FAULT ;

	if (shell[0] == '\0') return SR_INVALID ;
	if (cmd[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("spawncmdproc: shell=%s\n",shell) ;
	debugprintf("spawncmdproc: cmd=>%s<\n",cmd) ;
#endif

	shprog = shell ;
	if ((rs = findfilepath(NULL,shbuf,shell,am)) >= 0) {
	    char	cmdfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("spawncmdproc: findfilepath() rs=%d\n",rs) ;
	debugprintf("spawncmdproc: shbuf=%s\n",shbuf) ;
#endif

	    if (rs == 0) {
	        char	pwd[MAXPATHLEN + 1] ;
	        shprog = shbuf ;
	        if ((rs = getpwd(pwd,MAXPATHLEN)) >= 0) {
	            rs = mkpath2(shbuf,pwd,shell) ;
	        }
	    }

#if	CF_DEBUGS
	debugprintf("spawncmdproc: rs=%d shprog=%s\n",rs,shprog) ;
#endif

/* create and write to shell file */

	    if ((rs = mkcmdfname(cmdfname,shprog,cmd)) >= 0) {
		int	cl ;
	        cchar	*cp ;

#if	CF_DEBUGS
	debugprintf("spawncmdproc: mkcmdfname() rs=%d \n",rs) ;
#endif

/* spawn the shell */

	        if ((cl = sfbasename(shprog,-1,&cp)) > 0) {
		    cchar	*argz ;
		    if ((rs = uc_mallocstrw(cp,cl,&argz)) >= 0) {
	                cchar	*av[3] ;
	                av[0] = argz ;
	                av[1] = cmdfname ;
	                av[2] = NULL ;
	                if ((rs = spawnproc(psp,shprog,av,NULL)) >= 0) {
			    pid = rs ;
			}
			uc_free(argz) ;
		    } /* end if (m-a-f) */
	        } else
	            rs = SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("spawncmdproc: procspawn() rs=%d \n",rs) ;
#endif

/* remove the file */

	    } /* end if (mkcmdfname) */

	} /* end if (findfilepath) */

#if	CF_DEBUGS
	debugprintf("spawncmdproc: ret rs=%d pid=%u\n",rs,pid) ;
#endif

	return (rs >= 0) ? pid: rs ;
}
/* end subroutine (spawncmdproc) */


/* local subroutines */


static int mkcmdfname(char *cmdfname,cchar *shprog,cchar *cmd)
{
	int		rs ;
	int		rs1 ;
	const char	*tmpdname = getenv(VARTMPDNAME) ;
	char		inname[MAXPATHLEN + 1] ;

	if (tmpdname == NULL) tmpdname = TMPDNAME ;

	if ((rs = mkpath2(inname,tmpdname,"pscXXXXXXXXXXX")) >= 0) {
	    const mode_t	omode = 0770 ;
	    const int		oflags = (O_WRONLY | O_CREAT | O_TRUNC) ;

#if	CF_DEBUGS
	debugprintf("spawncmdproc/mkcmdfname: inname=%s\n",inname) ;
#endif

	    if ((rs = opentmpfile(inname,oflags,omode,cmdfname)) >= 0) {
		const int	cmdlen = CMDBUFLEN ;
	        const int	sfd = rs ;
	        char		*cmdbuf ;

#if	CF_DEBUGS
	debugprintf("spawncmdproc/mkcmdfname: opentmpfile() rs=%d\n",rs) ;
#endif

		if ((rs = uc_malloc((cmdlen+1),&cmdbuf)) >= 0) {
	            FILEBUF	b ;
		    if ((rs = filebuf_start(&b,sfd,0L,0,0)) >= 0) {
			int	i ;
			int	cl ;

	    	        for (i = 0 ; (rs >= 0) && (i < 4) ; i += 1) {
			    cchar	*sp ;
			    switch (i) {
		            case 0:
		                rs = sncpy2(cmdbuf,cmdlen,"#!",shprog) ;
		                break ;
#if	CF_DEBUGS
		             case 1:
	    	                sp = "exec 2> ee ; set -x" ;
	    	                rs = sncpy1(cmdbuf,cmdlen,sp) ;
		    		break ;
#endif /* CF_DEBUGS */
			     case 2:
				sp = "rm -f " ;
	    	    	        rs = sncpy2(cmdbuf,cmdlen,sp,cmdfname) ;
		    		break ;
			     case 3:
	    	    		rs = sncpy2(cmdbuf,cmdlen,"exec ",cmd) ;
		    		break ;
			     default:
		    		rs = 0 ;
		    		break ;
			    } /* end switch */
			    cl = rs ;
			    if ((rs >= 0) && (cl > 0)) {
	            	        rs = filebuf_print(&b,cmdbuf,cl) ;
	        	    }
			} /* end for */

	                rs1 = filebuf_finish(&b) ;
		        if (rs >= 0) rs = rs1 ;
	            } /* end if (filebuf) */
		    uc_free(cmdbuf) ;
		} /* end if (m-a-f) */

	        if ((rs < 0) && (cmdfname[0] != '\0')) {
	            u_unlink(cmdfname) ;
	            cmdfname[0] = '\0' ;
	        }

	        u_close(sfd) ;
	    } /* end if (opentmpfile) */

	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (mkcmdfname) */


