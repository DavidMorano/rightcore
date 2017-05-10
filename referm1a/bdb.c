/* bdb */

/* Bibliographical DataBase */


#define	CF_DEBUG	0


/* revision history:

	- David A.D. Morano, September 1987
	This code module was originally written.

	- David A.D. Morano, February 1996
	This module was enhanced with the 'bdbloadu' subroutine
	to allow for the loading of paramters with names in
	the same character string of the parameters values.

	- David A.D. Morano, September 1998
	This module was changed to serve in the REFERM program.

*/


/*******************************************************************

	This code module contains subroutines used to add paramters
	to parameter lists and such for later access.

	All parameter names and values are stored in freshly 
	allocated memory.  The original storage for parameter
	names and values can be freed after they are stored
	using these routines.


*********************************************************************/




#include	<sys/types.h>
#include	<sys/param.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>

#include	<bfile.h>
#include	<baops.h>
#include	<char.h>
#include	<vecstr.h>
#include	<hdb.h>

#include	"localmisc.h"
#include	"bdb.h"



/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif



/* external subroutines */

extern char	*malloc_sbuf() ;


/* forward references */

int	bdbloads() ;
int	bdbload() ;
int	bdbfind() ;




int bdbinit(bdbp)
struct bdb	*bdbp ;
{


	return hdbinit(&bdbp->lh,5,NULL) ;
}


/* load a string parameter into the string DB */
int bdbloads(bdbp,s,len)
struct bdb	*bdbp ;
char		*s ;
int		len ;
{
	char	*cp ;


#if	CF_DEBUG
	debugprintf("bdbloads: entered\n") ;
#endif

	if (len < 0) len = strlen(s) ;

	while ((cp = strpbrk(s," \t,:")) != NULL) {

#if	CF_DEBUG
	    debugprintf("bdbloads: about to call 'bdbload'\n") ;
#endif

	    *cp = '\0' ;
	    if (bdbload(&bdbp->lh,s,len) < 0) return BAD ;

#if	CF_DEBUG
	    debugprintf("bdbloads: returned from 'bdbload'\n") ;
#endif

	    s = cp + 1 ;

	}  /* end while */

	if (((int) strlen(s)) > 0)
	    if (bdbload(&bdbp->lh,s,len) < 0) return BAD ;

#if	CF_DEBUG
	debugprintf("bdbloads: exited OK\n") ;
#endif

	return OK ;
}
/* end subroutine (bdbloads) */


/* load a single parameter into the "params" DB */
int bdbload(bdbp,s,ulen)
struct bdb	*bdbp ;
char		s[] ;
int		ulen ;
{
	HDB_DATUM	key, value ;
	int	len = ulen ;
	int	rs ;

	char	*sp, *cp ;
	char	*rp ;


#if	CF_DEBUG
	debugprintf("bdbload: entered, s=%s\n",s) ;
#endif

/* clean up the value a little */

	sp = s ;
	while (CHAR_ISWHITE(*sp) || (*sp == ',')) {

		sp += 1 ;
		len -= 1 ;

	} /* end while */

	cp = sp ;
	while ((len > 0) && 
		(CHAR_ISWHITE(sp[len - 1]) || (sp[len - 1] == ',')))
			sp[--len] = '\0' ;

	if (len <= 0) return OK ;

#if	CF_DEBUG
	debugprintf("bdbload: non-zero len value, v=\"%s\"\n",
	    sp) ;
#endif

/* do we have one of these named parameters already ? */

	if ((rs = bdbfind(&bdbp->lh,sp,len,&rp)) < 0) {

#if	CF_DEBUG
	debugprintf("bdbload: not already present\n") ;
#endif

		key.buf = malloc_sbuf(sp,len) ;

		key.len = len ;
		value.buf = NULL ;
		value.len = 0 ;
		rs = hdbstore(&bdbp->lh,key,value) ;

#if	CF_DEBUG
	debugprintf("bdbload: stored rs %d\n",rs) ;
#endif

	} /* end if */

#if	CF_DEBUG
	debugprintf("bdbload: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bdbload) */


/* find a paramter by name */
int bdbfind(bdbp,name,len,rpp)
struct bdb		*bdbp ;
char			name[] ;
char			len ;
struct bdb_value	**rpp ;
{
	HDB_DATUM	key, value ;

	int		rs ;


#if	CF_DEBUG
	debugprintf("bdbfind: entered, n=%s\n",name) ;
#endif

	key.buf = name ;
	key.len = (len >= 0) ? len : strlen(name) ;

	if ((rs = hdbfetch(&bdbp->lh,NULL,key,&value)) < 0)
		return rs ;

#if	CF_DEBUG
	debugprintf("bdbfind: got it\n") ;
#endif

	*rpp = (struct bdb_value *) value.buf ;
	return value.len ;
}
/* end subroutine (bdbfind) */


int bdbfree(bdbp)
struct bdb	*bdbp ;
{


	return hdbfree(&bdbp->lh) ;
}


int bdbcount(bdbp)
struct bdb	*bdbp ;
{


	return hdbcount(&bdbp->lh) ;
}


int bdbget(bdbp,cp,rpp)
struct bdb		*bdbp ;
struct bdb_cur	*cp ;
char			**rpp ;
{
	HDB_DATUM	key, value ;

	int	rs ;


#if	CF_DEBUG
	debugprintf("bdbget: entered\n") ;
#endif

	while ((rs = hdbenum(&bdbp->lh,(HDB_CUR *) cp,
		&key,&value)) >= 0) {

#if	CF_DEBUG
	debugprintf("bdbget: loop\n") ;
#endif

		if ((key.buf != NULL) && (key.len > 0)) {

			*rpp = key.buf ;
			break ;
		}

	} /* end while */

#if	CF_DEBUG
	debugprintf("bdbget: exiting rs %d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bdbget) */


int bdbdelcursor(bdbp,cp)
struct bdb		*bdbp ;
struct bdb_cur	*cp ;
{


	return hdbdelcursor(&bdbp->lh,(HDB_CUR *) cp) ;
}


int bdbnullcursor(cp)
struct bdb_cur	*cp ;
{


	return hdbnullcursor((HDB_CUR *) cp) ;
}






