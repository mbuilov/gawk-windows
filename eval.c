/*
 * eval.c - gawk bytecode interpreter 
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2011 the Free Software Foundation, Inc.
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

extern void after_beginfile(IOBUF **curfile);
extern double pow(double x, double y);
extern double modf(double x, double *yp);
extern double fmod(double x, double y);
NODE **fcall_list;
long fcall_count;
int currule = 0;
IOBUF *curfile = NULL;		/* current data file */
int exiting = FALSE;

#ifdef DEBUGGING
extern int pre_execute(INSTRUCTION **);
extern void post_execute(INSTRUCTION *);
#else
#define r_interpret interpret
#endif

/*
 * Flag which executable this is; done here because eval.c is compiled
 * differently for each of them.
 */
enum exe_mode which_gawk =
#ifdef PROFILING
			   exe_profiling	/* pgawk */
#else
# ifdef DEBUGGING
			   exe_debugging	/* dgawk */     
# else
			   exe_normal		/* normal gawk */   
# endif
#endif
			   ;	/* which_gawk */

#if __GNUC__ < 2
NODE *_t;		/* used as a temporary in macros */
#endif
int OFSlen;
int ORSlen;
int OFMTidx;
int CONVFMTidx;

/* Profiling stuff */
#ifdef PROFILING
#define INCREMENT(n)	n++
#else
#define INCREMENT(n)	/* nothing */
#endif

/* This rather ugly macro is for VMS C */
#ifdef C
#undef C
#endif
#define C(c) ((char)c)  
/*
 * This table is used by the regexp routines to do case independant
 * matching. Basically, every ascii character maps to itself, except
 * uppercase letters map to lower case ones. This table has 256
 * entries, for ISO 8859-1. Note also that if the system this
 * is compiled on doesn't use 7-bit ascii, casetable[] should not be
 * defined to the linker, so gawk should not load.
 *
 * Do NOT make this array static, it is used in several spots, not
 * just in this file.
 *
 * 6/2004:
 * This table is also used for IGNORECASE for == and !=, and index().
 * Although with GLIBC, we could use tolower() everywhere and RE_ICASE
 * for the regex matcher, precomputing this table once gives us a
 * performance improvement.  I also think it's better for portability
 * to non-GLIBC systems.  All the world is not (yet :-) GNU/Linux.
 */
#if 'a' == 97	/* it's ascii */
char casetable[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	/* ' '     '!'     '"'     '#'     '$'     '%'     '&'     ''' */
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	/* '('     ')'     '*'     '+'     ','     '-'     '.'     '/' */
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	/* '0'     '1'     '2'     '3'     '4'     '5'     '6'     '7' */
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	/* '8'     '9'     ':'     ';'     '<'     '='     '>'     '?' */
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	/* '@'     'A'     'B'     'C'     'D'     'E'     'F'     'G' */
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	/* 'H'     'I'     'J'     'K'     'L'     'M'     'N'     'O' */
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	/* 'P'     'Q'     'R'     'S'     'T'     'U'     'V'     'W' */
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	/* 'X'     'Y'     'Z'     '['     '\'     ']'     '^'     '_' */
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	/* '`'     'a'     'b'     'c'     'd'     'e'     'f'     'g' */
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	/* 'h'     'i'     'j'     'k'     'l'     'm'     'n'     'o' */
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	/* 'p'     'q'     'r'     's'     't'     'u'     'v'     'w' */
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	/* 'x'     'y'     'z'     '{'     '|'     '}'     '~' */
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',

	/* Latin 1: */
	C('\200'), C('\201'), C('\202'), C('\203'), C('\204'), C('\205'), C('\206'), C('\207'),
	C('\210'), C('\211'), C('\212'), C('\213'), C('\214'), C('\215'), C('\216'), C('\217'),
	C('\220'), C('\221'), C('\222'), C('\223'), C('\224'), C('\225'), C('\226'), C('\227'),
	C('\230'), C('\231'), C('\232'), C('\233'), C('\234'), C('\235'), C('\236'), C('\237'),
	C('\240'), C('\241'), C('\242'), C('\243'), C('\244'), C('\245'), C('\246'), C('\247'),
	C('\250'), C('\251'), C('\252'), C('\253'), C('\254'), C('\255'), C('\256'), C('\257'),
	C('\260'), C('\261'), C('\262'), C('\263'), C('\264'), C('\265'), C('\266'), C('\267'),
	C('\270'), C('\271'), C('\272'), C('\273'), C('\274'), C('\275'), C('\276'), C('\277'),
	C('\340'), C('\341'), C('\342'), C('\343'), C('\344'), C('\345'), C('\346'), C('\347'),
	C('\350'), C('\351'), C('\352'), C('\353'), C('\354'), C('\355'), C('\356'), C('\357'),
	C('\360'), C('\361'), C('\362'), C('\363'), C('\364'), C('\365'), C('\366'), C('\327'),
	C('\370'), C('\371'), C('\372'), C('\373'), C('\374'), C('\375'), C('\376'), C('\337'),
	C('\340'), C('\341'), C('\342'), C('\343'), C('\344'), C('\345'), C('\346'), C('\347'),
	C('\350'), C('\351'), C('\352'), C('\353'), C('\354'), C('\355'), C('\356'), C('\357'),
	C('\360'), C('\361'), C('\362'), C('\363'), C('\364'), C('\365'), C('\366'), C('\367'),
	C('\370'), C('\371'), C('\372'), C('\373'), C('\374'), C('\375'), C('\376'), C('\377'),
};
#elif 'a' == 0x81 /* it's EBCDIC */
char casetable[] = {
 /*00  NU    SH    SX    EX    PF    HT    LC    DL */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
 /*08              SM    VT    FF    CR    SO    SI */
      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 /*10  DE    D1    D2    TM    RS    NL    BS    IL */
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
 /*18  CN    EM    CC    C1    FS    GS    RS    US */
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
 /*20  DS    SS    FS          BP    LF    EB    EC */
      0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
 /*28              SM    C2    EQ    AK    BL       */
      0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
 /*30              SY          PN    RS    UC    ET */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
 /*38                    C3    D4    NK          SU */
      0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
 /*40  SP                                           */
      0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
 /*48             CENT    .     <     (     +     | */
      0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
 /*50   &                                           */
      0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
 /*58               !     $     *     )     ;     ^ */
      0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
 /*60   -     /                                     */
      0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
 /*68               |     ,     %     _     >     ? */
      0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
 /*70                                               */
      0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
 /*78         `     :     #     @     '     =     " */
      0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
 /*80         a     b     c     d     e     f     g */
      0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
 /*88   h     i           {                         */
      0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
 /*90         j     k     l     m     n     o     p */
      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
 /*98   q     r           }                         */
      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
 /*A0         ~     s     t     u     v     w     x */
      0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
 /*A8   y     z                       [             */
      0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
 /*B0                                               */
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
 /*B8                                 ]             */
      0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
 /*C0   {     A     B     C     D     E     F     G */
      0xC0, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
 /*C8   H     I                                     */
      0x88, 0x89, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
 /*D0   }     J     K     L     M     N     O     P */
      0xD0, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
 /*D8   Q     R                                     */
      0x98, 0x99, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
 /*E0   \           S     T     U     V     W     X */
      0xE0, 0xE1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
 /*E8   Y     Z                                     */
      0xA8, 0xA9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
 /*F0   0     1     2     3     4     5     6     7 */
      0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
 /*F8   8     9                                     */
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};
#else
#include "You lose. You will need a translation table for your character set."
#endif

#undef C

/* load_casetable --- for a non-ASCII locale, redo the table */

void
load_casetable(void)
{
#if defined(LC_CTYPE)
	int i;
	char *cp;
	static int loaded = FALSE;

	if (loaded || do_traditional)
		return;

	loaded = TRUE;
	cp = setlocale(LC_CTYPE, NULL);

	/* this is not per standard, but it's pretty safe */
	if (cp == NULL || strcmp(cp, "C") == 0 || strcmp(cp, "POSIX") == 0)
		return;

#ifndef ZOS_USS
	for (i = 0200; i <= 0377; i++) {
		if (isalpha(i) && islower(i) && i != toupper(i))
			casetable[i] = toupper(i);
	}
#endif
#endif
}

/*
 * This table maps node types to strings for debugging.
 * KEEP IN SYNC WITH awk.h!!!!
 */

static const char *const nodetypes[] = {
	"Node_illegal",
	"Node_val",
	"Node_regex",
	"Node_dynregex",
	"Node_var",
	"Node_var_array",
	"Node_var_new",
	"Node_param_list",
	"Node_func",
	"Node_hashnode",
	"Node_ahash",
	"Node_array_ref",
	"Node_arrayfor",
	"Node_frame",
	"Node_instruction",
	"Node_final --- this should never appear",
	NULL
};


/*
 * This table maps Op codes to strings.
 * KEEP IN SYNC WITH awk.h!!!!
 */

