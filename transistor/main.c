/* last modified %G% version %I% */

/* sort transistors by net name connections */

/*
	Don Shugard
	September 1993
*/


#include <stdio.h>
#include <ctype.h>


#define FALSE 0
#define TRUE 1

#define DEBUG FALSE
#define DEBUG1 TRUE

#define MIN(a,b) ( ( (a) < (b) ) ? (a):(b) )
#define MAX(a,b) ( ( (a) > (b) ) ? (a):(b) )
#define CANNON(a,b) if( (a) > (b) ){ double tmp; tmp=(a); (a)=(b); (b)=tmp; }
#define BETWEEN(l,m,u) ( (m) >= (l) && (m) <= (u) )
#define INRANGE( a1, a2, b1, b2 ) ( (a1) <= (b2) && (a2) > (b2) )
#define FUNCTION
#define FPR (void)fprintf
#define INF 99999999
#define BUFFER_SIZE 256
#define GATE_SCALE 10000

typedef char BOOLEAN;

FILE *ofp = stdout;
FILE *ifp = stdin;

struct tran {
	char name[32];
	char drain[32];
	char gate[32];
	char source[32];
	char back[32];
	char model[32];
	char width[32];
	BOOLEAN printed;
} trans[256];
int num_trans;

extern double atof();
extern double strtod();
extern char *optarg;
extern int optind;

static void FUNCTION usage(progname)
char *progname;
{
	FPR(stderr,"%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\n",
	    progname,
	    "<-a <impedance/current accurracy>>",
	    "<-d <source to drain voltage>>",
	    "<-g <source to gate voltage>>",
	    "<[-i <current>][-z <impedance>]>",
	    "<-s <step voltage>>",
	    "files ...");
	FPR(stderr,"all option arguments are floating point numbers\n\n");
	FPR(stderr,"advice output files must have these variables in\n");
	FPR(stderr,"this order in the first 4 columns:\n");
	FPR(stderr,"TIME ISD VSG VSD\n");
}

static char * FUNCTION get_var(s, var)
char * s;
char ** var;
{
	static char buf[BUFFER_SIZE];
	char *p;

	while( isspace(*s) ) s++;
	p=buf;
	while(*s!=' ' && *s!='\n' ){
	    *p++ = *s++;
	}
	*p = '\0';
	*var = buf;
	return s;
}

static void FUNCTION process(filename)
char * filename;
{
	char line[BUFFER_SIZE];
	char * s;
	char *p;
	FILE *ifp;

	if( filename==NULL){
	    ifp = stdin;
	    filename = "STDIN";
	}
	else {
	    ifp = fopen(filename,"r");
	}
	if( ifp == NULL ){
	    FPR(stderr,"could not open file %s\n",filename);
	    return;
	}

	num_trans = 0;
	while( (s=fgets(line,BUFFER_SIZE,ifp)) != NULL ){
	    s = get_var(s, &p);
	    (void)strcpy(trans[num_trans].name,p);
	    s = get_var(s, &p);
	    (void)strcpy(trans[num_trans].drain,p);
	    s = get_var(s, &p);
	    (void)strcpy(trans[num_trans].gate,p);
	    s = get_var(s, &p);
	    (void)strcpy(trans[num_trans].source,p);
	    s = get_var(s, &p);
	    (void)strcpy(trans[num_trans].back,p);
	    s = get_var(s, &p);
	    (void)strcpy(trans[num_trans].model,p);
/* swallow SW */
	    s = get_var(s, &p);
/* swallow = */
	    s = get_var(s, &p);
	    s = get_var(s, &p);
	    (void)strcpy(trans[num_trans].width,p);
	    num_trans++;
	}
	(void)fflush(ofp);
	(void)fclose(ifp);
}

int FUNCTION tsort( t1, t2 )
struct tran *t1,*t2;
{
	int n;

	n=strcasecmp(t1->gate, t2->gate);
	if( n == 0 ){
	    n=strcasecmp(t1->drain, t2->drain);
	    if( n == 0 ){
	        n=strcasecmp(t1->source, t2->source);
	    }
	    return n;
	}
	return n;
}

main(argc,argv)
int argc;
char *argv[];
{
	int i;
	int j;
	char c;

	while( (c=getopt(argc, argv, "?o:")) != -1 ){
	    switch(c){
	    case 'o':
	        ofp = fopen(optarg,"w");
	        if( ofp == NULL ){
	            FPR(stderr,
	                "could not open output file %s\n",
	                optarg);
	            exit(1);
	        }
	        break;
	    case '?':
	        usage(argv[0]);
	        exit(1);
	    }
	}

#if DEBUG
	FPR(stdout,"a=%g g=%d i=%g s=%g z=%g\n",acc, vsg, cur, step, imp);
#endif

	if( optind >= argc ){
	    process((char *) NULL);
	}
	else {
	    for(; optind  < argc; optind++) {
	        process(argv[optind]);
	    }
	}

	qsort( trans, num_trans, sizeof( struct tran) , tsort);
	for(i=0; i<num_trans; i++){
	    if( trans[i].printed ){
	        continue;
	    }
	    fprintf(ofp,"%s %s %s %s %s SW = %s\n",
	        trans[i].name,
	        trans[i].drain,
	        trans[i].gate,
	        trans[i].source,
	        trans[i].back,
	        trans[i].width);
	    trans[i].printed = TRUE;
	    for( j=i+1; j<num_trans; j++){
	        if( strcmp(trans[i].gate, trans[j].gate) != 0 ){
	            break;
	        }

	        if( (strcmp(trans[i].drain, trans[j].drain) == 0 &&
	            strcmp(trans[i].source, trans[j].source) == 0 ) ||
	            (strcmp(trans[i].drain, trans[j].source) == 0 &&
	            strcmp(trans[i].source, trans[j].drain) == 0 )  ){
	            trans[j].printed = TRUE;
	            fprintf(ofp,"%s %s %s %s %s SW = %s\n",
	                trans[j].name,
	                trans[j].drain,
	                trans[j].gate,
	                trans[j].source,
	                trans[j].back,
	                trans[j].width);
	        }
	    }
	}
	(void)fclose(ofp);
	return 0;
}


