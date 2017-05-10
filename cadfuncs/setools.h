/* static char *h_setools="@(#)setools.h	1.3";	/* *  12/6/93  15:23:57 */

#ifndef setools_DEFINED
#define	setools_DEFINED
#include <sys/types.h>

#ifndef ON
#define ON	1
#endif
#ifndef OFF
#define OFF	0
#endif
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/* BEGINNING OF GIN HEADER INFORMATION */

/*
 *	These are the recognized GIN_MODEs of cui_gin() & cui_graphics_input
 *	They are set using cui_set_ginmode()
 */
#define NO_GIN			0

#ifndef NORMAL
#endif
					   /*     I M P O R T A N T     */
	/* These numeric assignments must NOT be changed.  Changing any */
	/* may cause existing journals, which use them, to break.	*/
	/* New MODES may safely be added to the end of the list.	*/
					   /*     I M P O R T A N T     */
#define RB_MANHATTAN_AUTO_X	2
#define RB_MANHATTAN_AUTO_Y	3
#define RB_MANHATTAN_X_FIRST	4
#define RB_MANHATTAN_Y_FIRST	5
#define RB_X_ONLY		6
#define RB_Y_ONLY		7
#define RB_LINE			8
#define DRAG_OBJECT		9
#define RB_CIRCLE		10
#define RB_RECTANGLE		11
#define MOUSE_STILL		12
#define RB_45_INCREMENT		13
#define MAX_MODE		13

	/* This is the return code when more than 1 mode is used. */
#define RB_COMPOUND		99

	/* The following are pseudo-GINmodes, or GIN-mode modifiers */
#define CUI_GINAPPEND		((int)'A')	/* decimal 65 */
#define CUI_GINRESET		((int)'R')	/* decimal 82 */
#define CUI_XLIMIT		((int)'X')	/* decimal 88 */
#define CUI_YLIMIT		((int)'Y')	/* decimal 89 */

/* END OF GIN HEADER INFORMATION */


/*************/
/* Highlight */
/*************/
#define HL_ALL	0	/* Code used for indicating ALL highlight-objects */

/**************/
/* color mode */
/**************/
#define OPAQUE		'0'	/* sets colormap and draw mode to opaque */
#define ADDITIVE	'1'	/* sets colormap and draw mode to additive */
#define OPAQUE_COLORMAP		'2'	/* just affects the colormap */
#define ADDITIVE_COLORMAP	'3'	/* just affects the colormap */
#define OPAQUE_DRAW		'4'	/* just affects the draw mode */
#define ADDITIVE_DRAW		'5'	/* just affects the draw mode */

/* BEGINNING OF ATTRIBUTE DEFINITIONS NEEDED FOR PANEL AND WINDOW ATTRIBUTES */

#define ATTR_PKG_FIRST		1
#define ATTR_PKG_PANEL		( ATTR_PKG_FIRST + 0 )
#define ATTR_PKG_WINDOW		( ATTR_PKG_FIRST + 1 )
#define ATTR_PKG_LAST		( ATTR_PKG_WINDOW )

#define ATTR_NONE	0
#define ATTR_RECURSIVE	1
#define ATTR_NULL	2
#define ATTR_COUNTED	3

#define ATTR(pkg, type, ordinal)	\
    ( ((((unsigned)(pkg))	& 0x7f) << 24) | \
      ((((unsigned)(ordinal))	& 0xff) << 16) | \
       (((unsigned)(type))	& 0xefef)	)

#define ATTR_LIST_OF(list_type, list_ptr_type, base_cardinality) \
    (((((unsigned)(list_type)) & 0x3) << 14) | \
     (((unsigned)(list_ptr_type) & 0x1) << 13) | \
     (((unsigned)(base_cardinality)) & 0x3fff))

#define ATTR_LIST_IS_INLINE	0

#define ATTR_LIST_INLINE(list_type, base_cardinality)	\
    ATTR_LIST_OF(list_type, ATTR_LIST_IS_INLINE, base_cardinality)

#define	ATTR_CODE(attr)		((unsigned) (attr))
#define ATTR_ORDINAL(attr)      \
	((unsigned) ((ATTR_CODE(attr) >> 16) & 0xFF))

#define ATTR_BASE_CHAR						0
#define ATTR_BASE_SHORT						1
#define ATTR_BASE_INT						2
#define ATTR_BASE_UNSIGNED_CHAR					3
#define ATTR_BASE_UNSIGNED_SHORT				4
#define ATTR_BASE_UNSIGNED_INT					5
#define ATTR_BASE_INT_INT					6
#define ATTR_BASE_INT_INT_INT					7
#define ATTR_BASE_LONG						8
#define ATTR_BASE_FLOAT						9
#define ATTR_BASE_DOUBLE					10
#define ATTR_BASE_STRING					11
#define ATTR_BASE_FUNCPTR					12
#define ATTR_BASE_OPAQUE					13
#define ATTR_BASE_NO_VALUE					14
#define ATTR_BASE_ENUM						15
#define ATTR_BASE_BOOLEAN					16
#define ATTR_BASE_INT_STRING					17
#define ATTR_BASE_INT_INT_STRING				18
#define ATTR_BASE_OPAQUE_STRING_INT				19
#define ATTR_BASE_OPAQUE_OPAQUE_STRING				20
#define ATTR_BASE_FUNCPTR_INT					21
#define ATTR_BASE_INT_OPAQUE_STRING				22
#define ATTR_BASE_STRING_INT_INT				23
#define ATTR_BASE_INT_STRUCT_STRING_INT				24
#define ATTR_BASE_STRUCT_STRING_INT				25
#define ATTR_BASE_STRUCT					26
#define ATTR_BASE_INT_INT_INT_INT				27
#define ATTR_BASE_INT_OPAQUE					28
#define ATTR_BASE_INT_STRING_INT_INT				29
#define ATTR_BASE_INT_OPAQUE_INT_INT				30
#define ATTR_BASE_OPAQUE_INT_INT    				31
#define ATTR_BASE_INT_FUNCPTR         				32
#define ATTR_BASE_FUNCPTR_OPAQUE    				33
#define ATTR_BASE_STRING_STRING    				34
#define ATTR_BASE_STRING_OPAQUE    				35
#define ATTR_BASE_STRING_FUNCPTR_OPAQUE     			36
#define ATTR_BASE_STRING_OPAQUE_STRING     			37
#define ATTR_BASE_STRING_STRING_STRING     			38
#define ATTR_BASE_STRING_OPAQUE_FUNCPTR_OPAQUE			39
#define ATTR_BASE_STRING_STRING_FUNCPTR_OPAQUE			40
#define ATTR_BASE_STRING_OPAQUE_STRING_FUNCPTR_OPAQUE		41
#define ATTR_BASE_STRING_STRING_STRING_FUNCPTR_OPAQUE		42
#define ATTR_BASE_INT_STRING_OPAQUE_STRING_FUNCPTR_OPAQUE	43
#define ATTR_BASE_INT_STRING_STRING_STRING_FUNCPTR_OPAQUE	44
#define ATTR_BASE_STRUCTARRAY_INT				45
#define ATTR_BASE_STRUCTARRAY_INT_INT_STRING			46

#define ATTR_TYPE(base_type, cardinality) \
    (((((unsigned)(base_type)) & 0x07f) << 5) | \
     (((unsigned)(cardinality)) & 0x0f))
 
