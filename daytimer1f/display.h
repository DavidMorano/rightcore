/* display */


#ifndef	DISPLAY_INCLUDE
#define	DISPLAY_INCLUDE		1


#include	<sys/types.h>

#include	<time.h>

#include	"misc.h"		/* for type 'uint' */



/* local defines */

#define	DISPLAY		struct display

#define	DISPLAY_STRLEN	100




struct display_flags {
	uint	status : 1 ;		/* where on the display ? */
	uint	mail : 1 ;		/* there was mail last time */
	uint	blank : 1 ;
} ;

struct display {
	unsigned long		magic ;
	struct display_flags	f ;
	time_t	t_full ;
	time_t	t_refresh ;
	time_t	t_last ;
	time_t	t_offset ;
	int	fd ;
	char	last_str[DISPLAY_STRLEN + 1] ;	/* last string written */
} ;


#if	(! defined(DISPLAY_MASTER)) || (DISPLAY_MASTER == 0)

extern int display_init(DISPLAY *,int,int,time_t,time_t) ;
extern int display_free(DISPLAY *) ;
extern int display_show(DISPLAY *,time_t,int) ;
extern int display_blank(DISPLAY *) ;
extern int display_check(DISPLAY *,time_t) ;

#endif /* DISPLAY_MASTER */


#endif /* DISPLAY_INCLUDE */



