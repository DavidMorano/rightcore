/* systems */

/* get additional machine dialing information */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFIELD	0		/* some other debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

	- 2003-11-04, David A­D­ Morano
        I don't know where all this has been (apparently "around") but I grabbed
        it from the SYSTEMS object!

*/

/* Copyright © 1998,2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is a container for "systems" listed in "systems" files.
	These are files that are keyed by the name of a "system" (whatever that
	is) and has a composite value that is one or more strings.


*******************************************************************************/


#define	SYSTEMS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"systems.h"


/* local defines */

#define	SYSTEMS_FILE	struct systems_file

#define	TI_FILECHECK	3
#define	SYSLINELEN	((4 * 1024) + 1)
#define	ARGSBUFLEN	((6 * MAXPATHLEN) + 35)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	starmat(const char *,const char *) ;
extern int	getpwd(char *,int) ;
extern int	field_srvarg(FIELD *,const uchar *,char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif /* CF_DEBUGS */


/* external variables */


/* local structures */

struct systems_file {
	const char	*fname ;
	time_t		mtime ;
	size_t		fsize ;
	dev_t		dev ;
	ino64_t		ino ;
} ;


/* forward references */

static int systems_fileparse(SYSTEMS *,int,SYSTEMS_FILE *) ;
static int systems_filealready(SYSTEMS *,dev_t,uino_t) ;
static int systems_procline(SYSTEMS *,int,FIELD *) ;
static int systems_delfes(SYSTEMS *,int) ;

int	systems_close(SYSTEMS *) ;

static int file_start(SYSTEMS_FILE *,const char *) ;
static int file_finish(SYSTEMS_FILE *) ;

static int entry_start(SYSTEMS_ENT *,int,const char *,int) ;
static int entry_dialer(SYSTEMS_ENT *,const char *,int) ;
static int entry_args(SYSTEMS_ENT *,const char *,int) ;
static int entry_finish(SYSTEMS_ENT *) ;

static int bdumpline(bfile *,char *,int) ;


/* local variables */

/* entry field terminators */
static const unsigned char 	fterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x24,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

/* entry argument terminators (just '#' to provide "remainder" function) */
static const unsigned char 	remterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int systems_open(SYSTEMS *op,cchar sysfname[])
{
	int		rs ;
	int		size ;
	int		opts ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("systems_open: sysfname=%s\n",sysfname) ;
#endif

	size = sizeof(SYSTEMS_FILE) ;
	opts = VECOBJ_OREUSE ;
	if ((rs = vecobj_start(&op->files,size,10,opts)) >= 0) {
	    size = sizeof(SYSTEMS_ENT) ;
	    opts = 0 ;
	    if ((rs = vecobj_start(&op->entries,size,20,opts)) >= 0) {
	        op->magic = SYSTEMS_MAGIC ;
	        if (sysfname != NULL) {
	            rs = systems_fileadd(op,sysfname) ;
	        }
	        if (rs < 0) {
		    op->magic = 0 ;
		    vecobj_finish(&op->entries) ;
	        }
	    }
	    if (rs < 0)
	       vecobj_finish(&op->files) ;
	} /* end if (vecobj_start) */

	return rs ;
}
/* end subroutine (systems_open) */


/* free up the resources occupied by a SYSTEMS list */
int systems_close(SYSTEMS *op)
{
	SYSTEMS_ENT	*dep ;
	SYSTEMS_FILE	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSTEMS_MAGIC) return SR_NOTOPEN ;

/* free up all enties */

	for (i = 0 ; vecobj_get(&op->entries,i,&dep) >= 0 ; i += 1) {
	    if (dep != NULL) {
	        rs1 = entry_finish(dep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecobj_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

/* free up the files */

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep != NULL) {
	        if (fep->fname != NULL) {
	            rs1 = uc_free(fep->fname) ;
		    if (rs >= 0) rs = rs1 ;
	        }
	    }
	} /* end for */

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (systems_close) */


/* add the contents of a file to the dialer list */
int systems_fileadd(SYSTEMS *op,cchar sysfname[])
{
	int		rs = SR_OK ;
	const char	*np ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if ((op == NULL) || (sysfname == NULL)) return SR_FAULT ;

	if (op->magic != SYSTEMS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("systems_fileadd: sysfname=%s\n",sysfname) ;
#endif

/* make a file pathname if necessary */

	np = sysfname ;
	if (sysfname[0] != '/') {
	    char	pwdbuf[MAXPATHLEN+1] ;
	    if ((rs = getpwd(pwdbuf,MAXPATHLEN)) >= 0) {
	        np = tmpfname ;
	        rs = mkpath2(tmpfname,pwdbuf,sysfname) ;
	    }
	} /* end if */

	if (rs >= 0) {
	    SYSTEMS_FILE	fe, *fep ;
	    if ((rs = file_start(&fe,np)) >= 0) {
	        if ((rs = vecobj_add(&op->files,&fe)) >= 0) {
	            int		fi = rs ;
	            if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	                rs = systems_fileparse(op,fi,fep) ;
	                if ((rs < 0) && (rs != SR_EXIST)) {
		            vecobj_del(&op->files,fi) ;
	                }
	            }
	            if (rs < 0)
		        vecobj_del(&op->files,fi) ;
	        } /* end if (vecobj_add) */
	        if (rs < 0)
		    file_finish(&fe) ;
	    } /* end if (file_start) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("systems_fileadd: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (systems_fileadd) */


int systems_curbegin(SYSTEMS *op,SYSTEMS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSTEMS_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (systems_curbegin) */


int systems_curend(SYSTEMS *op,SYSTEMS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSTEMS_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (systems_curend) */


/* enumerate all entries */
int systems_enum(SYSTEMS *op,SYSTEMS_CUR *curp,SYSTEMS_ENT **depp)
{
	int		rs ;
	int		ei ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (depp == NULL) return SR_FAULT ;

	if (op->magic != SYSTEMS_MAGIC) return SR_NOTOPEN ;

	ei = (curp->i < 0) ? 0 : (curp->i + 1) ;

	if ((rs = vecobj_get(&op->entries,ei,depp)) >= 0)
	    curp->i = ei ;

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (systems_enum) */


/* fetch a system entry (if there is one) */
int systems_fetch(SYSTEMS *op,cchar *name,SYSTEMS_CUR *curp,SYSTEMS_ENT **depp)
{
	int		rs ;
	int		ei ;

	if (op == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (depp == NULL) return SR_FAULT ;

	if (op->magic != SYSTEMS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("systems_fetch: ent name=%s\n",name) ;
#endif

	ei = (curp->i < 0) ? 0 : (curp->i + 1) ;

	while ((rs = vecobj_get(&op->entries,ei,depp)) >= 0) {
	    if (*depp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("systems_fetch: sysname=%s\n",(*depp)->sysname) ;
#endif

	    if (starmat((*depp)->sysname,name))
	        break ;

	    ei += 1 ;

	} /* end while */

	if (rs >= 0)
	    curp->i = ei ;

#if	CF_DEBUGS
	debugprintf("systems_fetch: ret rs=%d ei=%d\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (systems_fetch) */


/* check if any files have changed */
int systems_check(SYSTEMS *op,time_t dt)
{
	struct ustat	sb ;
	SYSTEMS_FILE	*fep ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	int		f ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSTEMS_MAGIC) return SR_NOTOPEN ;

	if (dt <= 0)
	    dt = time(NULL) ;

/* should we even check? */

	if ((dt - op->checktime) <= TI_FILECHECK)
	    return SR_OK ;

	op->checktime = dt ;

/* loop through files looking for a change */

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

	    rs = u_stat(fep->fname,&sb) ;

	    f = ((rs < 0) || (sb.st_mtime > fep->mtime) || 
	        (sb.st_size != fep->fsize)) ;

	    if (f) {

	        c += 1 ;
	        systems_delfes(op,i) ;

	        if (rs >= 0)
	            systems_fileparse(op,i,fep) ;

	    } /* end if (file changed) */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("systems_check: ret rs=%d c=%u\n",rs,c) ;
#endif

	return c ;
}
/* end subroutine (systems_check) */


/* private subroutines */


/* add the contents of a file to the dialer list */
static int systems_fileparse(SYSTEMS *op,int fi,SYSTEMS_FILE *fep)
{
	bfile		dialfile, *sfp = &dialfile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = bopen(sfp,fep->fname,"r",0664)) >= 0) {
	    USTAT	sb ;
	    if ((rs = bcontrol(sfp,BC_STAT,&sb)) >= 0) {
		const dev_t	dev = sb.st_dev ;
		const uino_t	ino = sb.st_ino ;
#if	CF_DEBUGS
		debugprintf("systems_fileparse: dev=\\x%08x ino=%llu\n",
			dev,ino) ;
#endif
		if ((rs = systems_filealready(op,dev,ino)) == 0) {
		    const int	llen = SYSLINELEN ;
		    char	*lbuf ;
		    if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
		        FIELD	fsb ;
		        int	len ;
		        fep->mtime = sb.st_mtime ;
		        fep->fsize = (size_t) (sb.st_size & UINT_MAX) ;
		        fep->dev = sb.st_dev ;
		        fep->ino = sb.st_ino ;
		        while ((rs = breadlines(sfp,lbuf,llen,NULL)) > 0) {
	    	            len = rs ;

	    	        if (len == 1) continue ;	/* blank line */

	    	        if (lbuf[len - 1] != '\n') {
			    bdumpline(sfp,lbuf,llen) ;
	        	    continue ;
	    	        }

	    		len -= 1 ;
	    		if ((len == 0) || (lbuf[0] == '#')) continue ;

	    	        if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
			    int		fl ;
			    const char	*fp ;

	        	    fl = field_get(&fsb,fterms,&fp) ;

#if	CF_DEBUGS && CF_DEBUGSFIELD
	    	   	     debugprintf("systems_fp: field> %t\n",
				fp,fl) ;
#endif

	        	    if ((fl > 0) && (fsb.term != '#')) {
	            	        rs = systems_procline(op,fi,&fsb) ;
	            	        if (rs > 0) c += 1 ;
	        	    } /* end if */

	        	    field_finish(&fsb) ;
	    	        } /* end if (field) */

	                if (rs < 0) break ;
	            } /* end while (reading lines) */
			uc_free(lbuf) ;
		    } /* end if (m-a) */
	        } /* end if (not-already) */
	    } /* end if (bcontrol) */
	    rs1 = bclose(sfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (systems_fileparse) */


static int systems_filealready(SYSTEMS *op,dev_t dev,uino_t ino)
{
	SYSTEMS_FILE	*fep ;
	vecobj		*flp = &op->files ;
	int		rs ;
	int		i ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("systems_filealready: searching dev=\\x%08x ino=%llu\n",
		dev,ino) ;
#endif

	for (i = 0 ; (rs = vecobj_get(flp,i,&fep)) >= 0 ; i += 1) {
	    if (fep != NULL) {
		f = ((fep->dev == dev) && (fep->ino == ino)) ;
		if (f) break ;
	    }
	} /* end for */
	if (rs == SR_NOTFOUND) rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("systems_filealready: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (systems_filealready) */


static int systems_procline(SYSTEMS *op,int fi,FIELD *fsp)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if ((fsp->fl > 0) && (fsp->term != '#')) {
	    SYSTEMS_ENT	e ;
	    int		fl = fsp->fl ;
	    cchar	*fp = (cchar *) fsp->fp ;
	    if ((rs = entry_start(&e,fi,fp,fl)) >= 0) {
		int	f_fin = TRUE ;
	        if ((fl = field_get(fsp,fterms,&fp)) > 0) {
	            if ((rs = entry_dialer(&e,fp,fl)) >= 0) {
	                if (fsp->term != '#') {
	                    const int	alen = ARGSBUFLEN ;
			    const uchar	*ft = remterms ;
	                    char	abuf[ARGSBUFLEN + 1] ;
	                    if ((rs = field_srvarg(fsp,ft,abuf,alen)) >= 0) {
	                	const int	al = rs ;
	                	if ((rs = entry_args(&e,abuf,al)) >= 0) {
				    vecobj	*elp = &op->entries ;
	                    	    if ((rs = vecobj_add(elp,&e)) >= 0) {
					f = TRUE ;
					f_fin = FALSE ;
				    }
#if	CF_DEBUGS
	debugprintf("systems_procline: mid5 rs=%d\n",rs) ;
#endif
			        }
#if	CF_DEBUGS
	debugprintf("systems_procline: mid6 rs=%d\n",rs) ;
#endif
	             	    } /* end if (field_srvarg) */
#if	CF_DEBUGS
	debugprintf("systems_procline: mid7 rs=%d\n",rs) ;
#endif
	                } /* end if (not a comment) */
#if	CF_DEBUGS
	debugprintf("systems_procline: mid8 rs=%d\n",rs) ;
#endif
	            } /* end if (entry_dialer) */
#if	CF_DEBUGS
	debugprintf("systems_procline: mid9 rs=%d\n",rs) ;
#endif
	        } /* end if (field_get) */
	        if ((rs < 0) || f_fin)
		    entry_finish(&e) ;
	    } /* end if (entry_start) */
#if	CF_DEBUGS
	debugprintf("systems_procline: mid10 rs=%d\n",rs) ;
#endif
	} /* end if (possible) */

#if	CF_DEBUGS
	debugprintf("systems_procline: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (systems_procline) */


static int systems_delfes(SYSTEMS *op,int fi)
{
	SYSTEMS_ENT	*ep ;
	vecobj		*elp = &op->entries ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(elp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (ep->fi == fi) {
	            rs1 = entry_finish(ep) ;
		    if (rs >= 0) rs = rs1 ;
	            rs1 = vecobj_del(&op->entries,i--) ;
		    if (rs >= 0) rs = rs1 ;
	        }
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (systems_delfes) */


static int file_start(SYSTEMS_FILE *fep,cchar fname[])
{
	int		rs ;
	cchar		*cp ;

	if (fname == NULL) return SR_FAULT ;

	memset(fep,0,sizeof(SYSTEMS_FILE)) ;

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    fep->fname = cp ;
	}

	return rs ;
}
/* end subroutine (file_start) */


static int file_finish(SYSTEMS_FILE *fep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (fep == NULL) return SR_FAULT ;

	if (fep->fname != NULL) {
	    rs1 = uc_free(fep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (file_finish) */


static int entry_start(SYSTEMS_ENT *ep,int fi,cchar *sp,int sl)
{
	int		rs ;
	const char	*cp ;

	memset(ep,0,sizeof(SYSTEMS_ENT)) ;

	ep->fi = fi ;
	if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	    ep->sysnamelen = sl ;
	    ep->sysname = cp ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_dialer(SYSTEMS_ENT *ep,cchar *dp,int dl)
{
	int		rs ;
	cchar		*cp ;

	if ((rs = uc_mallocstrw(dp,dl,&cp)) >= 0) {
	    ep->dialernamelen = dl ;
	    ep->dialername = cp ;
	}

#if	CF_DEBUGS
	debugprintf("entry_dialer: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (entry_dialer) */


static int entry_args(SYSTEMS_ENT *ep,cchar args[],int argslen)
{
	int		rs = SR_OK ;

	if (argslen > 0) {
	    const char	*cp ;
	    ep->dialerargslen = argslen ;
	    if ((rs = uc_mallocstrw(args,argslen,&cp)) >= 0) {
		ep->dialerargs = cp ;
	    }
	} /* end if */

	return (rs >= 0) ? argslen : rs ;
}
/* end subroutine (entry_args) */


/* free up an entry */
static int entry_finish(SYSTEMS_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->sysnamelen <= 0)
	    return SR_OK ;

	if (ep->dialerargs != NULL) {
	    rs1 = uc_free(ep->dialerargs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->dialername != NULL) {
	    rs1 = uc_free(ep->dialername) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->sysname != NULL) {
	    rs1 = uc_free(ep->sysname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	ep->sysnamelen = 0 ;
	return SR_OK ;
}
/* end subroutine (entry_finish) */


static int bdumpline(bfile *fp,char *lbuf,int llen)
{
	int		rs ;
	while ((rs = breadlines(fp,lbuf,llen,NULL)) > 0) {
	    if (lbuf[rs - 1] == '\n') break ;
	}
	return rs ;
}
/* end subroutine (bdumpline) */


