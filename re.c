/*
 * re.c - compile regular expressions.
 */

/* 
 * Copyright (C) 1991-2011 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "awk.h"

static reg_syntax_t syn;
static void check_bracket_exp(char *s, size_t len);

/* make_regexp --- generate compiled regular expressions */

Regexp *
make_regexp(const char *s, size_t len, int ignorecase, int dfa, int canfatal)
{
	Regexp *rp;
	const char *rerr;
	const char *src = s;
	static char *buf = NULL;
	static size_t buflen;
	const char *end = s + len;
	char *dest;
	int c, c2;
	static short first = TRUE;
	static short no_dfa = FALSE;
	int has_anchor = FALSE;
	int may_have_range = 0;
	reg_syntax_t dfa_syn;

	/*
	 * The number of bytes in the current multibyte character.
	 * It is 0, when the current character is a singlebyte character.
	 */
	size_t is_multibyte = 0;
#ifdef MBS_SUPPORT
	mbstate_t mbs;

	if (gawk_mb_cur_max > 1)
		memset(&mbs, 0, sizeof(mbstate_t)); /* Initialize.  */
#endif

	if (first) {
		first = FALSE;
		/* for debugging and testing */
		no_dfa = (getenv("GAWK_NO_DFA") != NULL);
	}

	/* always check */
	check_bracket_exp((char *) s, len);

	/* Handle escaped characters first. */

	/*
	 * Build a copy of the string (in buf) with the
	 * escaped characters translated, and generate the regex
	 * from that. 
	 */
	if (buf == NULL) {
		emalloc(buf, char *, len + 2, "make_regexp");
		buflen = len;
	} else if (len > buflen) {
		erealloc(buf, char *, len + 2, "make_regexp");
		buflen = len;
	}
	dest = buf;

	while (src < end) {
#ifdef MBS_SUPPORT
		if (gawk_mb_cur_max > 1 && ! is_multibyte) {
			/* The previous byte is a singlebyte character, or last byte
			   of a multibyte character.  We check the next character.  */
			is_multibyte = mbrlen(src, end - src, &mbs);
			if (   (is_multibyte == 1)
			    || (is_multibyte == (size_t) -1)
			    || (is_multibyte == (size_t) -2
			    || (is_multibyte == 0))) {
				/* We treat it as a singlebyte character.  */
				is_multibyte = 0;
			}
		}
#endif

		/* We skip multibyte character, since it must not be a special
		   character.  */
		if ((gawk_mb_cur_max == 1 || ! is_multibyte) &&
		    (*src == '\\')) {
			c = *++src;
			switch (c) {
			case 'a':
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
			case 'v':
			case 'x':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				c2 = parse_escape(&src);
				if (c2 < 0)
					cant_happen();
				/*
				 * Unix awk treats octal (and hex?) chars
				 * literally in re's, so escape regexp
				 * metacharacters.
				 */
				if (do_traditional
				    && ! do_posix
				    && (isdigit(c) || c == 'x')
				    && strchr("()|*+?.^$\\[]", c2) != NULL)
					*dest++ = '\\';
				*dest++ = (char) c2;
				break;
			case '8':
			case '9':	/* a\9b not valid */
				*dest++ = c;
				src++;
				break;
			case 'y':	/* normally \b */
				/* gnu regex op */
				if (! do_traditional) {
					*dest++ = '\\';
					*dest++ = 'b';
					src++;
					break;
				}
				/* else, fall through */
			default:
				*dest++ = '\\';
				*dest++ = (char) c;
				src++;
				break;
			} /* switch */
		} else {
			c = *src;
			if (c == '^' || c == '$')
				has_anchor = TRUE;
			if (c == '[' || c == '-' || c == ']')
				may_have_range++;

			*dest++ = *src++;	/* not '\\' */
		}
		if (gawk_mb_cur_max > 1 && is_multibyte)
			is_multibyte--;
	} /* while */

	*dest = '\0';
	len = dest - buf;

	emalloc(rp, Regexp *, sizeof(*rp), "make_regexp");
	memset((char *) rp, 0, sizeof(*rp));
	rp->dfareg = NULL;
	rp->pat.allocated = 0;	/* regex will allocate the buffer */
	emalloc(rp->pat.fastmap, char *, 256, "make_regexp");

	/*
	 * Lo these many years ago, had I known what a P.I.T.A. IGNORECASE
	 * was going to turn out to be, I wouldn't have bothered with it.
	 *
	 * In the case where we have a multibyte character set, we have no
	 * choice but to use RE_ICASE, since the casetable is for single-byte
	 * character sets only.
	 *
	 * On the other hand, if we do have a single-byte character set,
	 * using the casetable should give  a performance improvement, since
	 * it's computed only once, not each time a regex is compiled.  We
	 * also think it's probably better for portability.  See the
	 * discussion by the definition of casetable[] in eval.c.
	 */

	ignorecase = !! ignorecase;	/* force to 1 or 0 */
	if (ignorecase) {
		if (gawk_mb_cur_max > 1) {
			syn |= RE_ICASE;
			rp->pat.translate = NULL;
		} else {
			syn &= ~RE_ICASE;
			rp->pat.translate = (RE_TRANSLATE_TYPE) casetable;
		}
	} else {
		rp->pat.translate = NULL;
		syn &= ~RE_ICASE;
	}

	dfa_syn = syn;
	if (ignorecase)
		dfa_syn |= RE_ICASE;
	dfasyntax(dfa_syn, ignorecase, '\n');
	re_set_syntax(syn);

	if ((rerr = re_compile_pattern(buf, len, &(rp->pat))) != NULL) {
		refree(rp);
		if (! canfatal) {
			/* rerr already gettextized inside regex routines */
			error("%s: /%s/", rerr, buf);
 			return NULL;
		}
		fatal("%s: /%s/", rerr, buf);
	}

	/* gack. this must be done *after* re_compile_pattern */
	rp->pat.newline_anchor = FALSE; /* don't get \n in middle of string */
	if (dfa && ! no_dfa) {
		rp->dfa = TRUE;
		rp->dfareg = dfaalloc();
		dfacomp(buf, len, rp->dfareg, TRUE);
	} else
		rp->dfa = FALSE;
	rp->has_anchor = has_anchor;
 
	return rp;
}

