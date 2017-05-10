/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


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
typedef enum { OUTPUT_PCM, OUTPUT_WAV, OUTPUT_AIF, OUTPUT_SUN} OutputType;

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
struct options {
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
} ;


#endif /* DEFS_INCLUDE */


