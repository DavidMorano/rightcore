static char sccsid[] = "@(#)pcsdisplay.c	PCS 3.0";
/*
 * pcsdisplay file1 file2
 *
 * To compile cc pcsdisplay.c -o pcsdisplay
 */


#include <stdio.h>

#include <string.h>


#define HELP 2
#define DISPLAY 3



int main (argc, argv)
int argc;
char *argv[];
{
    FILE *fd1, *fd2;
    char line1[MAXPATHLEN + 1], line2[MAXPATHLEN + 1] ;
    char file1[MAXPATHLEN + 1], file2[MAXPATHLEN + 1] ;
    char name[20], *c1, *c2;
    int number;
    int mode;
    char *pr1, *pr2, *pr3, *on, *deliv;
    

    mode = argc;
    strcpy( file1, argv[1] );
    if( mode == HELP )
    {
	strcpy( file2, argv[1] );
        file1[0] = '\0';
    }
    else
    {
	strcpy( file2, argv[2] );
    }
    
    if(*file1 != '\0' && ( fd1 = fopen( file1, "r" )) == NULL )
    {
	printf("%s: unable to open %s for reading\n",argv[0],file1);
	exit(1);
    }
    if(( fd2 = fopen( file2, "r" )) == NULL )
    {
	printf("%s: unable to open %s for reading\n", argv[0],file2);
	exit(1);
    }
    if( mode == DISPLAY )
    {
	printf("name		    protocols			on pcsnet	delivery\n");
        printf("   		 1	 2	 3				program\n\n");
        while(fgets( line1, 80, fd1 ) != NULL)
        {
	    sscanf(line1,"%s%d",name,&number);
	    rewind(fd2);
	    while( number >= 0 && (fgets( line2, 80, fd2 ) != NULL)) number--;
	    if( number < 0 )
	    {
	        c1 = line2;
	        c1 += 12;
	        pr1 = strtok( c1, ", \t" );
	        pr2 = strtok( 0, ", \t");
	        pr3 = strtok( 0, ", \t");
	        on = strtok( 0, ", \t");
	        deliv = strtok( 0, ", \t\n");
	        printf("%s	%s %s	 %s  %s	  \t%s	\t%s\n",
		name,(strlen(name) > 7)?"":"	", pr1, pr2, pr3, on, deliv);
	    }
	}

    } else { 
	printf(
"number		    protocols			on pcsnet	delivery\n");
        printf(
"   		 1	 2	 3				program\n\n");
	number = 0;
	while(fgets( line2, 80, fd2 ) != NULL) {

	        c1 = line2;
	        c1 += 12;
	        pr1 = strtok( c1, ", \t" );
	        pr2 = strtok( 0, ", \t");
	        pr3 = strtok( 0, ", \t");
	        on = strtok( 0, ", \t");
	        deliv = strtok( 0, ", \t\n");
	        printf("  %d	 %s	 %s  %s	   \t%s	\t%s\n",
		number++, pr1, pr2, pr3, on, deliv);
	}
    }

	return 0 ;
}
/* end subroutine (main) */


