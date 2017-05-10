/* ripmisc */



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
|    fatal_error
+---------------------------------------------------------------------*/
void fatal_error(char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}



void cwrite(FILE *out, char *buffer, size_t size)
{
    int result;

    result = fwrite((void*)buffer, size, 1, out);
    if (result == -1) {
        perror("cannot write to output");
        exit(EXIT_FAILURE);
    }
}




