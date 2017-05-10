/* cd */



#include <sys/types.h>
#include <sys/cdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <assert.h>
#include <volmgt.h>
#include <stdio.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* global data */

struct options	Options ;



/*----------------------------------------------------------------------
|    get_disk_id
+---------------------------------------------------------------------*/
unsigned long get_disk_id(CdInfo *cd)
{
	int i ;
	int t = 0 ;
	int n = 0 ;

	for (i=0; i<cd->nb_tracks; i++) {
	    n += checksum((cd->tracks[i].msf.minute-cd->tracks[0].msf.minute)*60 
	        + (cd->tracks[i].msf.second - cd->tracks[0].msf.second) ) ;
	    t += cd->tracks[i].duration.frames/FRAMES_PER_SECOND ;
	}
	return ((n % 0xFF) << 24 | t << 8 | cd->nb_tracks) ;
}


/*----------------------------------------------------------------------
|    get_cd_info
+---------------------------------------------------------------------*/
void get_cd_info(CdInfo *cd, char *device)
{
	int                 io_result ;
	struct cdrom_tochdr toc_hdr ;
	int                 i ;
	int                 index = 0 ;

	if (Options.verbose) {
	    if (volmgt_running()) {
	        fprintf(stderr, 
		"CDROM: volume management running, using device %s\n", 
			Options.device_name) ;
	    } else {
	        fprintf(stderr, 
			"CDROM: volume management not running, device %s\n", 
		Options.device_name) ;
	    }
	}

	cd->fd = open(device, O_RDONLY) ;
	CHECK (cd->fd >= 0, "unable to open cdrom device") ;

	io_result = ioctl(cd->fd, CDROMREADTOCHDR, &toc_hdr) ;
	CHECK(io_result >= 0, "ioctl CDROMREADTOCHDR failed") ;

	if (Options.change_block_size) {
/* get block size */
	    io_result = ioctl(cd->fd, CDROMGBLKMODE, &Options.block_size) ;
	    CHECK(io_result >= 0, "ioctl CDROMGBLKMODE failed") ;
	    if (Options.verbose) {
	        fprintf(stderr,"CDROM: block size was %d, setting to 2352\n",
	            Options.block_size) ;
	    }

/* set block size for NO_SUBCODE mode */
	    io_result = ioctl(cd->fd, CDROMSBLKMODE, CDROM_BLK_2352) ;
	    if (io_result < 0) {
	        fprintf(stderr, PROGNAME ": ioctl CDROMSBLKMODE failed\n") ;
	        fprintf(stderr, PROGNAME ": try using option '-b'\n") ;
	        exit(3) ;
	    } else {
	        Options.block_size = 2352 ;
	    }
	}

/* number of tracks */
	cd->nb_tracks = toc_hdr.cdth_trk1 - toc_hdr.cdth_trk0 + 1 ;
	cd->tracks = (Track *)malloc(cd->nb_tracks*sizeof(Track)) ;
	CHECK(cd->tracks, "unable to allocate tracks") ;

/* update first track and disc duration*/
	cd->duration.frames = 0 ;
	cd->tracks[0].index = toc_hdr.cdth_trk0 ;

	if (Options.verbose || Options.info_only) {
	    fprintf(stderr, "CDROM: +-----------------------------------------------------+\n") ;
	    if (Options.display_addr) {
	        fprintf(stderr, "CDROM: | TRACK |   ADDR   | TIMECODE | DURATION |    SIZE    |\n") ;
	    } else {
	        fprintf(stderr, "CDROM: | TRACK |  FRAME   | TIMECODE | DURATION |    SIZE    |\n") ;
	    }
	    fprintf(stderr, "CDROM: +-------+----------+----------+----------+------------+\n") ;
	}

/* get all tracks info */
	for (index = 0, i=cd->tracks[0].index; 
	    index < cd->nb_tracks; 
	    i++, index++) {
	    struct cdrom_tocentry toc_ent ;

	    cd->tracks[index].index = i ;
	    toc_ent.cdte_track = i ;

/* get the track info in MSF format */
	    toc_ent.cdte_format = CDROM_MSF ;
	    io_result = ioctl(cd->fd, CDROMREADTOCENTRY, &toc_ent) ;
	    CHECK(io_result >= 0, "ioctl CDROMREADTOCENTRY failed") ;

	    cd->tracks[index].msf.minute = toc_ent.cdte_addr.msf.minute ;
	    cd->tracks[index].msf.second = toc_ent.cdte_addr.msf.second ;
	    cd->tracks[index].msf.frame  = toc_ent.cdte_addr.msf.frame ;
	    cd->tracks[index].frame = msf_to_frames(cd->tracks[index].msf) ;

/* get the track info in LBA format */
	    toc_ent.cdte_format = CDROM_LBA ;
	    io_result = ioctl(cd->fd, CDROMREADTOCENTRY, &toc_ent) ;
	    CHECK(io_result >= 0, "ioctl CDROMREADTOCENTRY failed") ;

	    cd->tracks[index].lba = toc_ent.cdte_addr.lba ;

/* compute the start addr of the track */
	    if (Options.change_block_size) {
	        cd->tracks[index].addr = cd->tracks[index].lba ;
	    } else {
	        cd->tracks[index].addr = 
	            cd->tracks[index].frame - cd->tracks[0].frame ;
	    }
	}

	{
	    struct cdrom_tocentry toc_ent ;
	    int last_frame ;

/* get the leadout track for duration computation */
	    toc_ent.cdte_track = CDROM_LEADOUT ;
	    toc_ent.cdte_format = CDROM_MSF ;
	    io_result = ioctl(cd->fd, CDROMREADTOCENTRY, &toc_ent) ;
	    CHECK(io_result >= 0, "ioctl CDROMREADTOCENTRY failed") ;

	    {
	        Msf msf ;
	        msf.minute = toc_ent.cdte_addr.msf.minute ;
	        msf.second = toc_ent.cdte_addr.msf.second ;
	        msf.frame = toc_ent.cdte_addr.msf.frame ;
	        last_frame = msf_to_frames(msf) ;
	    }

	    for (index = 0; index < cd->nb_tracks; index++) {
	        if (index == cd->nb_tracks-1) {
	            cd->tracks[index].duration.frames = 
	                last_frame-cd->tracks[index].frame ;
	        } else {
	            cd->tracks[index].duration.frames = 
	                cd->tracks[index+1].frame - cd->tracks[index].frame ;
	        }

	        cd->duration.frames += cd->tracks[index].duration.frames ;

	        {
	            unsigned long frames = cd->tracks[index].duration.frames ;
	            cd->tracks[index].duration.msf.minute = frames / 
	                (60 * FRAMES_PER_SECOND) ;
	            frames -= (60 * FRAMES_PER_SECOND) * 
	                cd->tracks[index].duration.msf.minute ;
	            cd->tracks[index].duration.msf.second = frames / 
	                FRAMES_PER_SECOND ;
	            frames -= FRAMES_PER_SECOND * 
	                cd->tracks[index].duration.msf.second ;
	            cd->tracks[index].duration.msf.frame = frames ;
	        }

/* compute the pcm buffer size */
	        cd->tracks[index].size = 
	            cd->tracks[index].duration.frames*BLOCK_SIZE ;

/* bitrate */
	        cd->tracks[index].rate = CD_SAMPLE_RATE ;

	        if (Options.verbose || Options.info_only) {
	            float size = 
			(float) cd->tracks[index].duration.frames * 
			(float)BLOCK_SIZE / 1048576.0f ;
	            fprintf(stderr, 
	                "CDROM: | %5d | %8ld | %02d:%02d:%02d | %02d:%02d:%02d | %7.2f MB |\n",
	                index+cd->tracks[0].index,
	                Options.display_addr ?
	                cd->tracks[index].addr:
	                cd->tracks[index].frame,
	                cd->tracks[index].msf.minute,
	                cd->tracks[index].msf.second,
	                cd->tracks[index].msf.frame,
	                cd->tracks[index].duration.msf.minute,
	                cd->tracks[index].duration.msf.second,
	                cd->tracks[index].duration.msf.frame,
	                size) ;
	        }
	    }
	}

/* finalize cd duration */
	{
	    unsigned long frames = cd->duration.frames ;
	    cd->duration.msf.minute = frames / (FRAMES_PER_SECOND * 60) ;
	    frames -= (FRAMES_PER_SECOND * 60) * cd->duration.msf.minute ;
	    cd->duration.msf.second = frames / FRAMES_PER_SECOND ;
	    frames -= FRAMES_PER_SECOND * cd->duration.msf.second ;
	    cd->duration.msf.frame = frames ;
	}

	if (Options.verbose || Options.info_only) {
	    float size = (float)cd->duration.frames * (float)BLOCK_SIZE / 1E6f ;
	    fprintf(stderr, "CDROM: +-------+----------+----------+----------+------------+\n") ;
	    fprintf(stderr, 
	        "CDROM: | TOTAL | %8ld |          | %02d:%02d:%02d | %7.2f MB |\n",
	        cd->duration.frames,
	        cd->duration.msf.minute,
	        cd->duration.msf.second,
	        cd->duration.msf.frame,
	        size) ;
	    fprintf(stderr, "CDROM: +-------+----------+----------+----------+------------+\n") ;
	    fprintf(stderr, "CDROM: disk id = %lx\n", get_disk_id(cd)) ;
	}
}
/* end subroutine (get_cd_info) */


/* +---------------------------------------------------------------------
|    restore_default_config
+---------------------------------------------------------------------*/
void restore_default_config(CdInfo *cd)
{
	int io_result ;

	if (!Options.change_block_size) return ;

/* set block size for NO_SUBCODE mode */
	if (Options.verbose) {
	    fprintf(stderr,"CDROM: restoring block size of 512\n") ;
	}
	io_result = ioctl(cd->fd, CDROMSBLKMODE, CDROM_BLK_512) ;
	CHECK(io_result >= 0, "ioctl CDROMSBLKMODE failed") ;
}




