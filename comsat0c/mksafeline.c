/* mksafeline */

/* make a safe line */



/******************************************************************************

	This subroutine takes a raw text line and creates a line that
	is safe to print (like to a terminal).


	Synopsis:

	int mksafeline(linebuf,linelen)
	char	linebuf[] ;
	int	linelen ;


	Arguments:

	linebuf		buffer of raw line
	linelen		length of raw line


	Returns:

	>=0		length of safe line to print


******************************************************************************/





int mksafeline(linebuf,linebuflen)
char	linebuf[] ;
int	linebuflen ;
{
	int	rs = SR_OK ;
	int	ili, oli ;
	int	f_flipped = FALSE ;
	int	f_needeol = FALSE ;


	oli = 0 ;
	for (ili = 0 ; ili < linebuflen ; ili += 1) {

	    if (isprintlatin(linebuf[ili]) || ischarok(linebuf[ili])) {
	        if (f_flipped) {
	            linebuf[oli++] = linebuf[ili] ;
	        } else
	            oli += 1 ;
	    } else 
	        f_flipped = TRUE ;

	} /* end for */

	if ((oli > 0) && (linebuf[oli - 1] != '\n'))
	    f_needeol = TRUE ;

	if ((rs >= 0) && (oli > 0))
	    rs = bwrite(ofp,linebuf,oli) ;

	if ((rs >= 0) && f_needeol) {
		oli += 1 ;
		rs = bputc(ofp,'\n') ;
	}

	return (rs >= 0) ? oli : rs ;
}
/* end subroutine (mksafeline) */


