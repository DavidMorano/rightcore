/* fdb */

/* file database */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 96/03/01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/


/******************************************************************************

	This subroutine manages the default printer database.


	Synopsis:

	int fdbinit(pip,fname)
	struct proginfo	*pip ;
	const char	fname[] ;


	Arguments:

	pip		program information pointer
	fname		filename to open


	Returns:

	>=0		good
	<0		error


******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<field.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	DEFENTRIES		20



/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	vstrkeycmp(const char **,const char **) ;

extern char	*strnchr(const char *,int,int) ;
extern char	*strshrink(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x04, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;







int fdbinit(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	FIELD	fsb ;

	bfile	infile ;

	int	rs = SR_OK, rs1 ;
	int	len, cl ;
	int	opts, c ;
	int	kl, vl ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*kp, *vp ;
	char	*tp, *cp ;


	if (fname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("fdbinit: fname=%s\n",fname) ;
#endif

	opts = 0 ;
	rs = vecstr_start(&pip->fdb,DEFENTRIES,opts) ;

	if (rs < 0)
	    goto ret0 ;

	pip->f.fdbopen = TRUE ;

/* open the file (if we can) */

	if ((fname != NULL) && (strcmp(fname,STDINFNAME) != 0) &&
		(strcmp(fname,"-") != 0))
	    rs = bopen(&infile,fname,"r",0666) ;

	else
	    rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto bad1 ;

/* loop reading the file */

	c = 0 ;
	while ((rs = breadline(&infile,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;
	    if ((tp = strnchr(linebuf,len,'#')) != NULL) {
	        *tp = '\0' ;
		len = (tp - linebuf) ;
	    }

	    cl = sfshrink(linebuf,len,&cp) ;

	    if ((cp[0] == '\0') || (cp[0] == '#'))
	        continue ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("fdbinit: line=>%t<\n",cp,cl) ;
#endif

	    field_start(&fsb,cp,cl) ;

	    if ((kl = field_get(&fsb,fterms)) > 0) {

	        kp = fsb.fp ;
	        vp = NULL ;
	        vl = 0 ;
	        if ((fsb.term != '#') &&
	            ((vl = field_get(&fsb,fterms)) >= 0))
	            vp = fsb.fp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("fdbinit: key=>%t<\n",kp,kl) ;
		if (vp != NULL)
	debugprintf("fdbinit: val=>%t<\n",vp,vl) ;
	}
#endif

	        kp[kl] = '\0' ;
	        rs1 = vecstr_finder(&pip->fdb,kp,vstrkeycmp,NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("fdbinit: vecstr_finder() rs=%d\n",rs1) ;
#endif

	        if (rs1 == SR_NOTFOUND) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("fdbinit: adding\n") ;
#endif

	            c += 1 ;
	            rs = vecstr_addkeyval(&pip->fdb,kp,kl,vp,vl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("fdbinit: vecstr_addkeyval() rs=%d\n",rs) ;
#endif

	   	}

	        if (rs < 0)
	            break ;

	    } /* end if (got one) */

	    field_finish(&fsb) ;

	} /* end while (reading lines) */

	bclose(&infile) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	int	i ;
	for (i = 0 ; vecstr_get(&pip_.fdb,i,&cp) >= 0 ; i += 1) {
	            if (cp == NULL) continue ;
		debugprintf("fdbinit: k=%s v=%s\n",) ;
	}
	}
#endif

/* get out */
ret0:
	return (rs >= 0) ? c : rs ;

/* bad things */
bad1:
	vecstr_finish(&pip->fdb) ;

	goto ret0 ;
}
/* end subroutine (fdbinit) */


int fdbfetch(pip,keyname,buf,buflen)
struct proginfo	*pip ;
const char	keyname[] ;
char		buf[] ;
int		buflen ;
{
	int	rs = SR_NOTFOUND ;
	int	i ;

	char	*tp, *cp ;


	if (pip == NULL)
	    return SR_FAULT ;

	if (keyname == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("fdbfetch: keyname=%s\n", keyname) ;
#endif

	rs = vecstr_search(&pip->fdb,keyname,vstrkeycmp,&cp) ;

	if (rs >= 0) {

	    rs = 0 ;
	    if ((tp = strchr(cp,'=')) != NULL)
	    	rs = sncpy1(buf,buflen,(tp + 1)) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("fdbfetch: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (fdbfetch) */


int fdbfree(pip)
struct proginfo	*pip ;
{


	if (pip->f.fdbopen)
	    vecstr_finish(&pip->fdb) ;

	pip->f.fdbopen = FALSE ;
	return SR_OK ;
}
/* end subroutine (fdbfree) */



/* LOCAL SUBROUTINES */



