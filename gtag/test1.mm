.\"_
.BIB levosimxml.rbd
.\"_
.\"_ starting section number is (this + one)
.\"_ .nr H1 2
.\"_
.\"_ heading level saved for table of contents
.nr Cl 3
.\"_
.\"_
.\"_ heading fonts
.ds HF 3 3 3 2 2 2 2
.\"_
.ND "18 January 2002"
.\"_
.OH "'Levo''Levo state trace output'"
.EH "'Levo state trace output''Levo'"
.OF "'''\\*(DT'"
.EF "'\\*(DT'''"
.\"_
.\"_
.TL
Levo state trace output
.\"_
.AF "Northeastern University"
.AU "David A. Morano"
.\"_
.\"_
.\"_ released paper style
.MT 4
.\"_
.\"_
.\"_ default text width on page is 6 inches!
.\"_ file(page) height,width,yoffset,xoffset flags
.PI levo2.eps 8i,6i,0i,0i a270
.\"_
.\"_
.H 1 "introduction"
.P
This document will describe the Levo machine clock state output
trace format.  
Although the Levo machine being sequenced is
done so at the moment using the LevoSim simulator, this is not
strictly necessary and future Levo simulators may be used instead.
In any event, the output state of the Levo machine may contain
similar elements across more than one simulator.
.\"_
.H 2 "Levo machine"
.P
The output state trace described in this document is the
registered state information from a Levo machine.  
More information
on the Levo machine and its design philosophy can be found in
the Technical Report by Uht. \cite{Uht01}
.\"_
.H 2 "LevoSim"
.P
The current vehicle used to sequence a Levo machine is the LevoSim
simulator.
This simulator implements the actual hardware components of
a Levo machine using software.
Hardware components are modeled individually and are interconnected
to form the whole machine.  
The whole machine is somewhat hierarchical
and this hierarchy is usually expressed within the simulator also
with its various software pieces.  
For most of the whole Levo machine,
each hardware component is implemented with a corresponding software
component and possibly several subcomponents each of which can have
their own subcomponents.

The simulated hardware components process inputs that are usually
the outputs from hardware registers and produce new next-state.
The processing of combinatorial signals occurs over time
through a mechanism in the simulator known as clock phases.
These both somewhat mimic the phases of the hardware clock period
as well as for the provision of synchronization points in
time for the communication of signal between different hardware
components.
All hardware components are also clocked.  
Typically, all clocked
hardware components transition the previously computed next-state
signals
to be the current state of the new clock period.

In this way, the whole of the simulator resembles very closely the hardware
of an actual machine, transitioning synchronously through states in
many state machines.  
More information on the LevoSim simulator
is currently only found in some outline oriented documentation
that exists for most of the major components as well as in the
source code itself.
All information for LevoSim (documentation and source code)
is currently at the University of Rhode
Island in the normal Levo research file systems.
.\"_
.H 2 "XML"
.P
The Levo output state trace will make use of XML as the 
structuring of all machine state data.  
There will be two files
produced by LevoSim for the output state trace.  
One file will be the XML proper.  
The other file will be an index of the XML file.  
The index file is an index of the start of clock blocks
in the XML file.  
The index file contains both the starting offset
of clock blocks in the XML file as well as a means for finding an
XML clock block that contains a full dump of all machine registers
(that are being dumped at all).
Most of the rest of this document will describe the details of
these two files.
.\"_
.H 1 "XML usage"
.P
The decision to use XML as the means to structure the output
trace data was largely quite pragmatic.
A means was needed to both tag data with a name as well
as a way to structure related data hierarchically (thus matching
the hierarchy of the hardware components themselves).
XML conveniently provides both of these capabilities.
XML is also rather simple to write and to understand
giving it an advantage over other methods.
XML also brings a significant disadvantage than other approaches.
The main disadvantage of using XML for our purposes is that since
it is all ASCII, it tends to consume a large amount of space
for relatively modest information content.  
Further, closing tags are really redundant but take up space none-the-less.
However, it is felt that the advantages of XML outweigh the
disadvantages for our immediate goals.

