'\" te
.TH dhcp_inittab 4 "21 Mar 2001" "SunOS 5.8" "File Formats"
.SH "NAME"
dhcp_inittab \- information repository for DHCP options
.SH "DESCRIPTION"
.PP
The \fB/etc/dhcp/inittab\fR file contains information about the Dynamic Host Configuration Protocol (\fBDHCP\fR) options, which are network configuration parameters passed from \fBDHCP\fR servers to \fBDHCP\fR clients when a client machine uses \fBDHCP\fR\&. Since many \fBDHCP\fR-related commands must parse and understand these \fBDHCP\fR options, this file serves as a central location where information about these options may be obtained\&.
.PP
The \fBDHCP\fR \fBinittab\fR file provides three general pieces of information:
.IP "   \(bu" 6
A mnemonic alias, or symbol name, for each option number\&. For instance, option 12 is aliased to the name \fBHostname\fR\&. This is useful for \fBDHCP\fR-related programs that require human interaction, such as \fBdhcpinfo\fR(1)\&.
.IP "   \(bu" 6
Information about the syntax for each option\&. This includes information such as the type of the value, for example, whether it is a 16-bit integer or an \fBIP\fR address\&.
.IP "   \(bu" 6
The policy for what options are visible to which \fBDHCP\fR-related programs\&.
.PP
 The \fBdhcp_inittab\fR file can only be changed upon system upgrade\&. Only additions of \fBSITE\fR options (or changes to same) will be preserved during upgrade\&.
