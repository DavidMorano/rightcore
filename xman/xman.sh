 # <-- force CSH to use Bourne shell
# XMAN


#
# This is a special 'xman' such that the Sun Solaris versions
# of 'nroff' and 'eqn' get called.  These are required for some
# of Sun's new manual pages.  At least this is what I ** think **
# this version of XMAN is doing !
#
# OK, new information.  It now appears that we should rather let
# XMAN use the AT&T DWB version of NROFF ** rather ** than the
# native Sun Microsystems version of NROFF !  I do not know what
# "gives" either but that is what seems to work best.  Your feedback
# is welcome !
#


: ${DWBHOME:=/usr/add-on/dwb}
: ${OPENWINHOME:=/usr/openwin}
export DWBHOME OPENWINHOME


# use Sun Microsystems native version NROFF
#PATH=/usr/bin:${PATH}

# use AT&T DWB version of NROFF
PATH=${DWBHOME}/bin:${PATH}


export PATH

exec execname ${XDIR}/bin/xman xman "${@}"


