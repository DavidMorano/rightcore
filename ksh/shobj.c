


/* local variables */

static const char	*exts[] = {
	".so",
	".o",
	"",
	NULL
} ;

static const char	*dirs64[] = {
	"lib/dialers/sparcv9",
	"lib/dialers/sparc",
	"lib/dialers",
	NULL
} ;

static const char	*dirs32[] = {
	"lib/dialers/sparcv8",
	"lib/dialers/sparcv7",
	"lib/dialers/sparc",
	"lib/dialers",
	NULL
} ;

static const char	*subs[] = {
	"init",
	"check",
	"free",
	NULL
} ;

enum subs {
	sub_init,
	sub_check,
	sub_free,
} ;




