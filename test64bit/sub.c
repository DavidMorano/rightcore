/* sub */



#define	CF_DEBUGS	1


#include	<envstandard.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdarg.h>

#include	"localmisc.h"
#include	"defs.h"




extern struct global	g ;




void sub(LONG lv,...)
{
	va_list	ap ;

	ULONG	lv = 0 ;

	int	i ;
	int	iv = 0 ;
	int	len ;

	char	decbuf[40] ;


#if	CF_DEBUGS
	debugprintf("sub: entered\n") ;
#endif

	va_begin(ap,lv) ;

#if	CF_DEBUGS
	debugprintf("sub: before loop\n") ;
#endif

#if	CF_DEBUGS
	debugprintf("sub: LONG_MAX=%lld\n",(LONG) (LONG_MAX - 20)) ;

	/* lv = 23LL ; */
	lv = LONG_MIN + 20 ;
	lv = 9000072036854775787 ;
	lv = 0x8003456789abcdef ;
	lv = ULONG_MAX - 20 ;

	debugprintf("sub: format 64  hex lv=%016llX\n",lv) ;
	debugprintf("sub: format 64  dec lv=%lld\n",lv) ;
	debugprintf("sub: format 64 udec lv=%llu\n",lv) ;

	len = ctdecull(decbuf,36,lv) ;

	debugprintf("sub: ctdec64 len=%4X\n",len) ;

	debugprintf("sub: ctdec64 len=%d\n",len) ;

	debugprintf("sub: ctdec64 len=%d lv=%W\n",len,decbuf,MIN(len,20)) ;
#endif /* CF_DEBUGS */

	for (i = 1 ; i < 4 ; i += 1) {

#if	CF_DEBUGS
	debugprintf("sub: loop count=%d\n",i) ;
#endif

		iv = (int) va_arg(ap,int) ;

#if	CF_DEBUGS
	debugprintf("sub: before print, iv=%d\n",iv) ;
#endif

		bprintf(g.ofp,"%d> %08X\n",i,iv) ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("sub: after loop\n") ;
#endif

	va_end(ap) ;

#if	CF_DEBUGS
	debugprintf("sub: exiting\n") ;
#endif

}
/* end subroutine (sub) */