A brief outline of how XML is used in the state output
will now be presented.
XML consists of tags (opening and closing) and the data
associated with those tags.  
Tags may introduce blocks
that contain more tags, et cetera.
Some definitions of some XML structures is given next.

.VL 20
.LI "leaf tags"
these are tags that 
include only data elements in them and no further tags
.LI "block tags"
these tags introduce a block that only has other tags in it
.LI "generic block tags"
these are block tags but may appear in several other
tag blocks and will have the same meaning everywhere
.LE 1

For purposes of this document, block tags and generic tags
will be described individually.  
Leaf tags will always
be described along with the block tags that enclose them.
.\"_
.H 2 "miscellaneous tags"
.P
Some tags are not part of the Levo machine hardware but serve
to frame other important information.
Two tags are in this category so far.
These are described below:
.\"_
.H 3 "the configuration tag"
.P
This tag appears as the very first tab in the XML file.
This block tag given the information on the configuration of
the Levo machine that was sequenced.
It contains several leaf tags that give the parameters used
to configure the Levo machine.
These tags are:

.VL 20
.LI nsgrows
a decimal number that gives the number of Sharing Group rows in the machine
.LI nsgcols
a decimal number that
this gives the number of Sharing Group columns in the machine
.LI nasrpsg
a decimal number that gives the number of Active Station rows
per each Sharing Group; note that the number of Active Station
columns in each Sharing Group is currently fixed at two
.LI nrfspan
a decimal number that gives the span (in SGs) of the Register
Forwarding buses
.LI nrbspan
a decimal number that gives the span (in SGs) of the Register
Backwarding buses
.LI npfspan
a decimal number that gives the span (in SGs) of the Predicate
Forwarding buses
.LI nmfspan
a decimal number that gives the span (in SGs) of the Memory
Forwarding buses
.LI nmbspan
a decimal number that gives the span (in SGs) of the Memory
Backwarding buses
.LI ndeepaths
a decimal number that gives the number of possible DEE paths
in the machine
.LI nprpas
a decimal number that gives the number of possible Predicate
Registers that can be held in any single Active Station
.LI ifetchwidth
a decimal number that gives the with (in 32-bit units) of
the Instruction Response bus
.LI mfbinter
this is a number in hexadecimal that gives the bits that form the
bus interleave index for the Memory Forwarding bus 
between the execution window and the memory subsystem
.LI mbbinter
this is a number in hexadecimal that gives the bits that form the
bus interleave index for the Memory Backwarding bus
between the execution window and the memory subsystem
.LI mwbinter
this is a number in hexadecimal that gives the bits that form the
bus interleave index for the Memory Write bus
between the execution window and the memory subsystem
.LI iw:rftype
this is a decimal number that gives the type of register forwarding
that is used in the machine
.LI iw:btrbsize
this is a decimal number that gives the size of the Branch Tracking Buffer
.LI iw:sqsize
this is a decimal number that gives the Store Queue FIFO depth
.LI iw:wmfinter
this is a number in hexadecimal that gives the bits that form the
bus interleave index for the Memory Forwarding bus 
within the execution window
.LI iw:wmbinter
this is a number in hexadecimal that gives the bits that form the
bus interleave index for the Memory Backwarding bus 
within the execution window
.LE 1
.\"_
.H 3 "clock tag"
.P
This tag is used for all other basic blocks in the trace output.
It currently only has two sub-tags.  
Tags that can appear in this
tag block are:

.VL 20
.LI value
this is a leaf tag that gives the clock number of the current
clock block
.LI levo
this is a block tag that encloses all Levo machine hardware
component tags
.LE 1
.\"_
.H 2 "Levo machine component tags"
.P
This section describes the Levo machine hardware component tags.
These tags are divided into two major types.  
The block tags used
within for Levo machine components fall into two types.
These are the generic block tags and the non-generic block tags.
.\"_
.H 3 "Non-generic tags"
.P
This section describes most of the block tags used.
.\"_
.H 4 "levo tag"
.P
This tag encloses all other Levo machine component tags.
It's sub-tags are:

.VL 20
.LI lmem
this tag encloses all memory subsystem block tags
.LI iw
this tag encloses all execution window block tags
.LI busgroup
this is a generic block tag that describes a group of
related buses
.LI busname
this is a generic block tag that describes a single bus
.LE 1
.\"_
.H 4 "lmem tag"
.P
This tag represents the top of the entire memory subsystem of the
Levo machine.  
The sub-tags for this tag defined so far so far are:

.VL 20
.LI uid
a hexadecimal number that gives the unique ID of this block
.LE 1
.\"_
.H 4 "the iw tag"
.P
This block tag introduces the Levo machine execution window.
This is a big one!  
This contains most of the machine
components of the machine.  
The sub-tags of this block are:

.VL 20
.LI lifetch
this block tag introduces the LIFETCH hardware component
.LI llb
this block tag introduces the LLB (load buffer) hardware component
.LI sg
this is a block tag that introduces a Sharing Group logical
arrangement of Levo hardware components
.LI lwq
this is a block tag that introduces the Levo Write Queue (LWQ)
hardware component
.LI
.LE 1
.\"_
.H 4 "the lifetch tag"
.P
Currently, there are no sub-tags in this block.
.\"_
.H 4 "the llb tag"
.P
This block tag introduces the state for the Levo Load Buffer (LLB)
hardware component.  
This is basically just a register of
decoded instructions.  
The sub-tag for this block are:

.VL 20
.LI entry
this is a block tag that introduces one entry of many for the LLB
component; the number of entries is the height of an AS column
.LE 1
.\"_
.H 4 "the entry tag"
.P
This tag introduces one entry (row) of the LLB.
Its sub-tags are:

.VL 20
.LI row
a leaf tag containing a decimal number giving the row index of this
entry
.LI valid
a leaf tag containing a single binary digit indicating whether 
the \fIvalid\fP bit is set in this entry or not
.LI used
a leaf tag containing a single binary digit indicating whether the
entry contains an instruction of some sort or otherwise unused
.LI ia
if the entry is used (see above), then this leaf tag will appear
containing a hexadecimal number giving the instruction address of
the instruction in this LLB entry
.LI instr
if the entry is used (see above), then this leaf tag will appear
containing a hexadecimal number giving the 32-bit instruction word
for the
instruction in this LLB entry
.LI instrdis
if the entry is used (see above), then this leaf tag will appear
containing an ASCII string of the disassembled instruction that
is in this LLB entry
.LE 1
.\"_
.H 4 "the sg tag"
.P
This block tag introduces a Sharing Group (SG).  
A Sharing Group
is a logical arrangement of several hardware components.
There can be many Sharing Groups in the Levo machine execution window.
Not all of the components within a Sharing Group are currently
programmed to output state but some are.  
The sub-tags within this
block tag are:

.VL 20
.LI sgid
a leaf tag giving in decimal the identification of this Sharing
Group (IDs are small positive integers); SGIDs are unique within
the whole Levo machine execution window
.LI ascol
this is a block tag that introduces a column of Active Station
element (described later) component groups
.LI lpe
this is a block tag that introduces a Levo Processing Element (LPE)
hardware component
.LE 1
.\"_
.H 4 "the ascol tag"
.P
The sub-tags in this block tag are:

.VL 20
.LI id
this leaf tag gives in decimal the ID of this column within
the Sharing Group
.LI aselem
this is a block tag that introduces an Active Station element
within this Sharing Group
.LE 1
.\"_
.H 4 "the aselem tag"
.P
This is a block tag that exists with Sharing Groups.  
This contains a single 
The sub-tags of this block tag are:

.VL 20
.LI id
this leaf tag contains a decimal number giving the relative ID of this
Active Station element within the Active Station column grouping
.LI asrow
this leaf tag contains a decimal number giving the execution window
row index of this Active Station element within the context of the
entire execution window
.LI asid
this leaf tag contains a decimal number that is the ID of the enclosed
Active Station (to be described later) within the context of the entire
Levo machine execution window
.LI as
this block tag introduces the state within an Active Station hardware
component
.LE 1
.\"_
.H 4 "the as tag"
.P
This tag introduces the state within a single Active Station hardware
component.  
There are often many of these in any configured Levo machine.
Current restrictions on the geometry of the Levo machine as implemented
in LevoSim requires that there be at least two Active Stations in
each column of the execution window of the machine.
The sub-tag are:

.VL 20
.LI uid
this leaf tag contains a hexadecimal number giving the unique ID
of this Active Station component; UIDs are useful for tracking
hardware components in the software
.LI asid
this leaf tag contains a decimal number giving the ID of this
Active Station within the execution window (same as \fIasid\fP in
other block tags); this ID is useful for human tracking of ASes
.LI used
this leaf tag contains a single binary digit indicating whether
this AS has an instruction in it or if it is otherwise unused
.LI path
this leaf tag contains a decimal number that indicates what
path the current AS is associated with; paths are small non-zero
positive integers and will not exceed in value the number of
DEE paths configured for the current machine plus one
.LI tt
this leaf tag contains a decimal number indicating the time-tag
associated with the current AS; the number is a small positive
integer
.LI ia
if the entry is used (see above), then this leaf tag will appear
containing a hexadecimal number giving the instruction address of
the instruction in this AS
.LI instr
if the entry is used (see above), then this leaf tag will appear
containing a hexadecimal number giving the 32-bit instruction word
for the
instruction in this AS
.LI instrdis
if the entry is used (see above), then this leaf tag will appear
containing an ASCII string of the disassembled instruction that
is in this AS
.LI opclass
this leaf tag contains a decimal number indicating the class of
the instruction that has been issued to this AS;
the number is a small positive integer
.LI opexec
this leaf tag contains a decimal number indicating the decoded opcode
of the instruction that has been issued to this AS;
the number is a small positive integer
.LI srcreg
this is a block tag that contains a source register set
for this AS; there are currently up to four of these in
any AS
.LI dstreg
this is a block tag that contains a destination register set
for this AS; this destination register set is not the output
of the instruction execution in the current AS but rather the
input snoop/snarf information on this register; there are 
currently up to three of these in
any AS
.LE 1
.\"_
.H 4 "srcreg and dstreg tags"
.P
These block tags appear in Active Station blocks and are
currently very similar.  
A source register set contains the
register information associated with one source operand to the
instruction loaded in an AS.
A destination register set contains the register information
associated with one destination operand for the instruction
loaded in the AS.  
The term 'destination' may be misleading as
this block contains state information for the input of this
register to the current AS.  
The values in a destination register
set are those for the operand before any possible computation
by the current AS.  
Therefore both source and destination register
sets described here are inputs to as AS and are typically snooped
and snarfed identically.
Both of these block tags currently have the following sub-tags:

.VL 20
.LI name
a leaf tag containing an ASCII string taken to the name of the
register set
.LI addr
a leaf tag containing a hexadecimal number of the address of the
register operand
.LI dv
a leaf tag containing a hexadecimal number that is the current data value
of the register operand
.LI path
a leaf tag containing a decimal number giving the path that this register
operand came from
.LI tt
a leaf tag containing a decimal number giving the time-tag value of the
source of this register
operand; this is the time-tag of the hardware component where the
operand was generated from
.LE 1
.\"_
.H 4 "the lpe tag"
.P
This is a block tag that introduces the state for the Levo Processing
Element hardware component.  
Currently, there is no clearly defined
state for this component so there are not yet any sub-tags.
.\"_
.H 4 "the lwq tag"
.P
This tag introduces the Levo Write Queue (LWQ) hardware component.
There are not yet any sub-tags for this component.
.\"_
.H 3 "Generic tags"
.P
This section describes some generic block tags.
These tags are used in more than one place.  
That is, they may be
enclosed by more than one other block tag.
.\"_
.H 4 "the busgroup tag"
.P
This tag introduces a block that describes a group of
related buses.  
Some of the sub-tags of this block tag
are optional.  
Usually only one other block tag will be
enclosed by a \fIbusname\fP tag.
The \fIpossible\fP sub-tags are:

.VL 20
.LI name
this is a leaf tag whose value is a name assigned to the group of buses
.LI rfbus
this is a generic block tag that introducing a RFBUS type hardware
component
.LI libus
this is a generic block tag that introducing a LIBUS type hardware
component
.LE 1
.\"_
.H 4 "the busname tag"
.P
This tag introduces a single bus.
The enclosed bus is usually described by a generic block sub-tag.
Some of the sub-tags of this block tag
are optional.  
Usually only one other block tag will be
enclosed by a \fIbusname\fP tag.
The \fIpossible\fP sub-tags are:

.VL 20
.LI name
this is a leaf tag whose value is a name assigned to the group of buses
.LI rfbus
this is a generic block tag that introducing a RFBUS type hardware
component
.LI libus
this is a generic block tag that introducing a LIBUS type hardware
component
.LE 1
.\"_
.H 4 "the rfbus tag"
.P
This block tag introduces the state for the RFBUS hardware component.
The sub-tags are:

.VL 20 
.LI uid
a leaf tag containing a hexadecimal number giving the unique ID of the component
.LI busy
a leaf tag containing a single binary digit specifying whether the bus
is currently busy or not
.LI hold
a leaf tag containing a single binary digit specifying whether the bus
is currently being held (stalled) or not
.LI dp
a leaf tag containing a single binary digit specifying whether the bus
has any data present in the current clock
.LI lflowgroup
this is a generic block tag that contains a LFLOWGROUP object of related
bus data
.LE 1
.\"_
.H 4 "the libus tag"
.P
This block tag introduces the state for the RFBUS hardware component.
The sub-tags are:

.VL 20 
.LI uid
a leaf tag containing a hexadecimal number giving the 
unique ID of the component
.LI busy
a leaf tag containing a single binary digit specifying whether the bus
is currently busy or not
.LI hold
a leaf tag containing a single binary digit specifying whether the bus
is currently being held (stalled) or not
.LI dp
a leaf tag containing a single binary digit specifying whether the bus
has any data present in the current clock
.LI datalane
this is a generic block tag that contains a single lane of data
for multi-laned buses; there can be many of these tags in
a single \fIlibus\fP tag
.LE 1
.\"_
.H 4 "the datalane tag"
.P
This block tag introduces the information for a single lane of data
for some outer block tag.
The sub-tags are:

.VL 20 
.LI id
a leaf tag containing a decimal number giving the relative 
ID of the component
.LI lflowgroup
a block tag that contains the related data elements of an LFLOWGROUP
hardware object
.LE 1
.\"_
.H 4 "the lflowgroup tag"
.P
This is a block tag that introduces the information for the LFLOWGROUP
hardware object.  
This object generally corresponds with a "transaction"
in the Levo machine.  
Transactions contain both architectural data
as well as micro-architectural meta-data describing the context of
the architectural data.
The sub-tags are:

.VL 20 
.LI ftt
a leaf tag containing a decimal number giving the forwarder's
time tag value; a forwarder was a component that last forwarded
this LFLOWGROUP if any
.LI ott
a leaf tag containing a decimal number giving the originator's
time tag value; an originator was a component that first originated
this LFLOWGROUP 
.LI tt
a leaf tag containing a decimal number giving the 
time tag value of this present LFLOWGROUP; this is the time tag
used for snooping of this LFLOWGROUP
.LI seq
a leaf tag containing a decimal number giving the 
sequence number value of this present LFLOWGROUP 
.LI path
a leaf tag containing a decimal number giving the 
path for this present LFLOWGROUP 
.LI addr
a leaf tag containing a hexadecimal number giving the 
address of this present LFLOWGROUP 
.LI dv
a leaf tag containing a hexadecimal number giving the 
data value of this present LFLOWGROUP 
.LI dp
a leaf tag containing a decimal number giving the 
data-present bits (the low four bits)
for the data in this present LFLOWGROUP 
.LI trans
a leaf tag containing a decimal number giving the 
transaction code for this present LFLOWGROUP 
.LE 1
.\"_
.H 1 "Acknowledgments"
.P
Many thanks go to Sean Langford for the initial discussions
I had with him about using XML for the output state dump
and his enthusiasm for the idea.
.\"_
.\"_
.\"_ force odd page
.OP
.\"_ table of contents
.TC
.\"_
.\"_
