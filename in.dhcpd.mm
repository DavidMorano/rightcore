'\" te
.TH in\&.dhcpd 1M "13 Mar 2001" "SunOS 5.8" "Maintenance Commands"
.SH "NAME"
in\&.dhcpd \- Dynamic Host Configuration Protocol server
.SH "SYNOPSIS"
.PP
\fB/usr/lib/inet/in\&.dhcpd\fR [ -\fBdenv\fR ]  [ -\fBh\fR \fIrelay_hops\fR ]  [ -\fBi\fR \fIinterface,\fR \&.\&.\&. ]  [ -\fBl\fR \fIsyslog_local_facility\fR ]  [ -\fBb\fR automatic | manual ]  [ -\fBo\fR \fIDHCP_offer_time\fR ]  [ -\fBt\fR \fIdhcptab_rescan_interval\fR ] 
.PP
\fB/usr/lib/inet/in\&.dhcpd\fR [ -\fBdv\fR ]  [ -\fBh\fR \fIrelay_hops\fR ]  [ -\fBi\fR \fIinterface,\fR \&... ]  [ -\fBl\fR \fIsyslog_local_facility\fR ]  -\fBr\fR \fIIP_address\fR | \fIhostname,\fR \&.\&.\&. 
.SH "DESCRIPTION"
.PP
\fBin\&.dhcpd\fR is a daemon that responds to Dynamic Host Configuration Protocol (\fBDHCP\fR) requests and optionally to \fBBOOTP\fR protocol requests\&. The daemon forks a copy of itself that runs as a background process\&. It must be run as root\&. The daemon has two run modes, \fBDHCP\fR server (with optional \fBBOOTP\fR compatibility mode) and \fBBOOTP\fR relay agent mode\&. 
.PP
The first line in the \fBSYNOPSIS\fR section illustrates the options available in the DHCP/BOOTP server mode\&. The second line in the SYNOPSIS section illustrates the options available when the daemon is run in \fBBOOTP\fR relay agent mode\&.
.PP
The \fBDHCP\fR and \fBBOOTP\fR protocols are used to provide configuration parameters to Internet hosts\&. Client machines are allocated their \fBIP\fR addresses as well as other host configuration parameters through
this mechanism\&.
.PP
The \fBDHCP\fR/\fBBOOTP\fR daemon manages two types of \fBDHCP\fR data tables: the \fBdhcptab\fR configuration table and the DHCP network tables\&.
.PP
See \fBdhcptab\fR(4) regarding the dhcptab configuration table and \fBdhcp_network\fR(4) regarding the \fBDHCP\fR network tables\&.
.PP
The \fBdhcptab\fR contains macro definitions defined using a \fBtermcap\fR-like syntax which permits network administrators to define groups of \fBDHCP\fR configuration parameters to be returned to clients\&. However, a \fBDHCP/BOOTP\fR server always returns hostname, network broadcast address, network subnet mask, and \fBIP\fR maximum transfer unit (\fBMTU\fR) if requested by a client attached to the same network as the server machine\&. If those options have not been explicitly configured
in the \fBdhcptab\fR, \fBin\&.dhcpd\fR returns reasonable default values\&.
.PP
The \fBdhcptab\fR is read at startup, upon receipt of a \fBSIGHUP\fR signal, or periodically as specified by the -\fBt\fR option\&. A \fBSIGHUP\fR (sent using the command \fBpkill -HUP in\&.dhcpd\fR)
causes the \fBDHCP/BOOTP\fR daemon to reread the \fBdhcptab\fR within an interval from 0-60 seconds (depending on where the DHCP daemon is in its polling cycle)\&. For busy servers, users should run \fB/etc/init\&.d/dhcp stop\fR, followed by \fB/etc/init\&.d/dhcp
start\fR to force the \fBdhcptab\fR to be reread\&.
.PP
The DHCP network tables contain mappings of client identifiers to \fBIP\fR addresses\&. These tables are named after the network they support and the datastore used to maintain them\&. 
.PP
The DHCP network tables are consulted during runtime\&. A client request received from a network for which no DHCP network table exists is ignored\&.
.PP
This command may change in future releases of Solaris software\&. Scripts, programs, or procedures that use this command might need modification when upgrading to future Solaris software releases\&.The command line options provided with the \fBin\&.dhcpd\fR daemon are used only for the current
session, and include only some of the server options you can set\&. The \fBdhcpsvc\&.conf\fR(4) contains all the server default settings, and can be modified by using
the \fBdhcpmgr\fR utility\&. See \fBdhcpsvc\&.conf\fR(4) and \fBdhcpmgr\fR(1M) for more details\&.
.SH "OPTIONS"
.PP
The following options are supported: 
.IP "-\fBb\fR \fB automatic | manual\fR" 6
This option enables \fBBOOTP\fR compatibility mode, allowing the \fBDHCP\fR server to respond to \fBBOOTP\fR clients\&.
The option argument specifies whether the \fBDHCP\fR server should automatically allocate permanent lease \fBIP\fR addresses to requesting \fBBOOTP\fR clients if the clients are not registered in the DHCP network tables (\fBautomatic\fR) or respond only to \fBBOOTP\fR clients who have been manually registered in the DHCP network tables (\fB manual\fR)\&. This option only affects \fBDHCP\fR server mode\&.
.IP "-\fBd\fR" 6
Debugging mode\&. The daemon remains as a foreground process, and displays verbose messages as it processes \fBDHCP\fR and/or \fBBOOTP\fR datagrams\&. Messages are displayed on the current TTY\&.
This option can be used in both DHCP/BOOTP server mode and \fBBOOTP\fR relay agent mode\&.
.IP "-\fBh\fR\fI relay_hops\fR" 6
Specifies the maximum number of relay agent hops that can occur before the daemon drops the DHCP/BOOTP datagram\&. The default number of relay agent hops is 4\&. This option affects both DHCP/BOOTP server mode
and \fBBOOTP\fR relay agent mode\&.
.IP "-\fBi\fR\fI interface, \&.\|\&.\|\&.\fR" 6
Selects the network interfaces that the daemon should monitor for DHCP/BOOTP datagrams\&. The daemon ignores DHCP/BOOTP datagrams on network interfaces not specified in this list\&. This
option is only useful on machines that have multiple network interfaces\&. If this option is not specified, then the daemon listens for DHCP/BOOTP datagrams on all network interfaces\&. The option argument consists of a comma-separated list of interface names\&. It affects both DHCP/BOOTP server and \fBBOOTP\fR relay agent run modes\&.
.IP "-\fBl\fR \fIsyslog_local_facility\fR" 6
The presence of this option turns on  transaction logging for the \fBDHCP\fR server or \fBBOOTP\fR relay agent\&. The value specifies the \fBsyslog\fR local facility (an integer from \fB0\fR to \fB7\fR inclusive) the DHCP daemon should use for tagging the transactions\&. Using a facility separate from the \fBLOG_DAEMON\fR facility allows the network administrator to capture these transactions separately
from other DHCP daemon events for such purposes as generating transaction reports\&. See \fBsyslog\fR(3C), for details about local facilities\&. Transactions are logged using
a record with 9 space-separated fields as follows:
.RS 
.TP 3
1. 
Protocol: 
.sp
.nf
.sp
  Relay mode:     "BOOTP"
  Server mode:    "BOOTP" or "DHCP" based upon client
                       type\&. 
