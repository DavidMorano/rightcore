
#define VERS "Code Beautifier for C/C++/Java        Version 2.1n Aug 14, 1997\n"
#ifdef PRO_C
#define PGM "cbpc"
#define PCVERS "Also indenting Oracle Pro-C SQL\n"    
#else
#define PGM "cbVan"
#endif /* PRO_C */
/*
 *     Also called: Pretty-Printer, Reformatter or Indent (GNU)
 * 
 *     Swiped from the CIPG's UNIX system and modified to run
 *         under BDS C by William C. Colley, III
 *  
 *     Mods made July 1980
 *  
 *** REVISIONS ***
 * 
 *     Modified to run on the IBM-PC with C86 08/16/83  J.E.Will
 *  
 *     Put indent level option in  08/23/83   J.E.Will
 * 
 *     Change interface for single file name, old file is
 *     renamed .bak after program terminates. 08/25/83  J.E.Will
 * 
 *     Enhancements to run on MS Windows 95/NT:
 * 
 *     2.0h 08/01/1997  Van Di-Han Ho
 *     - read long file name with case sensitive (for Java Class)
 *     - processed C++ comments '//'
 *     - debugged C/C++ comments for better indentation
 *     - used blank spaces instead of tabs
 *     - portable to Unix (tested on Sun OS 4.0, Solaris 2.4)
 *     - ANSI (__STDC__) prototype or Kernighan & Ritchie
 * 
 *     2.1i 08/12/1997  Van Di-Han Ho
 *     - align comments for JavaDoc
 *
 *     2.1n 08/10/1997  Van Di-Han Ho
 *     - Oracle Pro-C: beginning with 'EXEC SQL' and ending with ';'
 *     - indent based on number of leading spaces 
 *     - indent between BEGIN DECLARE / END DECLARE
 *     - #define PRO_C to compile
 * 
 * 
 *** END ***
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


int s_level[10] ;
int c_level ;
int sp_flg[20][10] ;
int s_ind[20][10] ;
int s_if_lev[10] ;
int s_if_flg[10] ;
int if_lev ;
int if_flg ;
int level ;
int indent;/* number of spaces to indent */
static int ind[10] = {
	    0,0,0,0,0,0,0,0,0,0} ;
int e_flg ;
int paren ;
static int p_flg[10] = {
	    0,0,0,0,0,0,0,0,0,0} ;
char l_char ;
char p_char ;
int a_flg ;
int ct ;
int s_tabs[20][10] ;
int q_flg ;
char *w_if_[2] ;
char *w_else[2] ;
char *w_for[2] ;
char *w_ds[3] ;
char *w_cpp_comment[2] ;

#ifdef PRO_C
char *w_execsql[2] ;
char *w_sql_begin[2] ;
char *w_sql_end[2] ;
int  sql_flg ;
int  sql_declare;/* to indent lines between BEGIN/END */
#endif /* PRO_C */

int j ;
#define MAXLEN 256
char string[MAXLEN];/* input line max. length  */
char cc ;
int s_flg ;
int b_flg ;
char peek ;
int tabs ;
char last_char ;
int  use_tabs = 0;/* option 't' command line  will turn it on */
char c, c0 ;
char *w_kptr ;

int fixtabs ;

FILE *inpfil,*outfil,*fopen() ;

int comment(), putcoms(), getnl(), gotelse(), get_string(), lookup(),
indent_puts(), getchr(), p_tabs() ;
void cpp_comment(), cpp_putcoms() ;


/* code starts here, Jack */

/* check MAXLEN of string, also max. length of the line */
ck_MAXLEN()
{
	if (j > MAXLEN-10)
	{
	    printf ("%s\n\n", string) ;
	    printf ("String exceeding MAXLEN=%d\n", MAXLEN) ;
	    return (1) ;
	}
	return (0) ;
}
/*     return index of t in s, -1 if none     */
#ifdef __STDC__
lookin (char s[], char t[])
#else
lookin (s, t)
char s[], t[] ;
#endif
{
	int i, j, k ;
	for (i = 0; s[i] != '\0'; i++)
	{
	    for (j=i, k=0; t[k] != '\0' && toupper(s[j])==t[k]; j++, k++) ;
	    if (t[k] == '\0' )
	        return(i) ;
	}
	return(-1) ;
}


