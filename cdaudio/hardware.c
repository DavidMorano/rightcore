/*
 * @(#)hardware.c	1.12	12/17/92
 *
 * Get information about a CD.
 */
static char *ident = "@(#)hardware.c	1.12 12/17/92";

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <ustat.h>
#include <sys/time.h>
#ifdef hpux
#include <sys/scsi.h>
#else /* ! hpux */
# ifdef solbourne
#  include <mfg/dklabel.h>
#  include <mfg/dkio.h>
#  include <dev/srvar.h>
# else  /* ! solbourne */
#  ifdef ultrix
#   include <string.h>
#   include <sys/rzdisk.h>
#   include <sys/cdrom.h>
#  else  /* sun */
#   ifdef SYSV
#    include <sys/cdio.h>
#   else  /* ! SYSV */
#    include <sundev/srreg.h>
#   endif  /* SYSV */
#  endif  /* ultrix */
# endif  /* solbourne */
#endif  /* hpux */
#include "struct.h"

#ifdef hpux

int INTTOBCD(number)	 /* do an integer to bcd conversion */
unsigned char number;
{
	return (number+6*(number/10)) & 0xff ;
}
unsigned char BCDTOINT(number)	 /* do a bcd to integer conversion */
int number;
{
	return (number/16)*10 + number%16;
}
/*#define INTTOBCD(a) ((a / 10) << 4) | (a % 10)*/
/*#define BCDTOINT(a) (((a & 0xF0) >> 4) * 10) + (a & 0x0F)*/
        
union scsi_cd_reply {
        u_char	all[12];	/* reserve 12 bytes for reply */
        struct {
                unsigned playback_status:8;
#define CDAUDIO_SUBQ_PLAY	0x00
#define CDAUDIO_SUBQ_STILL	0x01
#define CDAUDIO_SUBQ_PAUSE	0x02
#define CDAUDIO_SUBQ_DONE	0x03
                unsigned control_data:8;
#define CDAUDIO_SUBQ_PREEMPHASIS	0x01
#define CDAUDIO_SUBQ_COPY_OK		0x02
#define CDAUDIO_SUBQ_DATA_TRK		0x04
#define CDAUDIO_SUBQ_4_CHAN		0x08
                unsigned track:8;
                unsigned index:8;
                unsigned minute:8;
                unsigned second:8;
                unsigned frame:8;
                unsigned abs_minute:8;
                unsigned abs_second:8;
                unsigned abs_frame:8;
        } rdsubq;
        union {
                struct {
                        unsigned first:8;
                        unsigned last:8;
                } track;
                struct {
                        unsigned minute:8;
                        unsigned second:8;
                        unsigned frame:8;
                        unsigned control_data:8;
                } time;
        } rdinfo;
        struct {
                unsigned mode_data:8;
        } rdmode;
} reply_buf;

#define SCSI_TIMEOUT 500

/* CD audio io opcodes */
#define CDAUDIOSTOP	0x1B	/* Stop */
#define CDAUDIOSEARCH	0xC0	/* Search Track */
#define CDAUDIOPLAY	0xC1	/* Play Audio */
#define CDAUDIOSTILL	0xC2	/* Still */
#define CDAUDIOSTOPTIME	0xC3	/* Set Stop Time */
#define CDAUDIOTROPEN	0xC4	/* Tray Open (Caddy eject) */
#define CDAUDIOTRCLOSE	0xC5	/* Tray Close */
#define CDAUDIORDSUBQ	0xC6	/* Read Subcode-Q Data */
#define CDAUDIORDINFO	0xC7	/* Read Disc Information */
#define CDAUDIORDMODE	0xC8	/* Read CD Mode */

/* Types for search and play */
#define CDAUDIO_TYPE_LBA	0x00	/* Logical block address */
#define CDAUDIO_TYPE_ABS	0x01<<6	/* Absolute time */
#define CDAUDIO_TYPE_TRK	0x02<<6	/* Track number */
#define CDAUDIO_TYPE_SAVE	0x03<<6	/* Preserve previous ending address (Play) */

/* Play modes */
#define CDAUDIO_PLAY_MUTE	0x00	/* Muting on */
#define CDAUDIO_PLAY_LEFT	0x01	/* Left channel Mono */
#define CDAUDIO_PLAY_RIGHT	0x02	/* Right channel Mono */
#define CDAUDIO_PLAY_STEREO	0x03	/* Stereo */
#define CDAUDIO_PLAY_SAVE	0x04	/* Preserve previous playback mode */

struct scsi_cmd_parms track_search = {
	10, 1, SCSI_TIMEOUT,
	CDAUDIOSEARCH, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, CDAUDIO_TYPE_ABS
};

struct scsi_cmd_parms play_audio = {
	10, 1, SCSI_TIMEOUT,
	CDAUDIOPLAY, 0x03, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, CDAUDIO_TYPE_ABS
};

/*
 * read_disc_info[4] values
 */
#define CDAUDIO_TOC_TRK		0x00	/* Track number of 1st and last on disc */
#define CDAUDIO_TOC_LEADOUT	0x01
#define CDAUDIO_TOC_CTRL	0x02	/* Starting pt of control */

struct scsi_cmd_parms read_disc_info = {
	10, 1, SCSI_TIMEOUT,
	CDAUDIORDINFO, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00
};

struct scsi_cmd_parms stop = {
	6, 1, SCSI_TIMEOUT,
	CDAUDIOSTOP, 0x00, 0x00, 0x00, 0x00, 0x00
};

struct scsi_cmd_parms playing_status = {
	10, 1, SCSI_TIMEOUT,
	CDAUDIORDSUBQ, 0x0a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00
};

struct scsi_cmd_parms disc_eject = {
	10, 1, SCSI_TIMEOUT,
	CDAUDIOTROPEN, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00
};

