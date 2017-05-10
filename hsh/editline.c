/* editline */

/* command mode */
/* last modified %G% version %I% */


#define	F_FORWARD	1


/* revision history:

	= 1988-01-10, David A­D­ Morano

	This subroutine was originally written and was heavily
	borrowed from the same stuff that were on the old PPI-like
	computers.


*/


/************************************************************************

	This module provides local line editing functions.


**************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<termstr.h>
#include	<ascii.h>

#include	"localmisc.h"
#include	"q.h"
#include	"defs.h"



/* local defines */

#define	CH_EOH	CH_ETX

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + 12)
#endif



/* external subroutines */

extern int	tty_read(), tty_control(), tty_write() ;
extern int	hi_insert(), hi_backup(), hi_copy() ;
extern int	editline() ;


/* local structures */

struct ldata {
	uchar	*cbp, *obp ;
	uchar	*pbp ;
	int	olen, clen ;
	int	plen ;
} ;


/* forward references */

int	csi(), ss3() ;







int editline(gdp,hp,comline,lenp,ps,pl,c)
struct gdata	*gdp ;
struct history	*hp ;
char	comline[] ;
int	*lenp ;
char	*ps ;
int	pl ;
int	c ;
{
	struct ldata	ld, *ldp = &ld ;

	int	old, mode ;

	int	i ;

	int	ifd = gdp->ifd ;
	int	ofd = gdp->ofd ;
	int	hi = hp->wi ;
	int	curpos ;
	int	f_redraw, flag, exit_flag ;

	uchar	original[100], buf[BUFLEN + 1], *bp ;


	ldp->obp = original ;
	ldp->olen = *lenp ;

	ldp->cbp = (uchar *) comline ;
	ldp->clen = *lenp ;

	movc((int) *lenp,comline,original) ;

	curpos = *lenp ;

/* get old terminal characteristics and remember for later */

	tty_control(ofd,FM_GETMODE,&old) ;

	mode = fm_noecho | fm_nofilter ;
	tty_control(ofd,FM_SETMODE,mode) ;


	exit_flag = FALSE ;

/* cause to go through the loop the first time w/o reading */

	flag = TRUE ;


/* enter continuous loop until user types a <cr> (carriage return) */

	while (flag || ((! exit_flag) && 
	    ((tty_read(&c,1L) > 0) && (c != '\r')))) {

	    flag = FALSE ;

	    f_redraw = NO ;
	    switch ((unsigned int) c) {

/* escape character */
	    case CH_ESC:
	        if (tty_read(&c,1L) <= 0) 
			break ;

	        if (c == '[') 
			goto case_csi ;

	        if (c == 'N') 
			goto case_ss2 ;

	        if (c == 'O') 
			goto case_ss3 ;

	        break ;

	    case CH_SS2:

case_ss2:
	        break ;

	    case CH_SS3:

case_ss3:
	        f_redraw = ss3(gdp,ldp,hp,&hi,comline,lenp,&curpos) ;

	        break ;

	    case CH_CSI:

case_csi:
	        f_redraw = csi(gdp,ldp,hp,&hi,comline,lenp,&curpos) ;

	        break ;

/* delete character */
	    case 0x08:
	    case 0x7F:
	        if (curpos <= 0) 
			break ;

	        if (comline[curpos - 1] == '\t') 
			f_redraw = TRUE ;

	        for (i = curpos ; i < *lenp ; i += 1)
	            comline[i - 1] = comline[i] ;

	        (*lenp) -= 1 ;
	        curpos -= 1 ;

	        if (! f_redraw) {

	            bp = buf ;
	            bp += bufprintf(bp,
			"\b%s",TERMSTR_DCH) ;

	            write(ofd,buf,bp - buf) ;

	        }

	        break ;

/* got a ^R so we will refresh the line */
	    case 0x12:
	        write(ofd," ^R\r\n",5) ;

	        f_redraw = YES ;
	        break ;

/* got a ^U or ^X */
	    case 0x15:
	    case 0x18:
	        write(ofd," ^U\r\n",5) ;

	        *lenp = 0 ;			/* set input length to zip */
	        curpos = 0 ;
	        f_redraw = YES ;
	        break ;

	    case '\n':
	        exit_flag = TRUE ;
	        break ;

/* handle all normal characters as input */
	    default:
	        if (c < 0x20) 
			break ;		/* discard control characters */

/* handle a <tab> character as normal for now */
	    case 0x09:
	        if (curpos >= CBL) {

	            tty_write("\007",1) ;

	            break ;
	        }

	        for (i = *lenp ; i > curpos ; i -= 1)
	            comline[i] = comline[i - 1] ;

	        comline[curpos] = c ;
	        curpos += 1 ; 
	        if (*lenp < CBL) 
			(*lenp) += 1 ;

	        if (c == '\t') {

	            f_redraw = TRUE ;

	        } else {

	            bp = buf ;
	            bp += bufprintf(bp,BUFLEN,
			"%s%c",TERMSTR_ICH,c) ;

	            write(ofd,buf,bp - buf) ;

	        }

	    } /* end switch */


/* redraw line if it has changed any */

	    if (f_redraw) {

	        comline[*lenp] = 0 ;

	        bp = buf ;

	        *bp++ = '\r' ;

	        for (i = 0 ; i < pl ; i += 1) {

	            if (ps[i] == '\t') {

	                bp += bufprintf(bp,BUFLEN,
				"\033[8X%c",'\t') ;

	            } else 

	                *bp++ = ps[i] ;

	        } /* end for */

	        for (i = 0 ; i < (*lenp) ; i += 1) {

	            if (comline[i] == '\t') {

	                bp += bufprintf(bp,BUFLEN,
				"\033[8X%c",'\t') ;

	            } else 

	                *bp++ = comline[i] ;

	        } /* end for */

	        bp += bufprintf(bp,BUFLEN,
			"%K\r%W%W",ps,pl,comline,curpos) ;

	        write(ofd,buf,bp - buf) ;

	    }

	} /* end while */


/* set the terminal mode back to what it was before */

	tty_control(ofd,FM_SETMODE,old) ;

	tty_write(ofd,"\r\n",2) ;	/* echo the <cr> typed by user */

	return 0 ;
}
/* end subroutine (editline) */