/* research --- do a regexp search. use dfa if possible */

int
research(Regexp *rp, char *str, int start,
	 size_t len, int flags)
{
	const char *ret = str;
	int try_backref;
	int need_start;
	int no_bol;
	int res;

	need_start = ((flags & RE_NEED_START) != 0);
	no_bol = ((flags & RE_NO_BOL) != 0);

	if (no_bol)
		rp->pat.not_bol = 1;

	/*
	 * Always do dfa search if can; if it fails, then even if
	 * need_start is true, we won't bother with the regex search.
	 *
	 * The dfa matcher doesn't have a no_bol flag, so don't bother
	 * trying it in that case.
	 *
	 * 7/2008: Skip the dfa matcher if need_start. The dfa matcher
	 * has bugs in certain multibyte cases and it's too difficult
	 * to try to special case things.
	 */
	if (rp->dfa && ! no_bol && ! need_start) {
		char save;
		int count = 0;
		/*
		 * dfa likes to stick a '\n' right after the matched
		 * text.  So we just save and restore the character.
		 */
		save = str[start+len];
		ret = dfaexec(rp->dfareg, str+start, str+start+len, TRUE,
					&count, &try_backref);
		str[start+len] = save;
	}

	if (ret) {
		if (need_start || rp->dfa == FALSE || try_backref) {
			/*
			 * Passing NULL as last arg speeds up search for cases
			 * where we don't need the start/end info.
			 */
			res = re_search(&(rp->pat), str, start+len,
				start, len, need_start ? &(rp->regs) : NULL);
		} else
			res = 1;
	} else
		res = -1;

	rp->pat.not_bol = 0;
	return res;
}

/* refree --- free up the dynamic memory used by a compiled regexp */

void
refree(Regexp *rp)
{
	if (rp == NULL)
		return; 
	rp->pat.translate = NULL;
	regfree(& rp->pat);
	if (rp->regs.start)
		free(rp->regs.start);
	if (rp->regs.end)
		free(rp->regs.end);
	if (rp->dfa) {
		dfafree(rp->dfareg);
		free(rp->dfareg);
	}
	efree(rp);
}

/* dfaerror --- print an error message for the dfa routines */

void
dfaerror(const char *s)
{
	fatal("%s", s);
}

/* re_update --- recompile a dynamic regexp */