#define ATTR_CHAR		ATTR_TYPE(ATTR_BASE_CHAR, 1)
#define ATTR_SHORT		ATTR_TYPE(ATTR_BASE_SHORT, 1)
#define ATTR_INT		ATTR_TYPE(ATTR_BASE_INT, 1)
#define ATTR_UNSIGNED_CHAR	ATTR_TYPE(ATTR_BASE_UNSIGNED_CHAR, 1)
#define ATTR_UNSIGNED_SHORT	ATTR_TYPE(ATTR_BASE_UNSIGNED_SHORT, 1)
#define ATTR_UNSIGNED_INT	ATTR_TYPE(ATTR_BASE_UNSIGNED_INT, 1)
#define ATTR_INT_INT		ATTR_TYPE(ATTR_BASE_INT_INT, 2)
#define ATTR_INT_INT_INT	ATTR_TYPE(ATTR_BASE_INT_INT_INT, 3)
#define ATTR_INT_INT_INT_INT	ATTR_TYPE(ATTR_BASE_INT_INT_INT_INT, 4)
#define ATTR_LONG		ATTR_TYPE(ATTR_BASE_LONG, 1)
#define ATTR_FLOAT		ATTR_TYPE(ATTR_BASE_FLOAT, 1)
#define ATTR_DOUBLE		ATTR_TYPE(ATTR_BASE_DOUBLE, 1)
#define ATTR_STRING		ATTR_TYPE(ATTR_BASE_STRING, 1)
#define ATTR_FUNCPTR		ATTR_TYPE(ATTR_BASE_FUNCPTR, 1)
#define ATTR_OPAQUE		ATTR_TYPE(ATTR_BASE_OPAQUE, 1)
#define ATTR_NO_VALUE		ATTR_TYPE(ATTR_BASE_NO_VALUE, 0)
#define ATTR_ENUM		ATTR_TYPE(ATTR_BASE_ENUM, 1)
#define ATTR_BOOLEAN		ATTR_TYPE(ATTR_BASE_BOOLEAN, 1)
#define ATTR_INT_STRING		ATTR_TYPE(ATTR_BASE_INT_STRING, 2)
#define ATTR_INT_INT_STRING	ATTR_TYPE(ATTR_BASE_INT_INT_STRING, 3)
#define ATTR_INT_OPAQUE		ATTR_TYPE(ATTR_BASE_INT_OPAQUE, 2)
#define ATTR_OPAQUE_STRING_INT	ATTR_TYPE(ATTR_BASE_OPAQUE_STRING_INT, 3)
#define ATTR_OPAQUE_OPAQUE_STRING	ATTR_TYPE(ATTR_BASE_OPAQUE_OPAQUE_STRING, 3)
#define ATTR_FUNCPTR_INT	ATTR_TYPE(ATTR_BASE_FUNCPTR_INT, 2)
#define ATTR_INT_OPAQUE_STRING	ATTR_TYPE(ATTR_BASE_INT_OPAQUE_STRING, 3)
#define ATTR_STRING_INT_INT	ATTR_TYPE(ATTR_BASE_STRING_INT_INT, 3)
#define ATTR_OPAQUE_INT_INT	ATTR_TYPE(ATTR_BASE_OPAQUE_INT_INT, 3)
#define ATTR_INT_STRUCT_STRING_INT	ATTR_TYPE(ATTR_BASE_INT_STRUCT_STRING_INT, 4)
#define ATTR_STRUCT_STRING_INT	ATTR_TYPE(ATTR_BASE_STRUCT_STRING_INT, 3)
#define ATTR_STRUCT		ATTR_TYPE(ATTR_BASE_STRUCT, 1)
#define ATTR_INT_STRING_INT_INT ATTR_TYPE(ATTR_BASE_INT_STRING_INT_INT, 4)
#define ATTR_INT_OPAQUE_INT_INT ATTR_TYPE(ATTR_BASE_INT_OPAQUE_INT_INT, 4)
#define ATTR_STRING_STRING 	ATTR_TYPE(ATTR_BASE_STRING_STRING, 2)
#define ATTR_STRING_OPAQUE 	ATTR_TYPE(ATTR_BASE_STRING_OPAQUE, 2)
#define ATTR_STRING_FUNCPTR_INT	ATTR_TYPE(ATTR_BASE_STRING_FUNCPTR_INT, 3)
#define ATTR_STRING_OPAQUE_FUNCPTR_INT	ATTR_TYPE(ATTR_BASE_STRING_OPAQUE_FUNCPTR_INT, 4)
#define ATTR_INT_FUNCPTR	ATTR_TYPE(ATTR_BASE_INT_FUNCPTR, 2) 
#define ATTR_FUNCPTR_OPAQUE	ATTR_TYPE(ATTR_BASE_FUNCPTR_OPAQUE, 2)
#define ATTR_STRING_FUNCPTR_OPAQUE	ATTR_TYPE(ATTR_BASE_STRING_FUNCPTR_OPAQUE, 3)
#define ATTR_STRING_OPAQUE_STRING	ATTR_TYPE(ATTR_BASE_STRING_OPAQUE_STRING, 3)
#define ATTR_STRING_STRING_STRING	ATTR_TYPE(ATTR_BASE_STRING_STRING_STRING, 3)
#define ATTR_STRING_OPAQUE_FUNCPTR_OPAQUE	ATTR_TYPE(ATTR_BASE_STRING_OPAQUE_FUNCPTR_OPAQUE, 4)
#define ATTR_STRING_STRING_FUNCPTR_OPAQUE	ATTR_TYPE(ATTR_BASE_STRING_STRING_FUNCPTR_OPAQUE, 4)
#define ATTR_STRING_OPAQUE_STRING_FUNCPTR_OPAQUE	ATTR_TYPE(ATTR_BASE_STRING_OPAQUE_STRING_FUNCPTR_OPAQUE, 5)
#define ATTR_STRING_STRING_STRING_FUNCPTR_OPAQUE	ATTR_TYPE(ATTR_BASE_STRING_STRING_STRING_FUNCPTR_OPAQUE, 5)
#define ATTR_INT_STRING_OPAQUE_STRING_FUNCPTR_OPAQUE	ATTR_TYPE(ATTR_BASE_INT_STRING_OPAQUE_STRING_FUNCPTR_OPAQUE, 6)
#define ATTR_INT_STRING_STRING_STRING_FUNCPTR_OPAQUE	ATTR_TYPE(ATTR_BASE_INT_STRING_STRING_STRING_FUNCPTR_OPAQUE, 6)
#define ATTR_STRUCTARRAY_INT				ATTR_TYPE(ATTR_BASE_STRUCTARRAY_INT, 2)
#define ATTR_STRUCTARRAY_INT_INT_STRING			ATTR_TYPE(ATTR_BASE_STRUCTARRAY_INT_INT_STRING, 4)

/* END OF ATTRIBUTE DEFINITIONS */

/* BEGINNING OF PANEL HEADER INFORMATION */

/***********************************************************************/
/* panel execute return values or user arg from control validate call  */
/***********************************************************************/
#define PANEL_OK	1
#define PANEL_CANCEL	0
#define PANEL_ERROR	-1
#define PANEL_PINNED	-2

/***********************************************************************/
/* panel validate source values					       */
/***********************************************************************/
/*	PANEL_OK	1	already defined			       */
#define PANEL_CONTROL	1
#define PANEL_FIELD	2
/*#define PANEL_SET	3	Will be used for panel item dynamic set*/

/***********************************************************************/
/* panel item status values                                            */
/***********************************************************************/
#define PANEL_ITEM_CHANGED	1
#define PANEL_ITEM_DISABLED	-1
#define PANEL_ITEM_NO_CHANGE	0

/***********************************************************************/
/* panel item types                                                    */
/***********************************************************************/
/*
 * This must be the same as MAX_SFE_ITEMS on the safe side, suncui.h
 */
#define MAX_PANEL_ITEMS	500

/***********************************************************************/
/* KEYBOARD_ITEM defines                                               */
/***********************************************************************/
#define ALPHA	1
#define INT	2
#define FLOAT	3
#define MINT	4
#define MFLOAT	5

#define	CUI_HORIZONTAL			0x00
#define CUI_VERTICAL			0x01
#define HORIZONTAL			CUI_HORIZONTAL
#define VERTICAL			CUI_VERTICAL

/***********************************************************************/
/* panel boolean op codes                                              */
/***********************************************************************/
#define AND	2
#define OR	1
#define END	0

/***********************************************************************/
/*	        	Attributes 				       */
/***********************************************************************/

#define	PANEL_ATTR(type, ordinal) \
	ATTR(ATTR_PKG_PANEL, type, ordinal)

/*
 * The attribute type used in the PANEL_ATTR macro,
 * which is shown in the comment, * has the following syntax:
 *	ATTR_INT_STRING_DOUBLE  
 * where the _ character is the delimeter between expected arguments. 
 * The example attribute above has an int, char *, and double as its arguments.
 */
