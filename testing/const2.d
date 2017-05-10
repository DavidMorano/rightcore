
const2:
 ***** PROGRAM EXECUTION HEADER *****
Type        Offset      Vaddr       Paddr
Filesz      Memsz       Flags       Align

PHDR        0x34        0x10034     0           
0xa0        0xa0        r-x         0           

INTERP      0xd4        0           0           
0x11        0           r--         0           

LOAD        0           0x10000     0           
0x71c       0x71c       r-x         0x10000     

LOAD        0x71c       0x2071c     0           
0x188       0x1a8       rwx         0x10000     

DYN         0x7c0       0x207c0     0           
0xb8        0           rwx         0           


	   **** SECTION HEADER TABLE ****
[No]	Type	Flags	Addr         Offset       Size        	Name
	Link	Info	Adralgn      Entsize

[1]	PBIT    -A-	0x100d4      0xd4         0x11         	.interp
	0	0	0x1          0            

[2]	HASH    -A-	0x100e8      0xe8         0xb8         	.hash
	3	0	0x4          0x4          

[3]	DYNS    -A-	0x101a0      0x1a0        0x150        	.dynsym
	4	1	0x4          0x10         

[4]	STRT    -A-	0x102f0      0x2f0        0x104        	.dynstr
	0	0	0x1          0            

[5]	VERN    -A-	0x103f4      0x3f4        0x20         	.SUNW_version
	4	1	0x4          0            

[6]	RELA    -A-	0x10414      0x414        0x24         	.rela.got
	3	13	0x4          0xc          

[7]	RELA    -A-	0x10438      0x438        0xc          	.rela.bss
	3	22	0x4          0xc          

[8]	RELA    -A-	0x10444      0x444        0x48         	.rela.plt
	3	14	0x4          0xc          

[9]	PBIT    -AI	0x1048c      0x48c        0x24c        	.text
	0	0	0x4          0            

[10]	PBIT    -AI	0x106d8      0x6d8        0x1c         	.init
	0	0	0x4          0            

[11]	PBIT    -AI	0x106f4      0x6f4        0x14         	.fini
	0	0	0x4          0            

[12]	PBIT    -A-	0x10708      0x708        0x14         	.rodata
	0	0	0x8          0            

[13]	PBIT    WA-	0x2071c      0x71c        0x28         	.got
	0	0	0x4          0x4          

[14]	PBIT    WAI	0x20744      0x744        0x7c         	.plt
	0	0	0x4          0xc          

[15]	DYNM    WA-	0x207c0      0x7c0        0xb8         	.dynamic
	4	0	0x4          0x8          

[16]	PBIT    WA-	0x20878      0x878        0x10         	.data
	0	0	0x4          0            

[17]	PBIT    WA-	0x20888      0x888        0x8          	.ctors
	0	0	0x4          0            

[18]	PBIT    WA-	0x20890      0x890        0x8          	.dtors
	0	0	0x4          0            

[19]	PBIT    WA-	0x20898      0x898        0x4          	.eh_frame
	0	0	0x4          0            

[20]	PBIT    WA-	0x2089c      0x89c        0x4          	.jcr
	0	0	0x4          0            

[21]	PBIT    WA-	0x208a0      0x8a0        0x4          	.data.rel.local
	0	0	0x4          0            

[22]	NOBI    WA-	0x208a4      0x8a4        0x20         	.bss
	0	0	0x4          0            

[23]	SYMT    ---	0            0x8a4        0x4e0        	.symtab
	24	58	0x4          0x10         

[24]	STRT    ---	0            0xd84        0x258        	.strtab
	0	0	0x1          0            

[25]	PBIT    ---	0            0xfdc        0x12a        	.comment
	0	0	0x1          0            

[26]	PBIT    ---	0            0x1108       0x24         	.stab.index
	0	0	0x4          0xc          

[27]	STRT    ---	0            0x112c       0xee         	.shstrtab
	0	0	0x1          0            

[28]	STRT    ---	0            0x121a       0x176        	.stab.indexstr
	0	0	0x1          0            