.PP
The \fBVENDOR\fR options defined here are intended for use by the Solaris \fBDHCP\fR client and \fBDHCP\fR management tools\&. The \fBSUNW\fR vendor space is owned by Sun, and changes are likely during upgrade\&. If you need to configure the Solaris \fBDHCP\fR server to support the vendor options of a different client, see \fBdhctab\fR(4) for details\&. 
.PP
Each \fBDHCP\fR option belongs to a certain category, which roughly defines the scope of the option; for instance, an option may only be understood by certain hosts within a given site, or it may be globally understood by all \fBDHCP\fR clients
and servers\&. The following categories are defined; the category names are not case-sensitive:
.RS
.IP "\fBSTANDARD\fR" 6
All client and server \fBDHCP\fR implementations agree on the semantics\&. These are administered by the Internet Assigned Numbers Authority (\fBIANA\fR)\&. These options are numbered from \fB1\fR
to \fB127\fR\&.
.IP "\fBSITE\fR" 6
Within a specific site, all client and server implementations agree on the semantics\&. However, at another site the type and meaning of the option may be quite different\&. These options are numbered from 128 to 254\&.
.IP "\fBVENDOR\fR" 6
Each vendor may define 254 options unique to that vendor\&. The vendor is identified within a \fBDHCP\fR packet by the "Vendor Class" option, number 60\&. An option with a specific numeric identifier belonging to one vendor will,
in general, have a type and semantics different from that of a different vendor\&. Vendor options are "super-encapsulated" into the vendor field number 43, as defined in \fIRFC 2132\fR\&. The \fBdhcp_inittab\fR file only contains  Sun vendor options\&. Define non-Sun vendor options
in the \fBdhcptab\fR file\&.
.IP "\fBFIELD\fR" 6
This category allows the fixed fields within a \fBDHCP\fR packet to be aliased to a mnemonic name for use with \fBdhcpinfo\fR(1)\&.
.IP "\fBINTERNAL\fR" 6
This category is internal to the Solaris \fBDHCP\fR implementation and will not be further defined\&.
.sp
.RE
.SS "DHCP inittab Format"
.PP
Data entries are written one per line and have seven fields; each entry provides information for one option\&. Each field is separated by a comma, except for the first and second, which are separated by whitespace (as defined in \fBisspace\fR(3C))\&. An entry cannot be continued onto another line\&. Blank lines and those whose first non-whitespace character is \&'#\&' are ignored\&.
.PP
The fields, in order, are: 
.IP "   \(bu" 6
Mnemonic Identifier
.sp
.PP
The Mnemonic Identifier is a user-friendly alias for the option number; it is not case sensitive\&. This field must be per-category unique and should be unique across all categories\&. The option names in the \fBSTANDARD\fR, \fBSITE\fR, and \fBVENDOR\fR spaces
should not overlap, or the behavior will be undefined\&. See \fBMnemonic Identifiers for Options\fR section of this man page for descriptions of the option names\&.
.IP "   \(bu" 6
Category (scope)
.sp
.PP
The Category field is one of \fBSTANDARD\fR, \fBSITE\fR, \fBVENDOR\fR, \fBFIELD\fR, or \fBINTERNAL\fR and identifies the scope in which the option falls\&.
.IP "   \(bu" 6
Option Number
.sp
.PP
The Option Number is the number of this option when it is in a \fBDHCP\fR packet\&. This field should be per-category unique and the \fBSTANDARD\fR and \fBSITE\fR fields should not have overlapping code fields or the behavior is undefined\&.
.IP "   \(bu" 6
Data Type 
.sp
Data Type is one of the following values, which are not case sensitive:
.RS
.IP "\fBAscii\fR" 6
A printable character string
.IP "\fBOctet\fR" 6
An array of bytes
.IP "\fBUnumber8\fR" 6
An 8-bit unsigned integer
.IP "\fBSnumber8\fR" 6
An 8-bit signed integer
.IP "\fBUnumber16\fR" 6
A 16-bit unsigned integer
.IP "\fBSnumber16\fR" 6
A 16-bit signed integer
.IP "\fBUnumber32\fR" 6
A 32-bit unsigned integer
.IP "\fBSnumber32\fR" 6
A 32-bit signed integer
.IP "\fBUnumber64\fR" 6
A 64-bit unsigned integer
.IP "\fBSnumber64\fR" 6
A 64-bit signed integer
.IP "\fBIp\fR" 6
An \fBIP\fR address
.sp
.RE
.sp
.PP
The data type field describes an indivisible unit of the option payload, using one of the values listed above\&.
.IP "   \(bu" 6
Granularity
.sp
.PP
The Granularity field describes how many "indivisible units" in the option payload make up a whole value or item for this option\&. 
.IP "   \(bu" 6
Maximum Number Of Items
.IP "   \(bu" 6
Visibility
.sp
.PP
The Visibility field specifies which \fBDHCP\fR-related programs make use of this information, and should always be defined as "\fBsdmi\fR" for newly added options\&.
.SS "Mnemonic Identifiers for Options"
.PP
The following table maps the mnemonic identifiers used in Solaris DHCP to RFC-2132 options:
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
.am 82
.br
.di a+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Subnet Mask, dotted Internet address (IP)\&.
.br
.di
.nr a| \n(dn
.nr a- \n(dl
..
.ec \
.eo
.am 82
.br
.di b+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Coordinated Universal time offset (seconds)\&.
.br
.di
.nr b| \n(dn
.nr b- \n(dl
..
.ec \
.eo
.am 82
.br
.di c+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
List of MIT-LCS UDP log servers, IP\&.
.br
.di
.nr c| \n(dn
.nr c- \n(dl
..
.ec \
.eo
.am 82
.br
.di d+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
List of RFC-1179 line printer servers, IP\&.
.br
.di
.nr d| \n(dn
.nr d- \n(dl
..
.ec \
.eo
.am 82
.br
.di e+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
List of RFC-887 resource location servers, IP\&.
.br
.di
.nr e| \n(dn
.nr e- \n(dl
..
.ec \
.eo
.am 82
.br
.di f+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Client\&'s hostname, value from hosts database\&.
.br
.di
.nr f| \n(dn
.nr f- \n(dl
..
.ec \
.eo
.am 82
.br
.di g+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Number of 512 octet blocks in boot image, NUMBER\&.
.br
.di
.nr g| \n(dn
.nr g- \n(dl
..
.ec \
.eo
.am 82
.br
.di h+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Path where core image should be dumped, ASCII\&. 
.br
.di
.nr h| \n(dn
.nr h- \n(dl
..
.ec \
.eo
.am 82
.br
.di i+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
IP Forwarding Enable/Disable, NUMBER\&.
.br
.di
.nr i| \n(dn
.nr i- \n(dl
..
.ec \
.eo
.am 82
.br
.di j+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Maximum datagram Reassembly Size, NUMBER\&.
.br
.di
.nr j| \n(dn
.nr j- \n(dl
..
.ec \
.eo
.am 82
.br
.di k+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Default IP Time to Live, (1=<x<=255), NUMBER\&.
.br
.di
.nr k| \n(dn
.nr k- \n(dl
..
.ec \
.eo
.am 82
.br
.di l+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
RFC-1191 Path MTU Aging Timeout, NUMBER\&.
.br
.di
.nr l| \n(dn
.nr l- \n(dl
..
.ec \
.eo
.am 82
.br
.di m+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
RFC-1191 Path MTU Plateau Table, NUMBER\&.
.br
.di
.nr m| \n(dn
.nr m- \n(dl
..
.ec \
.eo
.am 82
.br
.di n+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Static Route, Double IP (network router)\&.
.br
.di
.nr n| \n(dn
.nr n- \n(dl
..
.ec \
.eo
.am 82
.br
.di o+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
List of NetBIOS Distribution servers, IP\&.
.br
.di
.nr o| \n(dn
.nr o- \n(dl
..
.ec \
.eo
.am 82
.br
.di p+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
NetBIOS Node type (1=B-node, 2=P, 4=M, 8=H)
.br
.di
.nr p| \n(dn
.nr p- \n(dl
..
.ec \
.eo
.am 82
.br
.di q+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
List of X Window Display managers, IP\&.
.br
.di
.nr q| \n(dn
.nr q- \n(dl
..
.ec \
.eo
.am 82
.br
.di r+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Lease Time Policy, (-1 = PERM), NUMBER\&.
.br
.di
.nr r| \n(dn
.nr r- \n(dl
..
.ec \
.eo
.am 82
.br
.di s+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Message to be displayed on client, ASCII\&. 
.br
.di
.nr s| \n(dn
.nr s- \n(dl
..
.ec \
.eo
.am 82
.br
.di t+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
NetWare/IP Options, OCTET (unknown type)\&.
.br
.di
.nr t| \n(dn
.nr t- \n(dl
..
.ec \
.eo
.am 82
.br
.di u+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Simple Mail Transport Protocol Server, IP\&.
.br
.di
.nr u| \n(dn
.nr u- \n(dl
..
.ec \
.eo
.am 82
.br
.di v+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Post Office Protocol (POP3) Server, IP\&.
.br
.di
.nr v| \n(dn
.nr v- \n(dl
..
.ec \
.eo
.am 82
.br
.di w+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Network News Transport Proto\&. (NNTP) Server, IP\&. 
.br
.di
.nr w| \n(dn
.nr w- \n(dl
..
.ec \
.eo
.am 82
.br
.di x+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
StreetTalk Directory Assist\&. Server, IP\&.
.br
.di
.nr x| \n(dn
.nr x- \n(dl
..
.ec \
.eo
.am 82
.br
.di y+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Client Network Device Interface, OCTET\&.
.br
.di
.nr y| \n(dn
.nr y- \n(dl
..
.ec \
.eo
.am 82
.br
.di z+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
UUID/GUID-based client indentifier, OCTET\&.
.br
.di
.nr z| \n(dn
.nr z- \n(dl
..
.ec \
.eo
.am 82
.br
.di A+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Boot path prefix to apply to client\&'s requested boot file, ASCII\&.
.br
.di
.nr A| \n(dn
.nr A- \n(dl
..
.ec \
.eo
.am 82
.br
.di B+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Echo Vendor Class Identifier Flag, (Present=\fBTRUE\fR)
.br
.di
.nr B| \n(dn
.nr B- \n(dl
..
.ec \
.eo
.am 82
.br
.di C+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Lease is Negotiable Flag, (Present=\fBTRUE\fR)
.br
.di
.nr C| \n(dn
.nr C- \n(dl
..
.ec \
.eo
.am 82
.br
.di D+
.35
.ft \n(.f
.ll 3.011604in
.if \n(.l<\n(82 .ll \n(82u
.in 0
Include listed macro values in this macro\&.
.br
.di
.nr D| \n(dn
.nr D- \n(dl
..
.ec \
.35
.nf
.ll \n(34u
.nr 80 0
.nr 38 \w\fISymbol\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBSubnet\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBUTCoffst\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBRouter\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBTimeserv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBIEN116ns\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBDNSserv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBLogserv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBCookie\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBLprserv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBImpress\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBResource\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBHostname\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBBootsize\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBDumpfile\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBDNSdmain\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBSwapserv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBRootpath\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBExtendP\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBIpFwdF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNLrouteF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBPFilter\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBMaxIpSiz\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBIpTTL\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBPathTO\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBPathTbl\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBMTU\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBSameMtuF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBBroadcst\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBMaskDscF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBMaskSupF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBRDiscvyF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBRSolictS\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBStaticRt\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBTrailerF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBArpTimeO\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBEthEncap\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBTcpTTL\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBTcpKaInt\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBTcpKaGbF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNISdmain\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNISservs\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNTPservs\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNetBNms\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNetBDsts\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNetBNdT\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNetBScop\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBXFontSrv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBXDispMgr\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBLeaseTim\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBMessage\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBT1Time\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBT2Time\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNW_dmain\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNWIPOpts\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNIS+dom\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNIS+serv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBTFTPsrvN\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBOptBootF\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBMblIPAgt\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBSMTPserv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBPOP3serv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBNNTPserv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBWWWservs\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBFingersv\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBIRCservs\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBSTservs\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBSTDAservs\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBUserClas\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBSLP_DA\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBSLP_SS\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBAgentOpt\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBFQDN\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBPXEarch\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBPXEnii\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBPXEcid\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBBootFile\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBBootPath\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBBootSrvA\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBBootSrvN\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBEchoVC\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBLeaseNeg\fR
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBInclude\fR
.if \n(80<\n(38 .nr 80 \n(38
.80
.rm 80
.nr 38 1.395174in
.if \n(80<\n(38 .nr 80 \n(38
.nr 81 0
.nr 38 \w\fICode\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB1\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB2\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB3\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB4\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB5\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB6\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB7\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB8\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB9\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB10\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB11\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB12\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB13\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB14\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB15\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB16\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB17\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB18\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB19\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB20\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB21\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB22\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB23\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB24\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB25\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB26\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB27\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB28\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB29\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB30\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB31\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB32\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB33\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB34\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB35\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB36\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB37\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB38\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB39\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB40\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB41\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB42\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB44\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB45\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB46\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB47\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB48\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB49\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB51\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB56\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB58\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB59\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB62\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB63\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB64\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB65\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB66\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB67\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB68\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB69\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB70\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB71\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB72\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB73\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB74\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB75\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB76\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB77\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB78\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB79\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB82\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB89\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB93\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB94\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fB97\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fBN/A\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fBN/A\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fBN/A\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fBN/A\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fBN/A\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fBN/A\fR
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \w\fBN/A\fR
.if \n(81<\n(38 .nr 81 \n(38
.81
.rm 81
.nr 38 1.093222in
.if \n(81<\n(38 .nr 81 \n(38
.nr 82 0
.nr 38 \w\fIDescription\fR
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of Routers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of RFC-868 servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of IEN 116 name servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of DNS name servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of RFC-865 cookie servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of Imagen Impress servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wDNS domain name, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wClient\&'s swap server, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wClient\&'s Root path, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wExtensions path, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wNon-local Source Routing, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wPolicy Filter, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wInterface MTU, x>=68, NUMBER\&. 
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wAll Subnets are Local, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wBroadcast Address, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wPerform Mask Discovery, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wMask Supplier, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wPerform Router Discovery, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wRouter Solicitation Address, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wTrailer Encapsulation, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wARP Cache Time out, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wEthernet Encapsulation, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wTCP Default Time to Live, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wTCP Keepalive Interval, NUMBER\&. 
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wTCP Keepalive Garbage, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wNIS Domain name, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of NIS servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of NTP servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of NetBIOS Name servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wNetBIOS scope, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wList of X Window Font servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wRenewal (T1) time, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wRebinding (T2) time, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wNetWare/IP Domain Name, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wNIS+ Domain name, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wNIS+ servers, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wTFTP server hostname, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wOptional Bootfile path, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wMobile IP Home Agent, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wDefault WorldWideWeb Server, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wDefault Finger Server, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wInternet Relay Chat Server, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wStreetTalk Server, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wUser class information, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wDirectory agent, OCTET\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wService scope, OCTET\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wAgent circuit ID, OCTET\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wFully Qualified Domain Name, OCTET\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wClient system architecture, NUMBER\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wFile to Boot, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wBoot Server, IP\&.
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \wBoot Server Hostname, ASCII\&.
.if \n(82<\n(38 .nr 82 \n(38
.82
.rm 82
.nr 38 \n(a-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(b-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(c-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(d-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(e-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(f-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(g-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(h-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(i-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(j-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(k-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(l-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(m-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(n-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(o-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(p-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(q-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(r-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(s-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(t-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(u-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(v-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(w-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(x-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(y-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(z-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(A-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(B-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(C-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(D-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 3.011604in
.if \n(82<\n(38 .nr 82 \n(38
.35
.nf
.ll \n(34u
.nr 38 1n
.nr 79 0
.nr 40 \n(79+(0*\n(38)
.nr 80 +\n(40
.nr 41 \n(80+(3*\n(38)
.nr 81 +\n(41
.nr 42 \n(81+(3*\n(38)
.nr 82 +\n(42
.nr TW \n(82
.if t .if \n(TW>\n(.li .tm Table at line 254 file Input is too wide - \n(TW units
.fc  
.nr #T 0-1
.nr #a 0-1
.eo
.de T#
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.mk ##
.nr ## -1v
.ls 1
.ls
..
.ec
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fISymbol\fR\h'|\n(41u'\fICode\fR\h'|\n(42u'\fIDescription\fR
.ne \n(a|u+\n(.Vu
.if (\n(a|+\n(#^-1v)>\n(#- .nr #- +(\n(a|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBSubnet\fR\h'|\n(41u'\fB1\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
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
\&\h'|\n(40u'\fBUTCoffst\fR\h'|\n(41u'\fB2\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.b+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBRouter\fR\h'|\n(41u'\fB3\fR\h'|\n(42u'List of Routers, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBTimeserv\fR\h'|\n(41u'\fB4\fR\h'|\n(42u'List of RFC-868 servers, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBIEN116ns\fR\h'|\n(41u'\fB5\fR\h'|\n(42u'List of IEN 116 name servers, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBDNSserv\fR\h'|\n(41u'\fB6\fR\h'|\n(42u'List of DNS name servers, IP\&.
.ne \n(c|u+\n(.Vu
.if (\n(c|+\n(#^-1v)>\n(#- .nr #- +(\n(c|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBLogserv\fR\h'|\n(41u'\fB7\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.c+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBCookie\fR\h'|\n(41u'\fB8\fR\h'|\n(42u'List of RFC-865 cookie servers, IP\&.
.ne \n(d|u+\n(.Vu
.if (\n(d|+\n(#^-1v)>\n(#- .nr #- +(\n(d|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBLprserv\fR\h'|\n(41u'\fB9\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.d+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBImpress\fR\h'|\n(41u'\fB10\fR\h'|\n(42u'List of Imagen Impress servers, IP\&.
.ne \n(e|u+\n(.Vu
.if (\n(e|+\n(#^-1v)>\n(#- .nr #- +(\n(e|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBResource\fR\h'|\n(41u'\fB11\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.e+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(f|u+\n(.Vu
.if (\n(f|+\n(#^-1v)>\n(#- .nr #- +(\n(f|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBHostname\fR\h'|\n(41u'\fB12\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.f+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(g|u+\n(.Vu
.if (\n(g|+\n(#^-1v)>\n(#- .nr #- +(\n(g|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBBootsize\fR\h'|\n(41u'\fB13\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.g+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(h|u+\n(.Vu
.if (\n(h|+\n(#^-1v)>\n(#- .nr #- +(\n(h|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBDumpfile\fR\h'|\n(41u'\fB14\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.h+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBDNSdmain\fR\h'|\n(41u'\fB15\fR\h'|\n(42u'DNS domain name, ASCII\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBSwapserv\fR\h'|\n(41u'\fB16\fR\h'|\n(42u'Client\&'s swap server, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBRootpath\fR\h'|\n(41u'\fB17\fR\h'|\n(42u'Client\&'s Root path, ASCII\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBExtendP\fR\h'|\n(41u'\fB18\fR\h'|\n(42u'Extensions path, ASCII\&.
.ne \n(i|u+\n(.Vu
.if (\n(i|+\n(#^-1v)>\n(#- .nr #- +(\n(i|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBIpFwdF\fR\h'|\n(41u'\fB19\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.i+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNLrouteF\fR\h'|\n(41u'\fB20\fR\h'|\n(42u'Non-local Source Routing, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBPFilter\fR\h'|\n(41u'\fB21\fR\h'|\n(42u'Policy Filter, IP\&.
.ne \n(j|u+\n(.Vu
.if (\n(j|+\n(#^-1v)>\n(#- .nr #- +(\n(j|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBMaxIpSiz\fR\h'|\n(41u'\fB22\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.j+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(k|u+\n(.Vu
.if (\n(k|+\n(#^-1v)>\n(#- .nr #- +(\n(k|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBIpTTL\fR\h'|\n(41u'\fB23\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.k+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(l|u+\n(.Vu
.if (\n(l|+\n(#^-1v)>\n(#- .nr #- +(\n(l|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBPathTO\fR\h'|\n(41u'\fB24\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.l+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(m|u+\n(.Vu
.if (\n(m|+\n(#^-1v)>\n(#- .nr #- +(\n(m|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBPathTbl\fR\h'|\n(41u'\fB25\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.m+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBMTU\fR\h'|\n(41u'\fB26\fR\h'|\n(42u'Interface MTU, x>=68, NUMBER\&. 
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBSameMtuF\fR\h'|\n(41u'\fB27\fR\h'|\n(42u'All Subnets are Local, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBBroadcst\fR\h'|\n(41u'\fB28\fR\h'|\n(42u'Broadcast Address, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBMaskDscF\fR\h'|\n(41u'\fB29\fR\h'|\n(42u'Perform Mask Discovery, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBMaskSupF\fR\h'|\n(41u'\fB30\fR\h'|\n(42u'Mask Supplier, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBRDiscvyF\fR\h'|\n(41u'\fB31\fR\h'|\n(42u'Perform Router Discovery, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBRSolictS\fR\h'|\n(41u'\fB32\fR\h'|\n(42u'Router Solicitation Address, IP\&.
.ne \n(n|u+\n(.Vu
.if (\n(n|+\n(#^-1v)>\n(#- .nr #- +(\n(n|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBStaticRt\fR\h'|\n(41u'\fB33\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.n+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBTrailerF\fR\h'|\n(41u'\fB34\fR\h'|\n(42u'Trailer Encapsulation, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBArpTimeO\fR\h'|\n(41u'\fB35\fR\h'|\n(42u'ARP Cache Time out, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBEthEncap\fR\h'|\n(41u'\fB36\fR\h'|\n(42u'Ethernet Encapsulation, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBTcpTTL\fR\h'|\n(41u'\fB37\fR\h'|\n(42u'TCP Default Time to Live, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBTcpKaInt\fR\h'|\n(41u'\fB38\fR\h'|\n(42u'TCP Keepalive Interval, NUMBER\&. 
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBTcpKaGbF\fR\h'|\n(41u'\fB39\fR\h'|\n(42u'TCP Keepalive Garbage, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNISdmain\fR\h'|\n(41u'\fB40\fR\h'|\n(42u'NIS Domain name, ASCII\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNISservs\fR\h'|\n(41u'\fB41\fR\h'|\n(42u'List of NIS servers, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNTPservs\fR\h'|\n(41u'\fB42\fR\h'|\n(42u'List of NTP servers, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNetBNms\fR\h'|\n(41u'\fB44\fR\h'|\n(42u'List of NetBIOS Name servers, IP\&.
.ne \n(o|u+\n(.Vu
.if (\n(o|+\n(#^-1v)>\n(#- .nr #- +(\n(o|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNetBDsts\fR\h'|\n(41u'\fB45\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.o+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(p|u+\n(.Vu
.if (\n(p|+\n(#^-1v)>\n(#- .nr #- +(\n(p|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNetBNdT\fR\h'|\n(41u'\fB46\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.p+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNetBScop\fR\h'|\n(41u'\fB47\fR\h'|\n(42u'NetBIOS scope, ASCII\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBXFontSrv\fR\h'|\n(41u'\fB48\fR\h'|\n(42u'List of X Window Font servers, IP\&.
.ne \n(q|u+\n(.Vu
.if (\n(q|+\n(#^-1v)>\n(#- .nr #- +(\n(q|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBXDispMgr\fR\h'|\n(41u'\fB49\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.q+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(r|u+\n(.Vu
.if (\n(r|+\n(#^-1v)>\n(#- .nr #- +(\n(r|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBLeaseTim\fR\h'|\n(41u'\fB51\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.r+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(s|u+\n(.Vu
.if (\n(s|+\n(#^-1v)>\n(#- .nr #- +(\n(s|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBMessage\fR\h'|\n(41u'\fB56\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.s+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBT1Time\fR\h'|\n(41u'\fB58\fR\h'|\n(42u'Renewal (T1) time, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBT2Time\fR\h'|\n(41u'\fB59\fR\h'|\n(42u'Rebinding (T2) time, NUMBER\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNW_dmain\fR\h'|\n(41u'\fB62\fR\h'|\n(42u'NetWare/IP Domain Name, ASCII\&.
.ne \n(t|u+\n(.Vu
.if (\n(t|+\n(#^-1v)>\n(#- .nr #- +(\n(t|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNWIPOpts\fR\h'|\n(41u'\fB63\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.t+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNIS+dom\fR\h'|\n(41u'\fB64\fR\h'|\n(42u'NIS+ Domain name, ASCII\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNIS+serv\fR\h'|\n(41u'\fB65\fR\h'|\n(42u'NIS+ servers, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBTFTPsrvN\fR\h'|\n(41u'\fB66\fR\h'|\n(42u'TFTP server hostname, ASCII\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBOptBootF\fR\h'|\n(41u'\fB67\fR\h'|\n(42u'Optional Bootfile path, ASCII\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBMblIPAgt\fR\h'|\n(41u'\fB68\fR\h'|\n(42u'Mobile IP Home Agent, IP\&.
.ne \n(u|u+\n(.Vu
.if (\n(u|+\n(#^-1v)>\n(#- .nr #- +(\n(u|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBSMTPserv\fR\h'|\n(41u'\fB69\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.u+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(v|u+\n(.Vu
.if (\n(v|+\n(#^-1v)>\n(#- .nr #- +(\n(v|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBPOP3serv\fR\h'|\n(41u'\fB70\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.v+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(w|u+\n(.Vu
.if (\n(w|+\n(#^-1v)>\n(#- .nr #- +(\n(w|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBNNTPserv\fR\h'|\n(41u'\fB71\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.w+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBWWWservs\fR\h'|\n(41u'\fB72\fR\h'|\n(42u'Default WorldWideWeb Server, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBFingersv\fR\h'|\n(41u'\fB73\fR\h'|\n(42u'Default Finger Server, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBIRCservs\fR\h'|\n(41u'\fB74\fR\h'|\n(42u'Internet Relay Chat Server, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBSTservs\fR\h'|\n(41u'\fB75\fR\h'|\n(42u'StreetTalk Server, IP\&.
.ne \n(x|u+\n(.Vu
.if (\n(x|+\n(#^-1v)>\n(#- .nr #- +(\n(x|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBSTDAservs\fR\h'|\n(41u'\fB76\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.x+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBUserClas\fR\h'|\n(41u'\fB77\fR\h'|\n(42u'User class information, ASCII\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBSLP_DA\fR\h'|\n(41u'\fB78\fR\h'|\n(42u'Directory agent, OCTET\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBSLP_SS\fR\h'|\n(41u'\fB79\fR\h'|\n(42u'Service scope, OCTET\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBAgentOpt\fR\h'|\n(41u'\fB82\fR\h'|\n(42u'Agent circuit ID, OCTET\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBFQDN\fR\h'|\n(41u'\fB89\fR\h'|\n(42u'Fully Qualified Domain Name, OCTET\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBPXEarch\fR\h'|\n(41u'\fB93\fR\h'|\n(42u'Client system architecture, NUMBER\&.
.ne \n(y|u+\n(.Vu
.if (\n(y|+\n(#^-1v)>\n(#- .nr #- +(\n(y|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBPXEnii\fR\h'|\n(41u'\fB94\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.y+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(z|u+\n(.Vu
.if (\n(z|+\n(#^-1v)>\n(#- .nr #- +(\n(z|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBPXEcid\fR\h'|\n(41u'\fB97\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.z+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBBootFile\fR\h'|\n(41u'\fBN/A\fR\h'|\n(42u'File to Boot, ASCII\&.
.ne \n(A|u+\n(.Vu
.if (\n(A|+\n(#^-1v)>\n(#- .nr #- +(\n(A|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBBootPath\fR\h'|\n(41u'\fBN/A\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.A+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBBootSrvA\fR\h'|\n(41u'\fBN/A\fR\h'|\n(42u'Boot Server, IP\&.
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBBootSrvN\fR\h'|\n(41u'\fBN/A\fR\h'|\n(42u'Boot Server Hostname, ASCII\&.
.ne \n(B|u+\n(.Vu
.if (\n(B|+\n(#^-1v)>\n(#- .nr #- +(\n(B|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBEchoVC\fR\h'|\n(41u'\fBN/A\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.B+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(C|u+\n(.Vu
.if (\n(C|+\n(#^-1v)>\n(#- .nr #- +(\n(C|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBLeaseNeg\fR\h'|\n(41u'\fBN/A\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.C+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(D|u+\n(.Vu
.if (\n(D|+\n(#^-1v)>\n(#- .nr #- +(\n(D|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBInclude\fR\h'|\n(41u'\fBN/A\fR\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.D+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.fc
.nr T. 1
.T# 1
.35
.rm a+
.rm b+
.rm c+
.rm d+
.rm e+
.rm f+
.rm g+
.rm h+
.rm i+
.rm j+
.rm k+
.rm l+
.rm m+
.rm n+
.rm o+
.rm p+
.rm q+
.rm r+
.rm s+
.rm t+
.rm u+
.rm v+
.rm w+
.rm x+
.rm y+
.rm z+
.rm A+
.rm B+
.rm C+
.rm D+
.TE
.if \n-(b.=0 .nr c. \n(.c-\n(d.-146
.sp
.SH "EXAMPLES"
.PP
\fBExample 1: Altering the DHCP \fBinittab\fR File\fR
.PP
In general, the \fBDHCP\fR \fBinittab\fR file should only be altered to add \fBSITE\fR options\&. If other options are added, they will not be automatically carried forward when the system is upgraded\&. For instance:
.PP
.PP
.nf
\f(CWipPairs    SITE, 132, IP, 2, 0, sdmi\fR
.fi
.PP
.PP
describes an option named \fBipPairs\fR, that is in the \fBSITE\fR  category\&. That is, it is defined by each individual site, and is  option code 132, which is of type \fBIP\fR Address, consisting  of a potentially infinite number of pairs of \fBIP\fR
addresses\&.
.SH "FILES"
.IP "" 10
\fB/etc/dhcp/inittab\fR
.SH "ATTRIBUTES"
.PP
See \fBattributes\fR(5)  for descriptions of the following attributes:
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
.nr 38 \wSUNWcsr
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
.if t .if \n(TW>\n(.li .tm Table at line 284 file Input is too wide - \n(TW units
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
\&\h'|\n(40u'Availability\h'|\n(41u'SUNWcsr
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
\fBdhcpinfo\fR(1),\fBdhcpagent\fR(1M), \fBisspace\fR(3C), \fBdhctab\fR(4), \fBattributes\fR(5), \fBdhcp\fR(5), \fBdhcp_modules\fR(5)
.PP
\fISystem Administration Guide, Volume 3\fR
.PP
Alexander, S\&., and R\&. Droms, \fIDHCP Options and BOOTP Vendor Extensions\fR, RFC 2132, Network Working Group, March 1997\&.
.PP
Droms, R\&., \fIDynamic Host Configuration Protocol\fR, RFC 2131, Network Working Group, March 1997\&.
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 14:29
