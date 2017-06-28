/* pinghost */


#ifndef	PINGHOST_INCLUDE
#define	PINGHOST_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>


#define	PINGHOST		struct pinghost


struct pinghost {
	const char	*name ;
	int		intminping ;
	int		to ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int pinghost_start(PINGHOST *,const char *,int,int,int) ;
extern int pinghost_finish(PINGHOST *) ;

#ifdef	__cplusplus
extern "C" {
#endif


#endif /* PINGHOST_INCLUDE */


