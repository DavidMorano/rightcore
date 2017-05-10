/* dialtab */

/* get additional machine dialing information */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module provides a management object to access dialing information
        that is used by some PCS utilities to access remote machines. This whole
        dialing information thing was a hack when accessing other machine
        because so problematic due to security considerations.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecobj.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"dialtab.h"


/* local defines */

#define	DIALTAB_MAGIC	31415926

#define	DIALTAB_FILE	struct dialtab_file

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + 100)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	getpwd(char *,int) ;


/* external variables */


/* local structures */

struct dialtab_file {
	const char	*fname ;
	time_t		mtime ;
	dev_t		dev ;
	ino64_t		ino ;
	int		size ;
} ;


/* forward references */

int		dialtab_fileadd(DIALTAB *,const char *) ;
int		dialtab_finish(DIALTAB *) ;

static int	dialtab_filedump(DIALTAB *,int) ;
static int	dialtab_filedel(DIALTAB *,int) ;

static int	file_start(struct dialtab_file *,const char *) ;
static int	file_finish(struct dialtab_file *) ;

static int	entry_start(struct dialtab_ent *,const char *,int) ;
static int	entry_enough(struct dialtab_ent *) ;
static int	entry_finish(struct dialtab_ent *) ;

static void	freeit(const char **) ;


/* local variables */

static const unsigned char 	fterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x24,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
} ;

static const char	*dialkeys[] = {
	"uucp",
	"inet",
	"username",
	"password",
	NULL,
} ;

enum dialkeys {
	dialkey_uucp,
	dialkey_inet,
	dialkey_username,
	dialkey_password,
	dialkey_overlast
} ;


/* exported subroutines */


int dialtab_open(op,dialfname)
DIALTAB		*op ;
const char	dialfname[] ;
{
	int	rs ;
	int	size ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic == DIALTAB_MAGIC)
	    return SR_INUSE ;

	size = sizeof(struct dialtab_file) ;
	rs = vecobj_start(&op->files,size,10,VECOBJ_PREUSE) ;
	if (rs < 0)
	    goto bad1 ;

	size = sizeof(struct dialtab_ent) ;
	rs = vecobj_start(&op->entries,size,20,VECOBJ_PREUSE) ;
	if (rs < 0)
	    goto bad2 ;

	op->magic = DIALTAB_MAGIC ;
	if (dialfname != NULL) {

	    rs = dialtab_fileadd(op,dialfname) ;
	    if (rs < 0)
	        goto bad3 ;

	} /* end if (adding a file) */

ret0:
	return rs ;

/* bad stuff */
bad3:
	vecobj_finish(&op->entries) ;

bad2:
	vecobj_finish(&op->files) ;

bad1:
bad0:
	op->magic = 0 ;
	goto ret0 ;
}
/* end subroutine (dialtab_open) */


