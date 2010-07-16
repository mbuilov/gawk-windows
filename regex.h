/* Definitions for data structures callers pass the regex library.
   Requires sys/types.h for size_t.
   Version 0.1.
   Copyright (C) 1985, 89, 90, 91 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifndef __REGEXP_LIBRARY
#define __REGEXP_LIBRARY



/* This defines the particular regexp syntax to use.  */
extern int obscure_syntax;


/* The following bits are used in the obscure_syntax variable to choose among
   alternative regexp syntaxes.  */

/* If this bit is set, (...) defines a group, and \( and \) are literals.
   If not set, \(...\) defines a group, and ( and ) are literals.  */
#define RE_NO_BK_PARENS	1

/* If this bit is set, then | is an alternation operator, and \| is literal. 
   If not set, then \| is an alternation operator, and | is literal.  */
#define RE_NO_BK_VBAR (1 << 1)

/* If this bit is not set, then + and ? are operators, and \+ and \? are
     literals. 
   If set, then \+ and \? are operators and + and ? are literals.  */
#define RE_BK_PLUS_QM (1 << 2)
 
/* If this bit is set, then | binds tighter than ^ or $.
   If not set, the contrary.  */
#define RE_TIGHT_ALT (1 << 3)

/* If this bit is set, newline is an alternation operator.
   If not set, then newline is literal.  */
#define RE_NEWLINE_ALT (1 << 4)

/* If this bit is set, then special characters are always special
     regardless of where they are in the pattern.
   If this bit is not set, then special characters are special only in
     some contexts; otherwise they are ordinary.  Specifically, 
     
	*  + ?  and intervals are only special when not after the beginning,
		open-group, or alternation operator.  */
#define RE_CONTEXT_INDEP_OPS (1 << 5)

/* If this bit is not set, then \ inside a bracket expression is literal.
   If set, then such a \ quotes the following character.  */
#define RE_AWK_CLASS_HACK (1 << 6)

/* If this bit is set, then either \{...\} or {...} defines an
     interval, depending on RE_NO_BK_BRACES. 
   If not set, then \{, \}, {, and } are literals.  */
#define RE_INTERVALS (1 << 7)

/* If this bit is not set, then \{ and \} defines an interval,
     and { and } are literals.
   If set, then { and } defines an interval, and \{ and \} are literals.  */
#define RE_NO_BK_BRACES (1 << 8)

/* If this bit is set, then character classes are supported.  They are:
     [:alpha:],	[:upper:], [:lower:],  [:digit:], [:alnum:], [:xdigit:],
     [:space:], [:print:], [:punct:], [:graph:], and [:cntrl:].
   If not set, then character classes are not supported.  */
#define RE_CHAR_CLASSES (1 << 9)

/* If this bit is set, then period doesn't match a null.
   If not set, then it does.  */
#define RE_DOT_NOT_NULL (1 << 10)

/* If this bit is set, then [^...] doesn't match a newline.
   If not set, then it does.  */
#define RE_HAT_LISTS_NOT_NEWLINE (1 << 11)

/* If this bit is set, then back references are not recognized.
   If not set, then they are.  */
#define RE_NO_BK_REFS (1 << 12)

/* If this bit is set, then all back references must refer to a preceding
     subexpression.
   If not set, then a back reference to a nonexistent subexpression is
     treated as literal characters.  */ 
#define RE_NO_MISSING_BK_REF (1 << 13)

/* If this bit is set, then *, +, ?, and { cannot be first in an re or
     immediately after OR or BEGINGROUP.  Furthermore, OR cannot be
     first or last in an re, or immediately follow another OR or
     BEGINGROUP.  Also, ^ cannot appear in a nonleading position and $
     cannot appear in a nontrailing position (outside of bracket
     expressions, that is).  */
#define RE_CONTEXT_INVALID_OPS (1 << 14)

/* If this bit is set, then +, ? and | aren't recognized as operators.
   If not set, then they are.  */
#define RE_LIMITED_OPS (1 << 15)

/* If this bit is set, then an ending range point has to collate higher
     than or equal to the starting range point.
   If not set, then when the ending range point collates higher than the
     starting range point, the range is considered to be empty.  */ 
#define RE_NO_EMPTY_RANGES (1 << 16)

/* If this bit is set, then neither the match-beginning-of-line nor
     the match-end-of-line operator match a newline.  
   If not set, then these operators can match a newline.  */
