/* last modified %G% version %I% */

/* process file names */

/*
	David A.D. Morano
	August 86 
*/






/*****************************************************************************

program ()
{
	repeat {

	get next file name ;
	run specified command with file name as argument ;

	}
}

*****************************************************************************/






int main(argc,argv)
int	argc ;
char	*argv[] ;
long	al, ap ;
{
	bfile	berr, *efp = &berr ;

	bfile	bout, *ofp = &bout ;

	int	pan, i, n ;
	int	cal ;

	char	*cap ;

	pan = 0 ;			/* number of positional so far */

	i = 0 ;
	cap = argv[i++] ;
	cal = strlen(cap) ;

	argc -= 1 ;

	while (argc > 0) {

		cap = argv[i++] ;
		cal = strlen(cap) ;

		if ((cal > 0) && (*cap == '-')) {

			if (cal > 1) {

				cap +=1 ;
				switch ((int) *cap) {

			case 'b':
				rs = ghp(&fsb,&temp) ;
				if (rs == GP_BAD) return ;

				if (rs == GP_OK) tbuf = (char *) temp ;

				break ;

			case 'r':
				rs = ghp(&fsb,&temp) ;
				if (rs == GP_BAD) return ;

				if (rs == GP_OK) rbuf = (char *) temp ;

				break ;

			case 'l':
				rs = ghp(&fsb,&temp) ;
				if (rs == GP_BAD) return ;

				if (rs == GP_OK) ulen = temp ;

				break ;

			case 'd':
				rs = ghp(&fsb,&temp) ;
				if (rs == GP_BAD) return ;

				if (rs == GP_OK) udst = temp ;

				break ;

			case 't':
				rs = ghp(&fsb,&temp) ;
				if (rs == GP_BAD) return ;

				if (rs == GP_OK) wtime = temp ;

				break ;

			case 'w':
			case 'k':
				rs = ghp(&fsb,&temp) ;
				if (rs == GP_BAD) return ;

				if (rs == GP_OK) uk = temp ;

				break ;

			default:
				printf("unknown option - %c\n",*fsb.fp) ;
				return(BAD) ;

				} ; /* end switch */

			} else {

				i++ ;	/* increment position count */

			} ; /* end if */

		} else {

		    if (cal && (i < 6)) {

			rs = cfhex(cal,cap,&temp) ;

			if (rs != OK) {

				bprintf(efp,"bad HEX number in argument\n") ;

				goto return ;

			}

			switch (i) {

		case 0:
			udst = temp ; break ;

		case 1:
			tbuf = (char *) temp ; break ;

		case 2:
			rbuf = (char *) temp ; break ;

		case 3:
			ulen = temp ; break ;

		case 4:
			wtime = temp ; break ;

		case 5:
			uk = temp ; break ;
			} ;

		    } ;
		    i++ ;

		} ; /* end if */

	} ; /* end while */



/* check arguments */



return:
	bclose(efp) ;

	return OK ;
}


