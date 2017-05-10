/* pwfile */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PWFILE_INCLUDE
#define	PWFILE_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecitem.h>
#include	<hdb.h>
#include	<pwentry.h>


#ifndef	UINT
#define	UINT	unsigned int
#endif

/* object defines */

#define	PWFILE_MAGIC	0x98127643
#define	PWFILE		struct pwfile_head
#define	PWFILE_CUR	struct pwfile_c
#define	PWFILE_FL	struct pwfile_flags
#define	PWFILE_ENT	PWENTRY

#define	PWFILE_RECLEN	PWENTRY_BUFLEN
#define	PWFILE_ENTLEN	PWENTRY_BUFLEN

/* are these even needed? */

#define	PWFILE_NAMELEN	32		/* max username length */
#define	PWFILE_PASSLEN	32		/* max normal password length */
#define	PWFILE_COMLEN	128		/* max comment field length */


/* throw a bone or something to the entry structure */

struct pwfile_c {
	hdb_cur		hc ;
	int		i ;
} ;

struct pwfile_flags {
	UINT		locked:1 ;	/* locked */
	UINT		locked_cur:1 ;	/* locked for cursor operations */
	UINT		locked_explicit:1 ;	/* locked explicitly by user */
} ;

struct pwfile_head {
	UINT		magic ;
	const char	*fname ;
	PWFILE_FL	f ;
	vecitem		alist ;
	hdb		byuser ;
	hdb		byuid ;
	hdb		bylastname ;
	time_t		readtime ;
	int		lfd ;
} ;


#if	(! defined(PWFILE_MASTER)) || (PWFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pwfile_open(PWFILE *,const char *) ;
extern int pwfile_curbegin(PWFILE *,PWFILE_CUR *) ;
extern int pwfile_curend(PWFILE *,PWFILE_CUR *) ;
extern int pwfile_enum(PWFILE *,PWFILE_CUR *,PWENTRY *,char *,int) ;
extern int pwfile_fetchuser(PWFILE *,cchar *,PWFILE_CUR *,
		PWENTRY *,char *,int) ;
extern int pwfile_lock(PWFILE *,int,int) ;
extern int pwfile_close(PWFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PWFILE_MASTER */

#endif /* PWFILE_INCLUDE */


