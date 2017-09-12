/* main (testsort) */

/* sorted list testing */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vechand.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"vecsorthand.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern char	*malloc_str(char *) ;


/* forward references */

static int	slcmp() ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	SORTVEC		list1 ;
	VECHAND		list2 ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		tmpfile, *tfp = &tmpfile ;
	int		rs ;
	int		i, j, len ;
	int		err_fd ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	tmpfname_buf[MAXPATHLEN + 1] ;
	cchar	*cp, *cp1, *cp2 ;
	cchar	*lp ;
	cchar	*ep ;
	cchar	*progname ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = argv[0] ;
	(void) bopen(ofp,BFILE_STDOUT,"wct",0666) ;

#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif


	if (argc > 1)
	rs = bopen(ifp,argv[1],"r",0666) ;

	else
	rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0) goto badopen ;



	rs = vechandinit(&list2,0,VECHAND_PSORTED) ;

	rs = sortvec_init(&list1,0,slcmp) ;

#if	CF_DEBUGS
	debugprintf("main: sortvec_init rs=%d\n",rs) ;
#endif

	while ((len = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	    if (lbuf[len - 1] == '\n') len -= 1 ;

	    lbuf[len] = '\0' ;
	    cp = lbuf ;
	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    cp1 = cp ;
	    while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;

	    if (*cp != '\0') *cp++ = '\0' ;

	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    cp2 = cp ;


	    ep = malloc_str(cp1) ;

	    rs = sortvec_add(&list1,(void *) ep) ;

#if	CF_DEBUGS
	    debugprintf("main: sortvec_add rs=%d\n",rs) ;
#endif

	    vechandadd(&list2,ep) ;

	} /* end while (reading lines) */

	vechandsort(&list2,slcmp) ;


#if	CF_DEBUGS
	debugprintf("main: outputting phase\n") ;
#endif

	bprintf(ofp,"outputting phase\n\n") ;


	for (i = 0 ; sortvec_get(&list1,i,(void **) &lp) >= 0 ; i += 1) {
	    if (lp == NULL) continue ;

	    write(4,lp,strlen(lp)) ;
	    write(4,"\n",1) ;

	}


#if	CF_DEBUGS
	debugprintf("main: output the vector list\n") ;
#endif

	for (i = 0 ; vechandget(&list2,i,&lp) >= 0 ; i += 1) {
	    if (lp == NULL) continue ;

#if	CF_DEBUGS
	debugprintf("main: lp=%s\n",lp) ;
#endif

	    write(5,lp,strlen(lp)) ;

	    write(5,"\n",1) ;

	}



#if	CF_DEBUGS
	debugprintf("main: compare phase\n") ;
#endif

	bprintf(ofp,"comparing phase\n\n") ;

	for (i = 0 ; vechandget(&list2,i,&lp) >= 0 ; i += 1) {
	    if (lp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("main: i=%d ep2=%s\n",i,lp) ;
#endif

	    if (sortvec_get(&list1,i,(void **) &ep) < 0) goto badcheck ;

	    if (ep == NULL) goto badcheck2 ;

#if	CF_DEBUGS
	    debugprintf("main: ep1=%s\n",ep) ;
#endif

	    if (strcmp(lp,ep) != 0) goto badcheck3 ;

	} /* end for */

	bprintf(ofp,"\n") ;

	rs = 0 ;

done:
	sortvec_free(&list1) ;

	vechandfree(&list2) ;

	bclose(ifp) ;

	bclose(ofp) ;

	return rs ;

badcheck:
	bprintf(ofp,"badcheck i=%d\n",i) ;

	goto done ;

badcheck2:
	bprintf(ofp,"badcheck2 i=%d\n",i) ;

	goto done ;

badcheck3:
	bprintf(ofp,"badcheck3 i=%d\n",i) ;

	goto done ;

badopen:
	bprintf(ofp,"%s: could not open input file (rs=%d\n",
		progname,rs) ;

	goto done ;

}
/* end subroutine (main) */


static int slcmp(app,bpp)
char	**app, **bpp ;
{
	return strcmp(*app,*bpp) ;
}


