/* main */


#define	CF_DEBUGS	0


#include	<envstandards.h>

#include <string.h>
#include <ctype.h>
#include <stdio.h>



int main() 
{
	FILE *in_pipe ;
	char word[100] ;
	char buffer[100] ;
	char answer[100] ;
	char *p ;


	while (1) {

	    if( !fgets(word, sizeof(word), stdin) ){
	        break ;
	    }
	    p = rindex(word, '\n') ;
	    if( p != NULL ) {
	        *p = '\0' ;
	    }
/* fprintf(stdout,"word = %s\n",word);*/
	    if( isalpha(word[0]) ){
	        sprintf(buffer,"grope \"%s\" < /dev/null",word) ;

#if	CF_DEBUGS
	        fprintf(stdout,"buffer = %s\n",buffer) ;
#endif
	        in_pipe=popen(buffer,"r") ;

#if	CF_DEBUGS
	        fprintf(stdout,"inpipe = %d\n",in_pipe) ;
#endif
	        if( in_pipe == NULL ){
	            fprintf(stderr,
	                "error in the duplic grope pipe\n") ;
	            strcpy(answer,word) ;
	        }
	        else {
	            int n ;
	            n=fscanf(in_pipe,"%s",answer) ;
	            if( n != 1 ){
	                fprintf(stderr,
	                    "error in the duplic answer pipe n=%d\n",n) ;
	                exit(1) ;
	            }
	        }
	        fflush(in_pipe) ;
	        pclose(in_pipe) ;
	    }
	    else {
	        strcpy(answer,word) ;
	    }
	    printf("%s/%s\n",word,answer) ;

	} /* end if */

	return 0 ;
}
/* end subroutine (main) */