/* search table for string and return index number: lower or upper case */
#ifdef __STDC__
lookdown(char *tab[])
#else
lookdown(tab)
char *tab[] ;
#endif
{
	char r ;
	int l,kk,k,i ;

	if (j < 1) return(0) ;
	kk=0 ;
	while (string[kk] == ' ') kk++ ;
	for (i=0; tab[i] != 0; i++)
	{
	    l=0 ;
	    for (k=kk;(r = tab[i][l++]) == toupper(string[k]) && r != '\0';k++) ;
	    if (r == '\0' && (string[k] < 'a' || string[k] > 'z'))return(1) ;
	}
	return(0) ;
}
#ifdef PRO_C
/* Process Oracle Pro-C SQL statment */
void
sql_statement()
{
#define MAXSQL_INDENT 20
	int idx ;
	int lblank ;
	int blank_table[20], b_level = 0 ;
	int save_tab ;
	int save_s_flg ;
	int idx_orig = 4 ;
	int lookin_idx ;

	if (sql_declare == 0) tabs++ ;
	save_tab = tabs ;
	save_s_flg = s_flg ;
	for (b_level=0; b_level < MAXSQL_INDENT; b_level++)
	    blank_table[b_level] = 0 ;
	b_level = idx_orig ;
	lblank = 0 ;
	while ((c = string[j++] = getchr()) != ';')
	{
/* remove leading blanks, then indent */
	    if (string[0] == ' ')
	    {
	        lblank++ ;
	        j=0 ;
	        goto next_char ;
	    }
	    else if (string[0] == '\t')
	    {
	        lblank = lblank + indent ;
	        j=0 ;
	        goto next_char ;
	    }

	    if (c == '\n')
	    {
	        if (lblank>(blank_table[b_level]+1))
	        {
	            b_level++ ;
	            blank_table[b_level] = lblank ;
	        }
	        else
	            if (lblank < blank_table[idx_orig+1])
	            {
	                idx_orig-- ;
	                blank_table[idx_orig] = lblank ;
	            }

	        idx = b_level ;
/* indent at exact match or offset by 1 char */
	        while ((lblank - blank_table[idx]) <= 1
	            && idx >= 0) idx-- ;
	        if (idx<idx_orig) idx = idx_orig ;
	        if (lblank - blank_table[idx+1] == -1) idx++ ;

	        tabs = save_tab + idx - idx_orig ;



	        lblank = 0 ;

	        cpp_putcoms() ;
	        s_flg = 1;            /* indent yes */
	    }
next_char:        /* end while */
	     ;

	}
	if (sql_declare == 0)
	{
	    if (lookdown(w_sql_begin) == 1)
	    {
	        sql_declare = 1 ;
	    }
	    tabs++ ;

/*				            if (lookdown(w_sql_end) == 1) */
	}
	else
	{

	    if (lookin_idx = lookin(string, w_sql_end[0]) > 0)
	    {
	        sql_declare = -1 ;
	        if (tabs>0) tabs--;            /* re-adjust before printing */
	    }
	}


	cpp_putcoms() ;

	tabs = save_tab ;
	if (sql_declare>0)
	{
	    tabs++;        /* start indent after BEGIN */
	}
	else if (sql_declare<0)
	{
	    tabs--;        /* stop indent at END */
	    sql_declare = 0;        /* reset and look for next BEGIN */
	}
	s_flg = save_s_flg ;
	if (p_flg[level] > 0 && ind[level] == 0)
	{
	    tabs -= p_flg[level] ;
	    p_flg[level] = 0 ;
	}
	getnl() ;
	indent_puts() ;
	fprintf(outfil,"\n") ;
	s_flg = 1 ;
	if (if_lev > 0)
	    if (if_flg == 1)
	    {
	        if_lev-- ;
	        if_flg = 0 ;
	    }
	    else if_lev = 0 ;
	tabs-- ;
}
#endif /* PRO_C */

