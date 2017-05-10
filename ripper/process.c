/* process */



#include <sys/types.h>
#include <sys/cdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <assert.h>
#include <volmgt.h>
#include <errno.h>
#include	<stdio.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* global data */

struct options	Options ;


/* forward references */

void show_progress() ;




/*----------------------------------------------------------------------
|    init
+---------------------------------------------------------------------*/
void init(void)
{


    Options.filename = NULL;
    Options.verbose = 0;
    Options.display_addr = 1;
    Options.show_progress = 0;
    Options.xinu = 0;
    Options.exit_on_jitter = 0;
    Options.info_only = 0;
    if (volmgt_running()) {
        Options.device_name = "/vol/dev/aliases/cdrom0";
    } else {
        Options.device_name = "/dev/rdsk/c0t6d0s2";
    }
    Options.change_block_size = 1;
    Options.track = 0;
    Options.dump_all = 0;
    Options.format = OUTPUT_SUN;

}


/*----------------------------------------------------------------------
|    dump_track
+---------------------------------------------------------------------*/
void dump_track(CdInfo *cd, int index, FILE *out)
{
	struct cdrom_cdda   cdda ;
	int                 io_result ;
	int                 retries ;
	int                 current ;
	char                *buffer ;
	char                *previous ;
	Track               *track ;
	int                 bytes_to_skip ;
	int                 blocks_to_write ;
	char *              previous_end ;


	if (index < cd->tracks[0].index 
	    || index >= cd->tracks[0].index+cd->nb_tracks) {
	    fprintf(stderr, 
		PROGNAME ": track number out of range [%d]\n", index) ;
	    exit(1) ;
	}

	index -= cd->tracks[0].index ;
	track = &cd->tracks[index] ;

	buffer = (char *)malloc(BLOCK_SIZE * READ_BURST_SIZE) ;
	CHECK(buffer,"unable to malloc() buffer") ;

	previous = (char *)malloc(BLOCK_SIZE * READ_BURST_SIZE) ;
	CHECK(previous,"unable to malloc() buffer") ;

	switch(Options.format) {
	case OUTPUT_WAV:  
	    wav_header(track, out);  
	    break ;
	case OUTPUT_AIF: 
	    aiff_header(track, out); 
	    break ;
	case OUTPUT_SUN:  
	    sun_header(track, out);  
	    break ;
	default: 
	    break ;
	}

	for (current = track->addr; 
	    current < track->addr + track->duration.frames; 
	    current += blocks_to_write) {
	    int result ;
	    int blocks_to_read ;
	    int blocks_overlap = READ_OVERLAP ;

	    if (current == track->addr) blocks_overlap = 0 ;

	    blocks_to_read = min(READ_BURST_SIZE, 
	        track->addr + track->duration.frames 
	        - (current - blocks_overlap)) ;

	    cdda.cdda_addr    = current - blocks_overlap ;
	    cdda.cdda_length  = blocks_to_read ;
	    cdda.cdda_data    = (caddr_t)buffer ;
	    cdda.cdda_subcode = CDROM_DA_NO_SUBCODE ;

	    if (Options.verbose) {
	        printf("CDROM: read from %d to %d [blocks = %d]\n",
	            current - blocks_overlap,
	            current - blocks_overlap + blocks_to_read,
	            blocks_to_read) ;
	    }

	    retries = MAX_RETRIES ;
	    do {
	        io_result = ioctl(cd->fd, CDROMCDDA, &cdda) ;
	        if (io_result < 0) {
	            char tmp[128] ;
	            if (errno == EIO) {
/* this is an I/O error, we can retry */
	                if (Options.verbose) {
	                    fprintf(stderr,
	                        PROGNAME ": retrying [%d] after I/O error [block = %d]\n",
	                        MAX_RETRIES-retries,
	                        current) ;
	                }
	                if (retries--) continue ;
	            }
	            sprintf(tmp,
			PROGNAME ": ioctl CDROMCDDA failed [block = %d]", 
	                current) ;
	            perror(tmp) ;
	            exit(1) ;
	        }
	    } while(io_result < 0) ;

	    if (Options.xinu) {
	        int i ;
	        unsigned char *xinu_buffer = (unsigned char *)buffer ;

	        for (i=0; i<(BLOCK_SIZE*blocks_to_read)/sizeof(short); i++) {
	            unsigned char swapped = *xinu_buffer ;
	            *xinu_buffer = *(xinu_buffer++ + 1) ;
	            *xinu_buffer++ = swapped ;
	        }
	    }

/* now, figure out how many blocks to keep (jitter control)   */
/*  |-------- previous --------|                              */
/*                         |----------- buffer --------|      */
/*  we need to match the end of previous and start of buffer  */
/*  so we do a match, and compute how many bytes of overlap   */
/*  to skip (bytes_to_skip)                                   */

	    if (current == track->addr) {
/* first run, take it all */
	        bytes_to_skip = 0 ;
	        blocks_to_write = blocks_to_read ;
	    } else {
	        int i ;
	        int found_match = 0 ;
/* first, try jitter control starting at the 'normal'
               position working backwards. If that fails, try going
               forward */
	        for (i = BLOCK_SIZE*(READ_OVERLAP-BLOCKS_COMPARE) ;
	            i >= 0 ;
	            i -= 4) {
	            if (memcmp(previous_end - BLOCK_SIZE * BLOCKS_COMPARE, 
	                buffer + i,
	                BLOCK_SIZE * BLOCKS_COMPARE) == 0) {
	                found_match = 1 ;
	                break ;
	            }
	        }
	        if (!found_match) {
	            for (i = BLOCK_SIZE*(READ_OVERLAP-BLOCKS_COMPARE) ;
	                i < blocks_to_read*BLOCK_SIZE ;
	                i += 4) {
	                if (memcmp(previous_end - BLOCK_SIZE * BLOCKS_COMPARE, 
	                    buffer + i,
	                    BLOCK_SIZE * BLOCKS_COMPARE) == 0) {
	                    found_match = 1 ;
	                    break ;
	                }
	            }
	        }
	        if (!found_match) {
	            if (Options.exit_on_jitter) {
	                fprintf(stderr,
				"CDROM: FATAL (jitter control failed)\n") ;
	                exit(2) ;
	            } else {
	                fprintf(stderr,
				"CDROM: WARNING (jitter control failed)\n") ;
	                i = 0 ;
	            }
	        }
	        bytes_to_skip = i + BLOCK_SIZE * BLOCKS_COMPARE ;
	        blocks_to_write = blocks_to_read - blocks_overlap ;

/* make sure we don't write more than we read */
	        while (bytes_to_skip + blocks_to_write*BLOCK_SIZE
	            > blocks_to_read * BLOCK_SIZE) blocks_to_write-- ;

/* make shure we don't write more than what's on the track */
	        if (blocks_to_write+current > track->addr+track->duration.frames) {
	            blocks_to_write=track->addr + track->duration.frames-current ;
	        }

/* we might have to add a partially padded  block at the end */
	        if (blocks_to_write == 0) {
	            current = track->addr + track->duration.frames - 1 ;
	            blocks_to_write = 1 ;
/* padd the end of the block */
	            {
	                int valid_bytes = 
				BLOCK_SIZE-(bytes_to_skip % BLOCK_SIZE) ;


	                for (i=valid_bytes ;
	                    i<BLOCK_SIZE ;
	                    i++) {
	                    buffer[bytes_to_skip+i] = 0 ;
	                }
	            }
	        }

	        if (Options.verbose) {
	            fprintf(stderr,
				"SKIP - overlap = %d bytes, match = %d bytes \n"
	                "SKIP - offset = %d bytes, will write %d blocks\n", 
	                blocks_overlap*BLOCK_SIZE,
	                bytes_to_skip, 
	                bytes_to_skip - blocks_overlap*BLOCK_SIZE,
	                blocks_to_write) ;
	        } else {
	            if (Options.show_progress) {
	                show_progress(index + cd->tracks[0].index,
	                    current, 
	                    track->addr,
	                    track->duration.frames,
	                    bytes_to_skip - blocks_overlap*BLOCK_SIZE) ;
	            }
	        }
	    }

	    result = fwrite((void*)(buffer + bytes_to_skip), 
	        BLOCK_SIZE * blocks_to_write, 1, out) ;
	    if (result == -1) {
	        perror(PROGNAME ": cannot write to output") ;
	        exit(1) ;
	    }
	    {
	        char *tmp = buffer ;

/* remember the end */
	        previous_end = buffer + bytes_to_skip + 
	            blocks_to_write * BLOCK_SIZE ;

/* swap the buffers */
	        buffer = previous ;
	        previous = tmp ;

	    }
	}
	assert(current == track->addr + track->duration.frames) ;
	free(buffer) ;
	free(previous) ;
}


/*----------------------------------------------------------------------
|    show_progress
+---------------------------------------------------------------------*/
void show_progress(int track,
              unsigned long current, 
              unsigned long start, 
              unsigned long duration, 
              int jitter)
{
    int percent = (100*(current-start))/duration;
    static int max_jitter = 100;
    int cursor_offset;
    char jitter_bar[80];

    fprintf(stderr, "\r[%02d] %06ld/%06ld (%02d%%)",
            track,
            current-start,
            duration,
            percent);

    if (jitter > 0) {
        if (jitter > max_jitter) max_jitter = jitter;
        cursor_offset = 16+(15*jitter)/max_jitter;
    } else {
        if (-jitter > max_jitter) max_jitter = -jitter;
        cursor_offset = 16-(15*-jitter)/max_jitter;
    }
    sprintf(jitter_bar, "[---------------0+++++++++++++++]");
    jitter_bar[cursor_offset] = '#';
    fprintf(stderr, " %s (jitter=%5d)  ", jitter_bar, jitter);

}



