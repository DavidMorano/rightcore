/* hostsfile */

/* perform access table file related functions */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFIELD	0


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written (and largely forgotten).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object processes INET 'hosts' files for use by daemon multiplexing
        server programs that want to control access to their sub-servers.


*******************************************************************************/


#define	HOSTSFILE_MASTER	1


#include	<envstandards.h>

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
#include	<vecitem.h>
#include	<vecstr.h>
#include	<cthex.h>
#include	<char.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"hostsfile.h"
#include	"strtab.h"
#include	"rectab.h"


/* local defines */

#define	HOSTSFILE_MINCHECKTIME	5	/* file check interval (seconds) */
#define	HOSTSFILE_CHECKTIME	360	/* file check interval (seconds) */
#define	HOSTSFILE_CHANGETIME	2	/* wait change interval (seconds) */

#define	LINELEN			200
#define	RECTAB			VECITEM


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

int		hostsfile_fileadd(HOSTSFILE *,char *,VECITEM *) ;

static int	hostsfile_finishfiles() ;
static int	hostsfile_finishentries() ;
static int	hostsfile_parsefile(HOSTSFILE *,int,VECITEM *) ;
static int	hostsfile_addentry() ;
static int	hostsfile_finishfes() ;
static int	hostsfile_checkfiles(HOSTSFILE *,time_t,VECITEM *) ;

static int	entry_start(HOSTSFILE_ENT *) ;
static int	entry_mat2(HOSTSFILE_ENT *,HOSTSFILE_ENT *) ;
static int	entry_mat3(HOSTSFILE_ENT *,HOSTSFILE_ENT *) ;
static int	entry_finish(HOSTSFILE_ENT *) ;

static int	cmpstdfunc() ;

#ifdef	COMMENT
static int	cmpfunc2(), cmpfunc3() ;
#endif

static void	errline(VECITEM *,char *,int) ;
static void	freeit(const char **) ;


/* local variables */

/* argument field terminators (pound and all white space) */
static const unsigned char 	arg_terms[32] = {
	0x00, 0x1B, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
} ;


/* exported subroutines */


int hostsfile_start(atp,cdir,atfname,eep)
HOSTSFILE	*atp ;
char		cdir[] ;
char		atfname[] ;
VECITEM		*eep ;
{
	struct ustat	sb ;
	time_t		daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		f_defcache = FALSE ;

#if	CF_DEBUGS
	debugprintf("hostsfile_start: ent %s\n",atfname) ;
#endif

	if (atp == NULL)
	    return SR_FAULT ;

	atp->magic = 0 ;

/* initialize */

	if ((rs = vecitem_start(&atp->files,10,VECITEM_PNOHOLES)) < 0)
	    goto bad1 ;

	if ((rs = vecitem_start(&atp->aes_std,10,VECITEM_PSORTED)) < 0)
	    goto bad2 ;

	if ((rs = vecitem_start(&atp->aes_rgx,10,VECITEM_PNOHOLES)) < 0)
	    goto bad3 ;

/* store the cache directory */

	if (cdir == NULL) {
		f_defcache = TRUE ;
		cdir = HOSTSFILE_CACHEDIR ;
	}

/* try to make the last component of the cache directory */

	if ((rs = u_stat(cdir,&sb)) < 0) {

	    rs = u_mkdir(cdir,0777) ;

	    if ((rs < 0) && (! f_defcache)) {
		f_defcache = TRUE ;
		cdir = HOSTSFILE_CACHEDIR ;
	        if ((rs = u_stat(cdir,&sb)) < 0)
		    rs = u_mkdir(cdir,0777) ;
	    }

	} /* end if (tried to make cache directory) */

	u_chmod(cdir,0777) ;

	if (rs < 0)
		goto bad4 ;

	atp->cdir = mallocstr(cdir) ;

	if (atp->cdir == NULL) {
		rs = SR_NOMEM ;
		goto bad5 ;
	}

	atp->checktime = time(NULL) ;

	if (atfname != NULL) {

	    rs = hostsfile_fileadd(atp,atfname,eep) ;

		if (rs < 0)
			goto badparse ;

	}

	atp->magic = HOSTSFILE_MAGIC ;
	return rs ;

/* handle bad things */
badparse:
	uc_free(atp->cdir) ;

bad5:

bad4:
	vecitem_finish(&atp->aes_rgx) ;

bad3:
	vecitem_finish(&atp->aes_std) ;

bad2:
	vecitem_finish(&atp->files) ;

bad1:
	return rs ;
}
/* end subroutine (hostsfile_start) */


