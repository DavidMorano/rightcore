/* clusterdb */

/* perform access table file related functions */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGTEST	0		/* more */


/* revision history:

	- 2004-05-25, David A­D­ Morano
        This subroutine was generalized from a particular program for wider use.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object processes an access table for use by daemon multiplexing
        server programs that want to control access to their sub-servers.


*******************************************************************************/


#define	CLUSTERDB_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<hdb.h>
#include	<storeitem.h>
#include	<kvsfile.h>
#include	<localmisc.h>

#include	"clusterdb.h"


/* local defines */

#define	CLUSTERDB_MAGIC		0x31835926
#define	CLUSTERDB_MINCHECKTIME	5	/* file check interval (seconds) */
#define	CLUSTERDB_CHECKTIME	60	/* file check interval (seconds) */
#define	CLUSTERDB_CHANGETIME	3	/* wait change interval (seconds) */
#define	CLUSTERDB_DEFNETGROUP	"DEFAULT"

#define	CLUSTERDB_FILE		struct clusterdb_file
#define	CLUSTERDB_KEYNAME		struct clusterdb_keyname

#define	CLUSTERDB_KA		sizeof(char *(*)[2])
#define	CLUSTERDB_BO(v)		\
	((CLUSTERDB_KA - ((v) % CLUSTERDB_KA)) % CLUSTERDB_KA)

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif

#undef	ARGSBUFLEN
#define	ARGSBUFLEN		(3 * MAXHOSTNAMELEN)

#undef	DEFCHUNKSIZE
#define	DEFCHUNKSIZE		512

