
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif

{
    int num;
    char *ptr;
    struct {
	short  ltype;
	char  *lval;
    } match;
    struct {
	char  *gstring;
	short itterate;
    } geom;
} YYSTYPE;
extern YYSTYPE yylval;
# define LB 257
# define RB 258
# define LP 259
# define RP 260
# define MENUS 261
# define MENU 262
# define BUTTON 263
# define DEFAULT_FUNCTION 264
# define PLUS 265
# define MINUS 266
# define ASTERISK 267
# define GREATER 268
# define ALL 269
# define OR 270
# define CURSORS 271
# define PIXMAPS 272
# define ICONS 273
# define COLOR 274
# define SAVECOLOR 275
# define MONOCHROME 276
# define FUNCTION 277
# define ICONMGR_SHOW 278
# define ICONMGR 279
# define WINDOW_FUNCTION 280
# define ZOOM 281
# define ICONMGRS 282
# define ICONMGR_GEOMETRY 283
# define ICONMGR_NOSHOW 284
# define MAKE_TITLE 285
# define ICONIFY_BY_UNMAPPING 286
# define DONT_ICONIFY_BY_UNMAPPING 287
# define STICKY 288
# define NO_TITLE 289
# define AUTO_RAISE 290
# define NO_HILITE 291
# define ICON_REGION 292
# define META 293
# define SHIFT 294
# define LOCK 295
# define CONTROL 296
# define WINDOW 297
# define TITLE 298
# define ICON 299
# define ROOT 300
# define FRAME 301
# define COLON 302
# define EQUALS 303
# define TILDE 304
# define SQUEEZE_TITLE 305
# define DONT_SQUEEZE_TITLE 306
# define SQUEEZE_ICON 307
# define DONT_SQUEEZE_ICON 308
# define START_ICONIFIED 309
# define NO_TITLE_HILITE 310
# define TITLE_HILITE 311
# define MOVE 312
# define RESIZE 313
# define WAIT_CURS 314
# define SELECT 315
# define KILL 316
# define LEFT_TITLEBUTTON 317
# define RIGHT_TITLEBUTTON 318
# define NUMBER 319
# define KEYWORD 320
# define MKEYWORD 321
# define NKEYWORD 322
# define CKEYWORD 323
# define CLKEYWORD 324
# define FKEYWORD 325
# define FSKEYWORD 326
# define SKEYWORD 327
# define DKEYWORD 328
# define JKEYWORD 329
# define PKEYWORD 330
# define WINDOW_RING 331
# define WARP_CUR 332
# define ERRORTOKEN 333
# define NO_STACKMODE 334
# define ICON_TITLE 335
# define NO_ICON_TITLE 336
# define STRING 337
# define REGEXP 338
