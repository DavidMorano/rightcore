/* sortlist */

/* sorted list structures (Sorted List) */


#ifndef	SORTLIST_INCLUDE
#define	SORTLIST_INCLUDE	1


/* object defines */

#define	SORTLIST	struct sortlist_head
#define	SORTLIST_ENT	struct sortlist_ent
#define	SORTLIST_MAGIC	0x31415926


struct sortllist_ent {
	SORTLIST_ENT	*left, *right ;
	int		balance ;
	void		*keyp ;
	void		*valuep ;
} ;

struct sortlist_head {
	int		magic ;
	int		(*cmpfunc)(void *,void *) ;
	SORTLIST_ENT	*root ;
	int		balance ;
} ;


typedef struct sortlist_head	sortlist ;


#ifndef	SORTLIST_MASTER

extern int sortlist_init(sortlist *,int (*)()) ;
extern int sortlist_add(sortlist *,void *,void *,int) ;
extern int sortlist_free(sortlist *) ;
extern int sortlist_get(sortlist *,void *,int,void **) ;
extern int sortlist_del(sortlist *,void *) ;
extern int sortlist_count(sortlist *) ;
extern int sortlist_search(sortlist *,void *,int (*)(),void *) ;

#endif /* SORTLIST_MASTER */

#endif /* SORTLIST_INCLUDE */


