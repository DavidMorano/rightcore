/* wdt */


#ifndef	INC_PROCESS
#define	INC_PROCESS	1


#ifndef	TYPEDEF_UTIMET
#define	TYPEDEF_UTIMET
typedef unsigned long	utime_t	;
#endif

/* NULL linked structure list end */

#define	NULLLINK		((void *) 0xFFFFFFFF)


/* return values */

#define	PROR_OK			0
#define	PROR_BADTMP		-1
#define	PROR_BADTMPOPEN		-2
#define	PROR_STATOPEN		-3
#define	PROR_DUP		-4
#define	PROR_DIROPEN		-5
#define	PROR_BADSTAT		-6



#endif /* INC_PROCESS */



