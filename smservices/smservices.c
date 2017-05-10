/* smservices */

/* parse a Short Message Services (SMS) file */


#define	CF_DEBUGS	0
#define	CF_DEBUGSFIELD	0


/* revision history:

	= 91/06/01, David A­D­ Morano

	This subroutine was originally written.


	= 00/01/21, David A­D­ Morano

	This subroutine was enhanced for use by LevoSim.


*/


/******************************************************************************

	In this module, we parse a Short Message Services file and
	place the data in a DB for access through an interface.



******************************************************************************/


#define	SMSERVICES_MASTER	1


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<buffer.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"smservices.h"



/* local defines */

#define	SMSERVICES_MAGIC	0x04311637

#undef	LINELEN
#define	LINELEN		128
#undef	BUFLEN
#define	BUFLEN		(LINELEN * 2)



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;

extern char	*strncpylow(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static void	checkfree() ;


/* local static data */

/* these are the terminators for most everything */
static const unsigned char 	fterms[32] = {
	    0x7F, 0xFE, 0xC0, 0xFE,
	    0x8B, 0x00, 0x00, 0x24, 
	    0x00, 0x00, 0x00, 0x00, 
	    0x00, 0x00, 0x00, 0x80,
	    0x00, 0x00, 0x00, 0x00, 
	    0x00, 0x00, 0x00, 0x00, 
	    0x00, 0x00, 0x00, 0x00, 
	    0x00, 0x00, 0x00, 0x00, 
} ;

/* these are the terminators for options */
static const unsigned char 	oterms[32] = {
	    0x00, 0x0B, 0x00, 0x00,
	    0x09, 0x10, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00
} ;






int smservices_open(csp,fname)
SMSERVICES	*csp ;
char		fname[] ;
{
	bfile	sfile, *sfp = &sfile ;

	FIELD	fsb ;

	int	srs, rs = SR_OK ;
	int	i ;
	int	c, len1, len, line = 0 ;
	int	noptions = 0 ;

	char	linebuf[LINELEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;
	char	*bp, *cp ;


#if	CF_DEBUGS
	debugprintf("smservices_init: entered filename=%s\n",configfname) ;
#endif

	if (csp == NULL)
	    return SR_FAULT ;

	csp->magic = 0 ;
	if (configfname == NULL) 
	    return SR_FAULT ;

	if (configfname[0] == '\0')
	    return SR_INVALID ;

	(void) memset(csp,0,sizeof(SMSERVICES)) ;

/* open configuration file */

#if	CF_DEBUGS
	debugprintf("smservices_init: opened file\n") ;
#endif

	rs = bopen(cfp,configfname,"r",0664) ;

	if (rs < 0)
	    goto badopen ;

/* initialize */

#if	CF_DEBUGS
	debugprintf("smservices_init: initializing\n") ;
#endif



/* start processing the configuration file */

#if	CF_DEBUGS
	debugprintf("smservices_init: reading lines\n") ;
#endif

	while ((len = breadline(cfp,linebuf,LINELEN)) > 0) {

	    line += 1 ;
	    if (len == 1) continue ;	/* blank line */

	    if (linebuf[len - 1] != '\n') {

#ifdef	COMMENT
	        f_trunc = TRUE ;
#endif
	        while ((c = bgetc(cfp)) >= 0)
	            if (c == '\n') break ;

	        continue ;
	    }

	    fsb.lp = linebuf ;
	    fsb.rlen = len - 1 ;

#if	CF_DEBUGS
	    debugprintf("smservices_init: line> %W\n",fsb.lp,fsb.rlen) ;
#endif

	    field_get(&fsb,fterms) ;

#if	CF_DEBUGS
	    {
	        if (fsb.flen >= 0) {
	            debugprintf("smservices_init: field> %W\n",
	                fsb.fp,fsb.flen) ;
	        } else
	            debugprintf("smservices_init: field> *none*\n") ;
	    }
#endif /* CF_DEBUGSFIELD */

/* empty or comment only line */

	    if (fsb.flen <= 0) continue ;

/* convert name to lower case */

	    strncpylow(buf,fsb.fp,fsb.flen) ;

	    buf[fsb.flen] = '\0' ;

	} /* end while (reading lines) */

#if	CF_DEBUGS
	debugprintf("smservices_init: done reading lines\n") ;
#endif

	bclose(cfp) ;

	srs = len ;
	if (len < 0) {

	    rs = len ;
	    goto badread ;
	}




#if	CF_DEBUGS
	debugprintf("smservices_init: exiting OK\n") ;
#endif

	csp->magic = SMSERVICES_MAGIC ;
	return SR_OK ;

/* handle bad things */
badalloc:
badprogram:
badread:
badconfig:
	csp->badline = line ;

baddefines:
	bclose(cfp) ;

badopen:

#if	CF_DEBUGS
	debugprintf("smservices_init: exiting bad rs=%d\n",rs) ;
#endif

	return rs ;

badkey:
	rs = SR_INVALID ;
	goto badprogram ;
}
/* end subroutine (smservices_open) */


/* free up this object */
int smservices_close(csp)
SMSERVICES	*csp ;
{


	if (csp == NULL)
	    return SR_FAULT ;

	if (csp->magic != SMSERVICES_MAGIC)
	    return SR_NOTOPEN ;

	csp->magic = 0 ;

/* free up the complex data types */


	return SR_OK ;
}
/* end subroutine (smservices_close) */



/* LOCAL SUBROUTINES */



/* free up the resources occupied by a CONFIG_STRUCTURE */
static void checkfree(vp)
char	**vp ;
{


	if (*vp != NULL) {

	    free(*vp) ;

	    *vp = NULL ;
	}
}
/* end subroutine (checkfree) */



