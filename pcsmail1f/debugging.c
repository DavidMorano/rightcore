/* debugging */



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<errno.h>
#include	<unistd.h>
#include	<string.h>
#include	<signal.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"prompt.h"
#include	"header.h"



/* external subroutines */

extern int	logfile_printf() ;

extern char	*timestr_log() ;


/* external variables */

extern struct global	g ;

extern int	errno ;



void fileinfo(file,w)
char	file[] ;
char	w[] ;
{
	bfile		msgfile, *mfp = &msgfile ;

	struct ustat	sb ;

	int	i, j, rs, len, l ;

	char	linebuf[LINELEN + 1] ;
	char	buf1[LINELEN + 1], buf2[LINELEN + 2] ;


	if ((w != NULL) && (w[0] != '\0'))
	    logfile_printf(&g.eh,"fileinfo: where=\"%s\"\n",w) ;

	if ((file != NULL) || (file[0] != '\0')) {

	    logfile_printf(&g.eh,"fileinfo: file=\"%s\"\n",file) ;

	    if (stat(file,&sb) < 0) {

	        logfile_printf(&g.eh,"fileinfo: bad stat errno=%d\n",errno) ;

	    } else {

	        logfile_printf(&g.eh,
	            "fileinfo: len=%ld atime=%s mtime=%s\n",
	            sb.st_size,
	            timestr_log(sb.st_atime,buf1),
	            timestr_log(sb.st_mtime,buf2)) ;

	    }

	    if ((rs = bopen(mfp,file,"r",0666)) >= 0) {

		len = 0 ;
	        i = 0 ;

/* do all of the header lines */

	        while ((l = breadline(mfp,linebuf,LINELEN)) > 0) {

	            len += l ;

	            logfile_printf(&g.eh,"fileinfo: h %2d-%2d> %W",
	                i,l,linebuf,l) ;

	            if (linebuf[0] == '\n') break ;

	            i += 1 ;

	        } /* end while (header lines) */

/* do some of the body */

		j = 0 ;
	        while ((j < 5) && ((l = breadline(mfp,linebuf,LINELEN)) > 0)) {

	            len += l ;

	            logfile_printf(&g.eh,"fileinfo: b %2d-%2d> %W",
	                i,l,linebuf,l) ;

	            if (linebuf[0] == '\n') break ;

	            i += 1 ;
			j += 1 ;

	        } /* end while (body lines) */

	        logfile_printf(&g.eh,"fileinfo: %d header bytes, EOF=%d\n",
	            len,(l == 0)) ;

	        bclose(mfp) ;

	    } else {

	        logfile_printf(&g.eh,
	            "fileinfo: could not open (rs %d)\n",rs) ;

	    }

	} else
	    logfile_printf(&g.eh,"fileinfo: file was NULL\n") ;

}
/* end subroutine (fileinfo) */



