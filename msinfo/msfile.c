/* msfile */

/* object to manipulate a MSFILE file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGTEST	0		/* special debugging for test */
#define	CF_SAFE		1		/* safer? */
#define	CF_CREAT	0		/* always create the file? */
#define	CF_LOCKF	0		/* use 'lockf(3c)' */
#define	CF_SOLARISBUG	1		/* work around Solaris MMAP bug */
#define	CF_NIENUM	0		/* perform NI updates on ENUM */
#define	CF_NISEARCH	0		/* perform NI updates on SEARCH */


/* revision history:

	= 1999-06-01, David A­D­ Morano
	This subroutine was originally written.

	= 2003-06-26, David A­D­ Morano
        Although this object works, it was only a micracle that it did. There is
        a feature-bug in Solaris that doesn't allow a file to be both mapped and
        locked at the same time (in either order). But there seems to be a crack
        in the stupid Solaris implementation because it doesn't enforce its
        stupid bug carefully enough and this object here fell through the cracks
        and continued working by accident. We were locking the whole file beyond
        its end and that appears to get by the Solaris police-state bug-patrol
        and was accidentally being allowed. I reworked a good bit of this code
        to eliminate any file mapping (so that we can continue to use
        file-record locks). This whole Solaris crap (this is being done on
        Solaris 8 right now) is really a pain and Sun should face punitive
        charges for inhumanity to the programmer community. Solaris has some
        nice things since it was derived from the older (and better) System V
        UNIX®, but has really messed it up by not allowing what used to be
        allowed in the old days with things like the old RFS facility. Oh, while
        we're on the subject: NFS sucks cock meat!

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine maintains a MSFILE file. This file is used to maintain
        machine status for nodes in the local machine cluster.

	Format of file records:

	- (see the 'entry' structure in the header)

	Design note: 

        In summary, Solaris sucks cock meat! Solaris does not allow a file to be
        memory-mapped from an NFS remote server AND also be file-locked at the
        same time. A lot of stupid Solaris documentation notes say something to
        the effect that the Solaris VM system cannot handle a remote file that
        is both mapped and subject to file-locking at the same time. They use
        some sort of stupid circular reasoning that if any file is being
        file-locked, then obviously it cannot be memory-mapped since the file
        locking indicates that file-locking is taking place, and that obviously
        any file that is being file-locked cannot therefore also be memory
        mapped. That is pretty much their reasoning -- I kid you not!

        Unfortunately, code, like this code here, that was first designed under
        System V UNIX® that used file-locking AND memory mapping together really
        needs to be changed to eliminate either the file locking or the memory
        mapping. Remote files were cross mounted in the late 80s and very early
        90s using RFS (not stupid NFS). The use of RFS provided many advantages
        not the least of them being full UFS file-system semantics, but it is
        not clear why Solaris took a step backward from simply allowing remote
        files to be both memory-mapped and file-locked at the same time. Some
        bright light-bulb of a software developer must have gotten his underwear
        in a bunch at some point and decided to disallow both of these from ever
        occurring at the same time in Solaris. We all have suffered from these
        dumb-butt Solaris developers since we have to take time out to
        re-debug-write old code (like this code here) to handle the case of
        stupid Solaris not allowing memory mapping for a file that is also
        file-locked.

	Implementation note:

        The code was actually running when files were being locked in their
        entirety and beyond their ends. There was some sort of loop-hole in the
        stupid Solaris code that allowed a file to be both file-locked and
        memory mapped at the same time under certain circumstances. However,
        there seemed to be problems with this code when other parties on other
        (remote) systems tried to do the same thing. They sometimes failed with
        dead-lock types of errors (I forget the details). As a result, I decided
        to change the code to fully comply with the stupid Solaris requirements
        that no remote file be both memory mapped and file locked at the same
        time. Any code that is here now that has to be with mapping of files is
        really just code that now allocates local private memory. This is done
        instead of using the process heap but was really done because it seemed
        to offer the minimal changes to the code to get a private memory access
        to the file while still retaining the ability to file-lock the file.
        Finally, let me finish with the comment that Solaris sucks cock meat.

	Final note:

        Solaris sucks cock meat! Give me back simultaneous memory mapping and
        file locking. And while you're at it, give me back RFS also! And to you
        stupid Solaris VM developers, get out of Solaris development. Either get
        a new job somewhere else or think about committing suicide. Either way,
        we can all be happier with one (or more) of those alternatives.

	Anecdotal note:

        Hey, you stupid Solaris developers: give me back the ability to push
        SOCKMOD on a TPI endpoint also! Since you're so stupid, I know that you
        forgot that this was possible at one time. You hosed that ability away
        when you botched up making Solaris 2.6.


*******************************************************************************/


#define	MSFILE_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<inttypes.h>
#include	<limits.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<mapstrint.h>
#include	<stdorder.h>
#include	<localmisc.h>

#include	"msfile.h"
#include	"msfilee.h"
#include	"msflag.h"
#include	"ebuf.h"


/* local defines */

#define	MSFILE_IDOFF		0
#define	MSFILE_HEADTABOFF	(MSFILE_IDOFF + (16 + sizeof(uint)))
#define	MSFILE_TABOFF		(MSFILE_HEADTABOFF + (3 * sizeof(uint)))

#define	MSFILE_ENTSIZE		MSFILEE_SIZE
#define	MSFILE_MAXFILESIZE	(4 * 1024 * 1024)
#define	MSFILE_NWAYS		4
#define	MSFILE_NEPW		(((8*1024) / MSFILE_ENTSIZE) - 1)
#define	MSFILE_NIDXENT		100

