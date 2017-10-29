/*	static char h_errmsg[] = "@(#) errmsg.h:  5.1 2/22/83";	*/
/* errmsg.h -- define error messages and routines */
/* this must be preceded by errors.h */

/* routines */

void terrNUL();		/* message without other stuff */
void terrCHR();		/* message with character */
void terrTB();		/* message with text block */
void terrSTR();		/* message with string */
void terrWRN();		/* warning message */

/* messages */
/* there must be a one to one correspondence between these and the
 * definitions in errmsg.c
 */

extern struct TER Err_BNI;
extern struct TER Err_EBF;
extern struct TER Err_FER;
extern struct TER Err_ICE;
extern struct TER Err_IEC;
extern struct TER Err_IFC;
extern struct TER Err_IIA;
extern struct TER Err_ILL;
extern struct TER Err_ILN;
extern struct TER Err_INP;
extern struct TER Err_IQC;
extern struct TER Err_IQN;
extern struct TER Err_IRA;
extern struct TER Err_ISA;
extern struct TER Err_ISS;
extern struct TER Err_IUC;
extern struct TER Err_MAP;
extern struct TER Err_MEM;
extern struct TER Err_MLP;
extern struct TER Err_MRA;
extern struct TER Err_MRP;
extern struct TER Err_NAB;
extern struct TER Err_NAC;
extern struct TER Err_NAE;
extern struct TER Err_NAP;
extern struct TER Err_NAQ;
extern struct TER Err_NAS;
extern struct TER Err_NAU;
extern struct TER Err_NCA;
extern struct TER Err_NFI;
extern struct TER Err_NFO;
extern struct TER Err_NPA;
extern struct TER Err_NYA;
extern struct TER Err_NYI;
extern struct TER Err_OFO;
extern struct TER Err_OUT;
extern struct TER Err_PDO;
extern struct TER Err_PES;
extern struct TER Err_POP;
extern struct TER Err_PTL;
extern struct TER Err_SNI;
extern struct TER Err_SRH;
extern struct TER Err_STL;
extern struct TER Err_TAG;
extern struct TER Err_UTC;
extern struct TER Err_UTM;
extern struct TER Err_VNF;
extern struct TER Err_XAB;
extern struct TER Err_YCA;

extern struct TER Wrn_BAK;
extern struct TER Wrn_FNM;
extern struct TER Wrn_SFL;
extern struct TER Wrn_SUP;
extern struct TER Wrn_SAV;
