/* main */

/* part of TERMCHAR program */


#define	CF_CHARSETS	0		/* set character sets */
#define	CF_ANSILEVEL	0		/* set ANSI conformance level */
#define	CF_DECHEBREW	0		/* use DEC Hebrew */
#define	CF_DECSUP	0		/* use DEC Supplemental */
#define	CF_HEBREW	0		/* use DEC Hebrew */
#define	CF_ANSIREC	1		/* use ANSI recommendation */


/* revision history:

	= 2004-06-24, David A­D­ Morano
        I rewrote this from scratch. The previous version of this program was a
        hack.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ termchar [arguments]


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"termcharsets.h"


/* local defines */

#define	CBUFLEN		100


/* external subroutines */

extern int	termcharset(char *,int,int,int,const char *) ;


/* forward references */


/* local variables */

static const char	*ss2 = "\033N" ;
static const char	*ss3 = "\033O" ;

static const char	*ls0 = "\017" ;		/* also SI */
static const char	*ls1 = "\016" ;		/* also SO */
static const char	*ls2 = "\033n" ;
static const char	*ls3 = "\033o" ;

static const char	*ls1r = "\033~" ;
static const char	*ls2r = "\033}" ;
static const char	*ls3r = "\033|" ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	FILE		*fp = stdout ;
	int		rs = SR_OK ;
	int		i, j ;
	int		ch ;
	int		pcol ;
	int		et = 2 ;

/* argument processing */

	if ((argc > 1) && (argv[1] != NULL)) {
	    et = atoi(argv[1]) ;
	}

#if	CF_ANSILEVEL
	fprintf(fp,"\033 L") ; /* set ANSI conformance level to '1' */
#endif

#if	CF_CHARSETS
	{
		char	cbuf[CBUFLEN+1] ;
		const int	clen = CBUFLEN ;
		int	i = 0 ;
		i += termcharset((cbuf+i),(clen-i),0,0,TCS_ASCII) ;
#if	CF_ANSIREC
		i += termcharset((cbuf+i),(clen-i),1,1,TCS_ISOLATIN1) ;
		i += termcharset((cbuf+i),(clen-i),2,0,TCS_DECLINEDRAW) ;
		i += storebuf_strw(cbuf,clen,i,ls1r,-1) ;
#else
		i += termcharset((cbuf+i),(clen-i),1,0,TCS_DECLINEDRAW) ;
		i += termcharset((cbuf+i),(clen-i),2,1,TCS_ISOLATIN1) ;
		i += storebuf_strw(cbuf,clen,i,ls2r,-1) ;
#endif /* CF_ANSIREC */
#if	CF_DECSUP
		i += termcharset((cbuf+i),(clen-i),3,0,TCS_DECSUP) ;
#else
#if	CF_DECHEBREW
		i += termcharset((cbuf+i),(clen-i),3,0,TCS_DECHEBREW) ;
#else
		i += termcharset((cbuf+i),(clen-i),3,0,TCS_DECTECH) ;
#endif
#endif /* CF_DECSUP */
		fprintf(fp,"%s",cbuf) ;
	}
#endif /* CF_CHARSETS */

/* print out the characters */

	for (i = 0 ; i < 16 ; i += 1) { /* rows */

	    fprintf(fp,"\r") ;

	    for (j = 0 ; j < 32 ; j += 1) { /* columns */

	        if (j < 16) {

	            pcol = j & 7 ;
	            if ((pcol != 0) && (pcol != 1)) {
	                ch = (j * 16) + i ;
	            } else {
	                ch = ' ' ;
		    }

	            if (ch == ((7 * 16) + 15)) {
	                ch = ' ' ;
		    }

	            fprintf(fp," %c",ch) ;

	        } else {

	            pcol = j & 7 ;
	            if ((pcol != 0) && (pcol != 1)) {
	                ch = (pcol * 16) + i ;
	            } else {
	                ch = ' ' ;
		    }

	            if (ch == ((7 * 16) + 15)) {
	                ch = ' ' ;
		    }

	            if (ch != ' ') {
	                if (j < 24) {
	                    switch (et) {
	                    case 0:
	                        fprintf(fp,"  ") ;
	                        break ;
	                    case 1:
				ch &= 0x7f ;
	                        fprintf(fp," %c%c%c",CH_SO,ch,CH_SI) ;
	                        break ;
	                    case 2:
	                        fprintf(fp," %s%c",ss2,ch) ;
	                        break ;
	                    case 3:
	                        fprintf(fp," %s%c",ss3,ch) ;
	                        break ;
	                    } /* end switch */
	                } else {
	                    fprintf(fp," %s%c",ss3,ch) ;
			}
	            } else {
	                fprintf(fp," %c",ch) ;
		    }

	        } /* end if */

	    } /* end for (columns) */

	    fprintf(fp,"\n") ;

	} /* end for (rows) */

/* done */

#ifdef	COMMENT
	fprintf(fp,"\r") ;
#endif

	fclose(fp) ;

	return 0 ;
}
/* end subroutine (main) */


