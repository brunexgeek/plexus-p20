/*	Plexus - Sys3 - August 1982	*/

static char c21_c[] = "@(#)c21.c	1.2";

/*
 * C object code improver-- second part
 */

#include "c2.h"

rmove()
{
	register struct node *p;
	register int r;
	register  r1;
	register struct node *p1;

	for (p=first.forw; p!=0; p = p->forw) {
	switch (p->op) {

	case LD:
	ld:
		dualop(p);
		if( (r=isreg(regs[RT1])) >= 0 && r<NREG && regs[RT2][0] == '$'
		   && shortval(&regs[RT2][1]) <= 15) {
			p->op = LDK;
			nshortc++;
			goto ldk;
		}
		if( natural(regs[RT1]) && natural(regs[RT2])
		   && equstr(regs[RT1],regs[RT2]) ) {
			p->forw->back = p->back;
			p->back->forw = p->forw;
			redunm++;
			continue;
		}
		if ((r = findrand(regs[RT2], typeit(p->subop))) >= 0
		   && regnum( r, typeit(p->subop)) == isreg(regs[RT1]) ) {
			p->forw->back = p->back;
			p->back->forw = p->forw;
			redunm++;
			continue;
		}
		if ( equstr(regs[RT2], "$0") && p->subop != LONG ) {
			p->op = CLR;
			regs[RT2][0] = 0;
			clrop2( p->code );
			goto sngl;
		}
		repladdr(p, REPSRC);
		r = isreg(regs[RT2]);
		r1 = isreg(regs[RT1]);
		dest(regs[RT1], typeit(p->subop));
		if (r >= 0)
			if (r1 >= 0) {
				if ( regstype[rbase(r)] == rtype(r1) )
					savereg(r1, regs[rbase(r)]);
			}
			else
				savereg(r, regs[RT1]);
		else
			if (r1 >= 0)
				savereg(r1, regs[RT2]);
			else
				setcon(regs[RT2], regs[RT1], typeit(p->subop));
		source(regs[RT2]);
		continue;

	case LDA:
		dualop(p);
		dest( regs[RT1], typeit(p->subop) );
		continue;

	case LDK:
	ldk:
		dualop(p);
		dest( regs[RT1], typeit(p->subop) );
		savereg( isreg(regs[RT1]), regs[RT2] );
		continue;

	case ADD:
	case SUB:
		if( p->subop == LONG  )
			goto dble;
		dualop(p);
		if(regs[RT2][0] == '$' && (r1=shortval(&regs[RT2][1])) <=16){
			if( r1 != 0 ) {
				p->op = (p->op == ADD? INC: DEC);
				nshortc++;
				singop(p);
				goto sngl;
			} else {
				if( !needcc(p->forw) ) {
					p->forw->back = p->back;
					p->back->forw = p->forw;
					nlit++;
					continue;
				}
			}
		}
#ifndef QUICK1
		/* try for a 'lda' instruction */
		if( (r = isreg( regs[RT1] ) ) >= 0 && r < NREG
		    && regs[RT2][0] == '$' ) {
			char treg[20];
			char *c, *c1;

			if( p->back->op == LD ) {
				int sign = 0;
				c = treg; *c++ = ',';
				if( p->op == SUB ) sign = 1;
				c1 = &regs[RT2][1];
				if( *c1 == '-' ) {
					c1++;
					sign = !sign;
				}
				if( sign && ( *c1>'9' || *c1<'0' ) )
					/* negatives on lda must be constant */
					goto dble;
				if( sign ) *c++ = '-';
				while(*c++ = *c1++); c--;
				dualop( p->back );
				if( r == isreg(regs[RT1])
				    && isreg14(regs[RT2]) >= 0
				    && !needcc(p->forw) ) {
					nsaddr++;
					p->back->forw = p->forw;
					p->forw->back = p->back;
					p->back->op = LDA;
					c1 = regs[RT2];
					*c++ = '(';
					while(*c++ = *c1++); c--;
					*c++ = ')';
					*c = '\0';
					p->back->code =
						copy( 2, regs[RT1], treg );
				}
			}
		}
#endif
		goto dble;

	case AND:
	case OR:
	case MUL:
	case DIV:
	dble:
		dualop(p);
		if (p->op==AND && equstr(regs[RT2], "$0") && !needcc(p->forw)) {
			p->op = CLR;
			regs[RT2][0] = 0;
			clrop2( p->code );
			goto sngl;
		}
		if( (p->op==AND &&
		     (equstr(regs[RT2],"$-1") || equstr(regs[RT2],"$65535")))
		   || (p->op==OR && equstr(regs[RT2], "$0")) ) {
			if (!needcc(p->forw)) {
				p->back->forw = p->forw;
				p->forw->back = p->back;
				continue;
			}
		}
		repladdr(p, REPSRC);
		source(regs[RT2]);
		dest(regs[RT1], typeit(p->subop));
		ccloc[0] = 0;
		continue;

	case INC:
	case DEC:
		dualop(p);
		goto sngl;

	case CLR:
	case COM:
	case NEG:
		singop(p);
	sngl:
		dest(regs[RT1], typeit(p->subop));
		if (p->op==CLR )
			if ((r = isreg(regs[RT1])) >= 0)
				savereg(r, "$0");
			else
				setcon("$0", regs[RT1], typeit(p->subop));
		ccloc[0] = 0;
		continue;

	case TST:
		singop(p);
		repladdr(p, REPDST);
		source(regs[RT1]);
		if (equstr(regs[RT1], ccloc)) {
			p->back->forw = p->forw;
			p->forw->back = p->back;
			p = p->back;
			nrtst++;
			nchange++;
		}
		continue;

	case CP:
		dualop(p);
		source(regs[RT1]);
		source(regs[RT2]);
		if( equstr(regs[RT2],"$0")
		    && ( (p1 = p->forw)->op == CJP
		      || p1->op == LDK && (p1 = p1->forw)->op == TCC ) ) {
			switch( p1->subop ) {
			case JGE:
				p1->subop = JPL;
				goto simcomp;

			case JLT:
				p1->subop = JMI;

			case JPL:
			case JMI:
			case JEQ:
			case JNE:
			simcomp:
				p->op = TST;
				regs[RT2][0] = 0;
				clrop2( p->code );
				nchange++;
				ncomp0++;
			}
		}
		repladdr(p, REPSRC|REPDST);
		ccloc[0] = 0;
		continue;

	case CJP:
		if (p->back->op==TST || p->back->op==CP) {
			if (p->back->op==TST) {
				singop(p->back);
				savereg(RT2, "$0");
			} else
				dualop(p->back);
			r = compare(p->subop,
					findcon(RT1,typeit(p->back->subop)),
					findcon(RT2,typeit(p->back->subop)));
			if (r==0) {
				if( p->forw->op == CJP
				   || p->forw->op == TCC) {
					p->back->forw = p->forw;
					p->forw->back = p->back;
				} else {
					p->back->back->forw = p->forw;
					p->forw->back = p->back->back;
				}
				decref(p->ref);
				p = p->back->back;
				nrtst++;
				nchange++;
			} else if (r>0) {
				p->op = JP;
				p->subop = 0;
				p->back->back->forw = p;
				p->back = p->back->back;
				p = p->back;
				nrtst++;
				nchange++;
			}
		}
		ccloc[0] = 0;
		continue;

	case SLA:
	case SLL:
#ifndef QUICK1
		dualop(p);
		if( equstr( regs[RT2],"$1" ) ) goto shftadd;
		goto shift;
#endif

	case SRA:
	case SRL:
		dualop(p);
#ifndef QUICK1
		if( equstr( regs[RT2], "$-1" ) ) {
			register char *c,*c1;
	shftadd:
			p->op = ADD;
			c = regs[RT2];
			c1 = regs[RT1];
			*c++ = ',';
			while(*c++ = *c1++);
			p->code = copy( 2, regs[RT1], regs[RT2] );
			nshft++;
			goto dble;
		}
	shift:
#endif
		dest( regs[RT1], typeit(p->subop) );
		continue;

	case PUSH:
		dualop(p);
		if( !equstr(regs[RT1],"*r15") ) {
			clearreg();
			continue;
		}
		repladdr( p, REPSRC );
		source(regs[RT2]);
		continue;

	case EXTS:
#ifndef QUICK1
		singop(p);
		if(p->subop == BYTE && p->forw->op == AND
		   && p->forw->subop == 0) {
			r = isreg(regs[RT1]);
			dualop(p->forw);
			if( equstr(regs[RT2], "$255")
			   && r == isreg(regs[RT1]) ) {
				p->forw->back = p->back;
				p->back->forw = p->forw;
				p = p->forw;
				p->op = LD;
				p->subop = BYTE;
				regs[RT1][2] = regs[RT1][1]; /* rx to rhx */
				regs[RT1][1] = 'h';
				regs[RT1][3] = ',';
				regs[RT1][4] = '\0';
				p->code = copy(2, regs[RT1], "$0");
				nshortc++;
				goto ld;
			}
			singop(p);
		}
#endif
		goto sngl;

	case JP:
		redunbr(p);

	default:
		clearreg();
	}
	}
}

