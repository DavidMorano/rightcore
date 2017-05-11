/* userports */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	USERPORTS_INCLUDE
#define	USERPORTS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>

#include	<vsystem.h>		/* for 'uino_t' */
#include	<vecobj.h>
#include	<vecpstr.h>
#include	<localmisc.h>


#define	USERPORTS	struct userports_head
#define	USERPORTS_CUR	struct userports_cur
#define	USERPORTS_ENT	struct userports_ent

#define	USERPORTS_MAGIC	0x87437174
#define	USERPORTS_FNAME	"/etc/userports"

struct userports_ent {
	uid_t		uid ;
	const char	*protocol ;
	const char	*portname ;
} ;

struct userports_cur {
	int		i ;
} ;

struct userports_flags {
	uint		eof:1 ;
	uint		sorted:1 ;
} ;

struct userports_file {
	uino_t		ino ;
	dev_t		dev ;
	time_t		mtime ;
} ;

struct userports_head {
	uint		magic ;
	vecobj		ents ;
	vecpstr		protos ;
	vecpstr		ports ;
	struct userports_flags	f ;
	struct userports_file	fi ;
	const char	*fname ;
} ;


#if	(! defined(USERPORTS_MASTER)) || (USERPORTS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int userports_open(USERPORTS *,const char *) ;
extern int userports_query(USERPORTS *,uid_t,const char *,int) ;
extern int userports_curbegin(USERPORTS *,USERPORTS_CUR *) ;
extern int userports_enum(USERPORTS *,USERPORTS_CUR *,USERPORTS_ENT *) ;
extern int userports_fetch(USERPORTS *,USERPORTS_CUR *,uid_t,USERPORTS_ENT *) ;
extern int userports_curend(USERPORTS *,USERPORTS_CUR *) ;
extern int userports_close(USERPORTS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USERPORTS_MASTER */

#endif /* USERPORTS_INCLUDE */


