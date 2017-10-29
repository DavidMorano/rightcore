/* progaudit */

/* audito an index database */


#define	CF_DEBUG 	0		/* run-time debug print-outs */


/* revision history:

	= 1994-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a single file.

	Synopsis:

	int progaudit(pip,aip,terms,dbname,outfname)
	struct proginfo	*pip ;
	struct arginfo	*aip ;
	const uchar	terms[] ;
	const uchar	dbname[] ;
	char		outfname[] ;

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
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<field.h>
#include	<eigendb.h>
#include	<localmisc.h>

#include	"rtags.h"
#include	"offindex.h"
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
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;

extern int	hashmapverify(struct proginfo *,struct hashmap *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	tagindex(bfile *,HDB *) ;
static int	procquery(struct proginfo *,struct hashmap *,
			const uchar *,vecstr *,vecstr *,bfile *,bfile *) ;
static int	tag_parse(RTAGS_TAG *,const char *,const char *,
			const char *,int) ;


/* local variables */

static const uchar	aterms[] = {
	0x00, 0x3E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int progaudit(pip,aip,terms,dbname,outfname)
struct proginfo	*pip ;
struct arginfo	*aip ;
const uchar	terms[] ;
const char	dbname[] ;
char		outfname[] ;
{
	int	rs ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("progaudit: dbname=%s\n",dbname) ;
#endif

	rs = auditdb(pip,dbname) ;

	return rs ;
}
/* end subroutine (progaudit) */


