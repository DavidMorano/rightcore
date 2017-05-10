/* process */

/* process the input files */


#define	CF_DEBUG 	0


/* revision history :

	= 1994-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	Arguments:

	- g		global program data
	- filename	file to process
	- kfp		key (BIO) file pointer

	Returns:

	>=0		OK
	<0		error code


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<sfstr.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<field.h>
#include	<char.h>
#include	<eigendb.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	keysstart(), keysadd(), keysfinish() ;

extern char	*strcpylow(char *,const char *) ;
extern char	*strcpyup(char *,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylow(char *,const char *,int) ;


/* external variables */

#if	CF_DEBUG
extern struct global	g ;
#endif


/* forward references */

static int	ignoreline() ;


/* exported subroutines */


int process(pip,ofp,edbp,terms,delimiter,ignorechars,filename)
struct proginfo	*pip ;
bfile		*ofp ;
EIGENDB		*edbp ;
uchar		terms[] ;
char		delimiter[], ignorechars[] ;
char		filename[] ;
{
	bfile		infile, *ifp = &infile ;

	HDB		keydb ;

	struct ustat	sb ;

	struct field	fsb ;

	offset_t	offset, recoff ;

	int	rs ;
	int	len, dlen ;
	int	reclen ;
	int	hashsize ;
	int	entries = 0 ;
	int	c, nk, n = 0 ;
	int	f_start, f_ent, f_finish ;
	int	f_open = FALSE ;

	char	linebuf[LINELEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: entered, file=%s\n",filename) ;
#endif

/* open the file that we are supposed to process */

	if (filename[0] == '-') {
	    rs = bopen(ifp,BIO_STDIN,"dr",0666) ;
	} else
	    rs = bopen(ifp,filename,"r",0666) ;

	if (rs < 0)
	    return rs ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: continuing\n") ;
#endif


	dlen = strlen(delimiter) ;


/* figure a default hash DB size based on the input file length */

	if (pip->f.wholefile) {

	    if ((bcontrol(ifp,BC_STAT,&sb) >= 0) && (sb.st_size > 0)) {
	        hashsize = sb.st_size / 6 ;

	    } else
	        hashsize = 1000 ;

	} else
	    hashsize = 20 ;


/* go to it, read the file line by line  */

	f_start = pip->f.wholefile ;
	f_ent = FALSE ;
	f_finish = FALSE ;

	offset = 0 ;
	recoff = 0 ;
	reclen = 0 ;
	while ((len = bgetline(ifp,linebuf,LINELEN)) > 0) {

	    int	sl, ll = len, lr ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("process: line> %W",linebuf,len) ;
#endif

/* we ignore lo...ng lines entirely, but we try to resynchronize up */

	    if (linebuf[ll - 1] != '\n') {
	        offset += len ;
	        while (((c = bgetc(ifp)) != SR_EOF) && (c != '\n')) {
	            offset += 1 ;
	            recoff += 1 ;
	        } /* end while */
	        continue ;
	    } /* end if (discarding extra line input) */

	    linebuf[--ll] = '\0' ;
	    sl = sfshrink(linebuf,ll,&cp) ;

/* figure out where we start and/or end an entry */

	    if (! pip->f.wholefile) {

	        if (delimiter[0] == '\0') {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("process: delimit every line\n") ;
#endif

	            f_start = TRUE ;
	            f_finish = TRUE ;

	        } else if (CHAR_ISWHITE(delimiter[0])) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("process: delimit using blank lines\n") ;
#endif

	            if (sl == 0) {

	                if (f_ent)
	                    f_finish = TRUE ;

	            } else {

	                if (! f_ent)
	                    f_start = TRUE ;

	            }

	        } else {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("process: specified delimiter\n") ;
#endif

	            if (strncmp(linebuf,delimiter,dlen) == 0) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("process: got a delimiter\n") ;
#endif

	                if (f_ent)
	                    f_finish = TRUE ;

	            } else {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("process: regular line\n") ;
#endif

	                if (! f_ent)
	                    f_start = TRUE ;

	            }

	        } /* end if (delimiter cascade) */

	        if (f_finish) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("process: finishing off entry\n") ;
#endif

	            f_ent = FALSE ;
	            f_finish = FALSE ;
	            if (n > 0) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("process: closing a DB\n") ;
#endif

	                f_open = FALSE ;
	                reclen = ((int) (offset - recoff)) + len ;
	                nk = keysfinish(pip,&keydb,ofp,filename,recoff,reclen) ;

	                if (nk > 0)
	                    entries += 1 ;

	            }

	        } /* end if (finishing entry) */

	    } /* end if (not whole file -- determining entry boundary) */

	    if (f_start) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("process: starting off entry, offset=%ld\n",
	                offset) ;