Regexp *
re_update(NODE *t)
{
	NODE *t1;

	if ((t->re_flags & CASE) == IGNORECASE) {
		/* regex was compiled with settings matching IGNORECASE */
		if ((t->re_flags & CONSTANT) != 0) {
			/* it's a constant, so just return it as is */
			assert(t->type == Node_regex);
			return t->re_reg;
		}
		t1 = t->re_exp;
		if (t->re_text != NULL) {
			/* if contents haven't changed, just return it */
			if (cmp_nodes(t->re_text, t1) == 0)
				return t->re_reg;
			/* things changed, fall through to recompile */
			unref(t->re_text);
		}
		/* get fresh copy of the text of the regexp */
		t->re_text = dupnode(t1);
	}
	/* was compiled with different IGNORECASE or text changed */

	/* free old */
	if (t->re_reg != NULL)
		refree(t->re_reg);
	if (t->re_cnt > 0)
		t->re_cnt++;
	if (t->re_cnt > 10)
		t->re_cnt = 0;
	if (t->re_text == NULL || (t->re_flags & CASE) != IGNORECASE) {
		/* reset regexp text if needed */
		t1 = t->re_exp;
		unref(t->re_text);
		t->re_text = dupnode(t1);
	}
	/* compile it */
	t->re_reg = make_regexp(t->re_text->stptr, t->re_text->stlen,
				IGNORECASE, t->re_cnt, TRUE);

	/* clear case flag */
	t->re_flags &= ~CASE;
	/* set current value of case flag */
	t->re_flags |= IGNORECASE;
	return t->re_reg;
}

/* resetup --- choose what kind of regexps we match */

void
resetup()
{
	if (do_posix)
		syn = RE_SYNTAX_POSIX_AWK;	/* strict POSIX re's */
	else if (do_traditional)
		syn = RE_SYNTAX_AWK;		/* traditional Unix awk re's */
	else
		syn = RE_SYNTAX_GNU_AWK;	/* POSIX re's + GNU ops */

	/*
	 * Interval expressions are now on by default, as POSIX is
	 * wide-spread enough that people want it. The do_intervals
	 * variable remains for use with --traditional.
	 */
	if (do_intervals)
		syn |= RE_INTERVALS | RE_INVALID_INTERVAL_ORD;

	(void) re_set_syntax(syn);
	dfasyntax(syn, FALSE, '\n');
}

/* avoid_dfa --- return true if we should not use the DFA matcher */

int
avoid_dfa(NODE *re, char *str, size_t len)
{
	char *end;

	if (! re->re_reg->has_anchor)
		return FALSE;

	for (end = str + len; str < end; str++)
		if (*str == '\n')
			return TRUE;

	return FALSE;
}

/* reisstring --- return TRUE if the RE match is a simple string match */

int
reisstring(const char *text, size_t len, Regexp *re, const char *buf)
{
	static char metas[] = ".*+(){}[]|?^$\\";
	int i;
	int res;
	const char *matched;

	/* simple checking for has meta characters in re */
	for (i = 0; i < len; i++) {
		if (strchr(metas, text[i]) != NULL) {
			return FALSE;	/* give up early, can't be string match */
		}
	}

	/* make accessable to gdb */
	matched = &buf[RESTART(re, buf)];

	res = (memcmp(text, matched, len) == 0);

	return res;
}

/* remaybelong --- return TRUE if the RE contains * ? | + */

int
remaybelong(const char *text, size_t len)
{
	while (len--) {
		if (strchr("*+|?", *text++) != NULL) {
			return TRUE;
		}
	}

	return FALSE;
}

/* reflags2str --- make a regex flags value readable */

