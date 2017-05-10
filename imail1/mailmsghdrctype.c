/* mailmsghdrctype */

/* process the input messages and spool them up */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module parses a "content-type" header specification.
	The parsed results are broken into three types of items:
	the type, the sub-type, and parameters.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"mailmsghdrctype.h"


/* local defines */

#define	MAILMSGHDRCTYPE_MAGIC		0x53232857

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	HDRNAMELEN
#define	HDRNAMELEN	80
#endif

#ifndef	MSGLINELEN
#define	MSGLINELEN	(2 * 1024)
#endif

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif

#undef	CHAR_TOKSEP
#define	CHAR_TOKSEP(c)	(CHAR_ISWHITE(c) || (! isprintlatin(c)))


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int sfshrink(const char *,int,const char **) ;
extern int sfbasename(const char *,int,const char **) ;
extern int sfdirname(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mailmsghdrctype_start(op,hp,hl)
MAILMSGHDRCTYPE	*op ;
const char	*hp ;
int		hl ;
{
	mode_t	operms = 0660 ;

	int	rs ;
	int	vopts ;
	int	oflags = O_RDWR ;
	int	nmsgs = 0 ;

	const char	*tmpdname ;

	char	template[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


	if (op == NULL)
	    return SR_FAULT ;

	if (mfd < 0)
	    return SR_BADF ;

	memset(op,0,sizeof(MAILMSGHDRCTYPE)) ;

	op->tfd = -1 ;
	op->f.useclen = (opts & MAILMSGHDRCTYPE_OUSECLEN) ? 1 : 0 ;
	op->f.useclines = (opts & MAILMSGHDRCTYPE_OUSECLINES) ? 1 : 0 ;

	tmpdname = getenv(VARTMPDNAME) ;

	if (tmpdname == NULL)
	    tmpdname = MAILMSGHDRCTYPE_TMPDNAME ;

	rs = mkpath2(template,tmpdname,"msXXXXXXXXXXXX") ;

	if (rs >= 0) {
	    rs = opentmpfile(template,oflags,operms,tmpfname) ;
	    op->tfd = rs ;
	}

	if (rs < 0)
	    goto bad0 ;

	rs = uc_mallocstrw(tmpfname,-1,&op->tmpfname) ;
	if (rs < 0)
	    goto bad1 ;

	vopts = (VECHAND_OCOMPACT | VECHAND_OSTATIONARY) ;
	rs = vechand_start(&op->msgs,4,vopts) ;
	if (rs < 0)
	    goto bad2 ;

	rs = mailmsghdrctype_gather(op,mfd,to) ;
	nmsgs = rs ;
	if (rs < 0)
	    goto bad3 ;

	if (op->tflen > 0) {
	    size_t	msize = op->tflen ;
	    char	*p ;
	    int		mprot = PROT_READ ;
	    int		mflags = MAP_SHARED ;
	    rs = u_mmap(NULL,msize,mprot,mflags,op->tfd,0L,&p) ;
	    if (rs >= 0) {
	        op->mapdata = p ;
	        op->mapsize = msize ;
	    } else
	        goto bad3 ;

#if	CF_DEBUGS
	debugprintf("mailmsghdrctype_init: mapdata=\\x%p\n",op->mapdata) ;
	debugprintf("mailmsghdrctype_init: mapsize=%u\n",op->mapsize) ;
#endif

	} /* end if */

	op->magic = MAILMSGHDRCTYPE_MAGIC ;

ret0:
	return (rs >= 0) ? nmsgs : rs ;

/* bad stuff */
bad3:
	mailmsghdrctype_msgsfree(op) ;

	vechand_finish(&op->msgs) ;

bad2:
	if (op->tmpfname != NULL) {
	    uc_free(op->tmpfname) ;
	    op->tmpfname = NULL ;
	}

bad1:
	if (op->tfd >= 0) {
	    u_close(op->tfd) ;
	    op->tfd = -1 ;
	}

	if (tmpfname != NULL) {
	    if (tmpfname[0] != '\0')
	        u_unlink(tmpfname) ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (mailmsghdrctype_start) */


int mailmsghdrctype_finish(op)
MAILMSGHDRCTYPE	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MAILMSGHDRCTYPE_MAGIC)
	    return SR_NOTOPEN ;

	if ((op->mapsize > 0) && (op->mapdata != NULL))
	    u_munmap(op->mapdata,op->mapsize) ;
	op->mapdata = NULL ;
	op->mapsize = 0 ;

	rs1 = mailmsghdrctype_msgsfree(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->msgs) ;
	if (rs >= 0) rs = rs1 ;

	if (op->tfd >= 0) {
	    u_close(op->tfd) ;
	    op->tfd = -1 ;
	}

	if (op->tmpfname != NULL) {
	    if (op->tmpfname[0] != '\0')
	        u_unlink(op->tmpfname) ;
	    rs1 = uc_free(op->tmpfname) ;
	    op->tmpfname = NULL ;
	    if (rs >= 0) rs = rs1 ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mailmsghdrctype_finish) */


int mailmsghdrctype_paramget(op,i,rpp)
MAILMSGHDRCTYPE	*op ;
int		i ;
const char	**rpp ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MAILMSGHDRCTYPE_MAGIC)
	    return SR_NOTOPEN ;

	rs = vechand_count(&op->msgs) ;

	return rs ;
}
/* end subroutine (mailmsghdrctype_paramget) */


int mailmsghdrctype_paramfind(op,key,rpp)
MAILMSGHDRCTYPE	*op ;
const char	key[] ;
const char	**rpp ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MAILMSGHDRCTYPE_MAGIC)
	    return SR_NOTOPEN ;

	rs = vechand_count(&op->msgs) ;

	return rs ;
}
/* end subroutine (mailmsghdrctype_paramfind) */