jumpsw()
{
	register struct node *p, *p1;
	register t;
	register struct node *tp;
	int nj;

	t = 0;
	nj = 0;
	for (p=first.forw; p!=0; p = p->forw)
		p->refc = ++t;
	for (p=first.forw; p!=0; p = p1) {
		p1 = p->forw;
		if (p->op == CJP && p1->op==JP && p->ref && p1->ref
		 && abs(p->refc - p->ref->refc) > abs(p1->refc - p1->ref->refc)) {
			if (p->ref==p1->ref)
				continue;
			p->subop = revbr[p->subop];
			tp = p1->ref;
			p1->ref = p->ref;
			p->ref = tp;
			t = p1->labno;
			p1->labno = p->labno;
			p->labno = t;
			nrevbr++;
			nj++;
		}
	}
	return(nj);
}

shortjump()
{
	register struct node *p, *p1;
	int change;

	change = 0;
	for (p=first.forw; p!=0; p = p->forw)
		p->refc = ++change;

	change = 0;
	for ( p = &first; (p1 = p->forw) != 0; p = p1 ) {
#ifndef QUICK1
		if( p->op == DEC && p1->op == CJP
		  && p1->subop == JNE ) {
			dualop(p);
			if( isreg(regs[RT1])>=0 && equstr(regs[RT2],"$1") ) {
				if( toofar(p1, 1) )
					continue;
				dualop( p );
				p->code = copy( 1, regs[RT1] );
				p->labno = p1->labno;
				p->op = SOB;
				p->subop = 0;
				p1->forw->back = p;
				p->forw = p1->forw;
				nsob++;
				change++;
			}
		}
#endif
		if(p->op == JP || p->op == CJP) {
			if(toofar(p, 0))
				continue;
			njr++;
			change++;
			p->op = (p->op == JP? JR: CJR);
		}
	}
	return( change );
}

