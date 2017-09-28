/* tailemod */

/* tailemod storage object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 1996-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

	- 2003-11-04, David A­D­ Morano
        I don't know where all this has been (apparently "around") but I grabbed
        it from the CM object !

*/

/* Copyright © 1996,2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages what tailemod have been loaded so far.


*******************************************************************************/


#define	TAILEMOD_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<dlfcn.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<fsdir.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"tailemod.h"


/* local defines */

#define	TO_FILECHECK	3

#define	NEXTS		3


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,
			const char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;


/* external variables */


/* forward references */

static int	tailemod_searchdirs(TAILEMOD *,const char *,
			TAILEMOD_MODULE **) ;
static int	tailemod_checkdir(TAILEMOD *,const char *,const char *,
			TAILEMOD_MODULE **) ;

static ini	module_init(TAILEMOD_MODULE *,void *) ;
static ini	module_inc(TAILEMOD_MODULE *) ;
static ini	module_free(TAILEMOD_MODULE *) ;

static int	entry_init(TAILEMOD_ENT *,const char *,
			TAILEMOD_MODULE *) ;
static int	entry_subs(TAILEMOD_ENT *,void *) ;
static int	entry_hasname(TAILEMOD_ENT *,void *,const char *) ;
static int	entry_free(TAILEMOD_ENT *) ;

static int	vcmpname(TAILEMOD_ENT **,TAILEMOD_ENT **) ;


/* local variables */

static const char	*exts[] = {
	".so",
	".o",
	"",
	NULL
} ;

static const char	*dirs64[] = {
	"lib/tailemod/sparcv9",
	"lib/tailemod/sparc",
	"lib/tailemod",
	NULL
} ;

static const char	*dirs32[] = {
	"lib/tailemod/sparcv8",
	"lib/tailemod/sparcv7",
	"lib/tailemod/sparc",
	"lib/tailemod",
	NULL
} ;

static const char	*subs[] = {
	"init",
	"store",
	"check",
	"readline",
	"summary",
	"finish",
	"free",
	NULL
} ;

enum subs {
	sub_init,
	sub_store,
	sub_check,
	sub_readline,
	sub_summary,
	sub_finish,
	sub_free,
	sub_overlast
} ;







int tailemod_init(op,pr,dirs)
TAILEMOD	*op ;
const char	pr[] ;
const char	*dirs[] ;
{
	int	rs ;
	int	i ;
	int	size ;


	if (op == NULL)
	    return SR_FAULT ;

	if (pr == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(TAILEMOD)) ;

	op->pr = mallocstr(pr) ;

	if (op->pr == NULL)
	    goto bad0 ;

	if (dirs != NULL) {

	    rs = vecstr_start(&op->dirlist,10,0) ;

	    if (rs < 0)
	        goto bad2 ;

	    op->f.vsdirs = (rs >= 0)  ;
	    for (i = 0 ; (rs >= 0) && (dirs[i] != NULL) ; i += 1) {

	        rs = vecstr_add(&op->dirlist,(char *) dirs[i],-1) ;

		if (rs < 0)
			break ;

	    } /* end for */

	    if (rs >= 0) {

	        op->dirs = (const char **) op->dirlist.va ;

	    }

	    if (rs < 0)
	        goto bad3 ;

	} else
	    op->dirs = (sizeof(caddr_t) == 8) ? dirs64 : dirs32 ;

	size = sizeof(struct tailemod_module) ;
	opts = 0 ;
	rs = vecobj_start(&op->modules,size,10,opts) ;

	if (rs < 0)
	    goto bad4 ;

	size = sizeof(struct tailemod_ent) ;
	opts = VECOBJ_OORDERED ;
	rs = vecobj_start(&op->entries,size,10,opts) ;

	if (rs < 0)
	    goto bad5 ;

	op->magic = TAILEMOD_MAGIC ;
	return rs ;

/* bad stuff */
bad5:
	vecobj_finish(&op->modules) ;

bad4:
bad3:
	if (op->f.vsdirs)
		vecstr_finish(&op->dirlist) ;

bad2:
bad1:
	if (op->pr != NULL)
		uc_free(op->pr) ;

bad0:
	return rs ;
}
/* end subroutine (tailemod_init) */


