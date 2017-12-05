/* acctab */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	ACCTAB_INCLUDE
#define	ACCTAB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vecobj.h>
#include	<vecitem.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	ACCTAB_MAGIC		0x31415926
#define	ACCTAB			struct acctab_head
#define	ACCTAB_ENT		struct acctab_ae
#define	ACCTAB_ERR		struct acctab_errline
#define	ACCTAB_CUR		struct acctab_c


struct acctab_c {
	int		i, j ;
} ;

struct acctab_head {
	uint		magic ;
	time_t		checktime ;
	vecobj		files ;			/* files */
	vecitem		aes_std ;		/* access entries */
	vecitem		aes_rgx ;		/* access entries */
} ;

struct acctab_part {
	const char	*std ;
	const char	*rgx ;
	int		rgxlen ;
	int		type ;
} ;

struct acctab_ae {
	struct acctab_part	netgroup ;
	struct acctab_part	machine ;
	struct acctab_part	username ;
	struct acctab_part	password ;
	int		fi ;		/* file index */
} ;

struct acctab_errline {
	const char	*fname ;
	int		line ;
} ;


typedef struct acctab_head	acctab ;
typedef struct acctab_ae	acctab_ent ;
typedef struct acctab_c		acctab_cur ;
typedef struct acctab_errline	acctab_err ;


#if	(! defined(ACCTAB_MASTER)) || (ACCTAB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int acctab_open(ACCTAB *,cchar *) ;
extern int acctab_fileadd(ACCTAB *,cchar *) ;
extern int acctab_enum(ACCTAB *,ACCTAB_CUR *,ACCTAB_ENT **) ;
extern int acctab_check(ACCTAB *) ;
extern int acctab_curbegin(ACCTAB *,ACCTAB_CUR *) ;
extern int acctab_curend(ACCTAB *,ACCTAB_CUR *) ;
extern int acctab_allowed(ACCTAB *,cchar *,cchar *,cchar *,cchar *) ;
extern int acctab_anyallowed(ACCTAB *,vecstr *,vecstr *,cchar *,cchar *) ;
extern int acctab_close(ACCTAB *) ;

#ifdef	COMMENT
extern int acctab_find(ACCTAB *,const char *,ACCTAB_ENT **) ;
#endif

#ifdef	__cplusplus
}
#endif

#endif /* ACCTAB_MASTER */


#endif /* ACCTAB_INCLUDE */



