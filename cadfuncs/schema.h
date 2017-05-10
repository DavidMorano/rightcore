/* static char *h_schema="@(#)schema.h	1.19";	*/
/* run  12/16/94  09:12:58 */

/***************************************************************************/
/* This is the INCLUDE file which is to be linked by users when compiling  */
/* SCHEMA function files: cc -c file.c -Ipath_to_directory_with_this_file. */
/* It contains declarations of structures and functions that are exported  */
/* from SCHEMA for users to use.  It also contains declarations of         */
/* used constants and global variables.                                    */
/* You can find more information about SCHEMA's C-Interface capability in  */
/* the SCHEMA Reference Manual.                                            */
/***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include "setools.h"

/***************************************************************************/
/* define DATA and COORD structures                                        */
/***************************************************************************/
#define TINVIS		8    /* terminal status bit:  when =1 invisible */
#define DG_READ_ONLY	002
#define IsDrawingReadOnly(d)	(d->ptr->status & DG_READ_ONLY)

#define byte unsigned char
typedef  float  FltCoord;       	/* float coordinate*/
typedef  double DblCoord;       	/* double precision coordinate*/

typedef  struct { FltCoord x,y; } FltPoint;		/* float point*/
typedef  struct { DblCoord x,y; } DblPoint;		/* double point*/

typedef  struct { FltPoint lo[1],hi[1]; } FltExtent; 	/* float extent */
typedef  struct { DblPoint lo[1],hi[1]; } DblExtent;	/* double extent */

#define LISTHDR           /*LIST HEADER*/ \
	byte type;       /*type of object*/        \
	byte l_size;     /*index to size of list*/       \
	byte undo_mask;  /* see comments for "undo_mask" below */\
	unsigned short  l_num;    /*no. of entries*/    \
	char  **l_first;                /*pointer to array*/

typedef struct list {
	LISTHDR
} LIST;

#define FORLIST(i,l,a,c) {if((l)) for((i)=0; (i)<(int)((l)->l_num);(i)++){\
                        (a) = (c) *(((l)->l_first) + (i));
#define REVLIST(i,l,a,c) {if((l)) for((i)=((l)->l_num-1); (i) >= 0; (i)--){\
		        (a) = (c) *( ((l)->l_first) + (i) ) ;
#define ENDFOR }}

/*
 * geometric header
 */
#define ATOMHDR				/*ATOM HEADER*/			       \
	LISTHDR				/* list header*/		       \
	byte status;			/* status flag for dbmgr use*/	       \
	byte level;			/* level */			       \
	unsigned short segno;		/*segment number for 41xx terminal*/   \
	unsigned int sn;		/* serial # of element */	       \
	FltExtent extent[1];            /* extent of atom*/		       \
	struct atom *parent;            /* pointer to parent object */

typedef struct {
        double scale;                   /* scale factor (double wrd alignment)*/
        DblPoint offset[1];             /* offset */
        int orient;                     /* orientation matrix index*/
} Transform;

/*
 * ATOM and ATOM structure
 */
typedef struct {                        /* universal header     */
        ATOMHDR
} ATOM;

typedef  struct {
        Transform  trs[1];      /* atom transformation data */
        ATOM *ptr;              /* atom pointer */
}  DATA;
#define DATASIZ  (sizeof(DATA))

typedef struct {
	char *symbolName;
	char *terminalName;
	char *terminalNumber;
	char *netName;
} EXPANDED_CONNECTION_DATA;

typedef struct {
	DATA *object;
	char *objectName;
	DATA *terminal;
	char *terminalName;
	char *terminalNumber;
	int status;
	LIST *expandedConnections;
} CONNECTION_DATA;

typedef  struct {
	float x,y;		/*coordinates*/
}  COORD;
#define COORDSIZ  			(sizeof(COORD))

