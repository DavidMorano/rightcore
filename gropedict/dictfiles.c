/* dictfiles */

/* manage the GROPE dictionary */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGWORDS	0		/* debug word processing */
#define	CF_TRUNCATE	0		/* truncate files afterwards */
#define	CF_TRUNCALL	1		/* truncate all (do it!) */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This object manages the creation of the GROPE dictionary.


******************************************************************************/


#define	DICTFILES_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ids.h>
#include	<fsdir.h>
#include	<char.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"dictfiles.h"


/* local defines */

#define	DICTFILES_MAGIC		0x99227651
#define	DICTFILES_NEWDNAME	"NEW"
#define	DICTFILES_TO		(5 * 60)

#ifndef	NOFILE
#define	NOFILE	20
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t, gid_t *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	getpwd(char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	dictfiles_setup(DICTFILES *) ;
static int	dictfiles_setdown(DICTFILES *) ;
static int	dictfiles_openone(DICTFILES *,int) ;
static int	dictfiles_closeone(DICTFILES *) ;
static int	dictfiles_closefile(DICTFILES *,int) ;

static int	rmdirfiles(const char *) ;

#ifdef	COMMENT
static int	dictfiles_links(DICTFILES *,int) ;
#endif


/* local variables */


/* exported subroutines */


int dictfiles_open(op,maxopen,dname,prefix)
DICTFILES	*op ;
int		maxopen ;
const char	dname[] ;
const char	prefix[] ;
{
	int	rs = SR_OK ;
	int	size ;

	const char	*dnp ;

	char	pwd[MAXPATHLEN + 1] ;
	char	tmpdname[MAXPATHLEN + 1] ;


	if (op == NULL)
	    return SR_FAULT ;

	if ((dname == NULL) || (prefix == NULL))
	    return SR_FAULT ;

	if ((dname[0] == '\0') || (dname[0] == '\0'))
	    return SR_INVALID ;

	memset(op,0,sizeof(DICTFILES)) ;

	if (maxopen < 0)
	    maxopen = (NOFILE / 3) ;

	if (maxopen > DICTFILES_NLETTERS)
	    maxopen = DICTFILES_NLETTERS ;

	dnp = dname ;
	if (dname[0] != '/') {

	    dnp = tmpdname ;
	    rs = getpwd(pwd,MAXPATHLEN) ;

	    if (rs < 0)
	        goto bad0 ;

	    rs = mkpath2(tmpdname,pwd,dname) ;

	    if (rs < 0)
	        goto bad0 ;

	} /* end if (rooting as necessary) */

	rs = SR_NOMEM ;
	op->dictdname = mallocstr(dnp) ;

	if (op->dictdname == NULL)
	    goto bad1 ;

	op->prefix = mallocstr(prefix) ;

	if (op->prefix == NULL)
	    goto bad2 ;

	op->maxopen = maxopen ;
	size = op->maxopen * sizeof(struct dictfiles_file) ;
	rs = uc_malloc(size,&op->files) ;

	if (rs < 0)
	    goto bad3 ;

	memset(op->files,0,size) ;

/* is the directory used for new builds there? */

	rs = dictfiles_setup(op) ;

#if	CF_DEBUGS
	debugprintf("dictfiles_open: dictfiles_setup() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad4 ;

/* we are live, get out */

	op->magic = DICTFILES_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("dictfiles_open: rs=%d\n",rs) ;
#endif

	return rs ;

/* bad things */
bad4:
	if (op->files != NULL)
	    uc_free(op->files) ;

bad3:
	if (op->dictdname != NULL)
	    uc_free(op->prefix) ;

bad2:
	if (op->dictdname != NULL)
	    uc_free(op->dictdname) ;

bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (dictfiles_open) */


/* write to a dictionary file */
int dictfiles_write(op,buf,buflen)
DICTFILES	*op ;
const char	buf[] ;
int		buflen ;
{
	int	rs = SR_OK ;
	int	c ;

#if	CF_DEBUGS
	int	f_p = FALSE ;
#endif


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DICTFILES_MAGIC)
	    return SR_NOTOPEN ;

	c = CHAR_TOLC(buf[0]) ;

#if	CF_DEBUGS
	{
	    int	wl = buflen ;
	    while (wl && (buf[wl - 1] == '\n')) wl -= 1 ;
	    f_p = (c == 'p') ;
#if	CF_DEBUGWORDS
	    debugprintf("dictfiles_write: w=>%t<\n",buf,wl) ;
#else
	    if (f_p)
	        debugprintf("dictfiles_write: w=>%t<\n",buf,wl) ;
#endif
	}
#endif

/* if we do not have a file open, find one */

	if (! op->e[c].f_open) {

#if	CF_DEBUGS
	    debugprintf("dictfiles_write: need to open\n") ;
#endif

	    while (op->nopen >= op->maxopen)
	        rs = dictfiles_closeone(op) ;

#if	CF_DEBUGS
	    debugprintf("dictfiles_write: after closing some rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("dictfiles_write: call dictfiles_openone() \n") ;
#endif

	        rs = dictfiles_openone(op,c) ;

#if	CF_DEBUGS
	        debugprintf("dictfiles_write: dictfiles_openone() rs=%d\n",rs) ;
#endif

	    }

	} /* end if (was not already open) */

#if	CF_DEBUGS
	if (f_p)
	    debugprintf("dictfiles_write: proceeding rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    bfile	*fp ;

	    int	fi ;

	    fi = op->e[c].fi ;
	    fp = &op->files[fi].df ;
	    rs = bwrite(fp,buf,buflen) ;

	    if (rs >= 0) {
	        op->usage += 1 ;
	        op->e[c].len += buflen ;
	        op->e[c].usage = op->usage ;
	    }

	} /* end if */

#if	CF_DEBUGS
#if	CF_DEBUGWORDS
	debugprintf("dictfiles_write: ret rs=%d\n",rs) ;
#else
	if (f_p)
	    debugprintf("dictfiles_write: ret rs=%d\n",rs) ;
#endif
#endif

	return rs ;
}
/* end subroutine (dictfiles_write) */


int dictfiles_close(op)
DICTFILES	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	c ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DICTFILES_MAGIC)
	    return SR_NOTOPEN ;

/* close any open files */

	for (c = 0 ; c < DICTFILES_NLETTERS ; c += 1) {

	    if (op->e[c].f_open)
	        dictfiles_closefile(op,c) ;

#ifdef	COMMENT
	    rs = dictfiles_links(op,c) ;
#endif

#if	CF_DEBUGS
	    debugprintf("dictfiles_close: dictfiles_links() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        break ;

	} /* end for */

	if (op->files != NULL)
	    uc_free(op->files) ;

/* link up the files */

#if	CF_DEBUGS
	debugprintf("dictfiles_close: dictfiles_setdown() \n") ;
#endif

	rs1 = dictfiles_setdown(op) ;

#if	CF_DEBUGS
	debugprintf("dictfiles_close: dictfiles_setdown() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = rs1 ;

/* finish up */

	if (op->dictdname != NULL)
	    uc_free(op->dictdname) ;

	if (op->prefix != NULL)
	    uc_free(op->prefix) ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (dictfiles_close) */


/* local subroutines */


static int dictfiles_openone(op,c)
DICTFILES	*op ;
int		c ;
{
	int	rs = SR_NOENT ;
	int	fi, i ;


#if	CF_DEBUGS
	debugprintf("dictfiles_openone: c=%c\n",c) ;
#endif

/* find if there is a free one */

	fi = -1 ;
	for (i = 0 ; i < op->maxopen ; i += 1) {

	    if (! op->files[i].f_open) {
	        fi = i ;
	        break ;
	    }

	} /* end for */

	if (fi >= 0) {

	    bfile	*fp ;

	    char	tmpfname[MAXPATHLEN + 1] ;
	    char	sname[MAXNAMELEN + 1] ;
	    char	*oflags ;


	    fp = &op->files[i].df ;
	    tmpfname[0] = c ;
	    tmpfname[1] = '\0' ;
	    sncpy2(sname,MAXNAMELEN,op->prefix,tmpfname) ;

#if	CF_DEBUGS
	    debugprintf("dictfiles_openone: fname=%s\n",sname) ;
#endif

	    rs = mkpath3(tmpfname,op->dictdname,DICTFILES_NEWDNAME,sname) ;

#if	CF_DEBUGS
	    debugprintf("dictfiles_openone: rs=%d dictdname=%s\n",
	        rs,tmpfname) ;
#endif

	    if (rs >= 0) {

	        oflags = (op->e[c].len == 0) ? "wct" : "wca" ;
	        rs = bopen(fp,tmpfname,oflags,0664) ;

	        op->files[fi].f_open = (rs >= 0) ;
	        op->files[fi].c = c ;

	    }

	}

	if (rs >= 0) {
	    op->e[c].fi = fi ;
	    op->e[c].f_open = TRUE ;
	    op->nopen += 1 ;
	}

	return (rs >= 0) ? fi : rs ;
}
/* end subroutine (dictfiles_openone) */


static int dictfiles_closeone(op)
DICTFILES	*op ;
{
	int	rs = SR_NOTOPEN ;
	int	c, mc ;
	int	usage = INT_MAX ;


	mc = -1 ;
	for (c = 0 ; c < DICTFILES_NLETTERS ; c += 1) {

	    if (op->e[c].f_open) {
	        if (op->e[c].usage < usage) {
	            usage = op->e[c].usage ;
	            mc = c ;
	        }
	    }

	} /* end for */

#if	CF_DEBUGS
	debugprintf("dictfiles_closeone: mc=%d\n",mc) ;
#endif

	if (mc >= 0)
	    rs = dictfiles_closefile(op,mc) ;

#if	CF_DEBUGS
	debugprintf("dictfiles_closeone: rs=%d mc=%d\n",rs,mc) ;
#endif

	return (rs >= 0) ? mc : rs ;
}
/* end subroutine (dictfiles_closeone) */


static int dictfiles_closefile(op,c)
DICTFILES	*op ;
int		c ;
{
	offset_t	boff ;

	bfile	*fp ;

	int	rs = SR_NOTOPEN ;
	int	fi ;

#if	CF_DEBUGS
	int	f_p = FALSE ;
#endif


#if	CF_DEBUGS
	debugprintf("dictfiles_closefile: enter c=%c\n",c) ;
#endif

	if (op->e[c].f_open) {

#if	CF_DEBUGS
	    debugprintf("dictfiles_closefile: open\n",c) ;
#endif

	    rs = SR_OK ;
	    fi = op->e[c].fi ;
	    fp = &op->files[fi].df ;

#if	CF_TRUNCATE
	    if (! op->e[c].f_trunced) {

	        len = op->e[c].len ;

#if	CF_DEBUGS
	        f_p = (c == 'p') ;
	        if (f_p)
	            debugprintf("dictfiles_closefile: c=%c trunc len=%lu\n",
	                c,len) ;
#endif

	  	boff = len ;
#if	CF_TRUNCALL
	        rs = btruncate(fp,boff) ;
#else
	        if (len > 0)
	            rs = btruncate(fp,boff) ;
#endif /* CF_TRUNCALL */

	        op->e[c].f_trunced = TRUE ;
	    }
#endif /* CF_TRUNCATE */

	    bclose(fp) ;

	    op->files[fi].f_open = FALSE ;
	    op->e[c].f_open = FALSE ;
	    op->e[c].fi = -1 ;
	    op->nopen -= 1 ;

	} /* end if (was open) */

#if	CF_DEBUGS
	debugprintf("dictfiles_closefile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dictfiles_closefile) */


#ifdef	COMMENT

static int dictfiles_links(op,c)
DICTFILES	*op ;
int		c ;
{
	int	rs ;

	char	oldfname[MAXPATHLEN + 1] ;
	char	newfname[MAXPATHLEN + 1] ;
	char	sname[MAXNAMELEN + 1] ;


	newfname[0] = c ;
	newfname[1] = '\0' ;
	sncpy2(sname,MAXNAMELEN,op->prefix,newfname) ;

	rs = mkpath2(oldfname,op->dictdname,sname) ;

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("dictfiles_links: len=%lu\n",op->e[c].len) ;
#endif

	    if (op->e[c].len > 0) {

	        rs = mkpath3(newfname,op->dictdname,
	            DICTFILES_NEWDNAME,sname) ;

	        if (rs > 0) {

#if	CF_DEBUGS
	            debugprintf("dictfiles_links: new=%s\n",newfname) ;
	            debugprintf("dictfiles_links: old=%s\n",oldfname) ;
#endif

	            rs = u_rename(newfname,oldfname) ;

#if	CF_DEBUGS
	            debugprintf("dictfiles_links: u_rename() rs=%d\n",rs) ;
#endif

	        }

	        if (rs < 0)
	            u_unlink(newfname) ;

	    } else
	        u_unlink(oldfname) ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("dictfiles_links: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dictfiles_links) */

#endif /* COMMENT */


static int dictfiles_setup(op)
DICTFILES	*op ;
{
	struct ustat	sb ;

	IDS	us ;

	int	rs, rs1 ;
	int	i ;

	char	tmpdname[MAXPATHLEN + 1] ;


	rs = mkpath2(tmpdname,op->dictdname,DICTFILES_NEWDNAME) ;

#if	CF_DEBUGS
	debugprintf("dictfiles_setup: rs=%d tmpdname=%s\n",rs,tmpdname) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	rs = ids_load(&us) ;

#if	CF_DEBUGS
	debugprintf("dictfiles_setup: ds_load() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	rs = SR_ALREADY ;
	for (i = 0 ; i < 3 ; i += 1) {

	    rs1 = u_stat(tmpdname,&sb) ;

#if	CF_DEBUGS
	    debugprintf("dictfiles_setup: u_stat() rs=%d\n",rs1) ;
#endif

	    if (rs1 < 0) {
	        rs = SR_OK ;
	        break ;
	    }

	    {

	        time_t	daytime = time(NULL) ;


	        if ((daytime - sb.st_mtime) < DICTFILES_TO)
	            break ;

	        rmdirfiles(tmpdname) ;

	    } /* end block */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("dictfiles_setup: out_loop rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

	rs = mkdirs(tmpdname,0775) ;

#if	CF_DEBUGS
	debugprintf("dictfiles_setup: mkdirs() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

ret1:
	ids_release(&us) ;

ret0:
bad0:
	return rs ;

/* bad stuff */
bad1:
	goto ret1 ;
}
/* end subroutine (dictfiles_setup) */


static int dictfiles_setdown(op)
DICTFILES	*op ;
{
	int	rs, rs1 ;
	int	cl ;

	char	tmpdname[MAXPATHLEN + 1] ;


	rs = mkpath2(tmpdname,op->dictdname,DICTFILES_NEWDNAME) ;

#if	CF_DEBUGS
	debugprintf("dictfiles_setdown: new=%s\n",tmpdname) ;
#endif

/* link all new files to the old ones */

	if (rs >= 0) {

	    int		olr, nlr ;
	    int		c ;

	    char	oldfname[MAXPATHLEN + 1] ;
	    char	newfname[MAXPATHLEN + 1] ;
	    char	*ofp, *nfp ;


	    cl = mkpath2(oldfname,op->dictdname,op->prefix) ;

	    ofp = oldfname + cl ;
	    olr = MAXPATHLEN - cl ;
	    cl = mkpath3(newfname,op->dictdname,DICTFILES_NEWDNAME,
	        op->prefix) ;

	    nfp = newfname + cl ;
	    nlr = MAXPATHLEN - cl ;

	    for (c = 0 ; c < DICTFILES_NLETTERS ; c += 1) {

	        if (op->e[c].len > 0) {

	            ofp[0] = c ;
	            ofp[1] = '\0' ;

	            nfp[0] = c ;
	            nfp[1] = '\0' ;

	            rs1 = u_rename(newfname,oldfname) ;

	            if (rs1 < 0)
	                u_unlink(newfname) ;

	        } /* end if */

	    } /* end for */

	} /* end block */

/* remove the "new" directory (and any files in it) */

	rs1 = rmdirfiles(tmpdname) ;

	if (rs >= 0)
		rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("dictfiles_setdown: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dictfiles_setdown) */


static int rmdirfiles(dname)
const char	dname[] ;
{
	FSDIR		d ;
	FSDIR_ENT	ds ;

	int	rs, rs1 ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if ((rs = fsdir_open(&d,dname)) >= 0) {
	    int	ch ;

	    while (fsdir_read(&d,&ds) > 0) {

	        if (ds.name[0] == '.') {
	            ch = ds.name[1] ;
	            if (ch == '\0') continue ;
	            if ((ch == '.') && (ds.name[2] == '\0')) continue ;
	        }

	        mkpath2(tmpfname,dname,ds.name) ;

	        u_unlink(tmpfname) ;

	    } /* end while */

	    fsdir_close(&d) ;
	} /* end if */

/* now remove the directory itself */

	rs1 = u_rmdir(dname) ;

	if (rs >= 0)
	    rs = rs1 ;

	return rs ;
}
/* end subroutine (rmdirfiles) */



