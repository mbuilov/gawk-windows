/*
 * eval.c - gawk bytecode interpreter
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2019 the Free Software Foundation, Inc.
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

extern double pow(double x, double y);
extern double modf(double x, double *yp);
extern double fmod(double x, double y);
NODE **fcall_list = NULL;
long fcall_count = 0;
int currule = 0;
IOBUF *curfile = NULL;		/* current data file */
bool exiting = false;

int (*interpret)(INSTRUCTION *);
#define MAX_EXEC_HOOKS	10
static int num_exec_hook = 0;
static Func_pre_exec pre_execute[MAX_EXEC_HOOKS];
static Func_post_exec post_execute = NULL;

extern void frame_popped();

int OFSlen;
int ORSlen;
int OFMTidx;
int CONVFMTidx;

static NODE *node_Boolean[2];

/* This rather ugly macro is for VMS C */
#ifdef C
#undef C
#endif
#define C(c) ((char)c)
/*
 * This table is used by the regexp routines to do case independent
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
	/*
	 * 4/2019: This is now overridden; in single byte locales
	 * we call load_casetable from main and it fills in the values
	 * based on the current locale. In particular, we want LC_ALL=C
	 * to work correctly for values >= 0200.
	 */
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
#elif defined(USE_EBCDIC)
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
	static bool loaded = false;

	if (loaded || do_traditional)
		return;

	loaded = true;