static struct optypetab {
	char *desc;
	char *operator;
} optypes[] = {
	{ "Op_illegal", NULL },
	{ "Op_times", " * " },
	{ "Op_times_i", " * " },
	{ "Op_quotient", " / " },
	{ "Op_quotient_i", " / " },
	{ "Op_mod", " % " },
	{ "Op_mod_i", " % " },
	{ "Op_plus", " + " },
	{ "Op_plus_i", " + " },
	{ "Op_minus", " - " },
	{ "Op_minus_i", " - " },
	{ "Op_exp", " ^ " },
	{ "Op_exp_i", " ^ " },
	{ "Op_concat", " " },
	{ "Op_line_range", NULL },
	{ "Op_cond_pair", ", " },
	{ "Op_subscript", "[]" },
	{ "Op_sub_array", "[]" },
	{ "Op_preincrement", "++" },
	{ "Op_predecrement", "--" },
	{ "Op_postincrement", "++" },
	{ "Op_postdecrement", "--" },
	{ "Op_unary_minus", "-" },
	{ "Op_field_spec", "$" },
	{ "Op_not", "! " },
	{ "Op_assign", " = " },
	{ "Op_store_var", " = " },
	{ "Op_store_sub", " = " },
	{ "Op_store_field", " = " },
	{ "Op_assign_times", " *= " },
	{ "Op_assign_quotient", " /= " },
	{ "Op_assign_mod", " %= " },
	{ "Op_assign_plus", " += " },
	{ "Op_assign_minus", " -= " },
	{ "Op_assign_exp", " ^= " },
	{ "Op_assign_concat", " " },
	{ "Op_and", " && " },
	{ "Op_and_final", NULL },
	{ "Op_or", " || " },
	{ "Op_or_final", NULL },
	{ "Op_equal", " == " },
	{ "Op_notequal", " != " },
	{ "Op_less", " < " },
	{ "Op_greater", " > " },
	{ "Op_leq", " <= " },
	{ "Op_geq", " >= " },
	{ "Op_match", " ~ " },
	{ "Op_match_rec", NULL },
	{ "Op_nomatch", " !~ " },
	{ "Op_rule", NULL }, 
	{ "Op_K_case", "case" },
	{ "Op_K_default", "default" },
	{ "Op_K_break", "break" },
	{ "Op_K_continue", "continue" },
	{ "Op_K_print", "print" },
	{ "Op_K_print_rec", "print" },
	{ "Op_K_printf", "printf" },
	{ "Op_K_next", "next" },
	{ "Op_K_exit", "exit" },
	{ "Op_K_return", "return" },
	{ "Op_K_delete", "delete" },
	{ "Op_K_delete_loop", NULL },
	{ "Op_K_getline_redir", "getline" },
	{ "Op_K_getline", "getline" },
	{ "Op_K_nextfile", "nextfile" },
	{ "Op_builtin", NULL },
	{ "Op_in_array", " in " },
	{ "Op_func_call", NULL },
	{ "Op_indirect_func_call", NULL },
	{ "Op_push", NULL },
	{ "Op_push_arg", NULL },
	{ "Op_push_i", NULL },
	{ "Op_push_re", NULL },
	{ "Op_push_array", NULL },
	{ "Op_push_param", NULL },
	{ "Op_push_lhs", NULL },
	{ "Op_subscript_lhs", "[]" },
	{ "Op_field_spec_lhs", "$" },
	{ "Op_no_op", NULL },
	{ "Op_pop", NULL },
	{ "Op_jmp", NULL },
	{ "Op_jmp_true", NULL },
	{ "Op_jmp_false", NULL },
	{ "Op_get_record", NULL },
	{ "Op_newfile", NULL },
	{ "Op_arrayfor_init", NULL },
	{ "Op_arrayfor_incr", NULL },
	{ "Op_arrayfor_final", NULL },
	{ "Op_var_update", NULL },
	{ "Op_var_assign", NULL },
	{ "Op_field_assign", NULL },
	{ "Op_after_beginfile", NULL },
	{ "Op_after_endfile", NULL },
	{ "Op_ext_func", NULL },
	{ "Op_func", NULL },
	{ "Op_exec_count", NULL },
	{ "Op_breakpoint", NULL },
	{ "Op_lint", NULL },
	{ "Op_atexit", NULL },
	{ "Op_stop", NULL },
	{ "Op_token", NULL },
	{ "Op_symbol", NULL },
	{ "Op_list", NULL },
	{ "Op_K_do", "do" },
	{ "Op_K_for", "for" },
	{ "Op_K_arrayfor", "for" },
	{ "Op_K_while", "while" },
	{ "Op_K_switch", "switch" },
	{ "Op_K_if", "if" },
	{ "Op_K_else", "else" },
	{ "Op_K_function", "function" },
	{ "Op_cond_exp", NULL },
	{ "Op_final --- this should never appear", NULL },
	{ NULL, NULL },
};

/* nodetype2str --- convert a node type into a printable value */

const char *
nodetype2str(NODETYPE type)
{
	static char buf[40];

	if (type >= Node_illegal && type <= Node_final)
		return nodetypes[(int) type];

	sprintf(buf, _("unknown nodetype %d"), (int) type);
	return buf;
}

/* opcode2str --- convert a opcode type into a printable value */

const char *
opcode2str(OPCODE op)
{
	if (op >= Op_illegal && op < Op_final)
		return optypes[(int) op].desc;
	fatal(_("unknown opcode %d"), (int) op);
	return NULL;
}

const char *
op2str(OPCODE op)
{
	if (op >= Op_illegal && op < Op_final) {
		if (optypes[(int) op].operator != NULL)
			return optypes[(int) op].operator;
		else
			fatal(_("opcode %s not an operator or keyword"),
					optypes[(int) op].desc);
	} else
		fatal(_("unknown opcode %d"), (int) op);
	return NULL;
}


/* flags2str --- make a flags value readable */

const char *
flags2str(int flagval)
{
	static const struct flagtab values[] = {
		{ MALLOC, "MALLOC" },
		{ PERM, "PERM" },
		{ STRING, "STRING" },
		{ STRCUR, "STRCUR" },
		{ NUMCUR, "NUMCUR" },
		{ NUMBER, "NUMBER" },
		{ MAYBE_NUM, "MAYBE_NUM" },
		{ ARRAYMAXED, "ARRAYMAXED" },
		{ FUNC, "FUNC" },
		{ FIELD, "FIELD" },
		{ INTLSTR, "INTLSTR" },
		{ NUMIND, "NUMIND" },
#ifdef WSTRCUR
		{ WSTRCUR, "WSTRCUR" },
#endif
		{ 0,	NULL },
	};

	return genflags2str(flagval, values);
}

/* genflags2str --- general routine to convert a flag value to a string */

const char *
genflags2str(int flagval, const struct flagtab *tab)
{
	static char buffer[BUFSIZ];
	char *sp;
	int i, space_left, space_needed;

	sp = buffer;
	space_left = BUFSIZ;
	for (i = 0; tab[i].name != NULL; i++) {
		if ((flagval & tab[i].val) != 0) {
			/*
			 * note the trick, we want 1 or 0 for whether we need
			 * the '|' character.
			 */
			space_needed = (strlen(tab[i].name) + (sp != buffer));
			if (space_left < space_needed)
				fatal(_("buffer overflow in genflags2str"));

			if (sp != buffer) {
				*sp++ = '|';
				space_left--;
			}
			strcpy(sp, tab[i].name);
			/* note ordering! */
			space_left -= strlen(sp);
			sp += strlen(sp);
		}
	}

	return buffer;
}

/* posix_compare --- compare strings using strcoll */

static int
posix_compare(NODE *s1, NODE *s2)
{
	int ret = 0;
	char save1, save2;
	size_t l = 0;

	save1 = s1->stptr[s1->stlen];
	s1->stptr[s1->stlen] = '\0';

	save2 = s2->stptr[s2->stlen];
	s2->stptr[s2->stlen] = '\0';

	if (gawk_mb_cur_max == 1) {
		if (strlen(s1->stptr) == s1->stlen && strlen(s2->stptr) == s2->stlen)
			ret = strcoll(s1->stptr, s2->stptr);
		else {
			char b1[2], b2[2];
			char *p1, *p2;
			size_t i;

			if (s1->stlen < s2->stlen)
				l = s1->stlen;
			else
				l = s2->stlen;

			b1[1] = b2[1] = '\0';
			for (i = ret = 0, p1 = s1->stptr, p2 = s2->stptr;
			     ret == 0 && i < l;
			     p1++, p2++) {
				b1[0] = *p1;
				b2[0] = *p2;
				ret = strcoll(b1, b2);
			}
		}
		/*
		 * Either worked through the strings or ret != 0.
		 * In either case, ret will be the right thing to return.
		 */
	}
#ifdef MBS_SUPPORT
	else {
		/* Similar logic, using wide characters */
		(void) force_wstring(s1);
		(void) force_wstring(s2);

		if (wcslen(s1->wstptr) == s1->wstlen && wcslen(s2->wstptr) == s2->wstlen)
			ret = wcscoll(s1->wstptr, s2->wstptr);
		else {
			wchar_t b1[2], b2[2];
			wchar_t *p1, *p2;
			size_t i;

			if (s1->wstlen < s2->wstlen)
				l = s1->wstlen;
			else
				l = s2->wstlen;

			b1[1] = b2[1] = L'\0';
			for (i = ret = 0, p1 = s1->wstptr, p2 = s2->wstptr;
			     ret == 0 && i < l;
			     p1++, p2++) {
				b1[0] = *p1;
				b2[0] = *p2;
				ret = wcscoll(b1, b2);
			}
		}
		/*
		 * Either worked through the strings or ret != 0.
		 * In either case, ret will be the right thing to return.
		 */
	}
#endif

	s1->stptr[s1->stlen] = save1;
	s2->stptr[s2->stlen] = save2;
	return ret;
}


