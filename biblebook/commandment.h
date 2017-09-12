/* commandment */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	COMMANDMENT_INCLUDE
#define	COMMANDMENT_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<modload.h>
#include	<localmisc.h>

#include	<commandments.h>


#define	COMMANDMENT_MAGIC	0x99447242
#define	COMMANDMENT		struct commandment_head
#define	COMMANDMENT_CUR		struct commandment_c
#define	COMMANDMENT_CA		struct commandment_calls


struct commandment_c {
	uint	magic ;
	void	*scp ;
} ;

struct commandment_calls {
	int	(*open)(void *,const char *,const char *) ;
	int	(*audit)(void *) ;
	int	(*count)(void *) ;
	int	(*max)(void *) ;
	int	(*read)(void *,char *,int,uint) ;
	int	(*get)(void *,int,char *,int) ;
	int	(*curbegin)(void *,void *) ;
	int	(*curend)(void *,void *) ;
	int	(*enumerate)(void *,void *,void *,char *,int) ;
	int	(*close)(void *) ;
} ;

struct commandment_head {
	uint		magic ;
	MODLOAD		loader ;
	COMMANDMENT_CA	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;
	int		cursize ;
} ;


#if	(! defined(COMMANDMENT_MASTER)) || (COMMANDMENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	commandment_open(COMMANDMENT *,cchar *,cchar *) ;
extern int	commandment_audit(COMMANDMENT *) ;
extern int	commandment_count(COMMANDMENT *) ;
extern int	commandment_max(COMMANDMENT *) ;
extern int	commandment_read(COMMANDMENT *,char *,int,int) ;
extern int	commandment_get(COMMANDMENT *,int,char *,int) ;
extern int	commandment_curbegin(COMMANDMENT *,COMMANDMENT_CUR *) ;
extern int	commandment_curend(COMMANDMENT *,COMMANDMENT_CUR *) ;
extern int	commandment_enum(COMMANDMENT *,COMMANDMENT_CUR *,
			uint *,char *,int) ;
extern int	commandment_close(COMMANDMENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* COMMANDMENT_MASTER */

#endif /* COMMANDMENT_INCLUDE */


