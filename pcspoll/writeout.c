/* writeout */


#define	MAXOUTLEN	62


/* write out the output files from the executed program */
static int writeout(gp,fd,s)
struct proginfo	*gp ;
int	fd ;
char	s[] ;
{
	bfile		file, *fp = &file ;

	struct ustat	sb ;

	int		tlen, len ;
	int		lines ;

	char		linebuf[LINELEN + 1] ;


	tlen = 0 ;
	if ((u_fstat(fd,&sb) >= 0) && (sb.st_size > 0)) {

	    u_rewind(fd) ;

	    logfile_printf(&gp->lh,s) ;

	    lines = 0 ;
	    if (bopen(fp,(char *) fd,"dr",0666) >= 0) {

	        while ((len = bgetline(fp,linebuf,MAXOUTLEN)) > 0) {

	            tlen += len ;
	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&gp->lh,"| %W\n",
			linebuf,MIN(len,MAXOUTLEN)) ;

	            lines += 1 ;

	        } /* end while (reading lines) */

	        bclose(fp) ;

	    } /* end if (opening file) */

	    logfile_printf(&gp->lh,"lines=%d\n",lines) ;

	} /* end if (non-zero file size) */

	return tlen ;
}
/* end subroutine (writeout) */



