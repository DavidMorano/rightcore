/* progkjv */

/* do the actual file cleaning up */


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

#define	S_SEARCH	0
#define	S_GOTN		1
#define	S_GOTC		2
#define	S_GOTCV		3


/* external subroutines */

extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const uchar	terms[] = {
	0x00, 0x1F, 0x00, 0x00,
	0x01, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int progkjv(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	FIELD	fsb ;

	bfile	infile ;

	const int	llen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	len, sl, cl ;
	int	fl ;
	int	state ;
	int	c_book, c_chapter, c_verse ;
	int	c_word ;
	int	v ;
	int	f_leader = FALSE ;
	int	f_number = FALSE ;
	int	f_something = FALSE ;
	int	f_book = FALSE ;
	int	f ;

	const char	*sp, *cp ;
	const char	*fp ;

	char	lbuf[LINEBUFLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

	state = S_SEARCH ;
	c_book = 0 ;
	c_chapter = 0 ;
	c_verse = 0 ;
	c_word = 0 ;

	if ((fname[0] != '\0') && (strcmp(fname,"-") != 0)) {
	    rs = bopen(&infile,fname,"r",0666) ;
	} else
	    rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto ret0 ;

/* go through the loops */

	while ((rs = breadline(&infile,lbuf,llen)) > 0) {
	    len = rs ;

	    if (lbuf[len - 1] == '\n') len -= 1 ;
	    lbuf[len] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: line>%t<\n",lbuf,len) ;
#endif

	    sp = lbuf ;
	    sl = len ;
	    while (sl && CHAR_ISWHITE(*sp)) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	    if (sl == 0)
	        continue ;

	    cl = sfsub(sp,sl,"subbook=",&cp) ;

	    if (cl > 0)
	        continue ;

/* continue with normal line processing */

	    f_book = FALSE ;
	    f_something = FALSE ;
	    c_word = 0 ;
	    if ((rs = field_start(&fsb,sp,sl)) >= 0) {

	        while ((fl = field_word(&fsb,terms,&fp)) >= 0) {
	            if (fl == 0) continue ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("process: s=%u f>%t<\n",
	                    state,fp,fl) ;
#endif

	            f_number = FALSE ;
	            switch (state) {

	            case S_SEARCH:
	                {

	                    sl = 5 ;
	                    f = (fl >= sl) ;
	                    f = f && (strncmp("book=",fp,sl) == 0) ;

	                    if (! f) {

	                        sl = 7 ;
	                        f = (fl == sl) ;
	                        f = f && (strncmp("Chapter",fp,fl) == 0) ;

	                        if (! f) {

	                            sl = 5 ;
	                            f = (fl == sl) && (c_word <= 0) ;
	                            f = f && (strncmp("Psalm",fp,fl) == 0) ;

	                        }

	                        if (f) {

	                            c_verse = 0 ;
	                            cp = fp + sl ;
	                            cl = fl - sl ;
	                            if (fsb.term == '.') {

	                                f_leader = FALSE ;
	                                state = S_GOTCV ;
	                                c_verse = 1 ;
	                                cl -= 1 ;
	                                rs1 = cfdeci(cp,cl,&c_chapter) ;
	                                f_number = (rs1 >= 0) ;
	                                if (c_word > 0)
	                                    bputc(ofp,'\n') ;

	                            } else {

	                                if (cl > 0) {

	                                    state = S_GOTC ;
	                                    rs1 = cfdeci(cp,cl,&c_chapter) ;
	                                    f_number = (rs1 >= 0) ;

	                                } else
	                                    state = S_GOTN ;

	                            }

	                        } else {

	                            rs1 = cfdeci(fp,fl,&v) ;

	                            if (rs1 >= 0) {

	                                f_number = (rs1 >= 0) ;
	                                if (c_word > 0)
	                                    bputc(ofp,'\n') ;

	                                f_leader = FALSE ;
	                                c_verse = v ;

	                            } else {

	                                if (! f_leader) {

	                                    f_leader = TRUE ;
	                                    c_word = 0 ;
	                                    bprintf(ofp,"\n%u:%u:%u\n",
	                                        c_book,c_chapter,c_verse) ;

	                                }

	                                f_something = TRUE ;
	                                if (c_word > 0)
	                                    bputc(ofp,' ') ;

	                                bwrite(ofp,fp,fl) ;

	                                if (fsb.term == '.')
	                                    bputc(ofp,'.') ;

	                                c_word += 1 ;

	                            }

	                        }

	                    } else {

	                        f_book = TRUE ;
	                        c_book += 1 ;
	                        c_verse = 0 ;

	                    }

	                } /* end if */

	                break ;

	            case S_GOTN:
	                {

	                    if (fsb.term == '.') {

	                        state = S_GOTCV ;
	                        c_verse = 1 ;
	                        rs1 = cfdeci(fp,(fl - 1),&c_chapter) ;

	                    } else {

	                        state = S_GOTC ;
	                        rs1 = cfdeci(fp,fl,&c_chapter) ;

	                    }

	                    f_number = (rs1 >= 0) ;
	                    if (c_word > 0)
	                        bputc(ofp,'\n') ;

	                }

	                break ;

	            case S_GOTC:
	                {

	                    rs1 = cfdeci(fp,fl,&v) ;

	                    f_number = (rs1 >= 0) ;
	                    if (rs1 >= 0) {

	                        state = S_GOTCV ;
	                        c_verse = v ;
	                        if (c_word > 0)
	                            bputc(ofp,'\n') ;

	                    } else {

	                        f_leader = FALSE ;
	                        state = S_SEARCH ;
	                        if (! f_leader) {

	                            f_leader = TRUE ;
	                            c_word = 0 ;
	                            bprintf(ofp,"\n%u:%u:%u\n",
	                                c_book,c_chapter,c_verse) ;

	                        }

	                        f_something = TRUE ;
	                        if (c_word > 0)
	                            bputc(ofp,' ') ;

	                        bwrite(ofp,fp,fl) ;

	                        if (fsb.term == '.')
	                            bputc(ofp,'.') ;

	                        c_word += 1 ;

	                    }

	                } /* end if */

	                break ;

	            case S_GOTCV:
	                {

	                    f_leader = FALSE ;
	                    state = S_SEARCH ;
	                    if (! f_leader) {

	                        f_leader = TRUE ;
	                        c_word = 0 ;
	                        bprintf(ofp,"\n%u:%u:%u\n",
	                            c_book,c_chapter,c_verse) ;

	                    }

	                    f_something = TRUE ;
	                    if (c_word > 0)
	                        bputc(ofp,' ') ;

	                    bwrite(ofp,fp,fl) ;

	                    if (fsb.term == '.')
	                        bputc(ofp,'.') ;

	                    c_word += 1 ;

	                } /* end if */

	                break ;

	            } /* end switch */

	            if (f_book) break ;
	            if (rs < 0) break ;
	        } /* end while (fields) */

	        field_finish(&fsb) ;
	    } /* end if (fielding) */

	    if (f_book) continue ;
	    if (f_something && (! f_number)) bputc(ofp,'\n') ;

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	bclose(&infile) ;

ret0:
	return rs ;
}
/* end subroutine (progkjv) */



