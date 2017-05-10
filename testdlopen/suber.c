/* suber */


#define	CF_DEBUGS	0
#define	CF_DEBUGN	1
#define	CF_ENVIRON	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


#define	NDEBFNAME	"suber.deb"


#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	**environ ;


/* exported subroutines */


int suber(struct proginfo *pip)
{
	int	rs = SR_OK ;


#if	CF_DEBUGN
	nprintf(NDEBFNAME,"caller: entered\n") ;
	nprintf(NDEBFNAME,"caller: {environ}=%p\n",environ) ;
#endif

#if	CF_DEBUGS
	if (DEBUGLEVEL(5)) {
	debugprintf("caller: entered\n") ;
#if	CF_ENVIRON
	debugprintf("caller: {environ}=%p\n",environ) ;
#endif
	}
#endif /* CF_DEBUGS */


	return rs ;
}
/* end subroutine (caller) */



