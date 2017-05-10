/* main (levopixie) */



#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <stdio.h>


/*#include <disassembler.h> /* libmld.a */

#define	PROCESS_MEM	1
#define NO_OUTPUT	0

#define ASSMBLY_BUFF	60000000
#define MAX_INST	2000000000
#define BUFSIZE		50

enum {LOAD=0, STORE, BRANCH };

typedef struct disassembly {
	char mnem[12];
	char oprnd[40];
} DISASM;

typedef struct {
	int tail;
	long ent[BUFSIZE];
} TRACEBUF;

DISASM disasmlist[ASSMBLY_BUFF];
TRACEBUF readbuf, writebuf,instbuf,membuf;

int
main(int argc,char *argv[]) {

char buf1[200];
char buf2[200];
char address[12],mnemonic[12],operands[40];
char *tok;
char name[100];
int j,i,rs,index;
char * ret;
int first=1;
long targaddr;
char *target;
long base,addr,addr1,addr2,jaddr,garb;
long total_inst=0;
long max_inst;
FILE *fasm,*fpix,*fperr;
char type2,type1;
int ret1;

readbuf.tail = 0;
writebuf.tail = 0;
instbuf.tail = 0;
membuf.tail=0;

if(argc <= 1) {
	printf("usage: Pixie2Qlevo zoo.alist zoo.pixie.trace.[Z]\n");
	return 1;
	}

strcpy(name,argv[1]);
fasm = fopen(name, "rt");
if(fasm == NULL) {
	printf("cannot open dis-assembly file\n");
	return 0;
}

if(argv[2][strlen(argv[2])-1] == 'Z') {

	strcpy(name,"zcat ");
	strcat(name,argv[2]);
	fpix = popen(name, "rb");
	if(fpix == NULL) {
		printf("cannot open pixie basic block file\n");
		return 0;
	}
}
else {
	strcpy(name,argv[2]);
	fpix = fopen(name, "rb");
	if(fpix == NULL) {
		printf("cannot open pixie trace file\n");
		return 0;
	}
}

fperr = fopen("error.log","wt");
if(!fperr) {
	printf("cannot write to error file \n");
	return 0;
}


/* read the assembly file into a buffer */
ret = fgets(buf1,200,fasm);
while(ret) {
	if(*buf1 == '[') {
		tok = (char *)strtok(buf1,"]");
		tok = (char *)strtok(NULL,": \t");
		if(first) {
			base = strtol(tok,NULL,16);
			addr = base;
			first = 0;
		}
		else {
			addr = strtol(tok,NULL,16);
		}
		strtok(NULL," \t");
		strtok(NULL," \t");
		strtok(NULL," \t");
		strtok(NULL," \t");
		tok = (char *)strtok(NULL," \t\n");
/*	printf("%x %x %x\n",addr,base,addr-base); */

		index = (addr-base)/4;
		assert(index<ASSMBLY_BUFF);
		strcpy(disasmlist[index].mnem,tok);
		tok = (char *)strtok(NULL," \t");
		if(tok)
			strcpy(operands,tok);
		else
			*operands = NULL;
/*		printf("%s#%s",address,mnemonic); 
		if(operands)
			printf("#%s\n",operands); */
	
		if(operands)
			strcpy(disasmlist[index].oprnd,operands);
		else
			*disasmlist[index].oprnd = NULL;
	}
	ret = fgets(buf1,200,fasm);
}

/*
for(i=0;i<1000;++i) {
	printf("%x #%s ",i*4+base,disasmlist[i].mnem);
	if(disasmlist[i].oprnd != NULL)
		printf("#%s \n",disasmlist[i].oprnd );
	else
		printf("\n");
}
*/

	max_inst=MAX_INST;
	if(!get_next(fpix,&addr1,&type1))
		return 0;
	if(type1 != 'I')
		assert(0);
	ret1 = get_next(fpix,&addr2,&type2);

	do {

		if(addr1 == addr2) {
			if(type2 == 'I');
				ret1 = get_next(fpix,&addr2,&type2);
		}

		if(inst_type(addr1,base) == LOAD) {
			makesure(fperr,type2 == 'R');
			print_instruction(fpix,addr1,base,addr2);
			ret1 = get_next(fpix,&addr2,&type2);
		}
		else if(inst_type(addr1,base) == STORE) {
			makesure(fperr,type2 == 'W');
			print_instruction(fpix,addr1,base,addr2);
			ret1 = get_next(fpix,&addr2,&type2);
		}
		else if(inst_type(addr1,base) == BRANCH) {
			print_instruction(fpix,addr1,base,addr2);
			/* print the delay slot instruction */
			addr1 += 4;
			total_inst++;
			if(addr1 == addr2) {
				makesure(fperr,type1 == type2);
				ret1 = get_next(fpix,&addr2,&type2);
			}
			if(inst_type(addr1,base) == LOAD) {
				makesure(fperr,type2 == 'R');
				print_instruction(fpix,addr1,base,addr2);
				ret1 = get_next(fpix,&addr2,&type2);
			}
			else if(inst_type(addr1,base) == STORE) {
				makesure(fperr,type2 == 'W');
				print_instruction(fpix,addr1,base,addr2);
				ret1 = get_next(fpix,&addr2,&type2);
			}	
			else {
				print_instruction(fpix,addr1,base,addr2);
			}

			addr1 = addr2-4;
			if(type2 != 'I') {
				makesure(fperr,0);
				while(type2 != 'I')
					ret1 = get_next(fpix,&addr2,&type2);
				addr1 = addr2-4;
			}
		}
		else {
			print_instruction(fpix,addr1,base,addr2);
		}
		addr1+=4;
		total_inst++;

	} while(total_inst < max_inst && ret1);


return 1;
}