#define	KEYALIGNMENT		sizeof(char *(*)[2])


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	getpwd(char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct clusterdb_file {
	const char	*fname ;
	time_t		mtime ;
	uint		size ;
} ;

struct clusterdb_keyname {
	const char	*kname ;
	int		count ;
} ;

struct clusterdb_ie {
	const char	*(*keys)[2] ;
	const char	*svc, *clu, *sys ;
	int		nkeys ;			/* number of keys */
	int		size ;			/* total size */
	int		fi ;			/* file index */
} ;

struct svcentry {
	vecobj		keys ;
	const char	*svc ;
	const char	*clu ;
	const char	*sys ;
} ;

struct svcentry_key {
	const char	*kname ;
	const char	*args ;
	int		kl, al ;
} ;


/* forward references */

int		clusterdb_fileadd(CLUSTERDB *,const char *) ;

#if	(CLUSTERDB_MASTER == 1)
int		clusterdb_curbegin(CLUSTERDB *,CLUSTERDB_CUR *) ;
int		clusterdb_curend(CLUSTERDB *,CLUSTERDB_CUR *) ;
#endif


/* local variables */


/* exported subroutines */


int clusterdb_open(atp,fname)
CLUSTERDB	*atp ;
const char	fname[] ;
{
	struct ustat	sb ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("clusterdb_open: fname=%s\n",fname) ;
#endif

	if (atp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	memset(atp,0,sizeof(CLUSTERDB)) ;

	if ((rs = u_stat(fname,&sb)) >= 0) {
	    int	n ;
#if	CF_DEBUGTEST
	    n = 10000 ;
#else
	    n = MAX((sb.st_size / 4),10) ;
#endif
	    if ((rs = kvsfile_open(&atp->clutab,n,fname)) >= 0) {
	        atp->magic = CLUSTERDB_MAGIC ;
	    }
	} /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("clusterdb_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (clusterdb_open) */


/* free up the resources occupied by an CLUSTERDB list object */
int clusterdb_close(atp)
CLUSTERDB	*atp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (atp == NULL)
	    return SR_FAULT ;

	if (atp->magic != CLUSTERDB_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = kvsfile_close(&atp->clutab) ;
	if (rs >= 0) rs = rs1 ;

	atp->magic = 0 ;
	return rs ;
}
/* end subroutine (clusterdb_close) */


/* add a file to the list of files */
int clusterdb_fileadd(atp,fname)
CLUSTERDB	*atp ;
const char	fname[] ;
{
	int	rs ;


	if (atp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if (atp->magic != CLUSTERDB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("clusterdb_fileadd: fname=%s\n",fname) ;
#endif

	rs = kvsfile_fileadd(&atp->clutab,fname) ;

#if	CF_DEBUGS
	debugprintf("clusterdb_fileadd: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (clusterdb_fileadd) */


/* cursor manipulations */
int clusterdb_curbegin(atp,curp)
CLUSTERDB	*atp ;
CLUSTERDB_CUR	*curp ;
{
	int	rs ;


	if (atp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (atp->magic != CLUSTERDB_MAGIC) return SR_NOTOPEN ;

	rs = kvsfile_curbegin(&atp->clutab,&curp->cur) ;

	return rs ;
}
/* end subroutine (clusterdb_curbegin) */


int clusterdb_curend(atp,curp)
CLUSTERDB	*atp ;
CLUSTERDB_CUR	*curp ;
{
	int	rs ;


	if (atp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (atp->magic != CLUSTERDB_MAGIC) return SR_NOTOPEN ;

	rs = kvsfile_curend(&atp->clutab,&curp->cur) ;

	return rs ;
}
/* end subroutine (clusterdb_curend) */


/* enumerate the cluster names (keys) */
int clusterdb_enumcluster(atp,curp,kbuf,klen)
CLUSTERDB	*atp ;
CLUSTERDB_CUR	*curp ;
char		kbuf[] ;
int		klen ;
{
	KVSFILE_CUR	*ccp ;

	int	rs ;


	if (atp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (atp->magic != CLUSTERDB_MAGIC) return SR_NOTOPEN ;

	ccp = (curp != NULL) ? &curp->cur : NULL ;
	rs = kvsfile_enumkey(&atp->clutab,ccp,kbuf,klen) ;

#if	CF_DEBUGS
	debugprintf("clusterdb_enumcluster: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (clusterdb_enumcluster) */


/* enumerate the entries */
int clusterdb_enum(atp,curp,kbuf,klen,vbuf,vlen)
CLUSTERDB	*atp ;
CLUSTERDB_CUR	*curp ;
char		kbuf[], vbuf[] ;
int		klen, vlen ;
{
	KVSFILE_CUR	*ccp ;

	int	rs ;


	if (atp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if ((kbuf == NULL) || (vbuf == NULL)) return SR_FAULT ;

	if (atp->magic != CLUSTERDB_MAGIC) return SR_NOTOPEN ;

	ccp = (curp != NULL) ? &curp->cur : NULL ;
	rs = kvsfile_enum(&atp->clutab,ccp,kbuf,klen,vbuf,vlen) ;

#if	CF_DEBUGS
	debugprintf("clusterdb_enum: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (clusterdb_enum) */


int clusterdb_fetch(atp,cluname,curp,vbuf,vlen)
CLUSTERDB	*atp ;
char		cluname[] ;
CLUSTERDB_CUR	*curp ;
char		vbuf[] ;
int		vlen ;
{
	KVSFILE_CUR	*ccp ;

	int	rs ;


	if (atp == NULL) return SR_FAULT ;
	if (cluname == NULL) return SR_FAULT ;

	if (atp->magic != CLUSTERDB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("clusterdb_fetch: cluname=%s\n",cluname) ;
#endif

	ccp = (curp != NULL) ? &curp->cur : NULL ;
	rs = kvsfile_fetch(&atp->clutab,cluname,ccp,vbuf,vlen) ;

#if	CF_DEBUGS
	debugprintf("clusterdb_fetch: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (clusterdb_fetch) */


/* return cluster names given a node name */
int clusterdb_fetchrev(atp,nodename,curp,kbuf,klen)
CLUSTERDB	*atp ;
char		nodename[] ;
CLUSTERDB_CUR	*curp ;
char		kbuf[] ;
int		klen ;
{
	KVSFILE		*kop ;
	KVSFILE_CUR	vcur ;
	const int	nlen = NODENAMELEN ;
	const int	nrs = SR_NOTFOUND ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	f_match = FALSE ;
	char	nbuf[NODENAMELEN + 1] ;


	if (atp == NULL) return SR_FAULT ;
	if (nodename == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (nodename[0] == '\0') return SR_INVALID ;

	if (atp->magic != CLUSTERDB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("clusterdb_fetchrev: nodename=%s\n",nodename) ;
#endif

	kop = &atp->clutab ;
	if (curp != NULL) {
	    KVSFILE_CUR	*ccp = &curp->cur ;

	    while ((rs = kvsfile_enumkey(kop,ccp,kbuf,klen)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("clusterdb_fetchrev: kvsfile_enumkey() rs=%d\n",
			rs) ;
	        debugprintf("clusterdb_fetchrev: key=%t\n",kbuf,rs) ;
#endif

	        if ((rs = kvsfile_curbegin(kop,&vcur)) >= 0) {

	            while (rs >= 0) {

	                rs1 = kvsfile_fetch(kop,kbuf,&vcur,nbuf,nlen) ;
	                if (rs1 == nrs) break ;
	                rs = rs1 ;

#if	CF_DEBUGS
	                debugprintf("clusterdb_fetchrev: "
				"kvsfile_fetch() rs=%d\n",rs1) ;
	                debugprintf("clusterdb_fetchrev: nname=%s\n",nbuf) ;
#endif

	                if (rs >= 0) {
	                    f_match = (strcmp(nodename,nbuf) == 0) ;
	                }

	                if (f_match) break ;
	            } /* end while (fetching) */

	            rs1 = kvsfile_curend(kop,&vcur) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (kvsfile-cursor) */

	        if (f_match) break ;
	        if (rs < 0) break ;
	    } /* end while (enumerating keys) */

	} else {

	    if ((rs = kvsfile_curbegin(kop,&vcur)) >= 0) {

	        while (rs >= 0) {

	            rs = kvsfile_enum(kop,&vcur,kbuf,klen,nbuf,nlen) ;

	            if ((rs >= 0) && (strcmp(nodename,nbuf) == 0)) break ;

	        } /* end while (enumerating) */

	        rs1 = kvsfile_curend(kop,&vcur) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (kvsfile-cursor) */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("clusterdb_fetchrev: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (clusterdb_fetchrev) */


/* check if the access tables files have changed */
int clusterdb_check(atp,daytime)
CLUSTERDB	*atp ;
time_t		daytime ;
{
	int	rs = SR_OK ;


	if (atp == NULL)
	    return SR_FAULT ;

	if (atp->magic != CLUSTERDB_MAGIC)
	    return SR_NOTOPEN ;

	rs = kvsfile_check(&atp->clutab,daytime) ;

#if	CF_DEBUGS
	debugprintf("clusterdb_check: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (clusterdb_check) */



