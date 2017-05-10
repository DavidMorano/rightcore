/* main (checkout) */


#define	CF_DEBUGS	0


/* revision history:

	= 01/02/01, Alireza Khalafi

	This program was originally written (for LevoSim verification).


	= 01/07/11, David A­D­ Morano

	I added the '-e' option for batch mode operation.  This
	causes the program to exit when it would have otherwise
	just hung there waiting for user input.


*/



#include	<sys/types.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



/* local defines */

#ifndef	FALSE
#define	FALSE	0
#define	TRUE	1
#endif

#define	EXITCOUNT	1000000




struct reg {
	char val[20] ;
	long lval ;
	char addr[20] ;
	long laddr ;
	int  valid ;
} ;


int main(int argc, char *argv[])
{
	struct reg levoreg[10],dbxreg[10] ;

	FILE * fp,*fpd ;

	char line[200] ;
	char tline[200] ;
	char line1[200] ;
	char dline[200] ;
	char *fa,*fae ;
	char levo[20],dbx[200],clock[100] ;
	char	*rp ;

	int f1 ;
	int k,l,ll,lr ;
	int start_f ;
	int contin_f ;
	long int inst_commited ;
	int compare_data ;
	int compare_sp ;
	int ar ;
	int	count ;
	int	f_exit = FALSE ;


	if (argc <= 1) {
	    printf("Usage: checkout out.levo out.dbx [-no_reg] [-sp] [-e]\n") ;
	    return ;
	}

	fp=fopen(argv[1],"rt") ;
	if(!fp) {
	    printf("Cannot open input file\n") ;
	    return ;
	}
	fpd=fopen(argv[2],"rt") ;
	if(!fpd) {
	    printf("Cannot open input file\n") ;
	    return ;
	}

	f1 = 1 ;
	start_f = 0 ;
	inst_commited = 0 ;

	compare_data = 1 ;
	compare_sp = 0 ;
	contin_f = 0 ;

	printf("argc=%d\n",argc) ;

	if (argc >= 4) {

	    for(ar=3;ar<argc;ar += 1) {

	        if(!strcmp(argv[ar],"-noreg"))
	            compare_data = 0 ;

	        if(!strcmp(argv[ar],"-sp"))
	            compare_sp = 1 ;

	        if(!strcmp(argv[ar],"-cont"))
	            contin_f = 1 ;

	        if (strcmp(argv[ar],"-e") == 0)
	            f_exit = TRUE ;

	    } /* end for */

	} /* end if */

	rp = fgets(dline,150,fpd) ;

	if (f_exit && (rp == NULL))
		goto done ;

#if	CF_DEBUGS
	fprintf(stderr,"line> %s\n",dline) ;
#endif

	while (TRUE) {

	    if (f1==1) {

	        rp = fgets(line,150,fp) ;

	        if (f_exit && (rp == NULL))
			goto done ;

#if	CF_DEBUGS
	fprintf(stderr,"line> %s\n",line) ;
#endif

	    } else {
	        strncpy(line,line1,150) ;
	        f1 = 1 ;
	    }

	    count = 0 ;
	    while (! (line[0]=='A' && line[1] =='S' && line[2] == ' ')) {

	        rp = fgets(line,150,fp) ;

	        if (f_exit && (rp == NULL))
			goto done ;

#if	CF_DEBUGS
	fprintf(stderr,"line> %s\n",line) ;
#endif

	        if (line[0] == 'C' && line[1]=='L' && line[2]=='O')
	            strcpy(clock,line) ;

		if (f_exit && (count++ >= EXITCOUNT))
			goto done ;

	    } /* end while */

#if	CF_DEBUGS
	fprintf(stderr,"got an AS line> %s\n",line) ;
#endif

	    rp = fgets(line1,150,fp) ;
	    rp = fgets(line1,150,fp) ;

	    if (f_exit && (rp == NULL))
			goto done ;

#if	CF_DEBUGS
	fprintf(stderr,"line> %s\n",line1) ;
#endif

	    if (line1[0]=='I' && line1[1] == 'n' && line1[2] == 's') {
	        printf("\n%s", line) ;


/* get the register values */
	        strcpy(tline,line) ;
	        if(get_regval(line,&levoreg[0]))
	            if(get_regval(line,&levoreg[1]))
	                get_regval(line,&levoreg[2]) ;
	                else
	                *levoreg[2].addr = 0 ;
	                else
	            *levoreg[1].addr = *levoreg[2].addr = 0 ;

	        if(*levoreg[1].addr && levoreg[1].laddr == levoreg[0].laddr) *levoreg[0].addr = NULL ;
	        if(*levoreg[2].addr && levoreg[2].laddr == levoreg[1].laddr) *levoreg[1].addr = NULL ;
	        if(*levoreg[2].addr && levoreg[2].laddr == levoreg[0].laddr) *levoreg[0].addr = NULL ;

	        if(*levoreg[0].addr) printf("reg1(%li)=%x\n", levoreg[0].laddr,levoreg[0].lval) ;
	        if(*levoreg[1].addr) printf("reg2(%li)=%x\n", levoreg[1].laddr,levoreg[1].lval) ;
	        if(*levoreg[2].addr) printf("reg3(%li)=%x\n", levoreg[2].laddr,levoreg[2].lval) ;
	        strcpy(line,tline) ;

	        if(fa=strstr(line,"FA=")) {
	            fae=strchr(fa,',') ;
	            if(fae) {
	                *fae = 0 ;
	                strcpy(levo,fa+5) ;
	            }
	        }
	        strcpy(dbx,dline+2) ;
	        *strchr(dbx,' ')=0 ;
	        if(!strcmp(dbx,levo)) { /* compares the addresses */
	            printf("%s",dline) ;
	            inst_commited ++ ;
	            rp = fgets(dline,150,fpd) ;

	            if (f_exit && (rp == NULL))
			goto done ;

#if	CF_DEBUGS
	fprintf(stderr,"line> %s\n",dline) ;
#endif

	            l = 0 ;
	            while (! (dline[0] == '0' && dline[1] == 'x')) {

/* get the dbx reg values */

	                get_dbxregval(dline, &dbxreg[l]) ;
	                if (*dbxreg[l].addr) 
				printf("dbxregl(%li)=%x\n", 
				dbxreg[l].laddr,dbxreg[l].lval) ;

	                l++ ;

	                if ((rp = fgets(dline,150,fpd)) == NULL)
	                    return 1 ;

#if	CF_DEBUGS
	fprintf(stderr,"line> %s\n",dline) ;
#endif

	            }
	            printf("%s\n",clock) ;

/* compare the reg values */
	            for(ll=0;ll<l;++ll) {
	                if(compare_sp || *dbxreg[ll].addr && dbxreg[ll].laddr != 29) {
	                    for(lr=0;lr<3;++lr) {
	                        if(*levoreg[lr].addr) {
	                            if(levoreg[lr].laddr == dbxreg[ll].laddr)
	                                if(levoreg[lr].lval != dbxreg[ll].lval) {
	                                    printf("data mismatch!!! Inst commited:%ld, %s,%s\n",
	                                        inst_commited,line1,clock) ;

	                                    if (compare_data) {

	                                        fflush(stdout) ;

	                                        if (f_exit)
							goto done ;

	                                        k=getc(stdin) ;

	                                    }
	                                }
	                        }
	                    }
	                }
	            }

	            start_f = 1 ;
	        }
	        else if(start_f) {
	            printf("%s",dline) ;
	            printf("Address mismatch!!! Inst commited:%ld, %s,%s\n",inst_commited,line1,clock) ;

	            fflush(stdout) ;

	            if (f_exit)
			goto done ;

	            k=getc(stdin) ;
	        }
	        if(!contin_f) {

	            fflush(stdout) ;

	            if (f_exit)
			goto done ;

	            k=getc(stdin) ;

	            if(k == 'c')
	                contin_f = 1 ;
	        }

	    } else
	        f1 = 0; /* copy line1 to line */

	    fflush(stdout) ;

	    if (k == 'n') ;

	} /* end while */

done:
	fclose(stdout) ;

	fclose(stderr) ;

	return 0 ;
}
/* end subroutine (main) */



