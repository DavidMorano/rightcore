#!/usr/bin/ksh
# WEBCOUNTS


if [[ ${#} -gt 0 ]] ; then
  webcounter -l -db ${1}
fi