/* cmp_nodes --- compare two nodes, returning negative, 0, positive */

int
cmp_nodes(NODE *t1, NODE *t2)
{
	int ret = 0;
	size_t len1, len2;
	int l, ldiff;

	if (t1 == t2)
		return 0;

	if (t1->flags & MAYBE_NUM)
		(void) force_number(t1);
	if (t2->flags & MAYBE_NUM)
		(void) force_number(t2);
	if ((t1->flags & NUMBER) && (t2->flags & NUMBER)) {
		if (t1->numbr == t2->numbr)
			ret = 0;
		/* don't subtract, in case one or both are infinite */
		else if (t1->numbr < t2->numbr)
			ret = -1;
		else
			ret = 1;
		return ret;
	}

	(void) force_string(t1);
	(void) force_string(t2);
	len1 = t1->stlen;
	len2 = t2->stlen;
	ldiff = len1 - len2;
	if (len1 == 0 || len2 == 0)
		return ldiff;

	if (do_posix)
		return posix_compare(t1, t2);

	l = (ldiff <= 0 ? len1 : len2);
	if (IGNORECASE) {
		const unsigned char *cp1 = (const unsigned char *) t1->stptr;
		const unsigned char *cp2 = (const unsigned char *) t2->stptr;

#ifdef MBS_SUPPORT
		if (gawk_mb_cur_max > 1) {
			ret = strncasecmpmbs((const unsigned char *) cp1,
					     (const unsigned char *) cp2, l);
		} else
#endif
		/* Could use tolower() here; see discussion above. */
		for (ret = 0; l-- > 0 && ret == 0; cp1++, cp2++)
			ret = casetable[*cp1] - casetable[*cp2];
	} else
		ret = memcmp(t1->stptr, t2->stptr, l);
	return (ret == 0 ? ldiff : ret);
}


#if defined(PROFILING) || defined(DEBUGGING)
static void
push_frame(NODE *f)
{
	static long max_fcall;

	/* NB: frame numbering scheme as in GDB. frame_ptr => frame #0. */

	fcall_count++;
	if (fcall_list == NULL) {
		max_fcall = 10;
		emalloc(fcall_list, NODE **, (max_fcall + 1) * sizeof(NODE *), "push_frame");
	} else if (fcall_count == max_fcall) {
		max_fcall *= 2;
		erealloc(fcall_list, NODE **, (max_fcall + 1) * sizeof(NODE *), "push_frame");
	}

	if (fcall_count > 1)
		memmove(fcall_list + 2, fcall_list + 1, (fcall_count - 1) * sizeof(NODE *)); 
	fcall_list[1] = f;
}

static void
pop_frame()
{
#ifdef DEBUGGING
	extern void frame_popped();
#endif
	if (fcall_count > 1)
		memmove(fcall_list + 1, fcall_list + 2, (fcall_count - 1) * sizeof(NODE *)); 
	fcall_count--;
	assert(fcall_count >= 0);
#ifdef DEBUGGING
	frame_popped();
#endif
}
#else	/* not PROFILING or DEBUGGING */
#define push_frame(p)	/* nothing */
#define pop_frame()		/* nothing */
#endif


#ifdef PROFILING

/* dump_fcall_stack --- print a backtrace of the awk function calls */

void
dump_fcall_stack(FILE *fp)
{

	NODE *f, *func;
	long i = 0;

	if (fcall_count == 0)
		return;
	fprintf(fp, _("\n\t# Function Call Stack:\n\n"));

	/* current frame */
	func = frame_ptr->func_node;
	fprintf(fp, "\t# %3ld. %s\n", i, func->lnode->param);

	/* outer frames except main */
	for (i = 1; i < fcall_count; i++) {
		f = fcall_list[i];
		func = f->func_node;
		fprintf(fp, "\t# %3ld. %s\n", i, func->lnode->param);
	}

	fprintf(fp, "\t# %3ld. -- main --\n", fcall_count);
}

#endif /* PROFILING */

/* set_IGNORECASE --- update IGNORECASE as appropriate */

void
set_IGNORECASE()
{
	static short warned = FALSE;

	if ((do_lint || do_traditional) && ! warned) {
		warned = TRUE;
		lintwarn(_("`IGNORECASE' is a gawk extension"));
	}
	load_casetable();
	if (do_traditional)
		IGNORECASE = FALSE;
	else if ((IGNORECASE_node->var_value->flags & (STRING|STRCUR)) != 0) {
		if ((IGNORECASE_node->var_value->flags & MAYBE_NUM) == 0)
			IGNORECASE = (force_string(IGNORECASE_node->var_value)->stlen > 0);
		else
			IGNORECASE = (force_number(IGNORECASE_node->var_value) != 0.0);
	} else if ((IGNORECASE_node->var_value->flags & (NUMCUR|NUMBER)) != 0)
		IGNORECASE = (force_number(IGNORECASE_node->var_value) != 0.0);
	else
		IGNORECASE = FALSE;		/* shouldn't happen */
                  
	set_RS();	/* set_RS() calls set_FS() if need be, for us */
}

/* set_BINMODE --- set translation mode (OS/2, DOS, others) */

void
set_BINMODE()
{
	static short warned = FALSE;
	char *p;
	NODE *v;

	if ((do_lint || do_traditional) && ! warned) {
		warned = TRUE;
		lintwarn(_("`BINMODE' is a gawk extension"));
	}
	if (do_traditional)
		BINMODE = 0;
	else if ((BINMODE_node->var_value->flags & NUMBER) != 0) {
		BINMODE = (int) force_number(BINMODE_node->var_value);
		/* Make sure the value is rational. */
		if (BINMODE < 0)
			BINMODE = 0;
		else if (BINMODE > 3)
			BINMODE = 3;
	}
	else if ((BINMODE_node->var_value->flags & STRING) != 0) {
		v = BINMODE_node->var_value;
		p = v->stptr;

		/*
		 * Allow only one of the following:
		 * "0", "1", "2", "3",
		 * "r", "w", "rw", "wr"
		 * ANYTHING ELSE goes to 3. So there.
		 */
		switch (v->stlen) {
		case 1:
			switch (p[0]) {
			case '0':
			case '1':
			case '2':
			case '3':
				BINMODE = p[0] - '0';
				break;
			case 'r':
				BINMODE = 1;
				break;
			case 'w':
				BINMODE = 2;
				break;
			default:
				BINMODE = 3;
				goto bad_value;
				break;
			}
			break;
		case 2:
			switch (p[0]) {
			case 'r':
				BINMODE = 3;
				if (p[1] != 'w')
					goto bad_value;
				break;
			case 'w':
				BINMODE = 3;
				if (p[1] != 'r')
					goto bad_value;
				break;
			break;
		default:
	bad_value:
			lintwarn(_("BINMODE value `%s' is invalid, treated as 3"), p);
			break;
			}
		}
	}
	else
		BINMODE = 3;		/* shouldn't happen */
}

/* set_OFS --- update OFS related variables when OFS assigned to */

void
set_OFS()
{
	OFS = force_string(OFS_node->var_value)->stptr;
	OFSlen = OFS_node->var_value->stlen;
	OFS[OFSlen] = '\0';
}

/* set_ORS --- update ORS related variables when ORS assigned to */

void
set_ORS()
{
	ORS = force_string(ORS_node->var_value)->stptr;
	ORSlen = ORS_node->var_value->stlen;
	ORS[ORSlen] = '\0';
}

/* fmt_ok --- is the conversion format a valid one? */

NODE **fmt_list = NULL;
static int fmt_ok(NODE *n);
static int fmt_index(NODE *n);

static int
fmt_ok(NODE *n)
{
	NODE *tmp = force_string(n);
	const char *p = tmp->stptr;
#if ! defined(PRINTF_HAS_F_FORMAT) || PRINTF_HAS_F_FORMAT != 1
	static const char float_formats[] = "efgEG";
#else
	static const char float_formats[] = "efgEFG";
#endif
#if defined(HAVE_LOCALE_H)
	static const char flags[] = " +-#'";
#else
	static const char flags[] = " +-#";
#endif

	if (*p++ != '%')
		return 0;
	while (*p && strchr(flags, *p) != NULL)	/* flags */
		p++;
	while (*p && isdigit((unsigned char) *p))	/* width - %*.*g is NOT allowed */
		p++;
	if (*p == '\0' || (*p != '.' && ! isdigit((unsigned char) *p)))
		return 0;
	if (*p == '.')
		p++;
	while (*p && isdigit((unsigned char) *p))	/* precision */
		p++;
	if (*p == '\0' || strchr(float_formats, *p) == NULL)
		return 0;
	if (*++p != '\0')
		return 0;
	return 1;
}

/* fmt_index --- track values of OFMT and CONVFMT to keep semantics correct */

static int
fmt_index(NODE *n)
{
	int ix = 0;
	static int fmt_num = 4;
	static int fmt_hiwater = 0;

	if (fmt_list == NULL)
		emalloc(fmt_list, NODE **, fmt_num*sizeof(*fmt_list), "fmt_index");
	(void) force_string(n);
	while (ix < fmt_hiwater) {
		if (cmp_nodes(fmt_list[ix], n) == 0)
			return ix;
		ix++;
	}
	/* not found */
	n->stptr[n->stlen] = '\0';
	if (do_lint && ! fmt_ok(n))
		lintwarn(_("bad `%sFMT' specification `%s'"),
			    n == CONVFMT_node->var_value ? "CONV"
			  : n == OFMT_node->var_value ? "O"
			  : "", n->stptr);

	if (fmt_hiwater >= fmt_num) {
		fmt_num *= 2;
		erealloc(fmt_list, NODE **, fmt_num * sizeof(*fmt_list), "fmt_index");
	}
	fmt_list[fmt_hiwater] = dupnode(n);
	return fmt_hiwater++;
}

