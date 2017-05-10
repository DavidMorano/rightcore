/* calculate a file name */
int ow_setfname(lip,fname,ebuf,el,f_def,dname,name,suf)
OW		*lip ;
char		fname[] ;
const char	ebuf[] ;
const char	dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int	rs = 0 ;
	int	ml ;
	int	fl = 0 ;

	char	tmpname[MAXNAMELEN + 1] ;
	char	*np ;


	if ((f_def && (ebuf[0] == '\0')) || (strcmp(ebuf,"+") == 0)) {
	    np = (char *) name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {
	        np = tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;
	    }
	    if (np[0] != '/') {
	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,lip->pr,dname,np) ;
	        } else
	            rs = mkpath2(fname, lip->pr,np) ;
	    } else
	        rs = mkpath1(fname, np) ;
	    fl = rs ;
	} else if (strcmp(ebuf,"-") == 0) {
	    fname[0] = '\0' ;
	} else if (ebuf[0] != '\0') {
	    np = (char *) ebuf ;
	    if (el >= 0) {
	        np = tmpname ;
	        ml = MIN(MAXPATHLEN,el) ;
	        strwcpy(tmpname,ebuf,ml) ;
	    }
	    if (ebuf[0] != '/') {
	        if (strchr(np,'/') != NULL) {
	            rs = mkpath2(fname,lip->pr,np) ;
	        } else {
	            if ((dname != NULL) && (dname[0] != '\0')) {
	                rs = mkpath3(fname,lip->pr,dname,np) ;
	            } else
	                rs = mkpath2(fname,lip->pr,np) ;
	        }
	    } else
	        rs = mkpath1(fname,np) ;
	    fl = rs ;
	} /* end if */

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (ow_setfname) */



