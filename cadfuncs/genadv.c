/* genadv */

/* general ADVICE connectivity on a subcircuit */

/* called as :
	genadv <subcircuit> <output_file>

*/

int genadv(argc,argv)
int	argc ;
char	*argv[] ;
{
	int	rs, i ;
	int	f_pageonly = FALSE ;

	float	ml ;

	char	curname[GRP_NM_SZ + 1] ;
	char	*filename = "MAIN" ;
	char	*subcircuit ;


	if ((argc >= 2) && (argv[1][0] != '\0') && (argv[1][0] != '-')) {

	    subcircuit = argv[1] ;
	    if (strchr(subcircuit,'.') != NULL) f_pageonly = TRUE ;

	} else {

	    if (s_curgroup(curname) != 0) {

	        printf(NEED_PAG_SYM) ;

	        return BAD ;
	    }

	    for (i = 0 ; (curname[i] != '\0') ; i += 1) {

	        if (curname[i] == '.') {

	            curname[i] = '\0' ;
	            break ;
	        }

	    }

	    subcircuit = curname ;
	}

	if ((argc >= 3) && (argv[2][0] != '\0')) filename = argv[2] ;

	ml = s_getvar("message") + 0.3 ;

	s_command(dummy,"set var message 0") ;

	if (f_pageonly)
	rs = s_command(dummy,"gen adv page %s out %s hierarchy",
	    subcircuit,filename) ;

	else
	rs = s_command(dummy,"gen adv sub %s out %s hierarchy",
	    subcircuit,filename) ;

	s_command(dummy,"set var message %d\n",(int) ml) ;

	return rs ;
}
/* end subroutine (genadv) */



