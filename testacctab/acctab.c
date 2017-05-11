/* acctab */

/* perform access table file related functions */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_REGEX	0		/* stupid UNIX REGEX? (buggy) */


/* revision history:

	= 1998-06-01, David A�D� Morano
	This subroutine was originally written (and largely forgotten).

*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

        This object processes an access table for use by daemon multiplexing
        server programs that want to control access to their sub-servers.


*******************************************************************************/


#define	ACCTAB_MASTER	1


#include	<envstandards.h>

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
#include	<vecitem.h>
#include	<vecstr.h>
#include	<char.h>
#include	<localmisc.h>

#include	"acctab.h"


/* local defines */

#define	ACCTAB_MINCHECKTIME	5	/* file check interval (seconds) */
#define	ACCTAB_CHECKTIME	60	/* file check interval (seconds) */
#define	ACCTAB_CHANGETIME	3	/* wait change interval (seconds) */
#define	ACCTAB_DEFNETGROUP	"DEFAULT"
#define	ACCTAB_RGXLEN		256
#define	ACCTAB_FILE		struct acctab_file

#define	PARTTYPE_STD		0
#define	PARTTYPE_RGX		1
#define	PARTTYPE_UNKNOWN	2

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#undef	ACCTAB_REGEX
#define	ACCTAB_REGEX	CF_REGEX


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	starmat(const char *,const char *) ;
extern int	getpwd(char *,int) ;
extern int	isalnumlatin(int) ;


/* external variables */


/* local structures */

struct acctab_file {
	const char	*fname ;
	time_t		mtime ;
	dev_t		dev ;
	ino64_t		ino ;
	int		len ;
} ;


/* forward references */

int		acctab_fileadd(ACCTAB *,const char *,VECITEM *) ;

static int	acctab_filefins(ACCTAB *) ;
static int	acctab_freeentries(ACCTAB *) ;
static int	acctab_fileparse(ACCTAB *,int,VECITEM *) ;
static int	acctab_filedump(ACCTAB *,int) ;
static int	acctab_filedel(ACCTAB *,int) ;
static int	acctab_addentry() ;
static int	acctab_addentry(ACCTAB *,ACCTAB_ENT *) ;
static int	acctab_checkfiles(ACCTAB *,time_t,VECITEM *) ;

static int	entry_start(ACCTAB_ENT *) ;
static int	entry_load(ACCTAB_ENT *,cchar *,cchar *,cchar *,cchar *) ;
static int	entry_mat2(ACCTAB_ENT *,ACCTAB_ENT *) ;
static int	entry_mat3(ACCTAB_ENT *,ACCTAB_ENT *) ;
static int	entry_finish(ACCTAB_ENT *) ;

static int	file_start(ACCTAB_FILE *,const char *) ;
static int	file_release(ACCTAB_FILE *) ;
static int	file_finish(ACCTAB_FILE *) ;

static int	part_start(struct acctab_part *) ;
static int	part_copy(struct acctab_part *,struct acctab_part *) ;
static int	part_compile(struct acctab_part *,const char *,int) ;
static int	part_match(struct acctab_part *,const char *) ;
static int	part_finish(struct acctab_part *) ;

static int	parttype(const char *) ;
static int	cmpstdfunc() ;

#ifdef	COMMENT
static int	cmpfunc2(), cmpfunc3() ;
#endif

static void	freeit(const char **) ;


/* local static data */

/* key field terminators (pound, colon, and all white space) */

