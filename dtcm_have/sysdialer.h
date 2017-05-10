/* sysdialer */


/* revision history:

	= 2003-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSDIALER_INCLUDE
#define	SYSDIALER_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>		/* for 'uino_t' */
#include	<vecobj.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	SYSDIALER_MAGIC	31815927
#define	SYSDIALER	struct sysdialer_head
#define	SYSDIALER_ENT	struct sysdialer_ent
#define	SYSDIALER_CALLS	struct sysdialer_calls
#define	SYSDIALER_INFO	struct sysdialer_i
#define	SYSDIALER_ARGS	struct sysdialer_args
#define	SYSDIALER_FL	struct sysdialer_flags
#define	SYSDIALER_PRC	struct sysdialer_prcache
#define	SYSDIALER_MOD	struct sysdialer_module
#define	SYSDIALER_LF	"sysdialer"

#ifdef	COMMENT
#define	INTERFACE	struct interface
#endif

/* sysdialer flags */

#define	SYSDIALER_MFULL		0x0001
#define	SYSDIALER_MHALFOUT	0x0002
#define	SYSDIALER_MHALFIN	0x0004
#define	SYSDIALER_MCOR		0x0008
#define	SYSDIALER_MCO		0x0010
#define	SYSDIALER_MCL		0x0020
#define	SYSDIALER_MARGS		0x0040

#define	SYSDIALER_MRDONLY	SYSDIALER_MHALFIN
#define	SYSDIALER_MWRONLY	SYSDIALER_MHALFOUT


struct sysdialer_i {
	const char	*name ;
	const char	*version ;
	const char	*itype ;
	int		size ;
	int		flags ;
} ;

struct sysdialer_flags {
	uint		vsprs:1 ;
	uint		vsdirs:1 ;
} ;

struct sysdialer_prcache {
	const char	*domainname ;
	const char	**prs ;
} ;

struct sysdialer_head {
	uint		magic ;
	const char	*pr ;
	const char	**dirs ;
	SYSDIALER_FL	f ;
	SYSDIALER_PRC	pc ;
	vecobj		entries ;	/* sysdialer-module names */
	vecstr		prlist ;
	vecstr		dirlist ;
	time_t		ti_lastcheck ;
} ;

struct sysdialer_calls {
	int		(*open)() ;
	int		(*reade)() ;
	int		(*recve)() ;
	int		(*recvfrome)() ;
	int		(*recvmsge)() ;
	int		(*write)() ;
	int		(*send)() ;
	int		(*sendto)() ;
	int		(*sendmsg)() ;
	int		(*shutdown)() ;
	int		(*close)() ;
} ;

struct sysdialer_module {
	void		*dhp ;		/* SO-dlopen handle */
	dev_t		dev ;
	uino_t		ino ;
	int		count ;
} ;

struct sysdialer_ent {
	const char	*name ;
	const char	*itype ;
	SYSDIALER_MOD	*mp ;
	SYSDIALER_CALLS	c ;
	int		size ;		/* object size */
	int		flags ;
	int		count ;
} ;

struct sysdialer_args {
	const char	*pr ;		/* program root */
	const char	*prn ;		/* program root-name */
	const char	*ip ;		/* interface */
	const char	**argv ;
	const char	**envv ;
	int		timeout ;
	int		options ;
} ;

#ifdef	COMMENT
struct interface {
	char		*name ;
	int		size ;
} ;
#endif /* COMMENT */


#if	(! defined(SYSDIALER_MASTER)) || (SYSDIALER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysdialer_start(SYSDIALER *,const char *,cchar **,cchar **) ;
extern int sysdialer_loadin(SYSDIALER *,const char *,SYSDIALER_ENT **) ;
extern int sysdialer_loadout(SYSDIALER *,const char *) ;
extern int sysdialer_check(SYSDIALER *,time_t) ;
extern int sysdialer_finish(SYSDIALER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSDIALER_MASTER */

#endif /* SYSDIALER_INCLUDE */


