/* srvtab */

/* perform service table file related functions */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGSFIELD	0		/* other debug? */
#define	CF_REGEX	0		/* BROKEN !!! */


/* revision history:

	= 1999-07-01, David A­D­ Morano
	This subroutine was adopted for use in the 'rexecd' daemon program.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This object reads in a service table file and stores the information
        parsed from that file.


******************************************************************************/


#define	SRVTAB_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<regexpr.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecitem.h>
#include	<char.h>
#include	<ascii.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"srvtab.h"


/* local defines */

#define	SRVTAB_RGXLEN	256		/* regexp buffer length */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	BUFLEN
#define	BUFLEN		(2 * MAXPATHLEN)
#endif

#define	MAXOPENTIME	300		/* maximum FD cache time */
#define	TI_FILECHECK	9		/* file check interval (seconds) */
#define	TI_FILECHANGE	3		/* wait change interval (seconds) */

#define	SRVTAB_REGEX	CF_REGEX


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	field_srvarg(FIELD *,const uchar *,char *,int) ;
extern int	getpwd(char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */

static int	srvtab_fileparse(SRVTAB *,time_t,VECITEM *) ;
static int	srvtab_filedump(SRVTAB *) ;

static int	entry_start(SRVTAB_ENT *) ;
static int	entry_groupsload(SRVTAB_ENT *,const char *,int) ;
static int	entry_groupadd(SRVTAB_ENT *,const char *) ;
static int	entry_enough(SRVTAB_ENT *) ;
static int	entry_finish(SRVTAB_ENT *) ;

static int	stradd(const char **,const char *,int) ;

static void	freeit(const char **) ;


/* local variables */

/* key field terminators ('#', ',', ':', '=') */
static const unsigned char 	key_terms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x24,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
} ;

/* argument field terminators (pound '#' and comma ',') */
static const unsigned char 	saterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*srvkeys[] = {
	"program",
	"arguments",
	"args",
	"username",
	"groupname",
	"options",
	"opts",
	"groups",
	"access",
	"interval",
	"addr",
	"passfile",
	"project",
	NULL
} ;

enum srvkeys {
	srvkey_program,
	srvkey_arguments,
	srvkey_args,
	srvkey_username,
	srvkey_groupname,
	srvkey_options,
	srvkey_opts,
	srvkey_groups,
	srvkey_access,
	srvkey_interval,
	srvkey_addr,
	srvkey_passfile,
	srvkey_project,
	srbkey_overlast
} ;


/* exported subroutines */


int srvtab_open(op,fname,eep)
SRVTAB		*op ;
const char	fname[] ;
VECITEM		*eep ;
{
	time_t	daytime = time(NULL) ;

	int	rs = SR_OK ;
	int	fnl = -1 ;

	const char	*fnp ;
	const char	*cp ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if (op == NULL)
	    return SR_FAULT ;

	op->magic = 0 ;
	op->fd = -1 ;

/* initialize */

	op->opentime = daytime ;
	op->checktime = daytime ;

	fnp = fname ;
	if (fname[0] != '/') {
	    char	pwdbuf[MAXPATHLEN + 1] ;

	    fnp = tmpfname ;
	    rs = getpwd(pwdbuf,MAXPATHLEN) ;
	    if (rs >= 0) {
	        rs = mkpath2(tmpfname,pwdbuf,fname) ;
		fnl = rs ;
	    }

	} /* end if (rooting file) */

	if (rs < 0)
	    goto bad0 ;

	rs = uc_mallocstrw(fnp,fnl,&cp) ;
	if (rs < 0) goto bad1 ;
	op->fname = cp ;

	rs = vecitem_start(&op->e,10,VECITEM_PSWAP) ;
	if (rs < 0)
	    goto bad2 ;

	rs = srvtab_fileparse(op,daytime,eep) ;
	if (rs < 0)
	    goto bad3 ;

	op->magic = SRVTAB_MAGIC ;

ret0:
	return rs ;

/* handle bad things */
bad3:
bad2:
	vecitem_finish(&op->e) ;

bad1:
	uc_free(op->fname) ;
	op->fname = NULL ;

bad0:
	goto ret0 ;
}
/* end subroutine (srvtab_open) */