.fi
.sp
.TP 3
2. 
Type:
.sp
.nf
.sp
Relay mode:     "RELAY-CLNT", "RELAY-SRVR" 
Server mode:    "ASSIGN", "EXTEND", "RELEASE", 
                    "DECLINE", "INFORM", "NAK" "ICMP-ECHO\&." 
.fi
.sp
.TP 3
3. 
Transaction time: absolute time in seconds (unix time)
.TP 3
4. 
Lease time: 
.nf
.sp
Relay mode:     Always 0\&. 
Server mode:    0 for ICMP-ECHO events, absolute time in
                    seconds (unix time)  otherwise
.fi
.sp
.TP 3
5. 
Source IP address: Dotted Internet form
.nf
.sp
Relay mode:     Relay interface IP on RELAY-CLNT, 
                       INADDR_ANY on RELAY-SRVR\&.
Server mode:    Client IP\&. 
.fi
.sp
.TP 3
6. 
Destination IP address: Dotted Internet form
.nf
.sp
Relay mode:     Client IP on RELAY-CLNT, Server IP on
                                          RELAY-SRVR\&.
Server mode:    Server IP\&.
.fi
.sp
.TP 3
7. 
Client Identifier: Hex representation (0-9, A-F)
.nf
.sp
Relay mode:     MAC address                         
Server mode:    BOOTP - MAC address; DHCP - client id 
.fi
.sp
.TP 3
8. 
Vendor Class identifier (white space converted to                        periods (\&.))\&.
.nf
.sp
Relay mode:     Always "N/A"
Server mode:    Vendor class ID tokenized by 
                       converting white space characters 
                       to periods (\&.) 