#ifndef USE_EBCDIC
	/* use of isalpha is ok here (see is_alpha in awkgram.y) */
	for (i = 0200; i <= 0377; i++) {
		if (isalpha(i) && islower(i) && i != toupper(i))
			casetable[i] = toupper(i);
		else
			casetable[i] = i;
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
	"Node_ext_func",
	"Node_builtin_func",
	"Node_array_ref",
	"Node_array_tree",
	"Node_array_leaf",
	"Node_dump_array",
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
	{ "Op_unary_plus", "+" },
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
	{ "Op_K_return_from_eval", "return" },
	{ "Op_K_delete", "delete" },
	{ "Op_K_delete_loop", NULL },
	{ "Op_K_getline_redir", "getline" },
	{ "Op_K_getline", "getline" },
	{ "Op_K_nextfile", "nextfile" },
	{ "Op_K_namespace", "@namespace" },
	{ "Op_builtin", NULL },
	{ "Op_sub_builtin", NULL },
	{ "Op_ext_builtin", NULL },
	{ "Op_in_array", " in " },
	{ "Op_func_call", NULL },
	{ "Op_indirect_func_call", NULL },
	{ "Op_push", NULL },
	{ "Op_push_arg", NULL },
	{ "Op_push_arg_untyped", NULL },
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
	{ "Op_subscript_assign", NULL },
	{ "Op_after_beginfile", NULL },
	{ "Op_after_endfile", NULL },
	{ "Op_func", NULL },
	{ "Op_comment", NULL },
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
	{ "Op_parens", NULL },
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

/* opcode2str --- convert an opcode type into a printable value */

const char *
opcode2str(OPCODE op)
{
	if (op >= Op_illegal && op < Op_final)
		return optypes[(int) op].desc;
	fatal(_("unknown opcode %d"), (int) op);
	return NULL;
}

/* op2str --- convert an opcode type to corresponding operator or keyword */

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
		{ STRING, "STRING" },
		{ STRCUR, "STRCUR" },
		{ NUMCUR, "NUMCUR" },
		{ NUMBER, "NUMBER" },
		{ USER_INPUT, "USER_INPUT" },
		{ INTLSTR, "INTLSTR" },
		{ NUMINT, "NUMINT" },
		{ INTIND, "INTIND" },
		{ WSTRCUR, "WSTRCUR" },
		{ MPFN,	"MPFN" },
		{ MPZN,	"MPZN" },
		{ NO_EXT_SET, "NO_EXT_SET" },
		{ NULL_FIELD, "NULL_FIELD" },
		{ ARRAYMAXED, "ARRAYMAXED" },
		{ HALFHAT, "HALFHAT" },
		{ XARRAY, "XARRAY" },
		{ NUMCONSTSTR, "NUMCONSTSTR" },
		{ REGEX, "REGEX" },
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
			if (space_left <= space_needed)
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

	*sp = '\0';
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
#if ! defined(__DJGPP__)
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
cmp_nodes(NODE *t1, NODE *t2, bool use_strcmp)
{
	int ret = 0;
	size_t len1, len2;
	int l, ldiff;

	if (t1 == t2)
		return 0;

	(void) fixtype(t1);
	(void) fixtype(t2);

	if ((t1->flags & NUMBER) != 0 && (t2->flags & NUMBER) != 0)
		return cmp_numbers(t1, t2);

	(void) force_string(t1);
	(void) force_string(t2);
	len1 = t1->stlen;
	len2 = t2->stlen;
	ldiff = len1 - len2;
	if (len1 == 0 || len2 == 0)
		return ldiff;

	if (do_posix && ! use_strcmp)
		return posix_compare(t1, t2);

	l = (ldiff <= 0 ? len1 : len2);
	if (IGNORECASE) {
		const unsigned char *cp1 = (const unsigned char *) t1->stptr;
		const unsigned char *cp2 = (const unsigned char *) t2->stptr;
		char save1 = t1->stptr[t1->stlen];
		char save2 = t2->stptr[t2->stlen];


		if (gawk_mb_cur_max > 1) {
			t1->stptr[t1->stlen] = t2->stptr[t2->stlen] = '\0';
			ret = strncasecmpmbs((const unsigned char *) cp1,
					     (const unsigned char *) cp2, l);
			t1->stptr[t1->stlen] = save1;
			t2->stptr[t2->stlen] = save2;
		} else {
			/* Could use tolower() here; see discussion above. */
			for (ret = 0; l-- > 0 && ret == 0; cp1++, cp2++)
				ret = casetable[*cp1] - casetable[*cp2];
		}
	} else
		ret = memcmp(t1->stptr, t2->stptr, l);

	ret = ret == 0 ? ldiff : ret;
	return ret;
}

/* push_frame --- push a frame NODE onto stack */

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


/* pop_frame --- pop off a frame NODE*/

static void
pop_frame()
{
	if (fcall_count > 1)
		memmove(fcall_list + 1, fcall_list + 2, (fcall_count - 1) * sizeof(NODE *));
	fcall_count--;
	assert(fcall_count >= 0);
	if (do_debug)
		frame_popped();
}


/* dump_fcall_stack --- print a backtrace of the awk function calls */

void
dump_fcall_stack(FILE *fp)
{
	NODE *f, *func;
	long i = 0, k = 0;

	if (fcall_count == 0)
		return;
	fprintf(fp, _("\n\t# Function Call Stack:\n\n"));

	/* current frame */
	func = frame_ptr->func_node;
	fprintf(fp, "\t# %3ld. %s\n", k++, func->vname);

	/* outer frames except main */
	for (i = 1; i < fcall_count; i++) {
		f = fcall_list[i];
		func = f->func_node;
		fprintf(fp, "\t# %3ld. %s\n", k++, func->vname);
	}

	fprintf(fp, "\t# %3ld. -- main --\n", k);
}


/* set_IGNORECASE --- update IGNORECASE as appropriate */

void
set_IGNORECASE()
{
	static bool warned = false;

	if ((do_lint_extensions || do_traditional) && ! warned) {
		warned = true;
		lintwarn(_("`IGNORECASE' is a gawk extension"));
	}

	if (do_traditional)
		IGNORECASE = false;
   	else
		IGNORECASE = boolval(IGNORECASE_node->var_value);
	set_RS();	/* set_RS() calls set_FS() if need be, for us */
}

/* set_BINMODE --- set translation mode (OS/2, DOS, others) */

void
set_BINMODE()
{
	static bool warned = false;
	char *p;
	NODE *v = fixtype(BINMODE_node->var_value);

	if ((do_lint_extensions || do_traditional) && ! warned) {
		warned = true;
		lintwarn(_("`BINMODE' is a gawk extension"));
	}
	if (do_traditional)
		BINMODE = TEXT_TRANSLATE;
	else if ((v->flags & NUMBER) != 0) {
		BINMODE = get_number_si(v);
		/* Make sure the value is rational. */
		if (BINMODE < TEXT_TRANSLATE)
			BINMODE = TEXT_TRANSLATE;
		else if (BINMODE > BINMODE_BOTH)
			BINMODE = BINMODE_BOTH;
	} else if ((v->flags & STRING) != 0) {
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
				BINMODE = BINMODE_INPUT;
				break;
			case 'w':
				BINMODE = BINMODE_OUTPUT;
				break;
			default:
				BINMODE = BINMODE_BOTH;
				goto bad_value;
				break;
			}
			break;
		case 2:
			switch (p[0]) {
			case 'r':
				BINMODE = BINMODE_BOTH;
				if (p[1] != 'w')
					goto bad_value;
				break;
			case 'w':
				BINMODE = BINMODE_BOTH;
				if (p[1] != 'r')
					goto bad_value;
				break;
			}
			break;
		default:
	bad_value:
			lintwarn(_("BINMODE value `%s' is invalid, treated as 3"), p);
			break;
		}
	} else
		BINMODE = 3;		/* shouldn't happen */
}