/* add the contents of a file to the dialer list */
int dialtab_fileadd(op,dialfname)
DIALTAB		*op ;
const char	dialfname[] ;
{
	struct dialtab_ent	de ;

	struct dialtab_file	fe, *fep ;

	struct ustat		sb ;

	FIELD	fsb ;

	bfile	dialfile, *sfp = &dialfile ;

	int	rs ;
	int	ki ;
	int	fi, cl, len ;
	int	fl ;
	int	line = 0 ;
	int	c = 0 ;
	int	f_ent = FALSE ;

	const char	*fp ;
	const char	*fnp, *cp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


	if ((op == NULL) || (dialfname == NULL))
	    return SR_FAULT ;

	if (op->magic != DIALTAB_MAGIC)
	    return SR_NOTOPEN ;

	rs = bopen(sfp,dialfname,"r",0664) ;
	if (rs < 0)
	    goto bad0 ;

/* add the file name to */

	fnp = dialfname ;
	if (dialfname[0] != '/') {
	    char	pwdbuf[MAXPATHLEN+1] ;
	    fnp = tmpfname ;
	    rs = getpwd(pwdbuf,MAXPATHLEN) ;
	    if (rs >= 0)
	        rs = mkpath2(tmpfname,pwdbuf,dialfname) ;
	}

	if (rs >= 0)
	    rs = file_start(&fe,fnp) ;

	if (rs < 0)
	    goto bad1 ;

	rs = vecobj_add(&op->files,&fe) ;
	fi = rs ;
	if (rs < 0)
	    goto bad2 ;

	rs = vecobj_get(&op->files,fi,&fep) ;
	if (rs < 0)
	    goto bad3 ;

	bcontrol(sfp,BC_STAT,&sb) ;

	fep->dev = sb.st_dev ;
	fep->ino = sb.st_ino ;
	fep->mtime = sb.st_mtime ;
	fep->size = sb.st_size ;

/* start reading and processing the dial table file */

	while ((rs = breadline(sfp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    line += 1 ;
	    if (len == 1) continue ;	/* blank line */

	    if (linebuf[len - 1] != '\n') {
	        while ((c = bgetc(sfp)) >= 0) {
	            if (c == '\n') break ;
	        }
	        continue ;
	    }

	    cl = sfshrink(linebuf,len,&cp) ;

	    if ((cl == 0) || (*cp == '#')) continue ;

	    if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	        if ((fl = field_get(&fsb,fterms,&fp)) > 0) {

	            if (fsb.term == ':') {

			if (f_ent) {

	                if (entry_enough(&de) > 0) {

	                    rs = vecobj_add(&op->entries, &de) ;
	                    if (rs < 0) break ;

	                    c += 1 ;
			    f_ent = FALSE ;

	                } else {

	                    entry_finish(&de) ;
			    f_ent = FALSE ;

	                }

			} /* end if (entry) */

	                rs = entry_start(&de,fp,fl) ;
			f_ent = (rs >= 0) ;

	            } else {

	                ki = matostr(dialkeys,2,fp,fl) ;

	                if (ki >= 0) {

	                    if ((fl = field_get(&fsb,fterms,&fp)) > 0) {

	                        switch (ki) {

	                        case dialkey_uucp:
	                            de.uucp = mallocstrw(fp,fl) ;
	                            break ;

	                        case dialkey_inet:
	                            de.inet = mallocstrw(fp,fl) ;
	                            break ;

	                        case dialkey_username:
	                            de.username = mallocstrw(fp,fl) ;
	                            break ;

	                        case dialkey_password:
	                            de.password = mallocstrw(fp,fl) ;
	                            break ;

	                        } /* end switch */

	                    } /* end if (got value for this key) */

	                } /* end if (got a valid key) */

	            } /* end if */

	        } /* end if (non-zero-length field) */

	        field_finish(&fsb) ;
	    } /* end if */

	    if (rs < 0)
	        break ;

	} /* end while (reading lines) */

	if (rs < 0)
	    goto bad4 ;

	if (f_ent && (entry_enough(&de) > 0)) {

	    rs = vecobj_add(&op->entries,&de) ;

#if	CF_DEBUGS
	    debugprintf("dialtab_start: stored previous entry \"%s\"\n",
	        se.name) ;
#endif

	    if (rs >= 0) {
	        c += 1 ;
		f_ent = FALSE ;
	    }

	} else if (f_ent) {
	    entry_finish(&de) ;
	    f_ent = FALSE ;
	}

	if (rs < 0)
	    goto bad5 ;

/* done with configuration file processing */
ret1:
	bclose(sfp) ;

ret0:
	return (rs >= 0) ? c : rs ;

/* bad stuff */
bad5:
bad4:
	dialtab_filedump(op,fi) ;

	if (f_ent) {
	    entry_finish(&de) ;
	    f_ent = FALSE ;
	}

bad3:
	dialtab_filedel(op,fi) ;

bad2:
	file_finish(&fe) ;

bad1:
	bclose(sfp) ;

bad0:
	goto ret0 ;

}
/* end subroutine (dialtab_fileadd) */


/* free up the resources occupied by a DIALTAB list */
int dialtab_close(op)
DIALTAB		*op ;
{
	DIALTAB_ENT	*dep ;

	DIALTAB_FILE	*fep ;

	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DIALTAB_MAGIC)
	    return SR_NOTOPEN ;

/* free up the dial entries */

	for (i = 0 ; vecobj_get(&op->entries,i,&dep) >= 0 ; i += 1) {

	    if (dep == NULL) continue ;

	    entry_finish(dep) ;

	} /* end for */

	vecobj_finish(&op->entries) ;

/* free up the files */

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {

	    if (fep == NULL) continue ;

	    file_finish(fep) ;

	} /* end for */

	vecobj_finish(&op->files) ;

/* get out */

	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (dialtab_close) */


/* search the dial table for a name match */
int dialtab_search(op,name,depp)
DIALTAB		*op ;
const char	name[] ;
DIALTAB_ENT	**depp ;
{
	int	l, l1, l2, i ;

	const char	*sp, *cp ;


#if	CF_DEBUGS
	debugprintf("dialtab_search: ent name=%s\n",name) ;
#endif

	for (i = 0 ; vecobj_get(&op->entries,i,depp) >= 0 ; i += 1) {
	    if (*depp == NULL) continue ;

	    sp = (*depp)->name ;

#if	CF_DEBUGS
	    debugprintf("dialtab_search: got entry=\"%s\"\n",sp) ;
#endif

	    if (((cp = strchr(sp,'*')) != NULL) &&
	        (strchr(sp,'\\') == NULL)) {

#if	CF_DEBUGS
	        debugprintf("dialtab_search: got a star, cp=%s\n",cp) ;
#endif

	        if (strncmp(name,sp,cp - sp) == 0) {

#if	CF_DEBUGS
	            debugprintf("dialtab_search: leading matched up \n") ;
#endif

	            cp += 1 ;
	            l1 = strlen(name) ;

	            l2 = strlen(sp) ;

#if	CF_DEBUGS
	            debugprintf("dialtab_search: l1=%d l2=%d\n",l1,l2) ;
#endif

	            l = sp + l2 - cp ;

#if	CF_DEBUGS
	            debugprintf("dialtab_search: l=%d\n",l) ;
#endif

	            if (strncmp(name + l1 - l,cp,l) == 0)
	                return i ;

	        }

	    } else if (strcmp(name,sp) == 0)
	        return i ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("dialtab_search: did not match any entry\n") ;
#endif

	return -1 ;
}
/* end subroutine (dialtab_search) */


/* private subroutines */


static int dialtab_filedump(op,fi)
DIALTAB		*op ;
int		fi ;
{
	DIALTAB_ENT	*ep ;

	int	rs = SR_OK ;
	int	i ;


	for (i = 0 ; vecobj_get(&op->entries,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

	    if ((fi < 0) || (ep->fi == fi)) {
	        entry_finish(ep) ;
	        vecobj_del(&op->entries,i--) ;
	    }

	} /* end for */

	return rs ;
}
/* end subroutine (dialtab_filedump) */


static int dialtab_filedel(op,fi)
DIALTAB		*op ;
int		fi ;
{
	DIALTAB_FILE	*fep ;

	int	rs ;


	rs = vecobj_get(&op->files,fi,&fep) ;

	if ((rs >= 0) && (fep != NULL)) {
	    file_finish(fep) ;
	    rs = vecobj_del(&op->files,fi) ;
	}

	return rs ;
}
/* end subroutine (dialtab_filedel) */


static int file_start(fep,fname)
struct dialtab_file	*fep ;
const char	fname[] ;
{
	int	rs = SR_OK ;


	if ((fep == NULL) || (fname == NULL))
	    return SR_FAULT ;

	memset(fep,0,sizeof(struct dialtab_file)) ;

	fep->fname = mallocstr(fname) ;
	if (fep->fname == NULL)
	    rs = SR_NOMEM ;

	return rs ;
}
/* end subroutine (file_start) */


static int file_finish(fep)
struct dialtab_file	*fep ;
{


	if (fep == NULL)
	    return SR_FAULT ;

	if (fep->fname != NULL) {
	    uc_free(fep->fname) ;
	    fep->fname = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (file_finish) */


static int entry_start(dep,dname,nlen)
struct dialtab_ent	*dep ;
const char		dname[] ;
int			nlen ;
{
	int	rs = SR_OK ;


	if ((dep == NULL) || (dname == NULL))
	    return SR_FAULT ;

	memset(dep,0,sizeof(struct dialtab_ent)) ;

	dep->name = mallocstrw(dname,nlen) ;
	if (dep->name == NULL)
	    rs = SR_NOMEM ;

	return rs ;
}
/* end subroutine (entry_start) */


/* is there enough of a service entry to keep it? */
static int entry_enough(dep)
struct dialtab_ent	*dep ;
{


	if (dep == NULL)
	    return SR_FAULT ;

	if ((dep->name == NULL) || (dep->name[0] == '\0'))
	    return FALSE ;

	if ((dep->uucp != NULL) && (dep->uucp[0] != '\0'))
	    return TRUE ;

	if ((dep->inet != NULL) && (dep->inet[0] != '\0'))
	    return TRUE ;

	if ((dep->username != NULL) && (dep->username[0] != '\0'))
	    return TRUE ;

	if ((dep->password != NULL) && (dep->password[0] != '\0'))
	    return TRUE ;

	return FALSE ;
}
/* end subroutine (entry_enough) */


static int entry_finish(dep)
struct dialtab_ent	*dep ;
{


	if (dep == NULL)
	    return SR_FAULT ;

	if (dep->name == NULL)
	    return SR_OK ;

	freeit(&dep->name) ;

	freeit(&dep->uucp) ;

	freeit(&dep->inet) ;

	freeit(&dep->username) ;

	freeit(&dep->password) ;

	return SR_OK ;
}
/* end subroutine (entry_finish) */


static void freeit(pp)
const char	**pp ;
{

	if (*pp != NULL) {
	    uc_free(*pp) ;
	    *pp = NULL ;
	}

}
/* end subroutine (freeit) */


