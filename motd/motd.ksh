#!/usr/bin/ksh
# MOTD


MDIRS=" /var/adm /home/admin/var /etc "

# print a default MOTD if no others are appropriate
F_DEFMOTD=0


: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export LOCAL EXTRA

FPATH=${LOCAL}/fbin
if [[ ! -d $FPATH ]] ; then
  FPATH=${EXTRA}/fbin
fi

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${EXTRA}/bin
pathadd LD_LIBRARY_PATH ${EXTRA}/lib


integer c=0
userinfo - uid groupname | while read A J ; do
  case ${c} in
  0)
    : ${UID:=${A}}
    ;;
  1)
    : ${GROUPNAME:=${A}}
    ;;
  esac
  (( c += 1 ))
done



if [[ ${UID} -ge 100 ]] ; then
  (( c = 0 ))
  for D in ${MDIRS} ; do
    F=${D}/${GROUPNAME}.motd
    if [[ -r $F ]] ; then
      shcat $F
      (( c += 1 ))
    fi
  done
  if [[ $F_DEFMOTD -gt 0 ]] && [[ ${c} == 0 ]] ; then
    if [[ -r /etc/motd ]] ; then
      shcat /etc/motd
    fi
  fi
fi



