/* auditdb */

/* audit an index database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG 	0		/* run-time debug print-outs */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a single file.

	Synopsis:

	int auditdb(pip,dbname)
	struct proginfo	*pip ;
	const uchar	dbname[] ;

	Arguments:

	- pip		program information pointer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<char.h>
#include	<eigendb.h>
#include	<localmisc.h>

#include	"offindex.h"
#include	"rtags.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	NATURALWORDLEN
#endif

#ifndef	TAGBUFLEN
#define	TAGBUFLEN	MIN((MAXPATHLEN + 40),(2 * 1024))
#endif

#ifndef	LOWBUFLEN
#define	LOWBUFLEN	NATURALWORDLEN
#endif


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	hashmapverify(struct proginfo *,struct hashmap *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	offindex_tags(OFFINDEX *,bfile *) ;
static int	verifyentries(struct proginfo *,struct hashmap *) ;
static int	tag_parse(RTAGS_TAG *,const char *,const char *,
			const char *,int) ;


/* local variables */


/* exported subroutines */


int auditdb(pip,dbname)
struct proginfo	*pip ;
const char	dbname[] ;
{
	struct ustat	sb ;

	struct hashmap	hmi ;

	OFFINDEX	oi ;

	bfile	tagfile ;

	int	rs ;
	int	fd_hash ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: dbname=%s\n",dbname) ;
#endif /* CF_DEBUG */

	if (dbname == NULL)
	    return SR_FAULT ;

	if (dbname[0] == '\0')
	    return SR_INVALID ;

	memset(&hmi,0,sizeof(struct hashmap)) ;

/* open the HASH file */

	mkfnamesuf2(tmpfname,dbname,FE_HASH,ENDIANSTR) ;

	rs = u_open(tmpfname,O_RDONLY,0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: hasfile u_open() rs=%d\n",rs) ;
#endif

	fd_hash = rs ;
	if (rs < 0)
	    goto ret1 ;

/* open the TAG file */

	mkfnamesuf1(tmpfname,dbname,FE_TAG) ;

	rs = bopen(&tagfile,tmpfname,"r",0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: tagfile bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret2 ;

	rs = bcontrol(&tagfile,BC_STAT,&sb) ;

	if (rs < 0)
	    goto ret3 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: tfilesize=%u\n",sb.st_size) ;
#endif

	hmi.tfilesize = sb.st_size ;

/* index the TAG file */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: offindex_start() \n") ;
#endif

	rs = offindex_start(&oi,100) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: offindex_start() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret3 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: offindex_tags() \n") ;
#endif

	rs = offindex_tags(&oi,&tagfile) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: offindex_tags() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret3a ;

/* map the hash file */

	rs = u_fstat(fd_hash,&sb) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: u_fstat() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret3a ;

	{
	    int	prot, flags ;


	    hmi.hfilesize = sb.st_size ;

	    prot = PROT_READ ;
	    flags = MAP_SHARED ;
	    rs = u_mmap(NULL,(size_t) hmi.hfilesize,prot,flags,
	        fd_hash,0L,&hmi.filemap) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf( "auditdb: u_mmap() rs=%d\n",rs) ;
#endif

	} /* end block */

	if (rs < 0)
	    goto ret3a ;

	u_close(fd_hash) ;

	fd_hash = -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: hashmapverify()\n") ;
#endif

	rs = hashmapverify(pip,&hmi) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "auditdb: hashmapverify() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret4 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
		int	i ;
	    debugprintf( "auditdb: sdn=%s\n",hmi.sdn) ;
	    debugprintf( "auditdb: sfn=%s\n",hmi.sfn) ;
	    debugprintf( "auditdb: maxkeys=%u\n",hmi.maxtags) ;
	    debugprintf( "auditdb: minlen=%u\n",hmi.minlen) ;
	    debugprintf( "auditdb: maxlen=%u\n",hmi.maxlen) ;
	    debugprintf( "auditdb: tablen=%u\n",hmi.tablen) ;
	    for (i = 0 ; i < hmi.tablen ; i += 1) {
	        debugprintf( "auditdb: table[%u]=%u\n",i,hmi.table[i]) ;
	    }
	}
#endif /* CF_DEBUG */

/* verify that all list pointers and list entries are valid */

	rs = verifyentries(pip,&hmi) ;

/* done */
ret4:
	if (hmi.filemap != NULL)
	    u_munmap(hmi.filemap,(size_t) hmi.hfilesize) ;

ret3a:
	offindex_finish(&oi) ;

ret3:
	bclose(&tagfile) ;

ret2:
	if (fd_hash >= 0)
	    u_close(fd_hash) ;

ret1:
ret0:
	return rs ;
}
/* end subroutine (auditdb) */


/* local subroutines */


/* index the beginning-of-line offsets in the TAG file */
static int offindex_tags(oip,tfp)
OFFINDEX	*oip ;
bfile		*tfp ;
{
	offset_t	lineoff ;

	int	rs ;
	int	len ;
	int	n = 0 ;
	int	f_bol, f_eol ;

	char	linebuf[LINEBUFLEN + 1] ;


	lineoff = 0 ;
	f_bol = TRUE ;
	while ((rs = breadline(tfp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    f_eol = (linebuf[len - 1] == '\n') ;
	    if (f_bol && (! CHAR_ISWHITE(linebuf[0]))) {

	        n += 1 ;
	        rs = offindex_add(oip,lineoff,len) ;

	    }

	    lineoff += len ;
	    f_bol = f_eol ;
	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (offindex_tags) */


static int verifyentries(pip,hmip)
struct proginfo	*pip ;
struct hashmap	*hmip ;
{
	uint	listoff ;
	uint	tagoff ;
	uint	hfilesize ;
	uint	tfilesize ;
	uint	listsize ;
	uint	*uip ;

	int	rs = SR_OK ;
	int	i, j ;
	int	ntags ;


#ifdef	COMMENT

	debugprintf( "auditdb: sdn=%s\n",hmi.sdn) ;
	debugprintf( "auditdb: maxkeys=%u\n",hmi.maxtags) ;
	debugprintf( "auditdb: minlen=%u\n",hmi.minlen) ;
	debugprintf( "auditdb: maxlen=%u\n",hmi.maxlen) ;
	debugprintf( "auditdb: tablen=%u\n",hmi.tablen) ;
	for (i = 0 ; i < hmi.tablen ; i += 1) {
	    debugprintf( "auditdb: table[%u]=%u\n",i,hmi.table[i]) ;
	}

#endif /* COMMENT */

	hfilesize = hmip->hfilesize ;
	tfilesize = hmip->tfilesize ;

/* verify table length */

	{
	    uint	tabsize = hmip->tablen + sizeof(uint) ;


	    if (tabsize >= hfilesize) {

	        rs = SR_BADFMT ;
	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,
	                "%s: hash-table length=%u too long\n",
	                pip->progname,hmip->tablen) ;

	    } /* end if (error) */

	} /* end block */

/* loop over table entries */

	if (rs >= 0) {

	    for (i = 0 ; i < hmip->tablen ; i += 1) {

	        listoff = hmip->table[i] ;
	        if (listoff >= hfilesize) {

	            if (pip->debuglevel > 0)
	                bprintf(pip->efp,
	                    "%s: hash-table list-offset=%u too big\n",
	                    pip->progname,listoff) ;

	            break ;

	        } /* end if (error) */

	        uip = (uint *) (hmip->filemap + listoff) ;
	        ntags = *uip++ ;

		if (ntags > 0) {

	        if (ntags > hmip->maxtags) {

	            rs = SR_BADFMT ;
	            if (pip->debuglevel > 0)
	                bprintf(pip->efp,
	                    "%s: number of list-tags=%u too many\n",
	                    pip->progname,ntags) ;

	            break ;

	        } /* end if (error) */

	        listsize = (ntags + 1) * sizeof(uint) ;
	        if ((listsize + listoff) >= hfilesize) {

	            rs = SR_BADFMT ;
	            if (pip->debuglevel > 0)
	                bprintf(pip->efp,
	                    "%s: hash-table list-size=%u too big\n",
	                    pip->progname,listsize) ;

	            break ;

	        } /* end if (error) */

	        for (j = 0 ; j < ntags ; j += 1) {

	            tagoff = *uip++ ;

	            if (tagoff >= tfilesize) {

	                rs = SR_BADFMT ;
	                if (pip->debuglevel > 0)
	                    bprintf(pip->efp,
	                        "%s: tag-offset too big\n",
	                        pip->progname,tagoff) ;

	                break ;

	            } /* end if (error) */

	        } /* end for (tag-list entries) */

		} /* end if (had some tags) */

	        if (rs < 0)
	            break ;

	    } /* end for (hash-table entries) */

	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (verifyentries) */


static int tag_parse(tip,dnp,fnp,tagbuf,tagbuflen)
RTAGS_TAG	*tip ;
const char	*dnp ;
const char	*fnp ;
const char	tagbuf[] ;
int		tagbuflen ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	tnl ;

	char	*tnp ;
	char	*tp, *sp ;


/* parse out the file name and the offset-length */

	tnp = (char *) tagbuf ;
	tp = strnchr(tagbuf,tagbuflen,':') ;

	if (tp == NULL)
	    return SR_NOTFOUND ;

	tnl = tp - tagbuf ;
	if (tnl == 0) {

	    if ((fnp != NULL) && (fnp[0] != '\0')) {

	        tnp = (char *) fnp ;
	        tnl = strlen(fnp) ;

	    } else
	        rs = SR_NOTFOUND ;

	}

	if (rs >= 0) {

	    rs = storebuf_strw(tip->fname,MAXPATHLEN,0,tnp,tnl) ;

	    tip->recoff = -1 ;
	    tip->reclen = -1 ;
	    sp = tp + 1 ;
	    if ((tp = strchr(sp,',')) != NULL) {

	        rs1 = cfdeci(sp,(tp - sp),&tip->recoff) ;

	        if (rs1 < 0)
	            tip->recoff = 0 ;

	        sp = tp + 1 ;
	        rs1 = cfdeci(sp,-1,&tip->reclen) ;

	        if (rs1 < 0)
	            tip->reclen = -1 ;

	    } /* end if (getting record offset and length) */

	} /* end if */

	return rs ;
}
/* end subroutine (tag_parse) */



