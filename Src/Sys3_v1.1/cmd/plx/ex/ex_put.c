/* Copyright (c) 1979 Regents of the University of California */
#include "ex.h"
#include "ex_tty.h"
#include "ex_vis.h"

/*
 * Terminal driving and line formatting routines.
 * Basic motion optimizations are done here as well
 * as formatting of lines (printing of control characters,
 * line numbering and the like).
 */

/*
 * The routines outchar, putchar and pline are actually
 * variables, and these variables point at the current definitions
 * of the routines.  See the routine setflav.
 * We sometimes make outchar be routines which catch the characters
 * to be printed, e.g. if we want to see how long a line is.
 * During open/visual, outchar and putchar will be set to
 * routines in the file ex_vput.c (vputchar, vinschar, etc.).
 */
int	(*Outchar)() = termchar;
int	(*Putchar)() = normchar;
int	(*Pline)() = normline;

int (*
setlist(t))()
	bool t;
{
	register int (*P)();

	listf = t;
	P = Putchar;
	Putchar = t ? listchar : normchar;
	return (P);
}

int (*
setnumb(t))()
	bool t;
{
	register int (*P)();

	numberf = t;
	P = Pline;
	Pline = t ? numbline : normline;
	return (P);
}

/*
 * Format c for list mode; leave things in common
 * with normal print mode to be done by normchar.
 */
listchar(c)
	register short c;
{

	c &= (TRIM|QUOTE);
	switch (c) {

	case '\t':
	case '\b':
		outchar('^');
		c = ctlof(c);
		break;

	case '\n':
		break;

	case '\n' | QUOTE:
		outchar('$');
		break;

	default:
		if (c & QUOTE)
			break;
		if (c < ' ' && c != '\n' || c == DELETE)
			outchar('^'), c = ctlof(c);
		break;
	}
	normchar(c);
}

/*
 * Format c for printing.  Handle funnies of upper case terminals
 * and crocky hazeltines which don't have ~.
 */
normchar(c)
	register short c;
{
	register char *colp;

	c &= (TRIM|QUOTE);
	if (c == '~' && HZ) {
		normchar('\\');
		c = '^';
	}
	if (c & QUOTE)
		switch (c) {

		case ' ' | QUOTE:
		case '\b' | QUOTE:
			break;

		case QUOTE:
			return;

		default:
			c &= TRIM;
		}
	else if (c < ' ' && (c != '\b' || !OS) && c != '\n' && c != '\t' || c == DELETE)
		putchar('^'), c = ctlof(c);
	else if (UPPERCASE)
		if (isupper(c)) {
			outchar('\\');
			c = tolower(c);
		} else {
			colp = "({)}!|^~'`";
			while (*colp++)
				if (c == *colp++) {
					outchar('\\');
					c = colp[-2];
					break;
				}
		}
	outchar(c);
}

/*
 * Print a line with a number.
 */
numbline(i)
	int i;
{

	if (shudclob)
		slobber(' ');
	printf("%6d  ", i);
	normline();
}

/*
 * Normal line output, no numbering.
 */
normline()
{
	register char *cp;

	if (shudclob)
		slobber(linebuf[0]);
	/* pdp-11 doprnt is not reentrant so can't use "printf" here
	   in case we are tracing */
	for (cp = linebuf; *cp;)
		putchar(*cp++);
	if (!inopen)
		putchar('\n' | QUOTE);
}

/*
 * Given c at the beginning of a line, determine whether
 * the printing of the line will erase or otherwise obliterate
 * the prompt which was printed before.  If it won't, do it now.
 */
slobber(c)
	int c;
{

	shudclob = 0;
	switch (c) {

	case '\t':
		if (Putchar == listchar)
			return;
		break;

	default:
		return;

	case ' ':
	case 0:
		break;
	}
	if (OS)
		return;
	flush();
	putch(' ');
	if (BC)
		tputs(BC, 0, putch);
	else
		putch('\b');
}

/*
 * The output buffer is initialized with a useful error
 * message so we don't have to keep it in data space.
 */
static	char linb[66] = {
	'E', 'r', 'r', 'o', 'r', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e', ' ',
	'f', 'i', 'l', 'e', ' ', 'n', 'o', 't', ' ',
	'a', 'v', 'a', 'i', 'l', 'a', 'b', 'l', 'e', '\n', 0
};
static	char *linp = linb + 33;

