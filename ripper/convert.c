/* convert */



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




/*----------------------------------------------------------------------
|    le_convert_short
+---------------------------------------------------------------------*/
void le_convert_short(char *buffer, unsigned short s)
{
    unsigned char *b = (unsigned char *)buffer;
    b[0] = s & 0xFF;
    b[1] = s >> 8;
}

/*----------------------------------------------------------------------
|    le_convert_long
+---------------------------------------------------------------------*/
void le_convert_long(char *buffer, unsigned long l)
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
void be_convert_short(char *buffer, unsigned short s)
{
    unsigned char *b = (unsigned char *)buffer;
    b[1] = s & 0xFF;
    b[0] = s >> 8;
}

/*----------------------------------------------------------------------
|    be_convert_long
+---------------------------------------------------------------------*/
void be_convert_long(char *buffer, unsigned long l)
{
    unsigned char *b = (unsigned char *)buffer;
    b[3] = l & 0xFF; l >>= 8;
    b[2] = l & 0xFF; l >>= 8;
    b[1] = l & 0xFF; l >>= 8;
    b[0] = l & 0xFF;
}



/*----------------------------------------------------------------------
|    msf_to_frames
+---------------------------------------------------------------------*/
int msf_to_frames(Msf msf)
{
    return 
        msf.minute * FRAMES_PER_SECOND * 60 +
        msf.second * FRAMES_PER_SECOND +
        msf.frame;
}


/*----------------------------------------------------------------------
|    checksum
+---------------------------------------------------------------------*/
unsigned long checksum(unsigned long val)
{
    char ascii[128];
    char *s;
    int sum = 0;
    
    sprintf(ascii, "%lu", val);
    for (s = ascii; *s != '\0'; s++) sum += (*s - '0');

    return sum;
}