#define DataObject(data)  		(data->ptr)
#define SetDataObject(data,object)  	(data->ptr = (ATOM*)object)
#define Lo(ext)				((ext)->lo)	
#define Hi(ext)				((ext)->hi)	
#define PoX(pt)				((pt)->x)
#define PoY(pt)				((pt)->y)
#define LoX(pt)				((pt)->lo->x)	
#define LoY(pt)				((pt)->lo->y)	
#define HiX(pt)				((pt)->hi->x)
#define HiY(pt)				((pt)->hi->y)
#define ExtentsDontOverlap(e1,e2)	(LoX((e2)) > HiX((e1)) ||	\
					 LoX((e1)) > HiX((e2)) || 	\
					 LoY((e2)) > HiY((e1)) || 	\
					 LoY((e1)) > HiY((e2)))
#define IsSameExtent(e1,e2)		(LoX((e1)) == LoX((e2)) && 	\
					 LoY((e1)) == LoY((e2)) && 	\
					 HiX((e1)) == HiX((e2)) && 	\
					 HiY((e1)) == HiY((e2)))
#define IsSamePoint(p1,p2)		(PoX((p1)) == PoX((p2)) && 	\
					 PoY((p1)) == PoY((p2)))
#define IsPointInBox(p,e)	(!(p[0].x < e[0].x || p[0].y < e[0].y  || \
					p[0].x > e[1].x || p[0].y > e[1].y))

#define min(x,y)	(((x) < (y)) ? (x) : (y))
#define max(x,y)	(((x) > (y)) ? (x) : (y))

#define EXTENTS_EQUAL			1
#define EXTENTS1_INSIDE_EXTENTS2	2
#define EXTENTS2_INSIDE_EXTENTS1	3
#define EXTENTS_OVERLAP			4
#define EXTENTS_DONT_OVERLAP		5

#define NOFLIP  0	
#define ROT0	0	/* Rotate 0 degrees,			( x, y)	*/
#define FLIPY	1	/* Flip about y axis,			(-x, y)	*/
#define FLIPX	2	/* Flip about x axis,			( x,-y)	*/
#define ROT180	3	/* Rotate 180 degrees,			(-x,-y)	*/
#define R90FY  	4	/* Rotate 90, then flip about y axis,	( y, x)	*/
#define ROT270	5	/* Rotate 270 degrees,			( y,-x)	*/
#define ROT90	6	/* Rotate 90 degrees,			(-y, x)	*/
#define R90FX	7	/* Rotate 90, then flip about x axis,	(-y,-x)	*/

/* valid keywords for s_import() var arg list */
typedef enum eps_import_keyword {
	EPS_IMPORT_NULL = 0, 	/* used to terminate variable arg. list */
	EPS_AS_CAPTURED = 1,
	EPS_SWEEP = 2,
	EPS_CUSTOM = 3,
	EPS_SCALE = 4,
	EPS_WIDTH = 5,
	EPS_HEIGHT = 6,
	EPS_ROTATION = 7,
	EPS_FLIP = 8,
	EPS_AT_COORD = 9,
        /* keywords used to indicate that options were not explicitly specified 
           on the command line; used in the re-IMPORT functionality so that 
	   current settings will be used instead of the default values
        */
	EPS_SCALE_UNDEF	= -4,
	EPS_WIDTH_UNDEF	= -5,
	EPS_HEIGHT_UNDEF = -6,
	EPS_ROTATION_UNDEF = -7,
	EPS_FLIP_UNDEF = -8,	
	EPS_AT_COORD_UNDEF = -9,
	EPS_SIZE_UNDEF = -10
} EPS_IMPORT_KEYWORD;

/* valid keywords for s_export() var arg list */
typedef enum export_keyword {
	EXPORT_NULL = 0,	/* used to terminate variable arg. list */
	EXPORT_EPS = 1,
	EXPORT_DESIGN_INTENT = 2,
	EXPORT_OUTPUT = 3,
	EXPORT_NOCOLOR = 4,
	EXPORT_COLOR = 5,
	EXPORT_OUT_WIDTH = 6,
	EXPORT_OUT_HEIGHT = 7,
	EXPORT_MAXIMUM_WINDOW = 8,
	EXPORT_CURRENT_WINDOW = 9,
	EXPORT_SUBCIRCUIT = 10,
	EXPORT_ERROR = 11,
        /* keywords used to indicate that options were not explicitly specified 
           on the command line; used for validation of command line invocation
	 */
	EXPORT_OUT_WIDTH_UNDEF = -5,
	EXPORT_OUT_HEIGHT_UNDEF = -6
} EXPORT_KEYWORD;

