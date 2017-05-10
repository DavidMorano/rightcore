#!/usr/extra/bin/ksh
# SHOWMEMCONF

: ${LOCAL:=/usr/add-on/local}
export LOCAL

DN=/dev/null
LKCMD=${LOCAL}/lib/liblkcmd.so

builtin -f ${LKCMD} sysval 2> ${DN}

PMT=$( sysval pmt )
PMA=$( sysval pma )
PMU=$( sysval pmu )

print -- "total memory (MB) ${PMT}"
print -- "available memory (MB) ${PMA}"
print -- "memory utilization ${PMU}"