/* set_OFS --- update OFS related variables when OFS assigned to */

void
set_OFS()
{
	static bool first = true;
	size_t new_ofs_len;

	if (first)	/* true when called from init_vars() in main() */
		first = false;
	else {
		/* rebuild $0 using OFS that was current when $0 changed */
		if (! field0_valid) {
			get_field(UNLIMITED - 1, NULL);
			rebuild_record();
		}
	}

	/*
	 * Save OFS value for use in building record and in printing.
	 * Can't just have OFS point into the OFS_node since it's
	 * already updated when we come into this routine, and we need
	 * the old value to rebuild the record (see above).
	 */
	OFS_node->var_value = force_string(OFS_node->var_value);
	new_ofs_len = OFS_node->var_value->stlen;

	if (OFS == NULL)
		emalloc(OFS, char *, new_ofs_len + 1, "set_OFS");
	else if (OFSlen < new_ofs_len)
		erealloc(OFS, char *, new_ofs_len + 1, "set_OFS");

	memcpy(OFS, OFS_node->var_value->stptr, OFS_node->var_value->stlen);
	OFSlen = new_ofs_len;
	OFS[OFSlen] = '\0';
}

/* set_ORS --- update ORS related variables when ORS assigned to */

void
set_ORS()
{
	ORS_node->var_value = force_string(ORS_node->var_value);
	ORS = ORS_node->var_value->stptr;
	ORSlen = ORS_node->var_value->stlen;
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

	// We rely on the caller to zero-terminate n->stptr.

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
	char save;

	if (fmt_list == NULL)
		emalloc(fmt_list, NODE **, fmt_num*sizeof(*fmt_list), "fmt_index");
	n = force_string(n);

	save = n->stptr[n->stlen];
	n->stptr[n->stlen] = '\0';

	while (ix < fmt_hiwater) {
		if (cmp_nodes(fmt_list[ix], n, true) == 0)
			return ix;
		ix++;
	}

	/* not found */
	if (do_lint && ! fmt_ok(n))
		lintwarn(_("bad `%sFMT' specification `%s'"),
			    n == CONVFMT_node->var_value ? "CONV"
			  : n == OFMT_node->var_value ? "O"
			  : "", n->stptr);

	n->stptr[n->stlen] = save;

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
	NODE *n = fixtype(LINT_node->var_value);

	/* start with clean defaults */
	lintfunc = r_warning;
	do_flags &= ~(DO_LINT_ALL|DO_LINT_INVALID);

	if ((n->flags & STRING) != 0) {
		const char *lintval;
		size_t lintlen;

		lintval = n->stptr;
		lintlen = n->stlen;
		if (lintlen > 0) {
			if (lintlen == 7 && strncmp(lintval, "invalid", 7) == 0)
				do_flags |= DO_LINT_INVALID;
			else if (lintlen == 6 && strncmp(lintval, "no-ext", 6) == 0)
				do_flags &= ~DO_LINT_EXTENSIONS;
			else {
				do_flags |= DO_LINT_ALL;
				if (lintlen == 5 && strncmp(lintval, "fatal", 5) == 0)
					lintfunc = r_fatal;
			}
		}
	} else {
		if (! iszero(n))
			do_flags |= DO_LINT_ALL;
	}

	/* explicitly use warning() here, in case lintfunc == r_fatal */
	if (old_lint != do_lint && old_lint && ! do_lint)
		warning(_("turning off `--lint' due to assignment to `LINT'"));

	/* inform plug-in api of change */
	update_ext_api();
#endif /* ! NO_LINT */
}