/* keyword for special EPS file name attribute */
#define EPS_FILENAME_FIELD	"epsfilename"

/* keywords to indicate EXPORT or IMPORT format */
#define IMPORT_EPS 		1

/* EPS EXPORT limits */
#define EPS_EXPORT_MAX_WIDTH_IN_INCHES	455
#define EPS_EXPORT_MAX_HEIGHT_IN_INCHES	455

typedef enum ps_sheet_size {
	PS_8x11 = 1,	/* use 8x11 size sheet */
	PS_11x17 = 2 	/* use 11x17 size sheet */
} PS_SHEET_SIZE;

/***************************************************************************/
/* standard C library functions                                            */
/***************************************************************************/
extern int abs();
extern int access();
extern float atof();
extern int atoi();
extern char *calloc();
extern char *fgets();
extern char *free();
/* In Solaris, fprintf(), fscanf(), and sscanf() are declared in stdio.h */
#ifndef SOLARIS
extern int strlen();
extern int fprintf();
extern int fscanf();
extern int sscanf();
#endif
extern char *getenv();
extern char *malloc();
extern char *realloc();
extern double sqrt();
extern double fabs();
extern char *strchr();
extern char *strcat();
extern int strcmp();
extern char *strcpy();
extern char *strrchr();

/***************************************************************************/
/* schema functions                                                        */
/***************************************************************************/
extern int reset_glbl_idlist();
extern int s_atmcmd();
extern int s_button();
extern char *s_cad_file();
extern int s_command();
extern int s_compare();
extern Cui_object s_createbitmap();
extern int s_curgroup();      /* returns pointer to current SCHEMA group */
extern int s_curve();		/* calculate bezier or spline curve */
extern int s_echo();
extern int s_execute();
extern void s_exit();
extern int s_expand();
extern int s_expcheck();
extern int s_fclose();
extern int s_fflush();
extern char *s_findZoneOf();	/* find the zone of the specified object */
extern int s_flushstate();	/* undo cancel snapshot synchronization */
extern FILE *s_fopen();
extern char *s_functionkey();	/* Define function key command-actions */
extern int s_getatom();		/* gets an atom from the specified group */
extern int s_getatomat();	/* gets an atom at the specified point */
extern int s_getcwd();		/* returns pointer to current work directory */
extern int s_getgroup();	/* enables the specified group */
extern int s_getid();
extern int s_getinfo();
extern int s_getlist();		/* gets an atom from the specified worklist */
extern float s_getvar();	/* returns current value of a SCHEMA variable*/
extern int s_getwire();
extern int s_highlight();
extern int s_id();
extern int s_inquire();		/* gets info on atom, group, or list */
extern int s_makemenu();
extern int s_palette();
extern int s_point();
extern int s_popen2();
extern int s_postscript();
extern int s_printf();
extern int s_probe();	/* ADVICE probe points panel */
extern char *s_prompt();
extern int s_pulldown();
extern int s_savestate();	/* undo snapshot explicit syncronization func */
extern int s_select();
extern int s_selectobject();
extern int s_smcode();
extern int s_stracmp();
extern int s_strancmp();
extern char *s_stracpy();
extern char *s_strancpy();
extern char *s_strexpr();
extern int s_system();
extern int s_tmpline();
extern int s_tokens();
extern int s_tolower();
extern int s_toupper();
extern int s_unhighlight();
extern int s_wavedit();		/* ADVICE waveform editor */
extern int s_wirit();
extern int s_wirstub();
extern int s_sizeOfList();
extern LIST *s_initList(), *s_createList(), *s_freeList();

/***************************************************************************/
/* inter-process stuff                                                     */
/***************************************************************************/
extern int is_process_active();	/* Check if process is active */
extern int start_process();	/* start process in a safe window */
extern int stop_process();	/* stop process */
extern int send_process();	/* send process a command */
extern int show_processes();	/* show active processes */

