/* pathclean */

/* cleanup a path */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This was written as part of the FILECHECK utility.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine takes a path and "cleans" it up by removing certain
        redundant or otherwise unnecessary elements.

	Synopsis:

	int pathclean(rbuf,path,pathlen)
	char		rbuf[] ;
	const char	path[] ;
	int		pathlen ;

	Arguments:

	rbuf		buffer to receive cleaned up path string
	path		supplied path to clean up
	pathlen		length of supplied path to clean up

	Returns:

	>=0		length of cleaned up path
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	PATHBUF		struct pathbuf

#define	PATHBUF_BUF	(pbp->pbuf)
#define	PATHBUF_BUFLEN	(pbp->plen)
#define	PATHBUF_INDEX	(pbp->i)


/* local structures */

struct pathbuf {
	char		*pbuf ;
	int		plen ;
	int		i ;
} ;


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	pathbuf_start(struct pathbuf *,char *,int) ;
static int	pathbuf_char(struct pathbuf *,int) ;
static int	pathbuf_strw(struct pathbuf *,const char *,int) ;
static int	pathbuf_remove(struct pathbuf *) ;
static int	pathbuf_finish(struct pathbuf *) ;

#ifdef	COMMENT
static int	pathbuf_getlen(struct pathbuf *) ;
#endif

static int	nextname(const char *,int,const char **) ;


/* local variables */


/* exported subroutines */


int pathclean(rbuf,spathbuf,spathlen)
char		rbuf[] ;
const char	spathbuf[] ;
int		spathlen ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nc = 0 ;
	int		pl ;
	int		f_prev = FALSE ;
	const char	*pp ;

	if (rbuf == NULL) return SR_FAULT ;
	if (spathbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	pp = spathbuf ;
	if ((pl = strnlen(spathbuf,spathlen)) > 0) {
	    PATHBUF	pb ;
	    int		cl ;
	    const char	*cp ;

	    if ((rs = pathbuf_start(&pb,rbuf,MAXPATHLEN)) >= 0) {

	        if (*pp == '/') {

	            pp += 1 ;
	            pl -= 1 ;

	            f_prev = TRUE ;
	            if (*pp == '/')
	                pathbuf_char(&pb,'/') ;

	        } /* end if */

	        while ((cl = nextname(pp,pl,&cp)) > 0) {

	            if (cp[0] == '.') {

	                if ((cp[1] == '.') && (cl == 2)) {

	                    if (nc > 0) {

	                        pathbuf_remove(&pb) ;
	                        nc -= 1 ;

	                    } else {

	                        if (f_prev)
	                            pathbuf_char(&pb,'/') ;

	                        pathbuf_strw(&pb,cp,2) ;
	                        f_prev = TRUE ;
	                        nc = 0 ;

	                    } /* end if */

	                } else if (cl > 1) {

	                    if (f_prev)
	                        pathbuf_char(&pb,'/') ;

	                    pathbuf_strw(&pb,cp,cl) ;
	                    f_prev = TRUE ;
	                    nc += 1 ;

	                } /* end if */

	            } else {

	                if (f_prev)
	                    pathbuf_char(&pb,'/') ;

	                pathbuf_strw(&pb,cp,cl) ;
	                f_prev = TRUE ;
	                nc += 1 ;

	            } /* end if */

	            pl -= ((cp + cl) - pp) ;
	            pp = (cp + cl) ;

	        } /* end while */

	        rs1 = pathbuf_finish(&pb) ;
	        if (rs >= 0) rs = rs1 ;
	        if (rs1 >= 0) rbuf[rs1] = '\0' ;
	    } /* end if (pathbuf) */

	} /* end if (positive) */

	return rs ;
}
/* end subroutine (pathclean) */


/* local subroutines */


/* path buffer handling */
static int pathbuf_start(pbp,pbuf,plen)
PATHBUF		*pbp ;
char		pbuf[] ;
int		plen ;
{

	if (pbp == NULL)
	    return SR_FAULT ;

	PATHBUF_BUF = pbuf ;
	if (pbuf == NULL)
	    return SR_FAULT ;

	PATHBUF_BUFLEN = plen ;
	PATHBUF_INDEX = 0 ;
	return SR_OK ;
}
/* end subroutine (pathbuf_start) */


