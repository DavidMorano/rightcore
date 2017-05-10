/* MDDRIVER.C - test driver for MD2, MD4 and MD5
 */

/* Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */


#include	<sys/types.h>
#include <time.h>
#include <string.h>
#include <stdio.h>



/* local defines */

#define	VERSION		"0"



/* The following makes MD default to MD5 if it has not already been
  defined with C compiler flags.
 */
#ifndef MD
#define MD	5
#endif

#include "md5_global.h"

#if MD == 2
#include "md2.h"
#endif

#if MD == 4
#include "md4.h"
#endif

#if MD == 5
#include "md5.h"
#endif

/* Length of test block, number of test blocks.
 */
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000

static void MDString PROTO_LIST ((char *)) ;
static void MDTimeTrial PROTO_LIST ((void)) ;
static void MDTestSuite PROTO_LIST ((void)) ;
static void MDFilter PROTO_LIST ((void)) ;
static void MDPrint PROTO_LIST ((unsigned char [16])) ;
static int	MDFile PROTO_LIST ((char *)) ;

#if MD == 2
#define MD_CTX MD2_CTX
#define MDInit MD2Init
#define MDUpdate MD2Update
#define MDFinal MD2Final
#endif
#if MD == 4
#define MD_CTX MD4_CTX
#define MDInit MD4Init
#define MDUpdate MD4Update
#define MDFinal MD4Final
#endif
#if MD == 5
#define MD_CTX MD5_CTX
#define MDInit digestinit
#define MDUpdate digestupdate
#define MDFinal digestfinal
#endif

/* Main driver.

Arguments (may be any combination):
  -sstring - digests string
  -t       - runs time trial
  -x       - runs test script
  filename - digests file
  (none)   - digests standard input
 */



/* local global variables */

int	f_quiet ;
int	f_verbose ;
int	f_version ;
int	f_question ;




int main(argc, argv)
int argc ;
char *argv[] ;
{
	int i ;
	int	rs = 0 ;

	char	*progname ;


	f_quiet = 0 ;
	f_verbose = 0 ;
	f_version = 0 ;
	f_question = 0 ;


	progname = argv[0] ;

	if (argc > 1)

	    for (i = 1; i < argc; i += 1)

	        if (argv[i][0] == '-' && argv[i][1] == 's')
	            MDString (argv[i] + 2) ;

	        else if (strcmp (argv[i], "-V") == 0)
	            f_version = 1 ;

	        else if (strcmp (argv[i], "-?") == 0)
	            f_question = 1 ;

	        else if (strcmp (argv[i], "-q") == 0)
	            f_quiet = 1 ;

	        else if (strcmp (argv[i], "-v") == 0)
	            f_verbose = 1 ;

	        else if (strcmp (argv[i], "-t") == 0)
	            MDTimeTrial () ;

	        else if (strcmp (argv[i], "-x") == 0)
	            MDTestSuite () ;

	        else
	            rs = MDFile(argv[i]) ;

	else
	    MDFilter() ;

	if (f_version) {

		fprintf(stderr,"%s: version %s\n",
			progname,VERSION) ;

	}

	if (f_question) {

		fprintf(stderr,
			"%s: USAGE> %s [-x] [-t] [-v] [-q] [-V] [file]\n",
				progname,progname) ;

	}

/* we are out of here ! */

	fflush(stdout) ;

	fflush(stderr) ;

	return rs ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



/* Digests a string and prints the result.
 */
static void MDString (string)
char *string ;
{
	MD_CTX context ;
	unsigned char digest[16] ;
	unsigned int len = strlen (string) ;

	MDInit (&context) ;
	MDUpdate (&context, string, len) ;
	digestfinal (&context,digest) ;

	printf ("MD%d (\"%s\") = ", MD, string) ;
	MDPrint (digest) ;
	printf ("\n") ;
}

/* Measures the time to digest TEST_BLOCK_COUNT TEST_BLOCK_LEN-byte
  blocks.
 */
static void MDTimeTrial ()
{
	MD_CTX context ;

	time_t endTime, startTime ;
	time_t	elapsed ;

	unsigned char block[TEST_BLOCK_LEN], digest[16] ;
	unsigned int i ;


	fprintf(stderr,
	    "MD%d time trial. Digesting %d %d-byte blocks ...", MD,
	    TEST_BLOCK_LEN, TEST_BLOCK_COUNT) ;

/* Initialize block */
	for (i = 0; i < TEST_BLOCK_LEN; i++)
	    block[i] = (unsigned char)(i & 0xff) ;

/* Start timer */
	(void) time(&startTime) ;

/* Digest blocks */
	MDInit (&context) ;

	for (i = 0; i < TEST_BLOCK_COUNT; i++)
	    MDUpdate (&context, block, TEST_BLOCK_LEN) ;

	digestfinal (&context,digest) ;

/* Stop timer */
	time(&endTime) ;

	fprintf(stderr," done\n") ;

	printf("Digest = ") ;

	MDPrint(digest) ;

	printf("\n") ;

	fprintf(stderr,"Time = %ld seconds\n", (long)(endTime-startTime)) ;

	elapsed = endTime - startTime ;

	if (elapsed == 0)
		fprintf(stderr,"real fast\n") ;

	else
	fprintf(stderr,
	    "Speed = %ld bytes/second\n",
	    (long) TEST_BLOCK_LEN * (long) (TEST_BLOCK_COUNT/elapsed)) ;

}

/* Digests a reference suite of strings and prints the results.
 */
static void MDTestSuite ()
{
	printf ("MD%d test suite:\n", MD) ;


	MDString ("") ;
	MDString ("a") ;
	MDString ("abc") ;
	MDString ("message digest") ;
	MDString ("abcdefghijklmnopqrstuvwxyz") ;
	MDString
	    ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") ;
	MDString
	    ("1234567890123456789012345678901234567890\
1234567890123456789012345678901234567890") ;

}

/* Digests a file and prints the result.
 */
static int MDFile(filename)
char	filename[] ;
{
	FILE *file ;
	MD_CTX context ;
	int len, rs ;
	unsigned char buffer[1024], digest[16] ;


	if ((file = fopen (filename, "rb")) == NULL) {

		rs = -1 ;
	    if (! f_quiet)
	        fprintf(stderr,"md5: file \"%s\" can't be opened\n", filename) ;

	} else {

	    MDInit (&context) ;

	    while (len = fread(buffer, 1, 1024, file))
	        MDUpdate (&context, buffer, len) ;

	    digestfinal(&context,digest) ;

	    fclose(file) ;

		if (f_verbose)
	    printf("MD%d (%s) = ", MD, filename) ;

	    MDPrint(digest) ;

	    printf("\n") ;

		rs = 0 ;
	}

	return rs ;
}

/* Digests the standard input and prints the result.
 */
static void MDFilter ()
{
	MD_CTX context ;
	int len ;
	unsigned char buffer[16], digest[16] ;


	MDInit (&context) ;
	while (len = fread (buffer, 1, 16, stdin))
	    MDUpdate (&context, buffer, len) ;

	digestfinal (&context,digest) ;

	MDPrint (digest) ;
	printf ("\n") ;
}

/* Prints a message digest in hexadecimal.
 */
static void MDPrint (digest)
unsigned char digest[16] ;
{
	unsigned int i ;

	for (i = 0; i < 16; i++)
	    printf ("%02x", digest[i]) ;
}