static int csi(gdp,ldp,hp,hi,comline,lenp,cp)
struct gdata	*gdp ;
struct ldata	*ldp ;
struct history	*hp ;
int	*hi ;
char	comline[] ;
int	*lenp ;
int	*cp ;
{
	int	num = 0 ;
	int	f_redraw ;

	uchar	c, buf[80], *bp ;


	if (tty_read(gdp->ifd,&c,1L) < 1) 
		return (NO) ;

	f_redraw = NO ;
	switch ((int) c) {

/* request to move the cursor up one line */
	case 'A':
	    *hi = hi_backup(hp,*hi) ;

	    hi_copy(hp,*hi,comline,lenp) ;

	    if (*cp > *lenp) 
		*cp = *lenp ;

	    return (YES) ;

/* request to move the cursor down one line */
	case 'B':
	    *hi = hi_forward(hp,*hi,comline,lenp) ;

		if (*hi == hp->wi) {

			movc((int) ldp->olen,ldp->obp,comline) ;

			*lenp = ldp->olen ;

		} else hi_copy(hp,*hi,comline,lenp) ;

	    if (*cp > *lenp) *cp = *lenp ;

	    return (YES) ;

/* request to move the cursor right one character */
	case 'C':
	    if (*cp >= *lenp) 
		break ;

	    if (comline[*cp] == '\t') 
		write(gdp->ofd,"\t",1) ;

	    else 
		write(gdp->ofd,"\033[C",3) ;

	    *cp += 1 ;

	    break ;

/* request to move the cursor left one character */
	case 'D':
	    if (*cp <= 0) 
		break ;

	    *cp -= 1 ;

	    if (comline[*cp] == '\t') 
		f_redraw = YES ;

	    else 
		write(gdp->ofd,"\b",1) ;

	    break ;

/* request to move the cursor to the beginning of the current line */
	case 'H':
	    *cp = 0 ;
	    f_redraw = YES ;

	    break ;

/* handle other escape codes */
	default:
	    if (! isdigit(c)) 
		break ;

	    num = c - '0' ;

	    while ((tty_read(gdp->ifd,&c,1L) > 0) && isdigit(c))
	        num = (num * 10) + c - '0' ;

	    if (c != '~') 
		break ;

	    switch (num) {

/* FIND key */
	    case 1:
	        bp = buf ;
	        bp += bufprintf(bp,BUFLEN,
			"\ryou hit the FIND key%K\n") ;

	        tty_write(gdp->ofd,buf,(bp - buf)) ;


#ifdef	COMMENT
		ldp->current = *hi ;

	    *hi = hi_backup(hp,*hi) ;

	    hi_copy(hp,*hi,comline,lenp) ;

	    if (*cp > *lenp) 
		*cp = *lenp ;
#endif
	        f_redraw = TRUE ;
	        break ;

/* INSERT */
	    case 2:

/* REMOVE */
	    case 3:

/* SELECT key */
	    case 4:

/* PREVIOUS key */
	    case 5:

/* NEXT key */
	    case 6:

/* F6 key */
	    case 7:

	    default:
	        bp = buf ;
	        bp += bufprintf(bp,BUFLEN,
			"\rfunction key w/ code %d%K\n",num) ;

	        tty_write(gdp->ofd,buf,(bp - buf)) ;

	        f_redraw = TRUE ;
	        break ;

	        break ;

	    } /* end switch (inner) */

	} /* end switch (outer) */

	return f_redraw ;
}
/* end subroutine (csi) */