#define RE_NO_ANCHOR_AT_NEWLINE (1 << 17)
 
/* If this bit is set, then you can't have empty groups.
   If not set, then you can.  */
#define RE_NO_EMPTY_GROUPS (1 << 18)

/* If this bit is set, then you can't have empty alternatives.
   If not set, then you can.  */
#define RE_NO_EMPTY_ALTS (1 << 19)

/* If this bit is set, then you can't have more than one non-interval
     repetition operators (i.e., `*', `+' and `?') in a row, e.g., as in
     `a*+?*'.
   If not set, then you can.  */
#define RE_NO_CONSECUTIVE_REPEATS (1 << 20)
  

/* If this bit is set, then ignore anchors inside groups which in turn
     are operated on by repetion operators.
   If not set, then don't.  */
#define RE_REPEATED_ANCHORS_AWAY (1 << 21)

/* If this bit is set, then the match-any-character operator (.) matches
     a newline.
   If not set, then it doesn't.  */
#define RE_DOT_NEWLINE (1 << 22)

/* If this bit is set, then '^' and '$' can be anchors only at the
     beginning or the end of the pattern. 
   If not set, then they don't have to be at the beginning or end of the
     pattern to be anchors.  */
#define RE_ANCHORS_ONLY_AT_ENDS (1 << 23)

/* If this bit is set, then Regex considers an unmatched close-group
     operator to be the ordinary character parenthesis.
   If not set, then an unmatched close-group operator is invalid.  */
#define RE_UNMATCHED_RIGHT_PAREN_ORD (1 << 24)

/* If this bit is set, then ^ cannot appear in a nonleading position and
   $ cannot appear in a nontrailing position (outside of bracket
   expressions, that is).  */
#define RE_CONTEXT_INVALID_ANCHORS (1 << 25)

/* If this bit is set, then ^ and $ are always anchors, regardless of
     their positions in a regular expression.  
   If this bit is not set, then ^ is an anchor only if in a leading
     position and $ is one only if in a trailing position.  Specifically, 
     
        ^  is in a leading position if at the beginning of a regular
           expression , or after an open-group or an alternation operator;

        $  is in a trailing position if at the end of a regular
	   expression, or before close-group or an alternation operator.
*/
#define RE_CONTEXT_INDEP_ANCHORS (1 << 26)

/* If this bit is set, then the searching and matching routines will
     allocate enough register space to accommodate the number of groups
     in the regular expression.
   If this bit is not set, then the user must allocate the space.  */
#define RE_ALLOCATE_REGISTERS (1 << 27)


/* Define combinations of the above bits for the standard possibilities.  */ 
#define RE_SYNTAX_EMACS 0

#define RE_SYNTAX_AWK 							\
  (RE_NO_BK_PARENS | RE_NO_BK_VBAR | RE_ALLOCATE_REGISTERS		\
   | RE_AWK_CLASS_HACK)

#define RE_SYNTAX_POSIX_AWK	RE_SYNTAX_AWK

#define RE_SYNTAX_GREP 							\
  (RE_BK_PLUS_QM | RE_NEWLINE_ALT | RE_ALLOCATE_REGISTERS)

#define RE_SYNTAX_EGREP 						\
  (RE_NO_BK_PARENS | RE_NO_BK_VBAR | RE_CONTEXT_INDEP_OPS 		\
   | RE_CONTEXT_INDEP_ANCHORS | RE_NEWLINE_ALT | RE_ALLOCATE_REGISTERS)

#define RE_SYNTAX_POSIX_BASIC						\
  (RE_INTERVALS | RE_CHAR_CLASSES | RE_DOT_NOT_NULL			\
   | RE_NO_MISSING_BK_REF | RE_LIMITED_OPS | RE_NO_EMPTY_RANGES 	\
   | RE_NO_ANCHOR_AT_NEWLINE | RE_DOT_NEWLINE | RE_ALLOCATE_REGISTERS)