/* set_OFMT --- track OFMT correctly */

void
set_OFMT()
{
	OFMTidx = fmt_index(OFMT_node->var_value);
	OFMT = fmt_list[OFMTidx]->stptr;
}

/* set_CONVFMT --- track CONVFMT correctly */

void
set_CONVFMT()
{
	CONVFMTidx = fmt_index(CONVFMT_node->var_value);
	CONVFMT = fmt_list[CONVFMTidx]->stptr;
}

/* set_LINT --- update LINT as appropriate */

void
set_LINT()
{
#ifndef NO_LINT
	int old_lint = do_lint;

	if ((LINT_node->var_value->flags & (STRING|STRCUR)) != 0) {
		if ((LINT_node->var_value->flags & MAYBE_NUM) == 0) {
			const char *lintval;
			size_t lintlen;

			do_lint = (force_string(LINT_node->var_value)->stlen > 0);
			lintval = LINT_node->var_value->stptr;
			lintlen = LINT_node->var_value->stlen;
			if (do_lint) {
				do_lint = LINT_ALL;
				if (lintlen == 5 && strncmp(lintval, "fatal", 5) == 0)
					lintfunc = r_fatal;
				else if (lintlen == 7 && strncmp(lintval, "invalid", 7) == 0)
					do_lint = LINT_INVALID;
				else
					lintfunc = warning;
			} else
				lintfunc = warning;
		} else {
			if (force_number(LINT_node->var_value) != 0.0)
				do_lint = LINT_ALL;
			else
				do_lint = FALSE;
			lintfunc = warning;
		}
	} else if ((LINT_node->var_value->flags & (NUMCUR|NUMBER)) != 0) {
		if (force_number(LINT_node->var_value) != 0.0)
			do_lint = LINT_ALL;
		else
			do_lint = FALSE;
		lintfunc = warning;
	} else
		do_lint = FALSE;		/* shouldn't happen */

	if (! do_lint)
		lintfunc = warning;

	/* explicitly use warning() here, in case lintfunc == r_fatal */
	if (old_lint != do_lint && old_lint && do_lint == FALSE)
		warning(_("turning off `--lint' due to assignment to `LINT'"));
#endif /* ! NO_LINT */
}

/* set_TEXTDOMAIN --- update TEXTDOMAIN variable when TEXTDOMAIN assigned to */

void
set_TEXTDOMAIN()
{
	int len;

	TEXTDOMAIN = force_string(TEXTDOMAIN_node->var_value)->stptr;
	len = TEXTDOMAIN_node->var_value->stlen;
	TEXTDOMAIN[len] = '\0';
	/*
	 * Note: don't call textdomain(); this value is for
	 * the awk program, not for gawk itself.
	 */
}

/* update_ERRNO_saved --- update the value of ERRNO based on argument */

void
update_ERRNO_saved(int errcode)
{
	char *cp;

	if (errcode) {
		cp = strerror(errcode);
		cp = gettext(cp);
	} else
		cp = "";
	unref(ERRNO_node->var_value);
	ERRNO_node->var_value = make_string(cp, strlen(cp));
}

/* update_ERRNO --- update the value of ERRNO based on errno */

void
update_ERRNO()
{
	update_ERRNO_saved(errno);
}

/* update_NR --- update the value of NR */

void
update_NR()
{
	if (NR_node->var_value->numbr != NR) {
		unref(NR_node->var_value);
		NR_node->var_value = make_number((AWKNUM) NR);
	}
}

/* update_NF --- update the value of NF */

void
update_NF()
{
	if (NF == -1 || NF_node->var_value->numbr != NF) {
		if (NF == -1)
			(void) get_field(UNLIMITED - 1, NULL); /* parse record */
		unref(NF_node->var_value);
		NF_node->var_value = make_number((AWKNUM) NF);
	}
}

/* update_FNR --- update the value of FNR */

void
update_FNR()
{
	if (FNR_node->var_value->numbr != FNR) {
		unref(FNR_node->var_value);
		FNR_node->var_value = make_number((AWKNUM) FNR);
	}
}



NODE *frame_ptr;        /* current frame */
STACK_ITEM *stack_ptr = NULL;
STACK_ITEM *stack_bottom;
STACK_ITEM *stack_top;
static unsigned long STACK_SIZE = 256;    /* initial size of stack */
int max_args = 0;       /* maximum # of arguments to printf, print, sprintf,
                         * or # of array subscripts, or adjacent strings     
                         * to be concatenated.
                         */
NODE **args_array = NULL;

/* grow_stack --- grow the size of runtime stack */

/* N.B. stack_ptr points to the topmost occupied location
 *      on the stack, not the first free location.
 */

STACK_ITEM *
grow_stack()
{
	if (stack_ptr == NULL) {
		char *val;

		if ((val = getenv("GAWK_STACKSIZE")) != NULL) {
			if (isdigit((unsigned char) *val)) {
				unsigned long n = 0;
				for (; *val && isdigit((unsigned char) *val); val++)
					n = (n * 10) + *val - '0';
				if (n >= 1)
					STACK_SIZE = n;
			}
		}

		emalloc(stack_bottom, STACK_ITEM *, STACK_SIZE * sizeof(STACK_ITEM), "grow_stack");
		stack_ptr = stack_bottom - 1;
		stack_top = stack_bottom + STACK_SIZE - 1;

		/* initialize frame pointer */
		getnode(frame_ptr);
		frame_ptr->type = Node_frame;
		frame_ptr->stack = NULL;
		frame_ptr->func_node = NULL;	/* in main */
		frame_ptr->vname = NULL;
		return stack_ptr;
	}

	STACK_SIZE *= 2;
	erealloc(stack_bottom, STACK_ITEM *, STACK_SIZE * sizeof(STACK_ITEM), "grow_stack");
	stack_top = stack_bottom + STACK_SIZE - 1;
	stack_ptr = stack_bottom + STACK_SIZE / 2;
	return stack_ptr;
}

/*
 * r_get_lhs:
 * This returns a POINTER to a node pointer (var's value).
 * used to store the var's new value.
 */

NODE **
r_get_lhs(NODE *n, int reference)
{
	int isparam = FALSE;

	if (n->type == Node_param_list) {
		if ((n->flags & FUNC) != 0)
			fatal(_("can't use function name `%s' as variable or array"),
					n->vname);
		isparam = TRUE;
		n = GET_PARAM(n->param_cnt);
	}

	switch (n->type) {
	case Node_var_array:
		fatal(_("attempt to use array `%s' in a scalar context"),
				array_vname(n));
	case Node_array_ref:
		if (n->orig_array->type == Node_var_array)
			fatal(_("attempt to use array `%s' in a scalar context"),
					array_vname(n));
		n->orig_array->type = Node_var;
		n->orig_array->var_value = Nnull_string;
		/* fall through */
	case Node_var_new:
		n->type = Node_var;
		n->var_value = Nnull_string;
		break;

	case Node_var:
		break;

#if 0
	case Node_builtin:
		/* in gawk for a while */
		fatal(_("assignment is not allowed to result of builtin function"));
#endif

	default:
		cant_happen();
	}

	if (do_lint && reference && var_uninitialized(n))
		lintwarn((isparam ?
			_("reference to uninitialized argument `%s'") :
			_("reference to uninitialized variable `%s'")),
				n->vname);
	return &n->var_value;
}


/* r_get_field --- get the address of a field node */
 
static inline NODE **
r_get_field(NODE *n, Func_ptr *assign, int reference)
{
	long field_num;
	NODE **lhs;

	if (assign)
		*assign = NULL;
	if (do_lint) {
		if ((n->flags & NUMBER) == 0) {
			lintwarn(_("attempt to field reference from non-numeric value"));
			if (n->stlen == 0)
				lintwarn(_("attempt to field reference from null string"));
		}
	}

	field_num = (long) force_number(n);
	if (field_num < 0)
		fatal(_("attempt to access field %ld"), field_num);

	if (field_num == 0 && field0_valid) {		/* short circuit */
		lhs = &fields_arr[0];
		if (assign)
			*assign = reset_record;
	} else
		lhs = get_field(field_num, assign);
	if (do_lint && reference && (*lhs == Null_field || *lhs == Nnull_string))
		lintwarn(_("reference to uninitialized field `$%ld'"),
			      field_num);
	return lhs;
}


/*
 * calc_exp_posint --- calculate x^n for positive integral n,
 * using exponentiation by squaring without recursion.
 */

static AWKNUM
calc_exp_posint(AWKNUM x, long n)
{
	AWKNUM mult = 1;

	while (n > 1) {
		if ((n % 2) == 1)
			mult *= x;
		x *= x;
		n /= 2;
	}
	return mult * x;
}

/* calc_exp --- calculate x1^x2 */

AWKNUM
calc_exp(AWKNUM x1, AWKNUM x2)
{
	long lx;

	if ((lx = x2) == x2) {		/* integer exponent */
		if (lx == 0)
			return 1;
		return (lx > 0) ? calc_exp_posint(x1, lx)
				: 1.0 / calc_exp_posint(x1, -lx);
	}
	return (AWKNUM) pow((double) x1, (double) x2);
}


