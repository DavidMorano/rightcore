/* main */



#include	<stdlib.h>
#include	<stdio.h>

#include	"ssh.h"



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	SSH		hammocks ;

	SSH_ENT	*ep ;

	int	rs, ri ;
	int	ia ;
	int	fd_debug ;

	char	*cp ;


	if (((cp = getenv("DEBUGFD")) != NULL) &&
		(cfdeci(cp,-1,&fd_debug) >= 0))
		debugsetfd(fd_debug) ;


	if (argc < 2)
	    return 1 ;

	fprintf(stderr,"argv[1]=%s\n",argv[1]) ;

	rs = ssh_init(&hammocks,argv[1]) ;

	fprintf(stderr,"ssh_init() rs=%d\n",rs) ;

	if (argc >= 3) {

	fprintf(stderr,"enumerate\n") ;

	    for (ri = 0 ; ssh_get(&hammocks,ri,&ep) >= 0 ; ri += 1) {

	fprintf(stderr,"ep=%p\n",ep) ;

	        if (ep == NULL)
	            continue ;

	fprintf(stderr,"ia=%08x\n",ep->ia) ;

	        if ((ep->ia != 0) && (ep->type & SSH_BTSSH))
	            fprintf(stdout,"%08x\n",ep->ia) ;

	    } /* end for */

	fprintf(stderr,"number recs=%d\n",ri) ;

	} else {

	fprintf(stderr,"search\n") ;

	    for (ia = 0x400000 ; ia < 0x460000 ; ia += 4) {

	        rs = ssh_check(&hammocks,ia,&ep) ;

	        if ((rs >= 0) && (ep->type & SSH_BTSSH))
	            fprintf(stdout,"%08x\n",ia) ;

	    } /* end for */

	} /* end if */

	ssh_free(&hammocks) ;

	fclose(stdout) ;

	fclose(stderr) ;

	return 0 ;
}