/* load a plugin module */
int tailemod_load(op,name)
TAILEMOD	*op ;
const char	name[] ;
{
	struct tailemod_module	*mp ;

	TAILEMOD_ENT	se, e, *ep ;

	TAILEMOD_MODULE	*mp ;

	TAILEMOD_INFO	dip ;

	int	rs, rs1 ;
	int	i, nl, rl, cl ;
	int	size ;
	int	ei ;
	int	f_alloc = FALSE ;
	int	(*fp)(int) ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	*cp ;

	void	*dhp ;


	if ((name == NULL) || (name[0] == '\0'))
	    return SR_INVALID ;

/* we need to find if we already have this module */

	mp = NULL ;

/* search the existing entries to see if we already have this name */

	se.name = (char *) name ;
	rs1 = vecobj_search(&op->entries,&se,vcmpname,&ep) ;

	if (rs1 < 0) {

		rs1 = tailemod_searchdirs(op,name,&mp) ;


/* create a new load module descriptor */

	size = sizeof(TAILEMOD_MODULE) ;
	rs = uc_malloc(size,&mp) ;

	if (rs < 0)
	    goto bad0 ;

	f_alloc = TRUE ;
	memset(mp,0,size) ;

/* search for it in the filesystem */

	rs = entry_init(&e,name,mp) ;

	if (rs < 0)
	    goto bad1 ;

/* save this entry */
ret1:
	rs = vecobj_add(&op->entries,&e) ;

#if	CF_DEBUGS
	debugprintf("tailemod_load: vecobj_add() rs=%d\n",rs) ;
#endif

	ei = rs ;
	if (rs < 0)
	    goto bad2 ;

ret0:

#if	CF_DEBUGS
	debugprintf("tailemod_load: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? ei : rs ;

/* bad stuff */
bad2:
	entry_free(&e) ;

bad1:
	if (f_alloc)
	    uc_free(mp) ;

bad0:
	return rs ;
}
/* end subroutine (tailemod_load) */


int tailemod_unload(op,name)
TAILEMOD	*op ;
const char	name[] ;
{
	TAILEMOD_ENT	te, *dep ;

	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != TAILEMOD_MAGIC)
	    return SR_NOTOPEN ;

	if ((name == NULL) || (name[0] == '\0'))
	    return SR_INVALID ;

	te.name = (char *) name ;
	rs = vecobj_search(&op->names,&te,vcmpname,&dep) ;

	if (rs >= 0) {

	    if (dep->count <= 1) {

	        entry_free(dep) ;

	        vecobj_del(&op->names,rs) ;

	    } else
	        dep->count -= 1 ;

	}

	return rs ;
}
/* end subroutine (tailemod_unload) */


int tailemod_check(op,daytime)
TAILEMOD	*op ;
time_t		daytime ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != TAILEMOD_MAGIC)
	    return SR_NOTOPEN ;

	return 0 ;
}
/* end subroutine (tailemod_check) */


/* free up and get out */
int tailemod_free(op)
TAILEMOD	*op ;
{
	struct tailemod_ent	*ep ;

	struct tailemod_module	*mp ;

	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != TAILEMOD_MAGIC)
	    return SR_NOTOPEN ;

/* free up entries */

	for (i = 0 ; vecobj_get(&op->entries,i,&ep) >= 0 ; i += 1) {

	    if (ep == NULL) continue ;

	    entry_free(ep) ;

	} /* end for */

	vecobj_finish(&op->entries) ;

/* free up modules */

	for (i = 0 ; vecobj_get(&op->modules,i,&mp) >= 0 ; i += 1) {

	    if (mp == NULL) continue ;

	    module_free(mp) ;

	} /* end for */

	vecobj_finish(&op->modules) ;

/* free up everything else */

	if (op->f.vsdirs)
	    vecstr_finish(&op->dirlist) ;

	if (op->pr != NULL)
	    uc_free(op->pr) ;

	op->pr = NULL ;
	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (tailemod_free) */


/* private subroutines */