/* free up the resources occupied by a SRVTAB list */
int srvtab_close(op)
SRVTAB		*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVTAB_MAGIC)
	    return SR_NOTOPEN ;

	if (op->fd >= 0) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	}

	srvtab_filedump(op) ;

	vecitem_finish(&op->e) ;

	if (op->fname != NULL) {
	    uc_free(op->fname) ;
	    op->fname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("srvtab_close: ret\n") ;
#endif

	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (srvtab_close) */


/* search the service table for a service match (input is a RE) */
int srvtab_match(op,service,sepp)
SRVTAB		*op ;
const char	service[] ;
SRVTAB_ENT	**sepp ;
{
	VECITEM	*slp ;

	int	rs = SR_NOTFOUND ;
	int	sl, l1, l2 ;
	int	i ;

	const char	*sp, *cp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVTAB_MAGIC)
	    return SR_NOTOPEN ;

	if (service == NULL)
	    return SR_FAULT ;

	slp = &op->e ;
	for (i = 0 ; vecitem_get(slp,i,sepp) >= 0 ; i += 1) {
	    if (*sepp == NULL) continue ;

	    sp = (*sepp)->service ;

	    if (((cp = strchr(sp,'*')) != NULL) &&
	        (strchr(sp,'\\') == NULL)) {

	        if (strncmp(service,sp,cp - sp) == 0) {

	            cp += 1 ;
	            l1 = strlen(service) ;

	            l2 = strlen(sp) ;

	            sl = sp + l2 - cp ;

	            if (strncmp(service + l1 - sl,cp,sl) == 0)
	                rs = SR_OK ;

	        } /* end if */

	    } else if (strcmp(service,sp) == 0)
	        rs = SR_OK ;

	    if (rs >= 0)
		break ;

	} /* end for (looping through entries) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (srvtab_match) */


/* search the service table for a service match (input is a straight name) */
int srvtab_find(op,service,sepp)
SRVTAB		*op ;
const char	service[] ;
SRVTAB_ENT	**sepp ;
{
	VECITEM	*slp ;

	SRVTAB_ENT	*ep ;

	int	i ;

	const char	*sp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVTAB_MAGIC)
	    return SR_NOTOPEN ;

	if (service == NULL)
	    return SR_FAULT ;

	if (sepp == NULL)
	    sepp = &ep ;

#if	CF_DEBUGS
	debugprintf("srvsearch: ent, service=%s\n",service) ;
#endif

	slp = &op->e ;
	for (i = 0 ; vecitem_get(slp,i,sepp) >= 0 ; i += 1) {
	    if (*sepp == NULL) continue ;

	    sp = (*sepp)->service ;

#if	CF_DEBUGS
	    debugprintf("srvsearch: got entry=\"%s\"\n",sp) ;
#endif

	    if (strcmp(service,sp) == 0)
	        return i ;

	} /* end for (looping through entries) */

#if	CF_DEBUGS
	debugprintf("srvsearch: did not match any entry\n") ;
#endif

	return -1 ;
}
/* end subroutine (srvtab_find) */


/* enumerate the service entries */
int srvtab_get(op,i,sepp)
SRVTAB		*op ;
int		i ;
SRVTAB_ENT	**sepp ;
{
	VECITEM	*slp ;

	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVTAB_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("srvtab_get: ent, i=%d\n",i) ;
#endif

	slp = &op->e ;
	rs = vecitem_get(slp,i,sepp) ;

	return rs ;
}
/* end subroutine (srvtab_get) */


/* check if the server file has changed */
int srvtab_check(op,daytime,eep)
SRVTAB		*op ;
time_t		daytime ;
VECITEM		*eep ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVTAB_MAGIC)
	    return SR_NOTOPEN ;

	if (daytime <= 0)
	    daytime = time(NULL) ;

/* should we even check? */

	if ((daytime - op->checktime) <= TI_FILECHECK)
	    goto ret0 ;

	op->checktime = daytime ;

/* is the file open already? */

	if (op->fd < 0) {

	    rs = uc_open(op->fname,O_RDONLY,0666) ;
	    op->fd = rs ;
	    if (rs >= 0) {
	        op->opentime = daytime ;
	        uc_closeonexec(op->fd,TRUE) ;
	    }

	} /* end if (opening file to cache the FD) */
	if (rs < 0) goto ret0 ;

