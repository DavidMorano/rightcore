/* initinfo */

/* Initialization Information */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/******************************************************************************

	This is a hack to make the SFIO stuff useable an interchangeable
	way when not in an SFIO environment.  This generally occurs when a
	subroutine is used both in a SHELL builtin as well as stand-alone.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<localmisc.h>

#include	"initinfo.h"


/* local defines */

#define	INITINFO_MAGIC	0x43628192

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BASEDNAME	"/tmp"
#define	NENTRIES	10


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	vstrkeycmp(void *,void *) ;
extern int	procdefvar(VECSTR *,const char *) ;


/* forward references */


/* local variables */

#ifdef	COMMENT

static const char	*fnames[] = {
	STDINFNAME,
	STDOUTFNAME,
	STDERRFNAME,
	NULL
} ;

enum fnames {
	fname_stdin,
	fname_stdout,
	fname_stderr,
	fname_overlast
} ;

#endif /* COMMENT */





int initinfo_open(op,pr)
INITINFO	*op ;
const char	pr[] ;
{
	int	rs ;
	int	opts ;


	if (op == NULL)
	    return SR_FAULT ;

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(INITINFO)) ;

	op->pr = pr ;
	opts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&op->entries,NENTRIES,opts) ;

	if (rs < 0)
	    goto bad0 ;


	op->magic = INITINFO_MAGIC ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	vecstr_finish(&op->entries) ;

bad0:
	goto ret0 ;
}
/* end subroutine (initinfo_open) */


int initinfo_close(op)
INITINFO	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != INITINFO_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = vecstr_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (initinfo_close) */


int initinfo_curbegin(op,curp)
INITINFO	*op ;
INITINFO_CUR	*curp ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != INITINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (initinfo_curbegin) */


int initinfo_curend(op,curp)
INITINFO	*op ;
INITINFO_CUR	*curp ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != INITINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (initinfo_curend) */


/* enum: returns key-length */
int initinfo_enum(op,curp,kbuf,kbuflen,vbuf,vbuflen)
INITINFO	*op ;
INITINFO_CUR	*curp ;
char		kbuf[] ;
int		kbuflen ;
char		vbuf[] ;
int		vbuflen ;
{
	int	rs ;
	int	ci ;
	int	kl = 0 ;

	char	*tp ;
	char	*kp ;
	char	*cp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != INITINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	if (kbuf == NULL)
	    return SR_FAULT ;

	kbuf[0] = '\0' ;
	ci = (curp->i >= 0) ? (curp->i + 1) : 0 ;

	while ((rs = vecstr_get(&op->entries,ci,&cp)) >= 0) {

		if (cp != NULL)
			break ;

		ci += 1 ;

	} /* end while */

	if (rs >= 0) {

	    kp = cp ;
	    if ((tp = strchr(cp,'=')) != NULL) {
		kl = (tp - cp) ;
	    } else
		kl = strlen(cp) ;

	    rs = snwcpy(kbuf,kbuflen,kp,kl) ;

	    if ((rs >= 0) && (vbuf != NULL)) {

		if (tp != NULL) {

		    rs = sncpy1(vbuf,vbuflen,(tp + 1)) ;

		} else
		    vbuf[0] = '\0' ;

	    } /* end if (value) */

	} /* end if */

	if (rs >= 0)
	    curp->i = ci ;

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (initinfo_enum) */


/* query: returns value-length */
int initinfo_query(op,key,vbuf,vbuflen)
INITINFO	*op ;
const char	key[] ;
char		vbuf[] ;
int		vbuflen ;
{
	int	rs ;

	char	*tp ;
	char	*cp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != INITINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (key == NULL)
	    return SR_FAULT ;

	if (key[0] == '\0')
	    return SR_INVALID ;

	rs = vecstr_finder(&op->entries,key,vstrkeycmp,&cp) ;

	if (rs >= 0) {

	    tp = strchr(cp,'=') ;

	    if (vbuf != NULL) {

		if (tp != NULL) {

		    rs = sncpy1(vbuf,vbuflen,(tp + 1)) ;

		} else
		    vbuf[0] = '\0' ;

	    } else
		rs = (tp != NULL) ? strlen(tp + 1) : 0 ;

	} /* end if */

	return rs ;
}
/* end subroutine (initinfo_query) */



/* LOCAL SUBROUTINES */



