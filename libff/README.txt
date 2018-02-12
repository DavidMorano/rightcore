LIBFF

Fixed Standard I/O Library

These subroutines call the UNIX Standard I/O library but with a fixed
up subroutine call interface.  Like BIO and SFIO, all subroutines take
the "handle" as the first argument.  The "Open" type calls work the
same as the regular Standard I/O (and like SFIO) in that they return
the handle as the return value from the call.