#define RE_SYNTAX_POSIX_EXTENDED					\
  (RE_INTERVALS | RE_NO_BK_BRACES | RE_NO_BK_VBAR			\
   | RE_NO_BK_PARENS | RE_CHAR_CLASSES | RE_CONTEXT_INVALID_OPS 	\
   | RE_NO_BK_REFS | RE_NO_EMPTY_RANGES	| RE_UNMATCHED_RIGHT_PAREN_ORD	\
   | RE_DOT_NOT_NULL | RE_NO_EMPTY_GROUPS | RE_NO_EMPTY_ALTS		\
   | RE_NO_ANCHOR_AT_NEWLINE | RE_DOT_NEWLINE | RE_CONTEXT_INDEP_ANCHORS\
   | RE_ALLOCATE_REGISTERS)



/* Maximum number of duplicates an interval can allow.  */
#define RE_DUP_MAX  ((1 << 15) - 1) 


/* POSIX cflags bits (i.e., information for regcomp).  */

/* If this bit is set, then use extended regular expression syntax.
   If not set, then use basic regular expression syntax.  */
#define REG_EXTENDED 1

/* If this bit is set, then (line 526, p.687 of POSIX 1003.2/D10)
     newline loses its special significance; i.e., anchors do not match at
     newlines in the string.
   If not set, then anchors do match at newlines.  */
#define REG_NEWLINE (1 << 1)

/* If this bit is set, then ignore case when matching.
   If not set, then case is significant.  */
#define REG_ICASE (1 << 2)
 
/* If this bit is set, then report only success or fail in regexec ().
   If not set, then return nonzero indicating either not match or an error.  */
#define REG_NOSUB (1 << 3)


/* POSIX eflags bits (i.e., information for regexec).  */

/* If this bit is set, then the string's first character is not the
     beginning of a line, so the beginning-of-line anchor shouldn't
     match it.
   If not set, then the string's first character can match the
     beginning-of-line anchor.  */
#define REG_NOTBOL 1

/* If this bit is set, then the string's last character is not the
     end of a line, so the end-of-line anchor shouldn't match it.
   If not set, then the string's last character can match the
     end-of-line anchor.  */
#define REG_NOTEOL (1 << 1)


/* POSIX regexec return error value.  */

#define REG_NOMATCH 1	/* Didn't find a match.  */

/* POSIX regcomp return error codes.  */

#define REG_BADPAT 2	/* Found an invalid pattern.  */
#define REG_ECOLLATE 3  /* Not implemented.  */
#define REG_ECTYPE 4	/* Found an invalid character class name.  */
#define REG_EESCAPE 5 	/* Found a trailing backslash.  */
#define REG_ESUBREG 6   /* Found an invalid back reference.  */
#define REG_EBRACK 7    /* Found an unmatched left bracket.  */
#define REG_EPAREN 8 	/* Found a parentheses imbalance.  */ 
#define REG_EBRACE 9	/* Found an unmatched \{.  */
#define REG_BADBR 10	/* Found invalid contents of \{\}.  */
#define REG_ERANGE 11 	/* Found invalid range end.  */
#define REG_ESPACE 12   /* Ran out of memory.  */
#define REG_BADRPT 13	/* No preceding re for repetition op.  */
#define REG_ENEWLINE 14 /* Not implemented.  */

/* Some regcomp codes we've added.  */
#define REG_NOERROR 0	/* No error.  */
#define REG_EEND 15
#define REG_ESIZE 16

 


/* This data structure represents a compiled pattern.  Before calling
   the pattern compiler, the fields `buffer', `allocated', `fastmap',
   `translate', and `no_sub' can be set.  After the pattern has been
   compiled, the `re_nsub' field is available.  All other fields are
   private to the regex routines.  */

/* If this changes, change documentation in regex.texinfo.  */

struct re_pattern_buffer
{
	/* Space that holds the compiled pattern.  */
  char *buffer;

	/* Number of bytes to which `buffer' points.  */
  long allocated;

	/* Number of bytes actually used in `buffer'.  */
  long used;	

        /* Syntax setting with which the pattern was compiled.  */
  int syntax;

        /* Pointer to a fastmap, if any, otherwise zero.  re_search uses
           the fastmap, if there is one, to skip over impossible
           starting points for matches.  */
  char *fastmap;

        /* Either a translate table to apply to all characters before
           comparing them, or zero for no translation.  The translation
           is applied to a pattern when it is compiled and to a string
           when it is matched.  */
  char *translate;	      

	/* Number of subexpressions found by the compiler.  */
  size_t re_nsub;

        /* Set to 1 by re_compile_fastmap if this pattern can match the
           null string; 0 prevents the searcher from matching it with
           the null string.  Set to 2 if it might match the null string
           either at the end of a search range or just before a
           character listed in the fastmap.  */
  char can_be_null;


