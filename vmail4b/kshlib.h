/* kshlib */
/* lang=C89 */

/* library initialization for KSH built-in command libraries */
/* last modified %G% version %I% */


/* revision history:

	= 2001-11-01, David A­D­ Morano
        Written to have a place for the various KSH initialization subroutines.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	KSHLIB_INCLUDE
#define	KSHLIB_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sesmsg.h>


#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


#define	KSHLIB_NENTS		30
#define	KSHLIB_INTPOLL		5
#define	KSHLIB_INTSESCHECK	(1*3600)
#define	KSHLIB_NOTE		struct kshlib_note
#define	KSHLIB_SESHOUR		18 /* 6:00pm */
#define	KSHLIB_SESDNAME		"/var/tmp/sessions"

#define	KSHLIB_RMKSH	(1<<0)		/* run-mode KSH */
#define	KSHLIB_RMMAIN	(1<<1)		/* run-mode MAIN */

enum kshlibcmds {
	kshlibcmd_noteoff,
	kshlibcmd_noteon,
	kshlibcmd_notecount,
	kshlibcmd_notestate,
	kshlibcmd_overlast
} ;

struct kshlib_note {
	time_t		stime ;
	uint		type ;
	int		nlen ;
	char		nbuf[SESMSG_NBUFLEN+1] ;
	char		user[SESMSG_USERLEN+1] ;
} ;


#if	(! defined(KSHLIB_MASTER)) || (KSHLIB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern void	lib_init(int,void *) ;
extern void	lib_fini(void) ;

extern int	lib_initenviron(void *) ;
extern int	lib_initmemalloc(int) ;
extern int	lib_mainbegin(cchar **,const int *) ;
extern int	lib_mainend(void) ;
extern int	lib_kshbegin(void *,const int *) ;
extern int	lib_kshend(void) ;
extern int	lib_runmode(void) ;
extern int	lib_serial(void) ;

extern int	lib_sigreset(int) ;
extern int	lib_sigquit(void) ;
extern int	lib_sigintr(void) ;
extern int	lib_sigterm(void) ;
extern int	lib_issig(int) ;

extern int	lib_proghave(cchar *) ;
extern int	lib_progaddr(cchar *,void *) ;
extern int	lib_progcall(cchar *,int,cchar **,cchar **,void *) ;
extern int	lib_progcalla(const void *,int,cchar **,cchar **,void *) ;

extern int	lib_callcmd(cchar *,int,cchar **,cchar **,void *) ;
extern int	lib_callfunc(const void *,int,cchar **,cchar **,void *) ;

extern int	lib_noteadm(int,...) ;
extern int	lib_noteread(KSHLIB_NOTE *,int) ;
extern int	lib_notedel(int) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(KSHLIB_MASTER)) || (KSHLIB_MASTER == 0) */

#endif /* KSHLIB_INCLUDE */


