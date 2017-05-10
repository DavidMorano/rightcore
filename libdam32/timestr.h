/* timestr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TIMESTR_INCLUDE
#define	TIMESTR_INCLUDE	1


#if	(! defined(TIMESTR_MASTER)) || (TIMESTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern char *timestr_date(time_t,int,char *) ;
extern char *timestr_std(time_t,char *) ;
extern char *timestr_edate(time_t,char *) ;
extern char *timestr_gmtstd(time_t,char *) ;
extern char *timestr_msg(time_t,char *) ;
extern char *timestr_hdate(time_t,char *) ;
extern char *timestr_log(time_t,char *) ;
extern char *timestr_gmlog(time_t,char *) ;
extern char *timestr_logz(time_t,char *) ;
extern char *timestr_gmlogz(time_t,char *) ;
extern char *timestr_elapsed(time_t,char *) ;
extern char *timestr_gmlog(time_t,char *) ;
extern char *timestr_scandate(time_t,char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(TIMESTR_MASTER)) || (TIMESTR_MASTER == 0) */

#endif /* TIMESTR_INCLUDE */


