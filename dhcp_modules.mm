'\" te
.TH dhcp_modules 5 "13 Mar 2001" "SunOS 5.8" "Standards, Environments, and Macros"
.SH "NAME"
dhcp_modules \- data storage modules for the DHCP service
.SH "DESCRIPTION"
.PP
This man page describes the characteristics of data storage modules (public modules) for use by the Solaris Dynamic Host Configuration Protocol (\fBDHCP\fR) service\&.
.PP
Public modules are the part of the \fBDHCP\fR service architecture that encapsulate the details of storing \fBDHCP\fR service data in a data storage service\&. Examples of data storage services are \fBNIS+\fR, Oracle, and \fBufs\fR file systems\&.
.PP
Public modules are dynamic objects which can be shipped separately from the Solaris \fBDHCP\fR service\&. Once installed, a public module is visible to the \fBDHCP\fR service, and can be selected for use by the service through the \fBDHCP\fR service management
interfaces (\fBdhcpmgr\fR(1M), \fBdhcpconfig\fR(1M), \fBdhtadm\fR(1M), and \fBpntadm\fR(1M))\&.
.PP
Public modules may be provided by Sun Microsystems, Inc or by third parties\&.
.PP
The Solaris \fBDHCP\fR service management architecture provides a mechanism for plugging in public module-specific administration functionality into the \fBdhcpmgr\fR(1M) and \fBdhcpconfig\fR(1M) utilities\&. This functionality is in the form of a Java Bean, which is provided by the public module vendor\&. This Java Bean collects public module-specific configuration from the user (you) and provides it to the Solaris \fBDHCP\fR service\&.
.PP
The Solaris \fBDHCP\fR service bundles three modules with the service, which are described below\&. There are three \fBdhcpsvc\&.conf\fR(4) \fBDHCP\fR service configuration parameters pertaining to public
modules: \fBRESOURCE\fR, \fBPATH\fR, and \fBRESOURCE_CONFIG\fR\&. See \fBdhcpsvc\&.conf\fR(4) for more information about these
parameters\&.
.SS "SUNWfiles"
.PP
This module stores it\&'s data in \fBASCII\fR files\&. Although the format is \fBASCII\fR, hand-editing is discouraged\&. It is useful for \fBDHCP\fR service environments that support several hundred to a couple thousand of clients and lease times are a few hours
or more\&.
.PP
This module\&'s data may be shared between \fBDHCP\fR servers through the use of \fBNFS\fR\&.
.SS "SUNWbinfiles"
.PP
This module stores it\&'s data in binary files\&. It is useful for \fBDHCP\fR service environments with many networks and many thousands of clients\&. This module provides an order of magnitude increase in performance and capacity over SUNWfiles\&.
.PP
This module\&'s data cannot be shared between \fBDHCP\fR servers\&.
.SS "SUNWnisplus"
.PP
This module stores its data within a \fBNIS+\fR domain\&. It is useful in environments where \fBNIS+\fR is already deployed and facilitates sharing among multiple \fBDHCP\fR servers\&. This module suports several hundred to a few thousand clients with lease times
of several hours or more\&.
.PP
The \fBNIS+\fR service should be hosted on a machine with ample \fBCPU\fR power, memory, and disk space, as the load on \fBNIS+\fR is significant when it is used to store \fBDHCP\fR data\&. Periodic checkpointing of the \fBNIS+\fR service
is necessary in order to roll the transaction logs and keep the \fBNIS+\fR service operating at its highest efficiency\&. See \fBnisping\fR(1M) and \fBcrontab\fR(1) for more information\&.
.SH "SEE ALSO"
.PP
\fBcrontab\fR(1), \fBdhcpconfig\fR(1M), \fBdhcpmgr\fR(1M), \fBdhtadm\fR(1M), \fBnisping\fR(1M), \fBpntadm\fR(1M), \fBdhcpsvc\&.conf\fR(4), \fBdhcp\fR(5)
.PP
\fISolaris DHCP Service Developer\&'s Guide\fR
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 14:38