#if	CF_DEBUGTEST
#define	TO_OPEN		(5 * 60)	/* maximum file-open time */
#define	TO_ACCESS	(2 * 60)	/* maximum access idle time */
#define	TO_LOCK		30		/* seconds */
#else
#define	TO_OPEN		(60 * 60)	/* maximum file-open time */
#define	TO_ACCESS	(2 * 60)	/* maximum access idle time */
#define	TO_LOCK		30		/* seconds */
#endif /* CF_DEBUGTEST */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	mkmagic(char *,int,cchar *) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	getfstype(char *,int,int) ;
extern int	iceil(int,int) ;
extern int	islocalfs(const char *,int) ;
extern int	isValidMagic(cchar *,int,cchar *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(cchar *,int,int) ;
extern int	stroflags(char *,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* forward references */

static int	msfile_fileopen(MSFILE *,time_t) ;
static int	msfile_fileclose(MSFILE *) ;
static int	msfile_filesetinfo(MSFILE *) ;
static int	msfile_lockget(MSFILE *,time_t,int) ;
static int	msfile_lockrelease(MSFILE *) ;
static int	msfile_filebegin(MSFILE *,time_t) ;
static int	msfile_acquire(MSFILE *,time_t,int) ;
static int	msfile_filecheck(MSFILE *) ;
static int	msfile_ebufstart(MSFILE *) ;
static int	msfile_ebuffinish(MSFILE *) ;

static int	msfile_filetopwrite(MSFILE *) ;
static int	msfile_filetopread(MSFILE *) ;
static int	msfile_fileverify(MSFILE *) ;
static int	msfile_headtab(MSFILE *,int) ;

static int	msfile_findname(MSFILE *,const char *,int,char **) ;
static int	msfile_search(MSFILE *,const char *,int,char **) ;
static int	msfile_readentry(MSFILE *,int,char **) ;

#if	CF_NISEARCH
static int	msfile_index(MSFILE *,const char *,int,int) ;
#endif

static int	msfile_headwrite(MSFILE *) ;

static int	namematch(const char *,const char *,int) ;


/* local variables */


/* exported subroutines */


int msfile_open(MSFILE *op,cchar *fname,int oflags,mode_t operm)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		f_created = FALSE ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("msfile_open: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("msfile_open: fname=%s\n", fname) ;
	    stroflags(timebuf,oflags) ;
	    debugprintf("msfile_open: oflags=%s\n",timebuf) ;
	    if (oflags & O_CREAT)
	        debugprintf("msfile_open: creating as needed\n") ;
	}
#endif /* CF_DEBUGS */

#if	CF_CREAT
	oflags |= O_CREAT ;
#endif
	oflags &= (~ O_TRUNC) ;

	memset(op,0,sizeof(MSFILE)) ;
	op->pagesize = getpagesize() ;
	op->fd = -1 ;
	op->oflags = oflags ;
	op->operm = operm ;

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    op->fname = cp ;
	    op->ti_mod = 0 ;
	    if ((rs = msfile_fileopen(op,dt)) >= 0) {
	        if ((rs = msfile_filebegin(op,dt)) >= 0) {
	            const int	n = MSFILE_NIDXENT ;
	            if ((rs = mapstrint_start(&op->ni,n)) >= 0) {
	                op->magic = MSFILE_MAGIC ;
	            }
	        }
	        if (rs < 0) {
	            msfile_fileclose(op) ;
	        }
	    }
	    if (rs < 0) {
	        uc_free(op->fname) ;
	        op->fname = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("msfile_open: ret rs=%d f_created=%u\n",rs,f_created) ;
#endif

	return (rs >= 0) ? f_created : rs ;
}
/* end subroutine (msfile_open) */


int msfile_close(MSFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("msfile_close: mapstrint_finish() \n") ;
#endif

	rs1 = mapstrint_finish(&op->ni) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("msfile_close: _fileclose() \n") ;
#endif

	rs1 = msfile_fileclose(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("msfile_close: uc_free() \n") ;
#endif

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("msfile_close: ret rs=%d\n") ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (msfile_close) */


/* get a count of the number of entries */
int msfile_count(MSFILE *op)
{
	int		rs = SR_OK ;
	int		c ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	c = ((op->filesize - MSFILE_TABOFF) / MSFILE_ENTSIZE) ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msfile_count) */


/* initialize a cursor */
int msfile_curbegin(MSFILE *op,MSFILE_CUR *curp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (curp == NULL) return SR_FAULT ;

	op->ncursors += 1 ;
	op->f.cursorlockbroken = FALSE ;
	op->f.cursoracc = FALSE ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (msfile_curbegin) */


/* free up a cursor */
int msfile_curend(MSFILE *op,MSFILE_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (curp == NULL) return SR_FAULT ;

	if (op->f.cursoracc) {
	    op->ti_access = time(NULL) ;
	} /* end if */

	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	if ((op->ncursors == 0) && (op->f.lockedread || op->f.lockedwrite)) {
	    rs1 = msfile_lockrelease(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	curp->i = -1 ;
	return rs ;
}
/* end subroutine (msfile_curend) */


/* enumerate the entries */
int msfile_enum(MSFILE *op,MSFILE_CUR *curp,MSFILE_ENT *ep)
{
	time_t		dt = 0 ;
	const int	ebs = MSFILE_ENTSIZE ;
	int		rs = SR_OK ;
	int		ei = 0 ;

#if	CF_DEBUGS
	debugprintf("msfile_enum: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (curp == NULL) return SR_FAULT ;

	if (dt == 0) dt = time(NULL) ;

	if ((rs = msfile_acquire(op,dt,1)) >= 0) {
	    char	*bp ;
	    ei = (curp->i < 0) ? 0 : curp->i + 1 ;
	    if ((rs = msfile_readentry(op,ei,&bp)) >= 0) {
	        if ((ep != NULL) && (bp != NULL)) {
	            rs = msfilee_all(ep,1,bp,ebs) ;
	        }
	        if (rs >= 0) {
	            curp->i = ei ;
	            op->f.cursoracc = TRUE ;
	        }
	    }
	} /* end if (msfile_acquire) */

#if	CF_DEBUGS
	debugprintf("msfile_enum: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msfile_enum) */


/* match on a nodename */
int msfile_match(MSFILE *op,time_t dt,cchar *nnp,int nnl,MSFILE_ENT *ep)
{
	const int	ebs = MSFILE_ENTSIZE ;
	int		rs = SR_OK ;
	int		i = MSFILE_NODENAMELEN ;
	int		ei = 0 ;
	char		*bp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (nnp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("msfile_match: ent nnl=%d nnp=%t\n",
	    nnl,nnp,strnlen(nnp,nnl)) ;
#endif

	if (nnl >= 0)
	    i = MIN(nnl,MSFILE_NODENAMELEN) ;

	nnl = strnlen(nnp,i) ;

#if	CF_DEBUGS
	debugprintf("msfile_match: nnl=%d nnp=%t\n",nnl,nnp,nnl) ;
#endif

	if (dt == 0) dt = time(NULL) ;

	if ((rs = msfile_acquire(op,dt,1)) >= 0) {

	    rs = msfile_findname(op,nnp,nnl,&bp) ;
	    ei = rs ;

	    if ((rs >= 0) && (ep != NULL)) {

#if	CF_DEBUGS 
	        debugprintf("msfile_match: found it rs=%d\n",rs) ;
#endif

	        rs = msfilee_all(ep,1,bp,ebs) ;

#if	CF_DEBUGS 
	        debugprintf("msfile_match: rs1=%d ep->nodename=%s\n",
	            rs,ep->nodename) ;
#endif

	    } /* end if */

/* if we are a writer (open for write), update the access time also */

	    if ((rs >= 0) && op->f.writable) {
	        MSFILEE_ATIME	a ;
	        if (dt == 0) dt = time(NULL) ;
	        a.atime = dt ;
	        msfilee_atime(&a,0,bp,ebs) ;
	        if ((rs = ebuf_write(&op->ebm,ei,NULL)) >= 0) {
	            rs = ebuf_sync(&op->ebm) ;
		}
	    } /* end if (updating access time) */

/* optionally release our lock if we didn't have a cursor outstanding */

	    if (op->ncursors == 0) {
	        msfile_lockrelease(op) ;
	        if (dt == 0) dt = time(NULL) ;
	        op->ti_access = dt ;
	    } else {
	        op->f.cursoracc = TRUE ;
	    }

	} /* end if (msfile_acquire) */

#if	CF_DEBUGS
	debugprintf("msfile_match: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msfile_match) */


/* write an entry (match on a nodename) */
int msfile_write(MSFILE *op,time_t dt,cchar *nnp,int nnl,MSFILE_ENT *ep)
{
	const int	ebs = MSFILE_ENTSIZE ;
	int		rs = SR_OK ;
	int		i = MSFILE_NODENAMELEN ;
	int		ei ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("msfile_write: ent nnl=%d nodename=%t\n",
	    nnl,nnp,strnlen(nnp,nnl)) ;
#endif /* CF_DEBUGS */

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (nnp == NULL) return SR_FAULT ;

	if (nnl >= 0)
	    i = MIN(nnl,MSFILE_NODENAMELEN) ;

	nnl = strnlen(nnp,i) ;

#if	CF_DEBUGS && 0
	debugprintf("msfile_write: nodename=%t\n",nnp,nnl) ;
#endif

	if ((rs = msfile_acquire(op,dt,1)) >= 0) {
	    if ((rs = msfile_findname(op,nnp,nnl,&bp)) >= 0) {
	        ei = rs ;

/* write the entry */

	        if (ep != NULL) {
	            MSFILEE_ALL	ew ;
		    int	f = FALSE ;
	            f = f || (ep->dtime == 0) ;
		    f = f || (ep->atime == 0) ;
		    f = f || (ep->stime == 0) ;
		    if (f) {
	                MSFILEE_DTIME	ed ;
	                MSFILEE_ATIME	ea ;
	                MSFILEE_STIME	es ;
	                int		f_swap = FALSE ;

	                if (ep->dtime == 0) {
	                    msfilee_dtime(&ed,1,bp,ebs) ;
	                    if (ed.dtime != 0) {
	                        if (! f_swap) {
	                            f_swap = TRUE ;
	                            ew = *ep ;
	                        }
	                        if (dt >= ed.dtime) {
	                            ew.flags &= (~ MSFLAG_MDISABLED) ;
	                            ew.dtime = 0 ;
	                        } else {
	                            ew.dtime = ed.dtime ;
	                        }
	                    } /* end if */
	                } /* end if (dtime) */

	                if (ep->atime == 0) {
	                    msfilee_atime(&ea,1,bp,ebs) ;
	                    if (ea.atime != 0) {
	                        if (! f_swap) {
	                            f_swap = TRUE ;
	                            ew = *ep ;
	                        }
	                        ew.atime = ea.atime ;
	                    } /* end if */
	                } /* end if (atime) */

	                if (ep->stime == 0) {
	                    msfilee_stime(&es,1,bp,ebs) ;
	                    if (es.stime != 0) {
	                        if (! f_swap) {
	                            f_swap = TRUE ;
	                            ew = *ep ;
	                        }
	                        ew.stime = es.stime ;
	                    } /* end if */
	                } /* end if (stime) */

	                if (f_swap)
	                    ep = &ew ;

	            } /* end if (some special handling) */

	            msfilee_all(ep,0,bp,ebs) ;

	            rs = ebuf_write(&op->ebm,ei,NULL) ; /* sync */

	        } /* end if (writing entry) */

/* update the file header-table (for a write) */

	        if ((rs >= 0) && op->f.writable) {
	            if (dt == 0) dt = time(NULL) ;
	            op->h.wcount += 1 ;
	            op->h.wtime = dt ;
	            if ((rs = msfile_headwrite(op)) >= 0) {
	                rs = ebuf_sync(&op->ebm) ;
	            }
	        } /* end if (updating header-table) */

/* optionally release our lock if we didn't have a cursor outstanding */

	        if (op->ncursors == 0) {
	            msfile_lockrelease(op) ;
	        }

/* update access time as appropriate */

	        if (op->ncursors == 0) {
	            if (dt == 0) dt = time(NULL) ;
	            op->ti_access = dt ;
	        } else {
	            op->f.cursoracc = TRUE ;
	        }

	    } /* end if (msfile_findname) */
	} /* end if (msfile_acquire) */

#if	CF_DEBUGS
	debugprintf("msfile_write: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msfile_write) */


/* update an entry */
int msfile_update(MSFILE *op,time_t dt,MSFILE_ENT *ep)
{
	int		rs = SR_OK ;
	int		ei = 0 ;

#if	CF_DEBUGS
	debugprintf("msfile_update: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("msfile_update: writable=%u\n",op->f.writable) ;
#endif

	if ((rs = msfile_acquire(op,dt,1)) >= 0) {
	    int		nnl ;
	    int		f_newentry = FALSE ;
	    const char	*nnp = ep->nodename ;
	    const int	ebs = MSFILE_ENTSIZE ;
	    char	ebuf[MSFILE_ENTSIZE + 2] ;
	    char	*bp ;

/* do the search */

	    nnl = strnlen(nnp,MSFILE_NODENAMELEN) ;
	    if (nnl > 0) {

	        if ((rs = msfile_findname(op,nnp,nnl,&bp)) >= 0) {
	            MSFILEE_ALL	ew ;
	            int		f ;
	            ei = rs ;

/* found existing entry */

#if	CF_DEBUGS
	            debugprintf("msfile_update: existing entry\n") ;
#endif

	            f = FALSE ;
	            f = f || (ep->dtime == 0) ;
	            f = f || (ep->atime == 0) ;
	            f = f || (ep->stime == 0) ;
	            f = f || (ep->pid == 0) ;
	            if (f) { /* read-modify-write */

#ifdef	COMMENT
	                {
	                    MSFILEE_DTIME	ed ;
	                    MSFILEE_ATIME	ea ;
	                    MSFILEE_STIME	es ;
	                    int			f_swap = FALSE ;

	                    if (ep->dtime == 0) {
	                        msfilee_dtime(&ed,1,bp,ebs) ;
	                        if (ed.dtime != 0) {
	                            if (! f_swap) {
	                                f_swap = TRUE ;
	                                ew = *ep ;
	                            }
	                            if (dt >= ed.dtime) {
	                                ew.flags &= (~ MSFLAG_MDISABLED) ;
	                                ew.dtime = 0 ;
	                            } else {
	                                ew.dtime = ed.dtime ;
	                            }
	                        }
	                    } /* end if (dtime) */

	                    if (ep->atime == 0) {
	                        msfilee_atime(&ea,1,bp,ebs) ;
	                        if (ea.atime != 0) {
	                            if (! f_swap) {
	                                f_swap = TRUE ;
	                                ew = *ep ;
	                            }
	                            ew.atime = ea.atime ;
	                        }
	                    } /* end if (atime) */

	                    if (ep->stime == 0) {
	                        msfilee_stime(&es,1,bp,ebs) ;
	                        if (es.stime != 0) {
	                            if (! f_swap) {
	                                f_swap = TRUE ;
	                                ew = *ep ;
	                            }
	                            ew.stime = es.stime ;
	                        }
	                    } /* end if (stime) */

	                    if (f_swap)
	                        ep = &ew ;

	                }
#else /* COMMENT */
	                {
	                    MSFILEE_ALL	et ;

	                    ew = *ep ;
	                    msfilee_all(&et,1,bp,ebs) ;

	                    if (ep->dtime == 0) {
	                        if (et.dtime != 0) {
	                            if (dt >= et.dtime) {
	                                ew.flags &= (~ MSFLAG_MDISABLED) ;
	                                ew.dtime = 0 ;
	                            } else {
	                                ew.dtime = et.dtime ;
	                            }
	                        }
	                    } /* end if (dtime) */

	                    if (ep->atime == 0) {
	                        ew.atime = et.atime ;
	                    } /* end if (atime) */

	                    if (ep->stime == 0) {
	                        ew.stime = et.stime ;
	                    } /* end if (stime) */

	                    if (ep->pid == 0) {
	                        ew.pid = et.pid ;
	                    }

	                    ep = &ew ;
	                }
#endif /* COMMENT */

	            } /* end if (some special handling) */

	            msfilee_all(ep,0,bp,ebs) ;

	            rs = ebuf_write(&op->ebm,ei,NULL) ; /* sync */

#if	CF_DEBUGS
	            debugprintf("msfile_update: 0 ebuf_write() rs=%d\n",rs) ;
#endif

	        } else if (rs == SR_NOTFOUND) {
	            MSFILEE_ALL		ew ;

	            f_newentry = TRUE ;
	            if (ep->atime == 0) {
	                ew = *ep ;
	                ew.atime = ep->utime ;
	                ep = &ew ;
	            } /* end if (atime) */

	            msfilee_all(ep,0,ebuf,ebs) ;

	            ei = op->h.nentries ;
	            rs = ebuf_write(&op->ebm,ei,ebuf) ;

#if	CF_DEBUGS
	            debugprintf("msfile_update: 1 ebuf_pwrite() rs=%d\n",rs) ;
#endif

	        } /* end if (entry update) */

/* update the file header */

#if	CF_DEBUGS
	        debugprintf("msfile_update: header rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            if (dt == 0) dt = time(NULL) ;
	            op->h.wcount += 1 ;
	            op->h.wtime = dt ;
	            if (f_newentry) {
	                op->h.nentries += 1 ;
	                op->filesize += ebs ;
	            }
	            if ((rs = msfile_headwrite(op)) >= 0) {
	                if ((rs = ebuf_sync(&op->ebm)) >= 0) {
	                    if (op->ncursors == 0) {
	                        rs = msfile_lockrelease(op) ;
	                        if (dt == 0) dt = time(NULL) ;
	                        op->ti_access = dt ;
	                    } else {
	                        op->f.cursoracc = TRUE ;
	                    }
	                }
	            } /* end if (ok) */

	        } /* end if (updating header-table) */
	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if (msfile_acquire) */

#if	CF_DEBUGS
	debugprintf("msfile_update: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msfile_update) */


/* do some checking */
int msfile_check(MSFILE *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSFILE_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (op->fd >= 0) {
	    if ((! op->f.lockedread) && (! op->f.lockedwrite)) {
	        if (dt == 0) dt = time(NULL) ;
	        f = ((dt - op->ti_access) >= TO_ACCESS) ;
	        f = f || ((dt - op->ti_open) >= TO_OPEN) ;
	        if (f) {
	            rs = msfile_fileclose(op) ;
	        }
	    } /* end if (not locked) */
	} /* end if (open) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (msfile_check) */


/* private subroutines */


static int msfile_findname(MSFILE *op,cchar *nnp,int nnl,char **rpp)
{
	int		rs ;
	int		ei = 0 ;
	char		*bp = NULL ;
	char		*np ;

	if ((rs = mapstrint_fetch(&op->ni,nnp,nnl,NULL,&ei)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("msfile_update: mapstrint_fetch() rs=%d\n",rs) ;
	    if (rs >= 0)
	        debugprintf("msfile_update: ei=%d\n",ei) ;
#endif

	    if ((rs = ebuf_read(&op->ebm,ei,&bp)) > 0) {

#if	CF_DEBUGS
	        debugprintf("msfile_update: ebuf_read() rs=%d\n",rs) ;
#endif

	        np = bp + MSFILEE_ONODENAME ;
	        if (! namematch(np,nnp,nnl)) {
	            rs = SR_NOTFOUND ;
	            mapstrint_delkey(&op->ni,nnp,nnl) ;
	        } /* end if */

	    } else {
	        rs = SR_NOTFOUND ;
	    }

	} /* end if (was in the index) */

#if	CF_DEBUGS
	debugprintf("msfile_update: mid rs=%d\n",rs) ;
#endif

	if (rs == SR_NOTFOUND) {
	    if ((rs = msfile_search(op,nnp,nnl,&bp)) >= 0) {
	        ei = rs ;
	        rs = mapstrint_add(&op->ni,nnp,nnl,ei) ;
	    }
	} /* end if */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? bp : NULL ;
	}

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msfile_findname) */


/* search for an entry */
static int msfile_search(MSFILE *op,cchar *nnp,int nnl,char **rpp)
{
	const int	ebs = MSFILE_ENTSIZE ;
	int		rs = SR_OK ;
	int		i ;
	int		ne ;
	int		ei = 0 ;
	int		f_found ;
	char		*bp ;
	char		*np = NULL ;

#if	CF_DEBUGS
	debugprintf("msfile_search: ent nnl=%u nodename=%t\n",
	    nnl,nnp,strnlen(nnp,nnl)) ;
#endif

	if (nnl < 0)
	    nnl = strlen(nnp) ;

#if	CF_DEBUGS 
	debugprintf("msfile_search: nnl=%u pagesize=%u ebs=%u\n",
	    nnl,op->pagesize,ebs) ;
#endif

	ei = 0 ;
	ne = 0 ;
	f_found = FALSE ;
	while ((rs >= 0) && (! f_found)) {

	    rs = ebuf_read(&op->ebm,ei,&bp) ;
	    ne = rs ;

#if	CF_DEBUGS
	    debugprintf("msfile_search: ebuf_read() rs=%d\n",rs) ;
#endif

	    if (rs <= 0)
	        break ;

	    for (i = 0 ; (rs >= 0) && (i < ne) ; i += 1) {

	        np = (bp + MSFILEE_ONODENAME) ;

/* is this a match for what we want? */

#if	CF_DEBUGS
	        debugprintf("msfile_search: i=%u db_node=%t\n",i,
	            np,strnlen(np,MSFILEE_LNODENAME)) ;
#endif

	        if (namematch(np,nnp,nnl)) {
	            f_found = TRUE ;
	            break ;
	        }

	        bp += ebs ;

	    } /* end for (looping through entries) */

#if	CF_DEBUGS
	    debugprintf("msfile_search: for-out i=%d f_found=%u\n",
	        i,f_found) ;
#endif

	    ei += i ;

#if	CF_DEBUGS
	    debugprintf("msfile_search: ei=%d f_found=%u\n",ei,f_found) ;
#endif

	    if (f_found) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("msfile_search: fin rs=%d f_found=%u\n",rs,f_found) ;
	debugprintf("msfile_search: ne=%d \n",ne) ;
#endif

	if (rs >= 0) {
	    if ((ne != 0) && f_found) {
	        if (rpp != NULL) *rpp = bp ;
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("msfile_search: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msfile_search) */


static int msfile_acquire(MSFILE *op,time_t dt,int f_read)
{
	const int	ebs = MSFILE_ENTSIZE ;
	int		rs = SR_OK ;
	int		f_changed = FALSE ;
	int		f ;

#if	CF_DEBUGS
	debugprintf("msfile_acquire: ent\n") ;
#endif

	if (dt == 0)
	    dt = time(NULL) ;

/* is the file open? */

	if (op->fd < 0) {
	    rs = msfile_fileopen(op,dt) ;
	}

	if ((rs >= 0) && (! f_read) && op->f.lockedread) {
	    rs = msfile_lockrelease(op) ;
	}

/* capture the lock if we do not already have it */

	f = (op->f.lockedread || op->f.lockedwrite) ;
	if ((rs >= 0) && (! f)) {
	    if ((rs = msfile_lockget(op,dt,f_read)) >= 0) {
	        f_changed = (rs > 0) ;
	        if ((rs = msfile_filecheck(op)) >= 0) {
	            f_changed = f_changed || (rs > 0) ;
	            if (f_changed) {
	                int n = ((op->filesize - MSFILE_TOPLEN) / ebs) ;
	                rs = ebuf_invalidate(&op->ebm,n) ;
	            }
	        }
	    }
	} /* end if (need lock) */

#if	CF_DEBUGS
	debugprintf("msfile_acquire: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (msfile_acquire) */


/* initialize the file header (either read it only or write it) */
static int msfile_filebegin(MSFILE *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f_locked = FALSE ;

#if	CF_DEBUGS
	debugprintf("msfile_filebegin: ent fd=%d filesize=%u\n",
	    op->fd,op->filesize) ;
#endif

	if (op->filesize == 0) {

	    op->f.fileinit = FALSE ;
	    if (op->f.writable) {

	        if (op->f.lockedread) {
	            rs = msfile_lockrelease(op) ;
		}

	        if ((rs >= 0) && (! op->f.lockedwrite)) {
	            f_locked = TRUE ;
	            rs = msfile_lockget(op,dt,0) ;
	        }

	        if (rs >= 0) {
	            rs = msfile_filetopwrite(op) ;
	            f_locked = (rs > 0) ;
	        }

	    } /* end if (writable) */

	} else if (op->filesize >= MSFILE_TABOFF) {
	    int		f ;

/* read the file header */

	    f = (op->f.lockedread || op->f.lockedwrite) ;
	    if (! f) {
	        rs = msfile_lockget(op,dt,1) ;
	        f_locked = (rs >= 0) ;
	    }

#if	CF_DEBUGS
	    debugprintf("msfile_filebegin: msfile_fileverify() \n") ;
#endif

	    if (rs >= 0) {
	        if ((rs = msfile_filetopread(op)) >= 0) {
	            if ((rs = msfile_fileverify(op)) >= 0) {
	        	rs = msfile_headtab(op,1) ;
	        	op->f.fileinit = (rs >= 0) ;
		    }
		}
	    }

	} /* end if */

/* if we locked, we unlock it, otherwise leave it! */

	if (f_locked) {
	    msfile_lockrelease(op) ;
	}

#if	CF_DEBUGS
	debugprintf("msfile_filebegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msfile_filebegin) */


static int msfile_filecheck(MSFILE *op)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (op->filesize < MSFILE_TABOFF) {
	    f_changed = TRUE ;
	    if (op->f.writable) {
	        rs = msfile_filetopwrite(op) ;
	    }
	} else {
	    if ((rs = msfile_filetopread(op)) >= 0) {
	        if ((rs = msfile_fileverify(op)) >= 0) {
	            rs = msfile_headtab(op,1) ;
	            f_changed = (rs > 0) ;
	            op->f.fileinit = (rs >= 0) ;
		}
	    }
	} /* end if */

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (msfile_filecheck) */


static int msfile_filetopwrite(MSFILE *op)
{
	offset_t	poff = 0L ;
	int		ml ;
	int		rs = SR_OK ;
	int		bl ;
	char		*bp = op->topbuf ;

/* write the file header stuff */

	ml = mkmagic(bp,MSFILE_FILEMAGICSIZE,MSFILE_FILEMAGIC) ;
	bp += ml ;

	*bp++ = MSFILE_FILEVERSION ;
	*bp++ = MSFILE_ENDIAN ;
	*bp++ = 0 ;		/* file type */
	*bp++ = 0 ;		/* unused */

/* next is the header (we just write zeros here) */

	memset(bp,0,MSFILE_HEADTABLEN) ;
	bp += MSFILE_HEADTABLEN ;

	bl = bp - op->topbuf ;
	op->fileversion = MSFILE_FILEVERSION ;
	op->filetype = 0 ;
	memset(&op->h,0,sizeof(struct msfile_h)) ;

#if	CF_DEBUGS
	debugprintf("msfile_filebegin: u_write() wlen=%d\n",bl) ;
#endif

	if ((rs = u_pwrite(op->fd,op->topbuf,bl,poff)) >= 0) {
	    op->filesize = rs ;
	    op->topsize = rs ;
	    op->f.fileinit = TRUE ;
	}

#if	CF_DEBUGS
	debugprintf("msfile_filetopwrite: u_write() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msfile_filetopwrite) */


static int msfile_filetopread(MSFILE *op)
{
	offset_t	poff = 0L ;
	int		rs ;

	rs = u_pread(op->fd,op->topbuf,MSFILE_TOPLEN,poff) ;
	op->topsize = rs ;

	return rs ;
}
/* end subroutine (msfile_filetopread) */


/* verify the file */
static int msfile_fileverify(MSFILE *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("msfile_fileverify: ent\n") ;
	debugprintf("msfile_fileverify: filemagic=%s\n",
		MSFILE_FILEMAGIC) ;
	debugprintf("msfile_fileverify: filemagicsize=%u\n",
		MSFILE_FILEMAGICSIZE) ;
#endif

	if (op->topsize >= MSFILE_TOPLEN) {
	    const int	msize = MSFILE_FILEMAGICSIZE ;
	    cchar	*cp = op->topbuf ;
#if	CF_DEBUGS
	    debugprintf("msfile_fileverify: ms=%t\n",cp,strlinelen(cp,-1,40)) ;
#endif
	    if (isValidMagic(cp,msize,MSFILE_FILEMAGIC)) {
	        cp += msize ;
	        if (cp[0] <= MSFILE_FILEVERSION) {
	            op->fileversion = cp[0] ;
	            if (cp[1] == MSFILE_ENDIAN) {
	                op->filetype = cp[2] ;
	            } else {
	                rs = SR_NOTSUP ;
	            }
	        } else {
	            rs = SR_NOTSUP ;
	        }
	    } else {
	        rs = SR_BADFMT ;
	    }
	} else {
	    rs = SR_INVALID ;
	}

#if	CF_DEBUGS
	debugprintf("msfile_fileverify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msfile_fileverify) */


/* read or write the file header */
static int msfile_headtab(MSFILE *op,int f_read)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;
	char		*bp = (op->topbuf + MSFILE_HEADTABOFF) ;

	if (op->topbuf == NULL) return SR_BADFMT ;

	if (f_read) {
	    struct msfile_h	h ;

	    int	hsize = sizeof(struct msfile_h) ;

	    stdorder_ruint(bp,&h.nentries) ;
	    bp += sizeof(uint) ;

	    stdorder_ruint(bp,&h.wtime) ;
	    bp += sizeof(uint) ;

	    stdorder_ruint(bp,&h.wcount) ;
	    bp += sizeof(uint) ;

	    f_changed = (memcmp(&h,&op->h,hsize) != 0) ;
	    op->h = h ;

	} else {

	    stdorder_wuint(bp,op->h.nentries) ;
	    bp += sizeof(uint) ;

	    stdorder_wuint(bp,op->h.wtime) ;
	    bp += sizeof(uint) ;

	    stdorder_wuint(bp,op->h.wcount) ;
	    bp += sizeof(uint) ;

	} /* end if */

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (msfile_headtab) */


/* acquire access to the file */
static int msfile_lockget(MSFILE *op,time_t dt,int f_read)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		lockcmd ;
	int		f_already = FALSE ;
	int		f_changed = FALSE ;

#if	CF_DEBUGS && 0
	debugprintf("msfile_lockget: ent f_read=%d\n",f_read) ;
#endif

	if (op->fd < 0) {
	    rs = msfile_fileopen(op,dt) ;
	} /* end if (needed to open the file) */

/* acquire a file record lock */

	if (rs >= 0) {

	    if (f_read || (! op->f.writable)) {
	        f_already = op->f.lockedread ;
	        op->f.lockedread = TRUE ;
	        op->f.lockedwrite = FALSE ;
	        lockcmd = F_RLOCK ;
	    } else {
	        f_already = op->f.lockedwrite ;
	        op->f.lockedread = FALSE ;
	        op->f.lockedwrite = TRUE ;
	        lockcmd = F_WLOCK ;
	    } /* end if */

/* get out if we have the lock that we want already */

	    if (! f_already) {

/* we need to actually do the lock */

#if	CF_LOCKF
	        rs = uc_lockf(op->fd,F_LOCK,0L) ;
#else /* CF_LOCKF */
#if	CF_SOLARISBUG
	        rs = lockfile(op->fd,lockcmd,0L,0L,TO_LOCK) ;
#else
	        {
	            offset_t	fs = op->filesize ;
	            rs = lockfile(op->fd,lockcmd,0L,fs,TO_LOCK) ;
	        }
#endif /* CF_SOLARISBUF */
#endif /* CF_LOCKF */

#if	CF_DEBUGS && 0
	        debugprintf("msfile_lockget: LOCK lockfile() rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            if ((rs = u_fstat(op->fd,&sb)) >= 0) {

	                f_changed = 
	                    (sb.st_size != op->filesize) ||
	                    (sb.st_mtime != op->ti_mod) ;

	                if (f_changed) {
	                    if (op->f.bufvalid) op->f.bufvalid = FALSE ;
	                    op->filesize = sb.st_size ;
	                    op->ti_mod = sb.st_mtime ;
	                } /* end if */

	                if (op->filesize < MSFILE_TABOFF) {
	                    op->f.fileinit = FALSE ;
	                }

	            } else {
	                msfile_lockrelease(op) ;
	            }
	        } else {
	            msfile_lockrelease(op) ;
	        }

	    } /* end if (not already) */

	} /* end if (ok) */

#if	CF_DEBUGS && 0
	debugprintf("msfile_lockget: ret rs=%d f_changed=%u\n",
	    rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (msfile_lockget) */


static int msfile_lockrelease(MSFILE *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS && 0
	debugprintf("msfile_lockrelease: ent\n") ;
#endif

	if ((op->f.lockedread || op->f.lockedwrite)) {

	    if (op->fd >= 0) {

#if	CF_LOCKF
	        rs = uc_lockf(op->fd,F_ULOCK,0L) ;
#else /* CF_LOCKF */
#if	CF_SOLARISBUG
	        rs = lockfile(op->fd,F_ULOCK,0L,0L,TO_LOCK) ;
#else
	        {
	            offset_t	fs = op->filesize ;
	            rs = lockfile(op->fd,F_ULOCK,0L,fs,TO_LOCK) ;
	        }
#endif /* CF_SOLARISBUF */
#endif /* CF_LOCKF */

#if	CF_DEBUGS && 0
	        debugprintf("msfile_lockrelease: lockfile() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	        debugprintf("msfile_lockrelease: UNLOCK lockfile() rs=%d\n",
	            rs) ;
#endif

	    } /* end if (file was open) */

	    op->f.lockedread = FALSE ;
	    op->f.lockedwrite = FALSE ;

	} /* end if (there was a possible lock set) */

#if	CF_DEBUGS
	debugprintf("msfile_lockrelease: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msfile_lockrelease) */


static int msfile_fileopen(MSFILE *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f_created = FALSE ;

#if	CF_DEBUGS
	debugprintf("msfile_fileopen: fname=%s\n",op->fname) ;
#endif

	if (op->fd < 0) {
	    int		oflags = (op->oflags & (~ O_CREAT)) ;
	    op->f.fileinit = FALSE ;
	    op->f.lockedread = FALSE ;
	    op->f.lockedwrite = FALSE ;
	    rs = u_open(op->fname,oflags,op->operm) ;
	    op->fd = rs ;
	    if (isNotPresent(rs) && (op->oflags & O_CREAT)) {
	        f_created = TRUE ;
	        oflags = op->oflags ;
	        rs = u_open(op->fname,oflags,op->operm) ;
	        op->fd = rs ;
	    } /* end if (creating file) */
	    if (rs >= 0) {
	        if (dt == 0) dt = time(NULL) ;
	        op->ti_open = dt ;
	        if ((rs = uc_closeonexec(op->fd,TRUE)) >= 0) {
	            if ((rs = msfile_filesetinfo(op)) >= 0) {
	                rs = msfile_ebufstart(op) ;
	            }
	        }
	        if (rs < 0) {
	            u_close(op->fd) ;
	            op->fd = -1 ;
	            op->ti_open = 0 ;
	        }
	    } /* end if (ok) */
	} /* end if (open-needed) */

#if	CF_DEBUGS
	debugprintf("msfile_fileopen: ret rs=%d f_create=%u\n",rs,f_created) ;
#endif

	return (rs >= 0) ? f_created : rs ;
}
/* end subroutine (msfile_fileopen) */


int msfile_fileclose(MSFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.ebuf) {
	    rs1 = msfile_ebuffinish(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->fd >= 0) {
	    op->f.lockedread = FALSE ;
	    op->f.lockedwrite = FALSE ;
	    rs1 = u_close(op->fd) ;
	    op->fd = -1 ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (msfile_fileclose) */


static int msfile_filesetinfo(MSFILE *op)
{
	struct ustat	sb ;
	int		rs ;
	int		amode ;

	amode = (op->oflags & O_ACCMODE) ;
	op->f.writable = ((amode == O_WRONLY) || (amode == O_RDWR)) ;

#if	CF_DEBUGS
	debugprintf("msfile_open: f_writable=%u\n",op->f.writable) ;
#endif

	if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	    char	fstype[USERNAMELEN + 1] ;
	    op->ti_mod = sb.st_mtime ;
	    op->filesize = sb.st_size ;
	    if ((rs = getfstype(fstype,USERNAMELEN,op->fd)) >= 0) {
	        int	f = islocalfs(fstype,rs) ;
	        op->f.remote = (! f) ; /* remote if not local! */
	    }
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (msfile_filesetinfo) */


static int msfile_ebufstart(MSFILE *op)
{
	int		rs = SR_OK ;

	if (! op->f.ebuf) {
	    if (op->fd >= 0) {
	        uint	soff = MSFILE_TABOFF ;
	        int	esize = MSFILE_ENTSIZE ;
	        int	nways = MSFILE_NWAYS ;
	        int	n = MSFILE_NEPW ;
	        rs = ebuf_start(&op->ebm,op->fd,soff,esize,nways,n) ;
	        op->f.ebuf = (rs >= 0) ;
	    } else {
	        rs = SR_NOANODE ;
	    }
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (msfile_ebufstart) */


static int msfile_ebuffinish(MSFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.ebuf) {
	    op->f.ebuf = FALSE ;
	    rs1 = ebuf_finish(&op->ebm) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (msfile_ebuffinish) */


static int msfile_readentry(MSFILE *op,int ei,char **rpp)
{
	int		rs ;
	char		*bp ;

	if ((rs = ebuf_read(&op->ebm,ei,&bp)) >= 0) {
	    if (rs == 0) rs = SR_NOTFOUND ;
	    if (rpp != NULL) {
	        *rpp = (rs >= 0) ? bp : NULL ;
	    }

/* enter into the index */

#if	CF_NIENUM
	    if (rs >= 0) {
	        int	nl ;
	        char	*np ;
	        np = bp + MSFILEE_ONODENAME ;
	        nl = strnlen(np,MSFILEE_LNODENAME) ;
	        rs = msfile_index(op,np,nl,ei) ;
	    } /* end if (got an entry) */
#endif /* CF_NIENUM */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("msfile_readentry: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msfile_readentry) */


#if	CF_NISEARCH

/* add to the name-index if necessary */
static int msfile_index(MSFILE *op,cchar *np,int nl,int ei)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		ei2 ;

	if (nl < 0)
	    nl = strlen(np) ;

	rs1 = mapstrint_fetch(&op->ni,np,nl,NULL,&ei2) ;

	if ((rs1 >= 0) && (ei != ei2)) {
	    rs1 = SR_NOTFOUND ;
	    mapstrint_delkey(&op->ni,np,nl) ;
	}

	if (rs1 == SR_NOTFOUND) {
	    int	nl2 = strnlen(np,MSFILEE_LNODENAME) ;
	    mapstrint_add(&op->ni,np,nl2,ei) ;
	} /* end if (not found) */

	return rs ;
}
/* end subroutine (msfile_index) */

#endif /* CF_NISEARCH */


static int msfile_headwrite(MSFILE *op)
{
	int		rs ;

	if ((rs = msfile_headtab(op,0)) >= 0) {
	    offset_t	poff ;
	    const int	toff = MSFILE_TABOFF ;
	    const int	htoff = MSFILE_HEADTABOFF ;
	    int		bl ;
	    const char	*bp ;
	    bp = (op->topbuf + htoff) ;
	    bl = (toff - htoff) ;
	    poff = htoff ;
	    rs = u_pwrite(op->fd,bp,bl,poff) ;
	}

	return rs ;
}
/* end subroutine (msfile_headwrite) */


static int namematch(cchar *np,cchar *nnp,int nnl)
{
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("namematch: ent np=%s\n",np) ;
	debugprintf("namematch: nnl=%d nnp=%t\n",nnl,nnp,strnlen(nnp,nnl)) ;
#endif

	if (nnl <= MSFILEE_LNODENAME) {
	    f = (strncmp(np,nnp,nnl) == 0) ;
	    f = f && (np[nnl] == '\0') ;
	}

#if	CF_DEBUGS
	debugprintf("namematch: ret f=%u\n",f) ;
#endif

	return f ;
}
/* end subroutine (namematch) */


