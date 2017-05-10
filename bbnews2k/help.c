/* help */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine prints out short descriptions of the interactive
        commands. This routine assumes that all relevant command names in
        HELPFILE are started in column one. And that all other text associated
        with a command starts in column two or more preceded by white space.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"help.h"
#include	"ds.h"


/* local defines */

#define	NOTFULL(a)	(((a) != ' ') && ((a) != '\n'))


/* external variables */

extern struct proginfo	g ;


/* local variables */

static int	page ;


/* exported subroutines */


int help(dsp,cmd, pageno)
DS	*dsp ;
int	cmd ;
int	pageno ;
{
	struct proginfo	*pip = dsp->pip ;

	bfile	hfile, *hfp = &hfile ;

	offset_t	offset ;

	int	rs ;
	int	ml, len, nextpage ;
	int	f_gotone, f_eof = FALSE ;
	int	f_done, f_first = FALSE ;

	char	helpline[LINEBUFLEN], *cp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("help: ent w/ cmd=%04X\n",cmd) ;
#endif

	if (pip->hfname == NULL) 
		return RHELP_NOFILE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("help: no help file\n") ;
#endif

	if ((rs = bopen(hfp,g.hfname, "r",0666)) < 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("help: bad open rs=%d\n",rs) ;
#endif

		return RHELP_NOFILE ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("help: got a file\n") ;
#endif

	pages[0] = 0 ;
	offset = 0 ;

/* position to the top of the message window area */

	wmove(dsp->w_viewer,0,0) ;


	if (pageno < 0) pageno = 0 ;

	if (pages[pageno] == -1) pageno = 0 ;

	if (pageno == 0) {

	    f_first = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("help: page=0 cmd=%04X\n",cmd) ;
#endif

	    if ((cmd != '?') && (cmd != '\0')) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	            debugprintf("help: fast forwarding to specific section\n") ;
#endif

/* fast forward to the command that we are looking for */

	        while ((len = breadline(hfp,helpline, LINEBUFLEN)) > 0) {

	            offset += len ;
	            if (helpline[0] == cmd) 
			break ;

	        } /* end while */

	        if (len <= 0) {

			bclose(hfp) ;

			return RHELP_NOCMD ;
		}

	    } else 
		pages[0] = 0 ;

	} else 
	    offset = (offset_t) bseek(hfp, pages[pageno], SEEK_SET) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("help: we got to the start\n") ;
#endif

/* read out the text that we want */

	f_done = FALSE ;
	if ((cmd == '\0') || (cmd == '?')) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("help: command summary\n") ;
#endif

/* give command summary */

	    for (ml = 0 ; ml < dsp->nm ; ml += 1) {

/* get a line from the help file */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	            debugprintf("help: working on a displaying a line\n") ;
#endif

	        f_gotone = FALSE ;
	        while ((! f_gotone) &&
	            ((len = breadline(hfp,helpline, LINEBUFLEN)) > 0)) {

	            if ((helpline[0] != ' ') && (helpline[0] != '\n')) {

	                f_gotone = TRUE ;
	                cp = (helpline[0] == '\t') ? helpline + 1 : helpline ;

	                if (helpline[len - 1] == '\n')
	                    helpline[len - 1] = '\0' ;

	                else 
	                    helpline[len] = '\0' ;

	                mvwprintw(dsp->w_viewer,ml,0,
	                    "%s",cp) ;

	                wclrtoeol(dsp->w_viewer) ;

	            }

	            offset += len ;

	        } /* end while */

	        if (! f_gotone) {

	            wmove(dsp->w_viewer,ml,0) ;

	            wclrtoeol(dsp->w_viewer) ;

	        }

	    } /* end for */

	    if (! f_gotone) f_done = TRUE ;

	} else {

/* give full explanation */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("help: give full (specific) explanation\n") ;
#endif

	    ml = 0 ;
	    while ((ml < dsp->nm) &&
	        ((len = breadline(hfp,helpline, LINEBUFLEN)) > 0)) {

	        offset += len ;
	        if (NOTFULL(helpline[0])) break ;

	        if (helpline[len - 1] == '\n')
	            helpline[len - 1] = '\0' ;

	        else 
	            helpline[len] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	            debugprintf("help: >%s\n",helpline) ;
#endif

	        mvwprintw(dsp->w_viewer,ml,0,
	            "%s",helpline) ;

	        wclrtoeol(dsp->w_viewer) ;

	        ml += 1 ;

	    } /* end while */

	    if (ml < dsp->nm) {

	        wmove(dsp->w_viewer,ml,0) ;

	        wclrtobot(dsp->w_viewer) ;

	    }

	    f_done = TRUE ;

	} /* end if (outer) */

/* finish up */

	nextpage = pageno + 1 ;
	if (! f_done) {

	    if (f_first) {

		mvwprintw(dsp->w_root,dsp->pl_bottom,COL_CURRENT,
			"--------------------------") ;

	        wmove(dsp->w_root,dsp->pl_bottom,COL_MORELINES) ;

	        wstandout(dsp->w_root) ;

	        wprintw(dsp->w_root,
	            "more help") ;

	        wstandend(dsp->w_root) ;

	        wprintw(dsp->w_root, "----------") ;

	    }

	    pages[nextpage] = offset ;

	} else {

	    mvwprintw(dsp->w_root,dsp->pl_bottom, COL_MORELINES, 
		"----------") ;

	    pages[nextpage] = -1 ;
	    nextpage = 0 ;
	}

	bclose(hfp) ;

	ds_refresh(dsp,(DSM_VIEWER | DSM_BACKGROUND)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("help: ret nextpage=%u\n",nextpage) ;
#endif

	return nextpage ;
}
/* end subroutine (help) */



