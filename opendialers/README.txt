OPENDIALERS

Open-Dialers are file-names that provide a two-way communication path to a
service that is reached by a dialer.  These file-names take the form:

<dialer>¥<svc>[:<arg0>][­<arg(s)>]

Each dialer makes it own interpretation of <svc> and <arg0>.  The arguments
<arg(2)> are also passed to the dialer, but again, dialers can really do what
they like with any of these.

Examples of time open-dialers are:

prog
tcp
tcpmux
uss
ussmux
udp
usd
finger
http

An example of opening a dialer with an actual file-name is:

$ shcat tcp¥localhost:daytime

Note that is this example (and not uncommonly in many others) both a <svc>
component and an <arg0> component are used in order to complete a propler
open-dialer filename.

