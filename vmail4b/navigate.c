/* navigate */

/* navigate among the mail message in the current mailbox */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Move the global curr.msgno message pointer to the previous or next
        message ; if none, leave pointer at the limit. Some useful information:

	curr.msgno	holds the  number of current message [1 to 'total']


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<curses.h>
#include	<stdio.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ds.h>


/* external subroutines */


/* external variables */

extern struct mailbox	mb ;


/* exported subroutines */


int navigate(pip,dsp,mbp,inc)
struct proginfo	*pip ;
DS		*dsp ;
struct mailbox	*mbp ;
int	inc ;
{
	int	f_scroll = FALSE ;
	int	nshown ;
	int	nscroll = 0 ;
	int	m_first, m_last, m_farlast, m_farfirst ;
	int	new, new_first, new_last ;
	int	new_cur ;


	new = mbp->current + inc ;
	if (new < 0) new = 0 ;

	if (new > (mbp->total - 1)) 
		new = (mbp->total - 1) ;

	curr.pageno = 0 ;
	nshown = dsp->nh ;

	m_first = mbp->shown ;
	m_farfirst = m_first - nshown ;
	if (m_farfirst < 0) 
		m_farfirst = 0 ;

	m_last = mbp->shown + nshown - 1 ;

#if	CF_DEBUG
	if (pip->debuglevel > 0) {
	    debugprintf("navigate: nshown current=%d new=%d t=%d margin=%d\n",
	        mbp->current,new,mbp->total,dsp->margin) ;
	    debugprintf("navigate: nshown current=%d s=%d l=%d f=%d\n",
	        mbp->current,nshown,
		m_last,mbp->shown) ;
	}
#endif

	m_farlast = m_last + nshown ;
	if (m_farlast > (mbp->total - 1)) 
		m_farlast = (mbp->total - 1) ;

/* handle the cases */
/* determine the number of lines to scroll the message display */

	if (new < (m_first + dsp->margin)) {

	    new_first = new - dsp->margin ;
	    if (new_first < 0) 
			new_first = 0 ;

	    nscroll = new_first - m_first ;
	    new_cur = new - new_first ;
	    ds_scan(dsp,&mb,new_first,nshown,nscroll,
	        dsp->hl_cur,new_cur) ;

	} else if (new > (m_last - dsp->margin)) {

	    new_last = new + dsp->margin ;
	    if (new_last > (mbp->total - 1)) 
			new_last = (mbp->total - 1) ;

	    new_first = new_last + 1 - nshown ;
	    if (new_first < 0) 
		new_first = 0 ;

	    nscroll = new_first - m_first ;
	    new_cur = new - new_first ;

#if	CF_DEBUG
	if (pip->debuglevel > 0)
debugprintf("navigate: last nf=%d, nl=%d, ns=%d, nc=%d\n",
	new_first,new_last,nscroll,new_cur) ;
#endif

	    ds_scan(dsp,&mb,new_first,nshown,nscroll,
	        dsp->hl_cur,new_cur) ;

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	debugprintf("navigate: back from scan\n") ;
#endif

	} else {

	    new_first = m_first ;
	    new_cur = new - new_first ;

#if	CF_DEBUG
	if (pip->debuglevel > 0)
debugprintf("middle nf=%d, ns=%d, nc=%d\n",
	new_first,nscroll,new_cur) ;
#endif

	    ds_scan(dsp,&mb,mbp->shown,nshown,0,
	        dsp->hl_cur,new_cur) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	debugprintf("navigate: out of 'if', about to update everything\n") ;
#endif

/* update everything */

	dsp->hl_cur = new_cur ;
	mbp->shown = new_first ;
	mbp->current = new ;
	curr.msgno = mbp->current ;

	cursline = dsp->hl_cur ;
	pages[0] = -1 ;
	curr.pages = 0 ;

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	debugprintf("navigate: exiting nscroll=%d\n",nscroll) ;
#endif

	return nscroll ;
}
/* end subroutine (navigate) */