typedef enum {
    /* panel attributes */	
    PANEL_NAME			= PANEL_ATTR(ATTR_STRING, 21),
    PANEL_ID			= PANEL_ATTR(ATTR_INT, 22),
    PANEL_SIZE			= PANEL_ATTR(ATTR_INT_INT, 23),
    PANEL_WIDTH			= PANEL_ATTR(ATTR_INT, 24),
    PANEL_HEIGHT		= PANEL_ATTR(ATTR_INT, 25),
    PANEL_POSITION		= PANEL_ATTR(ATTR_INT_INT, 26),
    PANEL_POSX			= PANEL_ATTR(ATTR_INT, 27),
    PANEL_POSY			= PANEL_ATTR(ATTR_INT, 28),
    PANEL_TITLE			= PANEL_ATTR(ATTR_STRING, 29),
    PANEL_MESSAGE		= PANEL_ATTR(ATTR_STRING, 30),
    PANEL_VALIDATE		= PANEL_ATTR(ATTR_FUNCPTR, 31),
    PANEL_FIRST_ITEM		= PANEL_ATTR(ATTR_NO_VALUE, 32),
    PANEL_NEXT_ITEM		= PANEL_ATTR(ATTR_NO_VALUE, 33),
    PANEL_CARET_ITEM		= PANEL_ATTR(ATTR_OPAQUE, 34),
    PANEL_DESTROY_ITEM		= PANEL_ATTR(ATTR_OPAQUE, 35),
    PANEL_DISABLE_ITEM		= PANEL_ATTR(ATTR_OPAQUE_OPAQUE_STRING, 36),
    PANEL_RETURN		= PANEL_ATTR(ATTR_OPAQUE_OPAQUE_STRING, 37),
    PANEL_ACTIVE		= PANEL_ATTR(ATTR_NO_VALUE, 38),
    PANEL_VALIDATE_CAUSE	= PANEL_ATTR(ATTR_INT, 39),
    PANEL_VALIDATE_CONTROL	= PANEL_ATTR(ATTR_OPAQUE, 40),
    PANEL_DEFAULT_CONTROL	= PANEL_ATTR(ATTR_OPAQUE, 41),
    PANEL_PUSHPIN		= PANEL_ATTR(ATTR_INT, 42),
    PANEL_PUSHPIN_IN		= PANEL_ATTR(ATTR_INT, 43),
    PANEL_PIN_CALLBACK		= PANEL_ATTR(ATTR_FUNCPTR_OPAQUE, 44),
    PANEL_UNPIN_CALLBACK	= PANEL_ATTR(ATTR_FUNCPTR_OPAQUE, 45),
    PANEL_ORIGIN_TOP_LEFT	= PANEL_ATTR(ATTR_NO_VALUE, 46),

    	
    /* panel item attributes */	
    PANEL_ITEM_NAME		= PANEL_ATTR(ATTR_STRING, 50),
    PANEL_ITEM_TYPE		= PANEL_ATTR(ATTR_ENUM, 51),
    PANEL_ITEM_ID		= PANEL_ATTR(ATTR_INT, 52),
    PANEL_ITEM_POSITION		= PANEL_ATTR(ATTR_INT_INT, 53),
    PANEL_ITEM_ROW		= PANEL_ATTR(ATTR_INT, 54),
    PANEL_ITEM_COL		= PANEL_ATTR(ATTR_INT, 55),
    PANEL_ITEM_LABEL		= PANEL_ATTR(ATTR_STRING, 56),
    PANEL_ITEM_PARENT		= PANEL_ATTR(ATTR_OPAQUE, 57),
    PANEL_ITEM_STATUS		= PANEL_ATTR(ATTR_ENUM, 58),
    PANEL_ITEM_LABEL_OBJECT	= PANEL_ATTR(ATTR_OPAQUE, 59),
    PANEL_ITEM_LOC_X		= PANEL_ATTR(ATTR_INT, 60),
    PANEL_ITEM_LOC_Y		= PANEL_ATTR(ATTR_INT, 61),
    PANEL_ITEM_SIZE_X		= PANEL_ATTR(ATTR_INT, 62),
    PANEL_ITEM_SIZE_Y		= PANEL_ATTR(ATTR_INT, 63),
	
    /* all panel items but text */	
    PANEL_ITEM_VALIDATE		= PANEL_ATTR(ATTR_FUNCPTR_INT, 70),
    PANEL_ITEM_VALIDATE_USER_ARG= PANEL_ATTR(ATTR_OPAQUE, 71),
    PANEL_ITEM_VALUE		= PANEL_ATTR(ATTR_STRING, 72),
    PANEL_ITEM_HELP		= PANEL_ATTR(ATTR_STRING, 73),
	
    /* Roys multi-choice menus */
    PANEL_ITEM_POLY_VALUE	= PANEL_ATTR(ATTR_STRING_STRING, 75),
    PANEL_ITEM_FIRST_VALUE	= PANEL_ATTR(ATTR_NO_VALUE, 76),
    PANEL_ITEM_NEXT_VALUE	= PANEL_ATTR(ATTR_NO_VALUE, 77),
    PANEL_ITEM_CALLBACK		= PANEL_ATTR(ATTR_FUNCPTR_INT, 78),
    PANEL_ITEM_CALLBACK_USER_ARG= PANEL_ATTR(ATTR_OPAQUE, 79),
	
    /* all panel items but text and positional choice */	
    PANEL_ITEM_ORIENT		= PANEL_ATTR(ATTR_ENUM, 80),
	
    /* keyboard panel items only */	
    PANEL_ITEM_FIELD_SIZE	= PANEL_ATTR(ATTR_INT_INT, 90),
    PANEL_ITEM_DISPLAY_LENGTH	= PANEL_ATTR(ATTR_INT, 91),
    PANEL_ITEM_STORED_LENGTH	= PANEL_ATTR(ATTR_INT, 92),
    PANEL_ITEM_FIELD_TYPE	= PANEL_ATTR(ATTR_INT, 93),
	
    /* scroll and choice panel items only */	
    PANEL_ITEM_OPTION		= PANEL_ATTR(ATTR_INT_STRING, 100),
    PANEL_ITEM_OPTIONS		= PANEL_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING), 101),
    /* scroll and choice panel item object attributes */
    PANEL_ITEM_OPTION_OBJECT	= PANEL_ATTR(ATTR_INT_OPAQUE, 110),
    PANEL_ITEM_OPTION_OBJECTS	= PANEL_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_OPAQUE), 111),

    /* postional choice panel items only */	
    PANEL_ITEM_POS_CHOICE_ELEMENT	= PANEL_ATTR(ATTR_STRING_INT_INT, 102),
    PANEL_ITEM_POS_CHOICE_ELEMENTS	= PANEL_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_INT_INT), 103),
    /* positional choice panel item object attributes */
    PANEL_ITEM_POS_OPTION	= PANEL_ATTR(ATTR_INT_STRING_INT_INT, 112),
    PANEL_ITEM_POS_OPTION_OBJECT= PANEL_ATTR(ATTR_INT_OPAQUE_INT_INT, 113),
    PANEL_ITEM_POS_OPTIONS	 = PANEL_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_INT_INT), 114),
    PANEL_ITEM_POS_OPTION_OBJECTS= PANEL_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_OPAQUE_INT_INT), 115),
    	
    /* boolean panel item only */	
    PANEL_BOOLEAN_EXPR		= PANEL_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_OPAQUE_STRING_INT), 104)
} Panel_attribute;


/***********************************************************************/
/* opaque types for panel and panel item                               */
/***********************************************************************/

typedef int Cui_handle;			/* Handle returned by cui_..._create */
typedef Cui_handle Cui_window;
typedef Cui_handle Cui_window_item;
typedef caddr_t 	Cui_object;	/* Handle for graphic objects */
typedef	caddr_t 	Panel;
typedef	caddr_t 	Panel_item;
typedef	caddr_t 	Panel_attribute_value;

/***********************************************************************/
/* macros                                                              */
/***********************************************************************/
#define CUI_PANEL_GET_VALUE(item)	((char *)cui_panel_get \
					(item,PANEL_ITEM_VALUE))

