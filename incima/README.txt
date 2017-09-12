INCIMA

This is a DWB filter program (like 'eqn' and 'tbl') but it recognizes a request
to include a picture of some sort. It takes the picture and translates it to EPS
for inclusion into a DWD-like text document. This program is run automatically
in many text processing contexts where the resulting file needs to be flattened.

A directive to include an inmage should appear as follows in the input text
stream:

.\"_
.\"_ format file(page) h w pos off flags label
.BG pnt scrollhead.pnt 3.5i 5.0i c 0.0i
.\"_

Compare with using the Picture Macros (which only allow for EPS inclusion):

.\"_
.\"_ <file.eps>(<page>) <h> <w> <pos> <off> <flags> <label>
.BP <file.eps> 3.5i 5.0i c 0.0i
.\"_

