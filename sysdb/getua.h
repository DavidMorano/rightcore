/* getua */

/* user-attribute database access */


#ifndef	GETUA_INCLUDE
#define	GETUA_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<user_attr.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int getua_begin() ;
extern int getua_ent(userattr_t *,char *,int) ;
extern int getua_end() ;
extern int getua_name(userattr_t *,char *,int,const char *) ;
extern int getua_uid(userattr_t *,char *,int,uid_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETUA_INCLUDE */


