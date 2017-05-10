/* ssh */

/* SS-hammock detection object */


#define	CF_DEBUGS	0
#define	CF_SAFE		0


/* revision history:

	= 02/05/01, David A­D­ Morano

	This object module was created for Levo research, to determine
	if a conditional branch at a given instruction address is
	a SS-Hamock or not.


*/


/******************************************************************************

	This object module provides an interface to a data base of
	information about SS-Hammock branchs.  A query can be made to
	retrieve information about a conditional branch as specified by
	its instruction address.


*****************************************************************************/


#define	SSH_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>		/* Memory Management */
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"ssh.h"



/* local defines */

#define	SSH_MAGIC	0x23456787

#define	MODP2(v,n)	((v) & ((n) - 1))

#ifndef	ENDIAN
#if	defined(SOLARIS) && defined(__sparc)
#define	ENDIAN		1
#else
#ifdef	_BIG_ENDIAN
#define	ENDIAN		1
#endif
#ifdef	_LITTLE_ENDIAN
#define	ENDIAN		0
#endif
#ifndef	ENDIAN
#error	"could not determine endianness of this machine"
#endif
#endif
#endif



/* external subroutines */

extern uint	nextpowtwo(uint) ;
extern uint	hashelf(const void *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static int	hashindex(uint,uint) ;






int ssh_init(op,fname)
SSH		*op ;
char		fname[] ;
{
	struct ustat	sb ;

	uint	*table ;

	int	rs, fd ;
	int	prot, flags ;
	int	magiclen ;

	char	*cp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (fname == NULL)
	    return SR_FAULT ;

	(void) memset(op,0,sizeof(SSH)) ;

	rs = u_open(fname,O_RDONLY,0666) ;

	if (rs < 0)
	    goto bad0 ;

	fd = rs ;
	rs = u_fstat(fd,&sb) ;

	if (rs < 0)
	    goto bad1 ;

	op->filesize = sb.st_size ;

	prot = PROT_READ ;
	flags = MAP_SHARED ;
	rs = u_mmap(NULL,(size_t) op->filesize,prot,flags,
	    fd,0L,&op->filemap) ;

	if (rs < 0)
	    goto bad1 ;

/* OK, check it out */

	magiclen = strlen(SSH_FILEMAGIC) ;

	cp = (char *) op->filemap ;
	if (strncmp(cp,SSH_FILEMAGIC,magiclen) != 0) {

#if	CF_DEBUGS
	debugprintf("ssh_init: bad magic=>%t<\n",
		cp,strnlen(cp,14)) ;
#endif

	    rs = SR_BADFMT ;
	    goto bad2 ;
	}

	cp += 16 ;
	if (cp[0] > SSH_FILEVERSION) {

	    rs = SR_NOTSUP ;
	    goto bad2 ;
	}

	if (cp[1] != ENDIAN) {

	    rs = SR_NOTSUP ;
	    goto bad2 ;
	}

/* if looks good, read the header stuff */

	table = (uint *) (op->filemap + 16 + 4) ;

	op->rectab = (SSH_ENT *) (op->filemap + table[0]) ;
	op->rtlen = table[1] ;

	op->recind = (uint (*)[2]) (op->filemap + table[2]) ;
	op->rilen = table[3] ;

#if	CF_DEBUGS
	debugprintf("ssh_init: rtlen=%d rilen=%d\n",op->rtlen,op->rilen) ;
#endif

/* we're out of here */

	op->magic = SSH_MAGIC ;

ret1:
	u_close(fd) ;

ret0:
	return rs ;

/* bad stuff comes here */
bad2:
	u_munmap(op->filemap,(size_t) op->filesize) ;

bad1:
	u_close(fd) ;

bad0:
	return rs ;
}
/* end subroutine (ssh_init) */


/* get the string count in the table */
int ssh_count(op)
SSH		*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SSH_MAGIC)
	    return SR_NOTOPEN ;

	return (op->rtlen - 1) ;
}
/* end subroutine (ssh_count) */


/* calculate the index table length (number of entries) at this point */
int ssh_countindex(op)
SSH		*op ;
{
	int	n ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SSH_MAGIC)
	    return SR_NOTOPEN ;

	return op->rilen ;
}
/* end subroutine (ssh_countindex) */


/* check on a branch */
int ssh_lookup(op,ia,rpp)
SSH		*op ;
uint		ia ;
SSH_ENT	**rpp ;
{
	uint	rhash ;
	uint	hi, ri ;

	int	rs ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;
#endif

/* this is needed for when no file was found */

	if (op->magic != SSH_MAGIC)
	    return SR_NOTOPEN ;

	if (rpp == NULL)
	    return SR_FAULT ;

	*rpp = NULL ;
	rhash = hashelf(&ia,sizeof(uint)) ;

#if	CF_DEBUGS
	debugprintf("ssh_lookup: rhash=%08x\n",rhash) ;
#endif

	hi = hashindex(rhash,op->rilen) ;

#if	CF_DEBUGS
	debugprintf("ssh_lookup: hi=%d\n",hi) ;
#endif

/* start searching ! */

	ri = op->recind[hi][0] ;
	if (ri == 0)
	    return SR_NOTFOUND ;

	while (op->rectab[ri].ia != ia) {

	    hi = op->recind[hi][1] ;
	    if (hi == 0)
	        break ;

	    ri = op->recind[hi][0] ;

	} /* end while */

	rs = SR_NOTFOUND ;
	if (op->rectab[ri].ia == ia) {

	    rs = SR_OK ;
	    *rpp = op->rectab + ri ;

	}

	return rs ;
}
/* end subroutine (ssh_lookup) */


/* get the entries (serially) */
int ssh_get(op,ri,rpp)
SSH		*op ;
int		ri ;
SSH_ENT	**rpp ;
{


#if	CF_DEBUGS
	debugprintf("ssh_get: entered 0\n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("ssh_get: entered 1\n") ;
#endif

	if (op->magic != SSH_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("ssh_get: entered 2\n") ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("ssh_get: entered ri=%d\n",ri) ;
#endif

	if ((ri < 0) || (ri >= op->rtlen))
		return SR_NOTFOUND ;

	*rpp = NULL ;
	if (ri > 0)
	    *rpp = op->rectab + ri ;

#if	CF_DEBUGS
	debugprintf("ssh_get: OK\n") ;
#endif

	return ri ;
}
/* end subroutine (ssh_get) */


/* get information about all static branches */
int ssh_info(op,rp)
SSH		*op ;
SSH_INFO	*rp ;
{
	int	rs = SR_NOTSUP ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SSH_MAGIC)
	    return SR_NOTOPEN ;

	if (rp == NULL)
	    return SR_FAULT ;

	memset(rp,0,sizeof(SSH_INFO)) ;

	rp->entries = op->rtlen ;
	rs = op->rtlen ;

	return rs ;
}
/* end subroutine (ssh_info) */


/* free up this ssh object */
int ssh_free(op)
SSH		*op ;
{
	int	rs = SR_BADFMT ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SSH_MAGIC)
	    return SR_NOTOPEN ;

	if (op->filemap != NULL)
	    rs = u_munmap(op->filemap,(size_t) op->filesize) ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (ssh_free) */



/* INTERNAL SUBROUTINES */



/* calculate the next hash from a given one */
static int hashindex(i,n)
uint	i, n ;
{
	int	hi ;


	hi = MODP2(i,n) ;

	if (hi == 0)
	    hi = 1 ;

	return hi ;
}
/* end if (hashindex) */