/*
 * Phadnl records when we have already had a complete line ending with \n.
 * If another line starts without a flush, and the terminal suggests it,
 * we switch into -nl mode so that we can send lineffeeds to avoid
 * a lot of spacing.
 */
static	bool phadnl;

/*
 * Indirect to current definition of putchar.
 */
putchar(c)
	int c;
{

	(*Putchar)(c);
}

/*
 * Termchar routine for command mode.
 * Watch for possible switching to -nl mode.
 * Otherwise flush into next level of buffering when
 * small buffer fills or at a newline.
 */
termchar(c)
	int c;
{

	if (pfast == 0 && phadnl)
		pstart();
	if (c == '\n')
		phadnl = 1;
	else if (linp >= &linb[63])
		flush1();
	*linp++ = c;
	if (linp >= &linb[63]) {
		fgoto();
		flush1();
	}
}

flush()
{

	flush1();
	flush2();
}

/*
 * Flush from small line buffer into output buffer.
 * Work here is destroying motion into positions, and then
 * letting fgoto do the optimized motion.
 */
flush1()
{
	register char *lp;
	register short c;

	*linp = 0;
	lp = linb;
	while (*lp)
		switch (c = *lp++) {

		case '\r':
			destline += destcol / COLUMNS;
			destcol = 0;
			continue;

		case '\b':
			if (destcol)
				destcol--;
			continue;

		case ' ':
			destcol++;
			continue;

		case '\t':
			destcol += value(TABSTOP) - destcol % value(TABSTOP);
			continue;

		case '\n':
			destline += destcol / COLUMNS + 1;
			if (destcol != 0 && destcol % COLUMNS == 0)
				destline--;
			destcol = 0;
			continue;

		default:
			fgoto();
			for (;;) {
				if (AM == 0 && outcol == COLUMNS)
					fgoto();
				c &= TRIM;
				putch(c);
				if (c == '\b') {
					outcol--;
					destcol--;
				} else if (c >= ' ' && c != DELETE) {
					outcol++;
					destcol++;
					if (XN && outcol % COLUMNS == 0)
						putch('\n');
				}
				c = *lp++;
				if (c <= ' ')
					break;
			}
			--lp;
			continue;
		}
	linp = linb;
}

flush2()
{

	fgoto();
	flusho();
	pstop();
}

/*
 * Sync the position of the output cursor.
 * Most work here is rounding for terminal boundaries getting the
 * column position implied by wraparound or the lack thereof and
 * rolling up the screen to get destline on the screen.
 */
fgoto()
{
	register int l, c;

	if (destcol > COLUMNS - 1) {
		destline += destcol / COLUMNS;
		destcol %= COLUMNS;
	}
	if (outcol > COLUMNS - 1) {
		l = (outcol + 1) / COLUMNS;
		outline += l;
		outcol %= COLUMNS;
		if (AM == 0) {
			while (l > 0) {
				if (pfast)
#ifdef CRNL
					if (xCR)
						tputs(xCR, 0, putch);
					else
#endif
						putch('\r');
#ifdef CRNL
				if (xNL)
					tputs(xNL, 0, putch);
				else
#endif
					putch('\n');
				l--;
			}
			outcol = 0;
		}
		if (outline > LINES - 1) {
			destline -= outline - (LINES - 1);
			outline = LINES - 1;
		}
	}
	if (destline > LINES - 1) {
		l = destline;
		destline = LINES - 1;
		if (outline < LINES - 1) {
			c = destcol;
			if (pfast == 0 && (!CA || holdcm))
				destcol = 0;
			fgoto();
			destcol = c;
		}
		while (l > LINES - 1) {
			/*
			 * The following linefeed (or simulation thereof)
			 * is supposed to scroll up the screen, since we
			 * are on the bottom line.  We make the assumption
			 * that linefeed will scroll.  If ns is in the
			 * capability list this won't work.  We should
			 * probably have an sc capability but sf will
			 * generally take the place if it works.
			 */
#ifdef CRNL
			if (xNL /* && !XB */ && pfast)
				tputs(xNL, 0, putch);
			else
#endif
				putch('\n');
			l--;
			if (pfast == 0)
				outcol = 0;
		}
	}
	if (destline < outline && !(CA && !holdcm || UP != NOSTR))
		destline = outline;
	if (CA && !holdcm)
		if (plod(costCM) > 0)
			plod(0);
		else
			tputs(tgoto(CM, destcol, destline), 0, putch);
	else
		plod(0);
	outline = destline;
	outcol = destcol;
}