.fi
.sp
.TP 3
9. 
MAC address: Hex representation (0-9, A-F)
.nf
.sp
Relay mode:     MAC address
Server mode:    MAC address
.fi
.sp
.RE
.sp
.sp
The format of this record is subject to change between releases\&.
.sp
Transactions are logged to the console if daemon is in debug mode (-\fBd\fR)\&. 
.sp
Logging transactions impact daemon performance\&.
.sp
.RS
It is suggested that you manage log file size periodically using a script run by \fBcron\fR(1M) and sending \fBsyslogd\fR(1M) a \fBSIGHUP\fR signal\&. You could, for example, clone \fB/usr/lib/newsyslog\fR and alter it to match your \fBDHCP\fR logging requirements\&.
.RE
.IP "-\fBn\fR" 6
Disable automatic duplicate \fBIP\fR address detection\&. When this option is specified, the \fBDHCP\fR server does not attempt to verify that an \fBIP address it is about
to\fR offer a client is not in use\&. By default, the \fBDHCP\fR server pings an \fBIP\fR address before offering it to a DHCP/BOOTP client, to verify that the address is not in use by another machine\&.
.IP "-\fBo\fR\fI DHCP_offer_time\fR" 6
Specifies the number of seconds the \fBDHCP\fR server should cache the offers it has extended to discovering \fBDHCP\fR clients\&. The default setting
is \fB10\fR seconds\&. On slow network media, this value can be increased to compensate for slow network performance\&. This option affects only \fBDHCP\fR server mode\&.
.IP "-\fBr\fR\fI IP_address | hostname, \&.\|\&.\|\&.\fR" 6
This option enables \fBBOOTP\fR relay agent mode\&. The option argument specifies a comma-separated list of \fBIP\fR addresses
or hostnames of \fBDHCP\fR or \fBBOOTP\fR servers to which the relay agent is to forward \fBBOOTP\fR requests\&. When the daemon is started in this mode, any \fBDHCP\fR tables are ignored, and the
daemon simply acts as a \fBBOOTP\fR relay agent\&.
.sp
A \fBBOOTP\fR relay agent listens to \fBUDP\fR port 68, and forwards \fBBOOTP\fR request packets received on this port to the destinations specified on the command line\&. It supports the \fBBROADCAST\fR flag described in \fBRFC\fR 1542\&. A \fBBOOTP\fR relay agent can run on any machine that has knowledge of local routers, and thus does not have to be an Internet gateway machine\&.
.sp
.RS
Note that the proper entries must be made to the \fBnetmasks\fR database so that the \fBDHCP\fR server being served by the \fBBOOTP\fR relay agents can identify the subnet mask of the foreign BOOTP/DHCP client\&'s network\&. See \fBnetmasks\fR(4) for the format and use of this database\&.
.RE
.IP "-\fBt\fR\fI dhcptab_rescan_interval\fR" 6
Specifies the interval in minutes that the \fBDHCP\fR server should use to schedule the automatic rereading of the \fBdhcptab\fR information\&. Typically,
you would use this option if the changes to the \fBdhcptab\fR are relatively frequent\&. Once the contents of the \fBdhcptab\fR have stabilized, you can turn off this option to avoid needless reinitialization of the server\&.
.IP "-\fBv\fR" 6
Verbose mode\&. The daemon displays more messages than in the default mode\&. Note that verbose mode can reduce daemon efficiency due to the time taken to display messages\&. Messages are displayed to the current \fBTTY\fR
if the debugging option is used; otherwise, messages are logged to the \fBsyslogd\fR facility\&. This option can be used in both DHCP/BOOTP server mode and \fBBOOTP\fR relay agent mode\&.
.SH "EXAMPLES"
.PP
\fBExample 1: Starting a \fBDHCP\fR Server in \fBBOOTP\fR Compatibility Mode\fR
.PP
The following command starts a \fBDHCP\fR server in \fBBOOTP\fR compatibility mode, permitting the server to automatically allocate permanent \fBIP\fR addresses to \fBBOOTP\fR clients which
are not registered in the server\&'s table; limits the server\&'s attention to incoming datagrams on network devices \fBle2\fR and \fBtr0\fR; drops \fBBOOTP\fR packets whose hop count exceeds 2; configures the \fBDHCP\fR server
to cache extended \fBDHCP\fR offers for 15 seconds; and schedules \fBdhcptab\fR rescans to occur every 10 minutes:
.PP
.PP
.nf
\fB# in\&.dhcpd -\fBi\fR le2,tr0 -\fBh\fR 2 -\fBo\fR 15 -\fBt\fR 10 -\fBb\fR automatic\fR
.fi
.PP
\fBExample 2: Starting the Daemon in \fBBOOTP\fR Relay Agent Mode\fR
.PP
The following command starts the daemon in \fBBOOTP\fR relay agent mode, registering the hosts \fBbladerunner\fR and \fB10\&.0\&.0\&.5\fR as relay destinations, with debugging and verbose modes enabled, and drops \fBBOOTP\fR
packets whose hop count exceeds 5:
.PP
.PP
.nf
\fB# in\&.dhcpd -\fBd\fR -\fBv\fR -\fBh\fR 5 -\fBr\fR bladerunner,10\&.0\&.0\&.5\fR
.fi
.SH "FILES"
.IP "\fB/etc/inet/dhcpsvc\&.conf\fR" 6
 