#ifdef __STDC__
int main(int argc,char *argv[])
#else
int main(argc, argv)
int argc ;
char *argv[] ;
#endif
{
	int double_colon = 0;    /* test if double colon */

#ifndef	COMMENT
	printf(VERS)        ;    /* tell the world about version */
#endif

/*  Initialize everything here.  */

#ifdef PRO_C
	printf(PCVERS)        ;    /* tell the world about version */
	sql_declare = 0 ;
#endif /* PRO_C */

	if (argc < 2 || argc > 4)
	{
	    printf("\nUsage: %s input.fil [indent] [tab]\n", PGM) ;
	    printf("(Executing:  %s)\n\n", argv[0]) ;
	    printf("input.fil - input source file in current directory\n") ;
	    printf("          - enclose long file name with space in double quotes\n") ;
	    printf("indent    - where 'indent' is a number from 1 to 8 equal\n") ;
	    printf("            to number of spaces for each indent level\n") ;
	    printf("            Default 4 if not supplied on cmd line\n") ;
	    printf("tab       - output tabs when 't' is supplied on the cmd line\n\n") ;
	    printf("EXAMPLES:\n") ;
	    printf("- %s hello.c\n", PGM) ;
	    printf("- %s hello.cpp 6\n", PGM) ;
	    printf("- %s hello.java 8 t\n", PGM) ;
	    printf("- %s \"hello world.cxx\"\n\n", PGM) ;
	    printf("NOTES:\n") ;
	    printf("- at normal end-of-processing, original source is in input.bak\n") ;
	    printf("- if abnormal end, original source is not changed\n") ;
	    exit(1) ;
	}

/* get name and check that it's not a back-up file */

	strcpy(string,argv[1]) ;
	if ((w_kptr = index(string,'.')) != 0)
	{
	    if ((strcmp(w_kptr,".BAK") == 0) || (strcmp(w_kptr,".bak") == 0))
	    {
	        printf("Input file can not be a .bak file!\n") ;
	        exit(20) ;
	    }
	}

	if ((inpfil = fopen(argv[1],"r")) == 0)
	{
	    printf("File not found: %s\n",argv[1]) ;
	    exit(21) ;
	}

	unlink("CB.$$$")        ;    /* delete any existing temp file */

	if ((outfil = fopen("CB.$$$","wr")) == 0)
	{
	    printf("Could not create CB.$$$, the temp output file\n") ;
	    exit(22) ;
	}


/* process indent option */

	indent = 4;    /* set default indent */

	if (argc >= 3)
	{
	    indent = atoi(argv[2]) ;
	    if  (indent == 0 && argc>3) indent = atoi(argv[3]) ;
	    if  (indent < 0) indent = -indent;        /* allow 6 or -6 */
	    if  (indent < 1 || indent > 8)
	    {
	        printf("Indent option value out of range, valid from 1 to 8.") ;
	        exit(10) ;
	    }

	}
	if (argc > 3)
	{
	    if (argv[3][0] == 't' || argv[3][1] == 't') use_tabs = 1 ;
	    if (argv[2][0] == 't' || argv[2][1] == 't') use_tabs = 1 ;
	}
	c_level = if_lev = level = e_flg = paren = 0 ;
	a_flg = q_flg = j = b_flg = tabs = 0 ;
	if_flg = peek = -1 ;
	s_flg = 1 ;
	w_if_[0] = "if" ;
	w_else[0] = "else" ;
	w_for[0] = "for" ;
	w_ds[0] = "case" ;
	w_ds[1] = "default" ;
	w_if_[1] = w_else[1] = w_for[1] = w_ds[2] = 0 ;
	w_cpp_comment[0] = "//" ;
	w_cpp_comment[1] = 0 ;

#ifdef PRO_C
	w_execsql[0] = "EXEC SQL" ;
	w_execsql[1] = 0 ;
	w_sql_begin[0] = "BEGIN DECLARE";    /* keep it simple for the time being */
	w_sql_begin[1] = 0 ;
	w_sql_end[0] = "END DECLARE" ;
	w_sql_end[1] = 0 ;
#endif /* PRO_C */

/*  End of initialization.  */

	while ((c = getchr()) != 032) {

	    switch(c) {

	    default:
	        string[j++] = c ;
	        if (c != ',') l_char = c ;
	        break ;

	    case ' ':
	    case '\t':
	        if (lookup(w_else) == 1)
	        {
	            gotelse() ;
	            if (s_flg == 0 || j > 0) string[j++] = c ;

	            indent_puts() ;
	            s_flg = 0 ;
	            break ;
	        }
	        if (s_flg == 0 || j > 0) string[j++] = c ;

#ifdef PRO_C
	        if (lookdown(w_execsql) == 1)
	        {
	            sql_flg = 1;                /* indent */
	            if (sql_declare == 0) indent_puts();                /* */
	            sql_statement() ;
	            break ;
	        }
	        else sql_flg = 0 ;
#endif /* PRO_C */

	        break ;

	    case '\n':
	        fflush (outfil) ;
	        if (e_flg = lookup(w_else) == 1) 
			gotelse() ;

	        if (lookup(w_cpp_comment) == 1)
	        {
	            if (string[j] == '\n')
	            {
	                string[j] = '\0' ;
	                j-- ;
	            }
	        }

	        indent_puts() ;
	        fprintf(outfil,"\n") ;
	        s_flg = 1 ;
	        if (e_flg == 1)
	        {
	            p_flg[level]++ ;
	            tabs++ ;
	        }
	        else
	            if (p_char == l_char)
	                a_flg = 1 ;
	        break ;

	    case '{':
	        if (lookup(w_else) == 1) 
			gotelse() ;

	        s_if_lev[c_level] = if_lev ;
	        s_if_flg[c_level] = if_flg ;
	        if_lev = if_flg = 0 ;
	        c_level++ ;
	        if (s_flg == 1 && p_flg[level] != 0)
	        {
	            p_flg[level]-- ;
	            tabs-- ;
	        }
	        string[j++] = c ;
	        indent_puts() ;
	        getnl() ;
	        indent_puts() ;
	        fprintf(outfil,"\n") ;
	        tabs++ ;
	        s_flg = 1 ;
	        if (p_flg[level] > 0)
	        {
	            ind[level] = 1 ;
	            level++ ;
	            s_level[level] = c_level ;
	        }
	        break ;

	    case '}':
	        c_level-- ;
	        if ((if_lev = s_if_lev[c_level]-1) < 0) 
			if_lev = 0 ;

	        if_flg = s_if_flg[c_level] ;
	        indent_puts() ;
	        tabs-- ;
	        p_tabs() ;
	        if ((peek = getchr()) == ';')
	        {
	            fprintf(outfil,"%c;",c) ;
	            peek = -1 ;
	        }
	        else 
			fprintf(outfil,"%c",c) ;

	        getnl() ;

	        indent_puts() ;

	        fprintf(outfil,"\n") ;

	        s_flg = 1 ;
	        if (c_level < s_level[level])
	            if (level > 0)
	                level-- ;

	        if (ind[level] != 0)
	        {
	            tabs -= p_flg[level] ;
	            p_flg[level] = 0 ;
	            ind[level] = 0 ;
	        }
	        break ;

	    case '"':
	    case '\'':
	        string[j++] = c ;
	        while ((cc = getchr()) != c) {

/*   Van: max. length of line should be 256 */
	            if (ck_MAXLEN()) exit(90) ;

	            string[j++] = cc ;
	            if (cc == '\\')
	            {
	                cc = string[j++] = getchr() ;
	            }
	            if (cc == '\n')
	            {
	                indent_puts() ;
	                s_flg = 1 ;
	            }
	        }
	        string[j++] = cc ;
	        if (getnl() == 1)
	        {
	            l_char = cc ;
	            peek = '\n' ;
	        }
	        break ;
	    case ';':

#ifdef PRO_C
	        sql_flg = 0 ;
#endif /* PRO_C */

	        string[j++] = c ;
	        indent_puts() ;
	        if (p_flg[level] > 0 && ind[level] == 0)
	        {
	            tabs -= p_flg[level] ;
	            p_flg[level] = 0 ;
	        }
	        getnl() ;
	        indent_puts() ;
	        fprintf(outfil,"\n") ;
	        s_flg = 1 ;
	        if (if_lev > 0)
	            if (if_flg == 1)
	            {
	                if_lev-- ;
	                if_flg = 0 ;
	            }
	            else 
			if_lev = 0 ;

	        break ;

	    case '\\':
	        string[j++] = c ;
	        string[j++] = getchr() ;
	        break ;

	    case '?':
	        q_flg = 1 ;
	        string[j++] = c ;
	        break ;

	    case ':':
	        string[j++] = c ;
	        peek = getchr() ;
	        if (peek == ':')
	        {
	            indent_puts() ;
	            fprintf (outfil,":") ;
	            peek = -1 ;
	            break ;
	        }
	        else 
	        {
	            double_colon = 0 ;

#ifdef PRO_C
	            if (sql_flg)
	            {
	                s_flg = 1;                    /* Van 2.1i indent SQL */
	                indent_puts() ;
	                s_flg = 1;                    /* Van 2.1i indent SQL */
	                fprintf (outfil,"%c", peek) ;
	                peek = -1 ;
	                break ;
	            }
	            else ;
#endif /* PRO_C */
	        }

	        if (q_flg == 1)
	        {
	            q_flg = 0 ;
	            break ;
	        }
	        if (lookup(w_ds) == 0)
	        {
	            s_flg = 0 ;
	            indent_puts() ;
	        }
	        else
	        {
	            tabs-- ;
	            indent_puts() ;
	            tabs++ ;
	        }
	        if ((peek = getchr()) == ';')
	        {
	            fprintf(outfil,";") ;
	            peek = -1 ;
	        }
	        getnl() ;
	        indent_puts() ;
	        fprintf(outfil,"\n") ;
	        s_flg = 1 ;
	        break ;

	    case '/':
	        c0 = string[j] ;
	        string[j++] = c ;
	        if ((peek = getchr()) == '/')
	        {
	            string[j++] = peek ;
	            peek = -1 ;
	            cpp_comment() ;
	            fprintf(outfil,"\n") ;
	            break ;
	        }
	        if ((peek = getchr()) != '*')
	            break ;

	        if (peek == '*')
	        {
	            string[j--] = '\0' ;
	            indent_puts() ;
	            string[j++] = '/' ;
	            string[j++] = '*' ;
	            peek = -1 ;
	            comment() ;
	            break ;
	        }

	    case ')':
	        paren-- ;
	        string[j++] = c ;
	        indent_puts() ;
	        if (getnl() == 1)
	        {
	            peek = '\n' ;
	            if (paren != 0)
	                a_flg = 1 ;

	            else if (tabs > 0)
	            {
	                p_flg[level]++ ;
	                tabs++ ;
	                ind[level] = 0 ;
	            }
	        }
	        break ;

	    case '#':
	        string[j++] = c ;
	        while ((cc = getchr()) != '\n') string[j++] = cc ;

	        string[j++] = cc ;
	        s_flg = 0 ;
	        indent_puts() ;
	        s_flg = 1 ;
	        break ;

	    case '(':
	        string[j++] = c ;
	        paren++ ;
	        if ((lookup(w_for) == 1))
	        {
	            while ((c = get_string()) != ';') ;

	            ct=0 ;
cont:
	            while ((c = get_string()) != ')')
	            {
	                if (c == '(') ct++ ;
	            }
	            if (ct != 0)
	            {
	                ct-- ;
	                goto cont ;
	            }
	            paren-- ;
	            indent_puts() ;
	            if (getnl() == 1)
	            {
	                peek = '\n' ;
	                p_flg[level]++ ;
	                tabs++ ;
	                ind[level] = 0 ;
	            }
	            break ;
	        }
	        if (lookup(w_if_) == 1)
	        {
	            indent_puts() ;
	            s_tabs[c_level][if_lev] = tabs ;
	            sp_flg[c_level][if_lev] = p_flg[level] ;
	            s_ind[c_level][if_lev] = ind[level] ;
	            if_lev++ ;
	            if_flg = 1 ;
	        }

	    } /* end switch */

	} /* end while */

/* eof processing */

/*  fprintf(outfil,"\032")        ;          throw in an EOF mark */
	fclose(outfil) ;
	fclose(inpfil) ;

/* get origional name with a .bak on it */

	strcpy(string,argv[1]) ;
	if ((w_kptr = index(string,'.')) == 0)
	    strcat(string,".bak") ;

	    else
	    strcpy(w_kptr,".bak") ;

	unlink(string)        ;    /* kill any .bak file */
	rename(argv[1],string)        ;    /* original is now .bak */
	rename("CB.$$$",argv[1])        ;    /* new now has original name */
	exit(0)        ;    /* normal exit */
}
/* expand indent into tabs and spaces */
p_tabs()
{
	int i,j,k ;

	if (tabs<0) tabs = 0;    /* Van 20b C++ inline gets next { */
	i = tabs * indent;    /* calc number of spaces */
	j = i/8;    /* calc number of tab chars */
	if (use_tabs)
	{
	    i -= j*8;        /* calc remaining spaces */
	    for (k=0;k < j;k++) fprintf(outfil,"\t") ;
	}
	for (k=0;k < i;k++) fprintf(outfil," ") ;
	return (0) ;
}
/* get character from stream or return the saved one */
getchr()
{
	if (peek < 0 && last_char != ' ' && last_char != '\t')
	    p_char = last_char ;

	last_char = (peek<0) ? getc(inpfil):peek ;
	if (last_char == -1)
	    last_char = 0x1a ;

	peek = -1 ;
	return(last_char == '\r' ? getchr():last_char) ;
}
/* put string with indent logic */
indent_puts()/* INDENT_PUTS */
{
	int k ;


	for (k=j; k<256; k++) string[k] = '\0' ;

	if (j > 0)
	{
	    if (s_flg != 0)
	    {
	        if ((tabs > 0) && (string[0] != '{') && (a_flg == 1))
	            tabs++ ;

	        p_tabs() ;
	        s_flg = 0 ;
	        if ((tabs > 0) && (string[0] != '{') && (a_flg == 1))
	            tabs-- ;

	        a_flg = 0 ;
	    }
	    fprintf(outfil,"%s",string) ;
	    j = 0 ;
	    sprintf (string,"") ;

	}
	else
	{
	    if (s_flg != 0)
	    {
	        s_flg = 0 ;
	        a_flg = 0 ;
	    }
	}
	return (0) ;
}
/* search table for character string and return index number */
#ifdef __STDC__
lookup(char *tab[])
#else
lookup(tab)
char *tab[] ;
#endif
{
	char r ;
	int l,kk,k,i ;

	if (j < 1) return(0) ;
	kk=0 ;
	while (string[kk] == ' ') kk++ ;

	for (i=0; tab[i] != 0; i++)
	{
	    l=0 ;
	    for (k=kk;(r = tab[i][l++]) == string[k] && r != '\0';k++) ;
	    if (r == '\0' && (string[k] < 'a' || string[k] > 'z'))return(1) ;
	}
	return(0) ;
}
/* read string between double quotes */
get_string()
{
	char ch ;
beg:
	if ((ch = string[j++] = getchr()) == '\\')
	{
	    string[j++] = getchr() ;
	    goto beg ;
	}
	if (ch == '\'' || ch == '"')
	{
	    while ((cc = string[j++] = getchr()) != ch)
	        if (cc == '\\')
	            cc = string[j++] = getchr() ;
	    goto beg ;
	}
	if (ch == '\n')
	{
	    indent_puts() ;
	    a_flg = 1 ;
	    goto beg ;
	}
	else 
		return(ch) ;
}
/* else processing */
gotelse()
{
	tabs = s_tabs[c_level][if_lev] ;
	p_flg[level] = sp_flg[c_level][if_lev] ;
	ind[level] = s_ind[c_level][if_lev] ;
	if_flg = 1 ;
	return (0) ;
}
/* read to new_line */
getnl()
{
	int save_s_flg ;
	save_s_flg = tabs ;
	while ((peek = getchr()) == '\t' || peek == ' ')
	{
	    string[j++] = peek ;
	    peek = -1 ;
	}
	if ((peek = getchr()) == '/')
	{
	    peek = -1 ;
	    if ((peek = getchr()) == '*')
	    {
	        string[j++] = '/' ;
	        string[j++] = '*' ;
	        peek = -1 ;
	        comment() ;
	    }
	    else if (peek=='/')        /* Van */
	    {        /* Inline C++ comment */
	        string[j++] = '/' ;
	        string[j++] = '/' ;
	        peek = -1 ;
	        cpp_comment() ;
	        return (1) ;

	    } else 
		string[j++] = '/' ;

	}

	if ((peek = getchr()) == '\n')
	{
	    peek = -1 ;
	    tabs = save_s_flg ;
	    return(1) ;
	}
	tabs = save_s_flg ;
	return(0) ;
}

