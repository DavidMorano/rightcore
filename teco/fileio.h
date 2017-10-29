/*	static char h_fileio[] = "@(#) fileio.h:  4.2 12/12/82";	*/
/* fileio.h -- declarations for fileio.c */
/* must be preceded by bool.h, stdio.h */

extern FILE * Fopread();	/* open for reading */
extern FILE * Fopwrite();	/* open for writing */
extern BOOL Ftestfile();	/* test for file existance */
extern BOOL Fclose();		/* close */
extern BOOL Frename();		/* rename file */
extern BOOL Fdelete();		/* delete file */
extern int Freadc();		/* read a character */
extern BOOL Ferror();		/* note error */
extern BOOL Fwritec();		/* write a character */
extern int Fread1line();	/* read a CR/LF delimited line */
extern BOOL Fmakebak();		/* make backup file filename */
extern BOOL Fmaketemp();	/* make temporary file name */
#ifdef FPRO
extern FPRO Fgetmode();		/* get file modes of existing file */
#endif	/* def FPRO */
extern BOOL Filepath();		/* try various paths of path variable */
