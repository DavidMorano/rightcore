/* process */

/* do the actual file cleaning up */


#define	CF_DEBUG	1


/* revision history:

	= 99/03/01, David A­D­ Morano

	This subroutine was originally written (its a fairly
	straight-forward text processing loop).


*/


/******************************************************************************

	Read the given file and clean it up.


******************************************************************************/


#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	S_SEARCH	0
#define	S_GOTN		1
#define	S_GOTC		2
#define	S_GOTCV		3



/* external subroutines */

extern int	sfsub(const char *,int,const char *,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;


/* external variables */


/* local structures */


/* local forward references */


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







int process(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	FIELD	fsb ;

	bfile	infile ;

	int	rs, rs1, len, sl, cl ;
	int	state ;
	int	c_book, c_chapter, c_verse ;
	int	c_word ;
	int	v ;
	int	f_leader = FALSE ;
	int	f_number = FALSE ;
	int	f_something = FALSE ;
	int	f_book = FALSE ;
	int	f ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*sp, *cp ;


	state = S_SEARCH ;
	c_book = 0 ;
	c_chapter = 0 ;
	c_verse = 0 ;
	c_word = 0 ;

	if ((fname != '\0') && (strcmp(fname,"-") != 0))
		rs = bopen(&infile,fname,"r",0666) ;

	else
		rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    return rs ;

/* go through the loops */

	while ((rs = breadline(&infile,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: line>%w<\n",linebuf,len) ;
#endif

	    sp = linebuf ;
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
	    field_init(&fsb,sp,sl) ;

	    while (field_word(&fsb,terms) >= 0) {

	        if (fsb.flen == 0)
	            continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: s=%u f>%w<\n",
	                state,fsb.fp,fsb.flen) ;
#endif

	        f_number = FALSE ;
	        switch (state) {

	        case S_SEARCH:
	            {

			sl = 5 ;
	                f = (fsb.flen >= sl) ;
	                f = f && (strncmp("book=",fsb.fp,sl) == 0) ;

	                if (! f) {

			    sl = 7 ;
	                    f = (fsb.flen == sl) ;
	                    f = f && (strncmp("Chapter",fsb.fp,fsb.flen) == 0) ;

			    if (! f) {

			    sl = 5 ;
	                    f = (fsb.flen == sl) && (c_word <= 0) ;
	                    f = f && (strncmp("Psalm",fsb.fp,fsb.flen) == 0) ;

			    }

	                    if (f) {

				c_verse = 0 ;
	                        cp = fsb.fp + sl ;
	                        cl = fsb.flen - sl ;
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

	                        rs1 = cfdeci(fsb.fp,fsb.flen,&v) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("process: f>%w< cfdeci() rs=%d\n",
	                                fsb.fp,fsb.flen,rs1) ;
#endif

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

	                            bwrite(ofp,fsb.fp,fsb.flen) ;

	                            if (fsb.term == '.')
	                                bputc(ofp,'.') ;

	                            c_word += 1 ;

	                        }

	                    }

	                } else {

				f_book = TRUE ;
	                    c_book += 1 ;
				c_verse = 0 ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3)) {
	                        debugprintf("process: new book=%u\n",c_book) ;
	                        debugprintf("line>%w<\n",linebuf,len) ;
				}
#endif

	                }

	            }

	            break ;

	        case S_GOTN:
	            {

	                if (fsb.term == '.') {

	                    state = S_GOTCV ;
	                    c_verse = 1 ;
	                    rs1 = cfdeci(fsb.fp,(fsb.flen - 1),&c_chapter) ;

	                } else {

	                    state = S_GOTC ;
	                    rs1 = cfdeci(fsb.fp,fsb.flen,&c_chapter) ;

	                }

	                f_number = (rs1 >= 0) ;
	                if (c_word > 0)
	                    bputc(ofp,'\n') ;

	            }

	            break ;

	        case S_GOTC:
	            {

	                rs1 = cfdeci(fsb.fp,fsb.flen,&v) ;

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

	                bwrite(ofp,fsb.fp,fsb.flen) ;

	                if (fsb.term == '.')
	                    bputc(ofp,'.') ;

	                c_word += 1 ;

			}

	            }

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

	                bwrite(ofp,fsb.fp,fsb.flen) ;

	                if (fsb.term == '.')
	                    bputc(ofp,'.') ;

	                c_word += 1 ;

	            }

	            break ;

	        } /* end switch */

		if (f_book)
			break ;

	    } /* end while (fields) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: c_word=%u f_something=%u f_number=%u\n",
	            c_word,f_something,f_number) ;
#endif

		if (f_book)
			continue ;

	    if (f_something && (! f_number))
	        bputc(ofp,'\n') ;

	    field_free(&fsb) ;

	} /* end while (reading lines) */

	bclose(&infile) ;


	return rs ;
}
/* end subroutine (process) */