/* set_TEXTDOMAIN --- update TEXTDOMAIN variable when TEXTDOMAIN assigned to */

void
set_TEXTDOMAIN()
{
	NODE *tmp;

	tmp = TEXTDOMAIN_node->var_value = force_string(TEXTDOMAIN_node->var_value);
	TEXTDOMAIN = tmp->stptr;
	/*
	 * Note: don't call textdomain(); this value is for
	 * the awk program, not for gawk itself.
	 */
}

/* update_ERRNO_int --- update the value of ERRNO based on argument */

void
update_ERRNO_int(int errcode)
{
	char *cp;

	update_PROCINFO_num("errno", errcode);
	if (errcode) {
		cp = strerror(errcode);
		cp = gettext(cp);
	} else
		cp = "";
	unref(ERRNO_node->var_value);
	ERRNO_node->var_value = make_string(cp, strlen(cp));
}

/* update_ERRNO_string --- update ERRNO */

void
update_ERRNO_string(const char *string)
{
	update_PROCINFO_num("errno", 0);
	unref(ERRNO_node->var_value);
	size_t len = strlen(string);
#if defined(USE_EBCDIC) && defined(ELIDE_IBM_ERROR_CODE)
	// skip over leading IBM error code
	// N.B. This code is untested
	if (isupper(string[0]) && isupper(string[1])) {
		while (*string && *string != ' ')
			string++;

		while (*string && *string == ' ')
			string++;

		len = strlen(string);
		if (string[len-1] == '.')
			len--;	// remove the final '.'
	}
#endif
	ERRNO_node->var_value = make_string(string, len);
}

/* unset_ERRNO --- eliminate the value of ERRNO */

void
unset_ERRNO(void)
{
	update_PROCINFO_num("errno", 0);
	unref(ERRNO_node->var_value);
	ERRNO_node->var_value = dupnode(Nnull_string);
}

/* update_NR --- update the value of NR */

void
update_NR()
{
#ifdef HAVE_MPFR
	if (is_mpg_number(NR_node->var_value))
		(void) mpg_update_var(NR_node);
	else
#endif
	if (NR_node->var_value->numbr != NR) {
		unref(NR_node->var_value);
		NR_node->var_value = make_number(NR);
	}
}

/* update_NF --- update the value of NF */

void
update_NF()
{
	long l;

	l = get_number_si(NF_node->var_value);
	if (NF == -1 || l != NF) {
		if (NF == -1)
			(void) get_field(UNLIMITED - 1, NULL); /* parse record */
		unref(NF_node->var_value);
		NF_node->var_value = make_number(NF);
	}
}

/* update_FNR --- update the value of FNR */