/***************************************************************************/
/* panel stuff                                                             */
/***************************************************************************/
extern int cui_popen();
extern int cui_pclose();
extern int sch_cui_pexec();
extern int cui_ptitle();
extern int cui_ptext();
extern int cui_pexists();
extern int cui_paction();
extern int cui_ptoggle();
extern int cui_pscroll();
extern int cui_pkeybrd();
extern int cui_pchoice();
extern int cui_pcbegin();
extern int cui_pcadd();
extern int cui_pcend();
extern int cui_pbool();
extern int cui_position();

/***************************************************************************/
/* clipboard stuff                                                         */
/***************************************************************************/
extern int s_addicon();
extern int s_initclipboard();
extern int s_showclipboard();

/***************************************************************************/
/* define global variables for all to use                                  */
/***************************************************************************/
extern DATA dummy[];
extern COORD dumxy[];
extern int sch4100, schpanel, schbuttonrow;
extern float pierce;
extern char *schcp;

/***************************************************************************/
/* define functions from master function file to be made available for all */
/***************************************************************************/
extern char *_schdeflt(), *_restralloc();
extern int _idonly(), _idname(), _isstring(), _getDesignDirectory();
extern DATA *_find_parent();
extern void _copydata();

/***************************************************************************/
/* define constants for return values and error messages                   */
/***************************************************************************/

#define E_OK		0
#define E_NOT_ON_LIST	-1
#define E_INCOMP	-105
#define E_NOCMD		-106	/*unknown command*/
#define E_NOOPT		-107	/*no option*/
#define E_NOGROUP	-108	/*group not found*/
#define E_NOFILE	-109	/*file not found*/
#define E_NOOPEN	-110	/*file not opened*/
#define E_NOMEM		-111	/*no more memory available*/
#define E_NOHELP	-112	/*help file not found*/
#define E_BADDATA	-113	/*bad data base*/
#define E_NULLCMD	-114	/*null command*/
#define E_NOLICENSE	-115	/*unlicensed command*/
#define E_OVERLAP       -116    /*overlapped terminals */
#define E_NOCONNECTION  -117	/*no more connections */

#define NEED_PAG_SYM "This command needs a page or symbol. First type 'EDIT PAGE page_name' or 'EDIT SYMBOL symbol_name'.\n"
#define NEED_PAGE "This command needs a page. First type 'EDIT PAGE page_name'.\n"
#define NEED_SYMBOL "This command needs a symbol. First type 'EDIT SYMBOL symbol_name'.\n"
#define MASTER "%s is a Read-Only %s and cannot be modified.\n"
#define EPS_EXPORT_NEED_POSITIVE_WIDTH \
	"\007Error:  Positive width (in inches) is expected to follow the keyword WIDTH.\n"
#define EPS_EXPORT_NEED_POSITIVE_HEIGHT \
	"\007Error:  Positive height (in inches) is expected to follow the keyword HEIGHT.\n"
#define EPS_EXPORT_OVER_MAX_WIDTH_IN_INCHES \
	"\007Error:  Maximum EXPORT width is %d inches.\n"
#define EPS_EXPORT_OVER_MAX_HEIGHT_IN_INCHES \
	"\007Error:  Maximum EXPORT height is %d inches.\n"
#define EPS_IMPORT_NEED_POSITIVE_WIDTH \
	"\007Error:  Positive width (in user units) is expected to follow the keyword WIDTH.\n"
#define EPS_IMPORT_NEED_POSITIVE_HEIGHT \
	"\007Error:  Positive height (in user units) is expected to follow the keyword HEIGHT.\n"
#define EPS_IMPORT_NEED_COORD \
	"\007Error:  Both x and y coordinate values are expected to follow the keyword @CO.\n"
#define EPS_IMPORT_NEED_POSITIVE_SCALE \
	"\007Error:  Positive scale value is expected to follow the keyword SCALE.\n"
#define EPS_IMPORT_NEED_STANDARD_ANGLE \
	"\007Error:  Angle of rotation must be 0, 90, 180, or 270 degrees.\n"
