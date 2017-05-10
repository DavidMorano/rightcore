/* main (galette) */
/*----------------------------------------------------------------------
|    Solaris CDA (audio CD) dumper
|
|    (c) 1996 Gilles Boccon-Gibod (bok@bok.net)
|             Alain Jobart        (aj@cybersoft.org)
|
+---------------------------------------------------------------------*/



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



/*----------------------------------------------------------------------
|    macros and constants
+---------------------------------------------------------------------*/
#define PROGNAME        "galette"
#define PROGVERSION     "1.2b (dam 0)"

#define EXIT_FAILURE      1
#define FRAMES_PER_SECOND 75
#define BLOCK_SIZE        2352
#define READ_BURST_SIZE   200
#define READ_OVERLAP      7
#define BLOCKS_COMPARE    3
#define WAVE_FORMAT_PCM   1
#define CD_SAMPLE_RATE    44100
#define MAX_RETRIES       10
#define IEEE_44100        "\x40\x0E\xAC\x44\0\0\0\0\0\0"
#define SND_HEADER_MAGIC  0x2E736E64
#define SND_FORMAT_LINEAR_16 3

#define CHECK(x,s) do { if (!(x)) fatal_error(s);} while(0)
#define min(a,b)  ((a)<(b))?(a):(b)

/*----------------------------------------------------------------------
|    types and enums
+---------------------------------------------------------------------*/
typedef enum { OUTPUT_PCM, OUTPUT_WAV, OUTPUT_AIFF, OUTPUT_SUN} OutputType;

typedef struct {
    unsigned char minute;
    unsigned char second;
    unsigned char frame;
} Msf;

typedef struct {
    int index;
    Msf msf;
    unsigned long lba;
    unsigned long frame;
    unsigned long addr;
    struct {
	unsigned long frames;
        Msf msf;
    } duration;
    unsigned long size;
    unsigned long rate;
} Track;

typedef struct {
    int         fd;
    int         nb_tracks;
    Track       *tracks;
    int         current;
    struct {
        unsigned long frames;
        Msf msf;
    } duration;
} CdInfo;

/*----------------------------------------------------------------------
|    globals
+--------------------------------------------------------------------*/
static struct {
    char        *filename;
    FILE        *out;
    int         xinu;
    int         exit_on_jitter;
    int         verbose;
    int         display_addr;
    int         show_progress;
    int         info_only;
    int         track;
    int         dump_all;
    int         block_size;
    int         change_block_size;
    char        *device_name;
    OutputType  format;
} Options;

