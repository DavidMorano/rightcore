/* encdec */


#define	CF_DEBUGS	0


/*
 *	Written by Jörgen Hägg 1993 <jh@efd.lth.se>
 *	Version 1.1
 *
 *	(This filter is written for use in a MTA written in perl.)
 *	
 *	Please send comments and bugfixes when you find them.
 *	Permission to use and change this program is given for any purpose
 *	as long as this note remains unchanged.
 *	
 *	The usage() is the manual.
 *	Use encdec as you wish :-)
 *
 *  Ported to DOS 23 (BC++ 2.0) Oct 1993 by:
 *  EM == Enzo Michelangeli - enzo@air.org
 *  - written a (hopefully bug-free) getopt()
 *  - attempted to deal with the hairy DOS' text/bin mode problem (stdin/out
 *    are pre-opened in text mode by default).
 */

#include <stdio.h>
#ifdef BSD
#include <strings.h>
#else
#include <string.h>
#endif
#include <ctype.h>
#include <fcntl.h>



char	vec[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=" ;


void	encode_base64();
void	decode_base64();
void	encode_quoted();
void	decode_quoted();


extern	char	*optarg;
extern	int	optind;



int main(int argc, char *argv[])
{
	int		c;
	int		encoding = 0, decoding = 0, base64 = 0, quoted = 0;


#ifdef MSDOS
	msdos_prdebugopen();
#endif

	while ((c = getopt(argc, argv,	"edbq")) != -1)

	    switch (c) {

	    case 'e':
	        encoding++;
	        break;
	    case 'd':
	        decoding++;
	        break;
	    case 'b':
	        base64++;
	        break;
	    case 'q':
	        quoted++;
	        break;
	    case '?':
	        usage();
	    }

	if (encoding && base64)
	    encode_base64();

	else if (encoding && quoted)
	    encode_quoted();

	else if (decoding && base64)
	    decode_base64();

	else if (decoding && quoted)
	    decode_quoted();

	else
	    usage();

	return 0 ;
}
/* end subroutine (main) */


usage()
{


	printf("encdec: usage: encdec -e/-d -b/-q\n");
	printf("Encdec encodes or decodes stdin\n");
	printf("and prints the result to stdout.\n");
	printf("\t-e\tencode\n");
	printf("\t-d\tdecode\n");
	printf("\t-b\tuse base64\n");
	printf("\t-q\tuse quoted-printable\n");
	exit(1);
}


void	encode_base64()
{
	int	c,
	n = 0,
	p,
	i,
	count = 0;
	long	val = 0;
	char	enc[4];


#if	CF_DEBUGS
	debugprintf("encode_base64: entered\n") ;
#endif

	while ((c = getc(stdin)) != EOF) {

	    if (n++ <= 2)
	    {
	        val <<= 8;
	        val += c;
	        continue;
	    }

	    for (i = 0; i < 4; i++) {

	        enc[i] = val & 63;
	        val >>= 6;
	    }

	    for (i = 3; i >= 0; i--)
	        putchar(vec[enc[i]]);

	    n = 1;
	    count += 4;
	    val = c;

	    if (count >= 76) {

	        printf("\n");
	        count = 0;
	    }
	}

	if (n == 1) {

	    val <<= 16;
	    for (i = 0; i < 4; i++) {

	        enc[i] = val & 63;
	        val >>= 6;
	    }
	    enc[0] = enc[1] = 64;

	}

	if (n == 2) {

	    val <<= 8;
	    for (i = 0; i < 4; i++) {

	        enc[i] = val & 63;
	        val >>= 6;
	    }
	    enc[0] = 64;
	}

	if (n == 3) for (i = 0; i < 4; i++) {

	    enc[i] = val & 63;
	    val >>= 6;
	}

	if (n) {

	    for (i = 3; i >= 0; i--)
	        putchar(vec[enc[i]]);

	}

	printf("\n");

}


void	decode_base64()
{
	int	i, num, len, j;
	long	d, val;
	char	nw[4], buf[81], *p, *c;


	while (fgets(buf, 80, stdin)) {

	    len = strlen(buf)-1;
	    for (i = 0; i < len; i += 4)
	    {
	        val = 0;
	        num = 3;
	        c = buf+i;
	        if (c[2] == '=')
	            num = 1;
	        else if (c[3] == '=')
	            num = 2;

	        for (j = 0; j <= num; j++)
	        {
	            if (!(p = strchr(vec, c[j])))
	            {
	                fprintf(stderr, "encdec: %s not in "
	                    "base64\n", buf);
	                exit(1);
	            }
	            d = p-vec;
	            d <<= (3-j)*6;
	            val += d;
	        }
	        for (j = 2; j >= 0; j--)
	        {
	            nw[j] = val & 255;
	            val >>= 8;
	        }
	        fwrite(nw, 1, num, stdout);
	    }
	}
}

#define	MAX	512

void	encode_quoted()
{
	int		count = 0;
	unsigned char	buf[MAX], *p;


	while (fgets((char *) buf, MAX-2, stdin)) {

	    buf[strlen((char *) buf) - 1] = '\0';

	    for (p = buf; *p; p++)
	    {
	        if (count > 73)
	        {
	            printf("=\n");
	            count = 0;
	        }
	        if (*p == 10)
	        {
	            printf("\n");
	            count = 0;
	        }
	        else if (*p == 9 || *p == 32)
	        {
	            if (!*(p+1))
	            {
	                printf("=%02X", *p);
	                count += 3;
	            }
	            else
	            {
	                putchar(*p);
	                count++;
	            }
	            continue;
	        }
	        else if ((*p >= 33 && *p <= 60) ||
	            (*p >= 62 && *p <= 126))
	        {
	            putchar(*p);
	            count++;
	        }
	        else
	        {
	            printf("=%02X", *p);
	            count += 3;
	        }

	    }
	    printf("\n");
	    count = 0;
	}
}

int hex(int x)
{
	return (isdigit(x) ? (x-'0') : isxdigit(x) ? ((x & 0x5f) - 'A')+10 : 0);
}

void	decode_quoted()
{
	int		x1, x2,
	count = 0,
	soft_line_break = 0;
	unsigned char	buf[MAX], *p;


	while (fgets((char *) buf, MAX-2, stdin)) {

	    buf[strlen((char *) buf)-1] = '\0';

	    soft_line_break = 0;
	    for (p = buf; *p; p++)
	    {
	        if (*p == 10)
	            putchar(*p);
	        else if (*p == '=')
	        {
	            if (!*(p+1))
	            {
	                soft_line_break++;
	                continue;
	            }
	            p++;
	            x1 = hex(*p++);
	            x2 = hex(*p);
	            putchar(x1*16+x2);
	        }
	        else
	            putchar(*p);
	    }
	    if (!soft_line_break)
	        putchar('\n');
	}
}

#ifdef MSDOS
/* --- Begin EM ----- */
void	msdos_prdebugopen()
{
	if((stdout->flags & _F_TERM) == 0)	/* if it's a file */
	{
/*		fprintf(stderr, "stdout to a file\n");	*/
	    if(setmode(fileno(stdout), O_BINARY) != 0)
	    {
	        fprintf(stderr, "can't set stdout's mode to binary\n");
	        exit(2);
	    }
	}
	if((stdin->flags & _F_TERM) == 0)	/* if it's a file */
	{
/*		fprintf(stderr, "stdin from a file\n");	*/
	    if(setmode(fileno(stdin), O_BINARY) != 0)
	    {
	        fprintf(stderr, "can't set stdin's mode to binary\n");
	        exit(2);
	    }
	}
}

int optind = 0;
char *optarg = "";
getopt(int argc, char *argv[],	char *opts)	/* not the real thing... */
{
	int c;
	static char *p = "";
	char *optr;
	for(;;)
	{
	    if(*p == 0)
	        p = NULL;
	    if(p == NULL)
	    {
	        if(++optind >= argc)
	            return -1;
	        p = argv[optind];
	        if(*p++ != '-')
	        {
	            p = NULL;
	            continue;
	        }
	    }

	    c = *p++;
	    if(c == '-')
	    {
	        optind = argc;
	        return -1;
	    }
	    optr = strchr(opts, c);	/* pointer in opts */
	    if(optr == NULL)
	    {
	        fprintf(stderr, "illegal option `%c' found\n", c);
	        return '?';
	    }
	    if(optr[1] == ':')	/* if it may take arguments */
	    {
	        char *pp = p;
	        while(isspace(*pp))
	            pp++;
	        optarg = pp;	/* skip leading spaces */
	    }
	    else
	        optarg = "";
	    return c;
	}

}
/* --- End   EM ----- */
#endif /* MSDOS */
