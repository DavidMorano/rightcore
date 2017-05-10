#!/bin/ksh
# STARTX

#EF=/tmp/startx.err
#touch $EF 2> /dev/null
#chmod ugo+rw $EF
#exec 2> $EF
#set -x

F_DEBUG=0

# 
# This is just a sample implementation of a slightly less primitive 
# interface than xinit.  It looks for user .xinitrc and .xserverrc
# files, then system xinitrc and xserverrc files, else lets xinit choose
# its default.  The system xinitrc should probably do things like check
# for .Xresources files and merge them in, startup up a window manager,
# and pop a clock and serveral xterms.
#
# Site administrators are STRONGLY urged to write nicer versions.
# 


: ${X_DEBUG:=0}
export X_DEBUG

if [ $X_DEBUG -ne 0 ] ; then
  PIPE=/tmp/startx_pipe
  mkpipe $PIPE
  tee < $PIPE /tmp/startx.err >&2 &
  exec 2> $PIPE
fi

: ${XDIR:=/usr/add-on/X11}
export XDIR

: ${OPENWINHOME:=/usr/openwin}

echo ${PATH} | fgrep "${XDIR}/bin" > /dev/null
if [ $? -ne 0 ] ; then
  if [ -d "${OPENWINHOME}" ] ; then

    echo ${PATH} | fgrep "${OPENWINHOME}/bin" > /dev/null
    if [ $? -ne 0 ] ; then
#      PATH=${PATH}:${XDIR}/bin
      PATH=${PATH}:${XDIR}/bin:${OPENWINHOME}/bin
    else
      PATH=${XDIR}/bin:${PATH}
    fi
  else
    PATH=${PATH}:${XDIR}/bin
  fi
fi
export PATH

if [ -n "${LD_LIBRARY_PATH}" ] ; then

  echo $LD_LIBRARY_PATH | fgrep ${XDIR}/lib > /dev/null
  if [ $? -ne 0 ] ; then
    LD_LIBRARY_PATH=${XDIR}/lib:${LD_LIBRARY_PATH}
  fi

  if [ -d "${OPENWINHOME}" ] ; then
    echo $LD_LIBRARY_PATH | fgrep ${OPENWINHOME}/lib > /dev/null
    if [ $? -ne 0 ] ; then
      LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${OPENWINHOME}/lib
    fi
  fi

else

  if [ -d "${OPENWINHOME}" ] ; then
    LD_LIBRARY_PATH=${XDIR}/lib:${OPENWINHOME}/lib
    export LD_LIBRARY_PATH
  fi

fi


XINIT=xinit
if [ -d "${OPENWINHOME}" ] ; then

  A=${XDIR}/lib/X11/%L/%T/%N%S
  A=${A}:${XDIR}/lib/X11/%T/%N%S
  A=${A}:${OPENWINHOME}/lib/locale/%L/%T/%N%S
  A=${A}:${OPENWINHOME}/lib/%T/%N%S
  XFILESEARCHPATH=${A}

  HELPPATH=${OPENWINHOME}/lib/locale:${OPENWINHOME}/lib/help
  export HELPPATH

  if [ -f $OPENWINHOME/bin/set_locale_env ]; then
    . $OPENWINHOME/bin/set_locale_env
  fi

#  XINIT=${OPENWINHOME}/bin/xinit

else

  A=${XDIR}/lib/X11/%L/%T/%N%S
  A=${A}:${XDIR}/lib/X11/%T/%N%S
  XFILESEARCHPATH=${A}

fi

export XFILESEARCHPATH


# handle what framebuffers to use

if [ -z "${XDEVICES}" ] ; then

XDEVICES=/dev/leo0:/dev/cgfourteen0
XDEVICES=${XDEVICES}:/dev/cgsix0:/dev/cgthree0
XDEVICES=${XDEVICES}:/dev/bwtwo0:/dev/bwtwo1
XDEVICES=${XDEVICES}:/dev/cgfour1:/dev/cgfour1
XDEVICES=${XDEVICES}:/dev/fb
export XDEVICES

fi


# set an alternate LOGNAME ('LOGNAME_X11') for stupid broken 'xinit's

if [ -n "${LOGNAME}" ] ; then
  LOGNAME_X11=${LOGNAME}
else
  LOGNAME_X11=`logname `
fi
export LOGNAME_X11


# rest of initialization

userclientrc=${XINITRC:-${HOME}/.xinitrc}
userserverrc=${HOME}/.xserverrc
sysclientrc=${XDIR}/lib/X11/xinit/xinitrc
sysserverrc=${XDIR}/lib/X11/xinit/xserverrc
clientargs=""
serverargs=""

if [ -f $userclientrc ]; then
    clientargs=$userclientrc
else if [ -f $sysclientrc ]; then
    clientargs=$sysclientrc
fi
fi

if [ -f $userserverrc ]; then
    serverargs=$userserverrc
else if [ -f $sysserverrc ]; then
    serverargs=$sysserverrc
fi
fi

whoseargs="client"
while [ "x$1" != "x" ]; do

    case "$1" in

	/''*|\.*)	if [ "$whoseargs" = "client" ]; then
		    clientargs="$1"
		else
		    serverargs="$1"
		fi ;;

	--)	whoseargs="server" ;;

	*)	if [ "$whoseargs" = "client" ]; then
		    clientargs="$clientargs $1"
		else
		    serverargs="$serverargs $1"
		fi ;;

    esac

    shift

done

if [ -x /bin/ksh ] ; then

  SHELL=/bin/ksh
  export SHELL

fi

: ${LOCAL:=/usr/add-on/local}

DEFHOSTS=${LOCAL}/etc/startx/xhosts
if [ -r ${DEFHOSTS} ] ; then {

  sleep 30
  while read H J ; do

    case "${H}" in

    '' | '#'* )
      ;;

    * )
      xhost +${H}
      ;;

    esac

  done < ${DEFHOSTS} > /dev/null 2>&1
 
} & fi

cleanup() {
  if tty > /dev/null ; then kbd_mode -a ; fi
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

if [ $X_DEBUG = true ] ; then
  echo "${0}: just before 'xinit'" >&2
fi

# Initialize X window system.

popdir() {
  if [ -z "${1}" ] ; then
    return 1
  fi
  if [ ! -w /tmp/${1} ] ; then
    rm -fr /tmp/${1}
    if [ -d /tmp/${1} ] ; then
      return 1
    fi
    mkdir -m 0777 /tmp/${1}
    if [ ! -w /tmp/${1} ] ; then
      return 1
    fi
  fi
  return 0
}

popdir .X11-pipe
RS=$?
if [ $RS -ne 0 ] ; then
  echo "${0}: could not create X11 directories" >&2
  exit 1
fi

popdir .X11-unix
RS=$?
if [ $RS -ne 0 ] ; then
  echo "${0}: could not create X11 directories" >&2
  exit 1
fi

if [ $F_DEBUG -ne 0 ] ; then
  echo "${0}: XINIT start up" >&2
  exit 1
fi

${XINIT} $clientargs -- $serverargs

# recover the Sun console keyboard
cleanup

exit 0