#endif

	        f_start = FALSE ;
	        f_ent = TRUE ;

	        n = 0 ;
	        reclen = 0 ;
	        recoff = offset ;
	        if (! f_open) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("process: opening a DB\n") ;
#endif

	            f_open = TRUE ;
	            keysstart(pip,&keydb,hashsize) ;

	        }

	    } /* end if (starting entry) */

/* process this current input line if we are supposed to */

	    if ((ll >= pip->minwordlen) && f_ent && 
	        (! ignoreline(linebuf,ll,ignorechars))) {

	        lr = field_init(&fsb,linebuf,ll) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("process: length in line %d\n",lr) ;
#endif

/* loop on parsing words (which become keys) from the input line */

	        while (fsb.rlen > 0) {

	            int		fl ;

	            char	lowbuf[LINELEN + 1] ;


#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("process: top of field while, rlen=%d\n",
	                    fsb.rlen) ;
#endif

	            if ((pip->keys > 0) && (n >= (pip->keys + KEYS)))
	                break ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("process: about to get next field\n") ;
#endif

	            if ((fl = field_word(&fsb,terms)) >= pip->minwordlen) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1) {

	                    if (fsb.flen > 0)
	                        eprintf("process: key=%W\n",fsb.fp,fsb.flen) ;

	                    else
	                        eprintf("process: zero length field\n") ;

	                } /* end block */
#endif /* CF_DEBUG */

	                if (pip->maxwordlen > 0)
	                    fl = MIN(fsb.flen,pip->maxwordlen) ;

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("process: fl=%d\n",fl) ;
#endif

/* check if this word is in the eigenword database */

	                (void) strwcpylow(lowbuf,fsb.fp,fsb.flen) ;

	                if (pip->eigenwords > 0) {

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        eprintf("process: have eigenwords=%d\n",
	                            pip->eigenwords) ;
#endif

			if (eigendb_exists(edbp,lowbuf,fsb.flen) < 0) {

	                        n += 1 ;
	                        keysadd(pip,&keydb,lowbuf,fl) ;

	                    }

	                } else {

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        eprintf("process: no eigenwords\n") ;
#endif

	                    n += 1 ;
	                    keysadd(pip,&keydb,lowbuf,fl) ;

	                }

	            } /* end if (non-zero field length) */

	        } /* end while (getting word fields from the line) */

		field_free(&fsb) ;

	    } /* end if (we were in an entry) */

	    offset += len ;
	    reclen += len ;

	} /* end while (looping reading lines) */


/* write out (finish off) the last (or only) entry */

	if (f_open) {

	    f_open = FALSE ;
	    reclen = (int) (offset - recoff) ;
	    nk = keysfinish(pip,&keydb,ofp,filename,recoff,reclen) ;

	    if (nk > 0)
	        entries += 1 ;

	}


	bclose(ifp) ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: exiting, entries=%d rs=%d\n",entries,rs) ;
#endif

	return entries ;
}
/* end subroutine (process) */


/* which input lines are supposed to be ignored ? */
static int ignoreline(linebuf,ll,ignorechars)
char	linebuf[], ignorechars[] ;
int	ll ;
{


	if ((ignorechars != NULL) && (linebuf[0] == '%')) {

	    if (ll < 2) return TRUE ;

	    if (strchr(ignorechars,linebuf[1]) != NULL)
	        return TRUE ;

	} /* end if */

	return FALSE ;
}
/* end subroutine (ignoreline) */