void
update_FNR()
{
#ifdef HAVE_MPFR
	if (is_mpg_number(FNR_node->var_value))
		(void) mpg_update_var(FNR_node);
	else
#endif
	if (FNR_node->var_value->numbr != FNR) {
		unref(FNR_node->var_value);
		FNR_node->var_value = make_number(FNR);
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
r_get_lhs(NODE *n, bool reference)
{
	bool isparam = false;

	if (n->type == Node_param_list) {
		isparam = true;
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
		if (n->orig_array->type != Node_var) {
			n->orig_array->type = Node_var;
			n->orig_array->var_value = dupnode(Nnull_string);
		}
		/* fall through */
	case Node_var_new:
		n->type = Node_var;
		n->var_value = dupnode(Nnull_string);
		break;

	case Node_var:
		break;

	default:
		cant_happen();
	}

	if (do_lint && reference && var_uninitialized(n))
		lintwarn((isparam ?
			_("reference to uninitialized argument `%s'") :
			_("reference to uninitialized variable `%s'")),
				n->vname);
	return & n->var_value;
}


/* r_get_field --- get the address of a field node */

NODE **
r_get_field(NODE *n, Func_ptr *assign, bool reference)
{
	long field_num;
	NODE **lhs;

	if (assign)
		*assign = NULL;
	if (do_lint) {
		if ((fixtype(n)->flags & NUMBER) == 0) {
			lintwarn(_("attempt to field reference from non-numeric value"));
			if (n->stlen == 0)
				lintwarn(_("attempt to field reference from null string"));
		}
	}

	(void) force_number(n);
	field_num = get_number_si(n);

	if (field_num < 0)
		fatal(_("attempt to access field %ld"), field_num);

	if (field_num == 0 && field0_valid) {		/* short circuit */
		lhs = &fields_arr[0];
		if (assign)
			*assign = reset_record;
	} else
		lhs = get_field(field_num, assign);
	if (do_lint && reference && ((*lhs)->flags & NULL_FIELD) != 0)
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

static INSTRUCTION *
setup_frame(INSTRUCTION *pc)
{
	NODE *r = NULL;
	NODE *m, *f, *fp;
	NODE **sp = NULL;
	int pcount, arg_count, i, j;

	f = pc->func_body;
	pcount = f->param_cnt;
	fp = f->fparms;
	arg_count = (pc + 1)->expr_count;

	if (pcount > 0) {
		ezalloc(sp, NODE **, pcount * sizeof(NODE *), "setup_frame");
	}

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

	for (i = 0, j = arg_count - 1; i < pcount; i++, j--) {
		getnode(r);
		memset(r, 0, sizeof(NODE));
		sp[i] = r;

		if (i >= arg_count) {
			/* local variable */
			r->type = Node_var_new;
			r->vname = fp[i].param;
			continue;
		}

		m = PEEK(j); /* arguments in reverse order on runtime stack */

		if (m->type == Node_param_list)
			m = GET_PARAM(m->param_cnt);

		/* $0 needs to be passed by value to a function */
		if (m == fields_arr[0]) {
			DEREF(m);
			m = dupnode(m);
		}

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
			r->var_value = dupnode(Nnull_string);
			break;

		case Node_val:
			r->type = Node_var;
			r->var_value = m;
			break;

		default:
			cant_happen();
		}
		r->vname = fp[i].param;
	}

	stack_adj(-arg_count);	/* adjust stack pointer */

	if (pc->opcode == Op_indirect_func_call) {
		r = POP();	/* indirect var */
		DEREF(r);
	}

	frame_ptr->vname = source;	/* save current source */

	if (do_profile || do_debug)
		push_frame(frame_ptr);

	/* save current frame in stack */
	PUSH(frame_ptr);

	/* setup new frame */
	getnode(frame_ptr);
	frame_ptr->type = Node_frame;
	frame_ptr->stack = sp;
	frame_ptr->prev_frame_size = (stack_ptr - stack_bottom); /* size of the previous stack frame */
	frame_ptr->func_node = f;
	frame_ptr->vname = NULL;
	frame_ptr->reti = pc; /* on return execute pc->nexti */

	return f->code_ptr;
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
	n = func->param_cnt;
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
	ri = frame_ptr->reti;     /* execution in calling frame
	                           * resumes from ri->nexti.
	                           */
	freenode(frame_ptr);
	if (do_profile || do_debug)
		pop_frame();

	/* restore frame */
	frame_ptr = fp;
	/* restore source */
	source = fp->vname;
	fp->vname = NULL;

	return ri->nexti;
}


/* free_arrayfor --- free 'for (var in array)' related data */

static inline void
free_arrayfor(NODE *r)
{
	if (r->for_list != NULL) {
		NODE *n;
		size_t num_elems = r->for_list_size;
		NODE **list = r->for_list;
		while (num_elems > 0) {
			n = list[--num_elems];
			unref(n);
		}
		efree(list);
	}
	freenode(r);
}


/*
 * unwind_stack --- pop items off the run-time stack;
 *	'n' is the # of items left in the stack.
 */

INSTRUCTION *
unwind_stack(long n)
{
	NODE *r;
	INSTRUCTION *cp = NULL;
	STACK_ITEM *sp;

	if (stack_empty())
		return NULL;

	sp = stack_bottom + n;

	if (stack_ptr < sp)
		return NULL;

	while ((r = POP()) != NULL) {
		switch (r->type) {
		case Node_frame:
			cp = restore_frame(r);
			break;
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
			/*
			 * Check `exiting' and don't produce an error for
			 * cases like:
			 *	func     _fn0() { exit }
			 *	BEGIN { ARRAY[_fn0()] }
			 */
			if (in_main_context() && ! exiting)
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

		if (stack_ptr < sp)
			break;
	}
	return cp;
}


/* pop_fcall --- pop off the innermost frame */
#define pop_fcall()	unwind_stack(frame_ptr->prev_frame_size)

/* pop the run-time stack */
#define pop_stack()	(void) unwind_stack(0)


static inline bool
eval_condition(NODE *t)
{
	if (t == node_Boolean[false])
		return false;

	if (t == node_Boolean[true])
		return true;

	return boolval(t);
}

typedef enum {
	SCALAR_EQ_NEQ,
	SCALAR_RELATIONAL
} scalar_cmp_t;

/* cmp_scalars -- compare two nodes on the stack */

static inline int
cmp_scalars(scalar_cmp_t comparison_type)
{
	NODE *t1, *t2;
	int di;

	t2 = POP_SCALAR();
	t1 = TOP();
	if (t1->type == Node_var_array) {
		DEREF(t2);
		fatal(_("attempt to use array `%s' in a scalar context"), array_vname(t1));
	}
	di = cmp_nodes(t1, t2, comparison_type == SCALAR_EQ_NEQ);
	DEREF(t1);
	DEREF(t2);
	return di;
}

/* op_assign --- assignment operators excluding = */

static void
op_assign(OPCODE op)
{
	NODE **lhs;
	NODE *t1, *t2;
	AWKNUM x = 0.0, x1, x2;

	lhs = POP_ADDRESS();
	t1 = *lhs;
	x1 = force_number(t1)->numbr;

	t2 = TOP_SCALAR();
	x2 = force_number(t2)->numbr;
	DEREF(t2);

	switch (op) {
	case Op_assign_plus:
		x = x1 + x2;
		break;
	case Op_assign_minus:
		x = x1 - x2;
		break;
	case Op_assign_times:
		x = x1 * x2;
		break;
	case Op_assign_quotient:
		if (x2 == (AWKNUM) 0) {
			decr_sp();
			fatal(_("division by zero attempted in `/='"));
		}
		x = x1 / x2;
		break;
	case Op_assign_mod:
		if (x2 == (AWKNUM) 0) {
			decr_sp();
			fatal(_("division by zero attempted in `%%='"));
		}
#ifdef HAVE_FMOD
		x = fmod(x1, x2);
#else   /* ! HAVE_FMOD */
		(void) modf(x1 / x2, &x);
		x = x1 - x2 * x;
#endif  /* ! HAVE_FMOD */
		break;
	case Op_assign_exp:
		x = calc_exp((double) x1, (double) x2);
		break;
	default:
		break;
	}

	if (t1->valref == 1 && t1->flags == (MALLOC|NUMCUR|NUMBER)) {
		/* optimization */
		t1->numbr = x;
	} else {
		unref(t1);
		t1 = *lhs = make_number(x);
	}

	UPREF(t1);
	REPLACE(t1);
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
 * Implementation of BEGINFILE and ENDFILE requires saving an execution
 * state and the ability to return to that state. The state is
 * defined by the instruction triggering the BEGINFILE/ENDFILE rule, the
 * run-time stack, the rule and the source file. The source line is available in
 * the instruction and hence is not considered a part of the execution state.
 */


typedef struct exec_state {
	struct exec_state *next;

	INSTRUCTION *cptr;  /* either getline (Op_K_getline) or the
	                     * implicit "open-file, read-record" loop (Op_newfile).
	                     */

	int rule;           /* rule for the INSTRUCTION */

	long stack_size;    /* For this particular usage, it is sufficient to save
	                     * only the size of the call stack. We do not
	                     * store the actual stack pointer to avoid problems
	                     * in case the stack gets realloc-ed.
	                     */

	const char *source; /* source file for the INSTRUCTION */
} EXEC_STATE;

static EXEC_STATE exec_state_stack;

/* push_exec_state --- save an execution state on stack */

static void
push_exec_state(INSTRUCTION *cp, int rule, char *src, STACK_ITEM *sp)
{
	EXEC_STATE *es;

	emalloc(es, EXEC_STATE *, sizeof(EXEC_STATE), "push_exec_state");
	es->rule = rule;
	es->cptr = cp;
	es->stack_size = (sp - stack_bottom) + 1;
	es->source = src;
	es->next = exec_state_stack.next;
	exec_state_stack.next = es;
}


/* pop_exec_state --- pop one execution state off the stack */

static INSTRUCTION *
pop_exec_state(int *rule, char **src, long *sz)
{
	INSTRUCTION *cp;
	EXEC_STATE *es;

	es = exec_state_stack.next;
	if (es == NULL)
		return NULL;
	cp = es->cptr;
	if (rule != NULL)
		*rule = es->rule;
	if (src != NULL)
		*src = (char *) es->source;
	if (sz != NULL)
		*sz = es->stack_size;
	exec_state_stack.next = es->next;
	efree(es);
	return cp;
}


/* register_exec_hook --- add exec hooks in the interpreter. */

int
register_exec_hook(Func_pre_exec preh, Func_post_exec posth)
{
	int pos = 0;

	/*
	 * multiple post-exec hooks aren't supported. post-exec hook is mainly
	 * for use by the debugger.
	 */

	if (! preh || (post_execute && posth))
		return false;

	if (num_exec_hook == MAX_EXEC_HOOKS)
		return false;

	/*
	 * Add to the beginning of the array but do not displace the
	 * debugger hook if it exists.
	 */
	if (num_exec_hook > 0) {
		pos = !! do_debug;
		if (num_exec_hook > pos)
			memmove(pre_execute + pos + 1, pre_execute + pos,
					(num_exec_hook - pos) * sizeof (preh));
	}
	pre_execute[pos] = preh;
	num_exec_hook++;

	if (posth)
		post_execute = posth;

	return true;
}


/* interpreter routine when not debugging */
#include "interpret.h"

/* interpreter routine with exec hook(s). Used when debugging and/or with MPFR. */
#define r_interpret h_interpret
#define EXEC_HOOK 1
#include "interpret.h"
#undef EXEC_HOOK
#undef r_interpret


void
init_interpret()
{
	long newval;

	if ((newval = getenv_long("GAWK_STACKSIZE")) > 0)
		STACK_SIZE = newval;

	emalloc(stack_bottom, STACK_ITEM *, STACK_SIZE * sizeof(STACK_ITEM), "grow_stack");
	stack_ptr = stack_bottom - 1;
	stack_top = stack_bottom + STACK_SIZE - 1;

	/* initialize frame pointer */
	getnode(frame_ptr);
	frame_ptr->type = Node_frame;
	frame_ptr->stack = NULL;
	frame_ptr->func_node = NULL;	/* in main */
	frame_ptr->vname = NULL;

	/* initialize true and false nodes */
	node_Boolean[false] = make_number(0.0);
	node_Boolean[true] = make_number(1.0);
	if (! is_mpg_number(node_Boolean[false])) {
		node_Boolean[false]->flags |= NUMINT;
		node_Boolean[true]->flags |= NUMINT;
	}

	/*
	 * Select the interpreter routine. The version without
	 * any exec hook support (r_interpret) is faster by about
	 * 5%, or more depending on the opcodes.
	 */

	if (num_exec_hook > 0)
		interpret = h_interpret;
	else
		interpret = r_interpret;
}