/*
 * Tab to column col by flushing and then setting destcol.
 * Used by "set all".
 */
tab(col)
	int col;
{

	flush1();
	destcol = col;
}

/*
 * Move (slowly) to destination.
 * Hard thing here is using home cursor on really deficient terminals.
 * Otherwise just use cursor motions, hacking use of tabs and overtabbing
 * and backspace.
 */

static int plodcnt, plodflg;

plodput(c)
{

	if (plodflg)
		plodcnt--;
	else
		putch(c);
}

plod(cnt)
{
	register int i, j, k;
	register int soutcol, soutline;

	plodcnt = plodflg = cnt;
	soutcol = outcol;
	soutline = outline;
	if (HO) {
		if (GT)
		i = (destcol / value(HARDTABS)) + (destcol % value(HARDTABS));
		else
			i = destcol;
	if (destcol >= outcol) {
		j = destcol / value(HARDTABS) - outcol / value(HARDTABS);
		if (GT && j)
			j += destcol % value(HARDTABS);
			else
				j = destcol - outcol;
	} else
			if (outcol - destcol <= i && (BS || BC))
				i = j = outcol - destcol;
			else
				j = i + 1;
		k = outline - destline;
		if (k < 0)
			k = -k;
		j += k;
		if (i + destline < j) {
			tputs(HO, 0, plodput);
			outcol = outline = 0;
		} else if (LL) {
			k = (LINES - 1) - destline;
			if (i + k + 2 < j) {
				tputs(LL, 0, plodput);
				outcol = 0;
				outline = LINES - 1;
			}
		}
	}
	if (GT)
	i = destcol % value(HARDTABS) + destcol / value(HARDTABS);
	else
		i = destcol;
/*
	if (BT && outcol > destcol && (j = (((outcol+7) & ~7) - destcol - 1) >> 3)) {
		j *= (k = strlen(BT));
		if ((k += (destcol&7)) > 4)
			j += 8 - (destcol&7);
		else
			j += k;
	} else
*/
		j = outcol - destcol;
	/*
	 * If we will later need a \n which will turn into a \r\n by
	 * the system or the terminal, then don't bother to try to \r.
	 */
	if ((NONL || !pfast) && outline < destline)
		goto dontcr;
	/*
	 * If the terminal will do a \r\n and there isn't room for it,
	 * then we can't afford a \r.
	 */
	if (NC && outline >= destline)
		goto dontcr;
	/*
	 * If it will be cheaper, or if we can't back up, then send
	 * a return preliminarily.
	 */
	if (j > i + 1 || outcol > destcol && !BS && !BC) {
#ifdef CRNL
		if (xCR)
			tputs(xCR, 0, plodput);
		else
#endif
			plodput('\r');
		if (NC) {
#ifdef CRNL
			if (xNL)
				tputs(xNL, 0, plodput);
			else
#endif
				plodput('\n');
			outline++;
		}
		outcol = 0;
	}
dontcr:
	while (outline < destline) {
		outline++;
#ifdef CRNL
			if (xNL)
				tputs(xNL, 0, plodput);
			else
#endif
			plodput('\n');
		if (plodcnt < 0)
			goto out;
		if (NONL || pfast == 0)
			outcol = 0;
	}
	if (BT)
		k = strlen(BT);
	while (outcol > destcol) {
		if (plodcnt < 0)
			goto out;
/*
		if (BT && !insmode && outcol - destcol > 4+k) {
			tputs(BT, 0, plodput);
			outcol--;
			outcol &= ~7;
			continue;
		}
*/
		outcol--;
		if (BC)
			tputs(BC, 0, plodput);
		else
			plodput('\b');
	}
	while (outline > destline) {
		outline--;
		tputs(UP, 0, plodput);
		if (plodcnt < 0)
			goto out;
	}
	if (GT && !insmode && destcol - outcol > 1) {
	for (;;) {
		i = (outcol / value(HARDTABS) + 1) * value(HARDTABS);
		if (i > destcol)
			break;
			if (TA)
				tputs(TA, 0, plodput);
			else
				plodput('\t');
			outcol = i;
		}
		if (destcol - outcol > 4 && i < COLUMNS && (BC || BS)) {
			if (TA)
				tputs(TA, 0, plodput);
			else
				plodput('\t');
			outcol = i;
			while (outcol > destcol) {
				outcol--;
				if (BC)
					tputs(BC, 0, plodput);
				else
					plodput('\b');
			}
		}
	}
	while (outcol < destcol) {
		/*
		 * move one char to the right.  We don't use ND space
		 * because it's better to just print the char we are
		 * moving over.  There are various exceptions, however.
		 * If !inopen, vtube contains garbage.  If the char is
		 * a null or a tab we want to print a space.  Other random
		 * chars we use space for instead, too.
		 */
		if (!inopen || vtube[outline]==NULL ||
			(i=vtube[outline][outcol]) < ' ')
			i = ' ';
		if (insmode && ND)
			tputs(ND, 0, plodput);
		else
			plodput(i);
		outcol++;
		if (plodcnt < 0)
			goto out;
	}
out:
	if (plodflg) {
		outcol = soutcol;
		outline = soutline;
	}
	return(plodcnt);
}

