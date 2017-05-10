/* display */


#ifndef	DISPLAY_INCLUDE
#define	DISPLAY_INCLUDE		1


#include	<sys/types.h>
#include	<time.h>
#include	<localmisc.h>		/* for type 'uint' */


/* local defines */

#define	DISPLAY		struct display
#define	DISPLAY_FL	struct display_flags
#define	DISPLAY_STRLEN	100


struct display_flags {
	uint	status : 1 ;		/* where on the display ? */
	uint	mail : 1 ;		/* there was mail last time */
	uint	blank : 1 ;
} ;

struct display {
	uint		magic ;
	DISPLAY_FL	f ;
	time_t		t_full ;
	time_t		t_refresh ;
	time_t		t_last ;
	time_t		t_offset ;
	int		fd ;
	char		last_str[DISPLAY_STRLEN + 1] ;
} ;


#if	(! defined(DISPLAY_MASTER)) || (DISPLAY_MASTER == 0)

extern int display_start(DISPLAY *,int,int,time_t,time_t) ;
extern int display_finish(DISPLAY *) ;
extern int display_show(DISPLAY *,time_t,int) ;
extern int display_blank(DISPLAY *) ;
extern int display_check(DISPLAY *,time_t) ;

#endif /* DISPLAY_MASTER */

#endif /* DISPLAY_INCLUDE */


