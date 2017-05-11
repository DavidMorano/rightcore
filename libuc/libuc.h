/* libuc */


#ifndef	LIBUC_INCLUDE
#define	LIBUC_INCLUDE	1


#ifndef	UCMALLOC_INCLUDE
#define	UCMALLOC_INCLUDE	1

#ifdef	__cplusplus
extern "C" {
#endif

extern int uc_malloc(int,void *) ;
extern int uc_mallocstrw(const char *,int,const char **) ;
extern int uc_mallocbuf(const void *,int,const void **) ;

#ifdef	__cplusplus
}
#endif

#endif /* UCMALLOC_INCLUDE */


#endif /* LIBUC_INCLUDE */



