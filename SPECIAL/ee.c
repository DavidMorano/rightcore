	    argp = argv[i++] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (cfdec(argp + 1,argl - 1,&messnum)) goto badarg ;

	            } else if (argp[1] == '-') {

			f_ddash = TRUE ;

	            } else {

	                aop = argp ;
	                aol = argl ;
	                while (--aol) {

	                    aop += 1 ;
	                    switch (*aop) {