static int ss3(gdp,ldp,hp,hi,comline,lenp,cp)
struct gdata	*gdp ;
struct ldata	*ldp ;
struct history	*hp ;
int	*hi ;
char	comline[] ;
int	*lenp ;
int	*cp ;
{
	int	f_redraw ;

	char	c ;


	if (tty_read(gdp->ifd,&c,1L) < 1) 
		return (NO) ;

	f_redraw = NO ;
	switch ((int) c) {

/* request to move the cursor up one line */
	case 'A':
	    *hi = hi_backup(hp,*hi) ;

	    hi_copy(hp,*hi,comline,lenp) ;

	    if (*cp > *lenp) 
		*cp = *lenp ;

	    return (YES) ;

/* request to move the cursor down one line */
	case 'B':
	    *hi = hi_forward(hp,*hi,comline,lenp) ;

		if (*hi < 0) {

			movc((int) ldp->olen,ldp->obp,comline) ;

			*hi = hp->wi ;

		} else 
			hi_copy(hp,*hi,comline,lenp) ;

	    if (*cp > *lenp) 
		*cp = *lenp ;

	    return (YES) ;

/* request to move the cursor right one character */
	case 'C':
	    if (*cp >= *lenp) 
		break ;

	    if (comline[*cp] == '\t') 
		write(gdp->ofd,"\t",1) ;

	    else 
		write(gdp->ofd,"\033[C",3) ;

	    *cp += 1 ;
	    break ;

/* request to move the cursor left one character */
	case 'D':
	    if (*cp <= 0) 
		break ;

	    *cp -= 1 ;

	    if (comline[*cp] == '\t') 
		f_redraw = YES ;

	    else 
		write(gdp->ofd,"\b",1) ;

	    break ;

/* request to move the cursor to the beginning of the current line */
	case 'H':
	    *cp = 0 ;
	    f_redraw = YES ;

	    break ;

/* all other escape codes are no-ops for now */
	default:
	    ;

	} /* end switch */

	return (f_redraw) ;
}
/* end subroutine (ss3) */


/* history file operations */
int hi_copy(hp,ci,buf,len)
struct history	*hp ;
char		*buf ;
int		ci ;
int		*len ;
{
	int	i ;


	if (hp->c == 0) { 

	    *len = 0 ; 
	    return (hp->wi) ; 
	}

	if (ci == hp->wi) ci = hp->ri ;

	i = 0 ;
	while (hp->buf[ci] != CH_EOH) {

	    buf[i++] = hp->buf[ci] ;
	    ci = (ci + 1) % HSIZE ;
	}

	ci = (ci + 1) % HSIZE ;
	*len = i ;
	return (ci) ;
}
/* end subroutine (hi_copy) */


#if	F_FORWARD

int hi_forward(hp,ci)
struct history	*hp ;
int		ci ;
{


	if (hp->c == 0) 
		return hp->wi ;

	if (ci == hp->wi) 
		ci = hp->ri ;

	while (hp->buf[ci] != CH_EOH) 
		ci = (ci + 1) % HSIZE ;

	ci = (ci + 1) % HSIZE ;
	return (ci) ;
}
/* end subroutine (hi_forward) */

#endif


int hi_backup(hp,ci)
struct history	*hp ;
int		ci ;
{


	if (hp->c == 0) 
		return (hp->wi) ;

	if (ci == hp->ri) 
		ci = hp->wi ;

	ci = (ci - 1 + HSIZE) % HSIZE ;
	if (ci == hp->ri) 
		return (ci) ;

	ci = (ci - 1 + HSIZE) % HSIZE ;
	while ((hp->buf[ci] != CH_EOH) && (ci != hp->ri))
	    ci = (ci - 1 + HSIZE) % HSIZE ;

	if (hp->buf[ci] == CH_EOH) 
		ci = (ci + 1) % HSIZE ;

	return (ci) ;
}
/* end subroutine (hi_backup) */


int hi_insert(hp,buf,len)
struct history	*hp ;
char		*buf ;
int		len ;
{
	int	i, ri, wi ;


	while ((HSIZE - hp->c) < (len + 1)) {

	    ri = hp->ri ;
	    while (hp->buf[ri] != CH_EOH) 
		ri = (ri + 1) % HSIZE ;

	    ri = (ri + 1) % HSIZE ;

	    hp->c -= ((ri - hp->ri + HSIZE) % HSIZE) ;
	    hp->ri = ri ;
	    hp->n -= 1 ;

	} /* end while */

	wi = hp->wi ;
	for (i = 0 ; i < len ; i += 1) {

	    hp->buf[wi] = buf[i] ;
	    wi = (wi + 1) % HSIZE ;

	} /* end for */

	hp->buf[wi] = CH_EOH ;
	wi = (wi + 1) % HSIZE ;

	hp->c += (len + 1) ;
	hp->n += 1 ;
	hp->wi = wi ;

	return (wi) ;
}
/* end subroutine (hi_insert) */



int isdigit(c)
uchar	c ;
{
	return (c >= '0' && c <= '9') ? YES : NO ;
}