toofar(p,backonly)
struct node *p;
{
	register struct node *p1, *p2;
	register int len;
	int back = 0;

	if( !(p2 = p->ref))
		return(1);
	if(p2->refc < p->refc) {
		p1 = p2;
		p2 = p;
		back++;
	} else {
		p1 = p;
	}

	if(backonly && !back)
		return( 1 );
	len = 0;
	for (; p1 && p1!=p2; p1 = p1->forw)
		len += ilen(p1);
	if (len < 250)
		return(0);
	return(1);
}

ilen(p)
register struct node *p;
{
	register l;

	switch (p->op) {
	case LABEL:
		return(0);

	case 0:
	case CJR:
	case JR:
	case SOB:
	case LDK:
	case TCC:
		return(2);

	case INC:
	case DEC:
		dualop(p);
		return( 2+adrlen(regs[RT1]) );

	case LDIR:
	case LDDR:
	case SDA:
	case SDL:
	case SLA:
	case SLL:
	case SRA:
	case SRL:
	case JP:
	case CJP:
		return(4);

	case BIT:
	case SET:
	case RES:
		dualop(p);
		if( isreg(regs[RT2])>=0 )
			regs[RT2][0] = '_';
		else
			regs[RT2][0] = '\0';
		goto dbl;

	default:
		dualop(p);
dbl:
		l = 2 + adrlen(regs[RT1]) + adrlen(regs[RT2]);

		if( (p->subop == LONG && regs[RT2][0] == '$' )
		   || (p->op == LDM) )
			l += 2;
		return( l );
	}
}

adrlen(s)
register char *s;
{
	if (*s == 0)
		return(0);
	if (*s=='r')
		return(0);
	if (*s=='*')
		return(0);
	return(2);
}

abs(x)
{
	return(x<0? -x: x);
}