.IP "\fB/etc/init\&.d/dhcp\fR" 6
 
.IP "\fB/etc/init/hosts\fR" 6
 
.IP "\fB/usr/lib/inet/dhcp/nsu/rfc2136\&.so\&.1\fR" 6
 
.SH "ATTRIBUTES"
.PP
See \fBattributes\fR(5) for descriptions of the following attributes:
.sp
.TS
.if \n+(b.=1 .nr d. \n(.c-\n(c.-1
.de 35
.ps \n(.s
.vs \n(.vu
.in \n(.iu
.if \n(.u .fi
.if \n(.j .ad
.if \n(.j=0 .na
..
.nf
.nr #~ 0
.if n .nr #~ 0.6n
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.fc
.nr 33 \n(.s
.rm 80 81
.nr 80 0
.nr 38 \wATTRIBUTE TYPE
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wAvailability
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wInterface Stability
.if \n(80<\n(38 .nr 80 \n(38
.80
.rm 80
.nr 38 2.750000in
.if \n(80<\n(38 .nr 80 \n(38
.nr 81 0
.nr 38 \wATTRIBUTE VALUE
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wSUNWdhcsu
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wEvolving
.if \n(81<\n(38 .nr 81 \n(38
.81
.rm 81
.nr 38 2.750000in
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 1n
.nr 79 0
.nr 40 \n(79+(1*\n(38)
.nr 80 +\n(40
.nr 41 \n(80+(3*\n(38)
.nr 81 +\n(41
.nr TW \n(81
.nr TW +1*\n(38
.if t .if \n(TW>\n(.li .tm Table at line 217 file Input is too wide - \n(TW units
.ne 3v+0p
.if n .ne 8v
.fc  
.nr #T 0-1
.nr #a 0-1
.nr #a 0-1
.eo
.de T#
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.mk ##
.nr ## -1v
.ls 1
.if \n(#T>=0 .nr #a \n(#T
.if \n(T. .vs \n(.vu-\n(.sp
.if \n(T. \h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.if \n(T. .vs
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'|0'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'(|\n(41u+|\n(80u)/2u'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'|\n(TWu'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'
.ls
..
.ec
.nr 36 \n(.v
.vs \n(.vu-\n(.sp
\h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.vs \n(36u
.mk #a
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'ATTRIBUTE TYPE\h'|\n(41u'ATTRIBUTE VALUE
.nr 36 \n(.v
.vs \n(.vu-\n(.sp
\h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.vs \n(36u
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Availability\h'|\n(41u'SUNWdhcsu
.nr 36 \n(.v
.vs \n(.vu-\n(.sp
\h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.vs \n(36u
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Interface Stability\h'|\n(41u'Evolving
.fc
.nr T. 1
.T# 1
.35
.nr #a 0
.TE
.if \n-(b.=0 .nr c. \n(.c-\n(d.-7
.sp
.SH "SEE ALSO"
.PP
\fBcron\fR(1M), \fBdhcpmgr\fR(1M), \fBdhtadm\fR(1M), \fBpntadm\fR(1M), \fBsyslogd\fR(1M), \fBsyslog\fR(3C), \fBdhcpsvc\&.conf\fR(4), \fBdhcp_network\fR(4), \fBdhcptab\fR(4), \fBethers\fR(4), \fBhosts\fR(4), \fBnetmasks\fR(4), \fBnsswitch\&.conf\fR(4), \fBattributes\fR(5), \fBdhcp\fR(5)
.PP
\fISystem Administration Guide, Volume 3\fR
.PP
Alexander, S\&., and R\&. Droms, \fIDHCP Options and BOOTP Vendor Extensions\fR, RFC 2132, Silicon Graphics, Inc\&., Bucknell University, March 1997\&.
.PP
Droms, R\&., \fIInteroperation Between DHCP and BOOTP\fR, RFC 1534, Bucknell University, October 1993\&.
.PP
Droms, R\&., \fIDynamic Host Configuration Protocol\fR, RFC 2131, Bucknell University, March 1997\&.
.PP
Wimer, W\&., \fIClarifications and Extensions for the Bootstrap Protocol\fR, RFC 1542, Carnegie Mellon University, October 1993\&.
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 13:40
