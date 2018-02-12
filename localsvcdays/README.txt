LOCALSVC

This directory contains some loadable facility-open-services.  

service		description
--------------------------------------------------

cotd		Commandment-Of-The-Day
votd		Verse-of-the-Day
hotd		History-Of-The-Day

= COTD

This service provides the COTD type of function for an opened FD.
The service can take an argument which is a day-of-the-month specification
to indicate what COTD to print out.  COTDs are not cached as they are
very cheap already and essentially in an essentially almost cached format
as they are.

Synopsis:
local§cotd[­<day>]

= VOTD

This service provides the VOTD function for the returned FD.

Synopsis:
local§votd[­<day>][­-o­allcache]

= HOTD

This service provides the HOTD function for the returned FD.

Synopsis:
local§hotd[­<day>]

