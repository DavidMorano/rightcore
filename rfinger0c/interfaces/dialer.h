/* dialer */


/* revision history:

	- 2003-02-01, Dave Morano

	This subroutine was adopted for use from the DWD program.


*/


#ifndef	DIALER_INCLUDE
#define	DIALER_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecobj.h>
#include	<vecstr.h>

#include	"localmisc.h"



#define	DIALER		struct dialer_head
#define	DIALER_ENTRY	struct dialer_entry
#define	DIALER_ARGS	struct dialer_args
#define	DIALER_INFO	struct dialer_i
#define	DIALER_CALLS	struct dialer_calls

#define	INTERFACE	struct interface

/* dialer flags */

#define	DIALER_MFULL		0x0001
#define	DIALER_MHALFOUT		0x0002
#define	DIALER_MHALFIN		0x0004
#define	DIALER_MCOR		0x0008
#define	DIALER_MCO		0x0010
#define	DIALER_MCL		0x0020
#define	DIALER_MARGS		0x0040

#define	DIALER_MRDONLY		DIALER_MHALFIN
#define	DIALER_MWRONLY		DIALER_MHALFOUT



struct dialer_args {
	const char	*pr ;		/* program root */
	const char	*prn ;		/* program root-name */
	const char	*ip ;		/* interface */
	const char	**argv ;
	const char	**envv ;
	int		timeout ;
	int		options ;
} ;

struct dialer_flags {
	uint		vsprs : 1 ;
	uint		vsdirs : 1 ;
} ;

struct dialer_prcache {
	char		*domainname ;
	char		**prs ;
} ;

struct dialer_head {
	unsigned long	magic ;
	char		*pr ;
	const char	**dirs ;
	struct dialer_flags	f ;
	struct dialer_prcache	pc ;
	vecobj		names ;		/* dialer-module names */
	vecstr		prlist ;
	vecstr		dirlist ;
} ;

struct dialer_calls {
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

struct dialer_module {
	void		*dhp ;		/* SO-dlopen handle */
	int		dev, ino ;
	int		count ;
} ;

struct dialer_entry {
	char		*name ;
	char		*itype ;
	struct dialer_module	*mp ;
	struct dialer_calls	c ;
	int		size ;		/* object size */
	int		flags ;
	int		count ;
} ;

struct dialer_i {
	const char	*name ;
	const char	*version ;
	const char	*itype ;
	int		size ;
	int		flags ;
} ;

struct interface {
	char		*name ;
	int		size ;
} ;



#if	(! defined(DIALER_MASTER)) || (DIALER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dialer_init(DIALER *,const char *,const char **,const char **) ;
extern int dialer_loadin(DIALER *,const char *,DIALER_ENTRY **) ;
extern int dialer_loadout(DIALER *,const char *) ;
extern int dialer_check(DIALER *,time_t) ;
extern int dialer_free(DIALER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DIALER_MASTER */


#endif /* DIALER_INCLUDE */



