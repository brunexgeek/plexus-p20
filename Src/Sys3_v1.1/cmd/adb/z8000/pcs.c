#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"


MSG		NOBKPT;
MSG		SZBKPT;
MSG		EXBKPT;
MSG		NOPCS;
MSG		BADMOD;

/* breakpoints */
BKPTR		bkpthead;

CHAR		*lp;
CHAR		lastc;
POS		corhdr[ctob(1)];
POS		*endhdr;

INT		signo;
L_INT		dot;
INT		pid;
L_INT		cntval;
L_INT		loopcnt;



/* sub process control */

subpcs(modif)
{
	REG INT		check;
	INT		execsig;
	INT		runmode;
	REG BKPTR	bkptr;
	STRING		comptr;
	INT		offset;
	L_INT		olddot;
	extern int	adrflg;
	extern long	adrval;
	offset=0;
	execsig=0; loopcnt=cntval;

	switch(modif) {

	    case 'q': case 'Q':
		check = get(dot,ISP) & 0xff00;
		IF (check & 0xf000) == 0xe000
		THEN
		    offset = 2;
		ELIF check == 0x2100
		THEN
		    offset = 8;
		ELIF check == 0xbd00
		THEN
		    offset = 6;
		ELSE
		    offset = 4;
		FI
/*		IF (get(dot,ISP) & 0xff00) == 0xbd00
		THEN
		    offset = 6;
		ELSE
		    offset = 8;
		FI
*/
		    /* fall into normal breakpoint delete */

	    /* delete breakpoint */
	    case 'd': case 'D':
		IF (bkptr=scanbkpt(shorten(dot)+offset))
		THEN bkptr->flag=0; return;
		ELSE error(NOBKPT);
		FI

	    /* delete all breakpoints */
	    case 'x': case 'X':
		FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
		DO
		    bkptr->flag = 0;
		OD
		return;

	    /* set breakpoint upon current procedure exit */
	    case 'e': case 'E':
#ifdef DEBUG
printf("wanted = %X\n",adrval);
#endif
		if(adrflg) {
			if(srchfrm())
				printf("symbol not found in stack\n");
			return;
		}
		olddot = dot;
		dot = get(leng(endhdr[r14]+2),DSP);
		subpcs('b');
		dot=olddot;
		return;

	    /* set breakpoint at procedure entry */
	    case 'p': case 'P':
		check = get(dot,ISP) & 0xff00;
		IF (check & 0xf000) == 0xe000
		THEN
		    offset = 2;
		ELIF check == 0x2100
		THEN
		    offset = 8;
		ELIF check == 0xbd00
		THEN
		    offset = 6;
		ELSE
		    offset = 4;
		FI
		/* fall into normal breakpoint code */

	    /* set breakpoint */
	    case 'b': case 'B':
#ifdef DEBUG
printf("dot = %d, offset = %d\n",(int)dot,offset);
#endif
		IF (bkptr=scanbkpt(shorten(dot) + offset))
		THEN bkptr->flag=0;
		FI
		FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
		DO IF bkptr->flag == 0
		   THEN break;
		   FI
		OD
		IF bkptr==0
		THEN IF (bkptr=sbrk(sizeof *bkptr)) == -1
		     THEN error(SZBKPT);
		     ELSE bkptr->nxtbkpt=bkpthead;
			  bkpthead=bkptr;
		     FI
		FI
		bkptr->loc = dot + offset;
		bkptr->initcnt = bkptr->count = cntval;
		bkptr->flag = BKPTSET;
		check=MAXCOM-1; comptr=bkptr->comm; rdc(); lp--;
		REP *comptr++ = readchar();
		PER check-- ANDF lastc!=EOR DONE
		*comptr=0; lp--;
		IF check
		THEN return;
		ELSE error(EXBKPT);
		FI

	    /* exit */
	    case 'k' :case 'K':
		IF pid
		THEN printf("%d: killed", pid); endpcs(); return;
		FI
		error(NOPCS);

	    /* run program */
	    case 'r': case 'R':
		endpcs();
		setup();
		runmode=CONTIN;
		break;

	    /* single step */
	    case 's': case 'S':
		runmode=SINGLE;
		IF pid
		THEN execsig=getsig(signo);
		ELSE setup(); loopcnt--;
		FI
		break;

	    /* continue with optional signal */
	    case 'c': case 'C': case 0:
		IF pid==0 THEN error(NOPCS); FI
		runmode=CONTIN;
		execsig=getsig(signo);
		break;

	    default: error(BADMOD);
	}

	IF loopcnt>0 ANDF runpcs(runmode, execsig)
	THEN printf("breakpoint%16t");
	ELSE printf("stopped at%16t");
	FI
	delbp();
	printpc();
}

/* search through frame sequence for match to symbol address */
srchfrm()
{
	L_INT frame, olddot;
	INT oldadrflg,addr;
	extern int lastframe, callpc, calledpc, adrflg;
	extern long adrval;

	frame = endhdr[r14]&EVEN;
	callpc = get(frame+2,DSP);
	addr = get((long)(callpc-2),ISP);
#ifdef DEBUG
printf("frame = %X callpc = %x addr = %x\n",frame,callpc,addr);
#endif
	lastframe = 0;
	calledpc = dot;
	while(1) {
		chkerr();
		if((int)adrval == addr) {
			olddot = dot;
			oldadrflg = adrflg; adrflg = 0; /* prevent recursion */
			dot = callpc;	/*get(leng(callpc), DSP);*/
			subpcs('b');
			adrflg = oldadrflg;
			dot = olddot;
			return(0);
		}
		lastframe = frame;
		frame = get(frame, DSP)&EVEN;
		calledpc = callpc;
		callpc = get(frame+2, DSP);
		addr = get((long)(callpc-2),ISP);
#ifdef DEBUG
printf("frame = %X callpc = %x addr = %x\n",frame,callpc,addr);
#endif
		if(frame == 0) break;
	}
	return(1);
}
