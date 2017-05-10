/* rem: places items in reminder service directory.
* J. A. Kutsch  ho 43233   x-3059
* July 1980
* converted from extended shell script to c in January 1981
*/

#define REMDIR "/usr/add-on/pcs/lib/remdata"

#include "remfile.h"
#include <stdio.h>
#include <string.h>
struct msgrec msgrec;
FILE *outfile, *fopen();
main(argc,argv)
int argc;
char **argv;
{
    int i;
    char tempfile[50], realfile[50];
    char buffer[150];
    char hrbuf[2], minbuf[2],msgbuf[150];
    char unique[16];
    char s[256];
    char *edata, *getenv();

    char *adata;	/*string for three times in one */
    char *tok;

    if (strcmp(argv[1],"+mail")==0) {
        strcpy(tempfile,REMDIR);
        strcat(tempfile,"/mailon");
        system (tempfile);
        exit(0);
    }

    if (strcmp(argv[1],"-mail")==0) {
        strcpy(tempfile,REMDIR);
        strcat(tempfile,"/mailoff");
        system (tempfile);
        exit(0);
    }

    if ((strcmp(argv[1],"+today"))==0 || (strcmp(argv[1],"+t"))==0 || (strcmp(argv[1],"+to"))==0){
        strcpy(tempfile,REMDIR);
        strcat(tempfile,"/remtoday");
        system(tempfile);
        exit(0);
    }
    edata=getenv("LOGNAME");
    if (edata==NULL)
        fatal("LOGNAME not set in environment");
    strcpy(msgrec.name,edata);
    for (i=strlen(msgrec.name);i<sizeof(msgrec.name);i++)
        msgrec.name[i]=' ';

    /* Try for REM_TIMES */
    if((adata=getenv("REM_TIMES")) == NULL)
    {
        edata=getenv("REM_T1");
        if (edata==NULL)
            msgrec.t1[0]=msgrec.t1[1]='0';
        else tdata(edata,msgrec.t1);	/* convert to zero fill, right justified */
        edata=getenv("REM_T2");
        if (edata==NULL)
            msgrec.t2[0]=msgrec.t2[1]='0';
        else tdata(edata,msgrec.t2);	/* convert to zero fill, right justified */
        edata=getenv("REM_T3");
        if (edata==NULL)
            msgrec.t3[0]=msgrec.t3[1]='0';
        else tdata(edata,msgrec.t3);	/* convert to zero fill, right justified */
    }

    else
    {
        sprintf (s,adata);
        tok = strtok (s,":");
        if (tok != NULL) tdata (tok, msgrec.t1);
        tok = strtok (0,":");
        if (tok !=NULL) tdata (tok, msgrec.t2);
        tok = strtok (0,":");
        tdata (tok,msgrec.t3);
    }
    strcpy(unique,"XXXXXX");
    mktemp(unique);
    strcpy(tempfile,REMDIR);
    strcat(tempfile,"/TMP.");
    strcat(tempfile,unique);
    strcpy(realfile,REMDIR);
    strcat(realfile,"/rem.");
    strcat(realfile,unique);
    umask(0);  /* make created files readable */
    if ((outfile=fopen(tempfile,"w")) == NULL)
        fatal("cannot create tempfile");
    if (strcmp(argv[1],"-")!=0) {
        buffer[0]='\0';
        for (i=1;i<argc;i++) {
            strcat(buffer," ");
            strcat(buffer,argv[i]);
        }
        if (parse(buffer,hrbuf,minbuf,msgbuf))
            insert(hrbuf,minbuf,msgbuf);
        else fprintf(stderr,"Rem: bad time field\n");
    }
    else /* read from stdin */
        while (gets(buffer)!=NULL) {
            if (parse(buffer,hrbuf,minbuf,msgbuf))
                insert(hrbuf,minbuf,msgbuf);
        }

    fclose(outfile);
    sync();
    link(tempfile,realfile);
    unlink(tempfile);
}

insert (hourarg,minarg,msgarg)
char *hourarg, *minarg, *msgarg;
{
    char *digits = "0123456789";
    msgrec.hour[0]=hourarg[0];
    msgrec.hour[1]=hourarg[1];
    msgrec.min[0]=minarg[0];
    msgrec.min[1]=minarg[1];
    if (strncmp(msgrec.hour,"08",2)<0) {
        /* times before 8 a.m. are assumed to be 1200 through 1959 */
        msgrec.hour[0]='1';
        msgrec.hour[1]=digits[instr(digits,msgrec.hour[1])+2];
    }

    strcpy(msgrec.text,msgarg);
    fprintf(outfile,"%s\n",&msgrec);
}

fatal(msg)
char *msg;
{
    fprintf(stderr,"rem: %s\n",msg);
    exit(1);
}

instr(s,c)
/* uses index to see where char c is in string s */
char *s, c;
{
    char s2[2];
    s2[0]=c;
    s2[1]='\0';
    return(index(s,s2));
}