/* setup_frame --- setup new frame for function call */ 

static void
setup_frame(INSTRUCTION *pc)
{
	NODE *r = NULL;
	NODE *m;
	NODE *f;
	NODE **sp = NULL;
	char **varnames;
	int pcount, arg_count, i;

	f = pc->func_body;
	pcount = f->lnode->param_cnt;
	varnames = f->parmlist;
	arg_count = (pc + 1)->expr_count;

	/* check for extra args */ 
	if (arg_count > pcount) {
		warning(
			_("function `%s' called with more arguments than declared"),
       			f->vname);
		do {
			r = POP();
			if (r->type == Node_val)
				DEREF(r);
		} while (--arg_count > pcount);
	}

	if (pcount > 0) {
		emalloc(sp, NODE **, pcount * sizeof(NODE *), "setup_frame");
		memset(sp, 0, pcount * sizeof(NODE *));
	}

	for (i = 0; i < pcount; i++) {
		getnode(r);
		memset(r, 0, sizeof(NODE));
		sp[i] = r;
		if (i >= arg_count) {
			/* local variable */
			r->type = Node_var_new;
			r->vname = varnames[i];
			continue;
		}

		m = PEEK(arg_count - i - 1); /* arguments in reverse order on runtime stack */

		if (m->type == Node_param_list)
			m = GET_PARAM(m->param_cnt);
			
		switch (m->type) {
		case Node_var_new:
		case Node_var_array:
			r->type = Node_array_ref;
			r->orig_array = r->prev_array = m;
			break;

		case Node_array_ref:
			r->type = Node_array_ref;
			r->orig_array = m->orig_array;
			r->prev_array = m;
			break;

		case Node_var:
			/* Untyped (Node_var_new) variable as param became a
			 * scalar during evaluation of expression for a
			 * subsequent param.
			 */
			r->type = Node_var;
			r->var_value = Nnull_string;
			break;

		case Node_val:
			r->type = Node_var;
			r->var_value = m;
			break;

		default:
			cant_happen();
		}
		r->vname = varnames[i];
	}
	stack_adj(-arg_count);	/* adjust stack pointer */

	if (pc->opcode == Op_indirect_func_call) {
		r = POP();	/* indirect var */
		DEREF(r);
	}

	push_frame(frame_ptr);

	/* save current frame in stack */
	PUSH(frame_ptr);
	/* setup new frame */
	getnode(frame_ptr);
	frame_ptr->type = Node_frame;	
	frame_ptr->stack = sp;
	frame_ptr->func_node = f;
	frame_ptr->vname = NULL;

	frame_ptr->reti = (unsigned long) pc; /* on return execute pc->nexti */
}


/* restore_frame --- clean up the stack and update frame */

static INSTRUCTION *
restore_frame(NODE *fp)
{
	NODE *r;
	NODE **sp;
	int n;
	NODE *func;
	INSTRUCTION *ri;

	func = frame_ptr->func_node;
	n = func->lnode->param_cnt;
	sp = frame_ptr->stack;

	for (; n > 0; n--) {
		r = *sp++;
		if (r->type == Node_var)     /* local variable */
			DEREF(r->var_value);
		else if (r->type == Node_var_array)     /* local array */
			assoc_clear(r);
		freenode(r);
	}
	if (frame_ptr->stack != NULL)
		efree(frame_ptr->stack);
	ri = (INSTRUCTION *) frame_ptr->reti; /* execution in calling frame
	                                       * resumes from ri->nexti.
	                                       */
	freenode(frame_ptr);
	pop_frame();

	frame_ptr = fp;
	return ri->nexti;
}


/* free_arrayfor --- free 'for (var in array)' related data */

static inline void
free_arrayfor(NODE *r)
{
	if (r->var_array != NULL) {
		size_t num_elems = r->table_size;
		NODE **list = r->var_array;
		while (num_elems > 0)
			unref(list[--num_elems]);
		efree(list);
	}
	freenode(r);
}

/* unwind_stack --- pop the runtime stack */

void
unwind_stack(STACK_ITEM *sp_bottom)
{
	NODE *r;

	while (stack_ptr >= sp_bottom) {
		r = POP();
		switch (r->type) {
		case Node_instruction:
			freenode(r);
			break;

		case Node_frame:
			(void) restore_frame(r);
			source = frame_ptr->vname;
			break;

		case Node_arrayfor:
			free_arrayfor(r);
			break;

		case Node_val:
			DEREF(r);
			break;

		default:
			if (in_main_context())
				fatal(_("unwind_stack: unexpected type `%s'"),
						nodetype2str(r->type));
			/* else 
				* Node_var_array,
				* Node_param_list,
				* Node_var (e.g: trying to use scalar for array)
				* Node_regex/Node_dynregex
				* ?
			 */
			break;
		}
	}
}


/*
 * This generated compiler warnings from GCC 4.4. Who knows why.
 *
#define eval_condition(t)	(((t)->flags & MAYBE_NUM) && force_number(t), \
		((t)->flags & NUMBER) ? ((t)->numbr != 0.0) : ((t)->stlen != 0))
*/


static inline int
eval_condition(NODE *t)
{
	if ((t->flags & MAYBE_NUM) != 0)
		force_number(t);

	if ((t->flags & NUMBER) != 0)
		return (t->numbr != 0.0);

	return (t->stlen != 0);
}

/* cmp_scalar -- compare two nodes on the stack */

static inline int
cmp_scalar()
{
	NODE *t1, *t2;
	int di;

	t2 = POP_SCALAR();
	t1 = TOP();
	if (t1->type == Node_var_array) {
		DEREF(t2);
		fatal(_("attempt to use array `%s' in a scalar context"), array_vname(t1));
	}
	di = cmp_nodes(t1, t2);
	DEREF(t1);
	DEREF(t2);
	return di;
}

/* op_assign --- assignment operators excluding = */
 
static void
op_assign(OPCODE op)
{
	NODE **lhs;
	NODE *r = NULL;
	AWKNUM x1, x2;
#ifndef HAVE_FMOD
	AWKNUM x;
#endif

	lhs = POP_ADDRESS();
	x1 = force_number(*lhs);
	TOP_NUMBER(x2);
	unref(*lhs);
	switch (op) {
	case Op_assign_plus:
		r = *lhs = make_number(x1 + x2);
		break;
	case Op_assign_minus:
		r = *lhs = make_number(x1 - x2);
		break;
	case Op_assign_times:
		r = *lhs = make_number(x1 * x2);
		break;
	case Op_assign_quotient:
		if (x2 == (AWKNUM) 0) {
			decr_sp();
			fatal(_("division by zero attempted in `/='"));
		}
		r = *lhs = make_number(x1 / x2);
		break;
	case Op_assign_mod:
		if (x2 == (AWKNUM) 0) {
			decr_sp();
			fatal(_("division by zero attempted in `%%='"));
		}
#ifdef HAVE_FMOD
		r = *lhs = make_number(fmod(x1, x2));
#else   /* ! HAVE_FMOD */
		(void) modf(x1 / x2, &x);
		x = x1 - x2 * x;
		r = *lhs = make_number(x);
#endif  /* ! HAVE_FMOD */
		break;
	case Op_assign_exp:
		r = *lhs = make_number((AWKNUM) calc_exp((double) x1, (double) x2));
		break;
	default:
		break;
	}

	UPREF(r);
	REPLACE(r);
}


/* PUSH_CODE --- push a code onto the runtime stack */

void
PUSH_CODE(INSTRUCTION *cp)
{
	NODE *r;
	getnode(r);
	r->type = Node_instruction;
	r->code_ptr = cp;
	PUSH(r);
}

/* POP_CODE --- pop a code off the runtime stack */

INSTRUCTION *
POP_CODE()
{
	NODE *r;
	INSTRUCTION *cp;
	r = POP();
	cp = r->code_ptr;
	freenode(r);
	return cp;
}


/*
 * r_interpret:
 *   code is a list of instructions to run. returns the exit value
 *	 from the awk code.
 */
 
 /* N.B.:
 *   1) reference counting done for both number and string values.
 *   2) TEMP flag no longer needed (consequence of the above; valref = 0
 *	is the replacement).
 *   3) Stack operations:
 *       Use REPLACE[_XX] if last stack operation was TOP[_XX],
 *       PUSH[_XX] if last operation was POP[_XX] instead. 
 *   4) UPREF and DREF -- see awk.h 
 */


