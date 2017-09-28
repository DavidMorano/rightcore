/* tailemod */


#ifndef	TAILEMOD_INCLUDE
#define	TAILEMOD_INCLUDE	1


#define	TAILEMOD_MAGIC	31815927
#define	TAILEMOD	struct tailemod_head
#define	TAILEMOD_CALLS	struct tailemod_calls
#define	TAILEMOD_ENT	struct tailemod_ent
#define	TAILEMOD_MODULE	struct tailemod_module
#define	TAILEMOD_INFO	struct tailemod_i


#include	<sys/types.h>

#include	<vecobj.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* module flags */

#define	TAILEMOD_MFULL		0x0001
#define	TAILEMOD_MHALFOUT	0x0002
#define	TAILEMOD_MHALFIN	0x0004
#define	TAILEMOD_MCOR		0x0008
#define	TAILEMOD_MCO		0x0010
#define	TAILEMOD_MCL		0x0020
#define	TAILEMOD_MARGS		0x0040


struct tailemod_flags {
	uint		vsprs:1 ;
	uint		vsdirs:1 ;
} ;

struct tailemod_head {
	unsigned long	magic ;
	char		*pr ;
	const char	**dirs ;
	struct tailemod_flags	f ;
	vecobj		modules ;		/* shared objects */
	vecobj		entries ;		/* name entries */
	vecstr		dirlist ;
} ;

struct tailemod_calls {
	int	(*init)() ;
	int	(*store)() ;
	int	(*check)() ;
	int	(*getline)() ;
	int	(*summary)() ;
	int	(*finish)() ;
	int	(*free)() ;
} ;

struct tailemod_module {
	void		*dhp ;
	uino_t		ino ;
	dev_t		dev ;
	int		count ;
} ;

struct tailemod_ent {
	const char	*name ;
	struct tailemod_module	*mp ;
	struct tailemod_calls	c ;
	int		size ;		/* object size */
	int		flags ;
} ;

struct tailemod_i {
	const char	*name ;		/* module name */
	int		size ;		/* module object size */
	int		flags ;		/* module flags */
} ;


#if	(! defined(TAILEMOD_MASTER)) || (TAILEMOD_MASTER == 0)

extern int tailemod_start(TAILEMOD *,const char *,const char **) ;
extern int tailemod_load(TAILEMOD *,const char *) ;
extern int tailemod_unload(TAILEMOD *,const char *) ;
extern int tailemod_store(TAILEMOD *,const char *,int) ;
extern int tailemod_check(TAILEMOD *,time_t) ;
extern int tailemod_getline(TAILEMOD *,char *,int) ;
extern int tailemod_summary(TAILEMOD *,char *,int) ;
extern int tailemod_finish(TAILEMOD *,char *,int) ;
extern int tailemod_finish(TAILEMOD *) ;

#endif /* TAILEMOD_MASTER */


#endif /* TAILEMOD_INCLUDE */



