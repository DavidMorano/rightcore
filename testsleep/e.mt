execve("testsleep.x", 0xFFBEEC6C, 0xFFBEEC74)  argc = 1
open("/var/ld/ld.config", O_RDONLY)		Err#2 ENOENT
mmap(0x00000000, 8192, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANON, -1, 0) = 0xFF3A0000
open("/usr/add-on/local/lib/S5/librt.so.1", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/librt.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/dt/lib/librt.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/openwin/lib/librt.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/java/lib/librt.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/librt.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/ncmp/lib/librt.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/librt.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/S5/librt.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/librt.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/lib/librt.so.1", O_RDONLY)		= 3
fstat(3, 0xFFBEE33C)				= 0
mmap(0x00000000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF390000
mmap(0x00000000, 98304, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF370000
mmap(0xFF386000, 1784, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 24576) = 0xFF386000
munmap(0xFF376000, 65536)			= 0
memcntl(0xFF370000, 10032, MC_ADVISE, MADV_WILLNEED, 0, 0) = 0
close(3)					= 0
open("/usr/add-on/local/lib/S5/libpthread.so.1", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/libpthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/dt/lib/libpthread.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/openwin/lib/libpthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/java/lib/libpthread.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/libpthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/ncmp/lib/libpthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/libpthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/S5/libpthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/libpthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/lib/libpthread.so.1", O_RDONLY)	= 3
fstat(3, 0xFFBEE33C)				= 0
mmap(0xFF390000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 0) = 0xFF390000
mmap(0x00000000, 98304, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF350000
mmap(0xFF366000, 244, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 24576) = 0xFF366000
munmap(0xFF356000, 65536)			= 0
memcntl(0xFF350000, 15080, MC_ADVISE, MADV_WILLNEED, 0, 0) = 0
close(3)					= 0
open("/usr/add-on/local/lib/S5/libsocket.so.1", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/libsocket.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/dt/lib/libsocket.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/openwin/lib/libsocket.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/java/lib/libsocket.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/libsocket.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/ncmp/lib/libsocket.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/libsocket.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/S5/libsocket.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/libsocket.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/lib/libsocket.so.1", O_RDONLY)	= 3
fstat(3, 0xFFBEE33C)				= 0
mmap(0xFF390000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 0) = 0xFF390000
mmap(0x00000000, 114688, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF330000
mmap(0xFF34A000, 4349, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 40960) = 0xFF34A000
munmap(0xFF33A000, 65536)			= 0
memcntl(0xFF330000, 14440, MC_ADVISE, MADV_WILLNEED, 0, 0) = 0
close(3)					= 0
open("/usr/add-on/local/lib/S5/libnsl.so.1", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/libnsl.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/dt/lib/libnsl.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/openwin/lib/libnsl.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/java/lib/libnsl.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/libnsl.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/ncmp/lib/libnsl.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/libnsl.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/S5/libnsl.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/libnsl.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/lib/libnsl.so.1", O_RDONLY)		= 3
fstat(3, 0xFFBEE33C)				= 0
mmap(0xFF390000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 0) = 0xFF390000
mmap(0x00000000, 696320, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF280000
mmap(0xFF31A000, 32508, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 565248) = 0xFF31A000
mmap(0xFF322000, 30672, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_ANON, -1, 0) = 0xFF322000
munmap(0xFF30A000, 65536)			= 0
memcntl(0xFF280000, 81620, MC_ADVISE, MADV_WILLNEED, 0, 0) = 0
close(3)					= 0
open("/usr/add-on/local/lib/S5/libc.so.1", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/libc.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/dt/lib/libc.so.1", O_RDONLY)		Err#2 ENOENT
open("/usr/openwin/lib/libc.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/java/lib/libc.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/libc.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/ncmp/lib/libc.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/libc.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/local/lib/S5/libc.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/local/lib/libc.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/lib/libc.so.1", O_RDONLY)		= 3
fstat(3, 0xFFBEE33C)				= 0
mmap(0xFF390000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 0) = 0xFF390000
mmap(0x00000000, 786432, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF180000
mmap(0xFF238000, 24720, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 688128) = 0xFF238000
munmap(0xFF228000, 65536)			= 0
memcntl(0xFF180000, 112632, MC_ADVISE, MADV_WILLNEED, 0, 0) = 0
close(3)					= 0
open("/usr/add-on/local/lib/S5/libaio.so.1", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/libaio.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/dt/lib/libaio.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/openwin/lib/libaio.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/java/lib/libaio.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/libaio.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/ncmp/lib/libaio.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/libaio.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/lib/libaio.so.1", O_RDONLY)		= 3
fstat(3, 0xFFBEE33C)				= 0
mmap(0xFF390000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 0) = 0xFF390000
mmap(0x00000000, 106496, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF260000
mmap(0xFF278000, 1584, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 32768) = 0xFF278000
munmap(0xFF268000, 65536)			= 0
memcntl(0xFF260000, 7184, MC_ADVISE, MADV_WILLNEED, 0, 0) = 0
close(3)					= 0
open("/usr/add-on/local/lib/S5/libdl.so.1", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/libdl.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/dt/lib/libdl.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/openwin/lib/libdl.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/java/lib/libdl.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/libdl.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/ncmp/lib/libdl.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/libdl.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/lib/libdl.so.1", O_RDONLY)		= 3
fstat(3, 0xFFBEE33C)				= 0
mmap(0xFF390000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 0) = 0xFF390000
close(3)					= 0
open("/usr/add-on/local/lib/S5/libmp.so.2", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/libmp.so.2", O_RDONLY)	Err#2 ENOENT
open("/usr/dt/lib/libmp.so.2", O_RDONLY)	Err#2 ENOENT
open("/usr/openwin/lib/libmp.so.2", O_RDONLY)	Err#2 ENOENT
open("/usr/java/lib/libmp.so.2", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/libmp.so.2", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/ncmp/lib/libmp.so.2", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/libmp.so.2", O_RDONLY) Err#2 ENOENT
open("/usr/lib/libmp.so.2", O_RDONLY)		= 3
fstat(3, 0xFFBEE33C)				= 0
mmap(0x00000000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF250000
mmap(0x00000000, 90112, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF160000
mmap(0xFF174000, 865, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 16384) = 0xFF174000
munmap(0xFF164000, 65536)			= 0
memcntl(0xFF160000, 3124, MC_ADVISE, MADV_WILLNEED, 0, 0) = 0
close(3)					= 0
mprotect(0x00010000, 39712, PROT_READ|PROT_WRITE|PROT_EXEC) = 0
open("/usr/platform/SUNW,Ultra-2/lib/libc_psr.so.1", O_RDONLY) = 3
fstat(3, 0xFFBEE1CC)				= 0
mmap(0xFF250000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 0) = 0xFF250000
mmap(0x00000000, 16384, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF150000
close(3)					= 0
mmap(0x00000000, 8192, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANON, -1, 0) = 0xFF140000
mprotect(0x00010000, 39712, PROT_READ|PROT_EXEC) = 0
open("/usr/add-on/local/lib/S5/libthread.so.1", O_RDONLY) Err#2 ENOENT
open("/opt/SUNWspro/lib/libthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/dt/lib/libthread.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/openwin/lib/libthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/java/lib/libthread.so.1", O_RDONLY)	Err#2 ENOENT
open("/usr/add-on/X11/lib/libthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/ncmp/lib/libthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/add-on/gnu/lib/libthread.so.1", O_RDONLY) Err#2 ENOENT
open("/usr/lib/libthread.so.1", O_RDONLY)	= 3
fstat(3, 0xFFBEE1CC)				= 0
mmap(0xFF250000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 0) = 0xFF250000
mmap(0x00000000, 245760, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0xFF100000
mmap(0xFF12E000, 6684, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, 3, 122880) = 0xFF12E000
mmap(0xFF130000, 45780, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_ANON, -1, 0) = 0xFF130000
munmap(0xFF11E000, 65536)			= 0
memcntl(0xFF100000, 30844, MC_ADVISE, MADV_WILLNEED, 0, 0) = 0
close(3)					= 0
munmap(0xFF250000, 8192)			= 0
lwp_self()					= 1
sigfillset(0xFF23E928)				= 0
brk(0x0002A620)					= 0
sysconfig(_CONFIG_PAGESIZE)			= 8192
sysconfig(_CONFIG_SEM_VALUE_MAX)		= 2147483647
lwp_self()					= 1
sigprocmask(SIG_SETMASK, 0x00000000, 0xFFBEE868) = 0
sigprocmask(SIG_SETMASK, 0xFF12EFE8, 0x00000000) = 0
sigaction(SIGLWP, 0xFF12F808, 0x00000000)	= 0
sigaction(SIGCANCEL, 0xFF12F808, 0x00000000)	= 0
sigaction(SIGSEGV, 0xFF12F808, 0x00000000)	= 0
brk(0x0002A620)					= 0
brk(0x0002C620)					= 0
sysconfig(_CONFIG_STACK_PROT)			= 7
mmap(0x00000000, 8454144, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_NORESERVE|MAP_ANON, -1, 0) = 0xFE800000
mprotect(0xFE800000, 8192, PROT_NONE)		= 0
mprotect(0xFE902000, 8192, PROT_NONE)		= 0
mprotect(0xFEA04000, 8192, PROT_NONE)		= 0
mprotect(0xFEB06000, 8192, PROT_NONE)		= 0
mprotect(0xFEC08000, 8192, PROT_NONE)		= 0
mprotect(0xFED0A000, 8192, PROT_NONE)		= 0
mprotect(0xFEE0C000, 8192, PROT_NONE)		= 0
mprotect(0xFEF0E000, 8192, PROT_NONE)		= 0
lwp_create(0xFFBEE5A0, __LWP_ASLWP, 0xFF00FDB4)	= 2
lwp_create()	(returning as new lwp ...)	= 0
lwp_mutex_lock(0xFF3E3780)			= 0
door_create(0xFF116B18, 0x00000000, 0x00000002)	= 3
getpid()					= 1100 [1099]
lwp_schedctl(SC_STATE|SC_BLOCK, 3, 0xFFBEE7A4)	= 0
door_info(3, 0xFF134A98)			= 0
mmap(0x00000000, 24576, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_NORESERVE|MAP_ANON, -1, 0) = 0xFF0F0000
mprotect(0xFF0F0000, 8192, PROT_NONE)		= 0
lwp_create(0xFFBEE588, LWP_DETACHED|LWP_SUSPENDED, 0xFF0F5DB4) = 3
lwp_create()	(returning as new lwp ...)	= 0
lwp_continue(3)					= 0
close(3)					= 0
lwp_schedctl(SC_STATE|SC_BLOCK, -1, 0xFF0F5CB4)	= 0
lwp_schedctl(SC_DOOR, 0, 0x00000000)		= 3
lwp_mutex_wakeup(0xFF3E3780)			= 0
lwp_mutex_lock(0xFF3E3780)			= 0
lwp_mutex_wakeup(0xFF3E3780)			= 0
lwp_mutex_lock(0xFF3E3780)			= 0
door_bind(3)					= 0
close(3)					= 0
lwp_mutex_wakeup(0xFF3E3780)			= 0
lwp_mutex_lock(0xFF3E3780)			= 0
lwp_mutex_wakeup(0xFF3E3780)			= 0
lwp_mutex_lock(0xFF3E3780)			= 0
sigaction(SIGWAITING, 0xFF12EE38, 0x00000000)	= 0
lwp_self()					= 1
sysconfig(_CONFIG_SEM_VALUE_MAX)		= 2147483647
write(3, " m a i n :   e r r f d =".., 14)	Err#9 EBADF
getrlimit(RLIMIT_NOFILE, 0xFFBED8A0)		= 0
dup(2)						= 3
fcntl(3, F_GETFL, 0x00000000)			= 8193
fstat64(3, 0xFFBED6C8)				= 0
fcntl(3, F_GETFD, 0x00000000)			= 0
fcntl(3, F_SETFD, 0x00000001)			= 0
fstat64(3, 0xFFBED8A8)				= 0
ioctl(3, TCGETA, 0xFFBED82C)			Err#25 ENOTTY
llseek(3, 0, SEEK_CUR)				= 12232
brk(0x0002C620)					= 0
brk(0x00030620)					= 0
getpid()					= 1100 [1099]
access("/usr/sbin", 4)				= 0
getuid()					= 201 [201]
getuid()					= 201 [201]
getgid()					= 309 [309]
getgid()					= 309 [309]
getuid()					= 201 [201]
open64("/etc/.name_service_door", O_RDONLY)	= 4
fcntl(4, F_SETFD, 0x00000001)			= 0
door_info(4, 0xFF23E7A0)			= 0
signotifywait()					= 32
door_call(4, 0xFFBECEB0)			= 0
lwp_mutex_wakeup(0xFF3E3780)			= 0
getrlimit(RLIMIT_NOFILE, 0xFFBECE38)		= 0
lwp_mutex_lock(0xFF3E3780)			= 0
lwp_mutex_wakeup(0xFF3E3780)			= 0
door_return(0x00000000, 0, 0x00000000, 0)	= 0
lwp_mutex_lock(0xFF3E3780)			= 0
lwp_sigredirect(0, SIGWAITING, 0x00000000)	= 0
mmap(0x00000000, 24576, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_NORESERVE|MAP_ANON, -1, 0) = 0xFF0E0000
sigaction(SIGWAITING, 0xFF12EE58, 0x00000000)	= 0
mprotect(0xFF0E0000, 8192, PROT_NONE)		= 0
mmap(0x00000000, 16384, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_NORESERVE|MAP_ANON, -1, 0) = 0xFF0D0000
mprotect(0xFF0D0000, 8192, PROT_NONE)		= 0
lwp_create(0xFF0F59E0, LWP_DETACHED|LWP_SUSPENDED, 0xFF0E5DB4) = 4
lwp_create()	(returning as new lwp ...)	= 0
lwp_continue(4)					= 0
lwp_schedctl(SC_STATE|SC_BLOCK, -1, 0xFF0E5CB4)	= 0
lwp_create(0xFF00FA98, LWP_DETACHED|LWP_SUSPENDED, 0xFF0D3DB4) = 5
lwp_create()	(returning as new lwp ...)	= 0
lwp_continue(5)					= 0
lwp_schedctl(SC_DOOR, 0, 0x00000000)		= 5
door_bind(5)					= 0
lwp_schedctl(SC_STATE|SC_BLOCK, -1, 0xFF0D3CB4)	= 0
lwp_mutex_lock(0xFF3E3780)			= 0
lwp_mutex_wakeup(0xFF3E3780)			= 0
close(5)					= 0
time()						= 1014257351
lwp_mutex_wakeup(0xFF3E3780)			= 0
lwp_mutex_lock(0xFF3E3780)			= 0
open64("/etc/udomains", O_RDONLY|O_LARGEFILE)	= 5
lwp_mutex_lock(0xFF1355B0)			= 0
time()						= 1014257351
fcntl(5, F_GETFD, 0x00000000)			= 0
fcntl(5, F_SETFD, 0x00000001)			= 0
fstat64(5, 0xFFBECE40)				= 0
ioctl(5, TCGETA, 0xFFBECDC4)			Err#25 ENOTTY
llseek(5, 0, SEEK_CUR)				= 0
brk(0x00030620)					= 0
brk(0x00032620)					= 0
read(5, " #   U D O M A I N S   (".., 8192)	= 227
read(5, 0x00030000, 8192)			= 0
close(5)					= 0
uname(0xFFBECF20)				= 1
time()						= 1014257351
time()						= 1014257351
lwp_create(0xFFBED468, LWP_DETACHED|LWP_SUSPENDED, 0xFEE0BDB4) = 6
lwp_create()	(returning as new lwp ...)	= 0
lwp_continue(6)					= 0
lwp_self()					= 6
lwp_sema_post(0xFF12FA30)			= 0
lwp_schedctl(SC_STATE|SC_BLOCK, -1, 0xFEE0BD14)	= 0
lwp_mutex_wakeup(0xFF3E3780)			= 0
lwp_mutex_lock(0xFF3E3780)			= 0
lwp_mutex_wakeup(0xFF3E3780)			= 0
lwp_mutex_lock(0xFF3E3780)			= 0
sigaction(SIGALRM, 0xFEE0BC40, 0x00000000)	= 0
sigprocmask(SIG_BLOCK, 0xFEE0BDE0, 0x00000000)	= 0
sigprocmask(SIG_UNBLOCK, 0xFF12FA20, 0x00000000) = 0
lwp_sema_wait(0xFF12FA30)			= 0
sigprocmask(SIG_BLOCK, 0xFF12FA20, 0x00000000)	= 0
setitimer(ITIMER_REAL, 0xFEE0BC70, 0x00000000)	= 0
sigprocmask(SIG_UNBLOCK, 0xFF12FA20, 0x00000000) = 0
lwp_sema_wait(0x0002A448)	(sleeping...)
signotifywait()			(sleeping...)
lwp_cond_wait(0xFF1355A0, 0xFF1355B0, 0xFF0F5BF0) (sleeping...)
door_return(0x00000000, 0, 0x00000000, 0) (sleeping...)
lwp_cond_wait(0xFF1355A0, 0xFF1355B0, 0xFF0D3C48) (sleeping...)
lwp_sema_wait(0xFF12FA30)	(sleeping...)
    Received signal #14, SIGALRM, in lwp_sema_wait() [caught]
lwp_sema_wait(0xFF12FA30)			Err#91 ERESTART
sigprocmask(SIG_SETMASK, 0xFEE0BDE0, 0x00000000) = 0
lwp_sema_post(0x0002A448)			= 0
lwp_sema_wait(0x0002A448)			= 0
sigprocmask(SIG_SETMASK, 0xFF13ADB8, 0x00000000) = 0
setcontext(0xFEE0B9C8)
sigprocmask(SIG_BLOCK, 0xFF12FA20, 0x00000000)	= 0
sigprocmask(SIG_UNBLOCK, 0xFF12FA20, 0x00000000) = 0
close(3)					= 0
llseek(0, 0, SEEK_CUR)				= 1930555
_exit(125)