const char *
reflags2str(int flagval)
{
	static const struct flagtab values[] = {
		{ RE_BACKSLASH_ESCAPE_IN_LISTS, "RE_BACKSLASH_ESCAPE_IN_LISTS" },
		{ RE_BK_PLUS_QM, "RE_BK_PLUS_QM" },
		{ RE_CHAR_CLASSES, "RE_CHAR_CLASSES" },
		{ RE_CONTEXT_INDEP_ANCHORS, "RE_CONTEXT_INDEP_ANCHORS" },
		{ RE_CONTEXT_INDEP_OPS, "RE_CONTEXT_INDEP_OPS" },
		{ RE_CONTEXT_INVALID_OPS, "RE_CONTEXT_INVALID_OPS" },
		{ RE_DOT_NEWLINE, "RE_DOT_NEWLINE" },
		{ RE_DOT_NOT_NULL, "RE_DOT_NOT_NULL" },
		{ RE_HAT_LISTS_NOT_NEWLINE, "RE_HAT_LISTS_NOT_NEWLINE" },
		{ RE_INTERVALS, "RE_INTERVALS" },
		{ RE_LIMITED_OPS, "RE_LIMITED_OPS" },
		{ RE_NEWLINE_ALT, "RE_NEWLINE_ALT" },
		{ RE_NO_BK_BRACES, "RE_NO_BK_BRACES" },
		{ RE_NO_BK_PARENS, "RE_NO_BK_PARENS" },
		{ RE_NO_BK_REFS, "RE_NO_BK_REFS" },
		{ RE_NO_BK_VBAR, "RE_NO_BK_VBAR" },
		{ RE_NO_EMPTY_RANGES, "RE_NO_EMPTY_RANGES" },
		{ RE_UNMATCHED_RIGHT_PAREN_ORD, "RE_UNMATCHED_RIGHT_PAREN_ORD" },
		{ RE_NO_POSIX_BACKTRACKING, "RE_NO_POSIX_BACKTRACKING" },
		{ RE_NO_GNU_OPS, "RE_NO_GNU_OPS" },
		{ RE_INVALID_INTERVAL_ORD, "RE_INVALID_INTERVAL_ORD" },
		{ RE_ICASE, "RE_ICASE" },
		{ RE_CARET_ANCHORS_HERE, "RE_CARET_ANCHORS_HERE" },
		{ RE_CONTEXT_INVALID_DUP, "RE_CONTEXT_INVALID_DUP" },
		{ RE_NO_SUB, "RE_NO_SUB" },
		{ 0,	NULL },
	};

	if (flagval == RE_SYNTAX_EMACS) /* == 0 */
		return "RE_SYNTAX_EMACS";

	return genflags2str(flagval, values);
}

/*
 * dfawarn() is called by the dfa routines whenever a regex is compiled
 * must supply a dfawarn.
 */

void
dfawarn(const char *dfa_warning)
{
	/*
	 * This routine does nothing, since gawk does its own
	 * (better) check for bad [[:foo:]] syntax.
	 */
}

/* check_bracket_exp --- look for /[:space:]/ that should be /[[:space:]]/ */

static void
check_bracket_exp(char *s, size_t length)
{
	static struct reclass {
		const char *name;
		size_t len;
		short warned;
	} classes[] = {
		/*
		 * Ordered by what we hope is frequency,
		 * since it's linear searched.
		 */
		{ "[:alpha:]", 9, FALSE },
		{ "[:digit:]", 9, FALSE },
		{ "[:alnum:]", 9, FALSE },
		{ "[:upper:]", 9, FALSE },
		{ "[:lower:]", 9, FALSE },
		{ "[:space:]", 9, FALSE },
		{ "[:xdigit:]", 10, FALSE },
		{ "[:punct:]", 9, FALSE },
		{ "[:print:]", 9, FALSE },
		{ "[:graph:]", 9, FALSE },
		{ "[:cntrl:]", 9, FALSE },
		{ "[:blank:]", 9, FALSE },
		{ NULL, 0 }
	};
	int i;
	int found = FALSE;
	char save;
	char *sp, *sp2, *end;
	int len;
	int count = 0;

	if (length == 0)
		return;

	end = s + length;
	save = s[length];
	s[length] = '\0';
	sp = s;

again:
	sp = sp2 = memchr(sp, '[', (end - sp));
	if (sp == NULL)
		goto done;

	for (count++, sp++; *sp != '\0'; sp++) {
		static short range_warned = FALSE;

		if (*sp == '[')
			count++;
		else if (*sp == ']')
			count--;
		if (*sp == '-' && do_lint && ! range_warned && count == 1
		    && sp[-1] != '[' && sp[1] != ']'
		    && ! isdigit((unsigned char) sp[-1]) && ! isdigit((unsigned char) sp[1])
		    && ! (sp[-2] == '[' && sp[-1] == '^')) {
			range_warned = TRUE;
			warning(_("range of the form `[%c-%c]' is locale dependant"),
					sp[-1], sp[1]);
		}
		if (count == 0) {
			sp++;	/* skip past ']' */
			break;
		}
	}

	if (count > 0) {	/* bad regex, give up */
		goto done;
	}

	/* sp2 has start */

	for (i = 0; classes[i].name != NULL; i++) {
		if (classes[i].warned)
			continue;
		len = classes[i].len;
		if (   len == (sp - sp2)
		    && memcmp(sp2, classes[i].name, len) == 0) {
			found = TRUE;
			break;
		}
	}

	if (found && ! classes[i].warned) {
		warning(_("regexp component `%.*s' should probably be `[%.*s]'"),
				len, sp2, len, sp2);
		classes[i].warned = TRUE;
	}

	if (sp < end) {
		found = FALSE;
		goto again;
	}
done:
	s[length] = save;
}