struct scsi_cmd_parms still = {
	10, 1, SCSI_TIMEOUT,
	CDAUDIOSTILL, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

#ifdef ultrix
/*
 *   This structure will be filled with the TOC header and all entries.
 * Ultrix doesn't seem to allow getting single TOC entries.
 *                              - Chris Ross (cross@eng.umd.edu)
 */
struct cd_toc_header_and_entries {
	struct cd_toc_header	cdth;
	struct cd_toc_ent		cdte[CDROM_MAX_TRACK+1];
};
#endif /* Ultrix */

void *malloc();
char *strchr();

#ifdef ultrix
# define LINE_MAX_LENGTH	100			/* For reading uerf */

void	find_cdrom();
#endif

extern struct play *playlist;
extern struct cdinfo thiscd, *cd;

int	cd_fd = -1;

/*
 * The minimum volume setting for the Sun and DEC CD-ROMs is 128 but for other
 * machines this might not be the case.
 */
int	min_volume = 128;
int	max_volume = 255;


#ifndef	CDROMDEV
#define	CDROMDEV	"/vol/dev/aliases/cdrom0"
#endif

#ifdef hpux
char    *cd_device = "/dev/rscsi";
#else
#ifdef ultrix
char	*cd_device = NULL;
#else  /* sun */
/* char	*cd_device = "/dev/rdsk/c0t6d0s2"; */
# ifdef SYSV
char	*cd_device = CDROMDEV ;
# else
char	*cd_device = "/dev/rsr0\0";
# endif
#endif
#endif

extern int cur_track, cur_index, cur_lasttrack, cur_firsttrack, cur_pos_abs,	
	cur_frame, cur_pos_rel, cur_tracklen, cur_cdlen, cur_ntracks,	
	cur_nsections, cur_cdmode, cur_listno, cur_stopmode, exit_on_eject,
	cur_balance;
extern char *cur_artist, *cur_cdname, *cur_trackname;
extern char	cur_contd, cur_avoid;

#ifdef ultrix
/*
 * find_cdrom
 *
 * Read through the boot records (via a call to uerf) and find the SCSI
 * address of the CD-ROM.
 */
void
find_cdrom()
{
	char	data[LINE_MAX_LENGTH];
	FILE	*uerf;
	int	fds[2];
	int	pid;

	pipe(fds);

	if ((pid = fork()) == 0) {
		close(fds[0]);
		dup2(fds[1], 1);
		execl("/etc/uerf", "uerf", "-R", "-r", "300", NULL);
		_exit(1);
	} else if (pid < 0) {
		perror("fork");
		exit(1);
	}

	close(fds[1]);
	uerf = fdopen(fds[0], "r");

	while(fgets(data, LINE_MAX_LENGTH, uerf))
		if (strstr(data, "(RRD42)")) {
			char	*device;

			cd_device = (char *)malloc(sizeof("/dev/rrz##c"));
			strcpy(cd_device, "/dev/r");
			device = strstr(data, "rz");
			device[(int)(strchr(device, ' ') - device)] = '\0';
			strcat(cd_device, device);
			strcat(cd_device, "c");
			break;
		}

	fclose(uerf);

	if (cd_device == NULL) {
		fprintf(stderr, "No cdrom (RRD42) is installed on this system");
		exit(1);
	}

	kill(pid, 15);
	(void)wait((int *)NULL);
}
#endif

/*
 * read_toc()
 *
 * Read the table of contents from the CD.  Return a pointer to a cdinfo
 * struct containing the relevant information (minus artist/cdname/etc.)
 * This is a static struct.  Returns NULL if there was an error.
 *
 * XXX allocates one trackinfo too many.
 */
struct cdinfo *
read_toc()
{
	struct playlist		*l;
#ifdef hpux
#else /* ! hpux */
#ifdef ultrix
	struct cd_toc_header	hdr;
	struct cd_toc_ent	entry;
#else /* sun */
	struct cdrom_tochdr	hdr;
	struct cdrom_tocentry	entry;
#endif
#endif
	int			i, pos;

	if (cd_fd < 0)
		return(NULL);

#ifdef hpux
        read_disc_info.command[1] = CDAUDIO_TOC_TRK;
        if ((ioctl(cd_fd, SIOC_SET_CMD, &read_disc_info) < 0) ||
            (read(cd_fd, reply_buf, sizeof(reply_buf)) < 0))
#else
#ifdef ultrix
	if (ioctl(cd_fd, CDROM_TOC_HEADER, &hdr))
#else
	if (ioctl(cd_fd, CDROMREADTOCHDR, &hdr))
#endif
#endif
	{
		perror("readtochdr");
		return (NULL);
	}

	thiscd.artist[0] = thiscd.cdname[0] = '\0';
	thiscd.whichdb = thiscd.otherrc = thiscd.otherdb = NULL;
	thiscd.length = 0;
	thiscd.autoplay = thiscd.playmode = thiscd.volume = 0;
#ifdef hpux
	thiscd.ntracks = BCDTOINT(reply_buf.rdinfo.track.last);
#else
#ifdef ultrix
	thiscd.ntracks = hdr.th_ending_track;
#else
	thiscd.ntracks = hdr.cdth_trk1;
#endif
#endif
	if (thiscd.lists != NULL)
	{
		for (l = thiscd.lists; l->name != NULL; l++)
		{
			free(l->name);
			free(l->list);
		}
		free(thiscd.lists);
		thiscd.lists = NULL;
	}

	if (thiscd.trk != NULL)
		free(thiscd.trk);

	thiscd.trk = malloc((thiscd.ntracks + 1) * sizeof(struct trackinfo));
	if (thiscd.trk == NULL)
	{
		perror("malloc");
		return (NULL);
	}
#ifdef hpux
	for (i = 0; i <= thiscd.ntracks; i++)
	{
		if (i == thiscd.ntracks) {
                        read_disc_info.command[1] = CDAUDIO_TOC_LEADOUT;
			read_disc_info.command[2] = 0;
                }
		else {
                        read_disc_info.command[1] = CDAUDIO_TOC_CTRL;
                        read_disc_info.command[2] = INTTOBCD(i + 1); 
                }
                if ((ioctl(cd_fd, SIOC_SET_CMD, &read_disc_info) < 0) ||
                    (read(cd_fd, reply_buf, sizeof(reply_buf)) < 0))
                {
                        fprintf(stderr, "%s: read_disc_info: track=%d: %s\n",
				cd_device, i+1, strerror(errno));
                        return(NULL);
                }
                
		thiscd.trk[i].data =
                        thiscd.trk[i].avoid =
				reply_buf.rdinfo.time.control_data &
				CDAUDIO_SUBQ_DATA_TRK ? 1 : 0;
		thiscd.trk[i].length = BCDTOINT(reply_buf.rdinfo.time.minute) *
				60 + BCDTOINT(reply_buf.rdinfo.time.second);
		thiscd.trk[i].start = thiscd.trk[i].length * 75 +
				BCDTOINT(reply_buf.rdinfo.time.frame);
		thiscd.trk[i].songname = thiscd.trk[i].otherrc =
		thiscd.trk[i].otherdb = NULL;
		thiscd.trk[i].contd = 0;
		thiscd.trk[i].volume = 0;
		thiscd.trk[i].track = i + 1;
		thiscd.trk[i].section = 0;
        }
#else
#ifdef ultrix
	{
		struct cd_toc				toc;
		struct cd_toc_header_and_entries	toc_buffer;

		bzero((char *)&toc_buffer, sizeof(toc_buffer));
		toc.toc_address_format = CDROM_MSF_FORMAT;
		toc.toc_starting_track = 0;
		toc.toc_alloc_length = (u_short)(((hdr.th_data_len1 << 8) +
					hdr.th_data_len0) & 0xfff) + 2;
		toc.toc_buffer = (caddr_t)&toc_buffer;

		if (ioctl(cd_fd, CDROM_TOC_ENTS, &toc)) {
			perror("toc read");
			return(NULL);
		}

		for (i = 0; i <= thiscd.ntracks; i++)
		{
			bcopy((char *)&toc_buffer.cdte[i], (char *)&entry,
				sizeof(entry));

			thiscd.trk[i].data =
			thiscd.trk[i].avoid = entry.te_control &
				CDROM_DATA_TRACK ? 1 : 0;
			thiscd.trk[i].length = entry.te_absaddr.msf.m_units *
				60 + entry.te_absaddr.msf.s_units;
			thiscd.trk[i].start = thiscd.trk[i].length * 75 +
				entry.te_absaddr.msf.f_units;
			thiscd.trk[i].songname = thiscd.trk[i].otherrc =
			thiscd.trk[i].otherdb = NULL;
			thiscd.trk[i].contd = 0;
			thiscd.trk[i].volume = 0;
			thiscd.trk[i].track = i + 1;
			thiscd.trk[i].section = 0;
		}
	}
#else  /* sun */
	for (i = 0; i <= thiscd.ntracks; i++)
	{
		if (i == thiscd.ntracks)
			entry.cdte_track = CDROM_LEADOUT;
		else
			entry.cdte_track = i + 1;
		entry.cdte_format = CDROM_MSF;
		if (ioctl(cd_fd, CDROMREADTOCENTRY, &entry))
		{
			perror("tocentry read");
			return (NULL);
		}

		thiscd.trk[i].data =
		thiscd.trk[i].avoid = entry.cdte_ctrl & CDROM_DATA_TRACK ?
			1 : 0;
		thiscd.trk[i].length = entry.cdte_addr.msf.minute * 60 +
				entry.cdte_addr.msf.second;
		thiscd.trk[i].start = thiscd.trk[i].length * 75 +
				entry.cdte_addr.msf.frame;
		thiscd.trk[i].songname = thiscd.trk[i].otherrc =
		thiscd.trk[i].otherdb = NULL;
		thiscd.trk[i].contd = 0;
		thiscd.trk[i].volume = 0;
		thiscd.trk[i].track = i + 1;
		thiscd.trk[i].section = 0;
	}
#endif
#endif

/* Now compute actual track lengths. */
	pos = thiscd.trk[0].length;

	for (i = 0; i < thiscd.ntracks; i++)
	{
		thiscd.trk[i].length = thiscd.trk[i+1].length - pos;
		pos = thiscd.trk[i+1].length;
		if (thiscd.trk[i].data)
			thiscd.trk[i].length = (thiscd.trk[i + 1].start -
				thiscd.trk[i].start) * 2;
		if (thiscd.trk[i].avoid)
			strmcpy(&thiscd.trk[i].songname, "DATA TRACK");
	}

	thiscd.length = thiscd.trk[thiscd.ntracks].length;

	return (&thiscd);
}

/*
 * cd_status()
 *
 * Return values:
 *
 *	0	No CD in drive.
 *	1	CD in drive.
 *	2	CD has just been inserted (TOC has been read)
 *	3	CD has just been removed
 *
 * Updates cur_track, cur_pos_rel, cur_pos_abs and other variables.
 */
int
cd_status()
{
	char				realname[MAXPATHLEN];
	static int			warned = 0;
#ifdef hpux
        int                             flag = 1;
#else
#ifdef ultrix
	struct cd_sub_channel		sc;
	struct cd_subc_channel_data	scd;
#else
	struct cdrom_subchnl		sc;
#endif
#endif
	int				ret = 1;

	if (cd_fd < 0)
	{
#ifdef ultrix
		if (cd_device == NULL)
			find_cdrom();
#endif

#ifdef hpux
		if ((cd_fd = open(cd_device, O_RDWR)) < 0)
#else
		if ((cd_fd = open(cd_device, 0)) < 0)
#endif
		{

			if (errno == EACCES)
			{
				if (!warned)
				{
#if     defined(ultrix) || defined(hpux)
					/* Ultrix doesn't have a realpath(3) */
					strcpy(realname, cd_device);
#else
					if (realpath(cd_device, realname) ==
						NULL)
					{
						perror("realpath");
						return (0);
					}
#endif

					fprintf(stderr,
		"As root, please run\n\nchmod 666 %s\n\n%s\n", realname,
		"to give yourself permission to access the CD-ROM device.");
					warned++;
				}
			}
#if defined(ultrix) || defined(hpux)
			else if (errno != EINTR)
#else
			else if (errno != ENXIO)
#endif
			{
				perror(cd_device);
				exit(1);
			}
			return (0);
		}
		cur_cdmode = 5;
#ifdef hpux
                flag = 1;
                if (ioctl(cd_fd, SIOC_CMD_MODE, &flag) < 0)
                {
                        fprintf(stderr, "%s: SIOC_CMD_MODE: true: %s\n",
				cd_device, strerror(errno));
                        /*exit(1);*/
                }
#endif
	}

	if (warned)
	{
		warned = 0;
		fprintf(stderr, "Thank you.\n");
	}

#ifdef hpux
        if ((ioctl(cd_fd, SIOC_SET_CMD, &playing_status) < 0) ||
            (read(cd_fd, reply_buf, sizeof(reply_buf)) < 0))
#else
#ifdef ultrix
	sc.sch_address_format	= CDROM_MSF_FORMAT;
	sc.sch_data_format	= CDROM_CURRENT_POSITION;
	sc.sch_track_number	= 0;
	sc.sch_alloc_length	= sizeof(scd);
	sc.sch_buffer		= (caddr_t)&scd;

	if (ioctl(cd_fd, CDROM_READ_SUBCHANNEL, &sc))
#else
	sc.cdsc_format = CDROM_MSF;

	if (ioctl(cd_fd, CDROMSUBCHNL, &sc))
#endif
#endif
	{
		cur_cdmode = 5;
		cur_track = -1;
		cur_cdlen = cur_tracklen = 1;
		cur_pos_abs = cur_pos_rel = cur_frame = 0;

		if (exit_on_eject)
			exit(0);

		return (0);
	}

	if (cur_cdmode == 5)	/* CD has been ejected */
	{
		cur_pos_rel = cur_pos_abs = cur_frame = 0;

		if ((cd = read_toc()) == NULL)
		{
			close(cd_fd);
			cd_fd = -1;
			if (exit_on_eject)
				exit(0);
			else
				return (0);
		}
		cur_nsections = 0;
		cur_ntracks = cd->ntracks;
		cur_cdlen = cd->length;
		load();
		cur_artist = cd->artist;
		cur_cdname = cd->cdname;
		cur_cdmode = 4;
		ret = 2;
	}

#ifdef hpux
        switch (reply_buf.rdsubq.playback_status) {
        case CDAUDIO_SUBQ_PLAY:
		cur_cdmode = 1;
dopos:
                cur_pos_abs = BCDTOINT(reply_buf.rdsubq.abs_minute) * 60 +
                        BCDTOINT(reply_buf.rdsubq.abs_second);
		cur_frame = cur_pos_abs * 75 +
			BCDTOINT(reply_buf.rdsubq.abs_frame);

		/* Only look up the current track number if necessary. */
		if (cur_track < 1 || cur_frame < cd->trk[cur_track-1].start ||
				cur_frame >= (cur_track >= cur_ntracks ?
				(cur_cdlen + 1) * 75 :
				cd->trk[cur_track].start))
		{
			cur_track = 0;
			while (cur_track < cur_ntracks && cur_frame >=
					cd->trk[cur_track].start)
				cur_track++;
		}
		if (cur_track >= 1 && BCDTOINT(reply_buf.rdsubq.track) >
						cd->trk[cur_track-1].track)
			cur_track++;

		cur_index = BCDTOINT(reply_buf.rdsubq.index);
doall:
		if (cur_track >= 1 && cur_track <= cur_ntracks)
		{
			cur_trackname = cd->trk[cur_track-1].songname;
			cur_avoid = cd->trk[cur_track-1].avoid;
			cur_contd = cd->trk[cur_track-1].contd;
			cur_pos_rel = (cur_frame -
				cd->trk[cur_track-1].start) / 75;
			if (cur_pos_rel < 0)
				cur_pos_rel = -cur_pos_rel;
		}

		if (playlist != NULL && playlist[0].start)
		{
			cur_pos_abs -= cd->trk[playlist[cur_listno-1].
				start - 1].start / 75;
			cur_pos_abs += playlist[cur_listno-1].starttime;
		}
		if (cur_pos_abs < 0)
			cur_pos_abs = cur_frame = 0;

		if (cur_track < 1)
			cur_tracklen = cd->length;
		else
			cur_tracklen = cd->trk[cur_track-1].length;
		break;

	case CDAUDIO_SUBQ_PAUSE:
	case CDAUDIO_SUBQ_STILL:
		if (cur_cdmode == 1 || cur_cdmode == 3)
			cur_cdmode = 3;
		else
			cur_cdmode = 4;
		goto dopos;

	/*
	 * The following status is returned both when the CD is idle and
	 * when the disc stops (there is no "done playing" status.)
	 */
	case CDAUDIO_SUBQ_DONE:
		if (cur_cdmode == 1)
		{
			cur_cdmode = 0;
			break;
		}
		cur_cdmode = 4;
		cur_lasttrack = cur_firsttrack = -1;
		goto doall;

        default:
		cur_cdmode = 0;
                /*printf("Unknown status=%x\n",
			reply_buf.rdsubq.playback_status);*/
		break;
        }
#else
#ifdef ultrix
	switch (scd.scd_header.sh_audio_status) {
	case AS_PLAY_IN_PROGRESS:
		cur_cdmode = 1;
dopos:
		cur_pos_abs = scd.scd_position_data.scp_absaddr.msf.m_units*60+
			scd.scd_position_data.scp_absaddr.msf.s_units;
		cur_frame = cur_pos_abs * 75 +
				scd.scd_position_data.scp_absaddr.msf.f_units;

		/* Only look up the current track number if necessary. */
		if (cur_track < 1 || cur_frame < cd->trk[cur_track-1].start ||
				cur_frame >= (cur_track >= cur_ntracks ?
				(cur_cdlen + 1) * 75 :
				cd->trk[cur_track].start))
		{
			cur_track = 0;
			while (cur_track < cur_ntracks && cur_frame >=
					cd->trk[cur_track].start)
				cur_track++;
		}
		if (cur_track >= 1 && scd.scd_position_data.scp_track_number >
						cd->trk[cur_track-1].track)
			cur_track++;

		cur_index = scd.scd_position_data.scp_index_number;
doall:
		if (cur_track >= 1 && cur_track <= cur_ntracks)
		{
			cur_trackname = cd->trk[cur_track-1].songname;
			cur_avoid = cd->trk[cur_track-1].avoid;
			cur_contd = cd->trk[cur_track-1].contd;
			cur_pos_rel = (cur_frame -
				cd->trk[cur_track-1].start) / 75;
			if (cur_pos_rel < 0)
				cur_pos_rel = -cur_pos_rel;
		}

		if (playlist != NULL && playlist[0].start)
		{
			cur_pos_abs -= cd->trk[playlist[cur_listno-1].
				start - 1].start / 75;
			cur_pos_abs += playlist[cur_listno-1].starttime;
		}
		if (cur_pos_abs < 0)
			cur_pos_abs = cur_frame = 0;

		if (cur_track < 1)
			cur_tracklen = cd->length;
		else
			cur_tracklen = cd->trk[cur_track-1].length;
		break;

	case AS_PLAY_PAUSED:
		if (cur_cdmode == 1 || cur_cdmode == 3)
		{
			cur_cdmode = 3;
			goto dopos;
		}
		else
			cur_cdmode = 4;
		goto doall;

	case AS_PLAY_COMPLETED:
		cur_cdmode = 0;		/* waiting for next track. */
		break;

	case AS_NO_STATUS:
		cur_cdmode = 4;
		cur_lasttrack = cur_firsttrack = -1;
		goto doall;
	}
#else  /* sun */
	switch (sc.cdsc_audiostatus) {
	case CDROM_AUDIO_PLAY:
		cur_cdmode = 1;
dopos:
		cur_pos_abs = sc.cdsc_absaddr.msf.minute * 60 +
			sc.cdsc_absaddr.msf.second;
		cur_frame = cur_pos_abs * 75 + sc.cdsc_absaddr.msf.frame;

		/* Only look up the current track number if necessary. */
		if (cur_track < 1 || cur_frame < cd->trk[cur_track-1].start ||
				cur_frame >= (cur_track >= cur_ntracks ?
				(cur_cdlen + 1) * 75 :
				cd->trk[cur_track].start))
		{
			cur_track = 0;
			while (cur_track < cur_ntracks && cur_frame >=
					cd->trk[cur_track].start)
				cur_track++;
		}
		if (cur_track >= 1 && sc.cdsc_trk > cd->trk[cur_track-1].track)
			cur_track++;

		cur_index = sc.cdsc_ind;
doall:
		if (cur_track >= 1 && cur_track <= cur_ntracks)
		{
			cur_trackname = cd->trk[cur_track-1].songname;
			cur_avoid = cd->trk[cur_track-1].avoid;
			cur_contd = cd->trk[cur_track-1].contd;
			cur_pos_rel = (cur_frame -
				cd->trk[cur_track-1].start) / 75;
			if (cur_pos_rel < 0)
				cur_pos_rel = -cur_pos_rel;
		}

		if (playlist != NULL && playlist[0].start)
		{
			cur_pos_abs -= cd->trk[playlist[cur_listno-1].
				start - 1].start / 75;
			cur_pos_abs += playlist[cur_listno-1].starttime;
		}
		if (cur_pos_abs < 0)
			cur_pos_abs = cur_frame = 0;

		if (cur_track < 1)
			cur_tracklen = cd->length;
		else
			cur_tracklen = cd->trk[cur_track-1].length;
		break;

	case CDROM_AUDIO_PAUSED:
		if (cur_cdmode == 1 || cur_cdmode == 3)
		{
			cur_cdmode = 3;
			goto dopos;
		}
		else
			cur_cdmode = 4;
		goto doall;

	case CDROM_AUDIO_COMPLETED:
		cur_cdmode = 0;		/* waiting for next track. */
		break;

	case CDROM_AUDIO_NO_STATUS:
		cur_cdmode = 4;
		cur_lasttrack = cur_firsttrack = -1;
		goto doall;
	}
#endif  /* ultrix */
#endif  /* hpux */
	return (ret);
}

/*
 * scale_volume(vol, max)
 *
 * Return a volume value suitable for passing to the CD-ROM drive.  "vol"
 * is a volume slider setting; "max" is the slider's maximum value.
 *
 * On Sun and DEC CD-ROM drives, the amount of sound coming out the jack
 * increases much faster toward the top end of the volume scale than it
 * does at the bottom.  To make up for this, we make the volume scale look
 * sort of logarithmic (actually an upside-down inverse square curve) so
 * that the volume value passed to the drive changes less and less as you
 * approach the maximum slider setting.  The actual formula looks like
 *
 *     (max^2 - (max - vol)^2) * (max_volume - min_volume)
 * v = --------------------------------------------------- + min_volume
 *                           max^2
 *
 * If your system's volume settings aren't broken in this way, something
 * like the following should work:
 *
 *	return ((vol * (max_volume - min_volume)) / max + min_volume);
 */
scale_volume(vol, max)
	int	vol, max;
{
	return ((max * max - (max - vol) * (max - vol)) *
		(max_volume - min_volume) / (max * max) + min_volume);
}

/*
 * unscale_volume(cd_vol, max)
 *
 * Given a value between min_volume and max_volume, return the volume slider
 * value needed to achieve that value.
 *
 * Rather than perform floating-point calculations to reverse the above
 * formula, we simply do a binary search of scale_volume()'s return values.
 */
unscale_volume(cd_vol, max)
	int	cd_vol, max;
{
	int	vol, incr, scaled;

	for (vol = max / 2, incr = max / 4 + 1; incr; incr /= 2)
	{
		scaled = scale_volume(vol, max);
		if (cd_vol == scaled)
			break;
		if (cd_vol < scaled)
			vol -= incr;
		else
			vol += incr;
	}
	
	if (vol < 0)
		vol = 0;
	else if (vol > max)
		vol = max;

	return (vol);
}

/*
 * cd_volume(vol, bal, max)
 *
 * Set the volume levels.  "vol" and "bal" are the volume and balance knob
 * settings, respectively.  "max" is the maximum value of the volume knob
 * (the balance knob is assumed to always go from 0 to 20.)
 */
void
cd_volume(vol, bal, max)
	int	vol, bal, max;
{
	int	left, right;
#ifdef hpux
#else
#ifdef ultrix
	struct cd_playback		pb;
	struct cd_playback_status	ps;
	struct cd_playback_control	pc;
#else /* sun */
	struct cdrom_volctrl v;
#endif
#endif

/*
 * Set "left" and "right" to volume-slider values accounting for the
 * balance setting.
 *
 * XXX - the maximum volume setting is assumed to be in the 20-30 range.
 */
	if (bal < 9)
		right = vol - (9 - bal) * 2;
	else
		right = vol;
	if (bal > 11)
		left = vol - (bal - 11) * 2;
	else
		left = vol;

/* Adjust the volume to make up for the CD-ROM drive's weirdness. */
	left = scale_volume(left, max);
	right = scale_volume(right, max);

#ifdef hpux
        /* printf("HPUX cannot adjust volume - sigh.\n"); */
#else
#ifdef ultrix
# if 0
 /*
  * According to the ultrix 4.2 cdrom.h, which defines the below structure,
  * I'd assume that it should be possible to change the volume this way.
  * But, I can't for the life of me figure out how.  (Is there an ioctl
  * for it?  What is it?)
  *                           Chris Ross (cross@eng.umd.edu)
  */
	struct cd_volume_control	v;

	v.vc_channel_0 = left < 0 ? 0 : left > 255 ? 255 : left;
	v.vc_channel_1 = right < 0 ? 0 : right > 255 ? 255 : right;
	if (cd_fd >= 0)
		(void) ioctl(cd_fd, /**/, &v);
# else
 /* Alternative (working) method.  Messy, but functional. */

	bzero((char *)&pb, sizeof(pb));
	bzero((char *)&ps, sizeof(ps));
	bzero((char *)&pc, sizeof(pc));

	pb.pb_alloc_length = sizeof(ps);
	pb.pb_buffer = (caddr_t)&ps;

	if (cd_fd >= 0) {
		if (ioctl(cd_fd, CDROM_PLAYBACK_STATUS, &pb)) {
			perror("playback_status in cd_volume()");
			return;
		}
		pc.pc_chan0_select = ps.ps_chan0_select;
		pc.pc_chan0_volume = (left < CDROM_MIN_VOLUME) ?
			CDROM_MIN_VOLUME : (left > CDROM_MAX_VOLUME) ?
			CDROM_MAX_VOLUME : left;
		pc.pc_chan1_select = ps.ps_chan1_select;
		pc.pc_chan1_volume = (right < CDROM_MIN_VOLUME) ?
			CDROM_MIN_VOLUME : (right > CDROM_MAX_VOLUME) ?
			CDROM_MAX_VOLUME : right;

		pb.pb_alloc_length = sizeof(pc);
		pb.pb_buffer = (caddr_t)&pc;

		(void)ioctl(cd_fd, CDROM_PLAYBACK_CONTROL, &pb);
	}
# endif
#else  /* sun */
	v.channel0 = left < 0 ? 0 : left > 255 ? 255 : left;
	v.channel1 = right < 0 ? 0 : right > 255 ? 255 : right;
	if (cd_fd >= 0)
		(void) ioctl(cd_fd, CDROMVOLCTRL, &v);
#endif  /* ultrix */
#endif  /* hpux */
}

/*
 * pause_cd()
 *
 * Pause the CD, if it's in play mode.  If it's already paused, go back to
 * play mode.
 */
void
pause_cd()
{
	if (cd_fd < 0)	/* do nothing if there's no CD! */
		return;

	switch (cur_cdmode) {
	case 1:		/* playing */
		cur_cdmode = 3;
#ifdef hpux
		ioctl(cd_fd, SIOC_SET_CMD, &still);		/* pause  */
		(void)read(cd_fd, reply_buf, sizeof(reply_buf));
#else
#ifdef ultrix
		ioctl(cd_fd, CDROM_PAUSE_PLAY);
#else
		ioctl(cd_fd, CDROMPAUSE);
#endif
#endif
		break;
	case 3:		/* paused */
		cur_cdmode = 1;
#ifdef hpux
                play_audio.command[9] = CDAUDIO_TYPE_SAVE;
		ioctl(cd_fd, SIOC_SET_CMD, &play_audio);	/* play  */
		(void)read(cd_fd, reply_buf, sizeof(reply_buf));
#else
#ifdef ultrix
		ioctl(cd_fd, CDROM_RESUME_PLAY);
#else
		ioctl(cd_fd, CDROMRESUME);
#endif
#endif
	}
}

/*
 * stop_cd()
 *
 * Stop the CD if it's not already stopped.
 */
void
stop_cd()
{
	if (cd_fd < 0)
		return;

	if (cur_cdmode != 4)
	{
		cur_lasttrack = cur_firsttrack = -1;
		cur_cdmode = 4;
#ifdef hpux
		ioctl(cd_fd, SIOC_SET_CMD, &stop);	/* stop playing */
		(void)read(cd_fd, reply_buf, sizeof(reply_buf));
#else
#ifdef ultrix
		ioctl(cd_fd, SCSI_STOP_UNIT);
#else
		ioctl(cd_fd, CDROMSTOP);
#endif
#endif
		cur_track = 1;
	}
}

/*
 * play_chunk(start, end)
 *
 * Play the CD from one position to another (both in frames.)
 */
void
play_chunk(start, end)
	int start, end;
{
#ifdef hpux
        unsigned char Smin, Ssec, Sframe;
        unsigned char Fmin, Fsec, Fframe;
#else
#ifdef ultrix
	struct cd_play_audio_msf	msf;
#else
	struct cdrom_msf		msf;
#endif
#endif

	if (cd == NULL || cd_fd < 0)
		return;

	end--;
	if (start >= end)
		start = end-1;

#ifdef hpux
        Smin = start / (60*75);
        Ssec = (start % (60*75)) / 75;
        Sframe = start % 75;
        Fmin = end / (60*75);
        Fsec = (end % (60*75)) / 75;
        Fframe = end % 75;

        /*printf("start=%d:%d[%d] end=%d:%d[%d]\n", Smin, Ssec, Sframe,
		Fmin, Fsec, Fframe);*/
        
        track_search.command[1] = 1;
        track_search.command[2] = INTTOBCD(Smin);
        track_search.command[3] = INTTOBCD(Ssec);
        track_search.command[4] = INTTOBCD(Sframe);
        track_search.command[9] = CDAUDIO_TYPE_ABS;

        if ((ioctl(cd_fd, SIOC_SET_CMD, &track_search) < 0) ||
            (read(cd_fd, reply_buf, sizeof(reply_buf)) < 0)) {
                fprintf(stderr, "%s: track_search: %s\n", cd_device,
			strerror(errno));
                return;
        }

        play_audio.command[1] = CDAUDIO_PLAY_STEREO;
        play_audio.command[2] = INTTOBCD(Fmin);
        play_audio.command[3] = INTTOBCD(Fsec);
        play_audio.command[4] = INTTOBCD(Fframe);
        play_audio.command[9] = CDAUDIO_TYPE_ABS;
        
        if ((ioctl(cd_fd, SIOC_SET_CMD, &play_audio) < 0) ||
            (read(cd_fd, reply_buf, sizeof(reply_buf)) < 0)) {
                fprintf(stderr, "%s: play_audio: %s\n", cd_device,
			strerror(errno));
                return;
        }
#else
#ifdef ultrix
	msf.msf_starting_M_unit	= start / (60*75);
	msf.msf_starting_S_unit	= (start % (60*75)) / 75;
	msf.msf_starting_F_unit	= start % 75;
	msf.msf_ending_M_unit	= end / (60*75);
	msf.msf_ending_S_unit	= (end % (60*75)) / 75;
	msf.msf_ending_F_unit	= end % 75;

	if (ioctl(cd_fd, SCSI_START_UNIT))
	{
		perror("SCSI_START_UNIT");
		return;
	}
	if (ioctl(cd_fd, CDROM_PLAY_MSF, &msf))
	{
		printf("play(%d,%d)\n",start,end);
		printf("msf = %d:%d:%d %d:%d:%d\n",
			msf.msf_starting_M_unit, msf.msf_starting_S_unit,
			msf.msf_starting_F_unit, msf.msf_ending_M_unit,
			msf.msf_ending_S_unit, msf.msf_ending_F_unit);
		perror("CDROM_PLAY_MSF");
		return;
	}
#else
	msf.cdmsf_min0 = start / (60*75);
	msf.cdmsf_sec0 = (start % (60*75)) / 75;
	msf.cdmsf_frame0 = start % 75;
	msf.cdmsf_min1 = end / (60*75);
	msf.cdmsf_sec1 = (end % (60*75)) / 75;
	msf.cdmsf_frame1 = end % 75;

	if (ioctl(cd_fd, CDROMSTART))
	{
		perror("CDROMSTART");
		return;
	}
	if (ioctl(cd_fd, CDROMPLAYMSF, &msf))
	{
		printf("play(%d,%d)\n",start,end);
		printf("msf = %d:%d:%d %d:%d:%d\n",
			msf.cdmsf_min0, msf.cdmsf_sec0, msf.cdmsf_frame0,
			msf.cdmsf_min1, msf.cdmsf_sec1, msf.cdmsf_frame1);
		perror("CDROMPLAYMSF");
		return;
	}
#endif  /* ultrix */
#endif  /* hpux */
}

/*
 * play_cd(starttrack, pos, endtrack)
 *
 * Start playing the CD or jump to a new position.  "pos" is in seconds,
 * relative to start of track.
 */
void
play_cd(start, pos, end)
int start, pos, end;
{

	if (cd == NULL || cd_fd < 0)
		return;

	cur_firsttrack = start;
	start--;
	end--;
	cur_lasttrack = end;

	play_chunk(cd->trk[start].start + pos * 75, end >= cur_ntracks ?
		cur_cdlen * 75 : cd->trk[end].start - 1);
}

/*
 * Set the offset into the current track and play.  -1 means end of track
 * (i.e., go to next track.)
 */
void
play_from_pos(pos)
	int	pos;
{
	if (pos == -1)
		if (cd)
			pos = cd->trk[cur_track - 1].length - 1;
	if (cur_cdmode == 1)
		play_cd(cur_track, pos, playlist[cur_listno-1].end);
}

/*
 * Eject the current CD, if there is one, and set the mode to 5.
 *
 * Returns 0 on success, 1 if the CD couldn't be ejected, or 2 if the
 * CD contains a mounted filesystem.
 */
eject_cd()
{
	struct ustat	stbuf;
	struct ustat	ust;

	if (cur_cdmode == 5)		/* Already ejected! */
		return (0);

	if (fstat(cd_fd, &stbuf) != 0)
	{
		perror("fstat");
		return (1);
	}

	/* Is this a mounted filesystem? */
	if (ustat(stbuf.st_rdev, &ust) == 0)
		return (2);

#ifdef hpux
        if ((ioctl(cd_fd, SIOC_SET_CMD, &disc_eject) < 0) ||
            (read(cd_fd, reply_buf, sizeof(reply_buf)) < 0))
        {
#else
#ifdef ultrix
	if (ioctl(cd_fd, CDROM_EJECT_CADDY))
	{
		perror("CDROM_EJECT_CADDY");
#else
	if (ioctl(cd_fd, CDROMEJECT))
	{
		perror("CDEJECT");
#endif
#endif
		return (1);
	}

	if (exit_on_eject)
		exit(0);

	cur_track = -1;
	cur_cdlen = cur_tracklen = 1;
	cur_pos_abs = cur_pos_rel = cur_frame = 0;
	cur_cdmode = 5;

	return (0);
}

/* Try to keep the CD open all the time.  This is run in a subprocess. */
void
keep_cd_open()
{
	int	fd;
	struct flock	fl;
	extern	end;

	for (fd = 0; fd < 256; fd++)
		close(fd);

	if (fork())
		exit(0);

	if ((fd = open("/tmp/cd.lock", O_RDWR | O_CREAT, 0666)) < 0)
		exit(0);
	fl.l_type = F_WRLCK;
	fl.l_whence = 0;
	fl.l_start = 0;
	fl.l_len = 0;
	if (fcntl(fd, F_SETLK, &fl) < 0)
		exit(0);

	if (open(cd_device, 0) >= 0)
	{
		brk(&end);
		pause();
	}

	exit(0);
}

/*
 * find_trkind(track, index)
 *
 * Start playing at a particular track and index, optionally using a particular
 * frame as a starting position.  Returns a frame number near the start of the
 * index mark if successful, 0 if the track/index didn't exist.
 *
 * This is made significantly more tedious (though probably easier to port)
 * by the fact that CDROMPLAYTRKIND doesn't work as advertised.  The routine
 * does a binary search of the track, terminating when the interval gets to
 * around 10 frames or when the next track is encountered, at which point
 * it's a fair bet the index in question doesn't exist.
 */
find_trkind(track, index, start)
	int	track, index, start;
{
	int	top = 0, bottom, current, interval, ret = 0, i;

	if (cd == NULL || cd_fd < 0)
		return;

	for (i = 0; i < cur_ntracks; i++)
		if (cd->trk[i].track == track)
			break;
	bottom = cd->trk[i].start;

	for (; i < cur_ntracks; i++)
		if (cd->trk[i].track > track)
			break;

	top = i == cur_ntracks ? (cd->length - 1) * 75 : cd->trk[i].start;

	if (start > bottom && start < top)
		bottom = start;

	current = (top + bottom) / 2;
	interval = (top - bottom) / 4;

	do {
		play_chunk(current, current + 75);

		if (cd_status() != 1)
			return (0);
		while (cur_frame < current)
			if (cd_status() != 1 || cur_cdmode != 1)
				return (0);
			else
				susleep(1);

		if (cd->trk[cur_track - 1].track > track)
			break;

		if (cur_index >= index)
		{
			ret = current;
			current -= interval;
		}
		else
			current += interval;
		interval /= 2;
	} while (interval > 2);

	return (ret);
}

/*
 * Simulate usleep() using select().
 */
susleep(usec)
	int	usec;
{
	struct timeval	tv;

	timerclear(&tv);
	tv.tv_sec = usec / 1000000;
	tv.tv_usec = usec % 1000000;
	return (select(0, NULL, NULL, NULL, &tv));
}

/*
 * Read the initial volume from the drive, if available.  Set cur_balance to
 * the balance level (0-20, 10=centered) and return the proper setting for
 * the volume knob.
 *
 * "max" is the maximum value of the volume knob.
 */
read_initial_volume(max)
	int max;
{
	int	left, right;
#ifdef hpux
#else
#ifdef ultrix
	struct cd_playback		pb;
	struct cd_playback_status	ps;
#endif
#endif

#ifdef hpux
	/* Snakes can't read the volume; oh well */
	left = right = 255;
#else
#ifdef ultrix
	bzero((char *)&pb, sizeof(pb));
	bzero((char *)&ps, sizeof(ps));

	pb.pb_alloc_length = sizeof(ps);
	pb.pb_buffer = (caddr_t)&ps;

	if (cd_fd >= 0) {
		if (ioctl(cd_fd, CDROM_PLAYBACK_STATUS, &pb)) {
			perror("playback_status in read_initial_volume()");
			return;
		}
		left = ps.ps_chan0_volume;
		right = ps.ps_chan1_volume;
	}
	else
		left = right = CDROM_MAX_VOLUME;
#else /* sun */
	/* Suns can't read the volume; oh well */
	left = right = 255;
#endif
#endif

	left = unscale_volume(left, max);
	right = unscale_volume(right, max);

	if (left < right)
	{
		cur_balance = (right - left) / 2 + 11;
		if (cur_balance > 20)
			cur_balance = 20;

		return (right);
	}
	else if (left == right)
	{
		cur_balance = 10;
		return (left);
	}
	else
	{
		cur_balance = (right - left) / 2 + 9;
		if (cur_balance < 0)
			cur_balance = 0;

		return (left);
	}
}



