'\" te
.TH dhcpconfig 1M "13 Mar 2001" "SunOS 5.8" "Maintenance Commands"
.SH "NAME"
dhcpconfig \- DHCP service configuration utility
.SH "SYNOPSIS"
.PP
\fBdhcpconfig\fR -\fBD\fR  -\fBr\fR\ \fIresource\fR  -\fBp\fR\ \fIpath\fR  [ -\fBu\fR\ \fIuninterpreted\fR ]  [ -\fBl\fR\ \fIlease_length\fR ]  [ -\fBn\fR\  ]  [ -\fBd\fR\ \fIDNS_domain\fR ]  [ -\fBa\fR\ \fIDNS_server_addresses\fR ]  [ -\fBh\fR\ \fIhosts_resource\fR ]  [ -\fBy\fR\ \fIhosts_domain\fR ] 
.PP
\fBdhcpconfig\fR -\fBR\fR\ \fIserver_addresses\fR 
.PP
\fBdhcpconfig\fR -\fBU\fR  [ -\fBf\fR ]  [ -\fBx\fR ]  [ -\fBh\fR ] 
.PP
\fBdhcpconfig\fR -\fBN\fR\ \fInetwork_address\fR  [ -\fBm\fR\ \fIsubnet_mask\fR ]  [ -\fBb\fR\  ]  [ -\fBt\fR\ \fIrouter_addresses\fR ]  [ -\fBy\fR\ \fINIS-domain\fR ]  [ -\fBa\fR\ \fINIS_server_addresses\fR ] 
.PP
\fBdhcpconfig\fR -\fBC\fR  -\fBr\fR\ \fIresource\fR  -\fBp\fR\ \fIpath\fR  [ -\fBf\fR ]  [ -\fBk\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhcpconfig\fR -\fBX\fR\ \fIfilename\fR  [ -\fBm\fR\ \fImacro_list\fR ]  [ -\fBo\fR\ \fIoption_list\fR ]  [ -\fBa\fR\ \fInetwork_addresses\fR ]  [ -\fBf\fR ]  [ -\fBx\fR ] 
.PP
\fBdhcpconfig\fR -\fBI\fR\ \fIfilename\fR  [ -\fBf\fR ] 
.SH "DESCRIPTION"
.PP
The \fBdhcpconfig\fR command is used to configure and manage the Dynamic Host Configuration Protocol (DHCP) service or BOOTP relay services\&. It is intended for use by experienced Solaris system administrators and is
designed for ease of use in scripts\&. The \fBdhcpmgr\fR utility is recommended for less experienced administrators or those preferring a graphical utility to configure and manage the DHCP service or BOOTP relay service\&.
.PP
The \fBdhcpconfig\fR command can be run by root, or by other users assigned to the DHCP Management profile\&. See \fBrbac\fR(5) and \fBuser_attr\fR(4)\&.
.PP
\fBdhcpconfig\fR requires one of the following function flags: -\fBD\fR, -\fBR\fR, -\fBU\fR, -\fBN\fR, -\fBC\fR, -\fBX\fR, or -\fBI\fR\&.
.PP
The dhcpconfig menu driven mode is supported in Solaris 8 and previous versions of Solaris\&.
.SS "Where dhcpconfig Obtains Configuration Information"
.PP
\fBdhcpconfig\fR scans various configuration files on your Solaris machine for information it can use to assign values to options contained in macros it adds to the \fBdhcptab\fR configuration table\&. The following table lists information \fBdhcpconfig\fR needs,
the source used, and how the information is used: 
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
.rm 80 81 82
.nr 34 \n(.lu
.eo
.am 81
.br
.di a+
.35
.ft \n(.f
.ll 2.059242in
.if \n(.l<\n(81 .ll \n(81u
.in 0
System domainname, \fBnsswitch\&.conf\fR, NIS
.br
.di
.nr a| \n(dn
.nr a- \n(dl
..
.ec \
.eo
.am 81
.br
.di b+
.35
.ft \n(.f
.ll 2.059242in
.if \n(.l<\n(81 .ll \n(81u
.in 0
Network interface, \fBnetmasks\fR table in nameservice
.br
.di
.nr b| \n(dn
.nr b- \n(dl
..
.ec \
.35
.nf
.ll \n(34u
.nr 80 0
.nr 38 \w\fIInformation\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wTimezone
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wDNS parameters
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wNIS parameters
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wSubnetmask
.if \n(80<\n(38 .nr 80 \n(38
.80
.rm 80
.nr 38 1.381517in
.if \n(80<\n(38 .nr 80 \n(38
.nr 81 0
.nr 38 \w\fISource\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wSystem date, timezone settings
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fBnsswitch\&.conf\fR, \fB/etc/resolv\&.conf\fR
.if \n(81<\n(38 .nr 81 \n(38
.81
.rm 81
.nr 38 \n(a-
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \n(b-
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 2.059242in
.if \n(81<\n(38 .nr 81 \n(38
.nr 82 0
.nr 38 \w\fIWhere Used\fR
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \w\fBLocale\fR macro
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wServer macro
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wNetwork macros
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wNetwork macros
.if \n(82<\n(38 .nr 82 \n(38
.82
.rm 82
.nr 38 2.059242in
.if \n(82<\n(38 .nr 82 \n(38
.35
.nf
.ll \n(34u
.nr 38 1n
.nr 79 0
.nr 40 \n(79+(1*\n(38)
.nr 80 +\n(40
.nr 41 \n(80+(3*\n(38)
.nr 81 +\n(41
.nr 42 \n(81+(3*\n(38)
.nr 82 +\n(42
.nr TW \n(82
.nr TW +1*\n(38
.if t .if \n(TW>\n(.li .tm Table at line 48 file Input is too wide - \n(TW units
.ne 5v+0p
.if n .ne 12v
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
.if \n(#a>=0 \h'(|\n(42u+|\n(81u)/2u'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
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
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fIInformation\fR\h'|\n(41u'\fISource\fR\h'|\n(42u'\fIWhere Used\fR
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Timezone\h'|\n(41u'System date, timezone settings\h'|\n(42u'\fBLocale\fR macro
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'DNS parameters\h'|\n(41u'\fBnsswitch\&.conf\fR, \fB/etc/resolv\&.conf\fR\h'|\n(42u'Server macro
.ne \n(a|u+\n(.Vu
.if (\n(a|+\n(#^-1v)>\n(#- .nr #- +(\n(a|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'NIS parameters\h'|\n(41u'\h'|\n(42u'Network macros
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(41u
.in +\n(37u
.a+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(b|u+\n(.Vu
.if (\n(b|+\n(#^-1v)>\n(#- .nr #- +(\n(b|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Subnetmask\h'|\n(41u'\h'|\n(42u'Network macros
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(41u
.in +\n(37u
.b+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.fc
.nr T. 1
.T# 1
.35
.nr #a 0
.rm a+
.rm b+
.TE
.if \n-(b.=0 .nr c. \n(.c-\n(d.-13
.sp
.PP
If you have not set these parameters on your server machine, you should do so before configuring the DHCP server with \fBdhcpconfig\fR\&. Note that if you specify options with the \fBdhcpconfig \fR-\fBD\fR command line, the values you supply override the values
obtained from the system files\&. 
.SH "OPTIONS"
.PP
The following options are supported:
.IP "-\fBC\fR" 6
Convert to using a new data store, recreating the DHCP data tables in a format appropriate to the new data store, and setting up the DHCP server to use the new data store\&. 
.sp
The following sub-options are required:
.sp
.RS
.IP "-\fBp\fR \fIpath_to_data\fR" 6
The paths for \fBSUNWfiles\fR and \fBSUNWbinfiles\fR must be absolute \fBUNIX\fR pathnames\&. The path for \fBSUNWnisplus\fR
must be a fully specified \fBNIS+\fR directory (including the tailing period\&.) See \fBdhcp_modules\fR(5)\&.
.IP "-\fBr\fR \fIdata_resource\fR" 6
New data store resource\&. One of the following must be specified: \fBSUNWfiles\fR, \fBSUNWbinfiles\fR, or \fBSUNWnisplus\fR\&. See \fBdhcp_modules\fR(5)\&.
.sp
.RE
.RS
The following sub-options are optional:
.RE
.RS
.IP "-\fBf\fR" 6
Do not prompt for confirmation\&. If -\fBf\fR is not used, a warning and confirmation prompt are issued before the conversion starts\&.
.IP "-\fBk\fR" 6
Keep the old DHCP data tables after successful conversion\&. If any problem occurs during conversion, tables will not be deleted even if -\fBk\fR sub-option is not specified\&.
.IP "-\fBu\fR \fIuninterpreted\fR" 6
Data which will be ignored by \fBdhcpconfig\fR, but passed on to the datastore for interpretation\&. This might be used for a database account name or other authentication or authorization
parameters required by a particular data store\&. The -\fBu\fR sub-option is not used with the \fBSUNWfiles\fR, \fBSUNWbinfiles\fR, and \fBSUNWnisplus\fR data stores\&. See \fBdhcp_modules\fR(5)\&.
.sp
.RE
.IP "-\fBD\fR" 6
Configure the \fBDHCP\fR service\&. 
.sp
The following sub-options are required:
.sp
.RS
.IP "-\fBr\fR \fIdata_resource\fR" 6
One of the following must be specified: \fBSUNWfiles\fR, \fBSUNWbinfiles\fR, or \fBSUNWnisplus\fR\&. Other data stores may be
available\&.See \fBdhcp_modules\fR(5)\&.
.IP "-\fBp\fR \fIpath\fR" 6
The paths for \fBSUNWfiles\fR and \fBSUNWbinfiles\fR must be absolute \fBUNIX\fR pathnames\&. The path for \fBSUNWnisplus\fR must be a fully specified \fBNIS+\fR directory (including the tailing period\&.) \&. See \fBdhcp_modules\fR(5)\&.
.sp
.RE
.RS
The following sub-options are optional:
.RE
.RS
.IP "-\fBa\fR \fIDNS_servers\fR" 6
IP addresses of DNS servers, separated with commas\&.
.IP "-\fBd\fR \fIDNS_domain\fR" 6
DNS domain name\&.
.IP "-\fBh\fR \fIhosts_resource\fR" 6
Resource in which to place hosts data\&. Usually, the name service in use on the server\&. Valid values are \fBnisplus\fR, \fBfiles\fR, or \fBdns\fR\&.
.IP "-\fBl\fR \fIseconds\fR" 6
Lease length used for addresses not having a specified lease length, in seconds\&.
.IP "-\fBn\fR" 6
Non-negotiable leases
.IP "-\fBy\fR \fIhosts_domain\fR" 6
DNS or NIS+ domain name to be used for hosts data\&. Valid only if \fBdns\fR or \fBnisplus\fR is specified for -\fBh\fR sub-option\&.
.IP "-\fBu\fR \fIuninterpreted\fR" 6
Data which will be ignored by \fBdhcpconfig\fR, but passed on to the datastore for interpretation\&. This might be used for a database account name or other authentication or authorization
parameters required by a particular data store\&. The -\fBu\fR sub-option is not used with the \fBSUNWfiles\fR, \fBSUNWbinfiles\fR, and \fBSUNWnisplus\fR data stores\&. See \fBdhcp_modules\fR(5)\&.
.sp
.RE
.IP "-\fBI\fR \fIfilename\fR" 6
Import data from \fIfilename\fR, containing data previously exported from a Solaris DHCP server\&. Note that after importing, you may have to edit macros to specify the correct domain names,
and edit network tables to change the owning server of addresses in imported networks\&. Use \fBdhtadm\fR and \fBpntadm\fR to do this\&.
.sp
.RS
The following sub-option is supported:
.RE
.RS
.IP "-\fBf\fR" 6
Replace any conflicting data with the data being imported\&.
.sp
.RE
.IP "-\fBN\fR \fInet_address\fR" 6
Configure an additional network for DHCP service\&. 
.sp
.RS
The following sub-options are supported:
.RE
.RS
.IP "-\fBa\fR \fINIS_server_addresses\fR" 6
List of IP addresses of NIS servers\&. 
.IP "-\fBb\fR" 6
Network is a point-to-point (PPP) network, therefore no broadcast address should be configured\&. If -\fB\fRb is not used, the network is assumed to be a LAN, and the broadcast address is determined using the network address and subnet
mask\&.
.IP "-\fBm\fR \fIxxx\&.xxx\&.xxx\&.xxx\fR" 6
Subnet mask for the network; if -\fBm\fR is not used, subnet mask is obtained from netmasks\&.
.IP "-\fBt\fR \fIrouter_addresses\fR" 6
List of router IP addresses; if not specified, router discovery flag will be set\&.
.IP "-\fBy\fR \fINIS_domain_name\fR" 6
If NIS is used on this network, specify the NIS domain name\&.
.sp
.RE
.IP "-\fBR\fR \fIserver_addresses\fR" 6
Configure the BOOTP relay service\&.  BOOTP or DHCP requests are forwarded to the list of servers specified\&. 
.IP "-\fBU\fR" 6
Unconfigure the DHCP service or BOOTP relay service\&. 
.sp
.RS
The following sub-options are supported:
.RE
.RS
.IP "-\fBf\fR" 6
Do not prompt for confirmation\&. If -\fBf\fR is not used, a warning and confirmation prompt is issued\&.
.IP "-\fBh\fR" 6
Delete hosts entries from name service\&.
.IP "-\fBx\fR" 6
Delete the \fBdhcptab\fR and network tables\&.
.sp
.RE
.IP "-\fBX\fR \fIfilename\fR" 6
Export data from the DHCP data tables, saving to \fIfilename\fR, to move the data to another Solaris DHCP server\&. 
.sp
.RS
The following sub-options are optional:
.RE
.RS
.IP "-\fBa\fR \fInetworks_to_export\fR" 6
List of networks whose addresses should be exported, or the keyword ALL to specify all networks\&. If -\fBa\fR is not specified, no networks are exported\&.
.IP "-\fBm\fR \fImacros_to_export\fR" 6
List of macros to export, or the keyword \fBALL\fR to specify all macros\&. If -\fBm\fR is not specified, no macros are exported\&.
.IP "-\fBo\fR \fIoptions_to_export\fR" 6
List of options to export, or the keyword \fBALL\fR to specify all options\&. If -\fBo\fR is not specified, no options are exported\&.
.IP "-\fBx\fR" 6
Delete the data from this server after it is exported\&. If -\fBx\fR is not specified you are in effect copying the data\&.
.sp
.RE
.SH "EXAMPLES"
.PP
\fBExample 1: Configuring DHCP Service with Binary Files Data Store\fR
.PP
The following command configures DHCP service, using the binary files data store, in the DNS domain \fBacme\&.eng\fR, with a lease time of 28800 seconds (8 hours), 
.PP
.nf
example# \fBdhcpconfig -D -r SUNWbinfiles -p /var/dhcp -l 28800 -d acme\&.eng
     -a 120\&.30\&.33\&.4 -h dns -y acme\&.eng\fR
.fi
.PP
\fBExample 2: Configuring BOOTP Relay Agent\fR
.PP
The following command configures the DHCP daemon as a BOOTP relay agent, which will forward BOOTP and DHCP requests to the servers having the IP addresses 120\&.30\&.33\&.7 and 120\&.30\&.42\&.132: 
.PP
.nf
example# \fBdhcpconfig -R 120\&.30\&.33\&.7,120\&.30\&.42\&.132\fR
.fi
.PP
\fBExample 3: Unconfiguring DHCP Service\fR
.PP
The following command unconfigures the DHCP service, with confirmation, and deletes the DHCP data tables and host table entries:
.PP
.nf
example# \fBdhcpconfig -U -x -h\fR
.fi
.PP
\fBExample 4: Configuring a Network for DHCP Service\fR
.PP
The following command configures an additional LAN network for DHCP service, specifying that clients should use router discovery and providing the NIS domain name and NIS server address: 
.PP
.nf
example# \fBdhcpconfig -N 120\&.30\&.171\&.0 -y east\&.acme\&.eng\&.com -a 120\&.30\&.33\&.4\fR
.fi
.PP
\fBExample 5: Converting to SUNWnisplus Data Store\fR
.PP
The following command converts a DHCP server from using a text or binary files data store to a NIS+ data store, deleting the old data store\&'s DHCP tables:
.PP
.nf
example# \fBdhcpconfig -C -r SUNWnisplus -p whatever\&.com\&.\fR
.fi
.PP
\fBExample 6: Exporting a Network, Macros, and Options from a DHCP Server\fR
.PP
The following command exports one network (\fB120\&.30\&.171\&.0\fR) and its addresses, the macro \fB120\&.30\&.171\&.0\fR, and the options \fBmotd\fR and \fBPSptr\fRfrom a DHCP server, saves the exported data in file \fB/export/var/120301710_data\fR,
and deletes the exported data from the server\&.
.PP
.nf
example# \fBdhcpconfig -X /var/dhcp/120301710_export
     -a 120\&.30\&.171\&.0 -m 120\&.30\&.171\&.0 -o motd,PSptr
\fR
.fi
.PP
\fBExample 7: Importing Data on a DHCP Server\fR
.PP
The following command imports DHCP data from a file, \fB/net/golduck/export/var/120301710_data\fR, containing data previously exported from a Solaris DHCP server, and overwrites any conflicting data on the importing server:
.PP
.nf
example# \fBdhcpconfig -I /net/golduck/export/var/120301710_data -f\fR
.fi
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
.if t .if \n(TW>\n(.li .tm Table at line 254 file Input is too wide - \n(TW units
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
\fBdhcpmgr\fR(1M), \fBdhtadm\fR(1M), \fBin\&.dhcpd\fR(1M), \fBpntadm\fR(1M), \fBdhcp_network\fR(4), \fBdhcptab\fR(4), \fBdhcpsvc\&.conf\fR(4), \fBnsswitch\&.conf\fR(4), \fBresolv\&.conf\fR(4), \fBuser_attr\fR(4), \fBattributes\fR(5), \fBdhcp\fR(5), \fBdhcp_modules\fR(5), \fBrbac\fR(5)
.PP
\fISystem Administration Guide, Volume 3\fR
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 13:53
