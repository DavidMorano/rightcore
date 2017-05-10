MAKECP

This small program is close to the equivalent of:
	$ cp -p <src> <dst>
but does not attempt to retain the owner of the source file.  This is
(extremely) useful in Makefiles for copying a source file when the
source file has SUID or SGID privileges but the current user is not
the original privileged user.

