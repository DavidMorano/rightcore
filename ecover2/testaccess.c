/* testaccess */
/* lang=C99 */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<localmisc.h>
#include	<exitcodes.h>


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{
	const int	am = R_OK ;
	int		rs ;

	rs = u_access("main.c",am) ;

	printf("hooray! rs=%d\n",rs) ;

	return EX_OK ;
}
/* end subroutine (main) */