/*
 * An input line arrived.
 * Calculate new (approximate) screen line position.
 * Approximate because kill character echoes newline with
 * no feedback and also because of long input lines.
 */
noteinp()
{

	outline++;
	if (outline > LINES - 1)
		outline = LINES - 1;
	destline = outline;
	destcol = outcol = 0;
}

/*
 * Something weird just happened and we
 * lost track of whats happening out there.
 * Since we cant, in general, read where we are
 * we just reset to some known state.
 * On cursor addressible terminals setting to unknown
 * will force a cursor address soon.
 */
termreset()
{

	endim();
	if (TI)	/* otherwise it flushes anyway, and 'set tty=dumb' vomits */
		putpad(TI);	 /*adb change -- emit terminal initial sequence */
	destcol = 0;
	destline = LINES - 1;
	if (CA) {
		outcol = UKCOL;
		outline = UKCOL;
	} else {
		outcol = destcol;
		outline = destline;
	}
}

/*
 * Low level buffering, with the ability to drain
 * buffered output without printing it.
 */
char	*obp = obuf;

draino()
{

	obp = obuf;
}

flusho()
{

	if (obp != obuf) {
		write(1, obuf, obp - obuf);
		obp = obuf;
	}
}

putnl()
{

	putchar('\n');
}

putS(cp)
	char *cp;
{

	if (cp == NULL)
		return;
	while (*cp)
		putch(*cp++);
}


putch(c)
	int c;
{

	*obp++ = c & 0177;
	if (obp >= &obuf[sizeof obuf])
		flusho();
}

/*
 * Miscellaneous routines related to output.
 */

/*
 * Put with padding
 */
putpad(cp)
	char *cp;
{

	flush();
	tputs(cp, 0, putch);
}

/*
 * Set output through normal command mode routine.
 */
setoutt()
{

	Outchar = termchar;
}

/*
 * Printf (temporarily) in list mode.
 */
/*VARARGS2*/
lprintf(cp, dp)
	char *cp, *dp;
{
	register int (*P)();

	P = setlist(1);
	printf(cp, dp);
	Putchar = P;
}

/*
 * Newline + flush.
 */
putNFL()
{

	putnl();
	flush();
}

/*
 * Try to start -nl mode.
 */
pstart()
{

	if (NONL)
		return;
 	if (!value(OPTIMIZE))
		return;
	if (ruptible == 0 || pfast)
		return;
	fgoto();
	flusho();
	pfast = 1;
	normtty++;
#ifndef USG3TTY
	tty.sg_flags = normf & ~(ECHO|XTABS|CRMOD);
#else
	tty = normf;
	tty.c_oflag &= ~(ONLCR|TAB3);
	tty.c_lflag &= ~ECHO;
#endif
	sTTY(1);
}

/*
 * Stop -nl mode.
 */
pstop()
{

	if (inopen)
		return;
	phadnl = 0;
	linp = linb;
	draino();
	normal(normf);
	pfast &= ~1;
}

/*
 * Prep tty for open mode.
 */