  /* The remaining fields are all one-bit booleans.  */
  
        /* Set to zero when regex_compile compiles a pattern; set to one
           by re_compile_fastmap when it updates the fastmap, if any.  */
  unsigned fastmap_accurate : 1;

        /* If set, regexec reports only success or failure and does not
           return anything in pmatch[].  */
  unsigned no_sub : 1;

        /* If set, a beginning-of-line anchor never matches.  */ 
  unsigned not_bol : 1;

        /* Similarly for an end-of-line anchor.  */
  unsigned not_eol : 1;

        /* If set, and the regs argument is nonzero, the GNU
	   matching and searching functions return information 
           for as many registers as needed to report about the 
           whole pattern and all its subexpressions.  If not set, 
           and the regs argument is nonzero, then the functions
           return information for regs->num_regs registers.  */
  unsigned return_default_num_regs : 1;
};

typedef struct re_pattern_buffer regex_t;


/* search.c (search_buffer) in Emacs needs this one value.  It is
   defined both in `regex.c' and here.  */
#define RE_EXACTN_VALUE 1



/* struct re_registers: Structure to store register contents data in.  

   (If change comments here, change in regex.texinfo also.)
   
   Some groups in a regular expression match (possibly empty) substrings
   of the string that regular expression matched.  The matcher remembers
   the beginning and ending point of the substring matched by each
   group.  To get what they matched, pass the address of a structure of
   this type to a GNU matching or searching function.

   When you call a GNU matching and searching function, it stores
   information into this structure according to the following (in all
   examples below, `(' represents the open-group and `)' the
   close-group operator):

   If the regular expression has an i-th group that matches a substring
   of string, then the function sets REGS->start[i] to the index in
   string where the substring matched by the i-th group begins, and
   REGS->end[i] to the index just beyond that substring's end.  The
   function sets REGS->start[0] and REGS->end[0] to analogous
   information about the entire pattern.

   For example, when you match the pattern `((a)(b))' with the string
   `ab', you get:

      0 in REGS->start[0] and 2 in REGS->end[0] 
      0 in REGS->start[1] and 2 in REGS->end[1] 
      0 in REGS->start[2] and 1 in REGS->end[2] 
      1 in REGS->start[3] and 2 in REGS->end[3] 

   If a group matches more than once (as it might if followed by, e.g.,
   a repetition operator), then the function reports the information
   about what the group @emph{last matched.

   For example, when you match the string `aa' with the pattern `(a)*',
   you get:

      0 in REGS->start[0] and 2 in REGS->end[0] 
      1 in REGS->start[1] and 2 in REGS->end[1] 


   If the i-th group does not participate in a successful match, e.g.,
   it is an alternative not taken or a repetition operator allows zero
   repetitions of it, then the function sets REGS->start[i] and
   REGS->end[i] to -1.

   For example, when you match the string `b' with the pattern `(a)*b',
   you get:

      0 in REGS->start[0] and 1 in REGS->end[0] 
      -1 in REGS->start[1] and -1 in REGS->end[1] 


   If the i-th group matches a zero-length string, then the function
   sets REGS->start[i] and REGS->end[i] to the index just beyond that
   zero-length string.

   For example, when you match the string `b' with the pattern `(a*)b',
   you get:

      0 in REGS->start[0] and 1 in REGS->end[0] 
      0 in REGS->start[1] and 0 in REGS->end[1] 

   The function sets REGS->start[0] and REGS->end[0] to analogous
   information about the entire pattern.

   For example, when you match the empty string with the pattern `(a*)',
   you get:

      0 in REGS->start[0] and 0 in REGS->end[0] 
      0 in REGS->start[1] and 0 in REGS->end[1] 

   If an i-th group contains a j-th group and the function reports a
   match of the i-th group, then it records in REGS->start[j] and
   REGS->end[j] the last match (if it matched) of the j-th group.

   For example, when you match the string `abb' with the pattern
   `((a*)b)*, group 2' last matches the empty string, so you get:

      0 in REGS->start[0] and 3 in REGS->end[0] 
      2 in REGS->start[1] and 3 in REGS->end[1] 
      2 in REGS->start[2] and 2 in REGS->end[2] 

   When you match the string `abb' with the pattern `((a)*b)*', group 2
   doesn't participate in the last match, so you get:

      0 in REGS->start[0] and 3 in REGS->end[0] 
      2 in REGS->start[1] and 3 in REGS->end[1] 
      0 in REGS->start[2] and 1 in REGS->end[2] 

   If an i-th group contains a j-th group and the function sets
   REGS->start[i] and REGS->end[i] to -1, then it also sets REGS->start[j] 
   and REGS->end[j] to -1.

   For example, when you match the string `c' with the pattern
   `((a)*b)*c', you get:

      0 in REGS->start[0] and 1 in REGS->end[0] 
      -1 in REGS->start[1] and -1 in REGS->end[1] 
      -1 in REGS->start[2] and -1 in REGS->end[2] 
*/