#define EPS_IMPORT_NEED_FLIP_AXIS \
	"\007Error:  Flips can only be done on the X or Y axis.\n"
#define INPUT_UNRECOGNIZED_AND_IGNORED \
	"The following input is unrecognized and is ignored: %s\n"

/***************************************************************************/
/* define constants/macros for string manipulation			   */
/***************************************************************************/
#define NO		0
#define YES		1
#define OK		0
#ifdef NORMAL
#undef NORMAL
#endif
#define NORMAL		0
#define LOWER		1
#define UPPER		2
#define INVERT		3
#define stralloc(s)	strcpy((char*)malloc((unsigned)(strlen(s)+1)),s)
#define StripExtender(gname)                                                   \
	{ char *cp; if((cp=strrchr(gname,'.')) != NULL) *cp = NULL; }
#define catTok(buff,tok,case) \
	{ if(*buff) strcat(buff," "); s_stracpy(buff+strlen(buff),tok,case); }
#define ABSTRACT	0
#define PHYSICAL_OLD 	1
#define PHYSICAL_NEW 	2

/***************************************************************************/
/* define capacity of undo & redo stacks				   */
/***************************************************************************/
#define UNDOSIZE	4

/***************************************************************************/
/* define constants for atom types                                         */
/***************************************************************************/
#define SEGMENT		1
#define DFGP		5
#define PLGP		6
#define RDGP		7
#define ARC		8
#define POLY		9
#define CIRC		10
#define LINE		11
#define PIN		12
#define RECT		13
#define PTEXT		14
#define UTEXT		15
#define TTEXT		16
#define WIRE		17
#define SUBNET		18
#define JUNCTION	21
#define COMPLEX_SHAPE	22
#define EPSF		23
#define ANY_ATOM	100

#define LEGAL		0
#define ILLEGAL		1

#define CONN_VIRTUAL	1
#define CONN_REAL	2
#define CONN_ILLEGAL	3

#define isSegment(ap)   ((ap)->ptr->type == SEGMENT)
#define isDfgp(ap)      ((ap)->ptr->type == DFGP)
#define isPlgp(ap)      ((ap)->ptr->type == PLGP)
#define isRdgp(ap)      ((ap)->ptr->type == RDGP)
#define isSymbol(ap)	((ap)->ptr->type == PLGP || (ap)->ptr->type == RDGP)
#define isArc(ap)       ((ap)->ptr->type == ARC)
#define isPoly(ap)      ((ap)->ptr->type == POLY)
#define isCirc(ap)      ((ap)->ptr->type == CIRC)
#define isLine(ap)      ((ap)->ptr->type == LINE)
#define isPin(ap)       ((ap)->ptr->type == PIN)
#define isRect(ap)      ((ap)->ptr->type == RECT)
#define isText(ap)      ((ap)->ptr->type == PTEXT||(ap)->ptr->type == UTEXT|| \
						   (ap)->ptr->type == TTEXT)
#define isPtext(ap)     ((ap)->ptr->type == PTEXT)
#define isUtext(ap)     ((ap)->ptr->type == UTEXT)
#define isTtext(ap)     ((ap)->ptr->type == TTEXT)
#define isWire(ap)      ((ap)->ptr->type == WIRE)
#define isSubnet(ap)    ((ap)->ptr->type == SUBNET)
#define isJunction(ap)  ((ap)->ptr->type == JUNCTION)
#define isComplexShape(ap) ((ap)->ptr->type == COMPLEX_SHAPE)
#define isEPSF(ap) 	((ap)->ptr->type == EPSF)

#define IsSunTerminal(t)	(s_strancmp((t),"x",1,LOWER) == 0)
#define IsSunXTerminal(t)	(IsSunTerminal(t))
#define IsTek41Terminal(t)	(s_strancmp((t),"41",2,LOWER) == 0)
#define IsAsciiTerminal(t)	(*(t) == '0')
#define IsHPTerminal(t)		(s_strancmp((t),"26",2,LOWER) == 0)
#define IsPanelTerminal(t)	(IsHPTerminal(t) || IsSunTerminal(t) || \
			IsTek41Terminal(t))

