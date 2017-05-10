/* determine how to insert the new entry */

	hv = hep->hv ;
	hi = hv % htlen ;
	if (htaddr[hi] != NULL) {

	    keyp = &nhep->key ;
	    nkeydat = keyp->buf ;
	    keylen = keyp->len ;
	    for (ep = htaddr[hi] ; ep != NULL ; ep = ep->next) {

	        ihep = ep ;
	        keydat = ep->key.buf ;
	        if (keydat == NULL)
	            continue ;

	        f = (ep->hv == hv) ;
	        f = f && (ep->key.len == keylen) ;
	        f = f && ((*op->cmpfunc)(nkeydat,keydat,keylen) == 0) ;
	        if (f)
		    break ;

	    } /* end for */

	    if (ep != NULL) {

	        while (ep->same != NULL)
	            ep = ep ->same ;

	        ep->same = nhep ;

	    } else
	        htaddr[hi] = nhep ;

	} else
		htaddr[hi] = nhep ;

