/* elfmips (include) */


#ifndef	ELFMIPS_INCLUDE
#define	ELFMIPS_INCLUDE		1



/* special ELF header ('Elf32_Ehdr') flags (for element 'e_flags') */

#define EF_MIPS_PIC		0x00000002
#define EF_MIPS_CPIC		0x00000004
#define EF_MIPS_ARCH		0xf0000000
#define EF_MIPS_ARCH_1		0x00000000
#define EF_MIPS_ARCH_2		0x10000000
#define EF_MIPS_ARCH_3		0x20000000
#define EF_MIPS_ARCH_4		0x30000000


/* special program header types for MIPS */

#define	PT_REGINFO	0x70000000
#define	PT_MIPSOPTS	0x70000002


/* special section types for MIPS */

#define SHT_MDEBUG	0x70000005
#define	SHT_REGINFO	0x70000006
#define	SHT_MIPSOPTS	0x7000000d


#endif /* ELFMIPS_INCLUDE */



