/* procfile */

/* process an input file to the output */


#define	CF_DEBUG	0


/* revistion history :

	= 87/01/10, David A­D­ Morano

	This subroutine was originally written.


*/


/******************************************************************************

	This subroutine processes a file by writing its contents out
	to an output file with the correct host name placement.


******************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<netdb.h>

#include	<bfile.h>
#include	<char.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define		LINELEN		200
#define		BUFLEN		(LINELEN + LINELEN)



/* external subroutines */


/* forward references */


/* local structures */


/* local variables */







int procfile(gp,infname,fn)
struct global	*gp ;
char		infname[] ;
int		fn ;
{
	bfile		infile, *ifp = &infile ;

	int	rs, i, j ;
	int	lines = 0, len ;
	int	f_stdinput = FALSE ;

	char	linebuf[LINELEN + 1], *lbp ;
	char	hostname[MAXHOSTNAMELEN + 1] ;
	char	*cp ;


/* check the arguments */

	if ((infname == NULL) || (infname[0] == '-')) {

	    f_stdinput = TRUE ;
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	} else
	    rs = bopen(ifp,infname,"r",0666) ;

	if (rs < 0)
	    goto badinfile ;



	while ((rs = breadline(ifp,linebuf,LINELEN)) > 0) {

		len = rs ;
	    if (len <= 1) continue ;

	    linebuf[len] = '\0' ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("procfile: LINE>%W",linebuf,len) ;
#endif

	    if (linebuf[0] == ';') continue ;

	    if (isalpha(linebuf[0])) {

	        lbp = linebuf ;
	        while (*lbp && (! CHAR_ISWHITE(*lbp))) 
			lbp += 1 ;

	        strwcpy(hostname,linebuf,
	            MIN((lbp - linebuf),MAXHOSTNAMELEN)) ;

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("procfile: hostsname=%s\n",hostname) ;
#endif

	        while (CHAR_ISWHITE(*lbp)) 
			lbp += 1 ;

	        i = (lbp - linebuf) ;

	    } else
	        i = 0 ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("procfile: is it an IN record\n") ;
#endif

	    if ((j = sfsub(linebuf + i,len,"IN",&cp)) < 0) 
		continue ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("procfile: %d IN record> %s\n",i + j,linebuf + i) ;
#endif

	    lbp = linebuf + i + j + 2 ;
	    while (CHAR_ISWHITE(*lbp)) 
		lbp += 1 ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("procfile: is it an A record> %s\n",lbp) ;
#endif

	    if (*lbp != 'A') 
		continue ;

/* read the IP address */

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("procfile: reading IP address\n") ;
#endif

	    lbp += 1 ;
	    while (CHAR_ISWHITE(*lbp)) lbp += 1 ;

	    if (((i = sfshrink(lbp,-1,&cp)) > 0) && (hostname[0] != '\0'))
	        bprintf(gp->ofp,"%t	%s\n",cp,i,hostname) ;


	    lines += 1 ;

	} /* end while (line reading loop) */

done:
	bclose(ifp) ;

badinfile:
	return (rs >= 0) ? lines : rs ;
}
/* end subroutine (procfile) */