#define CUI_PANEL_SET_VALUE(item,value)	((char *)cui_panel_set \
					(item,PANEL_ITEM_VALUE,value,0))

#define FIRST_ITEM(p)		(cui_panel_item_find( p, PANEL_FIRST_ITEM, 0 ))
#define NEXT_ITEM(p)		(cui_panel_item_find( p, PANEL_NEXT_ITEM,  0 ))

#define FIRST_NON_TEXT_ITEM(p)	(cui_panel_item_find( p, PANEL_FIRST_ITEM, \
				    PANEL_ITEM_TYPE, NOT_TEXT_ITEM, 0 ))
#define NEXT_NON_TEXT_ITEM(p)	(cui_panel_item_find( p, PANEL_NEXT_ITEM, \
				    PANEL_ITEM_TYPE, NOT_TEXT_ITEM, 0 ))

#define FIRST_KEYBOARD_ITEM(p)	(cui_panel_item_find( p, PANEL_FIRST_ITEM, \
				    PANEL_ITEM_TYPE, KEYBOARD_ITEM, 0 ))
#define NEXT_KEYBOARD_ITEM(p)	(cui_panel_item_find( p, PANEL_NEXT_ITEM, \
				    PANEL_ITEM_TYPE, KEYBOARD_ITEM, 0 ))

#define TOGGLE_IS_ON(i)		(strcmp( (char *)cui_panel_get( i, \
					PANEL_ITEM_VALUE ), "on" ) == 0 )
#define TOGGLE_IS_OFF(i)	(strcmp( (char *)cui_panel_get( i, \
					PANEL_ITEM_VALUE ), "off" ) == 0 )

/* END OF PANEL HEADER INFORMATION */


/************************************************************************
 * Declarations for multiple window support.				*
 ***********************************************************************/

typedef struct point {
	int x, y;
} Cui_point;

typedef struct rect {
	Cui_point orig, corn;
} Cui_rect;

typedef struct colors {
	int red, green, blue;
} COLOR;

typedef struct xy_points {
	int x, y;
} POINT;

/***********************************************************************/
/* Data structure for aggregate option specification for window items  */
/***********************************************************************/
 
typedef struct opt_spec {
	char 		*name;
	char 		*label;
	Cui_object 	object;
	char 		*command;
#ifdef __cplusplus
	char 		*(*callback)( Cui_handle, caddr_t );
#else
	char 		*(*callback)();
#endif
	caddr_t 	callback_arg;
	int		selected;
	int		state;		/* default of 0 mapped to CUI_ACTIVE */
	char		*help;
} Cui_option_spec;

#define CUI_COUNT(m)	(sizeof(m)/sizeof(m[0]))

/* some hard coded defaults */
#define	BASE_FRAME_HANDLE	1
#define	TEXTSW_HANDLE		101
#define	HELP_FRAME_HANDLE	599

#define CUI_WIN_CLIPBOARD	999

/* this group of (3) names must match the ones in safe`window.h` */
typedef enum {
	/* 
	 * old window and window item types 
	 */
	CUI_BUTTON_ITEM =		0x16,
	CUI_PULLDOWN_ITEM =		0x17,
	CUI_OBJECT_ITEM =		0x18,
	CUI_WIN_FRAME,
	TOGGLE_ITEM,
	POS_CHOICE_ITEM,
	CONTROL_ITEM,
	/* 
	 * window item types 
	 */
	CUI_TEXT,
	CUI_BUTTON,
	CUI_PULLDOWN,
	CUI_SLIDER,
	CUI_KEYBOARD,
	CUI_ABBR_MENU,
	CUI_POPUP_MENU,
	CUI_POLY_RECT_CHOICE,
	CUI_EXCL_RECT_CHOICE,
	CUI_POLY_CHECK_CHOICE,
	CUI_EXCL_CHECK_CHOICE,
	CUI_POLY_RECT_SCROLL,
	CUI_EXCL_RECT_SCROLL,
	CUI_POLY_CHECK_SCROLL,
	CUI_EXCL_CHECK_SCROLL,
	/* 
	 * window types 
	 */
	CUI_FRAME,
	CUI_SUB_FRAME,
	CUI_CANVAS,
	CUI_COMMAND_IO,
	CUI_CONTROL,
	CUI_SCROLL_CONTROL,
	CUI_TEXTIO,
	CUI_SCROLL_TEXTIO,
	CUI_SELECT_DIALOG_BOX,
	CUI_FILE_DIALOG_BOX,
	CUI_MESSAGE,
	/* 
	 * misc types 
	 */
	CUI_BOOLEAN,
	NOT_TEXT_ITEM
} Cui_object_type;

/* old style names */
#define TEXT_ITEM	CUI_TEXT
#define KEYBOARD_ITEM	CUI_KEYBOARD
#define CHOICE_ITEM	CUI_EXCL_RECT_CHOICE
#define SCROLL_ITEM	CUI_ABBR_MENU
#define BOOLEAN_ITEM	CUI_BOOLEAN
#define CUI_WIN_CANVAS	CUI_CANVAS
#define CUI_WIN_CONTROL	CUI_SCROLL_CONTROL
#define	CUI_WIN_MESSAGE	CUI_MESSAGE
#define	CUI_WIN_TEXTSW	CUI_COMMAND_IO

typedef enum {
	CUI_ACTIVE			= 0x2,
	CUI_INACTIVE			= 0x4,
	CUI_NOT_DISPLAYED		= 0x8,
	CUI_ACTIVE_NOT_DISPLAYED	= 0x10
} Cui_state;

#define       CUI_WINDOW_ITEM_ACTIVE          CUI_ACTIVE
#define       CUI_WINDOW_ITEM_INACTIVE        CUI_INACTIVE

typedef enum {		/* gravity for objects */
	CUI_GRAV_UNKNOWN		= 0,
	CUI_GRAV_NORTH		= 0x1,
	CUI_GRAV_SOUTH		= 0x2,
	CUI_GRAV_EAST		= 0x4,
	CUI_GRAV_WEST		= 0x8,
	CUI_GRAV_NORTHWEST	= (CUI_GRAV_NORTH | CUI_GRAV_WEST),
	CUI_GRAV_NORTHEAST	= (CUI_GRAV_NORTH | CUI_GRAV_EAST),
	CUI_GRAV_SOUTHWEST	= (CUI_GRAV_SOUTH | CUI_GRAV_WEST),
	CUI_GRAV_SOUTHEAST	= (CUI_GRAV_SOUTH | CUI_GRAV_EAST),
	CUI_GRAV_CENTER		= 0x10
} Cui_gravity;

typedef enum {		/* k-shell editing options	*/
	CUI_DEFAULT	= -1,
	CUI_GMACS   	=  1,
	CUI_EMACS   	=  2,
	CUI_EDITVI  	=  4
} Cui_edit_style;

typedef enum {		/* layout for frames and containers */
	CUI_LAYOUT_COLUMN,
	CUI_LAYOUT_ROW,
	CUI_LAYOUT_ROW_ALIGNED,
	CUI_LAYOUT_NONE
} Cui_layout;

typedef enum {
	CUI_SIZE_TRACK_FULL,
	CUI_SIZE_TRACK_HORIZONTAL,
	CUI_SIZE_TRACK_VERTICAL,
	CUI_SIZE_TRACK_NONE
} Cui_size_track;

typedef enum {
	CUI_FIT_NONE		= 0x0,
	CUI_FIT_HORIZONTAL	= 0x1,
	CUI_FIT_VERTICAL	= 0x2,
	CUI_FIT_ALL		= CUI_FIT_HORIZONTAL | CUI_FIT_VERTICAL,
} Cui_fit;

/* Window MACROS */
#define CUI_WINDOW_ITEM_DESTROY(label)	\
	cui_window_item_destroy( \
		cui_window_item_find( CUI_WINDOW_ITEM_LABEL, (label), 0));

#define CUI_BUTTON_ITEM_DESTROY(label)	\
	cui_window_item_destroy( \
		cui_window_item_find(CUI_WINDOW_ITEM_TYPE, CUI_BUTTON_ITEM, \
			CUI_WINDOW_ITEM_LABEL, (label), 0));

