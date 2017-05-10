/* defs */


#include	<sys/types.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>
#include	<signal.h>
#include	<time.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<userinfo.h>
#include	<pcsconf.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



struct global	g ;

struct pcsconf	p ;

struct userinfo	u ;



