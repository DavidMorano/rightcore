/* progrena */

/* do the actual file cleaning up */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1999-03-01, David A­D­ Morano

	This subroutine was originally written (its a fairly
	straight-forward text processing loop).


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Read the given file and clean it up.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	field_word(FIELD *,const uchar *,const char **) ;

#if	CF_DEBUG || CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct cv {
	uint		c, v ;
} ;


/* forward references */

static int	isbook(const char *,int) ;
static int	ischap(const char *,int) ;
static int	iscite(const char *,int,struct cv *) ;


/* local variables */

static const uchar	wterms[] = {
	0x00, 0x3F, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*states[] = {
	"search",
	"getbook",
	"getchap",
	"getverse",
	NULL
} ;

enum states {
	state_search,
	state_getbook,
	state_getchap,
	state_getverse,
	state_overlast
} ;


/* exported subroutines */


int progrena(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	struct cv	cite ;

	FIELD	fsb ;

	bfile	infile ;

	uint	c_book, c_chap, c_verse ;
	uint	c_field ;
	uint	c_printed ;

	int	rs = SR_OK ;
	int	len, sl ;
	int	fl ;
	int	state ;
	int	wlen = 0 ;
	int	f_abandon = FALSE ;
	int	f_cite = FALSE ;

	const char	*fp ;
	const char	*sp ;

	char	linebuf[LINEBUFLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progrena: fname=%s\n",fname) ;
#endif

	state = state_search ;
	c_book = 0 ;
	c_chap = 0 ;
	c_verse = 0 ;
	c_field = 0 ;
	c_printed = 0 ;

	if ((fname[0] != '\0') && (strcmp(fname,"-") != 0)) {
	    rs = bopen(&infile,fname,"r",0666) ;
	} else
	    rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto ret0 ;

/* go through the loops */

	while ((rs = breadline(&infile,linebuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    if (linebuf[len - 1] == '\n') len -= 1 ;
	    linebuf[len] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progrena: state=%s\n",states[state]) ;
	        debugprintf("progrena: line>%t<\n",
			linebuf,strlinelen(linebuf,len,60)) ;
	    }
#endif

	    sp = linebuf ;
	    sl = len ;
	    while (sl && CHAR_ISWHITE(*sp)) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	    if (sl == 0)
	        continue ;

/* continue with normal line processing */

	    f_abandon = FALSE ;
	    c_field = 0 ;
	    c_printed = 0 ;
	    if ((rs = field_start(&fsb,sp,sl)) >= 0) {

		while (rs >= 0) {

	            fl = field_word(&fsb,wterms,&fp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progrena: field_word() fl=%d\n",
			fl) ;
#endif

		    if (fl < 0)
			break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progrena: state=%s f>%t<\n",
	                    states[state],fp,fl) ;
#endif

	            if (fl == 0) continue ;

	            switch (state) {

	            case state_search:
	                if (isbook(fp,fl)) {
	                    state = state_getbook ;
	                } else if (ischap(fp,fl)) {
	                    state = state_getchap ;
	                } else if (iscite(fp,fl,&cite)) {

	                    c_chap = cite.c ;
	                    c_verse = cite.v ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progrena: cite c=%u v=%u\n",
	                    c_chap,c_verse) ;
#endif

	                    state = state_getverse ;
	                    f_cite = FALSE ;
	                } else {
	                    if (! f_cite) {
	                        f_cite = TRUE ;
	                        rs = bprintf(ofp,"\n%u:%u:%u\n",
	                            c_book,c_chap,c_verse) ;
	                    }

	                    if ((rs >= 0) && (c_printed > 0))
	                        rs = bprintf(ofp," ") ;

	                    if (rs >= 0) {
	                        c_printed += 1 ;
	                        rs = bwrite(ofp,fp,fl) ;
	                    }

	                    state = state_getverse ;
	                }

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progrena: state=search newstate=%s\n",
	                        states[state]) ;
#endif

	                break ;

	            case state_getbook:
	                rs = cfdecui(fp,fl,&c_book) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progrena: state=getbook c_book=%u\n",
	                        c_book) ;
#endif

	                f_abandon = TRUE ;
	                state = state_search ;
	                break ;

	            case state_getchap:
	                rs = cfdecui(fp,fl,&c_chap) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progrena: state=getchap c_chap=%u\n",
	                        c_chap) ;
#endif

	                f_abandon = TRUE ;
	                state = state_search ;
	                break ;

	            case state_getverse:
	                if (! f_cite) {
	                    f_cite = TRUE ;
	                        rs = bprintf(ofp,"\n%u:%u:%u\n",
	                        c_book,c_chap,c_verse) ;
	                }
	                if ((rs >= 0) && (c_printed > 0))
	                    rs = bprintf(ofp," ") ;

	                if (rs >= 0) {
	                    c_printed += 1 ;
	                    rs = bwrite(ofp,fp,fl) ;
	                }

	                break ;

	            } /* end switch */

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("progrena: switch-end rs=%d\n",rs) ;
	                    debugprintf("progrena: c_field=%u\n",c_field) ;
	                    debugprintf("progrena: newstate=%s\n",
				states[state]) ;
	                    debugprintf("progrena: f_abandon=%u\n",
				f_abandon) ;
			}
#endif /* CF_DEBUG */

	            if (rs < 0)
	                break ;

	            if (f_abandon)
	                break ;

	            c_field += 1 ;

	        } /* end while (fields) */

	        field_finish(&fsb) ;
	    } /* end if (fielding) */

	    if (rs < 0)
	        break ;

	    if (c_printed > 0) {
	        rs = bprintf(ofp,"\n") ;
	        wlen += rs ;
	    }

	    if (state == state_getverse)
		state = state_search ;

	} /* end while (reading lines) */

	if ((rs >= 0) && (wlen > 0)) {
	        rs = bprintf(ofp,"\n") ;
	        wlen += rs ;
	}

	bclose(&infile) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progrena: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progrena) */


/* local subroutines */


static int isbook(fp,fl)
const char	*fp ;
int		fl ;
{
	int	n = 4 ;
	int	f ;


	f = ((fl == n) && (strncasecmp(fp,"BOOK",fl) == 0)) ;
	return f ;
}
/* end subroutine (isbook) */


static int ischap(fp,fl)
const char	*fp ;
int		fl ;
{
	int	n = 7 ;
	int	f ;


	f = ((fl == n) && (strncasecmp(fp,"Chapter",fl) == 0)) ;
	return f ;
}
/* end subroutine (ischap) */


static int iscite(fp,fl,cvp)
const char	*fp ;
int		fl ;
struct cv	*cvp ;
{
	const char	*tp ;

	int	rs1 ;
	int	cl ;
	int	f = FALSE ;

	const char	*cp ;


	if ((tp = strnchr(fp,fl,':')) != NULL) {
	    rs1 = cfdecui(fp,(tp - fp),&cvp->c) ;
	    if (rs1 >= 0) {
	        cp = (tp + 1) ;
	        cl = ((fp + fl) - (tp + 1)) ;
	        rs1 = cfdecui(cp,cl,&cvp->v) ;
	        f = (rs1 >= 0) ;
	    }
	} /* end if */

	return f ;
}
/* end subroutine (iscite) */



