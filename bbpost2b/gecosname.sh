
gecosname() {
  U=${LOGNAME}
  if [ $# -gt 0 ] ; then U=${1} ; fi

  L=`grep "^${U}:" /etc/passwd `
  if [ $? -ne 0 ] ; then

    L=`ypmatch $U passwd ` 2> /dev/null
    if [ $? -ne 0 ] ; then return 1 ; fi

  fi

  N=`${ECHO} $L | cut -d: -f5 `

  case "${N}" in

  *'-'*'('* )
    N=`${ECHO} $N | cut -d'-' -f2 `
    N=`${ECHO} $N | cut -d'(' -f1 `
    ;;

  esac

  ${ECHO} $N
  return 0
}

