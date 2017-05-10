/* kbdinfo */

/* keyboard-information database access */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	KBDINFO_INCLUDE
#define	KBDINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"keysymer.h"
#include	"termcmd.h"


#define	KBDINFO_MAGIC		0x24182138
#define	KBDINFO			struct kbdinfo_head
#define	KBDINFO_KE		struct kbdinfo_e
#define	KBDINFO_CUR		struct kbdinfo_c

/* types */

#define	KBDINFO_TREG		0
#define	KBDINFO_TESC		1
#define	KBDINFO_TCSI		2
#define	KBDINFO_TDCS		3
#define	KBDINFO_TPF		4
#define	KBDINFO_TFKEY		5
#define	KBDINFO_TOVERLAST	6


struct kbdinfo_flags {
	uint		dummy:1 ;	/* new mail arrived */
} ;

struct kbdinfo_e {
	const char	*a ;		/* the memory allocation */
	const char	*keyname ;	/* keysym */
	const char	*istr ;
	const char	*dstr ;
	short		*p ;		/* parameters */
	int		type ;		/* key type */
	int		name ;		/* key name */
	int		keynum ;	/* key number */
	int		nparams ;	/* number of paramters */
} ;

struct kbdinfo_c {
	int		i, j ;
} ;

struct kbdinfo_head {
	uint		magic ;
	vecobj		types[KBDINFO_TOVERLAST] ;
	struct kbdinfo_flags	f ;
	KEYSYMER	*ksp ;
} ;


#if	(! defined(KBDINFO_MASTER)) || (KBDINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int kbdinfo_open(KBDINFO *,KEYSYMER *,const char *) ;
extern int kbdinfo_count(KBDINFO *) ;
extern int kbdinfo_lookup(KBDINFO *,char *,int,TERMCMD *) ;
extern int kbdinfo_curbegin(KBDINFO *,KBDINFO_CUR *) ;
extern int kbdinfo_curend(KBDINFO *,KBDINFO_CUR *) ;
extern int kbdinfo_enum(KBDINFO *,KBDINFO_CUR *,KBDINFO_KE **) ;
extern int kbdinfo_close(KBDINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* KBDINFO_MASTER */

#endif /* KBDINFO_INCLUDE */