/* check the modification time on the file */

#if	CF_DEBUGS
	debugprintf("srvtab_check: open check\n") ;
#endif

	rs = u_fstat(op->fd,&sb) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS
	debugprintf("srvtab_check: modification time\n") ;
#endif

	if ((sb.st_mtime > op->mtime) &&
	    ((daytime - sb.st_mtime) >= TI_FILECHANGE)) {

#if	CF_DEBUGS
	    debugprintf("srvtab_check: file changed\n") ;
#endif

	    rs = srvtab_filedump(op) ;

	    if (rs >= 0) {
	        op->mtime = sb.st_mtime ;
	        rs = srvtab_fileparse(op,daytime,eep) ;
	    }

	} else if ((daytime - op->opentime) > MAXOPENTIME) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("srvtab_check: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (srvtab_check) */


/* private subroutines */


/* parse a server file */
static int srvtab_fileparse(op,daytime,eep)
SRVTAB		*op ;
time_t		daytime ;
VECITEM		*eep ;
{
	struct ustat	sb ;

	VECITEM		*slp ;

	SRVTAB_ENT	se ;

	FIELD	fsb ;

	bfile	sfile, *sfp = &sfile ;

	int	rs = SR_OK ;
	int	n, c, len ;
	int	line = 0 ;
	int	cl ;
	int	f_closed = FALSE ;
	int	f_ent = FALSE ;

	const char	*cp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	linebuf2[LINEBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("srvtab_fileparse: ent srvtab=%s\n",
	    op->fname) ;
#endif

	slp = &op->e ;
	if (op->fd >= 0) {

	    rs = bopen(sfp,((char *) op->fd),"dr",0664) ;
	    if (rs >= 0)
	        bseek(sfp,0L,SEEK_SET) ;

	} else
	    rs = bopen(sfp,op->fname,"r",0664) ;

#if	CF_DEBUGS
	debugprintf("srvtab_fileparse: bopen rs=%d\n", rs) ;
#endif

	if (rs < 0)
	    goto badopen ;

/* what about caching the file descriptor? */

	if (op->fd >= 0) {
	    if (daytime <= 0) daytime = time(NULL) ;
	    if ((daytime - op->opentime) > MAXOPENTIME) {
	        u_close(op->fd) ;
	        op->fd = -1 ;
	        f_closed = TRUE ;
	    }
	}

/* store the file modification time */

	bcontrol(sfp,BC_STAT,&sb) ;
	op->mtime = sb.st_mtime ;

/* start processing the file */

#if	CF_DEBUGS
	debugprintf("srvtab_fileparse: start processing\n") ;
#endif

	while ((rs = breadline(sfp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    line += 1 ;

	    if (linebuf[len - 1] != '\n') {
	        while ((c = bgetc(sfp)) >= 0) {
	            if (c == '\n') break ;
		}
	        continue ;
	    }

	    linebuf[--len] = '\0' ;
	    cp = linebuf ;
	    cl = len ;
	    while (CHAR_ISWHITE(*cp)) {
	        cp += 1 ;
	        cl -= 1 ;
	    }

	    if ((*cp == '\0') || (*cp == '#'))
	        continue ;

#if	CF_DEBUGSFIELD
	    debugprintf("srvtab_fileparse: line> %t\n",cp,cl) ;
#endif

	    if ((rs = field_start(&fsb,cp,cl)) >= 0) {
		int		fl ;
		const char	*fp ;

	        if ((fl = field_get(&fsb,key_terms,&fp)) > 0) {

	        if (fsb.term == ':') {

#if	CF_DEBUGS
	            debugprintf("srvtab_fileparse: got a service >%t<\n",
	                fp,fl) ;
#endif

	            if (f_ent) {

			if (entry_enough(&se) > 0) {

#if	CF_DEBUGS
	                debugprintf("srvtab_fileparse: checked enough\n") ;
	                debugprintf("srvtab_fileparse: service=%s SERVICE=%p\n",
	                    se.service,se.service) ;
#endif

	                rs = vecitem_add(slp, &se,sizeof(SRVTAB_ENT)) ;
	                if (rs < 0) {
			    f_ent = FALSE ;
	                    entry_finish(&se) ;
	                    break ;
			}

#if	CF_DEBUGS
	                debugprintf("srvtab_fileparse: previous \"%s\"\n",
	                    se.service) ;
#endif

	            } else {
			f_ent = FALSE ;
	                entry_finish(&se) ;
		    }

		    } /* end if (had previous entry) */

	            rs = entry_start(&se) ;
		    f_ent = (rs >= 0) ;

		    if (rs >= 0) {

	            se.service = mallocstrw(fp,fl) ;

	            if (se.service == NULL)
	                rs = SR_NOMEM ;

#ifdef	MALLOCLOG
	            malloclog_alloc(se.service,-1,"srvtab_fileparse:service") ;
#endif

#if	CF_DEBUGS
	            debugprintf("srvtab_fileparse: new service=%s SERVICE=%p\n",
	                se.service,se.service) ;
#endif

/* compile the expression also */

#if	SRCTAB_REGEX
	            se.matchlen = 0 ;
	            rs = uc_malloc(SRVTAB_RGXLEN,&se.matchcode) ;

	            if (rs > 0) {

#ifdef	MALLOCLOG
	                malloclog_alloc(se.matchcode,-1,
			"srvtab_fileparse:matchcode") ;
#endif

	                cp = compile(se.service,se.matchcode,
	                    (se.matchcode + SRVTAB_RGXLEN)) ;

	                if (cp != NULL) {

#if	CF_DEBUGS
	                    debugprintf("srvtab_fileparse: good compile\n") ;
#endif

	                    se.matchcode = cp ;
	                    se.matchlen = SRVTAB_RGXLEN ;

	                } else {

#if	CF_DEBUGS
	                    debugprintf("srvtab_fileparse: bad compile\n") ;
#endif

	                    se.matchlen = 0 ;
	                    freeit(&se.matchcode) ;

	                } /* end if (compiling RE) */

	            } else
	                se.matchcode = NULL ;
#endif /* SRVTAB_REGEX */

		    } /* end if (new entry) */

/* see if there is a service key on this same line */

		    if (rs >= 0)
	            fl = field_get(&fsb,key_terms,&fp) ;

	        } /* end if (a new service) */

/* loop while we have additional fields on this line */

	        while ((rs >= 0) && (fl >= 0)) {
		    int	ki = matstr(srvkeys,fp,fl) ;

#if	CF_DEBUGS
	            debugprintf("srvtab_fileparse: srvkeys i=%d\n", i) ;
#endif

	            if (fsb.term != ',') {

			fp = linebuf2 ;
	                fl = field_srvarg(&fsb,saterms,linebuf2,LINEBUFLEN) ;

#if	CF_DEBUGS
	                debugprintf("srvtab_fileparse: fieldarg=>%t<\n",
	                    fp,fl) ;
#endif

	            } else
	                fp = NULL ;

	            switch (ki) {

	            case srvkey_program:
	                if (fp != NULL) {
			    cl = sfshrink(fp,fl,&cp) ;
			    if (cl > 0) {
	                        freeit(&se.program) ;
	                        se.program = mallocstrw(cp,cl) ;
			    }
	                }
	                break ;

	            case srvkey_arguments:
	            case srvkey_args:
	                if (fp != NULL) {
	                    freeit(&se.args) ;
	                    se.args = mallocstrw(fp,fl) ;
	                }
	                break ;

	            case srvkey_username:
	                if ((fp != NULL) && (fl > 0)) {
	                    freeit(&se.username) ;
	                    se.username = mallocstrw(fp,fl) ;
	                }
	                break ;

	            case srvkey_groupname:
	                if ((fp != NULL) && (fl > 0)) {
	                    freeit(&se.groupname) ;
	                    se.groupname = mallocstrw(fp,fl) ;
	                }
	                break ;

	            case srvkey_options:
	            case srvkey_opts:
	                if (fp != NULL) {
	                    freeit(&se.options) ;
	                    se.options = mallocstrw(fp,fl) ;
	                }
	                break ;

/* groups were specified */
	            case srvkey_groups:
	                if (se.ngroups < 0)
	                    se.ngroups = 0 ;

	                if ((fl >= 0) && (fp != NULL))
	                    entry_groupsload(&se,fp,fl) ;

	                while ((fsb.ll > 0) && (fsb.term != ',')) {

			    fp = linebuf2 ;
	                    fl = field_srvarg(&fsb,saterms,
				linebuf2,LINEBUFLEN) ;

	                    if (fl > 0)
	                        entry_groupsload(&se,fp,fl) ;

	                } /* end while */

	                break ;

	            case srvkey_access:
	                if (fp != NULL) {
	                    freeit(&se.access) ;
	                    se.access = mallocstrw(fp,fl) ;
	                }
	                break ;

	            case srvkey_interval:
	                if (fp != NULL) {
	                    freeit(&se.interval) ;
	                    se.interval = mallocstrw(fp,fl) ;
	                }
	                break ;

	            case srvkey_addr:
	                if (fp != NULL) {
	                    stradd(&se.addr,fp,fl) ;
	                } /* end if (getting address) */
	                break ;

	            case srvkey_passfile:
	                if (fp != NULL) {
	                    freeit(&se.pass) ;
	                    se.pass = mallocstrw(fp,fl) ;
	                }
	                break ;

	            case srvkey_project:
	                if (fp != NULL) {
	                    freeit(&se.project) ;
	                    se.project = mallocstrw(fp,fl) ;
	                }
	                break ;

	            } /* end switch */

		    if (rs < 0) break ;

	            if (fsb.term == '#')
	                break ;

/* is there another service key on this line */

	            fl = field_get(&fsb,key_terms,&fp) ;

	        } /* end while (looping on fields) */

	    } /* end if (non-empty field) */

	    field_finish(&fsb) ;
	    } /* end if (field) */

	    if (rs < 0)
	        break ;

	} /* end while (reading lines) */

	if ((rs >= 0) && f_ent) {
	    if (entry_enough(&se) > 0) {
	        rs = vecitem_add(slp, &se,sizeof(SRVTAB_ENT)) ;
	        if (rs >= 0) f_ent = FALSE ;
	    }
	    if (f_ent) {
		f_ent = FALSE ;
	        entry_finish(&se) ;
	    }

	} /* end if (entry) */

	if (rs < 0)
	    goto badadd ;

	rs = vecitem_count(slp) ;
	n = rs ;

	if ((rs >= 0) && (! f_closed) && (op->fd < 0)) {
	    int	fd ;

	    if (bcontrol(sfp,BC_FD,&fd) >= 0) {

	        if (daytime <= 0)
	            daytime = time(NULL) ;

	        op->opentime = daytime ;
	        op->fd = u_dup(fd) ;

	        uc_closeonexec(op->fd,TRUE) ;

	    } /* end if (caching FD and setting CLOEXEC) */

	} /* end if (caching FD) */

	bclose(sfp) ;

ret0:

#if	CF_DEBUGS
	debugprintf("srvtab_fileparse: ret rs=%d n=%u\n", rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;

/* done with configuration file processing */
badadd:
	srvtab_filedump(op) ;

	if (f_ent) {
	    f_ent = FALSE ;
	    entry_finish(&se) ;
	}

	bclose(sfp) ;

badopen:
	if (op->fd >= 0) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	}

	goto ret0 ;
}
/* end subroutine (srvtab_fileparse) */


static int srvtab_filedump(op)
SRVTAB		*op ;
{
	SRVTAB_ENT	*ep ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	for (i = 0 ; vecitem_get(&op->e,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

	    rs1 = entry_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = vecitem_del(&op->e,i--) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	return rs ;
}
/* end subroutine (srvtab_filedump) */


/* initialize an entry */
static int entry_start(sep)
SRVTAB_ENT	*sep ;
{


	memset(sep,0,sizeof(SRVTAB_ENT)) ;

	return 0 ;
}
/* end subroutine (entry_start) */


/* free up an entry */
static int entry_finish(sep)
SRVTAB_ENT	*sep ;
{
	int	i ;


#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: service=%s SERVICE=%p\n",
	    sep->service, sep->service) ;
#endif

	if (sep->service == NULL)
	    return SR_OK ;

	freeit(&sep->service) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: matchcode\n") ;
#endif

	freeit(&sep->matchcode) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: program\n") ;
#endif

	freeit(&sep->program) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: args\n") ;
#endif

	freeit(&sep->args) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: options\n") ;
#endif

	freeit(&sep->options) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: username\n") ;
#endif

	freeit(&sep->username) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: groupname\n") ;
#endif

	freeit(&sep->groupname) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: groupnames\n") ;
#endif

	for (i = 0 ; sep->groupnames[i] != NULL ; i += 1)
	    freeit(sep->groupnames + i) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: access\n") ;
#endif

	freeit(&sep->access) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: interval\n") ;
#endif

	freeit(&sep->interval) ;

#if	CF_DEBUGS
	debugprintf("srvtab/entry_finish: ret\n") ;
#endif

	freeit(&sep->addr) ;

	freeit(&sep->pass) ;

	sep->matchlen = 0 ;
	return 0 ;
}
/* end subroutine (entry_finish) */


/* is there enough of a service entry to keep it? */
static int entry_enough(sep)
SRVTAB_ENT	*sep ;
{


	if ((sep->service == NULL) || (sep->service[0] == '\0'))
	    return FALSE ;

	if ((sep->program != NULL) && (sep->program[0] != '\0'))
	    return TRUE ;

	if ((sep->args != NULL) && (sep->args[0] != '\0'))
	    return TRUE ;

	return FALSE ;
}
/* end subroutine (entry_enough) */


/* load up some groups into the current entry */
static int entry_groupsload(sep,buf,buflen)
SRVTAB_ENT	*sep ;
const char	buf[] ;
int		buflen ;
{
	FIELD	fsb ;

	int	rs ;
	int	fl ;

	const char	*fp ;


	if ((rs = field_start(&fsb,buf,buflen)) >= 0) {

	while ((fl = field_get(&fsb,key_terms,&fp)) >= 0) {
	    if (fl == 0) continue ;

	    rs = entry_groupadd(sep,fp) ;
	    if (rs < 0) break ;

	} /* end while */

	field_finish(&fsb) ;
	} /* end if (field) */

ret0:
	return rs ;
}
/* end subroutine (entry_groupsload) */


/* add another (single) group to the current entry */
static int entry_groupadd(sep,name)
SRVTAB_ENT	*sep ;
const char	name[] ;
{
	int	rs = SR_OK ;
	int	i ;


/* enter the raw group name into a group slot */

	i = 0 ;
	while ((i < NGROUPS_MAX) && (sep->groupnames[i] != NULL))
	    i += 1 ;

	rs = i ;
	if (i < NGROUPS_MAX) {
	    const char	*sp ;

	    rs = uc_mallocstrw(name,-1,&sp) ;

	    if (rs >= 0) {
	        sep->groupnames[i++] = sp ;
	        sep->groupnames[i] = NULL ;
	    }

	} else
	    rs = SR_TOOBIG ;

	if (rs >= 0)
	    sep->ngroups = i ;

	return rs ;
}
/* end subroutine (entry_groupadd) */


/* add something to an existing string */
static int stradd(spp,s,slen)
const char	**spp ;
const char	s[] ;
int		slen ;
{
	int	rs = SR_OK ;
	int	sl ;
	int	len = 0 ;

	const char	*sp = *spp ;


	len = (slen + 1) ;
	if (sp != NULL) {
	    char	*cp ;
	    char	*osp = (char *) sp ;

	    sl = strlen(sp) ;

	    len += (sl + 1) ;
	    rs = uc_realloc(osp,len,&cp) ;

	    if (rs >= 0) {

		*spp = cp ;
		cp += sl ;
		*cp++ = CH_US ;
	        strwcpy(cp,s,slen) ;

	    } else
	        *spp = NULL ;

	} else {
	    rs = uc_mallocstrw(s,slen,&sp) ;
	    len = rs ;
	    if (rs >= 0) *spp = sp ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (stradd) */


static void freeit(pp)
const char	**pp ;
{


	if (*pp != NULL) {

#ifdef	MALLOCLOG
	    {
	        char	pbuf[100 + 1] ;
	        bufprintf(pbuf,100,"srvtab/freeit >%t<",*pp,strnlen(*pp,40)) ;
	        malloclog_free(*pp,pbuf) ;
	    }
#endif

	    uc_free(*pp) ;

	    *pp = NULL ;
	}
}
/* end subroutine (freeit) */


