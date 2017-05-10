/* mkdirlist */


#ifndef	MKDIRLIST_INCLUDE
#define	MKDIRLIST_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<vsystem.h>
#include	<vechand.h>
#include	<localmisc.h>


#define	MKDIRLIST_MAGIC		0x8987372A
#define	MKDIRLIST		struct mkdirlist_head
#define	MKDIRLIST_ENT		struct mkdirlist_ent
#define	MKDIRLIST_OFL		struct mkdirlist_eflags


struct mkdirlist_head {
	uint		magic ;
	VECHAND		dirs ;
} ;

struct mkdirlist_eflags {
	uint		excl:3 ;	/* ??? */
	uint		new:1 ;		/* new newsgroup */
	uint		options:1 ;	/* directory has options file */
	uint		subscribe:1 ;	/* subscribed in user file? */
	uint		seen:1 ;	/* already written out to user file */
	uint		show:1 ;	/* display to user */
	uint		link:1 ;	/* has been linked to another */
} ;

struct mkdirlist_ent {
	const char	*name ;
	MKDIRLIST_ENT	*link ;
	MKDIRLIST_OFL	f ;
	int		nlen ;
	mode_t		mode ;
	time_t		mtime ;
	time_t		utime ;
	uino_t		ino ;
	dev_t		dev ;
	int		narticles ;
	int		order ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int mkdirlist_start(MKDIRLIST *,const char *,const char *) ;
extern int mkdirlist_link(MKDIRLIST *) ;
extern int mkdirlist_defshow(MKDIRLIST *) ;
extern int mkdirlist_sort(MKDIRLIST *) ;
extern int mkdirlist_get(MKDIRLIST *,int,MKDIRLIST_ENT **) ;
extern int mkdirlist_ung(MKDIRLIST *,const char *,time_t,int,int) ;
extern int mkdirlist_showdef(MKDIRLIST *) ;
extern int mkdirlist_show(MKDIRLIST *,const char *,int) ;
extern int mkdirlist_audit(MKDIRLIST *) ;
extern int mkdirlist_finish(MKDIRLIST *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MKDIRLIST_INCLUDE */


