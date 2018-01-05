#!/usr/bin/ksh
# EXTLINES

FILES=
STRIDE=10
OFFSET=1

RF_DEBUG=false


S=files
OS=
for A in "${@}" ; do

  case ${A} in
  '-D' )
    RF_DEBUG=true
    ;;
  '-s' )
    OS=${S}
    S=stride
    ;;
  '-o' )
    OS=${S}
    S=offset
    ;;
  '-'[a-zA-Z]* )
    echo "${P}: unknown option \"${A}\"" >&2
    exit 1
    ;;
  '-' )
    FILES="${FILES} -"
    ;;
  * )
    case ${S} in
    files )
      FILES="${FILES} ${A}"
      ;;
    stride )
      STRIDE="${A}"
      ;;
    offset )
      OFFSET="${A}"
      ;;
    esac
    S=${OS}
    ;;
  esac

done


TF=/tmp/qp${$}

cleanup() {
  rm -f $TF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

cat <<\EOF > $TF

BEGIN {
	if (stride == 0) stride = 1 ;
}

{
	if (NR >= offset) {
	    mod = offset % stride ;
	    if (((NR - 1) % stride) == mod) {
		print $0
	    }
	}
}

EOF


for F in ${FILES} ; do
  nawk -f ${TF} -v stride=${STRIDE} -v offset=${OFFSET} ${F}
done

cleanup