#define CUI_PULLDOWN_ITEM_DESTROY(label)	\
	cui_window_item_destroy( \
		cui_window_item_find(CUI_WINDOW_ITEM_TYPE, CUI_PULLDOWN_ITEM, \
			CUI_WINDOW_ITEM_LABEL, (label), 0));

#define CUI_OBJECT_ITEM_DESTROY(label)	\
	cui_window_item_destroy( \
		cui_window_item_find(CUI_WINDOW_ITEM_TYPE, CUI_OBJECT_ITEM, \
			CUI_WINDOW_ITEM_LABEL, (label), 0));

#define WINDOW_ATTR(type, ordinal) \
	ATTR(ATTR_PKG_WINDOW, type, ordinal)

/*
 * The attribute type used in the WINDOW_ATTR macro has the following syntax:
 * 	ATTR_INT_STRING_DOUBLE  
 * where the '_' character is the delimeter between expected arguments. 
 * The example attribute above has an int, char *, and double as its arguments.
 */
/*
 * Define window attribute values for the "attribute code" part 
 * of the attribute-value pairs sent between CUI and SAFE.  Since 
 * these values are part of the inter-process communication protocol,
 * they must be printable characters - hence start with decimal 32.

	These attributes can be used in
		cui_window_create	( Window_type,   <attribute list> );
		cui_window_set		( Window_handle, <attribute list> );
		cui_window_get		( Window_handle, <attribute list> );
		cui_window_item_create	( Window_type,   <attribute list> );
		cui_window_item_set	( Window_handle, <attribute list> );
		cui_window_item_get	( Window_handle, <attribute list> );
 */

