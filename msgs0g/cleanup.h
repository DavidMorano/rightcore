/* cleanup */


/* users who can cleanup (remove files from) the messages spool area */

static const char	*cleanup_users[] = {
	"root",
	"special",
	"daemon",
	"sys",
	"adm",
	"admin",
	"uucp",
	"nuucp",
	"listen",
	"news",
	"pcs",
	"dam",
	NULL
} ;

static const char	*cleanup_groups[] = {
	"pcs",
	"sys",
	"staff",
	"adm",
	NULL
} ;


