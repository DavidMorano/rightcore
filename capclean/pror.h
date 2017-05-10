/* pror */


#ifndef	PROR_INCLUDE
#define	PROR_INCLUDE	1


#ifndef	TYPEDEF_UTIMET
#define	TYPEDEF_UTIMET
typedef unsigned long	utime_t	;
#endif

/* NULL linked structure list end */

#ifndef	NULLLINK
#define	NULLLINK		((void *) 0xFFFFFFFF)
#endif


/* return values */

#define	PROR_OK			0
#define	PROR_BADTMP		-1
#define	PROR_NAMEOPEN		-2
#define	PROR_STATOPEN		-3
#define	PROR_DUP		-4
#define	PROR_DIROPEN		-5
#define	PROR_BADSTAT		-6



#endif /* PROR_INCLUDE */



