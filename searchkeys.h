/* searchkeys */


/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

#ifndef	SEARCHKEYS_INCLUDE
#define	SEARCHKEYS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<strpack.h>
#include	<localmisc.h>

#include	"xwords.h"


#define	SEARCHKEYS_MAGIC	0x00885543
#define	SEARCHKEYS		struct searchkeys_head
#define	SEARCHKEYS_POP		struct searchkeys_pop
#define	SEARCHKEYS_CUR		struct searchkeys_c
#define	SEARCHKEYS_KW		struct searchkeys_kword
#define	SEARCHKEYS_PH		struct searchkeys_kphrase


struct searchkeys_c {
	int		i ;
	int		j ;
} ;

struct searchkeys_kword {
	const char	*kp ;
	char		kl ;
} ;

struct searchkeys_kphrase {
	SEARCHKEYS_KW	*kwords ;
	int		nwords ;
} ;

struct searchkeys_pop {
	uint		magic ;
	int		*nmatch ;
	int		cphrases ;
	int		f_prefix ;
} ;

struct searchkeys_head {
	uint		magic ;
	STRPACK		stores ;
	SEARCHKEYS_PH	*kphrases ;
	int		nphrases ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	searchkeys_start(SEARCHKEYS *,const char **) ;
extern int	searchkeys_popbegin(SEARCHKEYS *,SEARCHKEYS_POP *,int) ;
extern int	searchkeys_process(SEARCHKEYS *,SEARCHKEYS_POP *,cchar *,int) ;
extern int	searchkeys_processxw(SEARCHKEYS *,SEARCHKEYS_POP *,XWORDS *) ;
extern int	searchkeys_popend(SEARCHKEYS *,SEARCHKEYS_POP *) ;
extern int	searchkeys_curbegin(SEARCHKEYS *,SEARCHKEYS_CUR *) ;
extern int	searchkeys_enum(SEARCHKEYS *,SEARCHKEYS_CUR *,cchar **) ;
extern int	searchkeys_curend(SEARCHKEYS *,SEARCHKEYS_CUR *) ;
extern int	searchkeys_finish(SEARCHKEYS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SEARCHKEYS_INCLUDE */