int
makesure(FILE *fperr,int cond) {

static long viol=0;
	if(!cond) {
		++viol;
		fprintf(fperr,"viol=%ld\n",viol);
	}
}



int
inst_type(long addr1,long base) {

char *mnemonic,*operands;

	mnemonic = disasmlist[(addr1-base)/4].mnem;
	operands = disasmlist[(addr1-base)/4].oprnd;

	if(strchr(operands,'(')) {
		if(*mnemonic == 'l') {
			return LOAD;
		}
		else if(*mnemonic == 's') {
			return STORE;
		}
	}

	if((*mnemonic == 'b' && *(mnemonic+1)!='r'))
		return BRANCH;
	else if(!strcmp(mnemonic,"jal"))
		return BRANCH;
	else if(!strcmp(mnemonic,"jr"))
		return BRANCH;
	else if(!strcmp(mnemonic,"jalr"))
		return BRANCH;

}



int
get_next(FILE *fp, long *addr, char *type) {


	while(1) {


{
  unsigned char c,c1;
  unsigned long DAddr,IAddr,DSize;

    if(fread(&c1,1,1,fp)!=1){
      return 0;
    }
    fread(&c,1,1,fp);
    DAddr=c;DAddr<<=8;
    fread(&c,1,1,fp);
    DAddr|=c;DAddr<<=8;
    fread(&c,1,1,fp);
    DAddr|=c;
    c1&=0xf; /* Get the event type */

    switch(c1){
      case 0: /* 32bit WORD read */
      case 1: /* 64bit DWORD read */
      case 8: /* 32bit FLOAT read */
      case 9: /* 64bit DOUBLE Read */
        *type = 'R';
        break;
      case 2: /*  32bit WORD write */
      case 3: /*  64bit DWORD write */
      case 4: /*  8bit  BYTE write */
      case 5: /*  16bit HWHORD write */
      case 6: /*  Store word right */
      case 7: /*  Store word left */
      case 10: /* 32bit FLOAT write */
      case 11: /* 64bit DOUBLE Write */
        *type = 'W';
        break;
      case 12:       /* Basic block */
        IAddr=DAddr<<2;
	*type = 'I';
	*addr = IAddr;
        break;
      case 13: /* ANNUL */
        assert(DAddr*4==IAddr);
	*type = 'U';
	*addr = IAddr;
	assert(0);
	break;
      case 14: /* Unknown */
	*type = 'S';
	*addr = DAddr;
	break;
      default:
	assert(0);
    }

  switch (c1) {
    case 0: 

#ifdef	COMMENT
      // Byte, hword, LWL, LWR reads writen in the
      // trace as word reads, but the address remains.
      // We correct this beacuse of the word boundary
      // crossing problem
#endif

      DAddr&=0xfffffffcl;
    case 2:
    case 8:
    case 10:
      DSize=4ul;
      break;
    case 1:
    case 3:
    case 9:
    case 11:
      DSize=8ul;
      break;
    case 4:
      DSize=1ul;
      break;
    case 5:
      DSize=2ul;
      break;
    case 6:
      DSize=(DAddr|3ul)-DAddr+1;
      break;
    case 7:
      DSize=4-((DAddr|3)-DAddr);
      DAddr=DAddr-DSize+1;;
      break;
  }
if( *type == 'R')
	*addr = DAddr;
else if(*type == 'W')
	*addr = DAddr;
}


		if(*type != 'S') {
/*			printf("%c 0x%x\n",*type,*addr); */
			return 1;
		}
	}
	return 0;
}


int
old_get_next(FILE *fp, long *addr, char *type) {


char buf1[210],address[30],etype[10];

*etype = 0;

	while(fgets(buf1,200,fp)) {
		sscanf(buf1,"%s %s\n",etype,address);
		*addr = strtol(address,NULL,16);
		*type = *etype;
		if(*type != 'S')
			return 1;
	}
	return 0;

}




int
print_instruction(FILE *fp,long addr1, long base,long addr2) {

char address[30],*mnemonic,operands[80];
long memaddr;
char * endp;
int i;

	mnemonic = disasmlist[(addr1-base)/4].mnem;
	strcpy(operands, disasmlist[(addr1-base)/4].oprnd);
	endp = (char *) strchr(operands,'\n');
	if(endp) *endp = 0;

#if	PROCESS_MEM

	memaddr = -1;
	if(strchr(operands,'(')) {
		if(*mnemonic == 'l') {
			memaddr = addr2;
		}
		else if(*mnemonic == 's') {
			memaddr = addr2;
		}
	}

	if(memaddr != -1) {
#if	!NO_OUTPUT
		printf("0x%x %s %s <0x%x>\n",addr1, mnemonic, operands,memaddr);
#endif
	}
	else
#endif
#if	!NO_OUTPUT
		printf("0x%x %s %s\n",addr1, mnemonic, operands);
#endif
return 1;
}



int
gdb() {
static int i=0;
printf("%d\n",i++);
}