/**************************************************************************/
/* define data base traversal macros					  */
/**************************************************************************/
#define FORPAGE(gp,page,subcircuit,draw)				       \
	{ int lastpage=0,zz=0; for(zz=1;(zz-lastpage)<100;zz++) {	       \
		(void)sprintf(page,"%s.%d",subcircuit,zz);		       \
		if(s_getgroup(gp,page,draw))				       \
			continue;
#define ENDPAGE         lastpage = zz; }}
#define FORATOM(ap,gp,i) { for((i)=1;!s_getatom((ap),(gp),(i));(i)++) {
#define ENDATOM         }}
#define FORWIRE(ap,snp,i) { if(snp->ptr->type == SUBNET) {		       \
		for((i)=1;!s_getatom((ap),(snp),(i));(i)++) {		       \
			if(ap->ptr->type != WIRE) continue;
#define ENDWIRE         }}}
#define DESIGN		0
#define WRITE_READ	1
#define READ_ONLY	2
#define S_ADD		0
#define S_INQUIRE	1
#define FORLINKEDDIR(dtype,buff) {					       \
	FILE *lfp;							       \
	char dbuff[BUFFSIZE], *tk[BUFFSIZE];				       \
	if((lfp = fopen("linkdir.sch","r")) != NULL) {			       \
		while(fgets(dbuff,BUFFSIZE,lfp) != NULL) {		       \
			if(s_tokens(dbuff,tk,50) == 3) {		       \
				(void)strcpy(buff,tk[1]);			       \
				dtype = atoi(tk[2]);
#define ENDLINKEDDIR    }} (void)fclose(lfp);}}

/**************************************************************************/
/* define constants for FUNCTION KEYS                                     */
/**************************************************************************/
#define SET	1
#define GET	2
#define UNSHIFTED	CUI_NONE
#define SHIFTED		CUI_SHIFT

/* 
 * define constants for ASSIGN
 */
#define DEFAULT_GRID_SNAP 	12
#define DEFAULT_STUB_LENGTH  	180	
#define WIRENAME 		0
#define TERMSEQ			1
#define TERMNO			2
#define MAX_TRM_EXPAND		10000
#define PHYSICAL		3
#define PHYSICAL_INTERNAL	2	
#define PHYSICAL_EXTERNAL 	1	
#define TRMNO_GAP		30
#define WIRENAME_TERMSEQ_MISMATCH	1
#define WIRENAME_TERMNO_MISMATCH	2
#define TERMSEQ_TERMNO_MISMATCH		3
#define printStringError(key,string) { \
		s_printf("Invalid %s specified <%s>\n",key,string); \
		return(E_INCOMP); \
	}

/***************************************************************************/
/* define other constants                                                  */
/***************************************************************************/
#define GRP_NM_SZ       (256)
#define BUFFSIZE	(256)

	/* the following constants are used by s_selectobject() */
#define SELECT_APPEND 0
#define SELECT_REMOVE 1
#define SELECT_NEW 2
#define SELECT_PURGE 3
#define SELECT_TOGGLE 4

/*      used for menu and popup initialization */
#define M_ALL           (0)
#define M_MAIN          (1)
#define M_GLOBAL        (2)
#define M_EDIT1         (3)
#define M_EDIT2         (4)
#define M_FILEMGT       (5)
#define M_LSTCNTRL      (6)
#define M_OUTPUT        (7)
#define M_SETUP         (8)
#define M_TOOLS         (9)
#define M_SYMEDIT       (10)
#define M_LSTEDIT       (11)
#define M_MASTER1       (12)
#define M_USER          (13)
#define M_WINDOW1       (14)
#define M_WINDOW2       (15)
#define M_WINDOW        (16)
#define M_PALETTE       (17)
#define M_ADD           (18)
#define M_OTHER         (19)
#define P_EDIT          (20)
#define P_FILE          (21)
#define P_LIST          (22)
#define P_OUTPUT        (23)
#define P_SETUP         (24)
#define P_TOOLS         (25)
#define P_SYMEDT        (26)
#define P_FUNCS         (27)
#define M_TOOLS_A       (28) /* Alternate Menus */
#define P_TOOLS_A       (29)
#define M_UNDO          (30)
#define M_BUF           (31)
#define P_CP            (32)
#define M_FPGA		(33)
#define P_DDI		(34)
#define P_SIMULATION 	(35)

/* PANEL CONTROL ITEMS RETURN VALUES */

#define NEXT_PANEL              30      /* display next panel */
#define HELP_PANEL              31      /* display the help panel */
#define PREV_PANEL              32      /* display the previous panel */

/* s_graphicalUpdate() constants */
#define NOGRAPH_NOREDRAW        0
#define NOGRAPH_REDRAW          1
#define GRAPH_NOREDRAW          2
#define GRAPH_REDRAW            3

/* s_audit() constants */

typedef enum auditOperation {
	AUDIT_DEFINE, AUDIT_ADD, AUDIT_CHANGE, AUDIT_RUN, AUDIT_OUTPUT,
	AUDIT_RESET, AUDIT_QUERY
} AUDIT_OPERATION;

typedef enum auditSeverity {
	AUDIT_WARNING, AUDIT_ERROR
} AUDIT_SEVERITY;

typedef enum auditChangeOperation {
	AUDIT_CHANGE_NOTHING, AUDIT_TYPE, AUDIT_BANNER
} AUDIT_CHANGE_OPERATION;

typedef enum auditDisplay {
	AUDIT_ONE, AUDIT_MANY, AUDIT_WITH_SUMMARY, AUDIT_WITHOUT_SUMMARY
} AUDIT_DISPLAY;

typedef enum auditScope {
	AUDIT_GLOBAL, AUDIT_SUBCIRCUIT, AUDIT_PAGE, AUDIT_SYMBOL
} AUDIT_SCOPE;

#define AUDIT_ADVICE            (01)
#define AUDIT_CMPLIB            (02)
#define AUDIT_DDB               (04)
#define AUDIT_DTL               (010)
#define AUDIT_FPDL              (020)
#define AUDIT_LSL               (040)
#define AUDIT_SCF               (0100)
#define AUDIT_MANUAL            (0200)

#define AUDIT_ACTIVE_FOR_ALL	(AUDIT_ADVICE|AUDIT_CMPLIB|AUDIT_DDB|AUDIT_DTL|\
				 AUDIT_FPDL|AUDIT_FPDL|AUDIT_LSL|AUDIT_SCF)

#define AT_LOCATION_VIOLATION	"%s at location %s"

#define FOUR_WAY_BANNER \
	"The following locations have a JUNCTION with a FOUR-WAY TIE POINT:"
#define UNCONNECTED_TERMINAL_BANNER \
	"The following symbols have an UNCONNECTED TERMINAL:"
#define SYMBOL_NAME_LENGTH_BANNER \
	"The following SYMBOL NAMES EXCEED THE %d CHARACTER LIMIT:"
#define WIRE_NAME_LENGTH_BANNER \
	"The following WIRE NAMES EXCEED THE %d CHARACTER LIMIT:"
#define WIRE_END_DISTANCE_BANNER \
	"WIRES at the following locations MAY NEED A CONNECTION:"

/***************************************************************************/
/* color/fill								   */
/***************************************************************************/
#define DEFAULT_COLOR 	"DEFAULT_COLOR"

/* fill pattern types */
#define SOLID_FILL 	0
#define BITMAP 		1

/* color modes */
#define OPAQUE_MODE 		0
#define BLEND_MODE		1
#define TRANSPARENT_MODE	2

/* plot sequencing */
#define SORT_NORMAL	0
#define SORT_BY_DEPTH	1

/* escape handling */

#define PUSH		1
#define POP		2

/* overview window settings */

#define OVERVIEW_DISABLED		0
#define OVERVIEW_SEPARATE		1
#define OVERVIEW_EMBEDDED		2
#define OVERVIEW_DISABLED_STRING	"0"
#define OVERVIEW_SEPARATE_STRING	"1"
#define OVERVIEW_EMBEDDED_STRING	"2"

#define RESETID 1
#define RESETARGS 2