typedef enum {
    CUI_ABOVE			= WINDOW_ATTR(ATTR_INT, 8),
    CUI_ADD_TO_LAYOUT		= WINDOW_ATTR(ATTR_NO_VALUE, 9),
    CUI_ARG			= WINDOW_ATTR(ATTR_OPAQUE, 10),
    CUI_BACKGROUND_PIXEL	= WINDOW_ATTR(ATTR_INT, 11),
    CUI_BAR_SIZE		= WINDOW_ATTR(ATTR_INT, 12),
    CUI_BASE_DRAG_ENABLED	= WINDOW_ATTR(ATTR_INT, 14),
    CUI_BELOW			= WINDOW_ATTR(ATTR_INT, 15),
    CUI_CALLBACK		= WINDOW_ATTR(ATTR_FUNCPTR_OPAQUE, 16),
    CUI_CALLBACK_OPTIONS	= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_FUNCPTR_OPAQUE), 17),
    CUI_CENTER_CHILDREN		= WINDOW_ATTR(ATTR_NO_VALUE, 18),
    CUI_CHARACTER_ARG		= WINDOW_ATTR(ATTR_OPAQUE, 19),
    CUI_CHARACTER_CALLBACK 	= WINDOW_ATTR(ATTR_FUNCPTR_OPAQUE, 20),
    CUI_CLOSED			= WINDOW_ATTR(ATTR_NO_VALUE, 21),
    CUI_COL			= WINDOW_ATTR(ATTR_INT, 22),
    CUI_COLORMAP		= WINDOW_ATTR(ATTR_INT_STRING, 23),
    CUI_COMMAND			= WINDOW_ATTR(ATTR_STRING, 24),
    CUI_DATA			= WINDOW_ATTR(ATTR_OPAQUE, 25),
    CUI_DIRECTORY		= WINDOW_ATTR(ATTR_STRING, 26),
    CUI_DISPLAY			= WINDOW_ATTR(ATTR_OPAQUE, 27),
    CUI_DISPLAY_LENGTH		= WINDOW_ATTR(ATTR_INT, 28),
    CUI_DOUBLE_CLICK_FUZZ	= WINDOW_ATTR(ATTR_INT, 29),
    CUI_DOUBLE_CLICK_TIMEOUT	= WINDOW_ATTR(ATTR_INT, 30),
    CUI_DRAWABLE		= WINDOW_ATTR(ATTR_OPAQUE, 31),
    CUI_EDITABLE		= WINDOW_ATTR(ATTR_INT, 32),
    CUI_FIELD_SIZE		= WINDOW_ATTR(ATTR_INT_INT, 33),
    CUI_FILES			= WINDOW_ATTR(ATTR_STRING_STRING, 34),
    CUI_FILE_SELECT		= WINDOW_ATTR(ATTR_STRING, 35),
    CUI_FILTER			= WINDOW_ATTR(ATTR_STRING, 36),
    CUI_FILTER_LABEL		= WINDOW_ATTR(ATTR_STRING, 37),
    CUI_FIRST_SELECTION_COMMAND	= WINDOW_ATTR(ATTR_NO_VALUE, 38),
    CUI_FIRST_SELECTION_INDEX	= WINDOW_ATTR(ATTR_NO_VALUE, 39),
    CUI_FOREGROUND_PIXEL	= WINDOW_ATTR(ATTR_INT, 40),
    CUI_FRAME_THICKNESS		= WINDOW_ATTR(ATTR_INT, 41),
    CUI_GRAVITY			= WINDOW_ATTR(ATTR_ENUM, 42),
    CUI_HANDLE			= WINDOW_ATTR(ATTR_INT, 43),
    CUI_HANDLE_LOC		= WINDOW_ATTR(ATTR_INT, 44),
    CUI_HANDLE_RANGE		= WINDOW_ATTR(ATTR_INT, 45),
    CUI_HELP			= WINDOW_ATTR(ATTR_STRING, 46),
    CUI_ICON_FILE		= WINDOW_ATTR(ATTR_STRING, 47),
    CUI_ICON_FILES		= WINDOW_ATTR(ATTR_STRING_STRING, 48),
    CUI_LABEL			= WINDOW_ATTR(ATTR_STRING, 49),
    CUI_LAYOUT			= WINDOW_ATTR(ATTR_ENUM, 50),
    CUI_LAYOUT_RESUME		= WINDOW_ATTR(ATTR_NO_VALUE, 51),
    CUI_LAYOUT_SUSPEND		= WINDOW_ATTR(ATTR_NO_VALUE, 52),
    CUI_LEFT_JUSTIFY_CHILDREN	= WINDOW_ATTR(ATTR_NO_VALUE, 53),
    CUI_LEFT_OF			= WINDOW_ATTR(ATTR_INT, 54),
    CUI_LOAD_COLORMAP		= WINDOW_ATTR(ATTR_STRUCTARRAY_INT_INT_STRING, 55),
    CUI_LOAD_FILE		= WINDOW_ATTR(ATTR_STRING, 56),
    CUI_LOCATION		= WINDOW_ATTR(ATTR_INT_INT, 57),
    CUI_LOCX			= WINDOW_ATTR(ATTR_INT, 58),
    CUI_LOCY			= WINDOW_ATTR(ATTR_INT, 59),
    CUI_LOC_STILL_TIMEOUT	= WINDOW_ATTR(ATTR_INT, 60),
    CUI_MAXIMUM_LABEL		= WINDOW_ATTR(ATTR_STRING, 61),
    CUI_MAXIMUM_VALUE		= WINDOW_ATTR(ATTR_INT, 62),
    CUI_MENU			= WINDOW_ATTR(ATTR_STRING, 63),
    CUI_MINIMUM_LABEL		= WINDOW_ATTR(ATTR_STRING, 64),
    CUI_MINIMUM_SIZE		= WINDOW_ATTR(ATTR_INT_INT, 65),
    CUI_MINIMUM_SIZE_X		= WINDOW_ATTR(ATTR_INT, 66),
    CUI_MINIMUM_SIZE_Y		= WINDOW_ATTR(ATTR_INT, 67),
    CUI_MINIMUM_VALUE		= WINDOW_ATTR(ATTR_INT, 68),
    CUI_NAME			= WINDOW_ATTR(ATTR_STRING, 69),
    CUI_NEXT_SELECTION_COMMAND	= WINDOW_ATTR(ATTR_NO_VALUE, 70),
    CUI_NEXT_SELECTION_INDEX	= WINDOW_ATTR(ATTR_NO_VALUE, 71),
    CUI_NUMBER_OF_TICKS		= WINDOW_ATTR(ATTR_INT, 72),
    CUI_OBJECT			= WINDOW_ATTR(ATTR_OPAQUE, 73),
    CUI_OBJECT_ADD_OPTION	= WINDOW_ATTR(ATTR_INT_STRING_OPAQUE_STRING_FUNCPTR_OPAQUE, 74),
    CUI_OBJECT_CALLBACK_OPTIONS	= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_OPAQUE_FUNCPTR_OPAQUE), 75),
    CUI_OBJECT_COMMAND_OPTIONS	= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_OPAQUE_STRING), 76),
    CUI_OBJECT_LABEL_OPTIONS	= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_OPAQUE), 77),
    CUI_OBJECT_OPTIONS		= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_OPAQUE_STRING_FUNCPTR_OPAQUE), 78),
    CUI_OPTIONS_TITLE		= WINDOW_ATTR(ATTR_STRING, 79),
    CUI_OPTION_ARG 		= WINDOW_ATTR(ATTR_INT_OPAQUE, 80),
    CUI_OPTION_CALLBACK		= WINDOW_ATTR(ATTR_INT_FUNCPTR, 81),
    CUI_OPTION_COMMAND		= WINDOW_ATTR(ATTR_INT_STRING, 82),
    CUI_OPTION_DEFAULT		= WINDOW_ATTR(ATTR_INT, 83),
    CUI_OPTION_DELETE		= WINDOW_ATTR(ATTR_INT, 84),
    CUI_OPTION_HELP		= WINDOW_ATTR(ATTR_INT_STRING, 85),
    CUI_OPTION_LABEL		= WINDOW_ATTR(ATTR_INT_STRING, 86),
    CUI_OPTION_NAME		= WINDOW_ATTR(ATTR_INT_STRING, 87),
    CUI_OPTION_OBJECT		= WINDOW_ATTR(ATTR_INT_OPAQUE, 88),
    CUI_OPTION_SELECTED		= WINDOW_ATTR(ATTR_INT_INT, 89),
    CUI_OPTION_SPEC		= WINDOW_ATTR(ATTR_STRUCTARRAY_INT, 90),
    CUI_OPTION_STATE		= WINDOW_ATTR(ATTR_INT_INT, 91),
    CUI_ORIENT			= WINDOW_ATTR(ATTR_INT, 92),
    CUI_OUTPUT_FOCUS		= WINDOW_ATTR(ATTR_NO_VALUE, 93),
    CUI_OVERVIEW_OF		= WINDOW_ATTR(ATTR_INT, 94),
    CUI_OVERVIEW_RECT		= WINDOW_ATTR(ATTR_STRUCT, 95),
    CUI_OVERVIEW_SCALE		= WINDOW_ATTR(ATTR_INT_INT_INT_INT, 96),
    CUI_OVERVIEW_TRANSLATE	= WINDOW_ATTR(ATTR_INT_INT, 97),
    CUI_OVERVIEW_UPDATE		= WINDOW_ATTR(ATTR_INT, 98),
    CUI_PARENT			= WINDOW_ATTR(ATTR_OPAQUE, 99),
    CUI_PATHNAME		= WINDOW_ATTR(ATTR_STRING, 100),
    CUI_PIN_CALLBACK		= WINDOW_ATTR(ATTR_FUNCPTR_OPAQUE, 101),
    CUI_POSITION		= WINDOW_ATTR(ATTR_INT_INT, 102),
    CUI_PROMPT			= WINDOW_ATTR(ATTR_STRING_STRING, 103),
    CUI_PROMPT_HEAD		= WINDOW_ATTR(ATTR_STRING, 104),
    CUI_PROMPT_TAIL		= WINDOW_ATTR(ATTR_STRING, 105),
    CUI_PUSHPIN			= WINDOW_ATTR(ATTR_INT, 106),
    CUI_PUSHPIN_IN		= WINDOW_ATTR(ATTR_INT, 107),
    CUI_REGION			= WINDOW_ATTR(ATTR_INT_STRUCT_STRING_INT, 108),
    CUI_REMOVE_FROM_LAYOUT	= WINDOW_ATTR(ATTR_NO_VALUE, 109),
    CUI_RESIZABLE		= WINDOW_ATTR(ATTR_INT, 110),
    CUI_RIGHT_JUSTIFY_CHILDREN	= WINDOW_ATTR(ATTR_NO_VALUE, 111),
    CUI_RIGHT_OF		= WINDOW_ATTR(ATTR_INT, 112),
    CUI_ROW			= WINDOW_ATTR(ATTR_INT, 113),
    CUI_SAVE_FILE		= WINDOW_ATTR(ATTR_STRING_OPAQUE, 114),
    CUI_SCROLL_TITLE		= WINDOW_ATTR(ATTR_STRING, 115),
    CUI_SELECTION_COMMAND	= WINDOW_ATTR(ATTR_NO_VALUE, 116),
    CUI_SELECTION_INDEX		= WINDOW_ATTR(ATTR_NO_VALUE, 117),
    CUI_SIZE			= WINDOW_ATTR(ATTR_INT_INT, 118),
    CUI_SIZE_TRACK		= WINDOW_ATTR(ATTR_ENUM, 119),
    CUI_SIZE_X			= WINDOW_ATTR(ATTR_INT, 120),
    CUI_SIZE_Y			= WINDOW_ATTR(ATTR_INT, 121),
    CUI_SPACE			= WINDOW_ATTR(ATTR_INT, 122),
    CUI_SPACE_BOTTOM		= WINDOW_ATTR(ATTR_INT, 123),
    CUI_SPACE_LEFT		= WINDOW_ATTR(ATTR_INT, 124),
    CUI_SPACE_RIGHT		= WINDOW_ATTR(ATTR_INT, 125),
    CUI_SPACE_TOP		= WINDOW_ATTR(ATTR_INT, 126),
    CUI_STATE			= WINDOW_ATTR(ATTR_ENUM, 127),
    CUI_STORED_LENGTH		= WINDOW_ATTR(ATTR_INT, 128),
    CUI_TEXT_ADD_OPTION		= WINDOW_ATTR(ATTR_INT_STRING_STRING_STRING_FUNCPTR_OPAQUE, 129),
    CUI_TEXT_CALLBACK_OPTIONS	= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_STRING_FUNCPTR_OPAQUE), 130),
    CUI_TEXT_COMMAND_OPTIONS	= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_STRING_STRING), 131),
    CUI_TEXT_LABEL_OPTIONS	= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_STRING), 132),
    CUI_TEXT_OPTIONS		= WINDOW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_STRING_STRING_STRING_FUNCPTR_OPAQUE), 133),
    CUI_TITLE			= WINDOW_ATTR(ATTR_STRING, 134),
    CUI_TYPE			= WINDOW_ATTR(ATTR_INT, 135),
    CUI_UNIT_SIZE		= WINDOW_ATTR(ATTR_INT_INT, 136),
    CUI_UNIT_SIZE_X		= WINDOW_ATTR(ATTR_INT, 137),
    CUI_UNIT_SIZE_Y		= WINDOW_ATTR(ATTR_INT, 138),
    CUI_UNPIN_CALLBACK		= WINDOW_ATTR(ATTR_FUNCPTR_OPAQUE, 139),
    CUI_VALUE			= WINDOW_ATTR(ATTR_STRING, 140),
    CUI_VIEW_SIZE		= WINDOW_ATTR(ATTR_INT_INT, 141),
    CUI_VIEW_SIZE_X		= WINDOW_ATTR(ATTR_INT, 142),
    CUI_VIEW_SIZE_Y		= WINDOW_ATTR(ATTR_INT, 143),
    CUI_VISIBLE			= WINDOW_ATTR(ATTR_INT, 144),
    CUI_FIELD_TYPE		= WINDOW_ATTR(ATTR_INT, 145),
    /*
     * Unsorted additions
     */
    CUI_OPTION_MENU             = WINDOW_ATTR(ATTR_INT_OPAQUE, 146),
    CUI_BASE_OBJECT		= WINDOW_ATTR(ATTR_OPAQUE, 147),
    CUI_NUM_OPTIONS		= WINDOW_ATTR(ATTR_INT, 148),
    CUI_CARET_ITEM		= WINDOW_ATTR(ATTR_OPAQUE, 149),
    CUI_COMMAND_LINE_EDITING	= WINDOW_ATTR(ATTR_INT, 150),
    CUI_FIT			= WINDOW_ATTR(ATTR_ENUM, 151)
} Cui_attribute;
typedef Cui_attribute Window_attribute;