/*----------------------------------------------------------------------
|    fatal_error
+---------------------------------------------------------------------*/
static void
fatal_error(char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

/*----------------------------------------------------------------------
|    le_convert_short
+---------------------------------------------------------------------*/
static void
le_convert_short(char *buffer, unsigned short s)
{
    unsigned char *b = (unsigned char *)buffer;
    b[0] = s & 0xFF;
    b[1] = s >> 8;
}

/*----------------------------------------------------------------------
|    le_convert_long
+---------------------------------------------------------------------*/
static void
le_convert_long(char *buffer, unsigned long l)
{
    unsigned char *b = (unsigned char *)buffer;
    b[0] = l & 0xFF; l >>= 8;
    b[1] = l & 0xFF; l >>= 8;
    b[2] = l & 0xFF; l >>= 8;
    b[3] = l & 0xFF;
}

/*----------------------------------------------------------------------
|    be_convert_short
+---------------------------------------------------------------------*/
static void
be_convert_short(char *buffer, unsigned short s)
{
    unsigned char *b = (unsigned char *)buffer;
    b[1] = s & 0xFF;
    b[0] = s >> 8;
}

/*----------------------------------------------------------------------
|    be_convert_long
+---------------------------------------------------------------------*/
static void
be_convert_long(char *buffer, unsigned long l)
{
    unsigned char *b = (unsigned char *)buffer;
    b[3] = l & 0xFF; l >>= 8;
    b[2] = l & 0xFF; l >>= 8;
    b[1] = l & 0xFF; l >>= 8;
    b[0] = l & 0xFF;
}

/*----------------------------------------------------------------------
|    cwrite
+---------------------------------------------------------------------*/
static void
cwrite(FILE *out, char *buffer, size_t size)
{
    int result;

    result = fwrite((void*)buffer, size, 1, out);
    if (result == -1) {
        perror("cannot write to output");
        exit(EXIT_FAILURE);
    }
}

/*----------------------------------------------------------------------
|    aiff_header
+---------------------------------------------------------------------*/
static void
aiff_header(Track *this, FILE *out)
{
    char buffer[4];

    cwrite(out, "FORM", 4);

    be_convert_long(buffer, this->size + 8 + 18 + 8 + 12 );
    cwrite(out, buffer, 4);
    
    cwrite(out, "AIFF", 4);

    cwrite(out, "COMM", 4);
        
    be_convert_long(buffer, 18); /* COMM chunnk size */
    cwrite(out, buffer, 4);

    be_convert_short(buffer, 2); /* number of channels = 2 */
    cwrite(out, buffer, 2);

    be_convert_long(buffer, (this->size/4)); /* number of frames */
    cwrite(out, buffer, 4);

    be_convert_short(buffer, 16); /* bits per sample */
    cwrite(out, buffer, 2);

    cwrite(out, IEEE_44100, 10); /* sample rate */

    cwrite(out, "SSND", 4);
    
    be_convert_long(buffer, this->size + 8); /* chunk size */
    cwrite(out, buffer, 4);

    cwrite(out, "\0\0\0\0", 4); /* offset */

    cwrite(out, "\0\0\0\0", 4); /* block size */
}

/*----------------------------------------------------------------------
|    wav_header
+---------------------------------------------------------------------*/
static void
wav_header(Track *this, FILE *out)
{
    char buffer[4];

    cwrite(out, "RIFF",   4);

    le_convert_long(buffer, this->size + 8+16+12);  /* RIFF chunk size */
    cwrite(out, buffer, 4);

    cwrite(out, "WAVE",   4);

    cwrite(out, "fmt ",   4);

    le_convert_long(buffer, 16L);
    cwrite(out, buffer, 4);

    le_convert_short(buffer, WAVE_FORMAT_PCM);
    cwrite(out, buffer, 2);

    le_convert_short(buffer, 2);                /* number of channels */
    cwrite(out, buffer, 2);

    le_convert_long(buffer, this->rate);        /* sample rate */
    cwrite(out, buffer, 4);

    le_convert_long(buffer, this->rate*4);      /* bytes per second */
    cwrite(out, buffer, 4);

    le_convert_short(buffer, 4);                /* alignment   */
    cwrite(out, buffer, 2);

    le_convert_short(buffer, 16);               /* bits per sample */
    cwrite(out, buffer, 2);

    cwrite(out, "data", 4);

    le_convert_long(buffer, this->size);        /* data size */
    cwrite(out, buffer, 4);
}

/*----------------------------------------------------------------------
|    sun_header
+---------------------------------------------------------------------*/
static void
sun_header(Track *this, FILE *out)
{
    char buffer[4];

    be_convert_long(buffer, SND_HEADER_MAGIC);
    cwrite(out, buffer, 4);

    be_convert_long(buffer, 32);         /* data location (offset to data) */
    cwrite(out, buffer, 4);
    
    be_convert_long(buffer, this->size); /* size in bytes */
    cwrite(out, buffer, 4);

    be_convert_long(buffer, SND_FORMAT_LINEAR_16);  /* format */
    cwrite(out, buffer, 4);

    be_convert_long(buffer, CD_SAMPLE_RATE);  /* sample rate */
    cwrite(out, buffer, 4);
    
    be_convert_long(buffer, 2);          /* number of channels */
    cwrite(out, buffer, 4);
    
    cwrite(out, "BOK!", 4);              /* free text */
}

/*----------------------------------------------------------------------
|    show_progress
+---------------------------------------------------------------------*/
static void
show_progress(int track,
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

/*----------------------------------------------------------------------
|    dump_track
+---------------------------------------------------------------------*/
static void
dump_track(CdInfo *cd, int index, FILE *out)
{
    struct cdrom_cdda   cdda;
    int                 io_result;
    int                 retries;
    int                 current;
    char                *buffer;
    char                *previous;
    Track               *track;
    int                 bytes_to_skip;
    int                 blocks_to_write;
    char *              previous_end;

    if (index < cd->tracks[0].index 
        || index >= cd->tracks[0].index+cd->nb_tracks) {
        fprintf(stderr, PROGNAME ": track number out of range [%d]\n", index);
        exit(1);
    }

    index -= cd->tracks[0].index;
    track = &cd->tracks[index];

    buffer = (char *)malloc(BLOCK_SIZE * READ_BURST_SIZE);
    CHECK(buffer,"unable to malloc() buffer");

    previous = (char *)malloc(BLOCK_SIZE * READ_BURST_SIZE);
    CHECK(previous,"unable to malloc() buffer");

    switch(Options.format) {
      case OUTPUT_WAV:  wav_header(track, out);  break;
      case OUTPUT_AIFF: aiff_header(track, out); break;
      case OUTPUT_SUN:  sun_header(track, out);  break;
      default: break;
    }

    for (current = track->addr; 
         current < track->addr + track->duration.frames; 
         current += blocks_to_write) {
        int result;
	int blocks_to_read;
        int blocks_overlap = READ_OVERLAP;

        if (current == track->addr) blocks_overlap = 0;

	blocks_to_read = min(READ_BURST_SIZE, 
                             track->addr + track->duration.frames 
                             - (current - blocks_overlap));
	
        cdda.cdda_addr    = current - blocks_overlap;
        cdda.cdda_length  = blocks_to_read;
        cdda.cdda_data    = (caddr_t)buffer;
        cdda.cdda_subcode = CDROM_DA_NO_SUBCODE;

        if (Options.verbose) {
            printf("CDROM: read from %d to %d [blocks = %d]\n",
                   current - blocks_overlap,
                   current - blocks_overlap + blocks_to_read,
                   blocks_to_read);
        }

        retries = MAX_RETRIES;
        do {
            io_result = ioctl(cd->fd, CDROMCDDA, &cdda);
            if (io_result < 0) {
                char tmp[128];
                if (errno == EIO) {
                    /* this is an I/O error, we can retry */
                    if (Options.verbose) {
                        fprintf(stderr,
                                PROGNAME ": retrying [%d] after I/O error [block = %d]\n",
                                MAX_RETRIES-retries,
                                current);   
                    }
                    if (retries--) continue;
                }
                sprintf(tmp,PROGNAME ": ioctl CDROMCDDA failed [block = %d]", 
                        current);
                perror(tmp);
                exit(1);
            }           
        } while(io_result < 0);

        if (Options.xinu) {
            int i;
            unsigned char *xinu_buffer = (unsigned char *)buffer;

            for (i=0; i<(BLOCK_SIZE*blocks_to_read)/sizeof(short); i++) {
                unsigned char swapped = *xinu_buffer;
                *xinu_buffer = *(xinu_buffer++ + 1);
                *xinu_buffer++ = swapped;
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
            bytes_to_skip = 0;
            blocks_to_write = blocks_to_read;
	} else {
            int i;
            int found_match = 0;
            /* first, try jitter control starting at the 'normal'
               position working backwards. If that fails, try going
               forward */
            for (i = BLOCK_SIZE*(READ_OVERLAP-BLOCKS_COMPARE);
                 i >= 0;
                 i -= 4) {
                if (memcmp(previous_end - BLOCK_SIZE * BLOCKS_COMPARE, 
                           buffer + i,
                           BLOCK_SIZE * BLOCKS_COMPARE) == 0) {
                    found_match = 1;
                    break;
                }
            }
            if (!found_match) {
                for (i = BLOCK_SIZE*(READ_OVERLAP-BLOCKS_COMPARE);
                     i < blocks_to_read*BLOCK_SIZE;
                     i += 4) {
                    if (memcmp(previous_end - BLOCK_SIZE * BLOCKS_COMPARE, 
                               buffer + i,
                               BLOCK_SIZE * BLOCKS_COMPARE) == 0) {
                        found_match = 1;
                        break;
                    }
                }
            }
            if (!found_match) {
                if (Options.exit_on_jitter) {
                    fprintf(stderr,"CDROM: FATAL (jitter control failed)\n");
                    exit(2);
                } else {
                    fprintf(stderr,"CDROM: WARNING (jitter control failed)\n");
                    i = 0;
                }
            }
            bytes_to_skip = i + BLOCK_SIZE * BLOCKS_COMPARE;
            blocks_to_write = blocks_to_read - blocks_overlap;

            /* make sure we don't write more than we read */
            while (bytes_to_skip + blocks_to_write*BLOCK_SIZE
                   > blocks_to_read * BLOCK_SIZE) blocks_to_write--;

            /* make shure we don't write more than what's on the track */
            if (blocks_to_write+current > track->addr+track->duration.frames) {
                blocks_to_write=track->addr + track->duration.frames-current;
            }

            /* we might have to add a partially padded  block at the end */
            if (blocks_to_write == 0) {
                current = track->addr + track->duration.frames - 1;
                blocks_to_write = 1;
                /* padd the end of the block */
                {
                    int valid_bytes = BLOCK_SIZE-(bytes_to_skip % BLOCK_SIZE);
                    for (i=valid_bytes;
                         i<BLOCK_SIZE;
                         i++) {
                        buffer[bytes_to_skip+i] = 0;
                    }
                }
            }

            if (Options.verbose) {
                fprintf(stderr,"SKIP: overlap = %d bytes, match = %d bytes \n"
                        "SKIP: offset = %d bytes, will write %d blocks\n", 
                        blocks_overlap*BLOCK_SIZE,
                        bytes_to_skip, 
                        bytes_to_skip - blocks_overlap*BLOCK_SIZE,
                        blocks_to_write);
            } else {
                if (Options.show_progress) {
                    show_progress(index + cd->tracks[0].index,
                                  current, 
                                  track->addr,
                                  track->duration.frames,
                                  bytes_to_skip - blocks_overlap*BLOCK_SIZE);
                }
            }
	}

        result = fwrite((void*)(buffer + bytes_to_skip), 
			BLOCK_SIZE * blocks_to_write, 1, out);
        if (result == -1) {
            perror(PROGNAME ": cannot write to output");
            exit(1);
        }
	{
            char *tmp = buffer;

            /* remember the end */
            previous_end = buffer + bytes_to_skip + 
                blocks_to_write * BLOCK_SIZE;

            /* swap the buffers */
            buffer = previous;
            previous = tmp;

	}
    }
    assert(current == track->addr + track->duration.frames);
    free(buffer);
    free(previous);
}       

/*----------------------------------------------------------------------
|    msf_to_frames
+---------------------------------------------------------------------*/
static int
msf_to_frames(Msf msf)
{
    return 
        msf.minute * FRAMES_PER_SECOND * 60 +
        msf.second * FRAMES_PER_SECOND +
        msf.frame;
}

/*----------------------------------------------------------------------
|    checksum
+---------------------------------------------------------------------*/
static unsigned long
checksum(unsigned long val)
{
    char ascii[128];
    char *s;
    int sum = 0;
    
    sprintf(ascii, "%lu", val);
    for (s = ascii; *s != '\0'; s++) sum += (*s - '0');

    return sum;
}

/*----------------------------------------------------------------------
|    get_disk_id
+---------------------------------------------------------------------*/
static unsigned long
get_disk_id(CdInfo *cd)
{
    int i;
    int t = 0;
    int n = 0;

    for (i=0; i<cd->nb_tracks; i++) {
        n += checksum((cd->tracks[i].msf.minute-cd->tracks[0].msf.minute)*60 
                      + (cd->tracks[i].msf.second - cd->tracks[0].msf.second) );
        t += cd->tracks[i].duration.frames/FRAMES_PER_SECOND;
    }
    return ((n % 0xFF) << 24 | t << 8 | cd->nb_tracks);
}


/*----------------------------------------------------------------------
|    get_cd_info
+---------------------------------------------------------------------*/
static void
get_cd_info(CdInfo *cd, char *device)
{
    int                 io_result;
    struct cdrom_tochdr toc_hdr;
    int                 i;
    int                 index = 0;

    if (Options.verbose) {
        if (volmgt_running()) {
          fprintf(stderr, "CDROM: volume management running, using device %s\n", Options.device_name);
        } else {
          fprintf(stderr, "CDROM: volume management not running, using device %s\n", Options.device_name);
        }
    }

    cd->fd = open(device, O_RDONLY);
    CHECK (cd->fd >= 0, "unable to open cdrom device");
    
    io_result = ioctl(cd->fd, CDROMREADTOCHDR, &toc_hdr);
    CHECK(io_result >= 0, "ioctl CDROMREADTOCHDR failed");

    if (Options.change_block_size) {
        /* get block size */
        io_result = ioctl(cd->fd, CDROMGBLKMODE, &Options.block_size);
        CHECK(io_result >= 0, "ioctl CDROMGBLKMODE failed");
        if (Options.verbose) {
            fprintf(stderr,"CDROM: block size was %d, setting to 2352\n",
                    Options.block_size);
        }

        /* set block size for NO_SUBCODE mode */
        io_result = ioctl(cd->fd, CDROMSBLKMODE, CDROM_BLK_2352);
        if (io_result < 0) {
            fprintf(stderr, PROGNAME ": ioctl CDROMSBLKMODE failed\n");
            fprintf(stderr, PROGNAME ": try using option '-b'\n");
            exit(3);
	} else {
	    Options.block_size = 2352;
	}
    }

    /* number of tracks */
    cd->nb_tracks = toc_hdr.cdth_trk1 - toc_hdr.cdth_trk0 + 1;
    cd->tracks = (Track *)malloc(cd->nb_tracks*sizeof(Track));
    CHECK(cd->tracks, "unable to allocate tracks");

    /* update first track and disc duration*/
    cd->duration.frames = 0;
    cd->tracks[0].index = toc_hdr.cdth_trk0;

    if (Options.verbose || Options.info_only) {
        fprintf(stderr, "CDROM: +-----------------------------------------------------+\n");
        if (Options.display_addr) {
            fprintf(stderr, "CDROM: | TRACK |   ADDR   | TIMECODE | DURATION |    SIZE    |\n");
        } else {
            fprintf(stderr, "CDROM: | TRACK |  FRAME   | TIMECODE | DURATION |    SIZE    |\n");
        }
        fprintf(stderr, "CDROM: +-------+----------+----------+----------+------------+\n");
    }
        
    /* get all tracks info */
    for (index = 0, i=cd->tracks[0].index; 
         index < cd->nb_tracks; 
         i++, index++) {
        struct cdrom_tocentry toc_entry;

        cd->tracks[index].index = i;
        toc_entry.cdte_track = i;

        /* get the track info in MSF format */
        toc_entry.cdte_format = CDROM_MSF;
        io_result = ioctl(cd->fd, CDROMREADTOCENTRY, &toc_entry);
        CHECK(io_result >= 0, "ioctl CDROMREADTOCENTRY failed");
            
        cd->tracks[index].msf.minute = toc_entry.cdte_addr.msf.minute;
        cd->tracks[index].msf.second = toc_entry.cdte_addr.msf.second;
        cd->tracks[index].msf.frame  = toc_entry.cdte_addr.msf.frame;
        cd->tracks[index].frame = msf_to_frames(cd->tracks[index].msf);

        /* get the track info in LBA format */
        toc_entry.cdte_format = CDROM_LBA;
        io_result = ioctl(cd->fd, CDROMREADTOCENTRY, &toc_entry);
        CHECK(io_result >= 0, "ioctl CDROMREADTOCENTRY failed");

        cd->tracks[index].lba = toc_entry.cdte_addr.lba;

        /* compute the start addr of the track */
        if (Options.change_block_size) {
            cd->tracks[index].addr = cd->tracks[index].lba;
        } else {
            cd->tracks[index].addr = 
                cd->tracks[index].frame - cd->tracks[0].frame;
        }
    }
    
    {
        struct cdrom_tocentry toc_entry;
        int last_frame;

        /* get the leadout track for duration computation */
        toc_entry.cdte_track = CDROM_LEADOUT;
        toc_entry.cdte_format = CDROM_MSF;
        io_result = ioctl(cd->fd, CDROMREADTOCENTRY, &toc_entry);
        CHECK(io_result >= 0, "ioctl CDROMREADTOCENTRY failed");
         
        {
            Msf msf;
            msf.minute = toc_entry.cdte_addr.msf.minute;
            msf.second = toc_entry.cdte_addr.msf.second;
            msf.frame = toc_entry.cdte_addr.msf.frame;
            last_frame = msf_to_frames(msf);
        }

        for (index = 0; index < cd->nb_tracks; index++) {
            if (index == cd->nb_tracks-1) {
                cd->tracks[index].duration.frames = 
                    last_frame-cd->tracks[index].frame;
            } else {
                cd->tracks[index].duration.frames = 
                    cd->tracks[index+1].frame - cd->tracks[index].frame;
            }

	    cd->duration.frames += cd->tracks[index].duration.frames;

            {
                unsigned long frames = cd->tracks[index].duration.frames;
                cd->tracks[index].duration.msf.minute = frames / 
		  (60 * FRAMES_PER_SECOND);
                frames -= (60 * FRAMES_PER_SECOND) * 
		  cd->tracks[index].duration.msf.minute;
                cd->tracks[index].duration.msf.second = frames / 
		  FRAMES_PER_SECOND;
                frames -= FRAMES_PER_SECOND * 
		  cd->tracks[index].duration.msf.second;
                cd->tracks[index].duration.msf.frame = frames;
            }

            /* compute the pcm buffer size */
            cd->tracks[index].size = 
                cd->tracks[index].duration.frames*BLOCK_SIZE;

            /* bitrate */
            cd->tracks[index].rate = CD_SAMPLE_RATE;

            if (Options.verbose || Options.info_only) {
                float size = (float)cd->tracks[index].duration.frames * (float)BLOCK_SIZE / 1048576.0f;
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
                        size);
            }
        }
    } 
    
    /* finalize cd duration */
    {
        unsigned long frames = cd->duration.frames;
        cd->duration.msf.minute = frames / (FRAMES_PER_SECOND * 60);
        frames -= (FRAMES_PER_SECOND * 60) * cd->duration.msf.minute;
        cd->duration.msf.second = frames / FRAMES_PER_SECOND;
        frames -= FRAMES_PER_SECOND * cd->duration.msf.second;
        cd->duration.msf.frame = frames;
    }

    if (Options.verbose || Options.info_only) {
        float size = (float)cd->duration.frames * (float)BLOCK_SIZE / 1E6f;
        fprintf(stderr, "CDROM: +-------+----------+----------+----------+------------+\n");
        fprintf(stderr, 
                "CDROM: | TOTAL | %8ld |          | %02d:%02d:%02d | %7.2f MB |\n",
                cd->duration.frames,
                cd->duration.msf.minute,
                cd->duration.msf.second,
                cd->duration.msf.frame,
                size);
        fprintf(stderr, "CDROM: +-------+----------+----------+----------+------------+\n");
        fprintf(stderr, "CDROM: disk id = %lx\n", get_disk_id(cd));
    }
}

/*----------------------------------------------------------------------
|    print_usage_and_exit
+---------------------------------------------------------------------*/
static void
print_usage_and_exit(void)
{
    fprintf(stderr,"galette [options] [<filename> (default = stdout)]\n"
            "    Version " PROGVERSION "\n"
            "    options are:\n"
            "    -h       : prints this usage information\n"
            "    -V       : prints the program version number\n"
            "    -v       : verbose\n"
            "    -i       : print CD-ROM info and exit\n"
            "    -t <n>   : track number (default = 1)\n"
            "    -a       : dump all tracks, one by one (filename is used as a name pattern)\n"
            "    -f {WAV,AIFF,PCM,SUN}\n"
            "             : output format (default SUN)\n"
            "    -b       : do not try to set the block size to 2352 bytes\n"
            "    -x       : swap bytes in 16bits samples\n"
            "    -e       : exit if jitter control fails (exit status = 2)\n"
            "    -p       : show progress\n"
            "    -d <name>: CD-ROM device name\n"
            );
    exit(1);
}

/*----------------------------------------------------------------------
|    parse_command_line
+---------------------------------------------------------------------*/
static void
parse_command_line(char **argv)
{
    char *arg;

    while((arg = *argv++)) {
        char c;
        if (*arg == '-') {
            /* this is an option */
            switch(c = *++arg) {
              case 'h': print_usage_and_exit(); break;
              case 'V': {
                  fprintf(stderr, PROGNAME ": version " PROGVERSION "\n");
                  exit(0);
              }
              case 'v': Options.verbose = 1; break;
              case 'i': Options.info_only = 1; break;
              case 'x': Options.xinu = 1; break;
              case 'e': Options.exit_on_jitter = 1; break;
              case 'p': Options.show_progress = 1; break;
              case 'b': Options.change_block_size = 0; break;
	      case 'a': 
                if (Options.track) {
		  fprintf(stderr,PROGNAME ": options -t and -a are exclusive");
		  print_usage_and_exit();
		}
		Options.dump_all = 1;
		break;
              case 'f': {
                  char *format = *argv++;
                  if (!strcasecmp("WAV", format)) {
                      Options.format = OUTPUT_WAV;
                  } else if (!strcasecmp("PCM", format)) {
                      Options.format = OUTPUT_PCM;
                  } else if (!strcasecmp("AIF", format)) {
                      Options.format = OUTPUT_AIFF;
                  } else if (!strcasecmp("AIFF", format)) {
                      Options.format = OUTPUT_AIFF;
                  } else if (!strcasecmp("SUN", format)) {
                      Options.format = OUTPUT_SUN;
                  } else {
                      fprintf(stderr,
                              PROGNAME ": invalid output format %s\n", format);
                      print_usage_and_exit();
                  }
                  break;
              }
              case 't': 
                if (*argv) {
                    char *track = *argv++;
                    Options.track = atoi(track);
		    if (Options.dump_all) {
		      fprintf(stderr,
			      PROGNAME ": options -t and -a are exclusive");
		      print_usage_and_exit();
		    }
                    if (Options.track == 0 && errno == EINVAL) {
                        fprintf(stderr, 
                                PROGNAME ": invalid track number %s\n",
                                track);
                        print_usage_and_exit();
                    }
                }
                break;
              case 'd':
                if (*argv) {
                    Options.device_name = *argv++;
                } else {
                    fprintf(stderr, PROGNAME ": -d requires a device name\n");
                    print_usage_and_exit();
                }
                break;
            }
        } else {
            /* this is the output filename */
            if (Options.filename) {
                /* we already had a filename */
                fprintf(stderr, PROGNAME "argument %s ignored", arg);
            } else {
                Options.filename = arg;
            }
        }
    }
    if (Options.format == OUTPUT_AIFF || Options.format == OUTPUT_SUN) {
        Options.xinu = !Options.xinu;
    }
    if (Options.track == 0) Options.track = 1;
}

/*----------------------------------------------------------------------
|    restore_default_config
+---------------------------------------------------------------------*/
static void
restore_default_config(CdInfo *cd)
{
    int io_result;

    if (!Options.change_block_size) return;

    /* set block size for NO_SUBCODE mode */
    if (Options.verbose) {
        fprintf(stderr,"CDROM: restoring block size of 512\n");
    }
    io_result = ioctl(cd->fd, CDROMSBLKMODE, CDROM_BLK_512);
    CHECK(io_result >= 0, "ioctl CDROMSBLKMODE failed");
}

/*----------------------------------------------------------------------
|    init
+---------------------------------------------------------------------*/
static void
init(void)
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
|    main
+---------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *out = stdout;

    CdInfo cd;


    init();

    parse_command_line(++argv);

    get_cd_info(&cd, Options.device_name);

    if (Options.info_only) {
        restore_default_config(&cd);
        return 0;
    }

    if (Options.dump_all) {
        char filename[1024];
        int i;
      
        for (i=1; i<=cd.nb_tracks; i++) {
            if (Options.filename) {
                if (strstr(Options.filename, "%")) {
                    sprintf(filename, Options.filename, i);
                } else {
                    char *ext;


                    switch (Options.format) {

                      case OUTPUT_PCM:
                        ext = "pcm";
                        break;

                      case OUTPUT_WAV:
                        ext = "wav";
                        break;

                      case OUTPUT_AIFF:
                        ext = "aif";
                        break;

                      case OUTPUT_SUN:
                        ext = "au";
                        break;

                      default:
                        ext = "raw";
                        break;
                    }
                    sprintf(filename, "%s%02d.%s", Options.filename, i, ext);
                }
                if (out != stdout) fclose(out);
                out = fopen(filename, "w+");
                if (!out) {
                    perror(PROGNAME ": cannot open output");
                }
            }

            if (Options.verbose) {
                fprintf(stderr, PROGNAME ": dumping track %02d to %s\n",
                        i,
                        out == stdout ? "stdout" : filename);
            }
            dump_track(&cd, i, out);
            if (Options.show_progress) {
                fprintf(stderr, "\r                                                                          ");
                if (out == stdout) {
                    fprintf(stderr, "\rtrack %02d done\n", i);
                } else {      
                    fprintf(stderr, "\rtrack %02d (%s) done\n",
                            i,
                            filename);
                }
            }
        }
    } else {
        if (Options.filename) {
            out = fopen(Options.filename, "w+");
            if (!out) {
                perror(PROGNAME ": cannot open output");
            }
        }

        dump_track(&cd, Options.track, out);
    }

    restore_default_config(&cd);
    return 0;
}
/* end if (main) */