static const unsigned char 	group_terms[32] = {
	0x00, 0x1B, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

/* argument field terminators (pound and all white space) */

static const unsigned char 	arg_terms[32] = {
	0x00, 0x1B, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int acctab_open(ACCTAB *op,cchar *fname,VECITEM *eep)
{
	int		rs ;
	int		size ;
	int		o ;

#if	CF_DEBUGS
	debugprintf("acctab_open: fname=%s\n",fname) ;
#endif

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(ACCTAB)) ;
	op->checktime = time(NULL) ;

/* the vector of files should use a fixed position policy */

	size = sizeof(struct acctab_file) ;
	o = VECOBJ_PREUSE ;
	if ((rs = vecobj_start(&op->files,size,10,o)) >= 0) {
	    o = VECITEM_PSORTED ;
	    if ((rs = vecitem_start(&op->aes_std,10,o)) >= 0) {
		o = VECITEM_PNOHOLES ;
	        if ((rs = vecitem_start(&op->aes_rgx,10,o)) >= 0) {
	            op->magic = ACCTAB_MAGIC ;
	            if (fname != NULL) {
	                rs = acctab_fileadd(op,fname,eep) ;
	            }
		    if (rs < 0) {
	                op->magic = 0 ;
			vecitem_finish(&op->aes_rgx) ;
		    }
		} /* end if (rgx) */
	        if (rs < 0)
	            vecitem_finish(&op->aes_std) ;
	    } /* end if (std) */
	    if (rs < 0)
	        vecobj_finish(&op->files) ;
	} /* end if (files) */

	return rs ;
}
/* end subroutine (acctab_open) */


/* free up the resources occupied by an ACCTAB list object */
int acctab_close(op)
ACCTAB		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

/* secondary items */

	rs1 = acctab_freeentries(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = acctab_filefins(op) ;
	if (rs >= 0) rs = rs1 ;

/* primary items */

	rs1 = vecitem_finish(&op->aes_std) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecitem_finish(&op->aes_rgx) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (acctab_close) */


/* add a file to the list of files */
int acctab_fileadd(op,fname,eep)
ACCTAB		*op ;
const char	fname[] ;
VECITEM		*eep ;
{
	ACCTAB_FILE	fe ;
	int		rs = SR_OK ;
	int		fi ;
	const char	*np ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("acctab_fileadd: ent, file=%s\n",fname) ;
#endif

	np = fname ;
	if (fname[0] != '/') {
	    char	pwdbuf[MAXPATHLEN+1] ;
	    rs = getpwd(pwdbuf,MAXPATHLEN) ;
	    if (rs >= 0) {
	        np = tmpfname ;
		rs = mkpath2(tmpfname,pwdbuf,fname) ;
	    }
	} /* end if (rooting file) */

#if	CF_DEBUGS
	debugprintf("acctab_fileadd: np=%s\n",np) ;
#endif

	if (rs >= 0)
	    rs = file_start(&fe,np) ;

	if (rs < 0) goto bad0 ;

	rs = vecobj_add(&op->files,&fe) ;
	fi = rs ;
	if (rs < 0) goto bad1 ;

	file_release(&fe) ;

/* parse the file for the first time */

#if	CF_DEBUGS
	debugprintf("acctab_fileadd: acctab_fileparse()\n") ;
#endif

	rs = acctab_fileparse(op,fi,eep) ;
	if (rs < 0) goto bad2 ;

ret0:

#if	CF_DEBUGS
	debugprintf("acctab_fileadd: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* handle bad things */
bad2:
	acctab_filedel(op,fi) ;

bad1:
	file_finish(&fe) ;

bad0:
	goto ret0 ;
}
/* end subroutine (acctab_fileadd) */


/* search the netgroup table for a netgroup match */
int acctab_allowed(op,netgroup,machine,username,password)
ACCTAB		*op ;
const char	netgroup[], machine[] ;
const char	username[], password[] ;
{
	ACCTAB_ENT	ae, *aep ;
	VECITEM		*slp ;
	VECITEM_CUR	cur ;
	time_t		daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

	if (machine == NULL) return SR_INVALID ;

/* should we check for changes to underlying file? */

	if ((daytime - op->checktime) > ACCTAB_MINCHECKTIME) {

	    rs = acctab_checkfiles(op,daytime,NULL) ;
	    if (rs < 0)
	        goto ret0 ;

	} /* end if (we needed to check) */

/* load up a fake entry for comparison purposes */

	entry_start(&ae) ;

	entry_load(&ae,netgroup,machine,username,password) ;

/* search the STD entries first */

	slp = &op->aes_std ;

	vecitem_curbegin(slp,&cur) ;

	while ((rs = vecitem_fetch(slp,&ae,&cur,cmpstdfunc,&aep)) >= 0) {

	    if (aep == NULL) continue ;

	    f = entry_mat2(aep,&ae) ;
	    if (f)
	        break ;

	} /* end while */

	vecitem_curend(slp,&cur) ;

/* search the RGX entries (if necessary) */

	if ((rs < 0) || (! f)) {
	    slp = &op->aes_rgx ;

	    for (i = 0 ; vecitem_get(slp,i,&aep) >= 0 ; i += 1) {
	        if (aep == NULL) continue ;

	        f = entry_mat3(aep,&ae) ;
		if (f)
	            break ;

	    } /* end for (looping through entries) */

	} /* end if (comparing RGX entries) */

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (acctab_allowed) */


/* search the netgroup table for any netgroup match */
int acctab_anyallowed(op,ngp,mnp,username,password)
ACCTAB		*op ;
vecstr		*ngp, *mnp ;
const char	username[], password[] ;
{
	int		rs = SR_OK ;
	int		i, j ;
	int		f = FALSE ;
	const char	*netgroup ;
	const char	*machine ;

#if	CF_DEBUGS
	debugprintf("acctab_anyallowed: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; (rs = vecstr_get(ngp,i,&netgroup)) >= 0 ; i += 1) {
	    if (netgroup == NULL) continue ;

	    for (j = 0 ; (rs = vecstr_get(mnp,j,&machine)) >= 0 ; j += 1) {
	        if (machine == NULL) continue ;

	        rs = acctab_allowed(op,netgroup,machine,username,password) ;
	        if (rs >= 0) {
	            f = TRUE ;
		    break ;
		}

	    } /* end for */

	    if (f) break ;
	} /* end for (looping over netgroups) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (acctab_anyallowed) */


#ifdef	COMMENT

/* search the netgroup table for a netgroup match */
int acctab_find(op,netgroup,sepp)
ACCTAB		*op ;
const char	netgroup[] ;
ACCTAB_ENT	**sepp ;
{
	VECITEM		*slp ;
	int		i ;
	const char	*sp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("acctab_find: ent, netgroup=%s\n",netgroup) ;
#endif

	slp = &op->e ;
	for (i = 0 ; vecitem_get(slp,i,sepp) >= 0 ; i += 1) {
	    if (*sepp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("acctab_find: loop i=%d\n",i) ;
#endif

	    sp = (*sepp)->netgroup ;

#if	CF_DEBUGS
	    debugprintf("acctab_find: got entry=\"%s\"\n",sp) ;
#endif

	    if (strcmp(netgroup,sp) == 0)
	        return i ;

	} /* end for (looping through entries) */

#if	CF_DEBUGS
	debugprintf("acctab_find: did not match any entry\n") ;
#endif

	return -1 ;
}
/* end subroutine (acctab_find) */

#endif /* COMMENT */


/* cursor manipulations */
int acctab_curbegin(op,cp)
ACCTAB		*op ;
ACCTAB_CUR	*cp ;
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

	cp->i = cp->j = -1 ;
	return SR_OK ;
}
/* end subroutine (acctab_curbegin) */


int acctab_curend(op,cp)
ACCTAB		*op ;
ACCTAB_CUR	*cp ;
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

	cp->i = cp->j = -1 ;
	return SR_OK ;
}
/* end subroutine (acctab_curend) */


/* enumerate the netgroup entries */
int acctab_enum(op,cp,sepp)
ACCTAB		*op ;
ACCTAB_CUR	*cp ;
ACCTAB_ENT	**sepp ;
{
	VECITEM		*slp ;
	ACCTAB_ENT	*aep ;
	int		rs = SR_OK ;
	int		j ;


	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("acctab_enum: ent, i=%d\n",cp->i) ;
#endif

	if (sepp == NULL)
	    sepp = &aep ;

	rs = SR_NOTFOUND ;
	if (cp->i <= 0) {

#if	CF_DEBUGS
	    debugprintf("acctab_enum: STD entries\n") ;
#endif

	    slp = &op->aes_std ;
	    cp->j = (cp->j < 0) ? 0 : (cp->j + 1) ;
	    for (j = cp->j ; (rs = vecitem_get(slp,j,sepp)) >= 0 ; j += 1) {

	        if (*sepp != NULL)
	            break ;

	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("acctab_enum: vecitem_get rs=%d j=%d\n",rs,j) ;
#endif

	    if (rs < 0) {

	        cp->j = -1 ;
	        cp->i = 1 ;

	    } else {

#if	CF_DEBUGS
	        debugprintf("acctab_enum: STD n=%s m=%s u=%s p=%s\n",
	            (*sepp)->netgroup.std,
	            (*sepp)->machine.std,
	            (*sepp)->username.std,
	            (*sepp)->password.std) ;
#endif

	        cp->i = 0 ;
	        cp->j = j ;

	    } /* end if */

	} /* end if (cursor needed initialization) */

#if	CF_DEBUGS
	debugprintf("acctab_enum: intermediate rs=%d cur_i=%d cur_j=%d\n",
	    rs,cp->i,cp->j) ;
#endif

	if (cp->i == 1) {

#if	CF_DEBUGS
	    debugprintf("acctab_enum: RGX entries\n") ;
#endif

	    slp = &op->aes_rgx ;
	    cp->j = (cp->j < 0) ? 0 : cp->j + 1 ;
	    for (j = cp->j ; (rs = vecitem_get(slp,j,sepp)) >= 0 ; j += 1) {

	        if (*sepp != NULL)
	            break ;

	    } /* end for */

	    if (rs < 0) {

	        cp->j = -1 ;
	        cp->i += 1 ;

	    } else {

#if	CF_DEBUGS
	        debugprintf("acctab_enum: RGX n=%s m=%s u=%s p=%s\n",
	            (*sepp)->netgroup.rgx,
	            (*sepp)->machine.rgx,
	            (*sepp)->username.rgx,
	            (*sepp)->password.rgx) ;
#endif

	        cp->j = j ;

	    } /* end if */

	} /* end if (RGX entries) */

#if	CF_DEBUGS
	debugprintf("acctab_enum: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (acctab_enum) */


/* check if the access tables files have changed */
int acctab_check(op,eep)
ACCTAB		*op ;
VECITEM		*eep ;
{
	const time_t	daytime = time(NULL) ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ACCTAB_MAGIC) return SR_NOTOPEN ;

/* should we even check? */

	if ((daytime - op->checktime) > ACCTAB_CHECKTIME) {
	rs = acctab_checkfiles(op,daytime,eep) ;
	}

#if	CF_DEBUGS
	debugprintf("acctab_check: ret rs=%d\n", rs) ;
#endif

	return rs ;
}
/* end subroutine (acctab_check) */


/* private subroutines */


/* free up all of the files in this ACCTAB list */
static int acctab_filefins(op)
ACCTAB		*op ;
{
	ACCTAB_FILE	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

	    rs1 = file_finish(fep) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	return rs ;
}
/* end subroutine (acctab_filefins) */


/* check if the access table files have changed */
static int acctab_checkfiles(op,daytime,eep)
ACCTAB		*op ;
time_t		daytime ;
VECITEM		*eep ;
{
	struct ustat	sb ;
	ACCTAB_FILE	*fep ;
	int		rs = SR_OK ;
	int		i ;
	int		c_changed = FALSE ;

/* check the files */

#if	CF_DEBUGS
	debugprintf("acctab_checkfiles: about to loop\n",i) ;
#endif

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

	    if ((u_stat(fep->fname,&sb) >= 0) &&
	        (sb.st_mtime > fep->mtime) &&
	        ((daytime - sb.st_mtime) >= ACCTAB_CHANGETIME)) {

#if	CF_DEBUGS
	        debugprintf("acctab_checkfiles: file=%d changed\n",i) ;
	        debugprintf("acctab_checkfiles: freeing file entries\n") ;
#endif

	        acctab_filedump(op,i) ;

#if	CF_DEBUGS
	        debugprintf("acctab_checkfiles: parsing the file again\n") ;
#endif

	        rs = acctab_fileparse(op,i,eep) ;

		if (rs >= 0)
	            c_changed += 1 ;

#if	CF_DEBUGS
	        debugprintf("acctab_checkfiles: acctab_fileparse rs=%d\n",rs) ;
#endif

	    } /* end if */

	} /* end for */

	if ((rs >= 0) && c_changed) {

#if	CF_DEBUGS
	    debugprintf("acctab_checkfiles: sorting STD entries\n") ;
#endif

	    rs = vecitem_sort(&op->aes_std,cmpstdfunc) ;

	} /* end if (something changed) */

	op->checktime = daytime ;

#if	CF_DEBUGS
	debugprintf("acctab_checkfiles: exiting rs=%d changed=%d\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? c_changed : rs ;
}
/* end subroutine (acctab_checkfiles) */


/* parse an access server file */
static int acctab_fileparse(op,fi,eep)
ACCTAB		*op ;
int		fi ;
VECITEM		*eep ;
{
	struct acctab_part	netgroup ;
	bfile		file, *fp = &file ;
	ACCTAB_FILE	*fep ;
	ACCTAB_ENT	se ;
	struct ustat	sb ;
	FIELD		fsb ;
	int		rs ;
	int		c ;
	int		cl, len ;
	int		line = 0 ;
	int		c_added = 0 ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("acctab_fileparse: ent fi=%u\n",fi) ;
#endif

	rs = vecobj_get(&op->files,fi,&fep) ;

#if	CF_DEBUGS
	debugprintf("acctab_fileparse: vecobj_get() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	rs = bopen(fp,fep->fname,"r",0664) ;

#if	CF_DEBUGS
	debugprintf("acctab_fileparse: bopen() rs=%d\n", rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	rs = bcontrol(fp,BC_STAT,&sb) ;
	if (rs < 0)
	    goto done ;

/* have we already parsed this one? */

#if	CF_DEBUGS
	debugprintf("acctab_fileparse: 4\n") ;
#endif

	rs = SR_OK ;
	if (fep->mtime >= sb.st_mtime)
	    goto done ;

#if	CF_DEBUGS
	debugprintf("acctab_fileparse: 5\n") ;
#endif

	fep->dev = sb.st_dev ;
	fep->ino = sb.st_ino ;
	fep->mtime = sb.st_mtime ;
	fep->len = sb.st_size ;

/* start processing the configuration file */

#if	CF_DEBUGS
	debugprintf("acctab_fileparse: start processing\n") ;
#endif

	part_compile(&netgroup,ACCTAB_DEFNETGROUP,-1) ;

	c_added = 0 ;
	while ((rs = breadline(fp,lbuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    line += 1 ;
	    if (len == 1) continue ;	/* blank line */

	    if (lbuf[len - 1] != '\n') {
	        while ((c = bgetc(fp)) >= 0) {
	            if (c == '\n') break ;
		}
	        continue ;
	    }

	    cp = lbuf ;
	    cl = len ;
	    lbuf[--cl] = '\0' ;
	    while (CHAR_ISWHITE(*cp)) {
	        cp += 1 ;
	        cl -= 1 ;
	    }

	    if ((*cp == '\0') || (*cp == '#')) continue ;

	    if ((rs = field_start(&fsb,cp,cl)) >= 0) {
	int	fl ;
	const char	*fp ;

	    if ((fl = field_get(&fsb,group_terms,&fp)) > 0) {

/* we have something! */

	    entry_start(&se) ;

	    se.fi = fi ;
	    if (fsb.term == ':') {

#if	CF_DEBUGS
	        debugprintf("acctab_fileparse: new netgroup\n") ;
#endif

	        part_finish(&netgroup) ;

	        rs = part_compile(&netgroup,fp,fl) ;

	        fl = field_get(&fsb,arg_terms,&fp) ;

	    } /* end if */

/* see if there is a machine on this same line */

	    if (fl > 0) {

#if	CF_DEBUGS
	        debugprintf("acctab_fileparse: non-empty line\n") ;
#endif

	        part_copy(&se.netgroup,&netgroup) ;

/* there is something else on this line */

#if	CF_DEBUGS
	        debugprintf("acctab_fileparse: machine> %t\n",
	            fp,fl) ;
#endif

	        rs = part_compile(&se.machine,fp,fl) ;

	        if (rs == SR_NOMEM)
	            goto badpart ;

	        if ((fl = field_get(&fsb,arg_terms,&fp)) > 0) {

	            rs = part_compile(&se.username,fp,fl) ;

	            if (rs == SR_NOMEM)
	                goto badpart ;

	        }

	        if ((fl = field_get(&fsb,arg_terms,&fp)) > 0) {

	            rs = part_compile(&se.password,fp,fl) ;

	            if (rs == SR_NOMEM)
	                break ;

	        }

#if	CF_DEBUGS
	        debugprintf("acctab_fileparse: adding so far rs=%d\n",rs) ;
	        debugprintf("acctab_fileparse: n=%s m=%s\n",
	            se.netgroup.std,
	            se.machine.std) ;
#endif

	        rs = acctab_addentry(op,&se) ;

#if	CF_DEBUGS
	        debugprintf("acctab_fileparse: added entry?, rs=%d\n",rs) ;
#endif

	        if (rs >= 0)
	            c_added += 1 ;

	    } /* end if */

	    } /* end if (got something) */

	    field_finish(&fsb) ;
	    } /* end if (field) */

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	part_finish(&netgroup) ;

	if (rs < 0)
		goto bad4 ;

/* done with configuration file processing */
done:
	bclose(fp) ;

ret0:

#if	CF_DEBUGS
	debugprintf("acctab_fileparse: ret rs=%d added=%d\n",
	    rs,c_added) ;
#endif

	return (rs >= 0) ? c_added : rs ;

/* handle bad things */
bad4:
	acctab_filedump(op,fi) ;

badpart:
	part_finish(&netgroup) ;

	entry_finish(&se) ;

	goto done ;
}
/* end subroutine (acctab_fileparse) */


/* add an entry to the access entry list */
static int acctab_addentry(op,sep)
ACCTAB		*op ;
ACCTAB_ENT	*sep ;
{
	int		rs ;

	if (parttype(sep->netgroup.std) == PARTTYPE_STD) {

#if	CF_DEBUGS
	    debugprintf("acctab_addentry: has plain s=%s\n",sep->netgroup.std) ;
#endif

	    rs = vecitem_add(&op->aes_std,sep,sizeof(ACCTAB_ENT)) ;

	} else
	    rs = vecitem_add(&op->aes_rgx,sep,sizeof(ACCTAB_ENT)) ;

	return rs ;
}
/* end subroutine (acctab_addentry) */


/* free up all of the entries in this ACCTAB list associated w/ a file */
static int acctab_filedump(op,fi)
ACCTAB		*op ;
int		fi ;
{
	ACCTAB_ENT	*sep ;
	VECITEM		*slp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i, j ;

#if	CF_DEBUGS
	debugprintf("acctab_filedump: want to delete all fi=%d\n",fi) ;
#endif

	for (j = 0 ; j < 2 ; j += 1) {

	    slp = (j == 0) ? &op->aes_std : &op->aes_rgx ;
	    for (i = 0 ; (rs = vecitem_get(slp,i,&sep)) >= 0 ; i += 1) {
	        if (sep == NULL) continue ;

#if	CF_DEBUGS
	        debugprintf("acctab_filedump: i=%d fi=%d\n",i,sep->fi) ;
#endif

	        if ((sep->fi == fi) || (fi < 0)) {

#if	CF_DEBUGS
	            debugprintf("acctab_filedump: got one\n") ;
#endif

	            rs1 = entry_finish(sep) ;
		    if (rs >= 0) rs = rs1 ;

	            rs1 = vecitem_del(slp,i--) ;
		    if (rs >= 0) rs = rs1 ;

	        } /* end if (found matching entry) */

	    } /* end for (looping through entries) */

#if	CF_DEBUGS
	    debugprintf("acctab_filedump: popped up to, rs=%d i=%d\n",rs,i) ;
#endif

	} /* end for (looping on the different types of entries) */

#if	CF_DEBUGS
	debugprintf("acctab_filedump: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (acctab_filedump) */


static int acctab_filedel(op,fi)
ACCTAB		*op ;
int		fi ;
{
	ACCTAB_FILE	*fep ;
	int		rs ;

	rs = vecobj_get(&op->files,fi,&fep) ;

	if ((rs >= 0) && (fep != NULL)) {

		file_finish(fep) ;

		rs = vecobj_del(&op->files,fi) ;

	}

	return rs ;
}
/* end subroutine (acctab_filedel) */


/* free up all of the entries in this ACCTAB list */
static int acctab_freeentries(op)
ACCTAB		*op ;
{
	ACCTAB_ENT	*sep ;
	VECITEM		*slp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i, j ;

	for (j = 0 ; j < 2 ; j += 1) {
	    slp = (j == 0) ? &op->aes_std : &op->aes_rgx ;
	    for (i = 0 ; vecitem_get(slp,i,&sep) >= 0 ; i += 1) {
	        if (sep == NULL) continue ;

	        rs1 = entry_finish(sep) ;
		if (rs >= 0) rs = rs1 ;

	    } /* end for */
	} /* end for */

	return rs ;
}
/* end subroutine (acctab_freeentries) */


static int file_start(fep,fname)
ACCTAB_FILE	*fep ;
const char	fname[] ;
{
	int		rs = SR_OK ;
	const char	*cp ;

	if (fname == NULL) return SR_FAULT ;

	memset(fep,0,sizeof(ACCTAB_FILE)) ;

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    fep->fname = cp ;
	}

	return rs ;
}
/* end subroutine (file_start) */


static int file_release(fep)
ACCTAB_FILE	*fep ;
{

	fep->fname = NULL ;
	return SR_OK ;
}
/* end subroutine (file_release) */


static int file_finish(fep)
ACCTAB_FILE	*fep ;
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


/* initialize an entry */
static int entry_start(sep)
ACCTAB_ENT	*sep ;
{
	int		rs = SR_OK ;

	memset(sep,0,sizeof(ACCTAB_ENT)) ;
	sep->fi = -1 ;

	part_start(&sep->netgroup) ;

	part_start(&sep->machine) ;

	part_start(&sep->username) ;

	part_start(&sep->password) ;

	return rs ;
}
/* end subroutine (entry_start) */


/* free up an entry */
static int entry_finish(sep)
ACCTAB_ENT	*sep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("acctab/entry_finish: ent\n") ;
#endif

	if (sep->fi >= 0) {
	    rs1 = part_finish(&sep->netgroup) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = part_finish(&sep->machine) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = part_finish(&sep->username) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = part_finish(&sep->password) ;
	    if (rs >= 0) rs = rs1 ;
	    sep->fi = -1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_load(aep,netgroup,machine,username,password)
ACCTAB_ENT	*aep ;
const char	netgroup[] ;
const char	machine[] ;
const char	username[] ;
const char	password[] ;
{

	aep->netgroup.std = (char *) netgroup ;
	if ((netgroup == NULL) || (netgroup[0] == '\0'))
	    aep->netgroup.std = ACCTAB_DEFNETGROUP ;

	aep->machine.std = (char *) machine ;
	aep->username.std = (char *) username ;
	aep->password.std = (char *) password ;

	return SR_OK ;
}
/* end subroutine (entry_load) */


/* compare if all but the netgroup are equal (or matched) */
static int entry_mat2(e1p,e2p)
ACCTAB_ENT	*e1p, *e2p ;
{

#if	CF_DEBUGS
	debugprintf("acctab/entry_mat2: ent\n") ;
	debugprintf("acctab/entry_mat2: m=%s u=%s p=%s\n",
	    e2p->machine.std,
	    e2p->username.std,
	    e2p->password.std) ;
#endif

#ifdef	OPTIONAL
	if (! part_match(&e1p->netgroup,e2p->netgroup.std))
	    return FALSE ;
#endif

#if	CF_DEBUGS
	debugprintf("acctab/entry_mat2: machine?\n") ;
#endif

	if (! part_match(&e1p->machine,e2p->machine.std))
	    return FALSE ;

#if	CF_DEBUGS
	debugprintf("acctab/entry_mat2: user?\n") ;
#endif

	if (! part_match(&e1p->username,e2p->username.std))
	    return FALSE ;

#if	CF_DEBUGS
	debugprintf("acctab/entry_mat2: password?\n") ;
#endif

	if (! part_match(&e1p->password,e2p->password.std))
	    return FALSE ;

#if	CF_DEBUGS
	debugprintf("acctab/entry_mat2: succeeded!\n") ;
#endif

	return TRUE ;
}
/* end subroutine (entry_mat2) */


/* compare if all of the entry is equal (or matched) */
static int entry_mat3(e1p,e2p)
ACCTAB_ENT	*e1p, *e2p ;
{

	if (! part_match(&e1p->netgroup,e2p->netgroup.std))
	    return FALSE ;

	if (! part_match(&e1p->machine,e2p->machine.std))
	    return FALSE ;

	if (! part_match(&e1p->username,e2p->username.std))
	    return FALSE ;

	if (! part_match(&e1p->password,e2p->password.std))
	    return FALSE ;

	return TRUE ;
}
/* end subroutine (entry_mat3) */


static int part_start(pp)
struct acctab_part	*pp ;
{

	pp->type = 0 ;
	pp->std = NULL ;
	pp->rgx = NULL ;
	return SR_OK ;
}
/* end subroutine (part_start) */


/* p1 gets loaded up (from p2) */
static int part_copy(p1p,p2p)
struct acctab_part	*p1p, *p2p ;
{
	int		rs = SR_OK ;
	const char	*cp ;

	memset(p1p,0,sizeof(struct acctab_part)) ;

	p1p->type = p2p->type ;
	if (p2p->std != NULL) {
	    if ((rs = uc_mallocstrw(p2p->std,-1,&cp)) >= 0) {
	        p1p->std = cp ;
	    }
	} /* end if (copied STD) */

#if	ACCTAB_REGEX

#if	CF_DEBUGS
	debugprintf("acctab/part_copy: >%t<\n",
	    p2p->rgx,
	    ((p2p->rgx != NULL) ? strnlen(p2p->rgx,10) : 0)) ;
#endif

	if (rs >= 0) {
	if (p2p->rgx != NULL) {
	    if ((rs = uc_mallocstrw(p2p->rgx,p2p->rgxlen,&cp)) >= 0) {
	        p1p->rgx = cp ;
	        p1p->rgxlen = p2p->rgxlen ;
	    }
	} /* end if (copied RGX) */
	} /* end if (ok) */

	if (rs < 0) {
	    if (p1p->std != NULL) {
	        uc_free(p1p->std) ;
	        p1p->std = NULL ;
	    }
	}

#endif	/* ACCTAB_REGEX */

#if	CF_DEBUGS
	debugprintf("acctab/part_copy: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (part_copy) */


/* compile this part */
static int part_compile(pp,sp,sl)
struct acctab_part	*pp ;
const char	sp[] ;
int		sl ;
{
	int		rs = SR_OK ;
	const char	*cp ;

	pp->type = 0 ;
	pp->std = NULL ;
	pp->rgx = NULL ;
	pp->rgxlen = 0 ;

	rs = uc_mallocstrw(sp,sl,&cp) ;
	if (rs >= 0) pp->std = cp ;

#if	ACCTAB_REGEX
	if ((rs >= 0) && ((type = parttype(pp->std)) > PARTTYPE_STD)) {
	    char	*rgxbuf ;
	    char	*rp ;

	    if ((rs = uc_malloc(ACCTAB_RGXLEN,&rgxbuf)) >= 0) {

	        rp = compile(pp->std,rgxbuf,(rgxbuf + ACCTAB_RGXLEN)) ;

	        if (rp != NULL) {

	            pp->rgx = rp ;
	            pp->rgxlen = ACCTAB_RGXLEN ;
	            pp->type = type ;

	        } else {

	            rs = (regerrno == 0) ? SR_NOMEM : SR_INVALID ;
	            uc_free(rgxbuf) ;

	        }

	    } /* end if (got memory) */

	} /* end if (RGX indicated) */
#endif /* ACCTAB_REGEX */

	return rs ;
}
/* end subroutine (part_compile) */


/* see if we have a match on a part */
static int part_match(pp,s)
struct acctab_part	*pp ;
const char		s[] ;
{
	int		sl, sl1, sl2 ;
	int		f = FALSE ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("acctab/part_match: ent, type=%d s=%s\n",
	    pp->type,s) ;
#endif

	if (pp->std == NULL) return TRUE ;

	if (s == NULL)
	    return FALSE ;

	if (pp->rgx == NULL) {

#if	CF_DEBUGS
	    debugprintf("acctab/part_match: plain, s1=%s s2=%s\n",
		pp->std,s) ;
#endif

	    f = (strcmp(pp->std,s) == 0) ;
	    if (! f)
	        f = starmat(pp->std,s) ;

	} else {

	    if (pp->type == 1) {

#if	CF_DEBUGS
	        debugprintf("acctab/part_match: type 1, s1=%s s2=%s\n",
			pp->std,s) ;
#endif

	        if ((cp = strchr(pp->std,'*')) != NULL) {

#if	CF_DEBUGS
	            debugprintf("acctab/part_match: got a star, cp=%s\n",cp) ;
#endif

	            f = FALSE ;
	            if (strncmp(s,pp->std,(cp - pp->std)) == 0) {

#if	CF_DEBUGS
	                debugprintf("acctab/part_match: leading matched\n") ;
#endif

	                cp += 1 ;
	                sl1 = strlen(s) ;

	                sl2 = strlen(pp->std) ;

#if	CF_DEBUGS
	                debugprintf("acctab/part_match: sl1=%d sl2=%d\n",
				sl1,sl2) ;
#endif

	                sl = pp->std + sl2 - cp ;

#if	CF_DEBUGS
	                debugprintf("acctab/part_match: sl=%d\n",sl) ;
#endif

	                f = (strncmp(s + sl1 - sl,cp,sl) == 0) ;

	            } /* end if */

	        } else
	            f = (strcmp(pp->std,s) == 0) ;

	    } else {

#if	CF_DEBUGS
	        debugprintf("acctab/part_match: type 2, s1=%s s2=%s\n",
			pp->std,s) ;
#endif

#if	ACCTAB_REGEX
	        f = advance(pp->rgx,s) ;
#else
	        f = FALSE ;
#endif

	        if (! f)
	            f = starmat(pp->std,s) ;

	    } /* end if */

	} /* end if (STD or RGX) */

#if	CF_DEBUGS
	debugprintf("acctab/part_match: ret f=%u\n",f) ;
#endif

	return f ;
}
/* end subroutine (part_match) */


static int part_finish(pp)
struct acctab_part	*pp ;
{

	freeit(&pp->std) ;

	freeit(&pp->rgx) ;

	pp->rgxlen = 0 ;
	return 0 ;
}
/* end subroutine (part_finish) */


static void freeit(pp)
const char	**pp ;
{
	if (*pp != NULL) {
	    uc_free(*pp) ;
	    *pp = NULL ;
	}
}
/* end subroutine (freeit) */


/* compare just the 'netgroup' part of entries (used for sorting) */
static int cmpstdfunc(e1pp,e2pp)
ACCTAB_ENT	**e1pp, **e2pp ;
{

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	return strcmp((*e1pp)->netgroup.std,(*e2pp)->netgroup.std) ;
}
/* end subroutine (cmpstdfunc) */


#ifdef	COMMENT

/* compare the whole entries (except for the netgroup) */
static int cmpfunc2(e1pp,e2pp)
ACCTAB_ENT	**e1pp, **e2pp ;
{
	ACCTAB_ENT	*uep, *dbep ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

/* which one is the one from the DB? */

	if ((*e1pp)->fi >= 0) {

	    dbep = *e1pp ;
	    uep = *e2pp ;

	} else {

	    dbep = *e2pp ;
	    uep = *e1pp ;

	}

	return (entry_mat2(dbep,uep) == 0) ;
}
/* end subroutine (cmpfunc2) */


/* compare the whole entries (including the netgroup) */
static int cmpfunc3(e1pp,e2pp)
ACCTAB_ENT	**e1pp, **e2pp ;
{
	ACCTAB_ENT	*uep, *dbep ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

/* which one is the one from the DB? */

	if ((*e1pp)->fi >= 0) {

	    dbep = *e1pp ;
	    uep = *e2pp ;

	} else {

	    dbep = *e2pp ;
	    uep = *e1pp ;

	}

	return (entry_mat3(dbep,uep) == 0) ;
}
/* end subroutine (cmpfunc3) */

#endif /* COMMENT */


/* determine the type of this string part */
static int parttype(cchar *s)
{
	int		type = PARTTYPE_STD ;
	int		f = FALSE ;

	for ( ; *s ; s += 1) {
	    const int	ch = MKCHAR(*s) ;
	    f = f || isalnumlatin(ch) ;
	    f = f || (*s == '-') ;
	    f = f || (*s == '_') ;
	    if ((! f) && (*s == '*')) {
	        type = PARTTYPE_RGX ;
	        f = TRUE ;
	    }
	    if (! f) break ;
	} /* end for */

	return (*s == '\0') ? type : PARTTYPE_UNKNOWN ;
}
/* end subroutine (parttype) */

