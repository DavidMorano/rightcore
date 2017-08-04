/* main (testsort) */

/* sorted list testing */


#define	CF_DEBUGS	1


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstanards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<ftw.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<char.h>
#include	<localmisc.h>

#include	"sortlist.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern char	*malloc_str(char *) ;


/* forward references */

static int	slcmp() ;





int main(int argc,cchar **argv,cchar **envv)
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	tmpfile, *tfp = &tmpfile ;

	SORTLIST	list1 ;

	VECHAND		list2 ;

	int	rs, i, j, len ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	tmpfname_buf[MAXPATHLEN + 1] ;
	char	*cp, *cp1, *cp2 ;
	char	*lp ;
	char	*ep ;
	char	*progname ;


	progname = argv[0] ;
	(void) bopen(ofp,BFILE_STDOUT,"wct",0666) ;

#if	CF_DEBUGS
	debugprintf("main: entered\n\n") ;
#endif


	if (argc > 1)
	rs = bopen(ifp,argv[1],"r",0666) ;

	else
	rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0) goto badopen ;



	rs = vechand_start(&list2,0,SLP_SORTED) ;

	rs = sortlist_init(&list1,0,strcmp) ;

#if	CF_DEBUGS
	debugprintf("main: sortlist_init rs=%d\n",rs) ;
#endif

	while ((len = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {

	    if (linebuf[len - 1] == '\n') len -= 1 ;

	    linebuf[len] = '\0' ;
	    cp = linebuf ;
	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    cp1 = cp ;
	    while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;

	    if (*cp != '\0') *cp++ = '\0' ;

	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    cp2 = cp ;

	    ep = malloc_str(cp1) ;

	    rs = sortlist_add(&list1,ep,strcmp) ;

#if	CF_DEBUGS
	    debugprintf("main: sortlist_add rs=%d\n",rs) ;
#endif

	    vechand_add(&list2,ep) ;

	} /* end while (reading lines) */

	vechand_sort(&list2,slcmp) ;


#if	CF_DEBUGS
	debugprintf("main: compare phase\n") ;
#endif

	bprintf(ofp,"comparing phase\n\n") ;


	for (i = 0 ; sortlist_get(&list1,i,&lp) >= 0 ; i += 1) {

	    if (lp == NULL) continue ;

	    write(3,lp,strlen(lp)) ;

	    write(3,"\n",1) ;

	}


	for (i = 0 ; vechand_get(&list2,i,&lp) >= 0 ; i += 1) {

	    if (lp == NULL) continue ;

	    write(4,lp,strlen(lp)) ;

	    write(4,"\n",1) ;

	}




	for (i = 0 ; vechand_get(&list2,i,&lp) >= 0 ; i += 1) {

	    if (lp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("main: i=%d ep2=%s\n",i,lp) ;
#endif

	    if (sortlist_get(&list1,i,&ep) < 0) goto badcheck ;

	    if (ep == NULL) goto badcheck2 ;

#if	CF_DEBUGS
	    debugprintf("main: ep1=%s\n",ep) ;
#endif

	    if (strcmp(lp,ep) != 0) goto badcheck3 ;

	} /* end for */

	bprintf(ofp,"\n") ;

	rs = 0 ;


done:
	sortlist_free(&list1) ;

	vechand_finish(&list1) ;

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



