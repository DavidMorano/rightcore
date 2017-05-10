/* rectab */



#ifndef	RECTAB_INCLUDE
#define	RECTAB_INCLUDE	1


#include	<sys/types.h>

#include	<vecelem.h>



/* object defines */

#define	RECTAB			VECELEM
#define	RECTAB_ENT		struct rectab_ent



#define	RECTAB_STARTLEN	100	/* starting rectab length */



struct rectab_ent {
	uint	ia ;		/* Internet address */
	uint	csi ;		/* canonical string index */
	uint	si ;		/* string index */
} ;



#if	(! defined(RECTAB_MASTER)) || (RECTAB_MASTER == 0)

extern int	rectab_init(RECTAB *,int) ;
extern int	rectab_free(RECTAB *) ;
extern int	rectab_add(RECTAB *,uint,uint,uint) ;
extern int	rectab_gettab(RECTAB *,char **) ;
extern int	rectab_count(RECTAB *) ;
extern int	rectab_indexlen(RECTAB *) ;
extern int	rectab_mkindex(RECTAB *,char *,int) ;

#endif /* RECTAB_MASTER */


#endif /* RECTAB_INCLUDE */



