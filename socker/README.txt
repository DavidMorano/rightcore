SOCKER

This program listens on sockets for connections and spawns an appropriate
server.

Synopsis:
$ socker -d

If there is contention for a PID mutex or a file mutex, use:
$ socker -dq

To explicitly specify a configuration file:
$ socker -d -C ${PCS}/etc/socker/engeering.conf
or
$ socker -d ${PCS}/etc/socker/engeering.conf

To listen using a explicit program base:
$ socker -d -R ${PCS}/etc/socker/testbase