int
get_regval(char *line, struct reg *reg) {

	int	a,b,c ;
	char 	t1[200] ;
	char	*cp,*cpe ;

	cp = strchr(line,'(') ;
	if(!cp) return 0 ;
	if(*(cp+1) != '0' || *(cp+2) != 'x') {
	    cpe = strchr(cp,')') ;
	    strncpy(reg->addr,cp+1,cpe-cp-1) ;
	    *(reg->addr + (int)(cpe - cp-1)) = NULL ;
	    cp = strchr(cpe,' ') ;
	    strncpy(reg->val,cpe+2,cp-cpe-1) ;
	    *(reg->val + (int)(cp - cpe -2)) = NULL ;
	    strcpy(line,cp) ;
	    reg->lval = strtol(reg->val,NULL,16) ;
	    reg->laddr = strtol(reg->addr,NULL,10) ;
	    return 1 ;
	}
	strcpy(reg->addr,"") ;
	return 0 ;

}



int
get_dbxregval(char *line, struct reg *reg) {

	char 	t1[200] ;
	char	*cp,*cpe ;
	char	addr[20] ;
	char	val[20] ;

	if(*line != 'r') {
	    strcpy(reg->addr,"") ;
	    return 0 ;
	}
	cp = strchr(line,'/') ;
	strncpy(addr,line+1,cp-line-1) ;
	*(addr + (int)(cp - line -1)) = NULL ;

	cp = strchr(line,'=') ;
	strcpy(val,cp+1) ;

	strcpy(reg->addr,addr) ;
	strcpy(reg->val,val) ;
	reg->lval = strtol(reg->val,NULL,16) ;
	reg->laddr = strtol(reg->addr,NULL,10) ;
}


