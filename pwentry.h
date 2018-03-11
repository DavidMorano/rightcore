/* pwentry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PWENTRY_INCLUDE
#define	PWENTRY_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<localmisc.h>		/* unsigned types */


#define	PWENTRY		struct pwentry

#ifdef	_SC_GETPW_R_SIZE_MAX
#define	PWENTRY_BUFLEN	(_SC_GETPW_R_SIZE_MAX + MAXNAMELEN)
#else
#define	PWENTRY_BUFLEN	(MAXPATHLEN + MAXNAMELEN)
#endif


struct pwentry {
	const char	*username ;
	const char	*password ;
	const char	*gecos ;
	const char	*dir ;
	const char	*shell ;
	const char	*organization ;
	const char	*realname ;
	const char	*account ;
	const char	*bin ;
	const char	*name_f ;
	const char	*name_m1 ;
	const char	*name_m2 ;
	const char	*name_l ;
	const char	*office ;
	const char	*wphone ;
	const char	*hphone ;
	const char	*printer ;
	long		lstchg ;	/* password lastchanged date */
	long		min ;		/* min days between password changes */
	long		max ;		/* max days password is valid */
	long		warn ;		/* days to warn user to change passwd */
	long		inact ;		/* days the login may be inactive */
	long		expire ;	/* login expiration date */
	uint		flag ;		/* currently not being used */
	uid_t		uid ;
	gid_t		gid ;
} ;


#if	(! defined(PWENTRY_MASTER)) || (PWENTRY_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	pwentry_bufsize() ;

#ifdef	__cplusplus
}
#endif

#endif /* PWENTRY_MASTER */

#endif /* PWENTRY_INCLUDE */


