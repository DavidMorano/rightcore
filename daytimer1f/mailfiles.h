/* mailfiles */



#ifndef	MAILFILES_INCLUDE
#define	MAILFILES_INCLUDE	1



#include	<sys/types.h>

#include	<vecelem.h>



/* object defines */

#define	MAILFILES		VECELEM
#define	MAILFILES_ENT		struct mailfiles_ent




struct mailfiles_ent {
	char	*mailfile ;
	time_t	lasttime ;
	offset_t	lastsize ;
	int	f_changed ;
} ;




#if	(! defined(MAILFILES_MASTER)) || (MAILFILES_MASTER == 0)

extern int mailfiles_init(MAILFILES *) ;
extern int mailfiles_free(MAILFILES *) ;
extern int mailfiles_add(MAILFILES *,char *,int) ;
extern int mailfiles_get(MAILFILES *,int,MAILFILES_ENT **) ;
extern int mailfiles_check(MAILFILES *) ;
extern int mailfiles_count(MAILFILES *) ;

#endif /* MAILFILES_MASTER */


#endif /* MAILFILES_INCLUDE */



