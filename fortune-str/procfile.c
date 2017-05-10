/* procfile */




#include	<sys/types.h>
#include	<sys/param.h>
#include	<netinet/in.h>
#include	<ctype.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"strfile.h"



/* forward references */

void	order_unstr() ;




int procfile(gp,filename,mode)
struct global	*gp ;
char		filename[] ;
int		mode ;
{
	bfile		infile, datafile ;
	bfile		outfile ;

	STRFILE		tbl ;		/* description table */

	int		rs, len ;

	char		datafname[MAXPATHLEN + 1] ;



	if ((filename == NULL) || (filename[0] == '\0') ||
	    (strcmp(filename,"-") == 0)) {

	    bprintf(gp->efp,"%s: bad file specified\n",
	        gp->progname) ;

	    return BAD ;
	}

	if ((rs = bopen(&infile,filename,"r",0666)) < 0) {

	    bprintf(gp->efp,"%s: could not open file \"%s\"\n",
	        gp->progname,filename) ;

	    return BAD ;
	}

	bufprintf(datafname,MAXPATHLEN,"%s.dat",filename) ;

	if ((rs = bopen(&datafile,datafname,"r",0666)) < 0) {

	    bprintf(gp->efp,"%s: could not open data file \"%s\"\n",
	        gp->progname,filename) ;

	    return BAD ;
	}



	len = bread(&datafile,&tbl,sizeof(STRFILE)) ;

	if (len != sizeof(STRFILE)) {

	    bclose(&infile) ;

	    bclose(&datafile) ;

	    bprintf(gp->efp,"%s: bad table header in file \"%s\"\n",
	        gp->progname,filename) ;

	    return BAD ;
	}

	tbl.str_version = ntohl(tbl.str_version) ;
	tbl.str_numstr = ntohl(tbl.str_numstr) ;
	tbl.str_longlen = ntohl(tbl.str_longlen) ;
	tbl.str_shortlen = ntohl(tbl.str_shortlen) ;
	tbl.str_flags = ntohl(tbl.str_flags) ;

	order_unstr(gp,&infile,&datafile,&tbl) ;


	bclose(&datafile) ;

	bclose(&infile) ;

	return OK ;
}
/* end subroutine (procfile) */


void order_unstr(gp,ifp,dfp,tbl)
struct global	*gp ;
bfile		*ifp, *dfp ;
STRFILE		*tbl ;
{
	offset_t	boff ;

	long	pos ;

	int	i ;
	int	len ;

	char	linebuf[LINELEN + 1] ;


	for (i = 0; i < tbl->str_numstr; i += 1) {

	    bread(dfp,&pos,sizeof(long)) ;

	    moff = ntohl(pos) ;
	    bseek(ifp,boff,SEEK_SET) ;

	    if (i > 0)
	        bprintf(gp->ofp,"%c\n", tbl->str_delim) ;

	    while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	        if ((len == 2) && (linebuf[0] == tbl->str_delim))
	            break ;

	        bwrite(gp->ofp,linebuf,len) ;

	    } /* end while */

	} /* end for */

}
/* end subroutine (order_unstr) */



