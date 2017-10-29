/* postwrite */

/* postwrite the input files */


#define	CF_DEBUG 	0		/* switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine postwritees a single file.

	Synopsis:

	int postwrite(pip,hasha,nhash,mfp,filename)
	struct proginfo	*pip ;
	uint		hasha[] ;
	int		nhash ;
	MEMFILE		*mfp ;
	const char	filename[] ;

	Arguments:

	- pip		program information pointer
	- filename	file to postwrite

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<field.h>
#include	<localmisc.h>

#include	"memfile.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	POSTINDEX_FILEMAGIC	"POSTINDEX"
#define	POSTINDEX_FILEMAGICLEN	9

#define	POSTINDEX_VERSION	0
#define	POSTINDEX_TYPE		0

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
extern uint	hashelf(void *,int) ;
extern uint	uceil(uint,int) ;

extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	ffbsi(uint) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	iceil(int,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct header {
	uint		wtime ;
	uint		nhash ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int postwrite(pip,hasha,nhash,mfp,indexname)
struct proginfo	*pip ;
uint		hasha[] ;
int		nhash ;
MEMFILE		*mfp ;		/* post MEMFILE */
const char	indexname[] ;
{
	struct postentry	*posta ;

	offset_t	hoff ;

	bfile	pfile ;

	uint	hi, npi, himask, nfo ;
	uint	pi ;

	int	rs ;
	int	i, j, len ;
	int	size ;
	int	sl, cl ;
	int	shift ;
	int	c = 0 ;

	const char	*sp, *cp ;

	char	magicbuf[16] ;
	char	vetu[4] ;
	char	postfname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("postwrite: entered indexname=%s\n",indexname) ;
	    debugprintf("postwrite: nhash=%u\n",nhash) ;
	}
#endif

	len = uceil(sizeof(struct postentry),sizeof(uint)) ;

	himask = (nhash - 1) ;
	shift = ffbsi(len) ;

/* get the starting offset of the posting file */

	memfile_buf(mfp,&posta) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("postwrite: initial posta=%p\n",posta) ;
#endif

/* open the posting file */

	mkfnamesuf1(tmpfname,indexname,"npost") ;

	rs = bopen(&pfile,tmpfname,"wct",0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("postwrite: bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* write out the header */

	strncpy(magicbuf,POSTINDEX_FILEMAGIC,16) ;

	magicbuf[POSTINDEX_FILEMAGICLEN] = '\n' ;

	vetu[0] = POSTINDEX_VERSION ;
	vetu[1] = ENDIAN ;
	vetu[2] = POSTINDEX_TYPE ;
	vetu[3] = 0 ;

	bwrite(&pfile,magicbuf,16) ;

	bwrite(&pfile,vetu,4) ;

	{
	    struct header	h ;
	    h.wtime = (uint) time(NULL) ;
	    h.nhash = nhash ;
	    bwrite(&pfile,&h,sizeof(struct header)) ;
	}

/* make space for the hash table */

	btell(&pfile,&hoff) ;

	size = nhash * sizeof(uint) ;
	rs = bseek(&pfile,(offset_t) size,SEEK_CUR) ;

/* write out the link data */

	j = 0 ;
	for (i = 0 ; (rs >= 0) && (i < nhash) ; i += 1) {

	    if ((pi = hasha[i]) != 0) {

	        while (pi != 0) {

	            npi = j ;
	            nfo = posta[pi].noff ;
	            j += 1 ;
	            rs = bwrite(&pfile,&nfo,sizeof(uint)) ;

	            c += 1 ;
	            pi = posta[pi].next ;

		    if (rs < 0) break ;
	        } /* end while */

	        nfo = (uint) EOP ;		/* End-Of-Post */
	        j += 1 ;
	        if (rs >= 0) rs = bwrite(&pfile,&nfo,sizeof(uint)) ;

/* update the hash table with the new proper index to the posting data */

	        hasha[i] = npi ;

	    } /* end if */

	} /* end for */

/* seek back and now write the new hash table */

	if (rs >= 0) {
	    bseek(&pfile,hoff,SEEK_SET) ;
	    size = nhash * sizeof(uint) ;
	    rs = bwrite(&pfile,hasha,size) ;
	}

/* close and get out */
ret2:
	bclose(&pfile) ;

ret1:
	if (rs >= 0) {
	    mkfnamesuf1(postfname,indexname,"post") ;
	    if (rs >= 0)
	        rs = u_rename(tmpfname,postfname) ;
	}

ret0:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("postwrite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (postwrite) */