ttymode
ostart()
{
	ttymode f;

	if (!intty)
		error("Open and visual must be used interactively");
	gTTY(1);
	normtty++;
#ifndef USG3TTY
	f = tty.sg_flags;
	tty.sg_flags = (normf &~ (ECHO|XTABS|CRMOD)) |
# ifdef CBREAK
							CBREAK;
# else
							RAW;
# endif
# ifdef TIOCGETC
	ttcharoff();
# endif
#else
	f = tty;
	tty = normf;
	tty.c_iflag &= ~ICRNL;
	tty.c_lflag &= ~(ECHO|ICANON);
	tty.c_oflag &= ~TAB3;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;
	ttcharoff();
#endif
	sTTY(1);
	putpad(VS);
	pfast |= 2;
	return (f);
}

#ifdef TIOCGETC
/*
 * Turn off start/stop chars if they aren't the default ^S/^Q.
 * This is so idiots who make esc their start/stop don't lose.
 * We always turn off quit since datamedias send ^\ for their
 * right arrow key.
 */
ttcharoff()
{
	nttyc.t_quitc = '\377';
	if (nttyc.t_startc != CTRL(q))
		nttyc.t_startc = '\377';
	if (nttyc.t_stopc != CTRL(s))
		nttyc.t_stopc = '\377';
}
#endif

#ifdef USG3TTY
ttcharoff()
{
	tty.c_cc[VQUIT] = '\377';
# ifdef VSTART
	/*
	 * The following is sample code if USG ever lets people change
	 * their start/stop chars.  As long as they can't we can't get
	 * into trouble so we just leave them alone.
	 */
	if (tty.c_cc[VSTART] != CTRL(q))
		tty.c_cc[VSTART] = '\377';
	if (tty.c_cc[VSTOP] != CTRL(s))
		tty.c_cc[VSTOP] = '\377';
# endif
}
#endif

/*
 * Stop open, restoring tty modes.
 */
ostop(f)
	ttymode f;
{

#ifndef USG3TTY
	pfast = (f & CRMOD) == 0;
#else
	pfast = (f.c_oflag & OCRNL) == 0;
#endif
	termreset(), fgoto(), flusho();
	normal(f);
	putpad(VE);
}

#ifndef CBREAK
/*
 * Into cooked mode for interruptibility.
 */
vcook()
{

	tty.sg_flags &= ~RAW;
	sTTY(1);
}

/*
 * Back into raw mode.
 */
vraw()
{

	tty.sg_flags |= RAW;
	sTTY(1);
}
#endif

/*
 * Restore flags to normal state f.
 */
normal(f)
	ttymode f;
{

	if (normtty > 0) {
		setty(f);
		normtty--;
	}
}

/*
 * Straight set of flags to state f.
 */
ttymode
setty(f)
	ttymode f;
{
#ifndef USG3TTY
	register int ot = tty.sg_flags;
#else
	ttymode ot;
	ot = tty;
#endif

#ifndef USG3TTY
# ifdef TIOCGETC
	if (f == normf)
		nttyc = ottyc;
	else
		ttcharoff();
# endif
	tty.sg_flags = f;
#else
	if (tty.c_lflag & ICANON)
		ttcharoff();
	tty = f;
#endif
	sTTY(1);
	return (ot);
}

gTTY(i)
	int i;
{

#ifndef USG3TTY
	ignore(gtty(i, &tty));
# ifdef TIOCGETC
	ioctl(i, TIOCGETC, &ottyc);
# endif
#else
	ioctl(i, TCGETA, &tty);
#endif
#ifdef TIOCGETC
	nttyc = ottyc;
#endif
}

/*
 * sTTY: set the tty modes on file descriptor i to be what's currently
 * in global "tty".  (Also use nttyc if needed.)
 */
sTTY(i)
	int i;
{

#ifndef USG3TTY
/*
 * Bug in USG tty driver, put out a DEL as a patch.
 */
# ifdef USG
	if (tty.sg_ospeed >= B1200)
		write(1, "\377", 1);
# endif
# ifdef TIOCSETN
	/* Don't flush typeahead if we don't have to. */
	ioctl(i, TIOCSETN, &tty);
# else
	/* We have to.  Too bad. */
	stty(i, &tty);
# endif
# ifdef TIOCGETC
	/*
	 * Update the other random chars while we're at it.
	 * Heaven help us if TIOCGETC is defined but TIOCSETC isn't!
	 */
	ioctl(i, TIOCSETC, &nttyc);
# endif
#else
	/* USG 3 very simple: just set everything */
	ioctl(i, TCSETAW, &tty);
#endif
}

/*
 * Print newline, or blank if in open/visual
 */
noonl()
{

	putchar(Outchar != termchar ? ' ' : '\n');
}
