#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# ifndef YYLMAX 
# define YYLMAX BUFSIZ
# endif 
#ifndef __cplusplus
# define output(c) (void)putc(c,yyout)
#else
# define lex_output(c) (void)putc(c,yyout)
#endif

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
	int yyback(int *, int);
	int yyinput(void);
	int yylook(void);
	void yyoutput(int);
	int yyracc(int);
	int yyreject(void);
	void yyunput(int);
	int yylex(void);
#ifdef YYLEX_E
	void yywoutput(wchar_t);
	wchar_t yywinput(void);
#endif
#ifndef yyless
	int yyless(int);
#endif
#ifndef yywrap
	int yywrap(void);
#endif
#ifdef LEXDEBUG
	void allprint(char);
	void sprint(char *);
#endif
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void exit(int);
#ifdef __cplusplus
}
#endif

#endif
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
#ifndef __cplusplus
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#else
# define lex_input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#endif
#define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng;
#define YYISARRAY
char yytext[YYLMAX];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;


# line 4 "test.l"
/* comment */

int		num ;



# line 9 "test.l"
/* forward references */

static int	clean(char *,int) ;


# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
#ifdef __cplusplus
/* to avoid CC and lint complaining yyfussy not being used ...*/
static int __lex_hack = 0;
if (__lex_hack) goto yyfussy;
#endif
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:

# line 22 "test.l"
			{

					printf("PAR\n") ;
				}
break;
case 2:

# line 27 "test.l"
			{

					printf("PAR_END\n") ;
				}
break;
case 3:

# line 32 "test.l"
			{

					printf("PAR\n") ;
				}
break;
case 4:

# line 37 "test.l"
			{

					printf("PAR_END\n") ;
				}
break;
case 5:

# line 43 "test.l"
	{ 
					int	sl ;


					sl = clean(yytext,-1) ;

					fwrite(yytext,1,sl,stdout) ;

				}
break;
case 6:

# line 54 "test.l"
			{ printf("%d\n",1) ; }
break;
case 7:

# line 55 "test.l"
			{ printf("%d\n",2) ; }
break;
case 8:

# line 56 "test.l"
			{ printf("%d\n",3) ; }
break;
case 9:

# line 57 "test.l"
			{ printf("%d\n",4) ; }
break;
case 10:

# line 58 "test.l"
			{ printf("%d\n",5) ; }
break;
case 11:

# line 59 "test.l"
			{ printf("%d\n",6) ; }
break;
case 12:

# line 60 "test.l"
			{ printf("%d\n",7) ; }
break;
case 13:

# line 61 "test.l"
			{ printf("%d\n",8) ; }
break;
case 14:

# line 62 "test.l"
			{ printf("%d\n",9) ; }
break;
case 15:

# line 63 "test.l"
			{ printf("%d\n",10) ; }
break;
case 16:

# line 64 "test.l"
			{ printf("%d\n",11) ; }
break;
case 17:

# line 66 "test.l"
		{ 
					printf("text> %s\n", yytext) ;
				}
break;
case 18:

# line 70 "test.l"
			{ printf("%d\n",13) ; }
break;
case 19:

# line 71 "test.l"
			{ printf("%d\n",14) ; }
break;
case 20:

# line 72 "test.l"
			{ printf("%d\n",15) ; }
break;
case 21:

# line 74 "test.l"
		{ 
					printf("string> %s\n", yytext) ;
				}
break;
case 22:

# line 78 "test.l"
		{ 
					printf("regexp> %s\n", yytext) ;
				}
break;
case 23:

# line 82 "test.l"
		{ 
					printf("number> %s\n", yytext) ;
				}
break;
case 24:

# line 86 "test.l"
		{;}
break;
case 25:

# line 88 "test.l"
			{;}
break;
case 26:

# line 90 "test.l"
			{
					printf("DOT\n") ;
				}
break;
case -1:
break;
default:
(void)fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

# line 95 "test.l"


int main()
{


	yylex() ;

	printf("done\n") ;

	fclose(stdout) ;

	return 0 ;
}