int
r_interpret(INSTRUCTION *code)
{
	INSTRUCTION *pc;   /* current instruction */
	NODE *r = NULL;
	NODE *m;
	INSTRUCTION *ni;
	NODE *t1, *t2;
	NODE *f;	/* function definition */
	NODE **lhs;
	AWKNUM x, x1, x2;
	int di, pre = FALSE;
	Regexp *rp;
#if defined(GAWKDEBUG) || defined(ARRAYDEBUG)
	int last_was_stopme = FALSE;	/* builtin stopme() called ? */
#endif
	int stdio_problem = FALSE;

	if (args_array == NULL)
		emalloc(args_array, NODE **, (max_args + 2)*sizeof(NODE *), "r_interpret");
	else
		erealloc(args_array, NODE **, (max_args + 2)*sizeof(NODE *), "r_interpret");

/* array subscript */
#define mk_sub(n)  	(n == 1 ? POP_STRING() : concat_exp(n, TRUE))

#ifdef DEBUGGING
#define JUMPTO(x)	do { post_execute(pc); pc = (x); goto top; } while(FALSE)
#else
#define JUMPTO(x)	do { pc = (x); goto top; } while(FALSE)
#endif

	pc = code;

	/* N.B.: always use JUMPTO for next instruction, otherwise bad things
	 * may happen. DO NOT add a real loop (for/while) below to
	 * replace ' forever {'; this catches failure to use JUMPTO to execute
	 * next instruction (e.g. continue statement).
	 */

	/* loop until hit Op_stop instruction */

	/* forever {  */
top:
		if (pc->source_line > 0)
			sourceline = pc->source_line;

#ifdef DEBUGGING
		if (! pre_execute(&pc))
			goto top;
#endif

		switch (pc->opcode) {
		case Op_rule:
			currule = pc->in_rule;   /* for sole use in Op_K_next, Op_K_nextfile, Op_K_getline* */
			/* fall through */
		case Op_func:
		case Op_ext_func:
			source = pc->source_file;
			break;

		case Op_atexit:
			/* avoid false source indications */
			source = NULL;
			sourceline = 0;
			(void) nextfile(&curfile, TRUE);	/* close input data file */ 
			/*
			 * This used to be:
			 *
			 * if (close_io() != 0 && ! exiting && exit_val == 0)
			 *      exit_val = 1;
			 *
			 * Other awks don't care about problems closing open files
			 * and pipes, in that it doesn't affect their exit status.
			 * So we no longer do either.
			 */
			(void) close_io(& stdio_problem);
			/*
			 * However, we do want to exit non-zero if there was a problem
			 * with stdout/stderr, so we reinstate a slightly different
			 * version of the above:
			 */
			if (stdio_problem && ! exiting && exit_val == 0)
				exit_val = 1;
			break;

		case Op_stop:
			return 0;

		case Op_push_i:
			m = pc->memory;
			PUSH((m->flags & INTLSTR) != 0 ? format_val(CONVFMT, CONVFMTidx, m): m);
			break;

		case Op_push:
		case Op_push_arg:
		{
			NODE *save_symbol;
			int isparam = FALSE;

			save_symbol = m = pc->memory;
			if (m->type == Node_param_list) {
				if ((m->flags & FUNC) != 0)
					fatal(_("can't use function name `%s' as variable or array"),
							m->vname);
				isparam = TRUE;
				save_symbol = m = GET_PARAM(m->param_cnt);
				if (m->type == Node_array_ref)
					m = m->orig_array;
			}
				
			switch (m->type) {
			case Node_var:
				if (do_lint && var_uninitialized(m))
					lintwarn(isparam ?
						_("reference to uninitialized argument `%s'") :
						_("reference to uninitialized variable `%s'"),
								save_symbol->vname);
				m = m->var_value;
				UPREF(m);
				PUSH(m);
				break;

			case Node_var_new:
				m->type = Node_var;
				m->var_value = Nnull_string;
				if (do_lint)
					lintwarn(isparam ?
						_("reference to uninitialized argument `%s'") :
						_("reference to uninitialized variable `%s'"),
								save_symbol->vname);
				PUSH(Nnull_string);
				break;

			case Node_var_array:
				if (pc->opcode == Op_push_arg)
					PUSH(m);
				else
					fatal(_("attempt to use array `%s' in a scalar context"),
							array_vname(save_symbol));
				break;

			default:
				cant_happen();
			}
		}
			break;	

		case Op_push_param:		/* function argument */
			m = pc->memory;
			if (m->type == Node_param_list)
				m = GET_PARAM(m->param_cnt);
			if (m->type == Node_var) {
				m = m->var_value;
				UPREF(m);
				PUSH(m);
		 		break;
			}
 			/* else
				fall through */
		case Op_push_array:
			PUSH(pc->memory);
			break;

		case Op_push_lhs:
			lhs = get_lhs(pc->memory, pc->do_reference);
			PUSH_ADDRESS(lhs);
			break;

		case Op_subscript:
			t2 = mk_sub(pc->sub_count);
			t1 = POP_ARRAY();
			r = *assoc_lookup(t1, t2, TRUE);
			DEREF(t2);
			if (r->type == Node_val)
				UPREF(r);
			PUSH(r);
			break;

		case Op_sub_array:
			t2 = mk_sub(pc->sub_count);
			t1 = POP_ARRAY();
			r = in_array(t1, t2);
			if (r == NULL) {
				getnode(r);
				r->type = Node_var_array;
				r->var_array = NULL;
				r->vname = estrdup(t2->stptr, t2->stlen);	/* the subscript in parent array */
				r->parent_array = t1;
				*assoc_lookup(t1, t2, FALSE) = r;
			} else if (r->type != Node_var_array)
				fatal(_("attempt to use scalar `%s[\"%.*s\"]' as an array"),
						array_vname(t1), (int) t2->stlen, t2->stptr);
			DEREF(t2);
			PUSH(r);
			break;

		case Op_subscript_lhs:
			t2 = mk_sub(pc->sub_count);
			t1 = POP_ARRAY();
			lhs = assoc_lookup(t1, t2, pc->do_reference);
			if ((*lhs)->type == Node_var_array)
				fatal(_("attempt to use array `%s[\"%.*s\"]' in a scalar context"),
						array_vname(t1), (int) t2->stlen, t2->stptr);
			DEREF(t2);
			PUSH_ADDRESS(lhs);
			break;

		case Op_field_spec:
			t1 = TOP_SCALAR();
			lhs = r_get_field(t1, (Func_ptr *) 0, TRUE);
			decr_sp();
			DEREF(t1);
			/* This used to look like this:
			    PUSH(dupnode(*lhs));
			   but was changed to bypass an apparent bug in the z/OS C compiler.
			   Please do not remerge.  */
			r = dupnode(*lhs);     /* can't use UPREF here */
			PUSH(r);
			break;

		case Op_field_spec_lhs:
			t1 = TOP_SCALAR();
			lhs = r_get_field(t1, &pc->target_assign->field_assign, pc->do_reference);
			decr_sp();
			DEREF(t1);
			PUSH_ADDRESS(lhs);
			break;

		case Op_lint:
			if (do_lint) {
				switch (pc->lint_type) {
				case LINT_assign_in_cond:
					lintwarn(_("assignment used in conditional context"));
					break;

				case LINT_no_effect:
					lintwarn(_("statement has no effect"));
					break;

				default:
					cant_happen();
				}
			}
			break;

		case Op_K_break:
		case Op_K_continue:
		case Op_jmp:
			JUMPTO(pc->target_jmp);

		case Op_jmp_false:
			r = POP_SCALAR();
			di = eval_condition(r);
			DEREF(r);
			if (! di)
				JUMPTO(pc->target_jmp);
			break;

		case Op_jmp_true:
			r = POP_SCALAR();
			di = eval_condition(r);
			DEREF(r);			
			if (di)
				JUMPTO(pc->target_jmp);
			break;

		case Op_and:
		case Op_or:
			t1 = POP_SCALAR();
			di = eval_condition(t1);
			DEREF(t1);
			if ((pc->opcode == Op_and && di)
					|| (pc->opcode == Op_or && ! di))
				break;
			r = make_number((AWKNUM) di);
			PUSH(r);
			ni = pc->target_jmp;
			JUMPTO(ni->nexti);

		case Op_and_final:
		case Op_or_final:
			t1 = TOP_SCALAR();
			r = make_number((AWKNUM) eval_condition(t1));
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_not:
			t1 = TOP_SCALAR(); 
			r = make_number((AWKNUM) ! eval_condition(t1));
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_equal:
			r = make_number((AWKNUM) (cmp_scalar() == 0));
			REPLACE(r);
			break;

		case Op_notequal:
			r = make_number((AWKNUM) (cmp_scalar() != 0));
			REPLACE(r);
			break;

		case Op_less:
			r = make_number((AWKNUM) (cmp_scalar() < 0));
			REPLACE(r);
			break;

		case Op_greater:
			r = make_number((AWKNUM) (cmp_scalar() > 0));
			REPLACE(r);
			break;

		case Op_leq:
			r = make_number((AWKNUM) (cmp_scalar() <= 0));
			REPLACE(r);
			break;

		case Op_geq:
			r = make_number((AWKNUM) (cmp_scalar() >= 0));
			REPLACE(r);
			break;

		case Op_plus_i:
			x2 = force_number(pc->memory);
			goto plus;

		case Op_plus:
			POP_NUMBER(x2);
plus:
			TOP_NUMBER(x1);
			r = make_number(x1 + x2);
			REPLACE(r);
			break;

		case Op_minus_i:
			x2 = force_number(pc->memory);
			goto minus;

		case Op_minus:
			POP_NUMBER(x2);
minus:
			TOP_NUMBER(x1);
			r = make_number(x1 - x2);
			REPLACE(r);
			break;

		case Op_times_i:
			x2 = force_number(pc->memory);
			goto times;

		case Op_times:
			POP_NUMBER(x2);
times:
			TOP_NUMBER(x1);
			r = make_number(x1 * x2);
			REPLACE(r);
			break;

		case Op_exp_i:
			x2 = force_number(pc->memory);
			goto exponent;

		case Op_exp:
			POP_NUMBER(x2);
exponent:
			TOP_NUMBER(x1);
			x = calc_exp(x1, x2);
			r = make_number(x);
			REPLACE(r);
			break;

		case Op_quotient_i:
			x2 = force_number(pc->memory);
			goto quotient;

		case Op_quotient:
			POP_NUMBER(x2);
quotient:
			if (x2 == 0)
				fatal(_("division by zero attempted"));

			TOP_NUMBER(x1);
			x = x1 / x2;
			r = make_number(x);
			REPLACE(r);
			break;		

		case Op_mod_i:
			x2 = force_number(pc->memory);
			goto mod;

		case Op_mod:
			POP_NUMBER(x2);
mod:
			if (x2 == 0)
				fatal(_("division by zero attempted in `%%'"));

			TOP_NUMBER(x1);
#ifdef HAVE_FMOD
			x = fmod(x1, x2);
#else	/* ! HAVE_FMOD */
			(void) modf(x1 / x2, &x);
			x = x1 - x * x2;
#endif	/* ! HAVE_FMOD */
			r = make_number(x);
			REPLACE(r);
			break;		

		case Op_preincrement:
			pre = TRUE;
		case Op_postincrement:
			x2 = 1.0;
post:
			lhs = TOP_ADDRESS();
			x1 = force_number(*lhs);
			unref(*lhs);
			r = *lhs = make_number(x1 + x2);
			if (pre)
				UPREF(r);
			else
				r = make_number(x1);
			REPLACE(r);
			pre = FALSE;
			break;			

		case Op_predecrement:
			pre = TRUE;
		case Op_postdecrement:
			x2 = -1.0;
			goto post;					

		case Op_unary_minus:
			TOP_NUMBER(x1);
			r = make_number(-x1);
			REPLACE(r);
			break;

		case Op_store_sub:
			/* array[sub] assignment optimization,
			 * see awkgram.y (optimize_assignment)
			 */
			t1 = get_array(pc->memory, TRUE);	/* array */
			t2 = mk_sub(pc->expr_count);	/* subscript */
 			lhs = assoc_lookup(t1, t2, FALSE);
			if ((*lhs)->type == Node_var_array)
				fatal(_("attempt to use array `%s[\"%.*s\"]' in a scalar context"),
						array_vname(t1), (int) t2->stlen, t2->stptr);
			DEREF(t2);
			unref(*lhs);
			*lhs = POP_SCALAR();
			break;

		case Op_store_var:
			/* simple variable assignment optimization,
			 * see awkgram.y (optimize_assignment)
			 */
	
			lhs = get_lhs(pc->memory, FALSE);
			unref(*lhs);
			*lhs = POP_SCALAR();
			break;

		case Op_store_field:
		{
			/* field assignment optimization,
			 * see awkgram.y (optimize_assignment)
			 */

			Func_ptr assign;
			t1 = TOP_SCALAR();
			lhs = r_get_field(t1, &assign, FALSE);
			decr_sp();
			DEREF(t1);
			unref(*lhs);
			*lhs = POP_SCALAR();
			assert(assign != NULL);
			assign();
		}
			break;

		case Op_assign_concat:
			/* x = x ... string concatenation optimization */
			lhs = get_lhs(pc->memory, FALSE);
			t1 = force_string(*lhs);
			t2 = POP_STRING();

			free_wstr(*lhs);

			if (t1 != t2 && t1->valref == 1 && (t1->flags & PERM) == 0) {
				size_t nlen = t1->stlen + t2->stlen;
				erealloc(t1->stptr, char *, nlen + 2, "r_interpret");
				memcpy(t1->stptr + t1->stlen, t2->stptr, t2->stlen);
				t1->stlen = nlen;
				t1->stptr[nlen] = '\0';
			} else {
				size_t nlen = t1->stlen + t2->stlen;  
				char *p;

				emalloc(p, char *, nlen + 2, "r_interpret");
				memcpy(p, t1->stptr, t1->stlen);
				memcpy(p + t1->stlen, t2->stptr, t2->stlen);
				unref(*lhs);
				t1 = *lhs = make_str_node(p, nlen,  ALREADY_MALLOCED); 
			}
			t1->flags &= ~(NUMCUR|NUMBER);
			DEREF(t2);
			break;

		case Op_assign:
			lhs = POP_ADDRESS();
			r = TOP_SCALAR();
			unref(*lhs);
			*lhs = r;
			UPREF(r);
			REPLACE(r);
			break;

		/* numeric assignments */
		case Op_assign_plus:
		case Op_assign_minus:
		case Op_assign_times:
		case Op_assign_quotient:
		case Op_assign_mod:
		case Op_assign_exp:
			op_assign(pc->opcode);
			break;

		case Op_var_update:        /* update value of NR, FNR or NF */
			pc->update_var();
			break;

		case Op_var_assign:
			pc->assign_var();
			break;

		case Op_field_assign:
			pc->field_assign();
			break;

		case Op_concat:
			r = concat_exp(pc->expr_count, pc->concat_flag & CSUBSEP);
			PUSH(r);
			break;

		case Op_K_case:
			if ((pc + 1)->match_exp) {
				/* match a constant regex against switch expression instead of $0. */
				m = POP();	/* regex */
				t2 = TOP_SCALAR();	/* switch expression */
				(void) force_string(t2);
				rp = re_update(m);
				di = (research(rp, t2->stptr, 0, t2->stlen,
							avoid_dfa(m, t2->stptr, t2->stlen)) >= 0);
			} else {
				t1 = POP_SCALAR();	/* case value */
				t2 = TOP_SCALAR();	/* switch expression */
				di = (cmp_nodes(t2, t1) == 0);
				DEREF(t1);
			}

			if (di) {	/* match found */
				decr_sp();
				DEREF(t2);
				JUMPTO(pc->target_jmp);
			}
			break;

		case Op_K_delete:
			t1 = POP_ARRAY();
			do_delete(t1, pc->expr_count);
			stack_adj(-pc->expr_count);
			break;

		case Op_K_delete_loop:
			t1 = POP_ARRAY();
			lhs = POP_ADDRESS();	/* item */
			do_delete_loop(t1, lhs);
			break;

		case Op_in_array:
			t1 = POP_ARRAY();
			t2 = mk_sub(pc->expr_count);
			di = (in_array(t1, t2) != NULL);
			DEREF(t2);
			PUSH(make_number((AWKNUM) di));
			break;

		case Op_arrayfor_init:
		{
			NODE **list = NULL;
			NODE *array, *sort_str;
			size_t num_elems = 0;
			static NODE *sorted_in = NULL;
			const char *how_to_sort = "@unsorted";

			/* get the array */
			array = POP_ARRAY();

			/* sanity: check if empty */
			if (array->var_array == NULL || array->table_size == 0)
				goto arrayfor;

			num_elems = array->table_size;

			if (sorted_in == NULL)		/* do this once */
				sorted_in = make_string("sorted_in", 9);

			sort_str = NULL;
			/*
			 * If posix, or if there's no PROCINFO[],
			 * there's no ["sorted_in"], so no sorting
			 */
			if (! do_posix && PROCINFO_node != NULL)
				sort_str = in_array(PROCINFO_node, sorted_in);

			if (sort_str != NULL) {
				sort_str = force_string(sort_str);
				if (sort_str->stlen > 0)
					how_to_sort = sort_str->stptr;
			}

			list = assoc_list(array, how_to_sort, SORTED_IN);

			/*
			 * Actual array for use in lint warning
			 * in Op_arrayfor_incr
                         */
			list[num_elems] = array;

arrayfor:
			getnode(r);
			r->type = Node_arrayfor;
			r->var_array = list;
			r->table_size = num_elems;     /* # of elements in list */
			r->array_size = -1;            /* current index */
			PUSH(r);

			if (num_elems == 0)
				JUMPTO(pc->target_jmp);   /* Op_arrayfor_final */
		}
			break;

		case Op_arrayfor_incr:
			r = TOP();	/* Node_arrayfor */
			if (++r->array_size == r->table_size) {
				NODE *array;
				array = r->var_array[r->table_size];	/* actual array */
				if (do_lint && array->table_size != r->table_size)
					lintwarn(_("for loop: array `%s' changed size from %ld to %ld during loop execution"),
						array_vname(array), (long) r->table_size, (long) array->table_size);
				JUMPTO(pc->target_jmp);	/* Op_arrayfor_final */
			}

			t1 = r->var_array[r->array_size];
			lhs = get_lhs(pc->array_var, FALSE);
			unref(*lhs);
			*lhs = make_string(t1->ahname_str, t1->ahname_len);
			break; 			 

		case Op_arrayfor_final:
			r = POP();
			assert(r->type == Node_arrayfor);
			free_arrayfor(r);
			break;

		case Op_builtin:
			r = pc->builtin(pc->expr_count);
#if defined(GAWKDEBUG) || defined(ARRAYDEBUG)
			if (! r)
				last_was_stopme = TRUE;
			else
#endif
				PUSH(r);
			break;
			
		case Op_K_print:
			do_print(pc->expr_count, pc->redir_type);
			break;

		case Op_K_printf:
			do_printf(pc->expr_count, pc->redir_type);
			break;

		case Op_K_print_rec:
			do_print_rec(pc->expr_count, pc->redir_type);
			break;

		case Op_push_re:
			m = pc->memory;
			if (m->type == Node_dynregex) {
				r = POP_STRING();
				unref(m->re_exp);
				m->re_exp = r;
			}
			PUSH(m);
			break;
			
		case Op_match_rec:
			m = pc->memory;
			t1 = *get_field(0, (Func_ptr *) 0);
match_re:
			rp = re_update(m);
			/*
			 * Any place where research() is called with a last parameter of
			 * zero, we need to use the avoid_dfa test. This appears here and
			 * in the code for Op_K_case.
			 *
			 * A new or improved dfa that distinguishes beginning/end of
			 * string from beginning/end of line will allow us to get rid of
			 * this hack.
			 *
			 * The avoid_dfa() function is in re.c; it is not very smart.
			 */

			di = research(rp, t1->stptr, 0, t1->stlen,
								avoid_dfa(m, t1->stptr, t1->stlen));
			di = (di == -1) ^ (pc->opcode != Op_nomatch);
			if(pc->opcode != Op_match_rec) {
				decr_sp();
				DEREF(t1);
			}
			r = make_number((AWKNUM) di);
			PUSH(r);
			break;

		case Op_nomatch:
			/* fall through */
		case Op_match:
			m = pc->memory;
			t1 = TOP_STRING();
			if (m->type == Node_dynregex) {
				unref(m->re_exp);
				m->re_exp = t1;
				decr_sp();
				t1 = TOP_STRING();
			}
			goto match_re;
			break;

		case Op_indirect_func_call:
		{
			int arg_count;

			f = NULL;
			arg_count = (pc + 1)->expr_count;
			t1 = PEEK(arg_count);	/* indirect var */
			assert(t1->type == Node_val);	/* @a[1](p) not allowed in grammar */
			(void) force_string(t1);
			if (t1->stlen > 0) {
				/* retrieve function definition node */
				f = pc->func_body;
				if (f != NULL && STREQ(f->vname, t1->stptr))
					/* indirect var hasn't been reassigned */
					goto func_call;
				f = lookup(t1->stptr);
			}

			if (f == NULL || f->type != Node_func)
				fatal(_("function called indirectly through `%s' does not exist"), pc->func_name);	
			pc->func_body = f;     /* save for next call */

			goto func_call;
		}

		case Op_func_call:
			/* retrieve function definition node */
			f = pc->func_body;
			if (f == NULL) {
				f = lookup(pc->func_name);
				if (f == NULL || f->type != Node_func)
					fatal(_("function `%s' not defined"), pc->func_name);
				pc->func_body = f;     /* save for next call */
			}

			/* save current frame along with source */

func_call:
			frame_ptr->vname = source;          /* save current source */
			setup_frame(pc);

			ni = f->code_ptr;	/* function code */							
			if (ni->opcode == Op_ext_func) {
				/* dynamically set source and line numbers for an extension builtin. */
				ni->source_file = source;
				ni->source_line = sourceline;
				ni->nexti->source_line = sourceline;	/* Op_builtin */
				ni->nexti->nexti->source_line = sourceline;	/* Op_K_return */
			}

			/* run the function instructions */
			JUMPTO(ni);		/* Op_func or Op_ext_func */

		case Op_K_return:
			m = POP_SCALAR();       /* return value */

			r = POP();
			while (r->type != Node_frame) {
				switch (r->type) {
				case Node_arrayfor:
					free_arrayfor(r);
					break;
				case Node_val:
					DEREF(r);
					break;
				case Node_instruction:
					freenode(r);
					break;
				default:
					break;
				}
				r = POP();
			} 

			ni = restore_frame(r);
			source = frame_ptr->vname;
			
			/* put the return value back on stack */
			PUSH(m);
			JUMPTO(ni);

		case Op_K_getline_redir:
			if ((currule == BEGINFILE || currule == ENDFILE)
					&& pc->into_var == FALSE
					&& pc->redir_type == redirect_input)
				fatal(_("`getline' invalid inside `%s' rule"), ruletab[currule]);
			r = do_getline_redir(pc->into_var, pc->redir_type);
			PUSH(r);
			break;

		case Op_K_getline:	/* no redirection */
			if (currule == BEGINFILE || currule == ENDFILE)
				fatal(_("non-redirected `getline' invalid inside `%s' rule"),
						ruletab[currule]);
			do {
				int ret;
				ret = nextfile(&curfile, FALSE);
				if (ret <= 0)
					r = do_getline(pc->into_var, curfile);
				else {
					PUSH_CODE(pc);
					if (curfile == NULL)
						JUMPTO((pc + 1)->target_endfile);
					else
						JUMPTO((pc + 1)->target_beginfile);
				}
			} while (r == NULL);	/* EOF */
			PUSH(r);
			break;

		case Op_after_endfile:
			ni = POP_CODE();
			assert(ni->opcode == Op_newfile || ni->opcode == Op_K_getline);
			JUMPTO(ni);

		case Op_after_beginfile:
			after_beginfile(&curfile);
			ni = POP_CODE();
			if (ni->opcode == Op_K_getline
					|| curfile == NULL      /* skipping directory argument */
			)
				JUMPTO(ni);
			PUSH_CODE(ni);      /* for use in Op_K_nextfile and Op_get_record */
			break;              /* Op_get_record */

		case Op_newfile:
		{
			int ret;
			ret = nextfile(&curfile, FALSE);
			if (ret < 0)
				JUMPTO(pc->target_jmp);     /* end block or Op_atexit */
			else if (ret > 0) {
				PUSH_CODE(pc);
				if (curfile == NULL)
					JUMPTO(pc->target_endfile);
				break;	/* beginfile block */
			} else
				PUSH_CODE(pc);
				/* fall through */
		}
			
		case Op_get_record:
			if (curfile == NULL) {          /* from getline without redirection */
				ni = POP_CODE();            /* Op_newfile */
				ni = ni->target_jmp;        /* end_block or Op_atexit */
			} else if (inrec(curfile) == 0)
				break;                      /* prog(rule) block */
			else
				ni = POP_CODE();            /* Op_newfile */
			JUMPTO(ni);

		case Op_K_nextfile:
			if (currule != Rule && currule != BEGINFILE)
				fatal(_("`nextfile' cannot be called from a `%s' rule"), ruletab[currule]);
			(void) nextfile(&curfile, TRUE);
			while (currule == BEGINFILE) {
				r = POP();
				switch (r->type) {
				case Node_instruction:
					ni = r->code_ptr;
					freenode(r);
					if (ni->opcode == Op_newfile || ni->opcode == Op_K_getline)
						JUMPTO(ni);
					break;
				case Node_frame:
					(void) restore_frame(r);
					source = frame_ptr->vname;
					break;
				case Node_arrayfor:
					free_arrayfor(r);
					break;
				case Node_val:
					DEREF(r);
					break;
				default:
					break;
				}
			}

			assert(currule != BEGINFILE);
			unwind_stack(stack_bottom + 1);	/* don't pop Op_newfile */ 
			JUMPTO(pc->target_endfile);		/* endfile block */

		case Op_K_exit:
			if (currule == END)
				ni = pc->target_atexit;
			else
				ni = pc->target_end;

			exiting = TRUE;
			POP_NUMBER(x1);
			exit_val = (int) x1;
#ifdef VMS
			if (exit_val == 0)
				exit_val = EXIT_SUCCESS;
			else if (exit_val == 1)
				exit_val = EXIT_FAILURE;
			/* else
				just pass anything else on through */
#endif
			/* jump to either the first end_block instruction
			 * or to Op_atexit
			 */
			unwind_stack(stack_bottom);
			JUMPTO(ni);

		case Op_K_next:
			if (currule != Rule)
				fatal(_("`next' cannot be called from a `%s' rule"), ruletab[currule]);

			/* jump to Op_get_record */
			unwind_stack(stack_bottom + 1);	/* don't pop Op_newfile */
			JUMPTO(pc->target_jmp);

		case Op_pop:
#if defined(GAWKDEBUG) || defined(ARRAYDEBUG)
			if (last_was_stopme)
				last_was_stopme = FALSE;
			else
#endif
			{
				r = POP_SCALAR();
				DEREF(r);
			}
			break;

		case Op_line_range:
			if (pc->triggered)		/* evaluate right expression */
				JUMPTO(pc->target_jmp);
			/* else
				evaluate left expression */
			break;

		case Op_cond_pair:
		{
			int result;
			INSTRUCTION *ip;

			t1 = TOP_SCALAR();   /* from right hand side expression */
			di = (eval_condition(t1) != 0);
			DEREF(t1);

			ip = pc->line_range;            /* Op_line_range */

			if (! ip->triggered && di) {
				/* not already triggered and left expression is TRUE */
				decr_sp();
				ip->triggered = TRUE;
				JUMPTO(ip->target_jmp);	/* evaluate right expression */ 
			}

			result = ip->triggered || di;
			ip->triggered ^= di;            /* update triggered flag */
			r = make_number((AWKNUM) result);      /* final value of condition pair */
			REPLACE(r);
			JUMPTO(pc->target_jmp);
		}

		case Op_exec_count:
			INCREMENT(pc->exec_count);
			break;

		case Op_no_op:
		case Op_K_do:
		case Op_K_while:
		case Op_K_for:
		case Op_K_arrayfor:
		case Op_K_switch:
		case Op_K_default:
		case Op_K_if:
		case Op_K_else:
		case Op_cond_exp:
			break;

		default:
			fatal(_("Sorry, don't know how to interpret `%s'"), opcode2str(pc->opcode));
		}

		JUMPTO(pc->nexti);

/*	} forever */

	/* not reached */
	return 0;

#undef mk_sub
#undef JUMPTO
}
