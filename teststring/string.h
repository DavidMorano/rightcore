/* dstr */


#ifndef	DSTR_INCLUDE
#define	DSTR_INCLUDE	1


/* object defines */

#define	DSTR		struct dstr_head


struct dstr_head {
	char	*s ;
	int	slen ;
} ;



#if	(! defined(DSTR_MASTER)) || (DSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dstr_start(DSTR *,char *,int) ;
extern int dstr_reinit(DSTR *,char *,int) ;
extern int dstr_assign(DSTR *,DSTR *) ;
extern int dstr_finish(DSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DSTR_MASTER */


#endif /* DSTR_INCLUDE */