static int pathbuf_finish(pbp)
PATHBUF		*pbp ;
{
	int		len ;

	if (pbp == NULL)
	    return SR_FAULT ;

	if (PATHBUF_INDEX < 0)
	    return PATHBUF_INDEX ;

	len = PATHBUF_INDEX ;
	PATHBUF_BUF = NULL ;
	PATHBUF_BUFLEN = 0 ;
	PATHBUF_INDEX = SR_NOTOPEN ;
	return len ;
}
/* end subroutine (pathbuf_finish) */


/* store a string into the buffer */
static int pathbuf_strw(pbp,sp,sl)
PATHBUF		*pbp ;
const char	sp[] ;
int		sl ;
{
	int		rs = SR_OK ;
	int		len ;
	char		*bp ;

	if (pbp == NULL)
	    return SR_FAULT ;

	if (PATHBUF_INDEX < 0)
	    return PATHBUF_INDEX ;

	bp = (PATHBUF_BUF + PATHBUF_INDEX) ;
	if (PATHBUF_BUFLEN < 0) {

	    if (sl < 0) {

	        while (*sp)
	            *bp++ = *sp++ ;

	    } else {

	        while (*sp && (sl > 0)) {
	            *bp++ = *sp++ ;
	            sl -= 1 ;
	        }

	    } /* end if */

	} else {

	    if (sl < 0) {

	        while (*sp && (bp < (PATHBUF_BUF + PATHBUF_BUFLEN - 1)))
	            *bp++ = *sp++ ;

	    } else {

	        while (*sp && (sl > 0) && 
	            (bp < (PATHBUF_BUF + PATHBUF_BUFLEN - 1))) {

	            *bp++ = *sp++ ;
	            sl -= 1 ;
	        }

	    } /* end if */

	    if (bp >= (PATHBUF_BUF + PATHBUF_BUFLEN - 1)) {
	        PATHBUF_INDEX = SR_TOOBIG ;
	        rs = SR_TOOBIG ;
	    }

	} /* end if */

	if (rs >= 0) {
	    *bp = '\0' ;
	    len = bp - (PATHBUF_BUF + PATHBUF_INDEX) ;
	    PATHBUF_INDEX = (bp - PATHBUF_BUF) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pathbuf_strw) */


/* store a character */
static int pathbuf_char(pbp,c)
PATHBUF	*pbp ;
int	c ;
{
	char		buf[2] ;

	buf[0] = c ;
	buf[1] = '\0' ;
	return pathbuf_strw(pbp,buf,1) ;
}
/* end subroutine (pathbuf_char) */


/* remove a component from the path */
static int pathbuf_remove(pbp)
PATHBUF		*pbp ;
{
	char		*bp ;

	if (pbp == NULL)
	    return SR_FAULT ;

	if (PATHBUF_INDEX < 0)
	    return PATHBUF_INDEX ;

	bp = (PATHBUF_BUF + PATHBUF_INDEX) ;
	if ((PATHBUF_BUFLEN > 0) && (bp[-1] == '/')) {
	    PATHBUF_BUFLEN -= 1 ;
	    bp -= 1 ;
	}

	while ((PATHBUF_BUFLEN > 0) && (bp[-1] != '/')) {
	    PATHBUF_BUFLEN -= 1 ;
	    bp -= 1 ;
	} /* end while */

	if ((PATHBUF_BUFLEN > 0) && (bp[-1] == '/')) {
	    PATHBUF_BUFLEN -= 1 ;
	    bp -= 1 ;
	}

	PATHBUF_INDEX = (bp - PATHBUF_BUF) ;
	return 0 ;
}
/* end subroutine (pathbuf_remove) */


#ifdef	COMMENT /* not needed */
/* get the length filled so far */
static int pathbuf_getlen(pbp)
PATHBUF	*pbp ;
{

	if (pbp == NULL)
	    return SR_FAULT ;

	return PATHBUF_INDEX ;
}
/* end subroutine (pathbuf_getlen) */
#endif /* COMMENT */


static int nextname(sp,sl,rpp)
const char	*sp ;
int		sl ;
const char	**rpp ;
{
	int		f_len = (sl >= 0) ;

/* skip leading slashes */

	while (((! f_len) || (sl > 0)) && (*sp == '/')) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	*rpp = (char *) sp ;

/* skip the non-slash characters */

	while ((((! f_len) && *sp) || (sl > 0)) && (*sp != '/')) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return (sp - (*rpp)) ;
}
/* end subroutine (nextname) */



