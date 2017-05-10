/* checkaudio */

/* check this audio stream */


#define	CF_DEBUG	1		/* switchable debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar
	types of functions.


*/


/******************************************************************************

	Here we check the audio stream by reading and processing
	audio samples.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/audioio.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"alevel.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static int	audio_recordpause(int,int) ;


/* local variables */







int checkaudio(pip,ofp,devfname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	devfname[] ;
{
	audio_info_t	ainfo, aset ;

	int	rs ;
	int	n, i ;
	int	fd ;


	if (devfname == NULL)
	    return SR_FAULT ;

	if (ofp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkaudio: entered devfname=%s\n",devfname) ;
#endif

	AUDIO_INITINFO(&aset) ;


/* start recording */

	rs = u_open(devfname,O_RDONLY,0666) ;

	fd = rs ;
	if (rs >= 0) {

	    float	*fbuf = NULL ;

	    uint	bufsize, bufsamples ;
	    uint	rate, channels, precision ;

	    int		size ;

	    short	*abuf = NULL ;


/* query the audio device */

	    rs = u_ioctl(fd,AUDIO_GETINFO,&ainfo) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("checkaudio: u_ioctl() query rs=%d\n",rs) ;
	        debugprintf("checkaudio: port=%d\n",ainfo.record.port) ;
	        debugprintf("checkaudio: bufsize=%d\n",ainfo.record.buffer_size) ;
	        debugprintf("checkaudio: gain=%d\n",ainfo.record.gain) ;
	        debugprintf("checkaudio: chans=%d\n",ainfo.record.channels) ;
	        debugprintf("checkaudio: encoding=%d\n",ainfo.record.encoding) ;
	        debugprintf("checkaudio: precision=%d\n",ainfo.record.precision) ;
	        debugprintf("checkaudio: rate=%d\n",ainfo.record.sample_rate) ;
	        debugprintf("checkaudio: monitor=%d\n",ainfo.monitor_gain) ;
	    }
#endif

	    bufsize = ainfo.record.buffer_size ;

/* set the audio device */

	    if (pip->audioport >= 0)
	        aset.record.port = pip->audioport ;

	    rate = ainfo.record.sample_rate ;
	    if (pip->audiorate >= 0) {
		rate = pip->audiorate ;
	        aset.record.sample_rate = pip->audiorate ;
	    }

	    channels = ainfo.record.channels ;
	    if (pip->audiochans >= 0) {
		channels = pip->audiochans ;
	        aset.record.channels = pip->audiochans ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("checkaudio: setting audio\n") ;
	        debugprintf("checkaudio: port=%d\n",aset.record.port) ;
	        debugprintf("checkaudio: rate=%d\n",aset.record.sample_rate) ;
	        debugprintf("checkaudio: chans=%d\n",aset.record.channels) ;
	}
#endif

	    precision = 16 ;
	    aset.record.encoding = AUDIO_ENCODING_LINEAR ;
	    aset.record.precision = 16 ;

#ifdef	COMMENT
	    aset.record.gain = AUDIO_MAX_GAIN ;
#endif

	    if (pip->f.monitor)
	        aset.monitor_gain = pip->monitorvol ;

	    bufsamples = bufsize / ((precision / 8) * channels) ;

	    aset.record.pause = TRUE ;
	    rs = u_ioctl(fd,AUDIO_SETINFO,&aset) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("checkaudio: u_ioctl() rs=%d\n",rs) ;
#endif

#ifdef	COMMENT
	    if (rs >= 0)
	        rs = u_ioctl(fd,I_FLUSH,NULL) ;
#endif

	    if (rs >= 0) {

	        rs = uc_malloc(bufsize,&abuf) ;

	        if (rs < 0)
	            abuf = NULL ;

	    }

	    if (rs >= 0) {

	        size = bufsamples * channels * 
	            (precision / 8) * sizeof(float) ;

	        rs = uc_malloc(size,&fbuf) ;

	        if (rs < 0)
	            fbuf = NULL ;

	    }

	    if (rs >= 0) {

	        ALEVEL	al ;

	        uint	max, secs, c, m, s, r ;

	        int	len ;


	        alevel_init(&al) ;

	        audio_recordpause(fd,FALSE) ;

	        secs = 0 ;
	        c = 0 ;
	        while ((rs = u_read(fd,abuf,bufsize)) > 0) {

	            len = rs ;
	            n = len / (precision / 8) ;
	            for (i = 0 ; i < n ; i += 1) {

	                fbuf[i] = (float) abuf[i] ;
	                fbuf[i] /= SHORT_MAX ;
	                if (fbuf[i] < 0.0)
	                    fbuf[i] = (- fbuf[i]) ;

	            } /* end for */

		    s = (n / channels) ;
		    r = 0 ;
		    if ((c + s) >= rate) {
			r = (c + s) - rate ;
			s = rate - c ;
		    }

	            alevel_proc(&al,fbuf,(s * 2)) ;

	            c += (n / channels) ;
	            if (c >= rate) {

	                ULONG	levels[100] ;


	                secs += 1 ;
	                c -= rate ;
	                alevel_getlevel(&al,levels) ;

	                max = 0 ;
	                for (i = 99 ; i >= 0 ; i -= 1) {

	                    if (levels[i] > 0) {
	                        max = levels[i] ;
	                        break ;
	                    }

	                } /* end for */

	                bprintf(ofp,
	                    "%5u %2u=%5u\n",
	                    secs,i,max) ;

#ifdef	COMMENT
	                bflush(ofp) ;
#endif

	                alevel_zero(&al) ;

	            } /* end if (one second interval) */

			if (r > 0)
	            alevel_proc(&al,fbuf,(r * 2)) ;

	        } /* end while */

	        alevel_free(&al) ;

	    } /* end if (successful audio access) */

	    if (fbuf != NULL)
	        free(fbuf) ;

	    if (abuf != NULL)
	        free(abuf) ;

	    u_close(fd) ;

	} /* end if (opened audio) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("checkaudio: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checkaudio) */



/* LOCAL SUBROUTINES */



static int audio_recordpause(fd,f)
int	fd ;
int	f ;
{
	audio_info_t	ainfo ;

	int	rs ;


	AUDIO_INITINFO(&ainfo) ;
	ainfo.record.pause = (f) ? TRUE : FALSE ;
	rs = u_ioctl(fd,AUDIO_SETINFO,&ainfo) ;

	return rs ;
}
/* end subroutine (audio_recordpause) */



