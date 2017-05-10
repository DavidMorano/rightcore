%{

/* comment */

int		num ;


/* forward references */

static int	clean(char *,int) ;


%}

string				\"([^"]|\\.)*\"
regexp				\/([^/]|\\.)*\/
number				[0-9]+

%%


"<p>"				{

					printf("PAR\n") ;
				}

"</p>"				{

					printf("PAR_END\n") ;
				}

"<P>"				{

					printf("PAR\n") ;
				}

"</P>"				{

					printf("PAR_END\n") ;
				}


[a-zA-Z]+\?[a-z]		{ 
					int	sl ;


					sl = clean(yytext,-1) ;

					fwrite(yytext,1,sl,stdout) ;

				}


"{"				{ printf("%d\n",1) ; }
"}"				{ printf("%d\n",2) ; }
"("				{ printf("%d\n",3) ; }
")"				{ printf("%d\n",4) ; }
"="				{ printf("%d\n",5) ; }
"~"				{ printf("%d\n",6) ; }
":"				{ printf("%d\n",7) ; }
"+"				{ printf("%d\n",8) ; }
"-"				{ printf("%d\n",9) ; }
"|"				{ printf("%d\n",10) ; }
"*"				{ printf("%d\n",11) ; }

[a-zA-Z\.]+			{ 
					printf("text> %s\n", yytext) ;
				}

"!"				{ printf("%d\n",13) ; }
"!!"				{ printf("%d\n",14) ; }
"^"				{ printf("%d\n",15) ; }

{string}			{ 
					printf("string> %s\n", yytext) ;
				}

{regexp}			{ 
					printf("regexp> %s\n", yytext) ;
				}

{number}			{ 
					printf("number> %s\n", yytext) ;
				}

\#[^\n]*\n			{;}

[\n\t ]				{;}

.				{
					printf("DOT\n") ;
				}

%%


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


