
#ifndef _sh_config_
#define	_sh_config_	1
/*
 * This has been generated from install/config
 * The information is based on the compile time environment.
 * It may be necessary to change values of some parameters for cross
 *  development environments.
 */

#include	<sys/types.h>

#define _sys_acct_	1
#define _dirent_ 1
#define _sys_dirent_	1
#define _fcntl_ 1
#define _sys_fcntl_	1
#define _sys_file_	1
#define _sys_filio_	1
#define _sys_ioctl_	1
#define _locale_ 1
#define _sgtty_ 1
#define _sys_times_	1
#define _termio_ 1
#define _sys_termio_	1
#define _termios_ 1
#define _sys_termios_	1
#define _sys_wait_	1
#define _unistd_ 1
#define _sys_unistd_	1
#define _sys_utsname_	1
#define _vfork_ 1
#define _usr_ucb_	1
#define _bin_grep_	1
#define _usr_bin_lp_	1
#define _bin_newgrp_	1
#define _sys_resource_	1
#define _poll_	1
#define const /* empty */
#define VOID	void
#define _sys_timeb_	1
#define sigrelease(s)	sigsetmask(sigblock(0)&~(1<<((s)-1)))
#define sig_begin()	(sigblock(0),sigsetmask(0))
#define getpgid(a)	getpgrp(a)

#define sh_rand(x) ((x),(rand()>>3)&077777)
#define PIPE_ERR	29
#define SETJMP	_setjmp
#define LONGJMP	_longjmp
#define SOCKET	1
#define YELLOWP	1
#define MULTIGROUPS	0
#define SHELLMAGIC	1
#define PATH_MAX	1024
#define TIC_SEC	60
#define _FLOCK	1
#define NOBUF	1
#define SETREUID	1
#define LSTAT	1
#define SYSCALL	1
#include 	<sys/time.h>
#define included_sys_time_
#define ECHO_N	1
#define ECHO_RAW	1
#define JOBS	1
#define WINSIZE	1
#define FS_3D	1
#define _SELECT5_	1
#define ESH	1
#define JOBS	1
#define NEWTEST	1
#define OLDTEST	1
#define SUID_EXEC	1
#define TIMEOUT	300000
#define VSH	1
#endif
#define const /* empty */