/* print cpp comment */
void
cpp_putcoms()
{
	if (j > 0)
	{
	    if (s_flg != 0)
	    {
	        p_tabs() ;
	        s_flg = 0 ;
	    }
	    string[j] = '\0' ;
	    fprintf(outfil,"%s",string) ;
	    j = 0 ;
	    string[j] = '\0' ;
	}
}

/* special edition of put string for comment processing */
putcoms()
{
	int i, save_s_flg ;


	save_s_flg = s_flg ;
	if (j > 0) {

	    if (s_flg != 0)
	    {
	        p_tabs() ;
	        s_flg = 0 ;
	    }
	    string[j] = '\0' ;
	    i = 0 ;
	    while (string[i] == ' ') 
		i++ ;

	    if (string[i] == '/' && string[i+1]=='*') {

	        if (last_char != ';')
	            fprintf(outfil,"%s", &string[i]);        /* Van2.1d */

	            else
	            fprintf(outfil,"%s", string);        /* Van2.1d */

	        } else if (string[i] == '*' && string[i+1]=='/') {

	            fprintf(outfil," %s", &string[i]);        /* Van2.1d */

	        } else {

	            i = 0 ;
	            while (string[i] == ' ') 
			i++ ;

	            if (string[i] == '*') 
			fprintf(outfil,"%s", &string[i-1]); /* V
				    an2.1d */
	            else
	                fprintf(outfil," * %s", string); /* Van2.1d */

	        } /* end if */

/*        if (string[0] == '*') fprintf(outfil," ");   /* Van2.1d */

/*        fprintf(outfil,"%s",string); */

	    j = 0 ;

	} /* end if */

	return (0) ;
}


/* cpp comment processing */
void
cpp_comment()
{
	while ((c = getchr()) != '\n') string[j++]=c  ;
	cpp_putcoms() ;
	s_flg = 1 ;
}

/* comment processing */
comment()
{
	int save_s_flg ;
	save_s_flg = s_flg ;
rep:
	while ((c = string[j++] = getchr()) != '*')
	{
	    s_flg = 1;        /* Van2.1d */
	    if (c == '\n')
	        putcoms() ;

	}

gotstar:
	if ((c = string[j++] = getchr()) != '/')
	{
	    if (c == '*')
	        goto gotstar ;

	    s_flg = 1;        /* Van2.1d */
	    if (c == '\n')
	        putcoms() ;

	    goto rep ;
	}
	s_flg = 1;    /* Van2.1d */
	putcoms() ;
	s_flg = save_s_flg ;
	return (0) ;
}


