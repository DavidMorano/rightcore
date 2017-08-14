/* main (ripper) */


#include	<envstandards.h>

#include <sys/types.h>
#include <sys/cdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <assert.h>
#include <volmgt.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include <stdio.h>

#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* global data */

struct options	Options ;


/* forward references */

void parse_command_line();
void print_usage_and_exit() ;




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

                      case OUTPUT_AIF:
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


/*----------------------------------------------------------------------
|    parse_command_line
+---------------------------------------------------------------------*/
void parse_command_line(char **argv)
{
    char *arg;

    while((arg = *argv++)) {
        char c;
        if (*arg == '-') {
            /* this is an option */
            switch(c = *++arg) {
              case 'h': 
			print_usage_and_exit(); break;

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
                      Options.format = OUTPUT_AIF;
                  } else if (!strcasecmp("AIFF", format)) {
                      Options.format = OUTPUT_AIF;
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
    if (Options.format == OUTPUT_AIF || Options.format == OUTPUT_SUN) {
        Options.xinu = !Options.xinu;
    }
    if (Options.track == 0) Options.track = 1;
}



/*----------------------------------------------------------------------
|    print_usage_and_exit
+---------------------------------------------------------------------*/
void print_usage_and_exit()
{
    fprintf(stderr,
		"ripper [options] [<filename> (default = stdout)]\n"
            "    Version " PROGVERSION "\n"
            "    options are:\n"
            "    -h       : prints this usage information\n"
            "    -V       : prints the program version number\n"
            "    -v       : verbose\n"
            "    -i       : print CD-ROM info and exit\n"
            "    -t <n>   : track number (default = 1)\n"
            "    -a       : dump all tracks, one by one (filename is used as a name pattern)\n"
            "    -f {WAV,AIF,PCM,SUN}\n"
            "             : output format (default SUN)\n"
            "    -b       : do not try to set the block size to 2352 bytes\n"
            "    -x       : swap bytes in 16bits samples\n"
            "    -e       : exit if jitter control fails (exit status = 2)\n"
            "    -p       : show progress\n"
            "    -d <name>: CD-ROM device name\n"
            );

}
/* end if (usage) */