/*
 * mapping of old attributes to new attributes for backward compatability
 */
typedef enum {
	CUI_NOOP			= WINDOW_ATTR(ATTR_NO_VALUE, 200),
	CUI_NOOP_NOOP			= WINDOW_ATTR(ATTR_NO_VALUE, 201),
	CUI_NOOP_INT			= WINDOW_ATTR(ATTR_INT, 202),
	CUI_WIN_CREATE			= WINDOW_ATTR(ATTR_INT, 203),
	CUI_WIN_DESTROY			= WINDOW_ATTR(ATTR_NO_VALUE, 204),
	CUI_WINDOW_HORIZONTAL_SCROLLBAR	= WINDOW_ATTR(ATTR_INT, 205),
	CUI_WINDOW_VERTICAL_SCROLLBAR	= WINDOW_ATTR(ATTR_INT, 206)
} Old_attribute;

#define CUI_HELP_DATA			CUI_HELP
#define CUI_MINIMUM_TEXT_LINES		CUI_VIEW_SIZE_Y
#define CUI_WINDOW_CENTER_ITEMS		CUI_CENTER_CHILDREN
#define CUI_WINDOW_ITEM_LABEL		CUI_LABEL
#define CUI_WINDOW_ITEM_TYPE		CUI_TYPE
#define CUI_WINDOW_LEFT_JUSTIFY_ITEMS	CUI_LEFT_JUSTIFY_CHILDREN
#define CUI_WINDOW_RIGHT_JUSTIFY_ITEMS	CUI_RIGHT_JUSTIFY_CHILDREN
#define CUI_WINDOW_TEXT_LINES		CUI_VIEW_SIZE_Y
#define CUI_WIN_ABOVE			CUI_ABOVE
#define CUI_WIN_BACKGROUND_COLOR	CUI_BACKGROUND_PIXEL
#define CUI_WIN_BELOW			CUI_BELOW
#define CUI_WIN_CLOSED			CUI_CLOSED
#define CUI_WIN_COLORMAP		CUI_COLORMAP
#define CUI_WIN_DISPLAY			CUI_DISPLAY
#define CUI_WIN_DRAWABLE		CUI_DRAWABLE
#define CUI_WIN_FIT			CUI_NOOP_NOOP
#define CUI_WIN_FOREGROUND_COLOR	CUI_FOREGROUND_PIXEL
#define CUI_WIN_FRONT			CUI_NOOP
#define CUI_WIN_HANDLE			CUI_PARENT
#define CUI_WIN_HEIGHT			CUI_SIZE_Y
#define CUI_WIN_LABEL			CUI_LABEL
#define CUI_WIN_LEFT_OF			CUI_LEFT_OF
#define CUI_WIN_LOC			CUI_LOCATION
#define CUI_WIN_OUTPUT_FOCUS		CUI_OUTPUT_FOCUS
#define CUI_WIN_OVERVIEW_OF		CUI_OVERVIEW_OF
#define CUI_WIN_OVERVIEW_RECT		CUI_OVERVIEW_RECT
#define CUI_WIN_OVERVIEW_SCALE		CUI_OVERVIEW_SCALE
#define CUI_WIN_OVERVIEW_TRANSLATE	CUI_OVERVIEW_TRANSLATE
#define CUI_WIN_OVERVIEW_UPDATE		CUI_OVERVIEW_UPDATE
#define CUI_WIN_REGION			CUI_REGION
#define CUI_WIN_RIGHT_OF		CUI_RIGHT_OF
#define CUI_WIN_SIZE			CUI_SIZE
#define CUI_WIN_VISIBLE			CUI_VISIBLE
#define CUI_WIN_WIDTH			CUI_SIZE_X
#define CUI_WIN_X			CUI_LOCX
#define CUI_WIN_Y			CUI_LOCY

/************************************************************************
 * Declarations input datum support					*
 ***********************************************************************/

#define	CUI_STRING	0
#define	CUI_CHAR	1
#define	CUI_LEFTBUTTON	2
#define	CUI_MIDDLEBUTTON 3
#define	CUI_SPECIAL	4

#define CUI_NONE	0x00
#define	CUI_SHIFT	0x01
#define	CUI_CTRL	0x02
#define	CUI_META	0x04
#define	CUI_DBL_CLICK	0x08

typedef caddr_t		Cui_input_datum;

/*
 *      Def'n of CROSSHAIR Attributes
 */
#define HAIRRESET	((int)( 0176 ))		      /* 0176 '~' 126 */

#define HAIRCOLOR       ((int)( 040 ))                /* 040 ' ' 32 */
#define XHAIRCOLOR      ((int)( HAIRCOLOR | 01 ))     /* 041 '!' 33 */
#define YHAIRCOLOR      ((int)( HAIRCOLOR | 02 ))     /* 042 '"' 34 */
#define XYHAIRCOLOR     ((int)( HAIRCOLOR | 03 ))     /* 043 '#' 35 */

#define HAIRTHICKNESS   ((int)( 044 ))                /* 044 '$' 36 */
#define XHAIRTHICKNESS  ((int)( HAIRTHICKNESS | 01 )) /* 045 '%' 37 */
#define YHAIRTHICKNESS  ((int)( HAIRTHICKNESS | 02 )) /* 046 '&' 38 */
#define XYHAIRTHICKNESS ((int)( HAIRTHICKNESS | 03 )) /* 047 ''' 39 */

#define HAIRGAP         ((int)( 050 ))                /* 050 '(' 40 */
#define XHAIRGAP        ((int)( HAIRGAP | 01 ))       /* 051 ')' 41 */
#define YHAIRGAP        ((int)( HAIRGAP | 02 ))       /* 052 '*' 42 */
#define XYHAIRGAP       ((int)( HAIRGAP | 03 ))       /* 053 '+' 43 */

#define HAIRLENGTH      ((int)( 054 ))                /* 054 ',' 44 */
#define XHAIRLENGTH     ((int)( HAIRLENGTH | 01 ))    /* 055 '-' 45 */
#define YHAIRLENGTH     ((int)( HAIRLENGTH | 02 ))    /* 056 '.' 46 */
#define XYHAIRLENGTH    ((int)( HAIRLENGTH | 03 ))    /* 057 '/' 47 */

#define HAIRGRAVITY     ((int)( 060 ))                /* 060 '0' 48 */
#define XHAIRGRAVITY    ((int)( HAIRGRAVITY | 01 ))   /* 061 '1' 49 */
#define YHAIRGRAVITY    ((int)( HAIRGRAVITY | 02 ))   /* 062 '2' 50 */
#define XYHAIRGRAVITY   ((int)( HAIRGRAVITY | 03 ))   /* 063 '3' 51 */

#define HAIRSTYLE       ((int)( 070 ))                /* 070 '0' 56 */
#define XHAIRSTYLE      ((int)( HAIRSTYLE | 01 ))     /* 071 '1' 57 */
#define YHAIRSTYLE      ((int)( HAIRSTYLE | 02 ))     /* 072 '2' 58 */
#define XYHAIRSTYLE     ((int)( HAIRSTYLE | 03 ))     /* 073 '3' 59 */
 
/*
 *      Def'n of RUBBERBAND Attributes
 */
#define RBRESET		((int) (HAIRRESET) )		/* 0176 '~' 126 */

#define RBCOLOR		((int)( HAIRCOLOR ))		/* 040 ' ' 32 */
#define RBTHICKNESS	((int)( HAIRTHICKNESS ))	/* 044 '$' 36 */
#define RBSTYLE		((int)( 064 ))			/* 064 '4' 52 */

/*
 *	Line/Rubberband style values
 */
#define CUI_SOLID		1
#define CUI_DOTTED		2
#define CUI_DASHDOTTED		3
#define CUI_DOTDASHED		3	/* User-friendly */
#define CUI_DASHED		4
#define CUI_LONGDASHED		5
#define CUI_DASHDOTDOTTED	6

