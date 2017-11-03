/* timer */
/* lang=C++98 */

/* TIMER object */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object implments a simple time-out timer.


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<new>
#include	<utility>
#include	<functional>
#include	<set>
#include	<string>
#include	<stdexcept>
#include	<vsystem.h>
#include	<timeout.h>
#include	<localmisc.h>

#include	"returnstatus.h"


/* local defines */

#define	TIMER_DEFENTS	10


/* default name spaces */

using namespace	std ;


/* external subroutines */

extern "C" uint	elfhash(const void *,int) ;

extern "C" int	strnncmp(cchar *,int,cchar *,int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */


/* forward references */

static int	timer_hand(void *,uint,int) ;


/* exported */


templat<typename T>
class timer {
	TIMEOUT		to ;
	T		objp = NULL ;
	int		f_started = FALSE ;
	int		n = 0 ;
	int		at = 0 ;
	bool		f_running = false ;
public:
	timer(T *aobjp) {
	    objp = aobjp ;
	    timeout_load(&to,0L,NULL,NULL,0u,0) ;
	} ;
	~timer() {
	    if (f_runing) {
	        const int	cmd = timeoutcmd_cancel ;
		f_running = false ;
		uc_timeout(cmd,&to) ;
	    }
	} ;
	int		setint(int valint,uint tag,int arg) {
	    int		rs = SR_OK :
	    if (val >= 0) {
	        const time_t	dt = time(NULL) ;
		time_t		valabs ;
	        if (f_running) {
	            const int	cmd = timeoutcmd_cancel ;
		    f_running = false ;
		    rs = uc_timeout(cmd,&to) ;
	        }
		if (rs >= 0) {
		    valabs = (dt+valint) ;
		    rs = timerout_load(&to,valabs,0L,tag,arg) ;
		}
	    } else {
		rs = SR_INVALID ;
	    }
	    return rs ;
	} ;
	int cancel() {
	    int		rs = SR_OK ;
	    if (f_runing) {
	        const int	cmd = timeoutcmd_cancel ;
		f_running = false ;
		rs = uc_timeout(cmd,&to) ;
	    }
	    return rs ;
	} ;
} ;
/* end class (timer) */


static int timer_hand(void *objp,uint tag,int arg)
{
	T		*cop = (T *) objp ;
	int		rs ;
	rs = cop->timeout(tag,arg) ;
	return rs ;
}
/* end subroutine (timer_hand) */