struct re_registers
{
  unsigned num_regs;
  int *start;
  int *end;
};


/* POSIX specification for registers.  See comments for struct
   re_registers for how this is used and read `POSIX' for `GNU',
   `PMATCH' for `REGS', `PMATCH[i].rm_so' for `REGS->start' and
   `PMATCH[i].rm_eo' for `REGS->end'.  */

typedef off_t regoff_t;

typedef struct
{
  regoff_t rm_so;  /* Byte offset from string's start to substring' start.  */
  regoff_t rm_eo;  /* Byte offset from string's end to substring' end.  */
} regmatch_t;



#ifdef __STDC__

/* Compile the regular expression PATTERN, with length LENGTH
   and syntax given by the global `obscure_syntax', into the buffer
   BUFFER.  Return NULL if successful, and an error string if not.  */

extern char *re_compile_pattern (const char *pattern, const int length,
                                 struct re_pattern_buffer *buffer);


/* Compile a fastmap for the compiled pattern in BUFFER; used to
   accelerate searches.  Return 0 if successful and -2 if was an
   internal error.  */

extern int re_compile_fastmap (struct re_pattern_buffer *buffer);


/* Search in the string STRING (with length LENGTH) for the pattern
   compiled into BUFFER.  Start searching at position START, for RANGE
   characters.  Return the starting position of the match or -1 for no
   match, or -2 for an internal error.  Also return register
   information in REGS (if REGS is non-null).  */

extern int re_search (struct re_pattern_buffer *buffer,
                      const char *string, const int length,
                      const int start, const int range, 
		      struct re_registers *regs);

/* Like `re_search', but search in the concatenation of STRING1 and
   STRING2.  Also, stop searching at index START + STOP.  */

extern int re_search_2 (struct re_pattern_buffer *buffer,
                        const char *string1, const int length1,
		        const char *string2, const int length2,
                        const int start, const int range, 
                        struct re_registers *regs,
                        const int stop);

/* Like `re_search', but return how many characters in STRING the regexp
   in BUFFER matched, starting at position START.  */

extern int re_match (const struct re_pattern_buffer *buffer,
                     const char *string, const int length,
                     const int start, struct re_registers *regs);


/* Relates to `re_match' as `re_search_2' relates to `re_search'.  */

extern int re_match_2 (const struct re_pattern_buffer *buffer,
                       const char *string1, const int length1,
	               const char *string2, const int length2,
                       const int start,
                       struct re_registers *regs,
                       const int stop);


/* 4.2 bsd compatibility.  */
extern char *re_comp (const char *);
extern int re_exec (const char *);

extern int regcomp (regex_t *preg, const char *pattern, int cflags);
extern int regexec (const regex_t *preg, const char *string, size_t nmatch,
		    regmatch_t pmatch[], int eflags);
extern size_t re_gerror (int errcode, const regex_t *preg, char* errbuf, 
			size_t errbuf_size);
extern void re_gfree (regex_t *preg);

#else /* not __STDC__ */

/* Support old C compilers.  */
#define const

extern char *re_compile_pattern ();
extern int re_search (), re_search_2 ();
extern int re_match (), re_match_2 ();

/* 4.2 bsd compatibility.  */
extern char *re_comp ();
extern int re_exec ();

extern int regcomp ();
extern int regexec ();
extern size_t re_gerror ();
extern void re_gfree ();

#endif /* __STDC__ */


#ifdef SYNTAX_TABLE
extern char *re_syntax_table;
#endif

#endif /* not __REGEXP_LIBRARY */



/*
Local variables:
make-backup-files: t
version-control: t
trim-versions-without-asking: nil
End:
*/