#define CUI_DEFAULT	       -1

#define CUI_USER_INPUT		0x1
#define CUI_USER_OUTPUT		0x2
#define CUI_USER_EXCEPT		0x4

#endif /* setools_DEFINED */


/***********************************************************************/
/* external declarations for C and C++			 	       */
/***********************************************************************/

extern int CUI_backward_compatibility;	/* initialized to 0 */

#ifdef __cplusplus
#    include <osfcn.h>

extern "C"	{
   char		*cad_file(char *, char *, char **);
   int		cadfil_(char *, char *, char *, char *, long, long, long, long);
   Cui_input_datum	*cui_alloc_datum();
   int		cui_batch_mode(int mode);
   int		cui_begin_highlight(int id);
   void		cui_begin_interaction();
   Cui_object	cui_begin_object();
   int		cui_button(...);
   int		cui_clear_graphics();
   int		cui_clear_rect(int index, int minx, int miny, int maxx,
   			int maxy);
   int		cui_clear_text();
   Cui_object	cui_create_object_from_file(const char *filename);
   int		cui_create_icon(const char *filename, const char *command,
   			int newcol, int flag);
   int		cui_datum_ginmode(Cui_input_datum *datum, int);
   int		cui_datum_id(Cui_input_datum *datum);
   int		cui_datum_is_ctrl(Cui_input_datum *datum);
   int		cui_datum_is_double_click(Cui_input_datum *datum);
   int		cui_datum_is_meta(Cui_input_datum *datum);
   int		cui_datum_is_shift(Cui_input_datum *datum);
   int		cui_datum_numpoints(Cui_input_datum *datum);
   Cui_point	*cui_datum_point(Cui_input_datum *datum, int n);
   char		*cui_datum_string(Cui_input_datum *datum);
   Cui_handle	cui_datum_win_handle(Cui_input_datum *datum);
   int		cui_define_icon(char *command, int newcol, int flag);
   int		cui_define_special_chars(char *special_Cs);
   int		cui_destroy_highlight(int id);
   int		cui_end();
   int		cui_end_highlight();
   void		cui_end_interaction();
   Cui_object	cui_end_object();
   int		cui_exit(int i);
   int		cui_filled_polygon_plot(int color, int x[], int y[], int n);
   int		cui_filled_polygons_plot(int color, int x[], int y[],
							int n,int m[]);
   int		cui_font_text(int x, int y, const char *str);
   void		cui_free_datum(Cui_input_datum *datum);
   int		cui_getcommand(char *cmdstrt, int maxsiz, char *keystr);
   int		cui_gin(char *c, int *x, int *y);
   int		cui_graphics_input(Cui_input_datum *datum);
   int		cui_graphics_mode(int i);
   char		*cui_initialize( int, char ** );
   int		cui_init_clipboard(const char *name);
   int		cui_init_colormap(const char *name, int size);
   int		cui_input(Cui_input_datum *datum, char *preload);
   void		cui_io_handler_insert( int fd, int mask,
			int (*f)(int fd, void *arg), void *arg );
   void		cui_io_handler_remove( int fd, int mask );
   int		cui_journal(int argc, char **argv, const char *script_file,
   			const char * record_file, const char *old_invoke_line);
   int		cui_load_colormap(int start, int count, COLOR clr[]);
   char		*cui_load_driver(const char *terminal);
   int		cui_menu(...);
   int		cui_menu_add_item(const char *menuname, const char nvpair,
   			int pos, int sel);
   int		cui_menu_del_item(const char *menuname, const char *label);
   int		cui_message(...);
   int		cui_mpop(const char *menuname, int mode);
   int		cui_mpush(const char *menuname);
   int		cui_paction(const char *action, ...);
   Panel	cui_panel_create(...);
   int		cui_panel_exec(Panel panel);
   Panel	cui_panel_find(...);
   Panel_attribute_value cui_panel_get(Panel panel, Panel_attribute attr, ...);
   Panel_item	cui_panel_item_create(Panel parent, int type, ...);
   Panel_item	cui_panel_item_find(Panel parent, ...);
   int		cui_panel_set(Panel object, ...);
   int		cui_pbool(...);
   int		cui_pcadd(const char *word, int row, int col);
   int		cui_pcbegin(const char *label, int row, int col);
   int		cui_pcend();
   int		cui_pchoice(const char *label, int row, int col, int orient,
   			const char *options);
   int		cui_pclose();
   int		cui_pexec(const char *name, char *value[], int status[],
   			char *errmsg, int curfld);
   int		cui_pexists(const char *s);
   int		cui_pkeybrd(const char label, int row, int col, int orient,
   			int nchars, int maxbuf, const char *tcode);
   int		cui_point_plot(int x[], int y[], int n);
   int		cui_point_send(int x, int y);
   int		cui_points(int n, POINT *point);
   int		cui_popen(const char *name, int nrows, int ncols);
   int		cui_position(int x, int y);
   int		cui_printf(...);
   int		cui_prompt(...);
   int		cui_pscroll(const char *label, int row, int col, int orient,
   			const char *options);
   int		cui_ptext(const char *label, int row, int col);
   int		cui_ptitle(const char *txt);
   int		cui_ptoggle(const char *label, int row, int col);
   int		cui_pulldown(...);
   int		cui_query(const char *attr, char *val);
   int		cui_rectangle(int x0, int y0, int x1, int y1);
   int		cui_report_canvas_size(int *width, int *height);
   int		cui_sample_colormap(const char *name);
   int		cui_select(const char *ptr, int dflt, char *choices[], int size,
   			int *errcode);
   int		cui_set(...);
   int		cui_set_color(int color);
   int		cui_set_colormode(char cp);
   int		cui_set_crosshair(...);
   int		cui_set_fill_pattern(int pattern_number);
   int		cui_set_ginmenu(const char *name);
   int		cui_set_ginmode(...);
   int		cui_set_highlight(int id, int flag);
   int		cui_set_line_style(int style);
   int		cui_set_line_width(int width);
   Cui_object	cui_set_object_offset(Cui_object object_id, int x, int y);
   int		cui_set_rubberband(...);
   int		cui_set_xhair_color(int i);
   int		cui_show_clipboard(int c);
   int		cui_system(const char *buff);
   int		cui_titlebar(...);
   int		cui_use_object_as_fill_pattern(Cui_object);
   int		cui_vector_plot(int x[], int y[], int n);
   Cui_handle	cui_window_create(unsigned int, ...);
   int		cui_window_set(Cui_handle, ...);
   int		cui_window_destroy(Cui_handle);
   caddr_t	cui_window_get(Cui_handle, ...);
   Cui_handle	cui_window_item_create(Cui_handle, int, ...);
   int		cui_window_item_set(Cui_handle, ...);
   int		cui_window_item_destroy(Cui_handle);
   int		cui_window_item_find(...);
   caddr_t	cui_window_item_get(Cui_handle, ...);
   void		cui_wait_done( int * );
}
#else
   extern Panel			cui_panel_create();
   extern Panel_attribute_value	cui_panel_get();
   extern int	 		cui_panel_set();
   extern int	 		cui_panel_exec();
   extern Panel	 		cui_panel_find();
   
   extern Panel_item 		cui_panel_item_create();
   extern Panel_item 		cui_panel_item_find();

   extern void			cui_free_datum();
   extern Cui_input_datum	*cui_alloc_datum();
   extern char			*cui_datum_string();
   extern Cui_point		*cui_datum_point();
   extern Cui_handle		cui_datum_win_handle();

   extern Cui_handle		cui_window_create();
   extern caddr_t		cui_window_get();
   extern caddr_t		cui_window_item_get();
   extern char			*cui_load_driver();
   extern Cui_object		cui_create_object_from_file();
   extern int			cui_use_object_as_fill_pattern();
   extern Cui_object		cui_begin_object();
   extern Cui_object		cui_end_object();
   extern Cui_object		cui_set_object_offset();
   extern char			*cui_initialize();
   extern char			*cad_file();
   extern void			cui_wait_done();
   extern void			cui_begin_interaction();
   extern void			cui_end_interaction();
   extern void			cui_io_handler_insert();
   extern void			cui_io_handler_remove();

#endif

