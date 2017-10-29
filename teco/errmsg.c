static char SCCSID[] = "@(#) errmsg.c:  5.1 2/22/83";
/* errmsg.c
 *
 *	TECO error message text
 *
 *	David Kristol, February, 1982
 *
 *	The form for these messages depends on the structure defined
 *	in errors.h .  Each consists of two parts, a short form and
 *	a longer form.  The % character marks where a substitution
 *	takes place.  Note that an entry must be made to errmsg.h
 *	when a message is added here.
 */

#include "errors.h"

struct TER Err_BNI={ ErrNUL, "BNI",	"> not in iteration"};
struct TER Err_EBF={ ErrNUL, "EBF",	"Bad EB file name"};
struct TER Err_FER={ ErrSTR, "FER",	"File error, file %"};
struct TER Err_ICE={ ErrCHR, "ICE",	"Illegal ^E Command in Search Argument \"%\""};
struct TER Err_IEC={ ErrCHR, "IEC",	"Illegal character \"%\" after E"};
struct TER Err_IFC={ ErrCHR, "IFC",	"Illegal character \"%\" after F"};
struct TER Err_IIA={ ErrNUL, "IIA",	"Illegal insert arg"};
struct TER Err_ILL={ ErrCHR, "ILL",	"Illegal command \"%\""};
struct TER Err_ILN={ ErrNUL, "ILN",	"Illegal number"};
struct TER Err_INP={ ErrNUL, "INP",	"Input error"};
struct TER Err_IQC={ ErrCHR, "IQC",	"Illegal \" character \"%\""};
struct TER Err_IQN={ ErrCHR, "IQN",	"Illegal Q-register name \"%\""};
struct TER Err_IRA={ ErrNUL, "IRA",	"Illegal radix argument to ^R"};
struct TER Err_ISA={ ErrNUL, "ISA",	"Illegal search argument"};
struct TER Err_ISS={ ErrNUL, "ISS",	"Illegal search string"};
struct TER Err_IUC={ ErrCHR, "IUC",	"Illegal character \"%\" following ^"};
struct TER Err_MAP={ ErrNUL, "MAP",	"Missing '"};
struct TER Err_MEM={ ErrNUL, "MEM",	"Memory overflow"};
struct TER Err_MLP={ ErrNUL, "MLP",	"Missing ("};
struct TER Err_MRA={ ErrNUL, "MRA",	"Missing >"};
struct TER Err_MRP={ ErrNUL, "MRP",	"Missing )"};
struct TER Err_NAB={ ErrNUL, "NAB",	"No arg before ^_"};
struct TER Err_NAC={ ErrNUL, "NAC",	"No arg before ,"};
struct TER Err_NAE={ ErrNUL, "NAE",	"No arg before ="};
struct TER Err_NAP={ ErrNUL, "NAP",	"No arg before )"};
struct TER Err_NAQ={ ErrNUL, "NAQ",	"No arg before \""};
struct TER Err_NAS={ ErrNUL, "NAS",	"No arg before ;"};
struct TER Err_NAU={ ErrNUL, "NAU",	"No arg before U"};
struct TER Err_NCA={ ErrNUL, "NCA",	"Negative argument to ,"};
struct TER Err_NFI={ ErrNUL, "NFI",	"No file for input"};
struct TER Err_NFO={ ErrNUL, "NFO",	"No file for output"};
struct TER Err_NPA={ ErrNUL, "NPA",	"Negative or 0 argument to P"};
struct TER Err_NYA={ ErrNUL, "NYA",	"Numeric argument with Y"};
struct TER Err_NYI={ ErrNUL, "NYI",	"Not Yet Implemented"};
struct TER Err_OFO={ ErrSTR, "OFO",	"Output file already open:  \"%\""};
struct TER Err_OUT={ ErrNUL, "OUT",	"Output error"};
struct TER Err_PDO={ ErrNUL, "PDO",	"Push-down list overflow"};
struct TER Err_PES={ ErrNUL, "PES",	"Attempt to Pop Empty Stack"};
struct TER Err_POP={ ErrSTR, "POP",	"Attempt to move Pointer Off Page with\
 \"%\""};
struct TER Err_PTL={ ErrTB,  "PTL",	"Pathname too long \"%\""};
struct TER Err_SNI={ ErrNUL, "SNI",	"; not in iteration"};
struct TER Err_SRH={ ErrTB,  "SRH",	"Search failure \"%\""};
struct TER Err_STL={ ErrTB,  "STL",	"String too long \"%\""};
struct TER Err_TAG={ ErrTB,  "TAG",	"Missing Tag !%!"};
struct TER Err_UTC={ ErrCHR, "UTC",	"Unterminated command \"%\""};
struct TER Err_UTM={ ErrNUL, "UTM",	"Unterminated macro"};
struct TER Err_VNF={ ErrSTR, "VNF",	"Environment variable \"%\" not found"};
struct TER Err_XAB={ ErrNUL, "XAB",	"Execution aborted"};
struct TER Err_YCA={ ErrNUL, "YCA",	"Y command aborted"};


/* these are warning messages */

struct TER Wrn_BAK={ ErrWRN, "No backup:  name too long"};
struct TER Wrn_FNM={ ErrWRN, "Filename contains non-graphic characters"};
struct TER Wrn_SFL={ ErrSWRN, "Search fail in iter"};		/* special !! */
struct TER Wrn_SUP={ ErrWRN, "Superseding existing file"};
struct TER Wrn_SAV={ ErrWRN, "Command buffer empty or already saved"};
