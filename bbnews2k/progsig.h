/* progsig */

/* program signal handling */
/* last modified %G% version %I% */


/* revision history:

	= 2001-11-01, David A­D­ Morano
	Originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	PROGSIG_INCLUDE
#define	PROGSIG_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>


#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


#define	PROGSIG_NENTS	30
#define	PROGSIG_NOTE	struct progsig_note

#define	PROGSIG_RMKSH	(1<<0)
#define	PROGSIG_RMMAIN	(1<<1)

#define	PROGSIG_NBUFLEN	100
#define	PROGSIG_USERLEN	100


struct progsig_note {
	time_t		stime ;
	int		type ;
	int		dlen ;
	char		dbuf[PROGSIG_NBUFLEN+1] ;
	char		user[PROGSIG_USERLEN+1] ;
} ;


#if	(! defined(PROGSIG_MASTER)) || (PROGSIG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	progsig_init(void) ;
extern void	progsig_fini(void) ;

extern int	progsig_mainbegin(const char **) ;
extern int	progsig_mainend(void) ;

extern int	progsig_runmode(void) ;
extern int	progsig_serial(void) ;

extern int	progsig_adm(int) ;

extern int	progsig_sigquit(void) ;
extern int	progsig_sigterm(void) ;
extern int	progsig_sigintr(void) ;
extern int	progsig_issig(int) ;

extern int	progsig_noteread(PROGSIG_NOTE *,int) ;
extern int	progsig_notedel(int) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(PROGSIG_MASTER)) || (PROGSIG_MASTER == 0) */

#endif /* PROGSIG_INCLUDE */


