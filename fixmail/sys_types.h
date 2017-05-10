/*      @(#)types.h	11.3 btl-uts  source        */

#ifndef TYPES_H
#define TYPES_H
/*
 * double word integer type: longlong_t.  The C-preprocessor defines
 * the symbol LONGLONG if the compiler handles the type "long long".
 */
#ifdef LONGLONG


/* David A.D. Morano commented these next two lines out and added the two 
that follow.

typedef long long longlong_t;
typedef unsigned long long ulonglong_t;
*/

typedef double    longlong_t;
typedef double    ulonglong_t;

#else
typedef double    longlong_t;
typedef double    ulonglong_t;
#endif

typedef	struct { int r[1]; } *	physadr;
typedef	long		daddr_t;
typedef	char *		caddr_t;

typedef	unsigned short		ino_t;
typedef int             cnt_t;
typedef	long		time_t;
typedef int             label_t[14];
typedef unsigned short          dev_t;
typedef	long		offset_t;
typedef	long		paddr_t;
typedef	long		key_t;
typedef longlong_t     cpu_t;         /* cpu time as stored by STPT */
typedef ulonglong_t     tod_t;         /* time of day stored by STCK */
typedef ulonglong_t     usec_t;        /* time in microseconds */
typedef long            sig_t;         /* bit string for signals */
typedef unsigned        bit_t;         /* for bit fields         */
typedef short		index_t;	/* RFS */
typedef short		sysid_t;	/* RFS */
typedef struct { short sema_owner; short  sema_value; short sema_svalue; } sema1_t ;
#endif