/* add a file to the list of files */
int hostsfile_fileadd(atp,atfname,eep)
HOSTSFILE	*atp ;
char		atfname[] ;
VECITEM		*eep ;
{
	HOSTSFILE_FILE	fe ;
	const int	clen = CACHENAMELEN ;
	int		rs ;
	int		fi ;
	const char	*cp ;
	char		hostsfname[MAXPATHLEN + 1], *hnp ;
	char		cachefname[MAXPATHLEN + 1] ;
	char		cachename[CACHENAMELEN + 1] ;

	if (atp == NULL) return SR_FAULT ;
	if (atfname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("hostsfile_fileadd: ent, file=%s\n",atfname) ;
#endif

	hnp = atfname ;
	if (atfname[0] != '/') {

		rs = getpwd(hostsfname,MAXPATHLEN) ;

		if (rs < 0)
			return rs ;

		if ((rs + 1) < MAXPATHLEN) {
			hostfname[rs++] = '/' ;
			hostfname[rs] = '\0' ;
		}

		if ((rs + 1) < MAXPATHLEN)
		strwcpy((hostsfname + rs),atfname,(MAXPATHLEN - rs)) ;

		hnp = hostsfname ;

	} /* end if (constructing full path) */

/* store the filename away for later use (polling) */

	if ((fe.hostsfname = mallocstr(hnp)) == NULL) {
	    rs = SR_NOMEM ;
	    goto badmal0 ;
	}

#ifdef	MALLOCLOG
	malloclog_alloc(fe.hostsfname,-1,"hostsfile_fileadd:filename") ;
#endif

/* calculate the cache name */

	rs = hashelf(hnp,-1) ;

	strcpy(cachename,"hc") ;

	cthexi((cachename+2),clen,CACHENAMELEN) ;

	cachename[10] = '\0' ;

/* calculate the cache filepath name */

	cp = strwcpy(cachefname,atp->cachedir,MAXPATHLEN) ;

	if ((cp - cachefname + 1) < MAXPATHLEN) {
		*cp++ = '/' ;
		*cp = '\0' ;
	}

	if ((cp - cachefname + 1) < MAXPATHLEN)
		strwcpy(cp,cachename,(MAXPATHLEN - (cp - cachefname))) ;

	if ((fe.cachefname = mallocstr(cachefname)) == NULL) {
	    rs = SR_NOMEM ;
	    goto badmal1 ;
	}

#ifdef	MALLOCLOG
	malloclog_alloc(fe.cachefname,-1,"hostsfile_fileadd:filename") ;
#endif

/* store the entry */

	fe.mtime = 0 ;
	rs = vecitem_add(&atp->files,&fe,sizeof(HOSTSFILE_FILE)) ;

	if (rs < 0)
	    goto badaddfile ;

	fi = rs ;

/* do we need to create the file? */

	rs = hostsfile_mapit(atp,fi,eep) ;

/* parse the file for the first time */

#if	CF_DEBUGS
	debugprintf("hostsfile_fileadd: calling hostsfile_parsefile\n") ;
#endif

	rs = hostsfile_parsefile(atp,fi,eep) ;

#if	CF_DEBUGS
	debugprintf("hostsfile_fileadd: exiting, rs=%d\n",rs) ;
#endif

	return rs ;

/* handle bad things */
badparse:

badaddfile:
	uc_free(fe.cachefname) ;

#ifdef	MALLOCLOG
	malloclog_free(fe.cachefname,"hostsfile_fileadd:cachefname") ;
#endif

badmal1:
	uc_free(fe.hostsfname) ;

#ifdef	MALLOCLOG
	malloclog_free(fe.cachefname,"hostsfile_fileadd:hostsfname") ;
#endif

badmal0:
	return rs ;
}
/* end subroutine (hostsfile_fileadd) */


/* free up the resources occupied by an HOSTSFILE list object */
int hostsfile_finish(atp)
HOSTSFILE	*atp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (atp == NULL) return SR_FAULT ;

	if (atp->magic != HOSTSFILE_MAGIC) return SR_NOTOPEN ;

	rs1 = hostsfile_finishentries(atp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = hostsfile_finishfiles(atp) ;
	if (rs >= 0) rs = rs1 ;

/* free up the rest of the main object data */

	rs1 = vecitem_finish(&atp->aes_std) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecitem_finish(&atp->aes_rgx) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecitem_finish(&atp->files) ;
	if (rs >= 0) rs = rs1 ;

	atp->magic = 0 ;
	return rs ;
}
/* end subroutine (hostsfile_finish) */


/* search the netgroup table for a netgroup match */
int hostsfile_allowed(atp,netgroup,machine,username,password)
HOSTSFILE	*atp ;
char		netgroup[], machine[] ;
char		username[], password[] ;
{
	HOSTSFILE_ENT	ae, *aep ;
	VECITEM		*slp ;
	VECITEM_CUR	cur ;
	time_t		daytime = time(NULL) ;
	int		rs ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("hostsfile_allowed: ent, netgroup=%s\n",netgroup) ;
	debugprintf("hostsfile_allowed: machine=%s\n",machine) ;
#endif

	if (atp == NULL) return SR_FAULT ;

	if (atp->magic != HOSTSFILE_MAGIC) return SR_NOTOPEN ;

	if (machine == NULL) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("hostsfile_allowed: ent, netgroup=%s\n",netgroup) ;
	debugprintf("hostsfile_allowed: args m=%s u=%s p=%s\n",
	    machine,
	    username,
	    password) ;
#endif

/* should we even check? */

	if ((daytime - atp->checktime) > HOSTSFILE_MINCHECKTIME) {

		rs = hostsfile_checkfiles(atp,daytime,NULL) ;
		if (rs < 0)
			goto ret0 ;

	} /* end if (we needed to check) */

/* load up a fake entry for comparison purposes */

	entry_start(&ae) ;

	ae.netgroup.std = netgroup ;
	if ((netgroup == NULL) || (netgroup[0] == '\0'))
	    ae.netgroup.std = HOSTSFILE_DEFNETGROUP ;

	ae.machine.std = machine ;
	ae.username.std = username ;
	ae.password.std = password ;

/* search the STD entries first */

	slp = &atp->aes_std ;

#if	CF_DEBUGS
	i = vecitem_count(slp) ;
	debugprintf("hostsfile_allowed: STD entries=%d\n",i) ;
#endif

	vecitem_curbegin(slp,&cur) ;

#if	CF_DEBUGS
	debugprintf("hostsfile_allowed: STD got cursor\n") ;
#endif

	while ((rs = vecitem_fetch(slp,&ae,&cur,cmpstdfunc,&aep)) >= 0) {
		if (aep == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("hostsfile_allowed: fetched one, machine=%s\n",
	        aep->machine.std) ;
#endif

	    if (entry_mat2(aep,&ae)) 
		break ;

#if	CF_DEBUGS
	    debugprintf("hostsfile_allowed: still in loop\n") ;
#endif

	} /* end while */

#if	CF_DEBUGS
	debugprintf("hostsfile_allowed: STD done looping rs=%d\n",rs) ;
#endif

	vecitem_curend(slp,&cur) ;

/* search the RGX entries (if necessary) */

	if (rs < 0) {

	    slp = &atp->aes_rgx ;

#if	CF_DEBUGS
	    i = vecitem_count(slp) ;
	    debugprintf("hostsfile_allowed: RGX entries=%d\n",i) ;
#endif

	    for (i = 0 ; vecitem_get(slp,i,&aep) >= 0 ; i += 1) {
	        if (aep == NULL) continue ;

#if	CF_DEBUGS
	        debugprintf("hostsfile_allowed: loop i=%d\n",i) ;
#endif

	        if (aep == NULL) continue ;

	        if (entry_mat3(aep,&ae)) 
			break ;

	    } /* end for (looping through entries) */

#if	CF_DEBUGS
	    debugprintf("hostsfile_allowed: RGX done\n") ;
#endif

	} /* end if (comparing RGX entries) */

ret0:

#if	CF_DEBUGS
	debugprintf("hostsfile_allowed: exiting, rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hostsfile_allowed) */


/* search the netgroup table for any netgroup match */
int hostsfile_anyallowed(atp,ngp,mnp,username,password)
HOSTSFILE	*atp ;
vecstr		*ngp, *mnp ;
char		username[], password[] ;
{
	int		rs ;
	int		i, j ;
	char		*netgroup ;
	char		*machine ;

#if	CF_DEBUGS
	debugprintf("hostsfile_anyallowed: ent\n") ;
#endif

	if (atp == NULL) return SR_FAULT ;

	if (atp->magic != HOSTSFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostsfile_anyallowed: 2\n") ;
#endif


	for (i = 0 ; (rs = vecstr_get(ngp,i,&netgroup)) >= 0 ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("hostsfile_anyallowed: trying netgroup=%s\n",netgroup) ;
#endif

	    for (j = 0 ; (rs = vecstr_get(mnp,j,&machine)) >= 0 ; j += 1) {

#if	CF_DEBUGS
	        debugprintf("hostsfile_anyallowed: trying machine=%s\n",
			machine) ;
#endif

	        rs = hostsfile_allowed(atp,netgroup,machine,username,password) ;

#if	CF_DEBUGS
	        debugprintf("hostsfile_anyallowed: rs=%d\n",rs) ;
#endif

	        if (rs >= 0) 
			goto done ;

	    } /* end for */

	} /* end for (looping over netgroups) */

#if	CF_DEBUGS
	debugprintf("hostsfile_anyallowed: exiting rs=%d\n",rs) ;
#endif

done:
	return rs ;
}
/* end subroutine (hostsfile_anyallowed) */


#ifdef	COMMENT

/* search the netgroup table for a netgroup match */
int hostsfile_find(atp,netgroup,sepp)
HOSTSFILE	*atp ;
char		netgroup[] ;
HOSTSFILE_ENT	**sepp ;
{
	VECITEM		*slp ;
	int		i ;
	const char	*sp ;

	if (atp == NULL) return SR_FAULT ;

	if (atp->magic != HOSTSFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostsfile_find: ent, netgroup=%s\n",netgroup) ;
#endif

	slp = &atp->e ;
	for (i = 0 ; vecitem_get(slp,i,sepp) >= 0 ; i += 1) {
	    if (*sepp == NULL) continue ;

	    sp = (*sepp)->netgroup ;

#if	CF_DEBUGS
	    debugprintf("hostsfile_find: got entry=\"%s\"\n",sp) ;
#endif

	    if (strcmp(netgroup,sp) == 0)
	        return i ;

	} /* end for (looping through entries) */

#if	CF_DEBUGS
	debugprintf("hostsfile_find: did not match any entry\n") ;
#endif

	return -1 ;
}
/* end subroutine (hostsfile_find) */

#endif /* COMMENT */


/* cursor manipulations */
int hostsfile_curbegin(atp,cp)
HOSTSFILE	*atp ;
HOSTSFILE_CUR	*cp ;
{

	if (atp == NULL) return SR_FAULT ;

	if (atp->magic != HOSTSFILE_MAGIC) return SR_NOTOPEN ;

	cp->i = cp->j = -1 ;
	return SR_OK ;
}
/* end subroutine (hostsfile_curbegin) */


int hostsfile_curend(atp,cp)
HOSTSFILE	*atp ;
HOSTSFILE_CUR	*cp ;
{

	if (atp == NULL) return SR_FAULT ;

	if (atp->magic != HOSTSFILE_MAGIC) return SR_NOTOPEN ;

	cp->i = cp->j = -1 ;
	return SR_OK ;
}
/* end subroutine (hostsfile_curend) */


/* enumerate the netgroup entries */
int hostsfile_enum(atp,cp,sepp)
HOSTSFILE	*atp ;
HOSTSFILE_CUR	*cp ;
HOSTSFILE_ENT	**sepp ;
{
	VECITEM		*slp ;
	HOSTSFILE_ENT	*aep ;
	int		rs ;
	int		i, j ;

	if (atp == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (atp->magic != HOSTSFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostsfile_enum: ent, i=%d\n",cp->i) ;
#endif

	if (sepp == NULL)
		sepp = &aep ;

	rs = SR_NOTFOUND ;
	if (cp->i <= 0) {

#if	CF_DEBUGS
	    debugprintf("hostsfile_enum: STD entries\n") ;
#endif

	    slp = &atp->aes_std ;
	    cp->j = (cp->j < 0) ? 0 : cp->j + 1 ;
	    for (j = cp->j ; (rs = vecitem_get(slp,j,sepp)) >= 0 ; j += 1)
	        if (*sepp != NULL) break ;

#if	CF_DEBUGS
	    debugprintf("hostsfile_enum: vecitem_get rs=%d j=%d\n",rs,j) ;
#endif

	    if (rs < 0) {

	        cp->j = -1 ;
	        cp->i = 1 ;

	    } else {

#if	CF_DEBUGS
	debugprintf("hostsfile_enum: STD n=%s m=%s u=%s p=%s\n",
		(*sepp)->netgroup.std,
		(*sepp)->machine.std,
		(*sepp)->username.std,
		(*sepp)->password.std) ;
#endif

	        cp->i = 0 ;
	        cp->j = j ;

	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("hostsfile_enum: intermediate rs=%d cur_i=%d cur_j=%d\n",
	    rs,cp->i,cp->j) ;
#endif

	if (cp->i == 1) {

#if	CF_DEBUGS
	    debugprintf("hostsfile_enum: RGX entries\n") ;
#endif

	    slp = &atp->aes_rgx ;
	    cp->j = (cp->j < 0) ? 0 : cp->j + 1 ;
	    for (j = cp->j ; (rs = vecitem_get(slp,j,sepp)) >= 0 ; j += 1)
	        if (*sepp != NULL) break ;

	    if (rs < 0) {

	        cp->j = -1 ;
	        cp->i += 1 ;

	    } else {

#if	CF_DEBUGS
	debugprintf("hostsfile_enum: RGX n=%s m=%s u=%s p=%s\n",
		(*sepp)->netgroup.rgx,
		(*sepp)->machine.rgx,
		(*sepp)->username.rgx,
		(*sepp)->password.rgx) ;
#endif

	        cp->j = j ;

		}

	} /* end if (RGX entries) */

#if	CF_DEBUGS
	debugprintf("hostsfile_enum: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hostsfile_enum) */


/* check if the access tables files have changed */
int hostsfile_check(atp,eep)
HOSTSFILE	*atp ;
VECITEM		*eep ;
{
	struct ustat	sb ;
	HOSTSFILE_FILE	*fep ;
	time_t		daytime = time(NULL) ;
	int		rs, i ;
	int		c_changed = 0 ;

	if (atp == NULL) return SR_FAULT ;

	if (atp->magic != HOSTSFILE_MAGIC) return SR_NOTOPEN ;

/* should we even check? */

	if ((daytime - atp->checktime) <= HOSTSFILE_CHECKTIME)
	    return SR_OK ;

	rs = hostsfile_checkfiles(atp,daytime,eep) ;

#if	CF_DEBUGS
	debugprintf("hostsfile_check: exiting rs=%d\n",
	    rs) ;
#endif

	return rs ;
}
/* end subroutine (hostsfile_check) */


/* private subroutines */


/* check if the files have changed */
static int hostsfile_checkfiles(atp,daytime,eep)
HOSTSFILE	*atp ;
time_t		daytime ;
VECITEM		*eep ;
{
	struct ustat	sb ;
	HOSTSFILE_FILE	*fep ;
	int		rs = SR_OK ;
	int		i ;
	int		c_changed = 0 ;

#if	CF_DEBUGS
	debugprintf("hostsfile_checkfiles: about to loop\n",i) ;
#endif

	for (i = 0 ; vecitem_get(&atp->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

	    if ((u_stat(fep->filename,&sb) >= 0) &&
	        (sb.st_mtime > fep->mtime) &&
	        ((daytime - sb.st_mtime) >= HOSTSFILE_CHANGETIME)) {

#if	CF_DEBUGS
	        debugprintf("hostsfile_checkfiles: file=%d changed\n",i) ;
	        debugprintf("hostsfile_checkfiles: freeing file entries\n") ;
#endif

	        hostsfile_finishfes(atp,i) ;

#if	CF_DEBUGS
	        debugprintf("hostsfile_checkfiles: parsing the file again\n") ;
#endif

	        if ((rs = hostsfile_parsefile(atp,i,eep)) >= 0)
	            c_changed += 1 ;

#if	CF_DEBUGS
	        debugprintf("hostsfile_checkfiles: "
			"hostsfile_parsefile() rs=%d\n",
			rs) ;
#endif

	    } /* end if */

	} /* end for */

	if ((rs >= 0) && c_changed) {

#if	CF_DEBUGS
	        debugprintf("hostsfile_checkfiles: sorting STD entries\n") ;
#endif

	        rs = vecitem_sort(&atp->aes_std,cmpstdfunc) ;

	}

	atp->checktime = daytime ;

#if	CF_DEBUGS
	debugprintf("hostsfile_checkfiles: exiting rs=%d changed=%d\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? c_changed : rs ;
}
/* end subroutine (hostsfile_check) */


/* parse a hosts file */
static int hostsfile_parsefile(atp,fi,eep)
HOSTSFILE	*atp ;
int		fi ;
VECITEM		*eep ;
{
	struct ustat	sb ;
	HOSTSFILE_FILE	*fep ;
	HOSTSFILE_ENT	se ;
	STRTAB		nst ;		/* name string table */
	FIELD		fsb ;
	bfile		file, *fp = &file ;
	int		rs = SR_OK ;
	int		i ;
	int		len, line ;
	int		fl ;
	int		c_added = 0 ;
	const char	*fp ;
	const char	*filename ;
	const char	*cp ;
	char		lbuf[LINELEN + 1] ;

#if	CF_DEBUGS
	debugprintf("hostsfile_parsefile: ent\n") ;
#endif

	rs = vecitem_get(&atp->files,fi,&fep) ;
	if (rs < 0)
	    goto ret0 ;

	if (fep == NULL) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("hostsfile_parsefile: 2\n") ;
#endif

	rs = bopen(fp,fep->filename,"r",0664) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("hostsfile_parsefile: bopen rs=%d\n",
	    rs) ;
#endif

	rs = bcontrol(fp,BC_STAT,&sb) ;
	if (rs < 0)
	    goto done ;

/* have we already parsed this one? */

#if	CF_DEBUGS
	debugprintf("hostsfile_parsefile: 4\n") ;
#endif

	if (fep->mtime >= sb.st_mtime)
	    goto done ;

#if	CF_DEBUGS
	debugprintf("hostsfile_parsefile: 5\n") ;
#endif

	fep->mtime = sb.st_mtime ;

/* loop through the lines of the file */

	rs = strtab_start(&nst,(int) (sb.st_size / 2)) ;
	if (rs < 0)
		goto bad2 ;

	rs = rectab_init(&rt,(int) (sb.st_size / 20)) ;
	if (rs < 0)
		goto bad3 ;

#if	CF_DEBUGS
	debugprintf("hostsfile_parsefile: start processing\n") ;
#endif

	c_added = 0 ;
	line = 0 ;
	while ((len = breadline(fp,lbuf,LINELEN)) > 0) {
		INETADDR	ia ;
		int		cnamelen, csi, si ;
		char		*cname ;


	    line += 1 ;
	    if (len == 1) 
		continue ;	/* blank line */

	    if (lbuf[len - 1] != '\n') {

	        while ((c = bgetc(fp)) >= 0)
	            if (c == '\n') break ;

	        continue ;
	    }

	    lbuf[--len] = '\0' ;
	    cp = lbuf ;
	    while (CHAR_ISWHITE(*cp)) {
	        cp += 1 ;
	        len -= 1 ;
	    }

	    if ((*cp == '\0') || (*cp == '#')) continue ;

		if ((rs = field_start(&fsb,cp,len)) >= 0) {

	    if ((fl = field_get(&fsb,arg_terms,&fp)) > 0) {

		rs = inetaddr_startstr(&ia,fp,fl) ;

		if (rs < 0) {
			errline(eep,fep->filename,line) ;
			continue ;
		}

/* pick off names until the end */

		i = 0 ;
		while ((fl = field_get(&fsb,arg_terms,&fp)) > 0) {

/* add this entry */

		si = strtab_add(&nst,fp,fl) ;

			if (i == 0) {

				cname = fp ;
				cnamelen = fl ;
				csi = si ;

			} /* end if (canonical name) */

/* add this record to the record table */

		rs = rectab_add(&rt,ia,csi,si) ;

		if (rs < 0)
			break ;

/* continue */

			i += 1 ;
			if (fsb.term == '#')
				break ;

		} /* end while (extracting names) */

		if (i < 1)
			errline(eep,fep->filename,line) ;

		inetaddr_finish(&ia) ;

	        if (rs >= 0)
	            c_added += i ;

		} /* end if */

		field_finish(&fsb) ;
	    } /* end if */

		if (rs < 0)
			break ;

	} /* end while (reading lines) */

/* OK, if no errors, make the indexes and write them out to the file */

	if (rs >= 0) {



	} /* end if (writing out cache file) */

bad4:
	rectab_free(&rt) ;

bad3:
	strtab_finish(&nst) ;

/* done with configuration file processing */
bad2:
done:
ret1:

#if	CF_DEBUGS
	debugprintf("hostsfile_parsefile: added=%d exiting, rs=%d\n",
	    c_added,rs) ;
#endif

	bclose(fp) ;

ret0:

#if	CF_DEBUGS
	debugprintf("hostsfile_parsefile: ret rs=%d c_added=%u\n",
	    rs,c_added) ;
#endif

	return (rs >= 0) ? c_added : rs ;
}
/* end subroutine (hostsfile_parsefile) */


/* add an entry to the access entry list */
static int hostsfile_addentry(atp,sep)
HOSTSFILE	*atp ;
HOSTSFILE_ENT	*sep ;
{
	int		rs = SR_OK ;

	if (parttype(sep->netgroup.std) == 0) {

#if	CF_DEBUGS
	    debugprintf("hostsfile_addentry: has plain s=%s\n",
		sep->netgroup.std) ;
#endif

	    rs = vecitem_add(&atp->aes_std,sep,sizeof(HOSTSFILE_ENT)) ;

	} else
	    rs = vecitem_add(&atp->aes_rgx,sep,sizeof(HOSTSFILE_ENT)) ;

	return rs ;
}
/* end subroutine (hostsfile_addentry) */


/* free up all of the entries in this HOSTSFILE list */
static int hostsfile_finishentries(atp)
HOSTSFILE	*atp ;
{
	HOSTSFILE_ENT	*sep ;
	VECITEM		*slp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i, j ;

	for (j = 0 ; j < 2 ; j += 1) {

	    slp = (j == 0) ? &atp->aes_std : &atp->aes_rgx ;
	    for (i = 0 ; vecitem_get(slp,i,&sep) >= 0 ; i += 1) {
	        if (sep == NULL) continue ;

	        rs1 = entry_finish(sep) ;
		if (rs >= 0) rs = rs1 ;

	    } /* end for */

	} /* end for */

	return rs ;
}
/* end subroutine (hostsfile_finishentries) */


/* free up all of the entries in this HOSTSFILE list associated w/ a file */
static int hostsfile_finishfes(atp,fi)
HOSTSFILE	*atp ;
int		fi ;
{
	HOSTSFILE_ENT	*sep ;
	VECITEM		*slp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i, j ;

#if	CF_DEBUGS
	debugprintf("hostsfile_finishfes: want to delete all fi=%d\n",fi) ;
#endif

	for (j = 0 ; j < 2 ; j += 1) {

	    slp = (j == 0) ? &atp->aes_std : &atp->aes_rgx ;
	    for (i = 0 ; (rs = vecitem_get(slp,i,&sep)) >= 0 ; i += 1) {
	        if (sep == NULL) continue ;

#if	CF_DEBUGS
	        debugprintf("hostsfile_finishfes: i=%d fi=%d\n",i,sep->fi) ;
#endif

	        if ((sep->fi == fi) || (fi < 0)) {

#if	CF_DEBUGS
	            debugprintf("hostsfile_finishfes: got one\n") ;
#endif

	            rs1 = entry_finish(sep) ;
		    if (rs >= 0) rs = rs1 ;

	            rs1 = vecitem_del(slp,i) ;
		    if (rs >= 0) rs = rs1 ;

	            i -= 1 ;

	        } /* end if */

	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("hostsfile_finishfes: popped up to rs=%d i=%d\n",
		rs,i) ;
#endif

	} /* end for */

#if	CF_DEBUGS
	debugprintf("hostsfile_finishfes: exiting\n") ;
#endif

	return rs ;
}
/* end subroutine (hostsfile_finishfes) */


/* free up all of the files in this HOSTSFILE list */
static int hostsfile_finishfiles(atp)
HOSTSFILE	*atp ;
{
	HOSTSFILE_FILE	*afp ;
	VECITEM		*slp = &atp->files ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; (rs = vecitem_get(slp,i,&afp)) >= 0 ; i += 1) {
	    if (afp == NULL) continue ;

	    if (afp->filename != NULL) {
	        rs1 = uc_free(afp->filename) ;
		if (rs >= 0) rs = rs1 ;
	    }
	    vecitem_del(slp,i--) ;

	} /* end for */

	return rs ;
}
/* end subroutine (hostsfile_finishfiles) */


/* initialize an entry */
static int entry_start(sep)
HOSTSFILE_ENT	*sep ;
{

	sep->fi = -1 ;
	part_start(&sep->netgroup) ;

	part_start(&sep->machine) ;

	part_start(&sep->username) ;

	part_start(&sep->password) ;

	return SR_OK ;
}
/* end subroutine (entry_start) */


/* free up an entry */
static int entry_finish(sep)
HOSTSFILE_ENT	*sep ;
{

#if	CF_DEBUGS
	debugprintf("hostsfile/entry_finish: ent\n") ;
#endif

	part_finish(&sep->netgroup) ;

	part_finish(&sep->machine) ;

	part_finish(&sep->username) ;

	part_finish(&sep->password) ;

	return 0 ;
}
/* end subroutine (entry_finish) */


/* compare if all but the netgroup are equal (or matched) */
static int entry_mat2(e1p,e2p)
HOSTSFILE_ENT	*e1p, *e2p ;
{

#if	CF_DEBUGS
	debugprintf("hostsfile/entry_mat2: ent\n") ;
	debugprintf("hostsfile/entry_mat2: m=%s u=%s p=%s\n",
	    e2p->machine.std,
	    e2p->username.std,
	    e2p->password.std) ;
#endif

#ifdef	OPTIONAL
	if (! part_match(&e1p->netgroup,e2p->netgroup.std))
	    return FALSE ;
#endif

#if	CF_DEBUGS
	debugprintf("hostsfile/entry_mat2: machine?\n") ;
#endif

	if (! part_match(&e1p->machine,e2p->machine.std))
	    return FALSE ;

#if	CF_DEBUGS
	debugprintf("hostsfile/entry_mat2: user?\n") ;
#endif

	if (! part_match(&e1p->username,e2p->username.std))
	    return FALSE ;

#if	CF_DEBUGS
	debugprintf("hostsfile/entry_mat2: password?\n") ;
#endif

	if (! part_match(&e1p->password,e2p->password.std))
	    return FALSE ;

#if	CF_DEBUGS
	debugprintf("hostsfile/entry_mat2: succeeded !\n") ;
#endif

	return TRUE ;
}
/* end subroutine (entry_mat2) */


/* compare if all of the entry is equal (or matched) */
static int entry_mat3(e1p,e2p)
HOSTSFILE_ENT	*e1p, *e2p ;
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


/* free up a previously allocated memory block */
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
HOSTSFILE_ENT	**e1pp, **e2pp ;
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


/* record table stuff */


static void errline(eep,filename,line)
VECITEM	*eep ;
char	filename[] ;
int	line ;
{
	struct hostsfile_errline	el ;

	if (eep == NULL) return ;

	el.filename = filename ;
	el.line = line ;
	vecitem_add(eep,&el,sizeof(struct hostsfile_errline)) ;

}
/* end subroutine (errline) */