static int tailemod_searchdirs(op,name,mpp)
TAILEMOD	*op ;
const char	name[] ;
TAILEMOD_MODULE	**mpp ;
{
	int	rs ;
	int	i ;


	for (i = 0 ; op->dirs[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("tailemod_searchdirs: dir=%s\n",op->dirs[i]) ;
#endif

	    rs = tailemod_checkdir(op,op->dirs[i],name,mpp) ;

#if	CF_DEBUGS
	    debugprintf("tailemod_searchdirs: tailemod_checkdir() rs=%d\n",rs) ;
#endif

	    if (rs > 0)
	        break ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("tailemod_load: search rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

	dhp = e.mp->dhp ;


}
/* end subroutine (tailemod_searchdirs) */


/* try to load a file with the given name */
static int tailemod__checkdir(op,dname,name,mpp)
TAILEMOD	*op ;
const char	dname[] ;
const char	name[] ;
TAILEMOD_MODULE	**mpp ;
{
	struct ustat	sb ;

	TAILEMOD_INFO	*dip ;

	fsdir		dir ;

	fsdir_ent	slot ;

	vecstr		enames ;

	int	rs, i, c ;
	int	dlmode ;
	int	nl, dnl, fl ;

	void	*dhp = NULL ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	dlfname[MAXPATHLEN + 1] ;
	char	fname[MAXNAMELEN + 1] ;
	char	*dnp ;


/* test for directory existing */

	mkpath2(tmpfname,op->pr,dname) ;

	rs = u_stat(tmpfname,&sb) ;

#if	CF_DEBUGS
	debugprintf("tailemod/entry_checkdir: u_stat() rs=%d\n",rs) ;
#endif

	if ((rs < 0) || (! S_ISDIR(sb.st_mode)))
	    return SR_NOENT ;

	rs = vecstr_start(&enames,3,0) ;

	if (rs < 0)
	    goto ret0 ;

/* read the directory looking for the prefix name parts */

	nl = strlen(name) ;

#if	CF_DEBUGS
	debugprintf("tailemod/entry_checkdir: nl=%u name=%s\n",nl,name) ;
#endif

	c = 0 ;
	rs = fsdir_open(&dir,tmpfname) ;

	if (rs >= 0) {

	while ((dnl = fsdir_read(&dir,&slot)) > 0) {

	    if (dnl < nl) 
		continue ;

	    dnp = slot.name ;
	    if (strncmp(dnp,name,nl) != 0)
	        continue ;

#if	CF_DEBUGS
	    debugprintf("tailemod/entry_checkdir: el=%u ext=%s\n",
	        (dnl - nl),
	        (dnp + nl)) ;
#endif

	    if (matstr(exts,(dnp + nl),(dnl - nl)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("tailemod/entry_checkdir: match d_name=%s\n",dnp) ;
#endif

	        c += 1 ;
	        rs = vecstr_add(&enames,(dnp + nl),(dnl - nl)) ;

		if (rs < 0)
			break ;

	        if (c >= NEXTS)
	            break ;

	    } /* end if (got a match) */

	} /* end while (directory entries) */

	fsdir_close(&dir) ;

	} /* end if (opened directory) */

/* try to load them in the specified order */

	if ((rs >= 0) && (c > 0)) {

	rs = SR_NOTFOUND ;
	for (i = 0 ; exts[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("tailemod/entry_checkdir: trying ext=%s\n",exts[i]) ;
#endif

	    rs = vecstr_findn(&enames,exts[i],-1) ;

#if	CF_DEBUGS
	    debugprintf("tailemod/entry_checkdir: vecstr_findn() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        rs = SR_INVALID ;
	        mkfnamesuf1(fname,name,exts[i]) ;

	        fl = mkpath2(dlfname,tmpfname,fname) ;

#if	CF_DEBUGS
	        debugprintf("tailemod/entry_checkdir: dfl=%u dlfname=%s\n",
			fl,dlfname) ;
#endif

	        dlmode = RTLD_LAZY | RTLD_LOCAL ;
	        dhp = dlopen(dlfname,dlmode) ;

#if	CF_DEBUGS
	        debugprintf("tailemod/entry_checkdir: dlopen() dhp=%p\n",dhp) ;
#endif

#if	CF_DEBUGS
	        if (dhp == NULL)
	            debugprintf("tailemod/entry_checkdir: dlopen() err=%s\n",
			dlerror()) ;
#endif

	        if (dhp != NULL) {

	            dip = (TAILEMOD_INFO *) dlsym(dhp,name) ;

	            if ((dip != NULL) && (strcmp(dip->name,name) == 0)) {

	                ep->size = dip->size ;
			ep->flags = dip->flags ;
	                ep->mp->dhp = dhp ;
	                rs = SR_OK ;
	                break ;

	            } else
	                dlclose(dhp) ;

	        }

	    } /* end if (tried one) */

	} /* end for */

	}

ret1:
	vecstr_finish(&enames) ;

ret0:
	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (tailemod_checkdir) */


static ini module_init(mp,dhp)
TAILEMOD_MODULE	*mp ;
coid		*dhp ;
{
	int	rs ;


	memset(mp,0,sizeof(TAILEMOD_MODULE)) ;

	mp->dhp = dhp ;
	return rs ;
}


static int module_inc(mp)
TAILEMOD_MODULE	*mp ;
{


	mp->count += 1 ;
	return 0 ;
}


static int module_free(mp)
TAILEMOD_MODULE	*mp ;
{


	if (mp->dhp != NULL)
		dlclose(mp->dhp) ;

	return 0 ;
}



static int entry_init(ep,name,mp)
TAILEMOD_ENT	*ep ;
const char	name[] ;
struct tailemod_module	*mp ;
{
	int	rs = SR_OK ;


	memset(ep,0,sizeof(TAILEMOD_ENT)) ;

	ep->name = mallocstr(name) ;

	if (ep->name == NULL)
	    rs = SR_NOMEM ;

	if (rs >= 0) {

	    mp->count += 1 ;
	    ep->mp = mp ;
	}

	return rs ;
}
/* end subroutine (entry_init) */


static int entry_free(ep)
TAILEMOD_ENT	*ep ;
{
	TAILEMOD_MODULE		*mp ;


	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->name != NULL)
	    uc_free(ep->name) ;

	if (ep->mp != NULL) {

	    mp = ep->mp ;
	    if (mp->count <= 1) {

	        if (mp->dhp != NULL)
	            dlclose(mp->dhp) ;

	        uc_free(mp) ;

	    } else
	        mp->count -= 1 ;

	}

#ifdef	COMMENT
	if (ep->dhp != NULL)
	    dlclose(ep->dhp) ;
#endif

	memset(ep,0,sizeof(TAILEMOD_ENT)) ;

	return 0 ;
}
/* end subroutine (entry_free) */


/* load up the available subroutines from this module */
static int entry_subs(ep,dhp)
TAILEMOD_ENT	*ep ;
void		*dhp ;
{
	int	rs = SR_OK, nl, cl ;
	int	i ;
	int	(*fp)() ;

	char	symname[MAXNAMELEN + 1] ;
	char	*cp ;


	strwcpy(symname,ep->name,MAXNAMELEN) ;

	nl = strlen(ep->name) ;

	cp = (char *) (symname + nl) ;
	cl = MAXNAMELEN - nl ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    sncpy2(cp,cl,"_",subs[i]) ;

	    fp = (int (*)()) dlsym(dhp,symname) ;

#if	CF_DEBUGS
	    debugprintf("entry_subs: sym=%s(%p)\n",symname,fp) ;
#endif

	    if (fp != NULL) {

	        switch (i) {

	        case sub_init:
	            ep->c.init = fp ;
	            break ;

	        case sub_store:
	            ep->c.store = fp ;
	            break ;

	        case sub_check:
	            ep->c.check = fp ;
	            break ;

	        case sub_readline:
	            ep->c.readline = fp ;
	            break ;

	        case sub_summary:
	            ep->c.summary = fp ;
	            break ;

	        case sub_finish:
	            ep->c.finish = fp ;
	            break ;

	        case sub_free:
	            ep->c.free = fp ;
	            break ;

	        } /* end switch */

	    } /* end if (got a symbol) */

	} /* end for */

/* do a minimal check */

	if (ep->c.init == NULL)
	    rs = SR_LIBACC ;

#if	CF_DEBUGS
	debugprintf("entry_subs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (entry_subs) */


static int entry_hasname(ep,dhp,name)
TAILEMOD_ENT	*ep ;
void		*dhp ;
const char	name[] ;
{
	TAILEMOD_INFO	*dip ;

	int	rs = SR_NOTFOUND ;


	dip = (TAILEMOD_INFO *) dlsym(dhp,name) ;

	if ((dip != NULL) && (strcmp(dip->name,name) == 0)) {

	    ep->size = dip->size ;
	    rs = SR_OK ;

	}

	return rs ;
}
/* end subroutine (entry_hasname) */


/* compare the whole entries (including the netgroup) */
static int vcmpname(e1pp,e2pp)
TAILEMOD_ENT	**e1pp, **e2pp ;
{


	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	return strcmp((*e1pp)->name,(*e2pp)->name) ;
}
/* end subroutine (vcmpname) */



