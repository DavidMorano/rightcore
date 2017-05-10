/* main */

/* shell program front end */
/* last modified %G% version %I% */



/* rebision history :

	= 88/01/10, David A­D­ Morano

	This subroutine was originally written (see 'editline.c') for
	more information.



*/


/************************************************************************

	This program provides a small portable screen editor
	for use by microcomputer based applications.



***************************************************************************/


#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<term.h>

#include	"localmisc.h"
#include	"q.h"
#include	"main.h"




/* external subroutines */

extern int	hi_insert(), hi_backup(), hi_copy() ;
extern int	editline() ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct gdata	gd, *gdp = &gd ;

	struct history	hi, *hp = &hi ;

	struct linebuf	lbuf[NLINEBUF], *lbp ;

	FIELD		fsb ;

	bfile	error, *efp = &error ;
	bfile	output, *ofp = &output ;
	bfile	input, *ifp = &input ;
	bfile	user_file, *ufp = &user_file ;
	bfile	temp0, temp1 ;
	bfile	*tfp, *bfp ;

	long		old_mode ;

	int		len, llen ;
	int		i, j ;
	int		n ;
	int		nlines, nchars ;	/* in file */
	int		iw, iw2, rs ;
	int		cllr ;
	int		cur_pos, redraw ;

	uchar		c ;
	uchar		*bp, buffer[LINESIZE], buf[LINESIZE], tbuf[10] ;
	uchar		*cbp ;
	uchar		*cbuf, comline[CBL+1] ;
	uchar		outbuf[2100], *obp = outbuf ;
	uchar		inbuf[100], *ibp = inbuf ;
	uchar		bg ;


	gdp->progname = argv[0] ;


	hi.c = hi.ri = hi.wi = 0 ;
	hi.n = 0 ;


	gdp->ifp = &input ;
	gdp->ofp = &output ;
	gdp->efp = &error ;

	gdp->hp = hp ;


/* OK play some games with the file descriptors */

	if ((iw = dup(0)) < 0) return iw ;

	gdp->ifd =  iw ;

	if ((iw = dup(1)) < 0) return iw ;

	gdp->ofd =  iw ;

	if ((iw = dup(2)) < 0) return iw ;

	gdp->efd =  iw ;


/* open the input and output basic I/O streams */

#ifdef	COMMENT
	bopen(gdp->ifp,gdp->ifd,"r",0666) ;
#endif

	bopen(gdp->ofp,gdp->ofd,"w",0666) ;

	bopen(gdp->efp,gdp->efd,"w",0666) ;


/* set up terminal for raw access */

	if (tty_open(gdp->ifd) != OK) {

	    bprintf(gdp->efp,"TTY open bad\n") ;

	    return BAD ;
	}


/* set initial terminal mode to default */

	gdp->term_mode = 0 ;


/* top of any further program access */
top:
	command_mode(gdp) ;


exit:

#ifdef	COMMENT
	tty_control(fc_setmode,old_mode) ;
#endif

	tty_close() ;

#ifdef	COMMENT
	bclose(gdp->ifp) ;
#endif

	bclose(gdp->ofp) ;

	bclose(gdp->efp) ;

	return OK ;
}
/* end subroutine (main) */



