	
	const int	n = s.length() ;
	int		c[3] = { 0, 0, 0 } ;
	int		nw ;
	int		w ;
	int		ct = 0 ;
	bool		f = true ;
	bool		f_open ;
	nw = nelem(c) ;
	for (int i = 0 ; i < n ; i += 1) {
	    int	ch = (s[i] & UCHAR_MAX) ;
	    f_open = false ;
	    switch (ch) {
	    case '(':
		f_open = true ;
		w = 0 ;
		break ;
	    case '{':
		f_open = true ;
		w = 1 ;
		break ;
	    case '[':
		f_open = true ;
		w = 2 ;
		break ;
	    case ')':
		w = 0 ;
		break ;
	    case '}':
		w = 1 ;
		break ;
	    case ']':
		w = 2 ;
		break ;
	    } /* end switch */
	    if (f_open) { /* open */
		if ((c[w] < (n-i)) || (ct < (n-i))) {
		    c[w] += 1 ;
		    ct += 1 ;
		} else {
		    f = false ;
		    break ;
		}
		break ;
	    } else { /* close */
		if (c[w] > 0) {
		    c[w] -= 1 ;
		    ct -= 1 ;
		} else {
		    f = false ;
		    break ;
		}
	    }
	} /* end for */
	return f ;