static int clean(buf,len)
char	*buf ;
int	len ;
{
	int	i ;


	if (len < 0)
		len = 100 ;

	for (i = 0 ; (i < len) && (buf[i] != '\0') ; i += 1) {

		if (buf[i] == '?')
			buf[i] = '\'' ;

	}

	return i ;
}


int yyvstop[] = {
0,

26,
0, 

25,
26,
0, 

25,
0, 

18,
26,
0, 

26,
0, 

26,
0, 

8,
26,
0, 

9,
26,
0, 

16,
26,
0, 

13,
26,
0, 

14,
26,
0, 

17,
26,
0, 

26,
0, 

23,
26,
0, 

12,
26,
0, 

26,
0, 

10,
26,
0, 

17,
26,
0, 

20,
26,
0, 

6,
26,
0, 

15,
26,
0, 

7,
26,
0, 

11,
26,
0, 

19,
0, 

21,
0, 

24,
0, 

17,
0, 

22,
0, 

23,
0, 

17,
0, 

21,
0, 

22,
0, 

3,
0, 

1,
0, 

5,
0, 

4,
0, 

2,
0, 
0};
# define YYTYPE unsigned char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	7,27,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	7,27,	7,27,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,6,	1,7,	
1,8,	6,26,	0,0,	0,0,	
0,0,	1,9,	1,10,	1,11,	
1,12,	29,42,	1,13,	1,14,	
1,15,	1,16,	0,0,	7,28,	
18,37,	0,0,	0,0,	35,43,	
0,0,	0,0,	0,0,	1,17,	
0,0,	1,18,	1,19,	7,27,	
7,27,	7,27,	1,20,	38,46,	
39,47,	2,6,	44,49,	2,8,	
45,50,	0,0,	0,0,	8,30,	
2,9,	2,10,	2,11,	2,12,	
0,0,	2,13,	7,27,	8,30,	
8,31,	18,38,	37,44,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	27,29,	2,17,	1,21,	
2,18,	2,19,	1,20,	33,35,	
35,35,	42,29,	43,35,	29,29,	
0,0,	0,0,	0,0,	0,0,	
8,30,	7,29,	0,0,	0,0,	
0,0,	0,0,	7,27,	0,0,	
0,0,	18,39,	37,45,	0,0,	
8,30,	8,30,	8,30,	0,0,	
1,22,	1,23,	1,24,	1,25,	
0,0,	0,0,	2,21,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	8,30,	
16,36,	16,36,	16,36,	16,36,	
16,36,	16,36,	16,36,	16,36,	
16,36,	16,36,	0,0,	0,0,	
0,0,	14,32,	0,0,	0,0,	
0,0,	0,0,	0,0,	2,22,	
2,23,	2,24,	2,25,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	8,30,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	14,32,	14,32,	
14,32,	14,32,	15,33,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	15,33,	15,33,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	15,33,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	15,33,	
15,34,	15,33,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	15,33,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
20,32,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	15,35,	0,0,	0,0,	
0,0,	20,40,	15,33,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	20,41,	20,41,	20,41,	
20,41,	40,48,	40,48,	40,48,	
40,48,	40,48,	40,48,	40,48,	
40,48,	40,48,	40,48,	40,48,	
40,48,	40,48,	40,48,	40,48,	
40,48,	40,48,	40,48,	40,48,	
40,48,	40,48,	40,48,	40,48,	
40,48,	40,48,	40,48,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-36,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+4,	0,		yyvstop+8,
yycrank+-17,	0,		yyvstop+11,
yycrank+-74,	0,		yyvstop+13,
yycrank+0,	0,		yyvstop+15,
yycrank+0,	0,		yyvstop+18,
yycrank+0,	0,		yyvstop+21,
yycrank+0,	0,		yyvstop+24,
yycrank+0,	0,		yyvstop+27,
yycrank+107,	0,		yyvstop+30,
yycrank+-229,	0,		yyvstop+33,
yycrank+92,	0,		yyvstop+35,
yycrank+0,	0,		yyvstop+38,
yycrank+5,	0,		yyvstop+41,
yycrank+0,	0,		yyvstop+43,
yycrank+262,	0,		yyvstop+46,
yycrank+0,	0,		yyvstop+49,
yycrank+0,	0,		yyvstop+52,
yycrank+0,	0,		yyvstop+55,
yycrank+0,	0,		yyvstop+58,
yycrank+0,	0,		yyvstop+61,
yycrank+0,	0,		yyvstop+64,
yycrank+-1,	yysvec+7,	0,	
yycrank+0,	0,		yyvstop+66,
yycrank+-11,	yysvec+7,	0,	
yycrank+0,	yysvec+8,	0,	
yycrank+0,	0,		yyvstop+68,
yycrank+0,	yysvec+14,	yyvstop+70,
yycrank+-7,	yysvec+15,	0,	
yycrank+0,	0,		yyvstop+72,
yycrank+-8,	yysvec+15,	0,	
yycrank+0,	yysvec+16,	yyvstop+74,
yycrank+6,	0,		0,	
yycrank+5,	0,		0,	
yycrank+6,	0,		0,	
yycrank+288,	0,		0,	
yycrank+0,	yysvec+20,	yyvstop+76,
yycrank+-9,	yysvec+7,	yyvstop+78,
yycrank+-10,	yysvec+15,	yyvstop+80,
yycrank+8,	0,		0,	
yycrank+10,	0,		0,	
yycrank+0,	0,		yyvstop+82,
yycrank+0,	0,		yyvstop+84,
yycrank+0,	0,		yyvstop+86,
yycrank+0,	0,		yyvstop+88,
yycrank+0,	0,		yyvstop+90,
0,	0,	0};
struct yywork *yytop = yycrank+410;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
  0,   1,   1,   1,   1,   1,   1,   1, 
  1,   9,  10,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  9,   1,  34,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,  46,  47, 
 48,  48,  48,  48,  48,  48,  48,  48, 
 48,  48,   1,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,   1,   1,   1,   1,   1, 
  1,  97,  97,  97,  97,  97,  97,  97, 
 97,  97,  97,  97,  97,  97,  97,  97, 
 97,  97,  97,  97,  97,  97,  97,  97, 
 97,  97,  97,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	Copyright (c) 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#pragma ident	"@(#)ncform	6.12	97/12/08 SMI"

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
#if defined(__cplusplus) || defined(__STDC__)
int yylook(void)
#else
yylook()
#endif
{
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
#ifndef __cplusplus
			*yylastch++ = yych = input();
#else
			*yylastch++ = yych = lex_input();
#endif
#ifdef YYISARRAY
			if(yylastch > &yytext[YYLMAX]) {
				fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
				exit(1);
			}
#else
			if (yylastch >= &yytext[ yytextsz ]) {
				int	x = yylastch - yytext;

				yytextsz += YYTEXTSZINC;
				if (yytext == yy_tbuf) {
				    yytext = (char *) malloc(yytextsz);
				    memcpy(yytext, yy_tbuf, sizeof (yy_tbuf));
				}
				else
				    yytext = (char *) realloc(yytext, yytextsz);
				if (!yytext) {
				    fprintf(yyout,
					"Cannot realloc yytext\n");
				    exit(1);
				}
				yylastch = yytext + x;
			}
#endif
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (uintptr_t)yyt > (uintptr_t)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((uintptr_t)yyt < (uintptr_t)yycrank) {	/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
#ifndef __cplusplus
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
#else
		yyprevious = yytext[0] = lex_input();
		if (yyprevious>0)
			lex_output(yyprevious);
#endif
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
#if defined(__cplusplus) || defined(__STDC__)
int yyback(int *p, int m)
#else
yyback(p, m)
	int *p;
#endif
{
	if (p==0) return(0);
	while (*p) {
		if (*p++ == m)
			return(1);
	}
	return(0);
}
	/* the following are only used in the lex library */
#if defined(__cplusplus) || defined(__STDC__)
int yyinput(void)
#else
yyinput()
#endif
{
#ifndef __cplusplus
	return(input());
#else
	return(lex_input());
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyoutput(int c)
#else
yyoutput(c)
  int c; 
#endif
{
#ifndef __cplusplus
	output(c);
#else
	lex_output(c);
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyunput(int c)
#else
yyunput(c)
   int c; 
#endif
{
	unput(c);
	}
