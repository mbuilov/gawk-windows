/* Extended regular expression matching and search library.
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


/* To test, compile with -Dtest.  This Dtestable feature turns this into
   a self-contained program which reads a pattern, describes how it
   compiles, then reads a string and searches for it.

   On the other hand, if you compile with both -Dtest and -Dcanned you
   can run some tests we've already thought of.  */

#ifdef GAWK
#include "config.h"
#endif

#ifdef REGEX_MALLOC

#define REGEX_ALLOCATE malloc
#define REGEX_REALLOCATE(source, size) (realloc (source, size))

#else /* not REGEX_MALLOC  */


/* Make alloca work the best possible way.  */
#ifdef __GNUC__
#define alloca __builtin_alloca
#else
#ifdef sparc
#include <alloca.h>
#else 
#ifdef _AIX
  #pragma alloca
#else /* not __GNUC__ or sparc or _AIX */
char *alloca ();
#endif  /* _AIX  */ 
#endif  /* sparc  */ 
#endif  /* not  __GNUC__  */

/* Still not defined (REGEX_MALLOC)  */

#define REGEX_ALLOCATE alloca

/* Requires a `void *destination' declared.  */
#define REGEX_REALLOCATE(source, size)					\
  (destination = alloca (size),						\
   bcopy (source, destination, size),					\
   destination)

#endif /* not defined (REGEX_MALLOC)  */



#ifdef emacs

/* The `emacs' switch turns on certain special matching commands
  that make sense only in emacs. */

#include "config.h"
#include "lisp.h"
#include "buffer.h"
#include "syntax.h"

/* Emacs uses `NULL' as a predicate.  */
#undef NULL


#else  /* not emacs */


#include <sys/types.h>  /* POSIX types.  */

#if defined(GAWK) || defined (USG) || defined (POSIX) || defined (STDC_HEADERS)
#ifndef BSTRING
#include <string.h>
#define bcopy(s,d,n)	memcpy ((d), (s), (n))
#define bcmp(s1,s2,n)	memcmp ((s1), (s2), (n))
#define bzero(s,n)	memset ((s), 0, (n))
#endif /* not BSTRING  */
#endif /* USG or POSIX or STDC_HEADERS  */

#if defined (STDC_HEADERS)
#include <stdlib.h>
#else
#ifdef __STDC__
void *malloc (size_t);
void *realloc (void *, size_t);
#else /* not __STDC__  */
char *malloc ();
char *realloc ();
#endif /* not __STDC__  */
#endif  /* not (POSIX or STDC_HEADERS) */



/* How many characters in the character set.  */
#define CHAR_SET_SIZE  256


/* Define the syntax stuff, so we can do the \<, \>, etc.  */

/* This must be nonzero for the wordchar and notwordchar pattern
   commands in re_match_2.  */
#ifndef Sword 
#define Sword 1
#endif /* not Sword  */

#define SYNTAX(c) re_syntax_table[c]


#ifdef SYNTAX_TABLE

extern char *re_syntax_table;

#else /* not SYNTAX_TABLE */

static char re_syntax_table[CHAR_SET_SIZE];

static void
init_syntax_once ()
{
   register int c;
   static int done = 0;

   if (done)
     return;

   bzero (re_syntax_table, sizeof re_syntax_table);

   for (c = 'a'; c <= 'z'; c++)
     re_syntax_table[c] = Sword;

   for (c = 'A'; c <= 'Z'; c++)
     re_syntax_table[c] = Sword;

   for (c = '0'; c <= '9'; c++)
     re_syntax_table[c] = Sword;

   re_syntax_table['_'] = Sword;

   done = 1;
}

#endif /* not SYNTAX_TABLE */
#endif /* not emacs */

/* We write fatal error messages on standard error.  */
#include <stdio.h>

/* isalpha(3) etc. are used for the character classes.  */
#include <ctype.h>

/* Sequents are missing isgraph.  */
#ifdef sequent
#define ISGRAPH_MISSING
#endif

#ifdef ISGRAPH_MISSING
#define isgraph(c) (isprint (c) && !isspace (c))
#endif


/* Get the interface, including the syntax bits.  */
#include "regex.h"

/* We will need this constant several times.  */
#define BYTEWIDTH 8




/* These are the command codes that appear in compiled regular
   expressions, one per byte.  Some command codes are followed by
   argument bytes.  A command code can specify any interpretation
   whatsoever for its arguments.  Zero-bytes may appear in the compiled
   regular expression.

   The value of `exactn' is needed in search.c (search_buffer) in emacs.
   So regex.h defines a symbol `RE_EXACTN_VALUE' to be 1; the value of
   `exactn' we use here must also be 1.  */

enum regexpcode
  {
    no_op=0,
    exactn=1,		/* Followed by one byte giving n, then by n
                           literal bytes.  */ 
    begline,		/* Fail unless at beginning of line.  */
    endline,		/* Fail unless at end of line.  */
    endline_in_repeat,	/* If in trailing position, turn into an endline, 
			   otherwise, turn into a no_op.  This should
                           never end up in the final compiled pattern!  */
    endline_before_newline,/* If after an endline, don't that endline turn into
			   an exactn for '$' when RE_CONTEXT_INDEP_ANCHORS
                           is set.  Should never end up in the compiled
                           pattern! */
    repeated_endline_before_newline, /* A combination of above two.  */
    no_pop_jump,	/* Followed by two byte relative address to
			   which to jump.  */ 
    jump_past_next_alt, /* Same as no_pop_jump, but don't jump if the
		           current group (the largest-numbered active
                           one) hasn't matched anything.  */
    on_failure_jump,	/* Followed by two byte relative address of 
                           place to resume at in case of failure.  */
    pop_failure_jump,	/* Throw away latest failure point and then jump to 
                            address.  */
    maybe_pop_jump,
                        /* Like jump but change to pop_failure_jump 
                          only if know won't have to backtrack to
                          match; otherwise change to no_pop_jump.
                          This is used to jump back to the
                          beginning of a repeat.  If what follows
                          this jump clearly won't match what the
                          repeat does, such that we can be sure
                          that there is no use backtracking out of
                          repetitions already matched, then we
                          change it to a pop_failure_jump.  */
    dummy_failure_jump,	/* Jump, and push a dummy failure point. This 
                           failure point will be thrown away if an
                           attempt is made to use it for a failure. A
                           `+' construct makes this before the first
                           repeat.  Also use it as an intermediary kind
                           of jump when compiling an alternative.  */
    succeed_n,		/* Used like on_failure_jump except has to
                           succeed n times; The two-byte relative
                           address following it is useless until then.
                           The address is followed by two bytes
                           containing n.  */
    no_pop_jump_n,	/* Similar to no_pop_jump, but jump n times
                           only; also the relative address following is
                           in turn followed by yet two more bytes
                           containing n.  */
    set_number_at,	/* Set the following relative location (two
                           bytes) to the subsequent (two-byte) number.  */
    anychar,		/* Matches any (more or less) character.  */
    charset,		/* Matches any one char belonging to specified set.
                           First following byte is number of bitmap
                           bytes.  Then come bytes for a bitmap saying
                           which chars are in.  Bits in each byte are
                           ordered low-bit-first.  A character is in the
                           set if its bit is 1.  A character too large
                           to have a bit in the map is automatically not
                           in the set.  */
    charset_not,	/* Same parameters as charset, but match any
                           character that is not one of those specified.  */
    start_memory,	/* Start remembering the text that is matched, for
                           storing in a memory register.  Followed by
                           one byte containing the register number.
                           Register numbers will be in the range 0
                           through one less than the pattern buffer's
                           re_nsub field.  */
    stop_memory,	/* Stop remembering the text that is matched
                           and store it in a memory register.  Followed
                           by one byte containing the register number.
                           Register numbers will be in the range 0
                           through one less than the pattern buffer's
                           re_nsub field.  */
    duplicate,		/* Match a duplicate of something remembered.
                           Followed by one byte containing the register
                           number.  */
    before_dot,		/* Succeeds if before point.  */
    at_dot,		/* Succeeds if at point.  */
    after_dot,		/* Succeeds if after point.  */
    begbuf,		/* Succeeds if at beginning of buffer.  */
    endbuf,		/* Succeeds if at end of buffer.  */
    wordchar,		/* Matches any word-constituent character.  */
    notwordchar,	/* Matches any char that is not a word-constituent.  */
    wordbeg,		/* Succeeds if at word beginning.  */
    wordend,		/* Succeeds if at word end.  */
    wordbound,  	/* Succeeds if at a word boundary.  */
    notwordbound,	/* Succeeds if not at a word boundary.  */
    syntaxspec,		/* Matches any character whose syntax is specified.
                           followed by a byte which contains a syntax
                           code, e.g., Sword.  */
    notsyntaxspec	/* Matches any character whose syntax differs from
                           that specified.  */
  };



#ifdef CHAR_UNSIGNED	/* for, e.g., IBM RT */
#define SIGN_EXTEND_CHAR(c) (((c)^128) - 128) /* As in Harbison and Steele.  */
#endif
#ifndef SIGN_EXTEND_CHAR
#define SIGN_EXTEND_CHAR	/* As nothing.  */
#endif



/* Store NUMBER in two contiguous bytes starting at DESTINATION.  */

#define STORE_NUMBER(destination, number)				\
  do {(destination)[0] = (number) & 0377;				\
   (destination)[1] = (number) >> 8; 					\
  } while (0)


/* Same as STORE_NUMBER, except increment the destination pointer to
   the byte after where the number is stored.  Watch out that values for
   DESTINATION such as p + 1 won't work, whereas p will.  */

#define STORE_NUMBER_AND_INCR(destination, number)			\
  do { STORE_NUMBER(destination, number);				\
    (destination) += 2;							\
  } while (0)






/* Put into DESTINATION a number stored in two contingous bytes starting
   at SOURCE.  */

#define EXTRACT_NUMBER(destination, source)				\
  do { (destination) = *(source) & 0377;				\
    (destination) += SIGN_EXTEND_CHAR (*(char *)((source) + 1)) << 8; 	\
  } while (0)

int
extract_number (source)
  unsigned char *source;
{
  int answer;
  int i_temp = * (char *) (source + 1);
  char c_temp = * (char *) (source + 1);
  
  i_temp = SIGN_EXTEND_CHAR (i_temp);
  c_temp = SIGN_EXTEND_CHAR (c_temp);
  
  i_temp <<= 8;
  c_temp <<= 8;

  answer = *source & 0377;
  answer += (SIGN_EXTEND_CHAR (*(char *)((source) + 1))) << 8;
  
  return answer;
}


/* Same as EXTRACT_NUMBER, except increment the pointer for source to
   point to second byte of SOURCE.  Note that SOURCE has to be a value
   such as p, not, e.g., p + 1. */

#define EXTRACT_NUMBER_AND_INCR(destination, source)			\
  do { EXTRACT_NUMBER (destination, source);				\
    (source) += 2; 							\
  } while (0)


void
extract_number_and_incr (destination, source)
  int *destination;
  unsigned char **source;
{ 
  *destination = extract_number (*source);
  *source += 2;
}



typedef enum { false = 0, true = 1 } boolean;

/* Number of failure points for which to initially allocate space
   when matching.  If this number is exceeded, we allocate more space---
   so it is not a hard limit.  */

#ifndef INIT_FAILURE_ALLOC
#define INIT_FAILURE_ALLOC 5
#endif


/* Specify the precise syntax of regexps for compilation.  This provides
   for compatibility for various utilities which historically have
   different, incompatible syntaxes.

   The argument SYNTAX is a bit-mask comprised of the various bits
   defined in regex.h.  */

int
re_set_syntax (syntax)
  int syntax;
{
  int ret;

  ret = obscure_syntax;
  obscure_syntax = syntax;
  return ret;
}

/* Set by re_set_syntax to the current regexp syntax to recognize.  */
int obscure_syntax = 0;



/* Routine used by re_compile_pattern, re_comp and regcomp.  */

#ifdef __STDC__
static char *regex_compile (const char *pattern, const int size,
                            const int syntax, struct re_pattern_buffer *bufp);
#else
static char *regex_compile ();
#endif



/* re_compile_pattern takes a regular-expression string and converts it
   into a buffer full of byte commands for matching.

   PATTERN   is the address of the pattern string.
   SIZE      is the length of it.

   BUFP      is a struct re_pattern_buffer * whose pertinent fields are
             mentioned below:

             It has a char * field called BUFFER which points to the
             space where this routine will put the compiled pattern; the
             user can either allocate this using malloc (whereupon they
             should set the long field ALLOCATED to the number of bytes
             malloced) or set ALLOCATED to 0 and let the routine
             allocate it.  The routine may use realloc to enlarge the
             buffer space.

             If the user wants to translate all ordinary elements in the
             compiled pattern, they should set the char * field
             TRANSLATE to a translate table, otherwise, they should set
             it to 0.

             The routine sets the int field SYNTAX to the value of the
             global variable `obscure_syntax'.

             It returns in the long field USED how many bytes long the
             compiled pattern is.

             It returns 0 in the char field FASTMAP_ACCURATE, on
             the assumption that the user usually doesn't compile the
             same pattern twice and that consequently any fastmap in the
             pattern buffer is inaccurate.

             In the size_t field RE_NSUB, it returns the number of
             subexpressions it found in PATTERN.

   Returns 0 if the pattern was valid and an error string if it wasn't.   */


char *
re_compile_pattern (pattern, size, bufp)
     const char *pattern;
     const int size;
     struct re_pattern_buffer *bufp;
{
  bufp->return_default_num_regs = (obscure_syntax & RE_ALLOCATE_REGISTERS) > 0;

  return regex_compile (pattern, size, obscure_syntax, bufp);
}     



/* Macros for regex_compile.  */

#define CHAR_CLASS_MAX_LENGTH  6


/* Fetch the next character in the uncompiled pattern---translating it 
   if necessary.  */

#define PATFETCH(c)							\
  do {if (p == pend) goto end_of_pattern;				\
    c = * (unsigned char *) p++;					\
    if (translate) 							\
    c = translate[c]; 							\
  } while (0)

/* Fetch the next character in the uncompiled pattern, with no
   translation.  */

#define PATFETCH_RAW(c)							\
  do {if (p == pend) goto end_of_pattern;				\
    c = * (unsigned char *) p++; 					\
  } while (0)

#define PATUNFETCH p--


/* Pattern offset stuff.  */

#define INIT_PATTERN_OFFSETS_LIST_SIZE 32

typedef short pattern_offset_type;

typedef struct {
  pattern_offset_type *offsets;
  unsigned size;
  unsigned avail;
} pattern_offsets_list_type;

#define PATTERN_OFFSETS_LIST_PTR_FULL(pattern_offsets_list_ptr)		\
  (pattern_offsets_list_ptr->avail == pattern_offsets_list_ptr->size)


/* Anchor and op list stuff.  */

typedef pattern_offsets_list_type anchor_list_type;
typedef pattern_offsets_list_type op_list_type;



/* Bits list declaration.  An arbitrarily long string of bits.  */

typedef struct {
   unsigned *bits;
   unsigned size;
} bits_list_type;


/* Bits list macros.  See below for routines.  */

#define BITS_BLOCK_SIZE (sizeof (unsigned) * BYTEWIDTH)
#define BITS_BLOCK(position) ((position) / BITS_BLOCK_SIZE)
#define BITS_MASK(position) (1 << ((position) % BITS_BLOCK_SIZE))


/* Initialize BITS_LIST (of type bits_list_type) to have one bits
   block.  Mostly analogous to routine init_bits_list, but, if
   REGEX_MALLOC is not defined, uses `alloca' instead of `malloc'.  This
   is because using malloc in re_search* or re_match* could cause core
   leaks when C-g is used in Emacs, plus malloc's slower and causes
   storage fragmentation.  This has to be a macro because the results of
   `alloca' disappear at the end of the routine it's in.  (If for some
   reason you delete this explanation, please put it in the comment for
   the failure stack.)
   
   Return 1 if there's enough memory to do so and 0 if there isn't.  */

#define INIT_BITS_LIST(bits_list)					\
  (bits_list.bits = (unsigned *) REGEX_ALLOCATE (sizeof (unsigned)),	\
   bits_list.bits == NULL 						\
   ? 0									\
   : (bits_list.size = BITS_BLOCK_SIZE,					\
      bits_list.bits[0] = 0,						\
      1))
 

/* Extend BITS_LIST_PTR (of type bits_list_type) by one bits block.
   Return 1 if there's enough memory to do so and 0 if there isn't.
   Analogous to routine extend_bits_list, but uses alloca instead of
   realloc, for reasons stated above in INIT_BITS_LIST's comment.  
   
   Because REGEX_REALLOCATE requires a declaration of `void
   *destination', so does this.  */
   

#define EXTEND_BITS_LIST(bits_list)					\
  (bits_list.bits							\
    = (unsigned *) REGEX_REALLOCATE (bits_list.bits, 			\
			bits_list.size / BYTEWIDTH 			\
                          + BITS_BLOCK_SIZE / BYTEWIDTH),		\
   bits_list.bits == NULL						\
   ? 0									\
   : (bits_list.size += BITS_BLOCK_SIZE,				\
      bits_list.bits[(bits_list.size/BITS_BLOCK_SIZE) - 1] = 0,		\
      1))


/* Set the bit for a positive POSITION in BITS_LIST_PTR to VALUE, which,
   in turn, can only be 0 or 1.

   Returns 1 	  if can set the bit.
	   0 	  if ran out of memory allocating (if necessary) room for it.  
	   value  if the value is invalid (i.e., not 0 or 1).  
           
           Because EXTENT_BITS_LIST requires a declaration of `void
           *destination', so does this.  */
   
#define SET_BIT_TO_VALUE(bits_list, position, value)			\
  (position > bits_list.size - 1					\
    && !EXTEND_BITS_LIST (bits_list)					\
    ? 0									\
    : (value == 1							\
       ? (bits_list.bits[BITS_BLOCK (position)] 			\
            |= BITS_MASK (position), 1)					\
       : (value == 0							\
          ? (bits_list.bits[BITS_BLOCK (position)]			\
             &= ~(BITS_MASK (position)), 1)				\
          : value)							\
       ))



/* Compile stack stuff.  */

typedef struct {
  pattern_offset_type laststart_offset;  
  pattern_offset_type fixup_alt_jump;
  pattern_offset_type regnum;
  pattern_offset_type begalt_offset;
} compile_stack_element;


typedef struct {
    compile_stack_element *stack;
    unsigned size;
    unsigned avail;			/* Offset of next open position.  */
  } compile_stack_type;


#define INIT_COMPILE_STACK_SIZE 32

#define COMPILE_STACK_EMPTY  (compile_stack.avail == 0)
#define COMPILE_STACK_FULL  (compile_stack.avail == compile_stack.size)


/* If the buffer isn't allocated when it comes in, use this.  */
#define INIT_BUF_SIZE  32

/* Make sure we have at least N more bytes of space in buffer.  */
#define GET_BUFFER_SPACE(n)						\
  {								        \
    while (b - bufp->buffer + (n) > bufp->allocated)			\
      EXTEND_BUFFER							\
  }

/* Make sure we have one more byte of buffer space and then add C to it.  */
#define BUF_PUSH(c)							\
  do {									\
    GET_BUFFER_SPACE (1);						\
    *b++ = (char) (c);							\
  } while (0)

/* Make sure we have two more bytes of buffer space and then add C1 and 
   C2 to it.  */
#define BUF_PUSH_2(c1, c2)						\
  do {									\
    GET_BUFFER_SPACE (2);						\
    *b++ = (char) (c1);							\
    *b++ = (char) (c2);							\
  } while (0)



#define MAX_BUF_SIZE (1L << 16)

/* Extend the buffer by twice its current size via realloc and
   reset the pointers that pointed into the old block to point to the
   correct places in the new one.  If extending the buffer results in it
   being larger than MAX_BUF_SIZE, then flag memory exhausted.  */
#define EXTEND_BUFFER							\
  { 									\
    char *old_buffer = bufp->buffer;					\
    if (bufp->allocated == MAX_BUF_SIZE) 				\
      goto too_big;							\
    bufp->allocated <<= 1;						\
    if (bufp->allocated > MAX_BUF_SIZE)					\
      bufp->allocated = MAX_BUF_SIZE; 					\
    bufp->buffer = (char *) realloc (bufp->buffer, bufp->allocated);	\
    if (bufp->buffer == NULL)						\
      goto memory_exhausted;						\
    b = (b - old_buffer) + bufp->buffer;				\
    begalt = (begalt - old_buffer) + bufp->buffer;			\
    beg_interval = (beg_interval - old_buffer) + bufp->buffer;		\
    if (fixup_alt_jump)							\
      fixup_alt_jump = (fixup_alt_jump - old_buffer) + bufp->buffer;	\
    if (laststart)							\
      laststart = (laststart - old_buffer) + bufp->buffer;		\
    if (pending_exact)							\
      pending_exact = (pending_exact - old_buffer) + bufp->buffer;	\
  }

/* Set the bit for character C in a list.  */
#define SET_LIST_BIT(c)  (b[(c) / BYTEWIDTH] |= 1 << ((c) % BYTEWIDTH))

/* Get the next unsigned number in the uncompiled pattern.  */
#define GET_UNSIGNED_NUMBER(num) 					\
  { if (p != pend)							\
     {									\
       PATFETCH (c); 							\
       while (isdigit (c)) 						\
         { 								\
           if (num < 0)						\
              num = 0;							\
           num = num * 10 + c - '0'; 					\
           if (p == pend) 						\
              break; 							\
           PATFETCH (c);						\
         } 								\
       } 								\
    }		


#define DO_RANGE							\
  {									\
             /* Get untranslated range start and end characters.  */	\
    char this_char = p[-2];						\
    char end;								\
                                                                       	\
    if (p == pend)							\
      goto invalid_range_end;						\
    PATFETCH_RAW (end);							\
    if ((syntax & RE_NO_EMPTY_RANGES) && this_char > end)		\
      goto invalid_range_end;						\
    while (this_char <= end)						\
      {									\
        SET_LIST_BIT (translate ? translate[this_char] : this_char);	\
        this_char++;							\
      }									\
    }


#define IS_CHAR_CLASS(string)						\
   (strcmp (string, "alpha") == 0 || strcmp (string, "upper") == 0	\
    || strcmp (string, "lower") == 0 || strcmp (string, "digit") == 0	\
    || strcmp (string, "alnum") == 0 || strcmp (string, "xdigit") == 0	\
    || strcmp (string, "space") == 0 || strcmp (string, "print") == 0	\
    || strcmp (string, "punct") == 0 || strcmp (string, "graph") == 0	\
    || strcmp (string, "cntrl") == 0)					\



/* Subroutines for regex_compile.  */

static void store_jump (), insert_jump (), store_jump_n (),
            insert_jump_n (), insert_op_2 (), remove_intervening_anchors (), 
            clear_this_and_higher_levels (), increase_level (),
            decrease_level (), adjust_pattern_offsets_list ();


static unsigned record_anchor_position (), init_bits_list (),
		get_level_match_status (), 
                set_this_level (), set_next_lower_level (),
                make_group_active (), make_group_inactive (),
                set_match_status_of_active_groups (), 
                get_group_match_status (), add_op (),
                init_pattern_offsets_list ();

static boolean is_in_compile_stack (), lower_levels_match_nothing (), 
	       no_levels_match_anything (), verify_and_adjust_endlines ();


static char *
regex_compile (pattern, size, syntax, bufp)
     const char *pattern;
     const int size;
     const int syntax;
     struct re_pattern_buffer *bufp;
{
  register char *b = bufp->buffer;
  const char *p = pattern;
  const char *pend = pattern + size;
  register unsigned c, c1;
  const char *p1;
  unsigned char *translate = (unsigned char *) bufp->translate;
  boolean enough_memory;

  /* Address of the count-byte of the most recently inserted `exactn'
     command.  This makes it possible to tell whether a new exact-match
     character can be added to that command or requires a new `exactn'
     command.  */

  char *pending_exact = 0;

  /* Address of the place where a forward jump should go to the end of
     the containing expression.  Each alternative of an `or', except the
     last, ends with a forward jump of this sort.  */

  char *fixup_alt_jump = 0;

  /* Address of start of the most recently finished expression.
     This tells, e. g., postfix * where to find the start of its operand.  */

  char *laststart = 0;

  /* In processing a repeat, 1 means zero matches is allowed.  */

  char zero_times_ok;

  /* In processing a repeat, 1 means many matches is allowed.  */

  char many_times_ok;

  /* Address of beginning of regexp, or inside of last group.  */

  char *begalt = b;
  
  /* In processing an interval, at least this many matches must be made.  */
  int lower_bound;

  /* In processing an interval, at most this many matches can be made.  */
  int upper_bound;

  /* Place in pattern (i.e., the {) to which to go back if the interval
     is invalid.  */
  const char *beg_interval = 0;
  const char *following_left_brace = 0;

  /* Counts \('s as they are encountered.  Remembered for the matching \),
     where it becomes the register number to put in the stop_memory
     command.  */

  int regnum = 1;

  compile_stack_type compile_stack;
  anchor_list_type anchor_list;

  /* Keeps track of whether or not the pattern at a given grouping level
     matches the empty string so far.  Each bit in the `bits' field of
     this variable corresponds to a level, starting at level zero (i.e.,
     the whole pattern) at the rightmost bit of list[0].  Level 1 is the
     bit to the left of that, etc.  Additional bits that won't fit in
     bits[0] are in bits[2], bits[3], etc.  */

  bits_list_type level_match_status;
  unsigned current_level = 0;
  
  /* Does a similar thing for groups that the above variable does for
     levels.  */
  bits_list_type group_match_status;

  /* Keeps track of whether or not a given group is active.  Accessed as
     is group_match_status.  */
  bits_list_type group_active_status;

  /* Keeps track of operations relevant to detecting valid position of '$'.  */
  op_list_type op_list;

  /* Keeps track of whether or not hit a `$' since the the beginning of
     the pattern or the last (if any) alternative; if so, then `^' is an
     ordinary character.  */
     
  boolean had_an_endline = false;


  compile_stack.stack
    = (compile_stack_element *) malloc (INIT_COMPILE_STACK_SIZE
                                        * sizeof (compile_stack_element));

  if (compile_stack.stack == NULL)
    goto memory_exhausted;

  compile_stack.size = INIT_COMPILE_STACK_SIZE;
  compile_stack.avail = 0;


  if (syntax & RE_REPEATED_ANCHORS_AWAY)
    if (!init_pattern_offsets_list (&anchor_list, 
	    INIT_COMPILE_STACK_SIZE << 1))
      goto memory_exhausted;

  if (!(init_bits_list (&level_match_status)  
       && init_bits_list (&group_match_status)
       && init_bits_list (&group_active_status)))
    goto memory_exhausted;


  if (!init_pattern_offsets_list (&op_list, INIT_PATTERN_OFFSETS_LIST_SIZE))
    goto memory_exhausted;


  bufp->syntax = syntax;
  bufp->fastmap_accurate = 0;
  bufp->not_bol = bufp->not_eol = 0;

  /* Always count groups, whether or not bufp->no_sub is set.  */
  bufp->re_nsub = 0;				

#ifndef emacs
#ifndef SYNTAX_TABLE
  /* Initialize the syntax table.  */
   init_syntax_once();
#endif
#endif


  if (bufp->allocated == 0)
    {
      bufp->allocated = INIT_BUF_SIZE;
      if (bufp->buffer)
	{	
          /* EXTEND_BUFFER loses when bufp->allocated is 0.  This loses if
             buffer's address is bogus.  */
          bufp->buffer = (char *) realloc (bufp->buffer, INIT_BUF_SIZE);
        }
      else
        {
          /* Caller did not allocate a buffer.  Do it for them.  */
          bufp->buffer = (char *) malloc (INIT_BUF_SIZE);
        }
      if (!bufp->buffer) goto memory_exhausted;
      begalt = b = bufp->buffer;
    }

  while (p != pend)
    {
      PATFETCH (c);

      switch (c)
        {
        case '$':
          {
            if ((syntax & RE_ANCHORS_ONLY_AT_ENDS) && p != pend
                && (syntax & RE_CONTEXT_INVALID_ANCHORS))
                  goto invalid_pattern;

	    if (syntax & RE_TIGHT_ALT)
              {
                /* Make operand of last alternation jump to this endline.  */

                if (fixup_alt_jump)
                  store_jump (fixup_alt_jump, jump_past_next_alt, b);

                fixup_alt_jump = 0;
              }

	    if (syntax & RE_REPEATED_ANCHORS_AWAY)
              if (!record_anchor_position (!COMPILE_STACK_EMPTY, 
                                           b - bufp->buffer, &anchor_list))
                goto memory_exhausted;

	    if (!add_op (&op_list, b - bufp->buffer))
              goto memory_exhausted;
              
            BUF_PUSH ((p != pend && *p == '\n')
            	      ? (int) endline_before_newline
		      : (int) endline);
            
	    /* If there's a chance this endline would have to turn into
	       `exactn 1 '$',' have to push dummy ops to make room;
               can't insert later because would mess up any surrounding
               jumps.  */

            if (!(syntax & RE_CONTEXT_INDEP_ANCHORS)
		&& !((syntax & RE_ANCHORS_ONLY_AT_ENDS) && p == pend))
	      {
		laststart = b - 1;
                BUF_PUSH_2 (no_op, no_op);
  	      }

            had_an_endline = true;                
            break;
          }

        case '^':
	  /* If change anything in this case, have to change analogous
             code in *endline* (yes, endline---because the routine goes
             backwards through the pattern) case of the routine
             verify_and_adjust_endlines.  */
             
          /* ^ means match the beginning of a string.  If
             RE_CONTEXT_INDEP_ANCHORS is set, then it represents the
             match-beginning-of-line operator anywhere in the regular
             expression.

             If that bit isn't set, then it represents the
             match-beginning-of-line operator in leading positions and
             matches itself in other positions (unless it's invalid
             there).  */

	   /* If the '^' must be at the pattern's beginning or else is
              in a leading position.  */
	   
           if (((syntax & RE_ANCHORS_ONLY_AT_ENDS) 
	        || (syntax & RE_TIGHT_ALT))
	       ? p - 1 == pattern

               /* If just after a newline, or...  */
               : ((p - 2 >= pattern && p[-2] == '\n')

               /* ...no levels match anything, then in a leading position.  */

                  || no_levels_match_anything (level_match_status)))
	     {
	       if (had_an_endline)
	         goto normal_char;

               if (syntax & RE_REPEATED_ANCHORS_AWAY)
                 if (!record_anchor_position (!COMPILE_STACK_EMPTY, 
					      b - bufp->buffer, &anchor_list))
               goto memory_exhausted;

             }
	
           else if (syntax & RE_CONTEXT_INVALID_ANCHORS)
             goto invalid_pattern;

	   /* If not just after a newline and not always supposed to be
              an anchor, consider it a ordinary character. */
           
           else if (!(syntax & RE_CONTEXT_INDEP_ANCHORS)
                    && ((syntax & RE_ANCHORS_ONLY_AT_ENDS)
	                 /* To make, e.g., `^(^a)' match `^a'.  */
                        ? p - 1 != pattern
                        : (int)laststart))
             goto normal_char;

           if (syntax & RE_TIGHT_ALT)
             {
               if (p != pattern + 1 && !(syntax & RE_CONTEXT_INDEP_ANCHORS))
                 goto normal_char;

               BUF_PUSH (begline);
               begalt = b;  /* Make alternative begin after the '^'.  */
             }
           else
             BUF_PUSH (begline);

           break;

        case '+':
        case '?':
          if ((syntax & RE_BK_PLUS_QM)
              || (syntax & RE_LIMITED_OPS))
            goto normal_char;
        handle_plus:
        case '*':
          /* If there is no previous pattern... */
          if (!laststart)
            {
              if (syntax & RE_CONTEXT_INVALID_OPS)
                goto missing_preceding_re;
              else if (!(syntax & RE_CONTEXT_INDEP_OPS))
                goto normal_char;
            }

          if ((syntax & RE_REPEATED_ANCHORS_AWAY)
              && (enum regexpcode) *laststart == start_memory)
            remove_intervening_anchors (laststart, b, anchor_list, bufp);

          /* If there is a sequence of repetition chars, collapse it
             down to just one.  We can't combine interval operators with
             these because we'd incorrect behavior for, e.g., `a{2}*',
             which should only match an even number of `a's.  */
             
          zero_times_ok = 0;
          many_times_ok = 0;

          while (1)
            {
              zero_times_ok |= c != '+';
              many_times_ok |= c != '?';

              if (p == pend)
                break;

              PATFETCH (c);

              if (c == '*')
		{
                  if (syntax & RE_NO_CONSECUTIVE_REPEATS)
		    goto invalid_preceding_re;                    
                }
	      else if (!(syntax & RE_BK_PLUS_QM)
		       && (c == '+' || c == '?'))
		{
                  if (syntax & RE_NO_CONSECUTIVE_REPEATS)
		    goto invalid_preceding_re;                    
                }
              else if ((syntax & RE_BK_PLUS_QM)
		       && c == '\\')
		{
		  if (p == pend)
                    goto trailing_backslash;
                    
                  PATFETCH (c1);

                  if (!(c1 == '+' || c1 == '?'))
		    {
		      PATUNFETCH;
		      PATUNFETCH;
		      break;
		    }

                  if (syntax & RE_NO_CONSECUTIVE_REPEATS)
		    goto invalid_preceding_re;                    

                  c = c1;
		}
	      else
		{
		  PATUNFETCH;
		  break;
		}
	     }

	  /* Star, etc. applied to an empty pattern is equivalent
	     to an empty pattern.  */
	 if (!laststart)  
	    break;

          /* Now we know whether or not zero matches is allowed
	     and also whether or not two or more matches is allowed.  */

          if (!add_op (&op_list, b - bufp->buffer))
            goto memory_exhausted;

          /* If more than one repetition is allowed, put in at the
             end a backward relative jump from b to before the next jump
             we're going to put in below (which jumps from laststart to
             after this jump).  */
	 
          if (many_times_ok)
            {
	      GET_BUFFER_SPACE (3);
              store_jump (b, maybe_pop_jump, laststart - 3);
              b += 3;  	/* Because store_jump puts stuff here.  */
	    }
          /* Otherwise, put in a no_op so verify_and_adjust_endlines can
             detect that, e.g., a preceding `$' is not an anchor.  */
          else
            BUF_PUSH (no_op);
            

          /* On failure, jump from laststart to b + 3, which will be the
             end of the buffer after this jump is inserted.  */

          if (syntax & RE_REPEATED_ANCHORS_AWAY)
            adjust_pattern_offsets_list (3, laststart - bufp->buffer, 
					 &anchor_list);

          adjust_pattern_offsets_list (3, laststart - bufp->buffer, &op_list);
          GET_BUFFER_SPACE (3);
          insert_jump (on_failure_jump, laststart, b + 3, b);
          pending_exact = 0;
	  b += 3;
          
	  if (!zero_times_ok)
	    {
	      /* At least one repetition is required, so insert a
                 dummy_failure before the initial on_failure_jump
                 instruction of the loop. This effects a skip over that
                 instruction the first time we hit that loop.  */

            if (syntax & RE_REPEATED_ANCHORS_AWAY)
                adjust_pattern_offsets_list (3, laststart - bufp->buffer, 
			                     &anchor_list);

              adjust_pattern_offsets_list (3, laststart - bufp->buffer, 
			                   &op_list);
              GET_BUFFER_SPACE (3);
              insert_jump (dummy_failure_jump, laststart, laststart + 6, b);
	      b += 3;
            }
	  break;

	case '.':
	  laststart = b;

          if (!add_op (&op_list, b - bufp->buffer))
            goto memory_exhausted;

          BUF_PUSH (anychar);

          if (!set_this_level (&level_match_status, current_level)
              || !set_match_status_of_active_groups (group_active_status,
						     &group_match_status))
            goto memory_exhausted;

          break;

        case '[':
	  {
            unsigned just_had_a_char_class = 0;
            
            if (p == pend)
              goto unmatched_left_bracket;

            while (b - bufp->buffer
                   > bufp->allocated - 3 - (1 << BYTEWIDTH) / BYTEWIDTH)
              EXTEND_BUFFER;

            laststart = b;

            if (!add_op (&op_list, b - bufp->buffer))
              goto memory_exhausted;

            if (*p == '^')
              {
                BUF_PUSH (charset_not); 
                p++;
              }
            else
              BUF_PUSH (charset);

            /* Remember the first position in the bracket expression.  */
            p1 = p;

            BUF_PUSH ((1 << BYTEWIDTH) / BYTEWIDTH);
            /* Clear the whole map */
            bzero (b, (1 << BYTEWIDTH) / BYTEWIDTH);

            if ((syntax & RE_HAT_LISTS_NOT_NEWLINE) 
                 && (enum regexpcode) b[-2] == charset_not)
              SET_LIST_BIT ('\n');


            /* Read in characters and ranges, setting map bits.  */
            while (1)
              {
                if (p == pend)
                  goto unmatched_left_bracket;

		PATFETCH (c);


                /* If set, \ escapes characters when inside [...].  */
                if ((syntax & RE_AWK_CLASS_HACK) && c == '\\')
                  {
		    if (p == pend)
                      goto trailing_backslash;
                      
                    PATFETCH(c1);
                    SET_LIST_BIT (c1);
                    continue;
                  }
                /* Could be the end of the bracket expression.  If it's
                   not (i.e., when the bracket expression is `[]' so
                   far), the ']' character bit gets set way below.  */

                if (c == ']' && p != p1 + 1)
		  break;


                /* Look ahead to see if it's a range when the last thing
                   was a character class.  */

                if (just_had_a_char_class && c == '-' && *p != ']')
                  goto invalid_range_end;
		    
   		/* Look ahead to see if it's a range when the last thing
                   was a character: if this is a hyphen not at the
                   beginning or the end of a list, then it's the range
                   operator.  */
                   
		if (c == '-' 
                    && !(p - 2 >= pattern && p[-2] == '[') 
	            && !(p - 3 >= pattern && p[-3] == '[' && p[-2] == '^')
                    && *p != ']')
                  {
                    DO_RANGE;
                  }
                  
                else if (p[0] == '-' && p[1] != ']')
                  {
                    /* This handles ranges made up of characters only.  */
                    PATFETCH (c1);		/* The `-'.  */
		    DO_RANGE;
                  }

                /* See if we're at the beginning of a possible character
                   class.  */
   
                else if ((syntax & RE_CHAR_CLASSES)
                          &&  c == '[' && p[0] == ':')
                  {
                    /* Longest valid character class word has six chars.  */
                    char str[CHAR_CLASS_MAX_LENGTH];
                    
                    PATFETCH (c);
                    c1 = 0;

                    /* If pattern is `[[:'.  */
                    if (p == pend)
                      goto unmatched_left_bracket;

                    while (1)
                      {
                        /* Don't translate the ``character class''
                           characters.  */
                        PATFETCH_RAW (c);
                        if (c == ':' || c == ']' || p == pend
                            || c1 == CHAR_CLASS_MAX_LENGTH)
                          break;
                        str[c1++] = c;
                      }
                    str[c1] = '\0';
                    
                    /* If isn't a word bracketed by `[:' and:`]':
                       undo the ending character, the letters, and leave 
                       the leading `:' and `[' (but set bits for them).  */

                    if (c == ':' && p[0] == ']')
                      {
                        if (!IS_CHAR_CLASS (str))
                          goto invalid_char_class;
                          
                        /* The ] at the end of the character class.  */
                        PATFETCH (c);					

                        if (p == pend)
                          goto unmatched_left_bracket;
                          
                        for (c = 0; c < (1 << BYTEWIDTH); c++)
                          {
                            if ((strcmp (str, "alpha") == 0  && isalpha (c))
                              || (strcmp (str, "upper") == 0  && isupper (c))
                              || (strcmp (str, "lower") == 0  && islower (c))
                              || (strcmp (str, "digit") == 0  && isdigit (c))
                              || (strcmp (str, "alnum") == 0  && isalnum (c))
                              || (strcmp (str, "xdigit") == 0  && isxdigit (c))
                              || (strcmp (str, "space") == 0  && isspace (c))
                              || (strcmp (str, "print") == 0  && isprint (c))
                              || (strcmp (str, "punct") == 0  && ispunct (c))
                              || (strcmp (str, "graph") == 0  && isgraph (c))
                              || (strcmp (str, "cntrl") == 0  && iscntrl (c)))
                            SET_LIST_BIT (c);
                          }
                        just_had_a_char_class = 1;
                      }
    	            else
		      {
                        c1++;
                        while (c1--)    
                          PATUNFETCH;
                        SET_LIST_BIT ('[');
                        SET_LIST_BIT (':');
                        just_had_a_char_class = 0;
                      }
                  }
                else
                  {
                    just_had_a_char_class = 0;
                    SET_LIST_BIT (c);
                  }
              }

            /* Discard any (non)matching list bytes that are all 0 at the
               end of the map. Decrement the map-length byte too.  */

            while ((int) b[-1] > 0 && b[b[-1] - 1] == 0) 
              b[-1]--; 
            b += b[-1];
          }

          if (!set_this_level (&level_match_status, current_level)
              || !set_match_status_of_active_groups (group_active_status,
						     &group_match_status))
            goto memory_exhausted;

          break;


        case '(':
	  if (!(syntax & RE_NO_BK_PARENS))
	    goto normal_char;
	  else
	    goto handle_open;

	case ')':
	  if (! (syntax & RE_NO_BK_PARENS))
	    goto normal_char;
	  else
	    goto handle_close;

        case '\n':
	  if (! (syntax & RE_NEWLINE_ALT))
	    goto normal_char;
	  else
	    goto handle_bar;

	case '|':
          if (!(syntax & RE_NO_BK_VBAR))
	    goto normal_char;
	  else
	    goto handle_bar;

	case '{':
           if ((syntax & RE_NO_BK_BRACES)
                && (syntax & RE_INTERVALS))
             goto handle_interval;
           else
             goto normal_char;
             
        case '\\':
	  if (p == pend) 
            goto trailing_backslash;
	
          PATFETCH_RAW (c);
	  switch (c)
	    {
            case '(':
	      if (syntax & RE_NO_BK_PARENS)
		goto normal_backsl;
            handle_open:
              bufp->re_nsub++;
              increase_level (&current_level);

              if (!make_group_active (&group_active_status, regnum))
        	goto memory_exhausted;
        
              if (syntax & RE_NO_EMPTY_GROUPS)
                {
                  p1 = p;
                  if (*p1 == '^') p1++;
                  if (*p1 == '$') p1++;
		  if (!(syntax & RE_NO_BK_PARENS) && *p1 == '\\') p1++;
                  
		  /* If found an empty group...  */
                  if (*p1 == ')') 
                    goto invalid_pattern;
                }
            
              /* Value to restore in laststart when hit end of this
                 group; should point to the start_memory that we are
                 about to push.  */
                 
	      if (COMPILE_STACK_FULL)
                { 
                  compile_stack.stack = (compile_stack_element *) 
                    realloc (compile_stack.stack,
                             (compile_stack.size << 1)
                             * sizeof (compile_stack_element));
	        
                  if (compile_stack.stack == NULL)
		    goto memory_exhausted;
 
                  compile_stack.size <<= 1;
  		}

              compile_stack.stack[compile_stack.avail].laststart_offset 
                = b - bufp->buffer;
	      compile_stack.stack[compile_stack.avail].fixup_alt_jump 
                = fixup_alt_jump ? fixup_alt_jump - bufp->buffer + 1 : 0;
	      compile_stack.stack[compile_stack.avail].regnum = regnum;
	      compile_stack.stack[compile_stack.avail].begalt_offset 
                = begalt - bufp->buffer;
  	      compile_stack.avail++;

	      if (!add_op (&op_list, b - bufp->buffer))
                goto memory_exhausted;

              BUF_PUSH_2 (start_memory, regnum);
              regnum++;
              fixup_alt_jump = 0;
	      laststart = 0;
	      begalt = b;
              break;

	    case ')':
	      if (syntax & RE_NO_BK_PARENS)
		goto normal_backsl;
	      
	      if (COMPILE_STACK_EMPTY)
		if (syntax & RE_UNMATCHED_RIGHT_PAREN_ORD)
                  goto normal_backsl;
                else
                  goto unmatched_close;	
        
            handle_close:
              if (fixup_alt_jump)
		store_jump (fixup_alt_jump, jump_past_next_alt, b);
	      
              /* See similar code for backslashed parens above.  */
              
              if (COMPILE_STACK_EMPTY)
		if (syntax & RE_UNMATCHED_RIGHT_PAREN_ORD)
                  goto normal_char;
                else
                  goto unmatched_close;	

	      if (get_level_match_status (level_match_status, current_level))
                if (!set_next_lower_level (&level_match_status, current_level))
		  goto memory_exhausted;
               
              /* Only call these if know you have a matched close.  */
              decrease_level (&current_level);
	      make_group_inactive (&group_active_status, regnum);

              compile_stack.avail--;		
	      begalt 
                = compile_stack.stack[compile_stack.avail].begalt_offset 
  	          + bufp->buffer;
              laststart 
                = (compile_stack.stack[compile_stack.avail].laststart_offset
                  + bufp->buffer);

              fixup_alt_jump = compile_stack.stack[compile_stack.avail].fixup_alt_jump
              		   ? compile_stack.stack[compile_stack.avail]
                             .fixup_alt_jump + bufp->buffer - 1 
                           : 0;

              if (!add_op (&op_list, b - bufp->buffer))
                goto memory_exhausted;

              BUF_PUSH_2 (stop_memory, 
	                  compile_stack.stack[compile_stack.avail].regnum);
	      break;

	    case '|':					/* `\|'.  */
              if ((syntax & RE_LIMITED_OPS)
	          || (syntax & RE_NO_BK_VBAR))
		goto normal_backsl;
	    handle_bar:
              if (syntax & RE_LIMITED_OPS)
                goto normal_char;
               
              /* Disallow empty alternatives if RE_NO_EMPTY_ALTS is set.
                 Caveat: can't detect if the vbar is followed by a
                 trailing '$' yet, unless it's the last thing in a
                 pattern; the routine for verifying endlines has to do
                 the rest.  */

              if ((syntax & RE_NO_EMPTY_ALTS)
                  && (!laststart  ||  p == pend 
                      || (*p == '$' && p + 1 == pend)
                      || ((syntax & RE_NO_BK_PARENS)
                           ? (p < pend  &&  *p == ')')
                           : (p + 1 < pend && p[0] == '\\' && p[1] == ')'))))
                goto invalid_pattern;

	      
              /* Clear some variables.  */
              
              if (lower_levels_match_nothing (level_match_status, 
				              current_level))
	        clear_this_and_higher_levels (&level_match_status, 
				              current_level);
	      had_an_endline = false;
              

              /* Insert before the previous alternative a jump which
                 jumps to this alternative if the former fails.  */
	      
              if (syntax & RE_REPEATED_ANCHORS_AWAY)
	        adjust_pattern_offsets_list (3, begalt - bufp->buffer, 
			                     &anchor_list);
          
              adjust_pattern_offsets_list (3, begalt - bufp->buffer, &op_list);
              GET_BUFFER_SPACE (3);
              insert_jump (on_failure_jump, begalt, b + 6, b);
	      pending_exact = 0;
	      b += 3;

              /* The alternative before this one has a jump after it
                 which gets executed if it gets matched.  Adjust that
                 jump so it will jump to this alternative's analogous
                 jump (put in below, which in turn will jump to the next
                 (if any) alternative's such jump, etc.).  The last such
                 jump jumps to the correct final destination.  A picture:
	                  _____ _____ 
                	  |   | |   |   
			  |   v |   v 
	                 a | b   | c   
		 
                 If we are at `b,' then fixup_alt_jump right now points to a
                 three-byte space after `a.'  We'll put in the jump, set
                 fixup_alt_jump to right after `b,' and leave behind three
                 bytes which we'll fill in when we get to after `c.'  */

              if (fixup_alt_jump)
		store_jump (fixup_alt_jump, jump_past_next_alt, b);
                
	      /* Mark and leave space for a jump after this alternative
                 ---to be filled in later either by next alternative or
                 when know we're at the end of a series of alternatives.  */

	      if (!add_op (&op_list, b - bufp->buffer))
                goto memory_exhausted;

              fixup_alt_jump = b;
	      GET_BUFFER_SPACE (3);
              b += 3;

              laststart = 0;
	      begalt = b;
	      break;

            case '{': 
                  /* If \{ is a literal.  */
              if (!(syntax & RE_INTERVALS)
		  /* If we're at a "\{" and it's not the open-interval 
                     operator.  */
                  || ((syntax & RE_INTERVALS)
                      && (syntax & RE_NO_BK_BRACES))
                  || (p - 2 == pattern  &&  p == pend))
                goto normal_backsl;

            handle_interval:
	      /* If got here, then intervals must be allowed.  */

              beg_interval = p - 1; 		/* The `{'.  */
	      following_left_brace = 0;
              lower_bound = -1;			/* So can see if are set.  */
	      upper_bound = -1;
              
              if (p == pend)
		{
                  if (syntax & RE_NO_BK_BRACES)
		    goto unfetch_interval;
                  else
                    goto unmatched_left_curly_brace;
                }

              GET_UNSIGNED_NUMBER (lower_bound);

              if (c == ',')
		{
                  GET_UNSIGNED_NUMBER (upper_bound);
		  if (upper_bound < 0)
		    upper_bound = RE_DUP_MAX;
		}
                
	      if (upper_bound < 0)
		upper_bound = lower_bound;

	      if (lower_bound < 0 || upper_bound > RE_DUP_MAX
		  || lower_bound > upper_bound)
		{
                  if (syntax & RE_NO_BK_BRACES)
                    goto unfetch_interval;
                  else 
                    goto invalid_braces_content;
                }

              if (!(syntax & RE_NO_BK_BRACES)) 
                {
                  if (c != '\\')
                    goto unmatched_left_curly_brace;
                    
                  PATFETCH (c);
                }

              if (c != '}')
		{
                  if (syntax & RE_NO_BK_BRACES)
                    goto unfetch_interval;
                  else 
                    goto invalid_braces_content;
                }


	      /* Parsed a valid interval, but if an interval can't
                 operate on another repetition operator, check that what
                 follows isn't one.  */

              if ((syntax & RE_NO_CONSECUTIVE_REPEATS) && p != pend)
                {
		  if (*p == '*' || *p == '+' || *p == '?')
                    goto invalid_preceding_re;
                    
                  if (syntax & RE_NO_BK_BRACES)
		    {
                      if (*p == '{')
                        {
                          /* Close but not exactly as above.  */
                          
                          int lower_bound = -1;
                          int upper_bound = -1;

                          following_left_brace = p++;
                          GET_UNSIGNED_NUMBER (lower_bound);

                          if (c == ',')
                            {
                              GET_UNSIGNED_NUMBER (upper_bound);
                              if (upper_bound < 0)
                                upper_bound = RE_DUP_MAX;
                            }

                          if (upper_bound < 0)
                            upper_bound = lower_bound;

			  /* If not a valid interval, then we don't have
                             an interval operating on another one; what
                             we have instead is a series match-self ops
                             starting with a '{'.  */
                          
                          if (lower_bound < 0 || upper_bound > RE_DUP_MAX
                              || lower_bound > upper_bound || c != '}')
			    {
                              /* Back up to  '{' so can use again
                                 put it in C, as the normal_char label
                                 code expects that; will go to that
                                 label after putting the preceding valid
                                 interval in the buffer.  */
                                 
                              p = following_left_brace;
                              PATFETCH (c);	
                            }
                          else
                            goto invalid_preceding_re;
                        }
                    }
                  else if (p[0] == '\\' && p[1] == '{')
		    goto invalid_preceding_re;
                }
                
                      
	      /* We just parsed a valid interval.  */
              
              /* If it's invalid to have no preceding re.  */
              if (!laststart)
	        {
                  if (syntax & RE_CONTEXT_INVALID_OPS)
                    goto missing_preceding_re;
                  else if (syntax & RE_CONTEXT_INDEP_OPS)
                    laststart = b;
                  else
                    goto unfetch_interval;
                }
	      else if ((syntax & RE_REPEATED_ANCHORS_AWAY)
		       && (enum regexpcode) *laststart == start_memory)
		remove_intervening_anchors (laststart, b, anchor_list, bufp);
                
              /* If upper_bound is zero, don't want to succeed at all; 
 		 jump from laststart to b + 3, which will be the end of
                 the buffer after this jump is inserted.  */
                 
               if (upper_bound == 0)
                 {
                   if (syntax & RE_REPEATED_ANCHORS_AWAY)
		     adjust_pattern_offsets_list (3, laststart - bufp->buffer, 
		           	                  &anchor_list);

                   adjust_pattern_offsets_list (3, laststart - bufp->buffer, 
				                &op_list);
                   GET_BUFFER_SPACE (3);
                   insert_jump (no_pop_jump, laststart, b + 3, b);
                   b += 3;
                 }

               /* Otherwise, after lower_bound number of succeeds, jump
                  to after the no_pop_jump_n which will be inserted at the end
                  of the buffer, and insert that no_pop_jump_n.  */
               else 
		 { /* Set to 5 if only one repetition is allowed and
                      hence no no_pop_jump_n is inserted at the current
                      end of the buffer.  Otherwise, need 10 bytes total
                      for the succeed_n and the no_pop_jump_n.  */
                      
                   unsigned slots_needed = upper_bound == 1 ? 5 : 10;
                     
                   GET_BUFFER_SPACE (slots_needed);
                   /* Initialize the succeed_n to n, even though it will
                      be set by its attendant set_number_at, because
                      re_compile_fastmap will need to know it.  Jump to
                      what the end of buffer will be after inserting
                      this succeed_n and possibly appending a
                      no_pop_jump_n.  */

	           if (syntax & RE_REPEATED_ANCHORS_AWAY)
                     adjust_pattern_offsets_list (5, laststart - bufp->buffer, 
		           	                  &anchor_list);
                  
	           adjust_pattern_offsets_list (5, laststart - bufp->buffer, 
				                &op_list);
                   insert_jump_n (succeed_n, laststart, b + slots_needed, 
		                  b, lower_bound);
                   b += 5; 	/* Just increment for the succeed_n here.  */

		
                  /* More than one repetition is allowed, so put in at
		     the end of the buffer a backward jump from b to the
                     succeed_n we put in above.  By the time we've gotten
                     to this jump when matching, we'll have matched once
                     already, so jump back only upper_bound - 1 times.  */

		   if (!add_op (&op_list, b - bufp->buffer))
        	     goto memory_exhausted;

                   if (upper_bound > 1)
                     {
                       store_jump_n (b, no_pop_jump_n, laststart, 
		                     upper_bound - 1);
                       b += 5;
                       /* When hit this when matching, reset the
                          preceding no_pop_jump_n's n to upper_bound - 1.  */
                          
                       BUF_PUSH (set_number_at);

                       /* Only need to get space for the numbers.  */
	               GET_BUFFER_SPACE (4);
                       STORE_NUMBER_AND_INCR (b, -5);
                       STORE_NUMBER_AND_INCR (b, upper_bound - 1);
                     }
                   /* Otherwise, put in a no_op, so verify_and_adjust_endlines
                      can detect, e.g., a preceding `$' is not an anchor.  */
		   else
		     BUF_PUSH (no_op);
                     
                    
                   /* When hit this when matching, set the succeed_n's n.  */

                   if (syntax & RE_REPEATED_ANCHORS_AWAY)
		     adjust_pattern_offsets_list (5, laststart - bufp->buffer, 
			   	                  &anchor_list);
	
                   adjust_pattern_offsets_list (5, laststart - bufp->buffer, 
			                        &op_list);
                   GET_BUFFER_SPACE (5);
		   insert_op_2 (set_number_at, laststart, b, 5, lower_bound);
                   b += 5;
                 }
              pending_exact = 0;
	      beg_interval = 0;
	      
              if (following_left_brace)
		goto normal_char;		

              break;

            unfetch_interval:
	      /* If an invalid interval, match the characters as literals.  */
	       if (beg_interval)
                 p = beg_interval;
  	       else
                 {
                   fprintf (stderr, 
		      "regex: no interval beginning to which to backtrack.\n");
		   exit (1);
                 }
               beg_interval = 0;
	       
               /* normal_char and normal_backsl expect a character in `c'.  */
               PATFETCH (c);	

               if (!(syntax & RE_NO_BK_BRACES))
                 {
                   if (p > pattern  &&  p[-1] == '\\')
		     goto normal_backsl;
                 }
               goto normal_char;

#ifdef emacs
	    case '=':
	      BUF_PUSH (at_dot);
	      break;

	    case 's':	
	      laststart = b;
	      PATFETCH (c);
	      BUF_PUSH_2 (syntaxspec, syntax_spec_code[c]);
	      break;

	    case 'S':
	      laststart = b;
	      PATFETCH (c);
	      BUF_PUSH_2 (notsyntaxspec, syntax_spec_code[c]);
	      break;
#endif /* emacs */

	    case 'w':
	      laststart = b;

              if (!add_op (&op_list, b - bufp->buffer))
                goto memory_exhausted;

	      BUF_PUSH (wordchar);

              if (!set_this_level (&level_match_status, current_level)
                  || !set_match_status_of_active_groups (group_active_status,
							 &group_match_status))
                goto memory_exhausted;

              break;

	    case 'W':
	      laststart = b;

              if (!add_op (&op_list, b - bufp->buffer))
                goto memory_exhausted;

	      BUF_PUSH (notwordchar);
	
              if (!set_this_level (&level_match_status, current_level)
                  || !set_match_status_of_active_groups (group_active_status,
							 &group_match_status))
                goto memory_exhausted;

              break;

	    case '<':
	      BUF_PUSH (wordbeg);
	      break;

	    case '>':
	      BUF_PUSH (wordend);
	      break;

	    case 'b':
	      BUF_PUSH (wordbound);
	      break;

	    case 'B':
	      BUF_PUSH (notwordbound);
	      break;

	    case '`':
	      BUF_PUSH (begbuf);
	      break;

	    case '\'':
	      BUF_PUSH (endbuf);
	      break;

	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	      if (syntax & RE_NO_BK_REFS)
                goto normal_char;

              c1 = c - '0';

              if (c1 >= regnum)
		{
  		  if (syntax & RE_NO_MISSING_BK_REF)
                    goto invalid_back_reference;
                  else
                    goto normal_char;
                }

              /* Can't back reference to a subexpression if inside of it.  */
	      if (is_in_compile_stack (compile_stack, c1))
                goto normal_char;

              laststart = b;

              if (!add_op (&op_list, b - bufp->buffer))
                goto memory_exhausted;

	      BUF_PUSH_2 (duplicate, c1);
              
              if (get_group_match_status (group_match_status, c1))
		if (!set_this_level (&level_match_status, current_level))
                  goto memory_exhausted;

              break;

	    case '+':
	    case '?':
	      if (syntax & RE_BK_PLUS_QM)
		goto handle_plus;
	      else
                goto normal_backsl;
              break;

            default:
	    normal_backsl:
	      /* You might think it would be useful for \ to mean
		 not to translate; but if we don't translate it
		 it will never match anything.  */
	
              if (translate) 
                c = translate[c];
	
              goto normal_char;
	    }
	  break;

        default:

        /* Expects the character in `c'!  */
	normal_char:
	      /* If no exactn currently being built.  */
          if (!pending_exact 

              /* If last exactn not at current position.  */
              || pending_exact + *pending_exact + 1 != b
              
	      || *pending_exact == 0177 

              /* If followed by a repetition operator.  */
              || *p == '*' || *p == '^'
	      || ((syntax & RE_BK_PLUS_QM)
		  ? *p == '\\' && (p[1] == '+' || p[1] == '?')
		  : (*p == '+' || *p == '?'))
	      || ((syntax & RE_INTERVALS)
                  && ((syntax & RE_NO_BK_BRACES)
		      ? *p == '{'
                      : (p[0] == '\\' && p[1] == '{'))))
	    {
	      /* Start building a new exactn.  */
              
              laststart = b;

              if (!add_op (&op_list, b - bufp->buffer))
                goto memory_exhausted;

	      BUF_PUSH_2 (exactn, 0);
	      pending_exact = b - 1;
    
              if (!set_this_level (&level_match_status, current_level))
                goto memory_exhausted;
            }
	  BUF_PUSH (c);
          (*pending_exact)++;
	  break;
          
        } /* end switch (c).  */
    } /* end while p!= pend.  */

  
  /* Through the pattern now.  */
  
  if (fixup_alt_jump)
    store_jump (fixup_alt_jump, jump_past_next_alt, b);

  if (!COMPILE_STACK_EMPTY) 
    goto unmatched_open;

  /* Have to set this before calling the next routine.  */
  bufp->used = b - bufp->buffer;

  if (!verify_and_adjust_endlines (op_list, group_match_status, bufp, 
				   &enough_memory))
    goto invalid_pattern;
  
  if (!enough_memory)
    goto memory_exhausted;
	

  /* Normal return.  */
  return 0;


 /* Abnormal return.  */

 invalid_pattern:
   bufp->used = b - bufp->buffer;
   return "Invalid regular expression";

 unmatched_open:
   bufp->used = b - bufp->buffer;
   return "Unmatched ( or \\(";

 unmatched_close:
   bufp->used = b - bufp->buffer;
   return "Unmatched ) or \\)";

 end_of_pattern:
   bufp->used = b - bufp->buffer;
   return "Premature end of regular expression";

 too_big:
   bufp->used = b - bufp->buffer;
   return "Regular expression too big";

 memory_exhausted:
   bufp->used = b - bufp->buffer;
   return "Memory exhausted";
  
 invalid_char_class:
   bufp->used = b - bufp->buffer;
   return "Invalid character class name";
 
 unmatched_left_bracket:
   bufp->used = b - bufp->buffer;
   return "Unmatched [ or [^";

 invalid_range_end:
   bufp->used = b - bufp->buffer;
   return "Invalid range end";

 trailing_backslash:
   bufp->used = b - bufp->buffer;
   return "Trailing backslash";
   
 unmatched_left_curly_brace:
   bufp->used = b - bufp->buffer;
   return "Unmatched \\{";

 invalid_braces_content:
   bufp->used = b - bufp->buffer;
   return "Invalid content of \\{\\}";
 
 missing_preceding_re:
   bufp->used = b - bufp->buffer;
   return "Missing preceding regular expression";

 invalid_preceding_re:
   bufp->used = b - bufp->buffer;
   return "Invalid preceding regular expression";
   
 invalid_back_reference:
   bufp->used = b - bufp->buffer;
   return "Invalid back reference";
}


/* Store a jump of the form <OPCODE> <relative address>.
   Store in the location FROM a jump operation to jump to relative
   address FROM - TO.  OPCODE is the opcode to store.  */

static void
store_jump (from, opcode, to)
     char *from, *to;
     char opcode;
{
  from[0] = opcode;
  STORE_NUMBER(from + 1, to - (from + 3));
}


/* Open up space before char FROM, and insert there a jump to TO.
   CURRENT_END gives the end of the storage not in use, so we know 
   how much data to copy up. OP is the opcode of the jump to insert.

   If you call this function, you must zero out pending_exact.  */

static void
insert_jump (op, from, to, current_end)
     char op;
     char *from, *to, *current_end;
{
  register char *pfrom = current_end;		/* Copy from here...  */
  register char *pto = current_end + 3;		/* ...to here.  */

  while (pfrom != from)			       
    *--pto = *--pfrom;
  store_jump (from, op, to);
}


/* Store a jump of the form <opcode> <relative address> <n> .

   Store in the location FROM a jump operation to jump to relative
   address FROM - TO.  OPCODE is the opcode to store, N is a number the
   jump uses, say, to decide how many times to jump.
   
   If you call this function, you must zero out pending_exact.  */

static void
store_jump_n (from, opcode, to, n)
     char *from, *to;
     char opcode;
     unsigned n;
{
  from[0] = opcode;
  STORE_NUMBER (from + 1, to - (from + 3));
  STORE_NUMBER (from + 3, n);
}


/* Similar to insert_jump, but handles a jump which needs an extra
   number to handle minimum and maximum cases.  Open up space at
   location FROM, and insert there a jump to TO.  CURRENT_END gives the
   end of the storage in use, so we know how much data to copy up. OP is
   the opcode of the jump to insert.

   If you call this function, you must zero out pending_exact.  */

static void
insert_jump_n (op, from, to, current_end, n)
     char op;
     char *from, *to, *current_end;
     unsigned n;
{
  register char *pfrom = current_end;		/* Copy from here...  */
  register char *pto = current_end + 5;		/* ...to here.  */

  while (pfrom != from)			       
    *--pto = *--pfrom;
  store_jump_n (from, op, to, n);
}


/* Open up space at location THERE, and insert operation OP followed by
   NUM_1 and NUM_2.  CURRENT_END gives the end of the storage in use, so
   we know how much data to copy up.

   If you call this function, you must zero out pending_exact.  */

static void
insert_op_2 (op, there, current_end, num_1, num_2)
     char op;
     char *there, *current_end;
     int num_1, num_2;
{
  register char *pfrom = current_end;		/* Copy from here...  */
  register char *pto = current_end + 5;		/* ...to here.  */

  while (pfrom != there)			       
    *--pto = *--pfrom;
  
  there[0] = op;
  STORE_NUMBER (there + 1, num_1);
  STORE_NUMBER (there + 3, num_2);
}


/* Compile stack routine for regex_compile.  */

/* Returns true if REGNUM is in one of COMPILE_STACK's elements and 
   false if it's not.  */

static boolean
is_in_compile_stack (compile_stack, regnum)
  compile_stack_type compile_stack;
  int regnum;
{
  int this_element;
  
  if (COMPILE_STACK_EMPTY)
    return false;

  for (this_element = compile_stack.avail - 1;  
       this_element >= 0; 
       this_element--)
    if (compile_stack.stack[this_element].regnum == regnum)
      return true;

  return false;
}


/* Pattern offsets list stuff.  */

/* Initializes a pattern offsets list PATTERN_OFFSETS_LIST_PTR to be 
   INIT_SIZE large.  
   
   Returns 1 if it can allocate the space and 0 if it can't.   */

static unsigned
init_pattern_offsets_list (pattern_offsets_list_ptr, init_size)
  pattern_offsets_list_type *pattern_offsets_list_ptr;
  int init_size;
{
  if (init_size < 0)
    {
      printf ("Can't initialize a pattern offsets list with a negative \
or zero init_size %d.\n", init_size);
      exit (1);
    }
  else
    {
      pattern_offsets_list_ptr->offsets 
        = (pattern_offset_type *) malloc (init_size
                                           * sizeof (pattern_offset_type));

      if (pattern_offsets_list_ptr->offsets == NULL)
        return 0;
        
      pattern_offsets_list_ptr->size = init_size;
      pattern_offsets_list_ptr->avail = 0;
    }
  return 1;
}


/* Doubles the size of a pattern offsets list PATTERN_OFFSETS_LIST_PTR. 
   
   Returns 1 if it can allocate the space and 0 if it can't.   */

static unsigned
double_pattern_offsets_list (pattern_offsets_list_ptr)
  pattern_offsets_list_type *pattern_offsets_list_ptr;
{
  pattern_offsets_list_ptr->offsets 
    = (pattern_offset_type *) realloc (pattern_offsets_list_ptr->offsets, 
      (pattern_offsets_list_ptr->size << 1) * sizeof (pattern_offset_type));
 
  if (pattern_offsets_list_ptr->offsets == NULL)
    return 0;

  pattern_offsets_list_ptr->size <<= 1;
  return 1;
}


/* Adds OFFSET to PATTERN_OFFSETS_LIST_PTR.

   Returns 1 if it can add the offset and 0 if it needs to allocate
   space for it and can't.  */

static unsigned
add_pattern_offset (pattern_offsets_list_ptr, offset)
  pattern_offsets_list_type *pattern_offsets_list_ptr;
  pattern_offset_type offset;
{
  if (PATTERN_OFFSETS_LIST_PTR_FULL (pattern_offsets_list_ptr))  
    if (!double_pattern_offsets_list (pattern_offsets_list_ptr))
      return 0;
      
  pattern_offsets_list_ptr->offsets[pattern_offsets_list_ptr->avail] = offset;
  pattern_offsets_list_ptr->avail++;
  
  return 1;
}

   
/*  Adjust each offset in PATTERN_OFFSETS_LIST_PTR by INCREMENT.  */

static void
adjust_pattern_offsets_list (increment, start_position, 
			     pattern_offsets_list_ptr)
  unsigned increment;
  unsigned start_position;
  pattern_offsets_list_type *pattern_offsets_list_ptr;
{
  unsigned this_pattern_offset = 0;
  
  while (this_pattern_offset < pattern_offsets_list_ptr->avail  
	 && pattern_offsets_list_ptr->offsets[this_pattern_offset] 
            < start_position)
    this_pattern_offset++;
    
  for (; this_pattern_offset < pattern_offsets_list_ptr->avail; 
       this_pattern_offset++)
    pattern_offsets_list_ptr->offsets[this_pattern_offset] += increment;
}


/* Anchor routines for regex_compile.  */

/* If it's in a group, record in ANCHOR_LIST_PTR an anchor offset that's 
   at OFFSET.  
   
   Returns 1 if can put the offset in ANCHOR_LIST_PTR.
   Returns 0 if runs out of memory allocating space for it.  */
   
static unsigned
record_anchor_position (in_a_group, offset, anchor_list_ptr)
  unsigned in_a_group;
  pattern_offset_type offset;
  anchor_list_type *anchor_list_ptr;
{
  if (in_a_group)
    if (!add_pattern_offset (anchor_list_ptr, offset))
      return 0;

  return 1;
}


/* Set all `begline's between START and END in BUFP to `no_op's.
   Set all such `endline's to either `endline_in_repeat's and all such
   `endline_before_newline's to `repeated_endline_before_repeat's.  */

static void
remove_intervening_anchors (start, end, anchor_list, bufp)
  char *start, *end;
  anchor_list_type anchor_list;
  struct re_pattern_buffer *bufp;
{
  unsigned this_anchor = 0;

  while (this_anchor < anchor_list.avail
         && start - bufp->buffer <= anchor_list.offsets[this_anchor]
         && anchor_list.offsets[this_anchor] <= end - bufp->buffer)
    {
      char *this_anchor_ptr 
        = bufp->buffer + anchor_list.offsets[this_anchor++];
      
      *this_anchor_ptr = *this_anchor_ptr == endline 
			 ? (char)endline_in_repeat 
                         : *this_anchor_ptr == endline_before_newline
                           ? (char)repeated_endline_before_newline
	                   : *this_anchor_ptr == begline
			     ? (char)no_op
                             : *this_anchor_ptr;
    }
}


/* Op list stuff.  */

/* Add OP_OFFSET to OP_LIST_PTR.  
   Return 1 if can add it and 0 if can't allocate the space to do so.  */

static unsigned
add_op (op_list_ptr, op_offset)
  op_list_type *op_list_ptr;
  pattern_offset_type op_offset;
{
  return add_pattern_offset (op_list_ptr, op_offset);
}


/* Verify that all `$'s in an entire pattern buffer BUFP are valid
   anchors or ordinary characters.  Either leave or change intermediate
   forms of `$' anchor ops into `endline' or `exactn ...' where
   appropriate.

   Return true in ENOUGH_MEMORY if don't run out of space allocating
   internal data structures.

   Return from the routine true if the pattern is valid and false 
   if it isn't.  */

static boolean
verify_and_adjust_endlines (op_list, group_forward_match_status, 
			    bufp, enough_memory)
  op_list_type op_list;
    /* `duplicate' case needs this: which groups matched something;
        set when went fowards through the pattern.  */
  bits_list_type group_forward_match_status;	
  struct re_pattern_buffer *bufp;
  boolean *enough_memory;
{
  int this_op_offset;	/* Has to be type int because decrementing it.  */
  /* See comments for analogous variables used for '^' in regex_compile.  */

  bits_list_type level_match_status;
  unsigned current_level = 0;
  bits_list_type group_match_status;
  bits_list_type group_active_status;
  char *bend = bufp->buffer + bufp->used;
  char *previous_p = NULL;


  if (!(init_bits_list (&level_match_status) 
        && init_bits_list (&group_match_status)
        && init_bits_list (&group_active_status)))
    {
      *enough_memory = false;
      return true;
    }
  else
    *enough_memory = true;

  for (this_op_offset = op_list.avail - 1; this_op_offset >= 0; 
       this_op_offset--)  
    {
      char *p = bufp->buffer + op_list.offsets[this_op_offset];

      if (!enough_memory)
        break;

      switch ((enum regexpcode) *p)
        {
          case endline:
          case endline_in_repeat:
          case endline_before_newline:
          case repeated_endline_before_newline:

           /* If the '$' must be at the pattern's end or else is
              in a trailing position.  */
	   
            if ((bufp->syntax & RE_ANCHORS_ONLY_AT_ENDS) 
                 ? p + 1 == bend
		 : ((bufp->syntax & RE_TIGHT_ALT)
                    ? p + 3 == bend  /* Would have two following no_ops.  */
                    : (*p == endline_before_newline
                       || *p == repeated_endline_before_newline
                       || no_levels_match_anything (level_match_status))))
	      {
                if ((enum regexpcode) *p == endline_in_repeat
                    || (enum regexpcode) *p == repeated_endline_before_newline)
                  if (bufp->syntax & RE_REPEATED_ANCHORS_AWAY)
                    *p = no_op;
    		  else
		    *p = endline;
		

                /* If this is a trailing '$' in an empty alternative.  */

                if ((bufp->syntax & RE_NO_EMPTY_ALTS)
	
                    /* If there's an alternation op right before this `$'.  */
                    && ((this_op_offset > 0 
			 && *(bufp->buffer 
	                      + op_list.offsets[this_op_offset - 1])
                            == jump_past_next_alt)
	
                     /* Or this `$' is the only thing in the first
                        alternative of more than one of them.  */
	
                        || ((this_op_offset == 0   /* It's first.  */
			     /* Or it's right after an open-group op.  */
                             || (this_op_offset > 0 
				 && *(bufp->buffer 
	                              + op_list.offsets[this_op_offset - 1])
                                    == start_memory)) 

                            /* And it's right before an alternation op.  */
	                    && previous_p != NULL 
                            && *previous_p == jump_past_next_alt)))
                  return false;
              }
	
            else if (bufp->syntax & RE_CONTEXT_INVALID_ANCHORS)
	      return false;

            else if (!(bufp->syntax & RE_CONTEXT_INDEP_ANCHORS))
              {
                p[0] = (char)exactn; 
                p[1] = (char)1;
                p[2] = '$';
              }
              
            break;


          /* Yes, start and stop_memory are switched because we're going
             backwards through the pattern!  */
    
          case stop_memory:
            increase_level (&current_level);

            if (!make_group_active (&group_active_status, p[1]))
	      enough_memory = false;	      

            break;

          case start_memory:
            if (get_level_match_status (level_match_status, current_level))
              if (!set_next_lower_level (&level_match_status, current_level))
                  enough_memory = false;
              else
                {
                  decrease_level (&current_level);
                  make_group_inactive (&group_active_status, p[1]);
                }
      
            break;               


          /* Hit an alternative.  */

          case jump_past_next_alt:
            if (lower_levels_match_nothing (level_match_status, current_level))
              clear_this_and_higher_levels (&level_match_status,current_level);

            break;

	  /* These below mean was followed by a repetition operator.  */
	  case no_op:
          case maybe_pop_jump:
	  case no_pop_jump_n:
	    if (bufp->syntax & RE_REPEATED_ANCHORS_AWAY)
              break;
          case charset:
	  case charset_not:
	  case wordchar:
	  case notwordchar:
	  case exactn:
          case anychar:
            if (!set_this_level (&level_match_status, current_level)
                || !set_match_status_of_active_groups (group_active_status,
                                                       &group_match_status))
              enough_memory = false;;

            break;

	  case duplicate: 
            /* Only set level_match_status if this back reference
               refers to a nonempty group.  */

            if (get_group_match_status (group_forward_match_status, p[1]))
              if (!set_this_level (&level_match_status, current_level))
                enough_memory = false;

            break;
                
          default: 
            printf ("Found an unknown operator %u in compiled pattern.\n", *p);
        }
      previous_p = p;
    }
  return true;
}


 
/* Bits list routines.  (See above for macros.)  */

/* Initialize BITS_LIST_PTR to have one bits block.  
   Return 1 if there's enough memory to do so and 0 if there isn't.  */

static unsigned
init_bits_list (bits_list_ptr)
  bits_list_type *bits_list_ptr;
{
  bits_list_ptr->bits = (unsigned *) malloc (sizeof (unsigned));

  if (bits_list_ptr->bits == NULL)
    return 0;

  bits_list_ptr->size = BITS_BLOCK_SIZE;
  bits_list_ptr->bits[0] = 0;

  return 1;
}
 
 
/* Extend BITS_LIST_PTR by one bits block.
   Return 1 if there's enough memory to do so and 0 if there isn't.  */

static unsigned
extend_bits_list (bits_list_ptr)
  bits_list_type *bits_list_ptr;
{
  bits_list_ptr->bits
    = (unsigned *) realloc (bits_list_ptr->bits, 
			    bits_list_ptr->size + sizeof (unsigned));

  if (bits_list_ptr->bits == NULL)
    return 0;

  bits_list_ptr->size += BITS_BLOCK_SIZE;
  bits_list_ptr->bits[(bits_list_ptr->size/BITS_BLOCK_SIZE) - 1] = 0;

  return 1;
}


/* Get the bit value at a positive POSITION in BITS_LIST.  */

static unsigned
get_bit (bits_list, position)
  bits_list_type bits_list;
  unsigned position;
{
  if (position < 0)
    {
      printf ("Tried to get a bit at position less than zero.\n");
      exit (1);
    }

  if (position > bits_list.size - 1)
    {
      printf ("Getting bit value: position %d exceeds bits list size %d.\n", 
	      position, bits_list.size);
      exit (1);
    }

  return bits_list.bits[BITS_BLOCK (position)] & BITS_MASK (position);
}


/* Set the bit for a positive POSITION in BITS_LIST_PTR to VALUE, which,
   in turn, can only be 0 or 1.  
   
   Returns 1 if can set the bit and 0 if ran out of memory allocating
   (if necessary) room for it.  */
   
static unsigned
set_bit_to_value (bits_list_ptr, position, value)
  bits_list_type *bits_list_ptr;
  unsigned position;
  unsigned value;
{
  if (position < 0)
    {
      printf ("Tried to set a bit at position less than zero.\n");
      exit (1);
    }

  if (position > bits_list_ptr->size - 1
      && !extend_bits_list (bits_list_ptr))
    return 0;

  if (value == 1)
    bits_list_ptr->bits[BITS_BLOCK (position)] |= BITS_MASK (position);
  else if (value == 0)
    bits_list_ptr->bits[BITS_BLOCK (position)] &= ~(BITS_MASK (position));
  else
    {
      printf ("Invalid value %d to set a bit.\n");
      exit (1);
    }
  return 1;
}


/* Level stuff.  */


/* Return 1 if LEVEL in LEVEL_MATCH_STATUS matches something and
   0 if it doesn't.  Assumes LEVEL is positive.   */

static unsigned
get_level_match_status (level_match_status, level)
  bits_list_type level_match_status;
  unsigned level;
{
  return get_bit (level_match_status, level);
}


/* Mark as matching something the level LEVEL in LEVEL_MATCH_STATUS_PTR.
   Assumes LEVEL is positive. 
   
   Return 1 if can mark the level and 0 if need to allocate space for it
   but can't.  */

static unsigned
set_this_level (level_match_status_ptr, level)
  bits_list_type *level_match_status_ptr;
  unsigned level;
{
  return set_bit_to_value (level_match_status_ptr, level, 1);
}


/* Mark as matching something the level below the LEVEL recorded in
   LEVEL_MATCH_STATUS_PTR.  Assumes LEVEL is greater than zero.
   
   Return 1 if can mark the level and 0 ran out of memory trying to do so.  */

static unsigned
set_next_lower_level (level_match_status_ptr, level)
  bits_list_type *level_match_status_ptr;
  unsigned level;
{
  unsigned this_level;

  return set_bit_to_value (level_match_status_ptr, level - 1, 1);
}


/* Mark as matching something the level LEVEL and all levels higher than
   it currently in LEVEL_MATCH_STATUS_PTR.  Assumes LEVEL is positive.

   Return 1 if can mark the levels and 0 ran out of memory trying to do so.  */

static void
clear_this_and_higher_levels (level_match_status_ptr, level)
  bits_list_type *level_match_status_ptr;
  unsigned level;
{
  unsigned this_level;

  for (this_level = level; 
       this_level < level_match_status_ptr->size; 
       this_level++)
    set_bit_to_value (level_match_status_ptr, this_level, 0);
}


/* Returns true if none of the levels in LEVEL_MATCH_STATUS less than a
   positive LEVEL match anything, and false otherwise.  */

static boolean
lower_levels_match_nothing (level_match_status, level)
  bits_list_type level_match_status;
  unsigned level;
{
  unsigned this_level;
  
  for (this_level = 0; this_level < level; this_level++)
    if (get_bit (level_match_status, this_level))
      return false;

  return true;
}

/* Returns true if none of the levels in LEVEL_MATCH_STATUS match
   anything, and false otherwise.  */

static boolean
no_levels_match_anything (level_match_status)
  bits_list_type level_match_status;
{
  unsigned this_bits_block;
  
  for (this_bits_block = 0; 
       this_bits_block < level_match_status.size/BITS_BLOCK_SIZE;
       this_bits_block++)
    if (level_match_status.bits[this_bits_block] != 0)
      return false;

  return true;
}


/* Increase CURRENT_LEVEL_PTR.  */

static void
increase_level (current_level_ptr)
  unsigned *current_level_ptr;
{
  (*current_level_ptr)++;
}


/* Decrease CURRENT_LEVEL_PTR, but exit on error if try to decrease
   below zero.  */

static void
decrease_level (current_level_ptr)
  unsigned *current_level_ptr;
{
  if (*current_level_ptr == 0)
    {
      printf ("Tried to decrease current level below zero.\n");
      exit (1);
    }
  (*current_level_ptr)--;
}


/* Group stuff.  */


/* Mark a positive GROUP in GROUP_ACTIVE_STATUS_PTR as active.  
   Return 1 if can mark the group and 0 ran out of memory trying to do so.  */

static unsigned
make_group_active (group_active_status_ptr, group)
  bits_list_type *group_active_status_ptr;
  unsigned group;
{
  return set_bit_to_value (group_active_status_ptr, group, 1);
}


/* Mark a positive GROUP in GROUP_ACTIVE_STATUS_PTR as inactive.  
   Return 1 if can mark the group and 0 ran out of memory trying to do so.  */

static unsigned
make_group_inactive (group_active_status_ptr, group)
  bits_list_type *group_active_status_ptr;
  unsigned group;
{
  return set_bit_to_value (group_active_status_ptr, group, 0);
}


/* Mark as active in GROUP_MATCH_STATUS_PTR those active groups recorded
   in GROUP_ACTIVE_STATUS_PTR.

   Return 1 if can mark the groups and 0 ran out of memory trying to do so.  */

static unsigned
set_match_status_of_active_groups (group_active_status, group_match_status_ptr)
  bits_list_type group_active_status;
  bits_list_type *group_match_status_ptr;
{
  unsigned this_bit_block;

  if (group_active_status.size > group_match_status_ptr->size
      && !extend_bits_list (group_match_status_ptr))
    return 0;
  
  for (this_bit_block = 0; 
       this_bit_block < group_active_status.size/BITS_BLOCK_SIZE;
       this_bit_block++)
    group_match_status_ptr->bits[this_bit_block] 
      |= group_active_status.bits[this_bit_block];

  return 1;
}


/* Return 1 if GROUP in GROUP_MATCH_STATUS matches something and
   0 if it doesn't.  Assumes GROUP is positive.   */

static unsigned
get_group_match_status (group_match_status, group)
  bits_list_type group_match_status;
  unsigned group;
{
  return get_bit (group_match_status, group);
}





/* Failure stack declarations and macros for both re_compile_fastmap and
   re_match_2.  Have to use `alloca' for reasons stated in INIT_BITS_LIST's
   comment.  */
   

/* Roughly the maximum number of failure points on the stack.  Would be
   exactly that if always used MAX_FAILURE_SPACE each time we failed.  */
   
int re_max_failures = 2000;


typedef unsigned char *failure_stack_element;

typedef struct {
    failure_stack_element *stack;
    unsigned size;
    unsigned avail;			/* Offset of next open position.  */
  } failure_stack_type;


#define FAILURE_STACK_EMPTY (failure_stack.avail == 0)
#define FAILURE_STACK_PTR_EMPTY  (failure_stack_ptr->avail == 0)
#define FAILURE_STACK_FULL (failure_stack.avail == failure_stack.size)


/* Initialize a failure stack.

   Return 1 if was able to allocate the space for (FAILURE_STACK) and
   0 if not.  */

#define INIT_FAILURE_STACK(failure_stack)				\
  ((failure_stack).stack = (failure_stack_element *)			\
    REGEX_ALLOCATE (INIT_FAILURE_ALLOC * sizeof (failure_stack_element)),\
									\
  (failure_stack).stack == NULL						\
  ? 0									\
  : ((failure_stack).size = INIT_FAILURE_ALLOC,				\
     (failure_stack).avail = 0,						\
     1))


/* Double the size of FAILURE_STACK, up to MAX_SIZE.  

   Return 1 if was able to double it, and 0 if either ran out of memory
   allocating space for it or it was already MAX_SIZE large.  
   
   REGEX_REALLOCATE requires `void *destination' be declared.   */

#define DOUBLE_FAILURE_STACK(failure_stack, max_size)			\
  ((failure_stack).size > max_size					\
   ? 0									\
   : ((failure_stack).stack = (failure_stack_element *)			\
        REGEX_REALLOCATE ((failure_stack).stack, 			\
          ((failure_stack).size << 1) * sizeof (failure_stack_element)),\
									\
      (failure_stack).stack == NULL					\
      ? 0								\
      : ((failure_stack).size <<= 1, 					\
         1)))


/* Push PATTERN_OP on (FAILURE_STACK). 

   Return 1 if was able to do so and 0 if ran out of memory allocating
   space to do so.

   DOUBLE_FAILURE_STACK requires `void *destination' be declared.   */

#define PUSH_PATTERN_OP(pattern_op, failure_stack)			\
  ((FAILURE_STACK_FULL							\
    && !DOUBLE_FAILURE_STACK (failure_stack, re_max_failures))		\
    ? 0									\
    : ((failure_stack).stack[(failure_stack).avail++] = pattern_op,	\
       1))


/* Push most of the information about the state we will want
   if we ever fail back to it.  
   
   Requires regstart, regend, reg_info, and num_internal_regs be declared.
   DOUBLE_FAILURE_STACK requires `void *destination' be declared.   
   
   Does a `return FAILURE_CODE' if runs out of memory.  */

#define PUSH_FAILURE_POINT(pattern_place, string_place, failure_stack, failure_code)				\
  do {									\
    long highest_used_reg, this_reg;					\
    void *destination;							\
                                                                        \
    /* Find out how many registers are active or have been matched.	\
       (Aside from register zero, which is only set at the end.)  */	\
									\
    for (highest_used_reg = num_internal_regs - 1; highest_used_reg > 0;\
         highest_used_reg--)						\
      if (regstart[highest_used_reg] != (unsigned char *) -1)		\
        break;								\
									\
    while (REMAINING_AVAIL_SLOTS < NUM_FAILURE_ITEMS)			\
      if (!DOUBLE_FAILURE_STACK (failure_stack, 			\
          re_max_failures * MAX_FAILURE_ITEMS))				\
        return failure_code;						\
									\
    /* Now push the info for each of those registers.  */		\
									\
    for (this_reg = 1; this_reg <= highest_used_reg; this_reg++)	\
      {									\
        (failure_stack).stack[(failure_stack).avail++] 			\
          = regstart[this_reg];						\
									\
        (failure_stack).stack[(failure_stack).avail++] = regend[this_reg];\
									\
        (failure_stack).stack[(failure_stack).avail++] 			\
          = (unsigned char *) &reg_info[this_reg];			\
      }									\
									\
    /* Push how many registers we saved.  */				\
    (failure_stack).stack[(failure_stack).avail++]			\
      = (unsigned char *) highest_used_reg;				\
									\
    (failure_stack).stack[(failure_stack).avail++] = pattern_place;	\
    (failure_stack).stack[(failure_stack).avail++] = string_place;	\
  } while (0)




/* Given a pattern, compute a fastmap from it.  The fastmap records
   which of the (1 << BYTEWIDTH) possible characters can start a string
   that matches the pattern.  This fastmap is used by re_search to skip
   quickly over totally impossible text.

   The caller must supply the address of a (1 << BYTEWIDTH)-byte data 
   area as bufp->fastmap.
   The other components of bufp describe the pattern to be used.  
   
   Returns 0 if it can compile a fastmap.
   Returns -2 if there is an internal error.   */

int
re_compile_fastmap (bufp)
     struct re_pattern_buffer *bufp;
{
  unsigned char *pattern = (unsigned char *) bufp->buffer;
  int size = bufp->used;
  register char *fastmap = bufp->fastmap;
  unsigned char *p = pattern;
  register unsigned char *pend = pattern + size;
  int j, k;
  unsigned char *translate = (unsigned char *) bufp->translate;
  failure_stack_type failure_stack;
  void *destination;


  INIT_FAILURE_STACK (failure_stack);

  bzero (fastmap, (1 << BYTEWIDTH));
  bufp->fastmap_accurate = 1;
  bufp->can_be_null = 0;
      
  while (p)
    {
      boolean is_a_succeed_n = false;
      
      if (p == pend)
        if (FAILURE_STACK_EMPTY)	  
          {
            bufp->can_be_null = 1;
            break;
          }
        else
          p = failure_stack.stack[--failure_stack.avail];
          
        
#ifdef SWITCH_ENUM_BUG
      switch ((int) ((enum regexpcode) *p++))
#else
      switch ((enum regexpcode) *p++)
#endif
	{
	case exactn:
          fastmap[translate ? translate[p[1]] : p[1]] = 1;
	  break;

        case begline:
        case before_dot:
	case at_dot:
	case after_dot:
	case begbuf:
	case endbuf:
	case wordbound:
	case notwordbound:
	case wordbeg:
	case wordend:
          continue;

	case endline:
          fastmap[translate ? translate['\n'] : '\n'] = 1;
            
	  if (! bufp->can_be_null)
	    bufp->can_be_null = 2;
	  break;

	case no_pop_jump_n:
        case pop_failure_jump:
	case maybe_pop_jump:
	case no_pop_jump:
        case jump_past_next_alt:
	case dummy_failure_jump:
          extract_number_and_incr (&j, &p);
	  p += j;	
	  if (j > 0)
	    continue;
            
          /* Jump backward reached implies we just went through
             the body of a loop and matched nothing.  Opcode jumped to
             should be an on_failure_jump or succeed_n.  Just treat it
             like an ordinary jump.  For a * loop, it has pushed its
             failure point already; If so, discard that as redundant.  */

          if ((enum regexpcode) *p != on_failure_jump
	      && (enum regexpcode) *p != succeed_n)
	    continue;

          p++;
          extract_number_and_incr (&j, &p);
          p += j;		
	  
          /* If what's on the stack is where we are now, pop it.  */
          
          if (!FAILURE_STACK_EMPTY 
	      && failure_stack.stack[failure_stack.avail - 1] == p)
            failure_stack.avail--;
            
          continue;
	  
        case on_failure_jump:
	handle_on_failure_jump:
          extract_number_and_incr (&j, &p);

          if (!PUSH_PATTERN_OP (p + j, failure_stack))
            return -2;

          if (is_a_succeed_n)
            extract_number_and_incr (&k, &p);	/* Skip the n.  */

          continue;

	case succeed_n:
	  is_a_succeed_n = true;
          /* Get to the number of times to succeed.  */
          p += 2;		
	  /* Increment p past the n for when k != 0.  */
          extract_number_and_incr (&k, &p);
          if (k == 0)
	    {
              p -= 4;
              goto handle_on_failure_jump;
            }
          continue;
          
	case set_number_at:
          p += 4;
          continue;

        case start_memory:
	case stop_memory:
	  p++;
	  continue;

	case duplicate:
	  bufp->can_be_null = 1;
	  fastmap['\n'] = 1;
	case anychar:
	  for (j = 0; j < (1 << BYTEWIDTH); j++)
	    if (j != '\n')
	      fastmap[j] = 1;
	  if (bufp->can_be_null)
	    return 0;
	  /* Don't return; check the alternative paths
	     so we can set can_be_null if appropriate.  */
	  break;

	case wordchar:
	  for (j = 0; j < (1 << BYTEWIDTH); j++)
	    if (SYNTAX (j) == Sword)
	      fastmap[j] = 1;
	  break;

	case notwordchar:
	  for (j = 0; j < (1 << BYTEWIDTH); j++)
	    if (SYNTAX (j) != Sword)
	      fastmap[j] = 1;
	  break;

#ifdef emacs
	case syntaxspec:
	  k = *p++;
	  for (j = 0; j < (1 << BYTEWIDTH); j++)
	    if (SYNTAX (j) == (enum syntaxcode) k)
	      fastmap[j] = 1;
	  break;

	case notsyntaxspec:
	  k = *p++;
	  for (j = 0; j < (1 << BYTEWIDTH); j++)
	    if (SYNTAX (j) != (enum syntaxcode) k)
	      fastmap[j] = 1;
	  break;
#endif /* not emacs */

        case charset:
          for (j = *p++ * BYTEWIDTH - 1; j >= 0; j--)
	    if (p[j / BYTEWIDTH] & (1 << (j % BYTEWIDTH)))
              fastmap[translate ? translate[j] : j] = 1;
	  break;

	case charset_not:
	  /* Chars beyond end of map must be allowed.  */
	  for (j = *p * BYTEWIDTH; j < (1 << BYTEWIDTH); j++)
            fastmap[translate ? translate[j] : j] = 1;

	  for (j = *p++ * BYTEWIDTH - 1; j >= 0; j--)
	    if (!(p[j / BYTEWIDTH] & (1 << (j % BYTEWIDTH))))
              fastmap[translate ? translate[j] : j] = 1;

          break;
	} /* End switch *p++.  */

      /* Getting here means we have successfully found the possible starting
         characters of one path of the pattern.  We need not follow this
         path any farther.  Instead, look at the next alternative
         remembered in the stack.  */

      if (!FAILURE_STACK_EMPTY)
        p = failure_stack.stack[--failure_stack.avail];
      else
	break;
    }
  return 0;
} /* re_compile_fastmap  */




/* Like re_search_2, below, but only one string is specified, and
   doesn't let you say where to stop matching. */

int
re_search (bufp, string, size, startpos, range, regs)
     struct re_pattern_buffer *bufp;
     const char *string;
     const int size, startpos, range;
     struct re_registers *regs;
{
  return re_search_2 (bufp, (char *) 0, 0, string, size, startpos, range, 
		      regs, size);
}


/* Using the compiled pattern in BUFP->buffer, first tries to match the
   virtual concatenation of STRING1 and STRING2, starting first at index
   STARTPOS, then at STARTPOS + 1, and so on.  RANGE is the number of
   places to try before giving up.  If RANGE is negative, it searches
   backwards, i.e., the starting positions tried are STARTPOS, STARTPOS - 1, 
   etc.  STRING1 and STRING2 have length SIZE1 and SIZE2, respectively.
   In REGS, return the indices of the virtual concatenation of STRING1
   and STRING2 that matched the entire BUFP->buffer and its contained
   subexpressions.  Do not consider matching one past the index MSTOP in
   the virtual concatenation of STRING1 and STRING2.

   The value returned is the position in the strings at which the match
   was found, -1 if no match was found, or -2 if error (such as
   failure stack overflow).  */

int
re_search_2 (bufp, string1, size1, string2, size2, startpos, range,
	     regs, stop)
     struct re_pattern_buffer *bufp;
     const char *string1, *string2;
     const int size1, size2;
     const int startpos;
     const int range;
     struct re_registers *regs;
     const int stop;
{
  register char *fastmap = bufp->fastmap;
  register unsigned char *translate = (unsigned char *) bufp->translate;
  int total_size = size1 + size2;
  int private_startpos = startpos;
  int private_endpos = startpos + range;
  int private_range = range;
  int val;
  const struct re_pattern_buffer *private_bufp;

  /* Check for out-of-range starting position.  */
  if (private_startpos < 0  ||  private_startpos > total_size)
    return -1;
    
  /* Fix up range if it would eventually take private_startpos outside
     of the virtual concatenation of string1 and string2.  */
     
  if (private_endpos < -1)
    private_range = -1 - private_startpos;

  else if (private_endpos > total_size)
    private_range = total_size - private_startpos;

  
/* Update the fastmap now if not correct already.  */
  if (fastmap && !bufp->fastmap_accurate)
    if (re_compile_fastmap (bufp) == -2)
      return -2;
  
  /* If the search isn't to be a backwards one, don't waste time in a
     long search for a pattern that says it is anchored.  */
  if (bufp->used > 0 && (enum regexpcode) bufp->buffer[0] == begbuf
      && private_range > 0)
    {
      if (private_startpos > 0)
	return -1;
      else
	private_range = 1;
    }

  private_bufp = bufp;

  while (1)
    { 
      /* If a fastmap is supplied, skip quickly over characters that
         cannot possibly be the start of a match.  Note, however, that
         if the pattern can possibly match the null string, we don't
         want to skip over characters; we want the first null string we
         can match.  */

      if (fastmap && private_startpos < total_size && !bufp->can_be_null)
	{
	  if (private_range > 0)	/* Searching forwards.  */
	    {
	      register int lim = 0;
	      register unsigned char *p;
	      int irange = private_range;

              if (private_startpos < size1 
                  && private_startpos + private_range >= size1)
                lim = private_range - (size1 - private_startpos);

	      p = ((unsigned char *)
		   &(private_startpos >= size1 
                   ? string2 - size1 
                   : string1)[private_startpos]);

              while (private_range > lim && !fastmap[translate 
                                             ? translate[*p++]
                                             : *p++])
		    private_range--;
	      private_startpos += irange - private_range;
	    }
	  else				/* Searching backwards.  */
	    {
	      register unsigned char c;

              if (size1 == 0 || private_startpos >= size1)
		c = string2[private_startpos - size1];
	      else 
		c = string1[private_startpos];

              c &= 0xff;
	      if (translate ? !fastmap[translate[c]] : !fastmap[c])
		goto advance;
	    }
	}

      if (private_range >= 0 && private_startpos == total_size
	  && fastmap && bufp->can_be_null == 0)
	return -1;

      val = re_match_2 (private_bufp, string1, size1, string2, size2,
	                private_startpos, regs, stop);
      if (val >= 0)
	return private_startpos;
        
      if (val == -2)
	return -2;

    advance:
      if (!private_range) 
        break;
      else if (private_range > 0) 
        {
          private_range--; 
          private_startpos++;
        }
      else
        {
          private_range++; 
          private_startpos--;
        }
    }
  return -1;
}



#ifndef emacs   /* emacs never uses this.  */
int
re_match (bufp, string, size, pos, regs)
     const struct re_pattern_buffer *bufp;
     const char *string;
     const int size, pos;
     struct re_registers *regs;
{
  return re_match_2 (bufp, (char *) 0, 0, string, size, pos, regs, size); 
}
#endif /* not emacs */



/* Routines for re_match_2, defined below.  */

static boolean group_can_match_nothing ();
static int bcmp_translate ();


/* Macros used by re_match_2, defined below:  */

/* Structure and accessing macros used in re_match_2:  */

typedef struct register_info
{
  bits_list_type inner_groups;	    /* Which groups are inside this one.  */
  int can_match_nothing;   	    /* Set if this group can match nothing;
				       -1 if not ever set.  */
  unsigned is_active : 1;
  unsigned matched_something : 1;
  unsigned ever_matched_something : 1;
} reg_info_type;
 

/* Macros used by re_match_2:  */
/* I.e., regstart, regend, and reg_info.  */

#define INNER_GROUPS(R)  ((R).inner_groups)
#define CAN_MATCH_NOTHING(R)  ((R).can_match_nothing)
#define IS_ACTIVE(R)  ((R).is_active)
#define MATCHED_SOMETHING(R)  ((R).matched_something)
#define EVER_MATCHED_SOMETHING(R)  ((R).ever_matched_something)


/* Record that group INNER is inside of all currently active groups.  */

#define NOTE_INNER_GROUP(inner)						\
  do { unsigned this_reg;						\
    for (this_reg = 0; this_reg < num_internal_regs; this_reg++)	\
      {									\
	void *destination;	/* For SET_BIT_TO_VALUE.  */		\
        int ret = SET_BIT_TO_VALUE (INNER_GROUPS (reg_info[this_reg]),	\
				    inner, 				\
                                    IS_ACTIVE(reg_info[this_reg])); 	\
        if (ret == 0)							\
          {								\
            printf ("Ran out of memory in re_match_2 (NOTE_INNER_GROUP).\n");\
            exit (1);							\
          }								\
        if (ret != 1)							\
          {								\
            printf ("Invalid value %d to set a bit.\n", ret);		\
            exit (1);							\
          }								\
      }									\
  } while (0)


/* Call this when have matched something; it sets `matched' flags for the
   registers corresponding to the group of which we currently are inside.  
   Also records whether this group ever matched something.  */

#define SET_REGS_MATCHED 						\
  do { unsigned this_reg; 						\
    for (this_reg = 0; this_reg < num_internal_regs; this_reg++)	\
      {									\
        MATCHED_SOMETHING (reg_info[this_reg]) =			\
        EVER_MATCHED_SOMETHING (reg_info[this_reg]) = 			\
          (IS_ACTIVE (reg_info[this_reg])) ? 1 : 0;			\
      }									\
  } while (0)



/* Failure stack macros for re_match_2.  */

/* This is the number of items that are pushed and popped on the stack
   for each register, i.e., its REGSTART, REGEND and REG_INFO.  */

#define NUM_REG_ITEMS  3

/* Refers to highest_used_reg (which we calculate), PATTERN_PLACE and
   STRING_PLACE, which are arguments to the PUSH_FAILURE_POINT macro.  */
   
#define NUM_OTHER_ITEMS 3

/* We put at most these many items on the stack whenever we push a
   failure point .  */

#define MAX_FAILURE_ITEMS						\
  (num_internal_regs * NUM_REG_ITEMS + NUM_OTHER_ITEMS)


/* We really push this many items when pushing a failure point.  We
   calculate highest_used_reg each time.  */

#define NUM_FAILURE_ITEMS						\
  (highest_used_reg * NUM_REG_ITEMS + NUM_OTHER_ITEMS)

/* How many items can still be added to the stack without overflowing it.  */
#define REMAINING_AVAIL_SLOTS						\
  (failure_stack.size - failure_stack.avail)



#define MATCHING_IN_FIRST_STRING  (dend == end_match_1)

/* Is true if there is a first string and if PTR is pointing anywhere
   inside it or just past the end.  */
   
#define IS_IN_FIRST_STRING(ptr) 					\
	(size1 && string1 <= (ptr) && (ptr) <= string1 + size1)

/* Call before fetching a character with *d.  This switches over to
   string2 if necessary.  */

#define PREFETCH							\
 while (d == dend)						    	\
  {									\
    /* end of string2 => fail.  */					\
    if (dend == end_match_2) 						\
      goto fail;							\
    /* end of string1 => advance to string2.  */ 			\
    d = string2;						        \
    dend = end_match_2;							\
  }



/* Test if at very beginning or at very end of the virtual concatenation
   of string1 and string2.  If there is only one string, we've put it in
   string2.  */

#define AT_STRINGS_BEG  (d == (size1 ? string1 : string2)  ||  !size2)
#define AT_STRINGS_END  (d == end2)	

#define AT_WORD_BOUNDARY						\
  (AT_STRINGS_BEG || AT_STRINGS_END || IS_A_LETTER (d - 1) != IS_A_LETTER (d))

/* We have two special cases to check for: 
     1) if we're past the end of string1, we have to look at the first
        character in string2;
     2) if we're before the beginning of string2, we have to look at the
        last character in string1; we assume there is a string1, so use
        this in conjunction with AT_STRINGS_BEG.  */

#define IS_A_LETTER(d)							\
  (SYNTAX ((d) == end1 ? *string2 : (d) == string2 - 1 ? *(end1 - 1) : *(d))\
   == Sword)

#ifdef REGEX_MALLOC
#define FREE_VARIABLES							\
  do {									\
    free (failure_stack.stack);						\
    free (regstart);							\
    free (regend);							\
    free (old_regstart);						\
    free (old_regend);							\
    free (reg_info);							\
    free (best_regstart);						\
    free (best_regend);							\
    reg_info = NULL;							\
    failure_stack.stack = NULL;						\
    regstart = regend = old_regstart = old_regend			\
      = best_regstart = best_regend = NULL;				\
  } while (0)
#endif



/* The main matching routine, re_match_2.  */

static void pop_failure_point();


/* re_match_2 matches a buffer full of byte commands for matching (gotten
   from compiling a regular expression) and matches it against the
   the virtual concatenation of its two string arguments.
   
   BUFP	     is a struct re_pattern_buffer * whose pertinent fields are
  	     mentioned below:
                
             It has a char * field BUFFER which points to the byte
             commands which make up the compiled pattern.
             
             Its char * field TRANSLATE, if not 0, translates all
             ordinary elements in the compiled pattern.

             Its int field SYNTAX is the syntax with which the pattern
             was compiled and hence should be matched with.
             
             The long field USED is how many bytes long the compiled
             pattern is.
             
             Its size_t field RE_NSUB contains how many subexpressions
             the pattern has.
             
	     It ignores its NO_SUB bit.
             
             If its RETURN_DEFAULT_NUM_REGS bit is set, then if REGS is
             nonzero, re_match_2 reports in REGS->start[i] and
             REGS->end[i], for i = 1 to BUFP->RE_NSUB + 1, which
             substring of the virtual concatenation of STRING1 and
             STRING2 matched the i-th subexpression of the regular
             expression compiled in BUFFER; it records in REGS->start[0]
             and REGS->end[0] information about all of that
             concatenation.  If RETURN_DEFAULT_NUM_REGS isn't set,
             re_match_2 returns in REGS similar information about i
             things for i = 1 to REGS->num_regs.  If REGS is zero,
             re_match_2 ignores it.  See the comment for `struct
             re_registers' for more details.

   STRING1 and STRING2
            are the addresses of the strings of which re_match_2 tries
            to match the virtual concatenation. Because of this
            concatenation, this function can be used on an Emacs
            buffer's contents.
            
   SIZE1    is the size of STRING1.

   SIZE2    is the size of STRING2.
   
   POS	    is the index in the virtual concatenation of STRING1 and
	    STRING2 at which re_match_2 tries to start the match.
            
   REGS	    is a struct re_registers *.  If it's not zero, then
            re_match_2 will fill its fields START and END with
            information about what substrings of the virtual
            concatenation of STRING1 and STRING2 were matched by the
            groups represented in BUFP's BUFFER field.  You must have
            allocated the correct amount of space in the `start' and
            `end' fields of REGS to accommodate `num_regs' (the other
            field) registers.  See the comment for `struct re_registers'
            in regex.h for more details.
   
   STOP	    is the index in the virtual concatenation of STRING1 and
	    STRING2 beyond which re_match_2 won't consider matching.

   It returns -1 if there is no match, -2 if there is an internal error
   (such as its stack overflowing).  Otherwise, it returns the length of
   the substring it matched.  */

int
re_match_2 (bufp, string1_arg, size1_arg, string2_arg, size2_arg, pos, 
	    regs, stop)
     const struct re_pattern_buffer *bufp;
     const char *string1_arg;
     const int size1_arg;
     const char *string2_arg;
     const int size2_arg;
     const int pos;
     struct re_registers *regs;
     const int stop;
{
  unsigned char *p = (unsigned char *) bufp->buffer;
  unsigned char *p1;

  /* Pointer to beyond end of buffer.  */
  register unsigned char *pend = p + bufp->used;

  unsigned char *string1 = (unsigned char *) string1_arg;
  unsigned char *string2 = (unsigned char *) string2_arg;
  int size1 = size1_arg;
  int size2 = size2_arg;
  unsigned char *end1;		/* Just past end of first string.  */
  unsigned char *end2;		/* Just past end of second string.  */


  /* Pointers into string1 and string2, just past the last characters in
     each to consider matching.  */
  unsigned char *end_match_1, *end_match_2;

  register unsigned char *d, *dend;
  int mcnt, mcnt2;				/* Multipurpose.  */
  unsigned char *translate = (unsigned char *) bufp->translate;
  unsigned is_a_jump_n = 0;

  /* This is how many registers the caller wants.  */
  unsigned num_regs_wanted = regs 
	       		     ? bufp->return_default_num_regs
                               ? bufp->re_nsub + 1
                               : regs->num_regs
                             : 0;

  /* Want to fill *all* the registers internally. */
  unsigned num_internal_regs = bufp->re_nsub + 1;

  void *destination;				/* For REGEX_REALLOCATE.  */
  

 /* Failure point stack.  Each place that can handle a failure further
    down the line pushes a failure point on this stack.  It consists of
    restart, regend, and reg_info for all registers corresponding to the
    subexpressions we're currently inside, plus the number of such
    registers, and, finally, two char *'s.  The first char * is where to
    resume scanning the pattern; the second one is where to resume
    scanning the strings.  If the latter is zero, the failure point is a
    ``dummy''; if a failure happens and the failure point is a dummy, it
    gets discarded and the next next one is tried.  */

  failure_stack_type failure_stack;

  /* Information on the contents of registers. These are pointers into
     the input strings; they record just what was matched (on this
     attempt) by a subexpression part of the pattern, that is, the
     regnum-th regstart pointer points to where in the pattern we began
     matching and the regnum-th regend points to right after where we
     stopped matching the regnum-th subexpression.  (The zeroth register
     keeps track of what the whole pattern matches.)  */
     
  unsigned char **regstart = (unsigned char **) 
    REGEX_ALLOCATE (num_internal_regs * sizeof (unsigned char *));
  unsigned char **regend = (unsigned char **) 
    REGEX_ALLOCATE (num_internal_regs * sizeof (unsigned char *));

  /* If a group that's operated upon by a repetition operator fails to
     match anything, then the register for its start will need to be
     restored because it will have been set to wherever in the string we
     are when we last see its open-group operator.  The argument is
     similar for a register's end.  */
     
  unsigned char **old_regstart 
    = (unsigned char **) REGEX_ALLOCATE (num_internal_regs * sizeof (unsigned char *));
  unsigned char **old_regend
    = (unsigned char **) REGEX_ALLOCATE (num_internal_regs * sizeof (unsigned char *));

  /* The is_active field of reg_info helps us keep track of which (possibly
     nested) subexpressions we are currently in. The matched_something
     field of reg_info[reg_num] helps us tell whether or not we have
     matched any of the pattern so far this time through the reg_num-th
     subexpression.  These two fields get reset each time through any
     loop their register is in.  */

  struct register_info *reg_info = (struct register_info *) 
	 REGEX_ALLOCATE (num_internal_regs * sizeof (struct register_info));


  /* The following record the register info as found in the above
     variables when we find a match better than any we've seen before. 
     This happens as we backtrack through the failure points, which in
     turn happens only if we have not yet matched the entire string.  */

  unsigned best_regs_set = 0;

  unsigned char **best_regstart
    = (unsigned char **) REGEX_ALLOCATE (num_internal_regs * sizeof (unsigned char *));

  unsigned char **best_regend
    = (unsigned char **) REGEX_ALLOCATE (num_internal_regs * sizeof (unsigned char *));

  unsigned current_reg = 0;

  /* End of declarations.  */  
  
  
  if (!INIT_FAILURE_STACK (failure_stack))
    return -2;
    
  if (!(regstart && regend && old_regstart && old_regend && reg_info 
        && best_regstart && best_regend)) 
    {
#ifdef REGEX_MALLOC
      FREE_VARIABLES;
#endif
      return -2;
    }

  /* The starting position is bogus.  */
  if (pos < 0 || pos > size1 + size2)
    {
#ifdef REGEX_MALLOC
      FREE_VARIABLES;
#endif
      return -1;
    }
    
  
  /* Initialize subexpression text positions to -1 to mark ones that no
     \( or ( and \) or ) has been seen for. Also set all registers to
     inactive and mark them as not having any inner groups, able to
     match the empty string, matched anything so far, or ever failed.  */

  for (mcnt = 0; mcnt < num_internal_regs; mcnt++)
    {
      regstart[mcnt] = regend[mcnt] 
        = old_regstart[mcnt] = old_regend[mcnt] = (unsigned char *) -1;

      if (!INIT_BITS_LIST (INNER_GROUPS (reg_info[mcnt])))
        {
#ifdef REGEX_MALLOC
          FREE_VARIABLES;
#endif
          return -2;
        }
        
      CAN_MATCH_NOTHING (reg_info[mcnt]) = -1;		/* I.e., unset.  */
        /* The bit fields.  */
      IS_ACTIVE (reg_info[mcnt]) = 0;
      MATCHED_SOMETHING (reg_info[mcnt]) = 0;
      EVER_MATCHED_SOMETHING (reg_info[mcnt]) = 0;
    }
  
  IS_ACTIVE (reg_info[0]) = 1;


  if (regs && num_regs_wanted > 0)
    {
      if (bufp->syntax & RE_ALLOCATE_REGISTERS) 
	{
          regs->num_regs = num_regs_wanted;
          regs->start = (int *) malloc (regs->num_regs * sizeof (int));

          if (regs->start == NULL)
            return -2;

          regs->end = (int *) malloc (regs->num_regs * sizeof (int));

          if (regs->end == NULL)
            return -2;
        }
  
    for (mcnt = 0; mcnt < regs->num_regs; mcnt++)
      {
        regs->start[mcnt] = -1;
        regs->end[mcnt] = -1;
      }
  }

  

  /* Set up pointers to ends of strings.
     Don't allow the second string to be empty unless both are empty.  */
  if (size2 == 0)
    {
      string2 = string1;
      size2 = size1;
      string1 = 0;
      size1 = 0;
    }
  end1 = string1 + size1;
  end2 = string2 + size2;


  /* Compute where to stop matching, within the two strings.  */
  if (stop <= size1)
    {
      end_match_1 = string1 + stop;
      end_match_2 = string2;
    }
  else
    {
      end_match_1 = end1;
      end_match_2 = string2 + stop - size1;
    }

  /* `p' scans through the pattern as `d' scans through the data. `dend'
     is the end of the input string that `d' points within. `d' is
     advanced into the following input string whenever necessary, but
     this happens before fetching; therefore, at the beginning of the
     loop, `d' can be pointing at the end of a string, but it cannot
     equal `string2'.  */

  if (size1 != 0 && pos <= size1)
    {
      d = string1 + pos;
      dend = end_match_1;
    }
  else
    {
      d = string2 + pos - size1;
      dend = end_match_2;
    }


  /* This loops over pattern commands.  It exits by returning from the
     function if match is complete, or it drops through if match fails
     at this starting point in the input data.  */

  while (1)
    {
      is_a_jump_n = 0;
      /* End of pattern means we might have succeeded.  */
      if (p == pend)
	{
	  /* If not end of string, try backtracking.  Otherwise done.  */
          if (d != end_match_2)
	    {
              if (!FAILURE_STACK_EMPTY)
                {
                  /* More failure points to try.  */

                  unsigned in_same_string = 
        	          	IS_IN_FIRST_STRING (best_regend[0]) 
	        	        == MATCHING_IN_FIRST_STRING;

                  /* If exceeds best match so far, save it.  */
                  if (! best_regs_set
                      || (in_same_string && d > best_regend[0])
                      || (! in_same_string && ! MATCHING_IN_FIRST_STRING))
                    {
                      best_regs_set = 1;
                      best_regend[0] = d;	/* Never use regstart[0].  */
                      
                      for (mcnt = 1; mcnt < num_internal_regs; mcnt++)
                        {
                          best_regstart[mcnt] = regstart[mcnt];
                          best_regend[mcnt] = regend[mcnt];
                        }
                    }
                  goto fail;	       
                }
              /* If no failure points, don't restore garbage.  */
              else if (best_regs_set)   
                {
	      restore_best_regs:
                  /* Restore best match.  */
                  d = best_regend[0];
                  
                  if (d >= string1 && d <= end1)
                    dend = end_match_1;

		  for (mcnt = 0; mcnt < num_internal_regs; mcnt++)
		    {
		      regstart[mcnt] = best_regstart[mcnt];
		      regend[mcnt] = best_regend[mcnt];
		    }
                }
            } /* if (d != end_match_2) */

	  /* If caller wants register contents data back, convert it 
	     to indices.  */
	  if (regs && regs->num_regs > 0)
	    {
	      regs->start[0] = pos;

              regs->end[0] = MATCHING_IN_FIRST_STRING
	                     ? d - string1
			     : d - string2 + size1;
                
	      for (mcnt = 1; mcnt < regs->num_regs; mcnt++)
		{
                  if (mcnt >= num_internal_regs
                      || regstart[mcnt] == (unsigned char *) -1 
                      || regend[mcnt] == (unsigned char *) -1)
		    {
		      regs->start[mcnt] = -1;
		      regs->end[mcnt] = -1;
		      continue;
		    }
		  
                  regs->start[mcnt] = IS_IN_FIRST_STRING (regstart[mcnt])
				      ? regstart[mcnt] - string1
                                      : regstart[mcnt] - string2 + size1;
                    
                  regs->end[mcnt] = IS_IN_FIRST_STRING (regend[mcnt])
		                    ? regend[mcnt] - string1
                                    : regend[mcnt] - string2 + size1;
		}
	    }

#ifdef REGEX_MALLOC
          FREE_VARIABLES;
#endif
          return d - pos - (MATCHING_IN_FIRST_STRING 
			    ? string1 
			    : string2 - size1);
        }

      /* Otherwise match next pattern command.  */
#ifdef SWITCH_ENUM_BUG
      switch ((int) ((enum regexpcode) *p++))
#else
      switch ((enum regexpcode) *p++)
#endif
	{

	/* \( [or `(', as appropriate] is represented by start_memory,
           \) by stop_memory.  Both of those commands are followed by a
           register number in the next byte.  The text matched within
           the \( and \) is recorded (in the internal registers data
           structure) under that number.  */

        case start_memory:
          /* Find out if this group can match the empty string.  */
	  p1 = p;		/* To send to group_can_match_nothing.  */
          
          if (CAN_MATCH_NOTHING (reg_info[*p]) == -1)
            CAN_MATCH_NOTHING (reg_info[*p]) 
              = group_can_match_nothing (&p1, pend, reg_info);

          /* Save the position in the string where we were the last time
             we were at this open-group operator in case the group is
             operated upon by a repetition operator, e.g., with `(a*)*b'
             against `ab'; then we want to ignore where we are now in
             the string in case this attempt to match fails.  */

          old_regstart[*p] = CAN_MATCH_NOTHING (reg_info[*p])
                             ? ((regstart[*p] == (unsigned char *) -1)
                                 ? d : regstart[*p])
                             : regstart[*p];
          regstart[*p] = d;

          IS_ACTIVE (reg_info[*p]) = 1;
          MATCHED_SOMETHING (reg_info[*p]) = 0;
          p++;
          break;

	case stop_memory:
          /* Save the position we were in the string the last time we
             were at this close-group operator in case the group is
             operated upon by a repetition operator, e.g., with 
             `((a*)*(b*)*)*' against `aba'; then we want to ignore where
             we are now in the string in case this attempt to match
             fails.  */
             
          old_regend[*p] = CAN_MATCH_NOTHING (reg_info[*p])
                           ? ((regend[*p] == (unsigned char *) -1)
                               ? d : regend[*p])
			   : regend[*p];
          regend[*p] = d;
          IS_ACTIVE (reg_info[*p]) = 0;
	  
	  /* Record that this group is inside of all currently active
             groups; makes no sense for group 1.  */
          if (*p != 1)
            NOTE_INNER_GROUP (*p);
            
          
          /* If just failed to match something this time around with a
             group that's operated on by a repetition operator, try to
             force exit from the ``loop,'' and restore the register
             information for this group that we had before trying this
             last match.  */
             
          if ((!MATCHED_SOMETHING (reg_info[*p])
               || (enum regexpcode) p[-3] == start_memory)
	      && (p + 1) != pend)              
            {
              p1 = p + 1;
              mcnt = 0;
              switch ((enum regexcode) *p1++)
                {
                  case no_pop_jump_n:
		    is_a_jump_n = 1;
                  case pop_failure_jump:
		  case maybe_pop_jump:
		  case no_pop_jump:
		  case dummy_failure_jump:
                    extract_number_and_incr (&mcnt, &p1);
		    if (is_a_jump_n)
		      p1 += 2;
                    break;
                }
	      p1 += mcnt;
        
              /* If the next operation is a jump backwards in the pattern
	         to an on_failure_jump right before the start_memory
                 corresponding to this stop_memory, exit from the loop
                 by forcing a failure after pushing on the stack the
                 on_failure_jump's jump in the pattern, and d.  */

              if (mcnt < 0 && (enum regexpcode) *p1 == on_failure_jump
                  && (enum regexpcode) p1[3] == start_memory && p1[4] == *p)
		{
                  /* If this group ever matched anything, then
		     restore what its registers were before trying
                     this last failed match, e.g., with `(a*)*b' against
                     `ab' for regstart[1], and, e.g., with `((a*)*(b*)*)*'
                     against `aba' for regend[3]. 
                     
                     Restore the registers for inner groups, too, e.g.,
                     for `((a*)(b*))*' against `aba' (register 2 gets
                     trashed).  */
                     
                  if (EVER_MATCHED_SOMETHING (reg_info[*p]))
		    {
		      unsigned this_reg; 
	              unsigned bits_mask;
        
                      EVER_MATCHED_SOMETHING (reg_info[*p]) = 0;
                      
		      /* Restore this group's registers.  */
                      
                        regstart[*p] = old_regstart[*p];
                        regend[*p] = old_regend[*p];

		      /* Restore the inner groups' (if any) registers.  */

                      for (this_reg = 0;
                           this_reg < INNER_GROUPS (reg_info[*p]).size;
                           this_reg++)
                        {
                          if (get_bit (INNER_GROUPS (reg_info[*p]), this_reg))
                            {
                              regstart[this_reg] = old_regstart[this_reg];

                              if ((int)old_regend[this_reg] 
                                   >= (int)regstart[this_reg])
                                regend[this_reg] = old_regend[this_reg];
                            }
                        }     
                    }
		  p1++;
                  extract_number_and_incr (&mcnt, &p1);
                  PUSH_FAILURE_POINT (p1 + mcnt, d, failure_stack, -2);

                  goto fail;
                }
            }
          p++;
          break;

	/* \<digit> has been turned into a `duplicate' command which is
           followed by the numeric value of <digit> as the register number.  */
        case duplicate:
	  {
	    register unsigned char *d2, *dend2;
	    int regno = *p++;   /* Get which register to match against.  */

	    /* Can't back reference a group which we've never matched.  */
            if ((regstart[regno] == (unsigned char *) -1
	         || regend[regno] == (unsigned char *) -1)
                && ! bufp->can_be_null)
              goto really_fail;
              
            /* Where in input to try to start matching.  */
            d2 = regstart[regno];
            
            /* Where to stop matching; if both the place to start and
               the place to stop matching are in the same string, then
               set to the place to stop, otherwise, for now have to use
               the end of the first string.  */

            dend2 = ((IS_IN_FIRST_STRING (regstart[regno]) 
		      == IS_IN_FIRST_STRING (regend[regno]))
		     ? regend[regno] : end_match_1);
	    while (1)
	      {
		/* If necessary, advance to next segment in register
                   contents.  */
		while (d2 == dend2)
		  {
		    if (dend2 == end_match_2) break;
		    if (dend2 == regend[regno]) break;

                    /* end of string1 => advance to string2. */
                    d2 = string2;
                    dend2 = regend[regno];
		  }
		/* At end of register contents => success */
		if (d2 == dend2) break;

		/* If necessary, advance to next segment in data.  */
		PREFETCH;

		/* How many characters left in this segment to match.  */
		mcnt = dend - d;
                
		/* Want how many consecutive characters we can match in
                   one shot, so, if necessary, adjust the count.  */
                if (mcnt > dend2 - d2)
		  mcnt = dend2 - d2;
                  
		/* Compare that many; failure if mismatch, else move
                   past them.  */
		if (translate 
                    ? bcmp_translate (d, d2, mcnt, translate) 
                    : bcmp (d, d2, mcnt))
		  goto fail;
		d += mcnt, d2 += mcnt;
	      }
	  }
	  break;

	case anychar:
	  PREFETCH;	  /* Fetch a data character. */
	  /* Match anything but possibly a newline or a null.  */
          if ((!(bufp->syntax & RE_DOT_NEWLINE)
               && (translate ? translate[*d] : *d) == '\n')
              || ((bufp->syntax & RE_DOT_NOT_NULL) 
                  && (translate ? translate[*d] : *d) == '\000'))
	    goto fail;

          SET_REGS_MATCHED;
          d++;
	  break;

	case charset:
	case charset_not:
	  {
	    int not = 0;	    /* Nonzero for charset_not.  */
	    register int c;
	    if (*(p - 1) == (unsigned char) charset_not)
	      not = 1;

	    PREFETCH;	    /* Fetch a data character. */

	    c = translate ? translate[*d] : *d;

	    if (c < *p * BYTEWIDTH
		&& p[1 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
	      not = !not;

	    p += 1 + *p;

	    if (!not) goto fail;
	    SET_REGS_MATCHED;
            d++;
	    break;
	  }

	case begline:
	  if (bufp->not_bol == 1)
            goto fail;

          if (d && (*d == '\n' || d[-1] == '\n')) 
            {
	      if (*d == '\n') 
                d++;
              
              if (bufp->syntax & RE_NO_ANCHOR_AT_NEWLINE)
                goto fail;
              else
                break;
            }
          
          if ((size1 != 0 && d == string1)
              || (size1 == 0 && size2 != 0 && d == string2)
              || (size1 == 0 && size2 == 0))
            break;
          else
            goto fail;
            
	case endline:
	  if (bufp->not_eol == 1)
            goto fail;
            
	  if (d == end2
	      || (d == end1  && size2 == 0))
	    break;

          if (*d == '\n' || (d == end1 && *string2 == '\n'))
            {
	      PREFETCH;

              if (*d == '\n')
                d++;

              if (bufp->syntax & RE_NO_ANCHOR_AT_NEWLINE)
                goto fail;
              else
                break;
            }
	  goto fail;

	/*  Uses of on_failure_jump:
        
           Each alternative starts with an on_failure_jump that points
           to the beginning of the next alternative.  Each alternative
           except the last ends with a jump that in effect jumps past
           the rest of the alternatives.  (They really jump to the
           ending jump of the following alternative, because tensioning
           these jumps is a hassle.)

           Repeats start with an on_failure_jump that points past both
           the repetition text and the following jump or
           pop_failure_jump back to this on_failure_jump.  */

	case on_failure_jump:
        on_failure:
          extract_number_and_incr (&mcnt, &p);
          PUSH_FAILURE_POINT (p + mcnt, d, failure_stack, -2);

          break;


        /* A smart repeat ends with a maybe_pop_jump.
	   We change it either to a pop_failure_jump or a no_pop_jump.  */

        case maybe_pop_jump:
          extract_number_and_incr (&mcnt, &p);
	  {
	    register unsigned char *p2 = p;

            /* Compare the beginning of the repeat with what in the
               pattern follows its end. If we can establish that there
               is nothing that they would both match, i.e., that we
               would have to backtrack because of (as would in, e.g.,
               `a*a') then we can change to pop_failure_jump, because
               we'll never have to backtrack.  */

	    /* Skip over parentheses.  */
	    while (p2 + 1 != pend
		   && (*p2 == (unsigned char) stop_memory
		       || *p2 == (unsigned char) start_memory))
	      p2 += 2;			/* Skip over reg number, too.  */

            if (p2 == pend)
	      p[-3] = (unsigned char) pop_failure_jump;
            else if (*p2 == (unsigned char) exactn
		     || *p2 == (unsigned char) endline)
	      {
		register int c = *p2 == (unsigned char) endline ? '\n' : p2[2];
		register unsigned char *p1 = p + mcnt;

                /* p1[0] ... p1[2] are the on_failure_jump corresponding
                   to the maybe_finalize_jump of this case. Examine what 
                   follows it.  */

                if (p1[3] == (unsigned char) exactn && p1[5] != c)
		  p[-3] = (unsigned char) pop_failure_jump;
		else if (p1[3] == (unsigned char) charset
			 || p1[3] == (unsigned char) charset_not)
		  {
		    int not = p1[3] == (unsigned char) charset_not;
		    if (c < p1[4] * BYTEWIDTH
			&& p1[5 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
		      not = !not;
		    /* `not' is equal to 1 if c would match, which means
                        that we can't change to pop_failure_jump.  */
		    if (!not)
		      p[-3] = (unsigned char) pop_failure_jump;
		  }
	      }
	  }
	  p -= 2;		/* Point at relative address again.  */
	  if (p[-1] != (unsigned char) pop_failure_jump)
	    {
	      p[-1] = (unsigned char) no_pop_jump;	
	      goto no_pop;
	    }
        /* Note fall through.  */

	/* The end of a simple repeat has a pop_failure_jump back to
           its matching on_failure_jump, where the latter will push a
           failure point point.  The pop_failure_jump takes off failure
           points put on by this pop_failure_jump's matching
           on_failure_jump; we got through the pattern to here from the
           matching on_failure_jump, so didn't fail.  Also remove the
           register information put on by the matching on_failure_jump.  */
           
        case pop_failure_jump:
        pop:
          pop_failure_point (&failure_stack);
          /* Note fall through.  */
          
        /* Jump without taking off any failure points.  */

        case no_pop_jump:
	no_pop:
	  extract_number_and_incr (&mcnt, &p);	/* Get the amount to jump.  */
	  p += mcnt;				/* Do the jump.  */
	  break;

	
        /* If the last alternative didn't match anything and empty
           alternatives aren't allowed, then don't skip over the next
           one.  */
   
        case jump_past_next_alt:
	  {
            int this_reg;	/* Counting down.  */

            /* The current register is the innermost (the one with the
               highest number) active one.  */

              for (this_reg = num_internal_regs - 1; 
                   this_reg >= 0; this_reg--)	
                if (IS_ACTIVE (reg_info[this_reg]))
                    break;

            if (!(bufp->syntax & RE_NO_EMPTY_ALTS)
            	|| MATCHED_SOMETHING (reg_info[this_reg]))
              goto no_pop;

            p += 2;			/* Skip past the jump's number.  */
            break;
	  }

        case dummy_failure_jump:
          /* Normally, the on_failure_jump pushes a failure point, which
             then gets popped at pop_failure_jump.  We will end up at
             pop_failure_jump, also, and with a pattern of, say, `a+', we
             are skipping over the on_failure_jump, so we have to push
             something meaningless for pop_failure_jump to pop.  */

          PUSH_FAILURE_POINT (0, 0, failure_stack, -2);
            
          goto no_pop;


        /* Have to succeed matching what follows at least n times.  Then
          just handle like an on_failure_jump.  */
        case succeed_n: 
          mcnt = extract_number (p + 2);
          /* Originally, this is how many times we HAVE to succeed.  */
          if (mcnt)
            {
               mcnt--;
	       p += 2;
               STORE_NUMBER_AND_INCR (p, mcnt);
            }
	  else if (mcnt == 0)
            {
	      p[2] = (char) no_op;
              p[3] = (char) no_op;
              goto on_failure;
            }
          else
	    { 
              fprintf (stderr, "regex: the succeed_n's n is not set.\n");
              exit (1);
	    }
          break;
        
        case no_pop_jump_n: 
          mcnt = extract_number (p + 2);
          /* Originally, this is how many times we CAN jump.  */
          if (mcnt)
            {
               mcnt--;
               STORE_NUMBER(p + 2, mcnt);
	       goto no_pop;	     
            }
          /* If don't have to jump any more, skip over the rest of command.  */
	  else      
	    p += 4;		     
          break;
        
	case set_number_at:
	  {
  	    register unsigned char *p1;

            extract_number_and_incr (&mcnt, &p);
            p1 = p + mcnt;
            extract_number_and_incr (&mcnt, &p);
	    STORE_NUMBER (p1, mcnt);
            break;
          }

        /* Ignore these.  Used to ignore the n of succeed_n's which
           currently have n == 0.  */
        case no_op:
          break;

        case wordbound:
	  if (AT_WORD_BOUNDARY)
	    break;
	  goto fail;

	case notwordbound:
	  if (AT_WORD_BOUNDARY)
	    goto fail;
	  break;

	case wordbeg:
          /* Have to check if AT_STRINGS_BEG before looking at d - 1.  */
	  if (IS_A_LETTER (d) && (AT_STRINGS_BEG || !IS_A_LETTER (d - 1)))
	    break;
	  goto fail;

	case wordend:
          /* Have to check if AT_STRINGS_BEG before looking at d - 1.  */
	  if (!AT_STRINGS_BEG && IS_A_LETTER (d - 1)
              && (!IS_A_LETTER (d) || AT_STRINGS_END))
	    break;
	  goto fail;

#ifdef emacs
#ifdef emacs19
  	case before_dot:
 	  if (PTR_CHAR_POS (d) >= point)
  	    goto fail;
  	  break;
  
  	case at_dot:
 	  if (PTR_CHAR_POS (d) != point)
  	    goto fail;
  	  break;
  
  	case after_dot:
 	  if (PTR_CHAR_POS (d) <= point)
  	    goto fail;
  	  break;
#else /* not emacs19 */
        case before_dot:
	  if (((d - string2 <= (unsigned) size2)
	       ? d - bf_p2 : d - bf_p1)
	      <= point)
	    goto fail;
	  break;

	case at_dot:
	  if (((d - string2 <= (unsigned) size2)
	       ? d - bf_p2 : d - bf_p1)
	      == point)
	    goto fail;
	  break;

	case after_dot:
	  if (((d - string2 <= (unsigned) size2)
	       ? d - bf_p2 : d - bf_p1)
	      >= point)
	    goto fail;
	  break;
#endif /* not emacs19 */

	case wordchar:
	  mcnt = (int) Sword;
	  goto matchsyntax;

	case syntaxspec:
	  mcnt = *p++;
	matchsyntax:
	  PREFETCH;
	  if (SYNTAX (*d++) != (enum syntaxcode) mcnt) goto fail;
          SET_REGS_MATCHED;
	  break;
	  
	case notwordchar:
	  mcnt = (int) Sword;
	  goto matchnotsyntax;

	case notsyntaxspec:
	  mcnt = *p++;
	matchnotsyntax:
	  PREFETCH;
	  if (SYNTAX (*d++) == (enum syntaxcode) mcnt) goto fail;
	  SET_REGS_MATCHED;
          break;

#else /* not emacs */

	case wordchar:
	  PREFETCH;
          if (!IS_A_LETTER (d))
            goto fail;
	  SET_REGS_MATCHED;
	  break;
	  
	case notwordchar:
	  PREFETCH;
	  if (IS_A_LETTER (d))
            goto fail;
          SET_REGS_MATCHED;
	  break;

#endif /* not emacs */

	case begbuf:
          if (AT_STRINGS_BEG)
            break;
          goto fail;

        case endbuf:
	  if (AT_STRINGS_END)
	    break;
	  goto fail;

	case exactn:
	  /* Match the next few pattern characters exactly.
	     mcnt is how many characters to match.  */
	  mcnt = *p++;
	  /* This is written out as an if-else so we don't waste time
             testing `translate' inside the loop.  */
          if (translate)
	    {
	      do
		{
		  PREFETCH;
		  if (translate[*d++] != *p++) goto fail;
		}
	      while (--mcnt);
	    }
	  else
	    {
	      do
		{
		  PREFETCH;
		  if (*d++ != *p++) goto fail;
		}
	      while (--mcnt);
	    }
	  SET_REGS_MATCHED;
          break;
	}
      continue;  /* Successfully executed one pattern command; keep going.  */

    /* Jump here if any matching operation fails. */
    fail:
      if (!FAILURE_STACK_EMPTY)
	/* A restart point is known.  Restart there and pop it. */
	{
          short highest_used_reg, this_reg;
          boolean is_a_jump_n = false;

          /* If this failure point is from a dummy_failure_point,
             just skip it.  */
             
          if (!failure_stack.stack[failure_stack.avail - 2])
            {
	      pop_failure_point (&failure_stack);
              goto fail;
            }

          /* Among other things, undo the last failure point push.  */

          d = failure_stack.stack[--failure_stack.avail];
          p = failure_stack.stack[--failure_stack.avail];


          /* If failed to a backwards jump that's part of a repetition
             loop, need to pop this failure point and use the next one.  */

          switch ((enum regexpcode) *p)
            {
            case no_pop_jump_n:
	      is_a_jump_n = true;
            case maybe_pop_jump:
            case pop_failure_jump:
            case no_pop_jump:
              p1 = p + 1;
              extract_number_and_incr (&mcnt, &p1);
	      p1 += mcnt;	

              if ((is_a_jump_n && *p1 == succeed_n)
              	  || (!is_a_jump_n && *p1 == on_failure_jump))
	        {
		  /* Put p and d back on the stack again...  */
                  failure_stack.avail += 2;	       
                  
                  /* ...and pop the whole failure point.  */
  		  pop_failure_point (&failure_stack);
		  goto fail;
                }
              break;
            }

          if (d >= string1 && d <= end1)
	    dend = end_match_1;

          /* Restore register info.  */
          highest_used_reg 
            = (short) failure_stack.stack[--failure_stack.avail];
          
          /* Make the ones that weren't saved -1 or 0 again.  */
          for (this_reg = num_internal_regs - 1; this_reg > highest_used_reg; 
	       this_reg--)
            {
              regend[this_reg] = (unsigned char *) -1;
              regstart[this_reg] = (unsigned char *) -1;
              IS_ACTIVE (reg_info[this_reg]) = 0;
              MATCHED_SOMETHING (reg_info[this_reg]) = 0;
            }
          
          /* And restore the rest from the stack.  */
          for ( ; this_reg > 0; this_reg--)
            {
              reg_info[this_reg] = *(struct register_info *) 
                failure_stack.stack[--failure_stack.avail];

              regend[this_reg] 
                = failure_stack.stack[--failure_stack.avail];
  
              regstart[this_reg] 
                = failure_stack.stack[--failure_stack.avail];
            }
        }
      else
        break;   /* Matching at this starting point really fails.  */
    } /* while (1) */

  really_fail:
  if (best_regs_set)
    goto restore_best_regs;

#ifdef REGEX_MALLOC
  FREE_VARIABLES;
#endif
  return -1;         			/* Failure to match.  */
}




/* Subroutine definitions for re_match_2.  */



/* Failure stack stuff.  */

/* Pops what PUSH_FAILURE_STACK pushes.  */

static void 
pop_failure_point(failure_stack_ptr)
  failure_stack_type *failure_stack_ptr;
{									
  int temp;								

  if (FAILURE_STACK_PTR_EMPTY)
    {									
      printf ("Tried to pop empty failure point in re_match_2.\n");	
      exit (1);							
    }

  /* Remove failure points and point to how many regs pushed.  */
  else
    {
      if (failure_stack_ptr->avail < 3)
        {
          printf ("Aren't enough items to pop on re_match_2 failure stack: \
there's only %d on it.\n", failure_stack_ptr->avail);
          exit (1);
        }
      failure_stack_ptr->avail -= 3;
      temp = (int) failure_stack_ptr->stack[failure_stack_ptr->avail];
      temp *= NUM_REG_ITEMS;		/* How much to take off the stack.  */
      
      if (failure_stack_ptr->avail < temp)
        {
          printf ("Can't pop %d items off re_match_2 failure stack: \
there's only %d on it.\n", temp, failure_stack_ptr->avail);
          exit (1);
        }
      failure_stack_ptr->avail -= temp;	/* Remove the register info.  */
    }
}


/* Other things.  */

static boolean common_op_can_match_nothing ();
static boolean alternative_can_match_nothing ();


/* We are given P pointing to a register number after a start_memory.
   
   Return true if the pattern up to the corresponding stop_memory can
   match the empty string, and false otherwise.
   
   If we find the matching stop_memory, sets P to point to one past its number.
   Otherwise, sets P to an undefined byte less than or equal to END.

   We don't handle duplicates properly (yet).  */

static boolean
group_can_match_nothing (p, end, reg_info)
  unsigned char **p, *end;
  struct register_info *reg_info;
{
  int mcnt;
  unsigned char *p1 = *p + 1;  	/* Point to after this register number.  */
  
  while (p1 < end)
    {
      /* Skip over opcodes that can match nothing, and return true or
	 false, as appropriate, when we get to one that can't, or to the
         matching stop_memory.  */
      
      switch ((enum regexpcode) *p1)
        {
        /* Could be either a loop or a series of alternatives.  */
        case on_failure_jump:
          p1++;
          extract_number_and_incr (&mcnt, &p1);
          
          /* If the next operation is not a jump backwards in the
	     pattern.  */

	  if (mcnt >= 0)
	    {
              /* Go through the on_failure_jumps of the alternatives,
                 seeing if any of the alternatives cannot match nothing.
                 The last alternative starts with only a no_pop_jump,
                 whereas the rest start with on_failure_jump and end
                 with a no_pop_jump, e.g., here is the pattern for `a|b|c':

                 /on_failure_jump/0/6/exactn/1/a/jump_past_next_alt/0/6
                 /on_failure_jump/0/6/exactn/1/b/jump_past_next_alt/0/3
                 /exactn/1/c						

                 So, we have to first go through the first (n-1)
                 alternatives and then deal with the last one separately.  */


              /* Deal with the first (n-1) alternatives, which start
                 with an on_failure_jump (see above) that jumps to right
                 past a jump_past_next_alt.  */

              while ((enum regexpcode) p1[mcnt-3] == jump_past_next_alt)
                {
                  /* MCNT holds how many bytes long the alternative
                     is, including the ending `jump_past_next_alt' and its number.  */

                  if (!alternative_can_match_nothing (p1, p1 + mcnt - 3, 
				                      reg_info))
                    return false;

                  /* Move to right after this alternative, including the
		     jump_past_next_alt.  */
                     
                  p1 += mcnt;	

                  /* Break if it's the beginning of an n-th alternative
                     that doesn't begin with an on_failure_jump.  */
      
                  if ((enum regexpcode) *p1 != on_failure_jump)
                    break;
		
		  /* Still have to check that it's not an n-th
		     alternative that starts with an on_failure_jump.  */
		  p1++;
                  extract_number_and_incr (&mcnt, &p1);
                  if ((enum regexpcode) p1[mcnt-3] != jump_past_next_alt)
                    {
		      /* Get to the beginning of the n-th alternative.  */
                      p1 -= 3;
                      break;
                    }
                }

              /* Deal with the last alternative: go back and get number
                 of the jump_past_next_alt just before it.  MCNT contains how
                 many bytes long the alternative is.  */

              mcnt = extract_number (p1 - 2);

              if (!alternative_can_match_nothing (p1, p1 + mcnt, reg_info))
                return false;

              p1 += mcnt;	/* Get past the n-th alternative.  */

            } /*  if mcnt > 0  */

          break;
          
        case stop_memory:
          if (p1[1] == **p)
            {
              *p = p1 + 2;
              return true;
            }
          else
	    {
              printf ("Error: encountered an unmatched (%d) stop_memory in \
group_can_match_nothing.\n", **p);
	      exit (1);
	    }          
	  break;
	
          default: 
            if (!common_op_can_match_nothing (&p1, end, reg_info))
	      return false;
        }
    }  /* While p1 < end.  */

  return false;
}


/* Similar to group_can_match_nothing, but doesn't deal with alternatives:
   It expects P to be the first byte of a single alternative and END one
   byte past the last. The alternative can contain groups.  */
   

static boolean
alternative_can_match_nothing (p, end, reg_info)
  unsigned char *p, *end;
  struct register_info *reg_info;
{
  int mcnt;
  unsigned char *p1 = p;
  
  while (p1 < end)
    {
      /* Skip over opcodes that can match nothing, and break when we get 
         to one that can't.  */
      
      switch ((enum regexpcode) *p1)
        {
	/* It's a loop.  */
        case on_failure_jump:
          p1++;
          extract_number_and_incr (&mcnt, &p1);
          p1 += mcnt;
          break;
          
	default: 
          if (!common_op_can_match_nothing (&p1, end, reg_info))
            return false;
        }
    }  /* While not at the end of the alternative.  */

  return true;
}


/* Deals with the ops common to group_can_match_nothing and
   alternative_can_match_nothing.  
   
   Sets P to one after the op and its arguments, if any.  */

static boolean
common_op_can_match_nothing (p, end, reg_info)
  unsigned char **p, *end;
  struct register_info *reg_info;
{
  int mcnt;
  unsigned char *p1 = *p;
  boolean ret;
  int reg_no;

  switch ((enum regexp1code) *p1++)
    {
    case no_op:
    case begline:
    case endline:
    case endline_in_repeat:
    case endline_before_newline:
      break;

    case start_memory:
      reg_no = *p1;
      ret = group_can_match_nothing (&p1, end, reg_info);
      
      /* Have to set this here in case we're checking a group which
         contains a group and a back reference to it.  */

      if (CAN_MATCH_NOTHING (reg_info[reg_no]) == -1)
         CAN_MATCH_NOTHING (reg_info[reg_no]) = ret;

      if (!ret)
        return false;
      break;
          
    /* If this is an optimized succeed_n for zero times, make the jump.  */
    case no_pop_jump:
      extract_number_and_incr (&mcnt, &p1);
      
      if (mcnt >= 0)
        p1 += mcnt;
      else
        return false;
      break;

    case succeed_n:
      /* Get to the number of times to succeed.  */
      p1 += 2;		
      extract_number_and_incr (&mcnt, &p1);

      if (mcnt == 0)
        {
          p1 -= 4;
          extract_number_and_incr (&mcnt, &p1);
          p1 += mcnt;
        }
      else
        return false;
      break;

    case duplicate: 
      if (!CAN_MATCH_NOTHING (reg_info[*p1]))
        return false;
      break;

    case set_number_at:
      p1 += 4;
    case before_dot:
    case at_dot:
    case after_dot:
    case begbuf:
    case endbuf:
    case wordbeg:
    case wordend:
    case wordbound:
    case notwordbound:
      break;

    default:
      /* All other opcodes mean we cannot match the empty string.  */
      return false;
  }

  *p = p1;
  return true;
}



/* Return zero if TRANSLATE[S1] and TRANSLATE[S2] are identical for LEN
   bytes; nonzero otherwise.  */
   
static int
bcmp_translate (s1, s2, len, translate)
     unsigned char *s1, *s2;
     register int len;
     unsigned char *translate;
{
  register unsigned char *p1 = s1, *p2 = s2;
  while (len)
    {
      if (translate [*p1++] != translate [*p2++]) return 1;
      len--;
    }
  return 0;
}




/* Entry points compatible with 4.2 BSD regex library.  We don't define
   them if this is an Emacs or POSIX compilation.  */

#if !defined(GAWK) && !defined (emacs) && !defined (_POSIX_SOURCE)

static struct re_pattern_buffer re_comp_buf;

char *
re_comp (s)
  const char *s;
{
  char *return_value;
  
  if (!s)
    {
      if (!re_comp_buf.buffer)
	return "No previous regular expression";
      return 0;
    }

  if (!re_comp_buf.buffer)
    {
      re_comp_buf.buffer = (char *) malloc (200);

      if (re_comp_buf.buffer == NULL)
        return "Memory exhausted";

      re_comp_buf.allocated = 200;

      re_comp_buf.fastmap = (char *) malloc (1 << BYTEWIDTH);

      if (re_comp_buf.fastmap == NULL)
	return "Memory exhausted";
    }
  return regex_compile (s, strlen (s), obscure_syntax, &re_comp_buf);
}

int
re_exec (s)
  const char *s;
{
  const int len = strlen (s);
  return 0 <= re_search (&re_comp_buf, s, len, 0, len, 
		         (struct re_registers *) 0);
}

#endif /* not emacs and not _POSIX_SOURCE */



/* Entry points compatible with POSIX regex library.  Only define these
   when this is a POSIX compilation (and it's not Emacs).  */

#if !defined(emacs) && !defined(GAWK)

/* regcomp takes a regular-expression string and converts it into a
   buffer full of byte commands for matching.

   PREG      is a regex_t * whose pertinent fields are mentioned in below:
                
             It has a char * field called BUFFER which points to the
             space where this routine will put the compiled pattern; the
             user can either allocate this using malloc (whereupon they
             should set the long field ALLOCATED to the number of bytes
             malloced) or set ALLOCATED to 0 and let the routine
             allocate it.  The routine may use realloc to enlarge the
             buffer space.
             
             If the user wants to translate all ordinary elements in the
             compiled pattern, they should set the char * field
             TRANSLATE to a translate table (and not set the REG_ICASE
             bit of CFLAGS, which would override this translate table
             with one that ignores case); otherwise, they should set
             TRANSLATE to 0.
             
             The routine sets the int field SYNTAX to RE_SYNTAX_POSIX_EXTENDED
             if the REG_EXTENDED bit in CFLAGS is set; otherwise, it sets it 
             to RE_SYNTAX_POSIX_BASIC.
             
             It returns in the long field USED how many bytes long the
             compiled pattern is.
             
             It returns 0 in the char field FASTMAP_ACCURATE, on
             the assumption that the user usually doesn't compile the
             same pattern twice and that consequently any fastmap in the
             pattern buffer is inaccurate.
                   
	     In the size_t field RE_NSUB, it returns the number of
             subexpressions it found in PATTERN.

   PATTERN   is the address of the pattern string.

   CFLAGS    is a series of bits ORed together which affect compilation.
             If the bit REG_EXTENDED is set, regcomp compiles the
             pattern as an extended regular expression, otherwise it
             compiles it as a basic one.  If the bit REG_NEWLINE is set,
             then dot and nonmatching lists won't match a newline, but
             pattern anchors will match at them.  If the bit REG_ICASE
             is set, then it considers upper- and lowercase versions of
             letters to be equal when matching.  If the bit REG_NOSUB is
             set, then when PREG is passed to regexec, that routine will
             only report success or failure.


   It returns 0 if it succeeds, nonzero if it doesn't.  (See regex.h for
   POSIX return codes and their meanings.)  */


int
regcomp (preg, pattern, cflags)
  regex_t *preg;
  const char *pattern; 
  int cflags;
{
  char *return_value;
  
  int syntax = cflags & REG_EXTENDED
	       ? RE_SYNTAX_POSIX_EXTENDED
	       : RE_SYNTAX_POSIX_BASIC;

  if (cflags & REG_NEWLINE)
    {
      syntax &= ~RE_DOT_NEWLINE;
      syntax |= RE_HAT_LISTS_NOT_NEWLINE;
      syntax &= ~RE_NO_ANCHOR_AT_NEWLINE;
    }

  if (cflags & REG_ICASE)
    {
      unsigned i;
      
      preg->translate = (char *) malloc (CHAR_SET_SIZE);

      if (preg->translate == NULL)
        return REG_ESPACE;

      /* Map any uppercase characters into corresponding lowercase ones.  */
      for (i = 0; i < CHAR_SET_SIZE; i++)
        preg->translate[i] = isupper (i) ? tolower (i) : i;
    }
  else
    preg->translate = 0;

  preg->no_sub = cflags & REG_NOSUB;

  return_value = regex_compile (pattern, strlen (pattern), syntax, preg);


  if (return_value == 0)
    return 0;
  else if (strcmp (return_value, "Invalid regular expression") == 0)
    return REG_BADPAT;
  else if (strcmp (return_value, "Invalid character class name") == 0)
    return REG_ECTYPE;
  else if (strcmp (return_value, "Trailing backslash") == 0)
    return REG_EESCAPE;
  else if (strcmp (return_value, "Invalid back reference") == 0)
    return REG_ESUBREG;
  else if (strcmp (return_value, "Unmatched [ or [^") == 0)
    return REG_EBRACK;
  else if (strcmp (return_value, "Unmatched ( or \\(") == 0
           || strcmp (return_value, "Unmatched ) or \\)") == 0)
    return REG_EPAREN;
  else if (strcmp (return_value, "Unmatched \\{") == 0)
    return REG_EBRACE;
  else if (strcmp (return_value, "Invalid content of \\{\\}") == 0)
    return REG_BADBR;
  else if (strcmp (return_value, "Invalid range end") == 0)
    return REG_ERANGE;
  else if (strcmp (return_value, "Memory exhausted") == 0)
    return REG_ESPACE;
  else if (strcmp (return_value, "Invalid preceding regular expression") == 0
           || strcmp (return_value, 
		      "Missing preceding regular expression") == 0)
    return REG_BADRPT;

  /* Codes added by GNU.  */
  
  else if (strcmp (return_value, "Premature end of regular expression") == 0)
    return REG_EEND;
  else if (strcmp (return_value, "Regular expression too big") == 0)
    return REG_ESIZE;
else
    return REG_BADPAT;
}


/* regexex matches a buffer full of byte commands for matching (gotten
   from compiling a regular expression) and matches it against a string.
   
   PREG      is a regex_t * whose pertinent fields are mentioned below:
                
             It has a char * field called BUFFER which points to 
             the byte commands which make up the compiled pattern.
             
             Its char * field TRANSLATE, if not 0, translates all
             ordinary elements in the compiled pattern.

             Its int field SYNTAX is the syntax with which the pattern
             was compiled and hence should be matched with.
             
             The long field USED is how many bytes long the compiled
             pattern is.
             
             Its size_t field RE_NSUB contains how many subexpressions
             the pattern has.  (This may be useful for choosing a value
             for NMATCH).
             
	     If its unsigned NO_SUB bit is set, then regexec will not
             return anything in PMATCH, but only report whether or not
             BUFFER matched STRING.
             
	     Regardless of how its unsigned RETURN_DEFAULT_NUM_REGS bit
             is set, regexec only returns in PMATCH information about
             the whole pattern and NMATCH - 1 of its subexpressions.
             
   STRING    is the address of the string to be matched.

   NMATCH    is how many elements of PMATCH regex should fill.

   PMATCH    is an array of struct regex_t's.  If PREG's NO_SUB field
             isn't set, then regexec records in PMATCH[i], for i = 1 to
             PMATCH - 1, which substring of STRING matched the i-th
             subexpression of the regular expression compiled in BUFFER;
             it records in PMATCH[0] that information about all of
             STRING.  See the comment for `typedef struct...regmatch_t'
             in regex.h for more details.

             The caller must allocate PMATCH to have at least NMATCH
             elements. 
             
   EFLAGS    is two bits OR-ed together which affect execution.  If the
             bit REG_NOTBOL is set, then STRING's first character is not
             the beginning of a line; that means any beginning-of-line
             byte command in BUFFER won't match that first character.
             If the bit REG_NOTEOL is set, then a similar things holds
             for STRING's last character: it isn't the end of a line and
             any end-of-line byte command in BUFFER won't match it.


   It returns 0 if it matches and REG_NOMATCH if it doesn't.  */

int
regexec (preg, string, nmatch, pmatch, eflags)
  const regex_t *preg;
  const char *string; 
  size_t nmatch; 
  regmatch_t pmatch[]; 
  int eflags;
{
  int return_value;
  unsigned this_op;
  struct re_registers regs;
  regex_t private_preg;
  
  private_preg = *preg;
  private_preg.not_bol = eflags & REG_NOTBOL;
  private_preg.not_eol = eflags & REG_NOTEOL;
    
  private_preg.return_default_num_regs = 0;
  
  if (!private_preg.no_sub && nmatch > 0)
    {
      regs.num_regs = nmatch;
      regs.start = malloc (nmatch * sizeof (int));
      regs.end = malloc (nmatch * sizeof (int));
    }
  else
    {
      regs.num_regs = 0;
      regs.start = NULL;
      regs.end = NULL;
    }

  return_value = re_match (&private_preg, string, strlen (string), 0, 
		           !private_preg.no_sub && nmatch > 0 ? &regs : 0);
  
  if (return_value == strlen (string))
    {
      if (!private_preg.no_sub && nmatch > 0)
        {
          unsigned this_reg;

          for (this_reg = 0; this_reg < nmatch; this_reg++)
            {
              pmatch[this_reg].rm_so = regs.start[this_reg];
              pmatch[this_reg].rm_eo = regs.end[this_reg];
            }
        }
    }    
  if (regs.start != NULL)
    free (regs.start);

  if (regs.end != NULL)
    free (regs.end);
  
  return return_value == strlen (string) ? 0 : REG_NOMATCH;
}


/* Puts the first BUFFER_SIZE - 1 characters in BUFFER (if BUFFER isn't null)
   and terminates it with a null.
   
   Returns one more than the size of MESSAGE.  */

static size_t
put_in_buffer (message, buffer, buffer_size)
  char *message;
  char *buffer;
  size_t buffer_size;
{
  unsigned this_char;
  
  if (buffer != NULL && buffer_size > 0)
    {
      strncpy (buffer, message, buffer_size - 1);
      buffer[buffer_size - 1] = 0;
    }

  return strlen (message) + 1;
}  


/* Returns a message corresponding to an error code, ERRCODE, returned
   from either regcomp or regexec.   */

size_t
re_gerror (errcode, preg, errbuf, errbuf_size)
  int errcode;
  const regex_t *preg;
  char *errbuf;
  size_t errbuf_size;
{
    switch (errcode)
    {
      case REG_NOERROR: 
        return put_in_buffer ("Regex message: no error.", errbuf, errbuf_size); 
        
      case REG_NOMATCH: 
        return put_in_buffer ("Regex error: regexec didn't find a match.",
			      errbuf, errbuf_size); 
      case REG_BADPAT: 
        return put_in_buffer ("Regex error: Invalid regular expression.", 
		              errbuf, errbuf_size); 
      case REG_ECOLLATE: 
        return put_in_buffer ("Regex error: (not implemented) Invalid \
collating character.", errbuf, errbuf_size); 
        
      case REG_ECTYPE: 
        return put_in_buffer ("Regex error: Invalid character class name.",
			      errbuf, errbuf_size); 
      case REG_EESCAPE: 
        return put_in_buffer ("Regex error: Trailing backslash.",
			      errbuf, errbuf_size); 
      case REG_ESUBREG: 
        return put_in_buffer("Regex error: Invalid back reference.", 
			      errbuf, errbuf_size); 
      case REG_EBRACK: 
        return put_in_buffer ("Regex error: Unmatched [ or [^.",
			      errbuf, errbuf_size); 
      case REG_EPAREN: 
        return put_in_buffer ("Regex error: Unmatched parenthesis.",
			      errbuf, errbuf_size); 
      case REG_EBRACE: 
        return put_in_buffer ("Regex error: Unmatched \\{.",
			      errbuf, errbuf_size); 
      case REG_BADBR: 
        return put_in_buffer ("Regex error: Invalid content of \\{\\}.",
        			      errbuf, errbuf_size); 
      case REG_ERANGE: 
        return put_in_buffer ("Regex error: Invalid range end.",
			      errbuf, errbuf_size); 
      case REG_ESPACE: 
        return put_in_buffer ("Regex error: Ran out of memory.", 
			      errbuf, errbuf_size); 
      case REG_BADRPT: 
        return put_in_buffer ("Regex error: Preceding regular expression \
either missing or not simple.", errbuf, errbuf_size); 

      case REG_EEND: 
        return put_in_buffer ("Regex error: Regular expression ended \
prematurely.", errbuf, errbuf_size); 

      case REG_ESIZE:
        return put_in_buffer ("Regex error: Excessively large regular \
expression.", errbuf, errbuf_size); 
    }
}


void
re_gfree (preg)
  regex_t *preg;
{
  if (preg->buffer != NULL)
    free (preg->buffer);
  preg->buffer = NULL;
  
  preg->allocated = 0;
  preg->used = 0;

  if (preg->fastmap != NULL)
    free (preg->fastmap);
  preg->fastmap = NULL;

  preg->fastmap_accurate = 0;

  if (preg->translate != NULL)
    free (preg->translate);
  preg->translate = NULL;
}

#endif /* not emacs  */




#ifdef test

#include <stdio.h>

/* Indexed by a character, gives the upper case equivalent of the
   character.  */

char upcase[0400] = 
  { 000, 001, 002, 003, 004, 005, 006, 007,
    010, 011, 012, 013, 014, 015, 016, 017,
    020, 021, 022, 023, 024, 025, 026, 027,
    030, 031, 032, 033, 034, 035, 036, 037,
    040, 041, 042, 043, 044, 045, 046, 047,
    050, 051, 052, 053, 054, 055, 056, 057,
    060, 061, 062, 063, 064, 065, 066, 067,
    070, 071, 072, 073, 074, 075, 076, 077,
    0100, 0101, 0102, 0103, 0104, 0105, 0106, 0107,
    0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117,
    0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127,
    0130, 0131, 0132, 0133, 0134, 0135, 0136, 0137,
    0140, 0101, 0102, 0103, 0104, 0105, 0106, 0107,
    0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117,
    0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127,
    0130, 0131, 0132, 0173, 0174, 0175, 0176, 0177,
    0200, 0201, 0202, 0203, 0204, 0205, 0206, 0207,
    0210, 0211, 0212, 0213, 0214, 0215, 0216, 0217,
    0220, 0221, 0222, 0223, 0224, 0225, 0226, 0227,
    0230, 0231, 0232, 0233, 0234, 0235, 0236, 0237,
    0240, 0241, 0242, 0243, 0244, 0245, 0246, 0247,
    0250, 0251, 0252, 0253, 0254, 0255, 0256, 0257,
    0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,
    0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,
    0300, 0301, 0302, 0303, 0304, 0305, 0306, 0307,
    0310, 0311, 0312, 0313, 0314, 0315, 0316, 0317,
    0320, 0321, 0322, 0323, 0324, 0325, 0326, 0327,
    0330, 0331, 0332, 0333, 0334, 0335, 0336, 0337,
    0340, 0341, 0342, 0343, 0344, 0345, 0346, 0347,
    0350, 0351, 0352, 0353, 0354, 0355, 0356, 0357,
    0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,
    0370, 0371, 0372, 0373, 0374, 0375, 0376, 0377
  };

#ifdef canned

#include "tests.h"

typedef enum {extended_test, basic_test, other_test, interface_test} test_type;

/* Use this to run the tests we've thought of.  */

void
main ()
{
  test_type t = interface_test;
  
  if (t == basic_test)
    test_posix_basic ();
  else if (t == extended_test)
    test_posix_extended (); 
  else if (t == other_test)
    test_others (); 
  else if (t == interface_test)
    test_posix_c_interface (); 

  exit (0);
}

#else /* not canned */

/* Use this to run interactive tests.  */

void
main (argc, argv)
     int argc;
     char **argv;
{
  char pat[80];
  struct re_pattern_buffer buf;
  int i;
  char c;
  char fastmap[(1 << BYTEWIDTH)];

  /* Allow a command argument to specify the style of syntax.  */
  if (argc > 1)
    re_set_syntax (atoi (argv[1]));

  buf.allocated = 40;
  buf.buffer = (char *) malloc (buf.allocated);
  buf.fastmap = fastmap;
  buf.translate = upcase;

  while (1)
    {
      gets (pat);

      if (*pat)
	{
          re_compile_pattern (pat, strlen(pat), &buf);

	  for (i = 0; i < buf.used; i++)
	    printchar (buf.buffer[i]);

	  putchar ('\n');

	  printf ("%d allocated, %d used.\n", buf.allocated, buf.used);

	  re_compile_fastmap (&buf);
	  printf ("Allowed by fastmap: ");
	  for (i = 0; i < (1 << BYTEWIDTH); i++)
	    if (fastmap[i]) printchar (i);
	  putchar ('\n');
	}

      gets (pat);	/* Now read the string to match against */

      i = re_match (&buf, pat, strlen (pat), 0, 0);
      printf ("Match value %d.\n", i);
    }
}

#endif


#if 0
print_buf (bufp)
     struct re_pattern_buffer *bufp;
{
  int i;

  printf ("buf is :\n----------------\n");
  for (i = 0; i < bufp->used; i++)
    printchar (bufp->buffer[i]);
  
  printf ("\n%d allocated, %d used.\n", bufp->allocated, bufp->used);
  
  printf ("Allowed by fastmap: ");
  for (i = 0; i < (1 << BYTEWIDTH); i++)
    if (bufp->fastmap[i])
      printchar (i);
  printf ("\nAllowed by translate: ");
  if (bufp->translate)
    for (i = 0; i < (1 << BYTEWIDTH); i++)
      if (bufp->translate[i])
	printchar (i);
  printf ("\nfastmap is%s accurate\n", bufp->fastmap_accurate ? "" : "n't");
  printf ("can %s be null\n----------", bufp->can_be_null ? "" : "not");
}
#endif /* 0 */


printchar (c)
     char c;
{
  if (c < 040 || c >= 0177)
    {
      putchar ('\\');
      putchar (((c >> 6) & 3) + '0');
      putchar (((c >> 3) & 7) + '0');
      putchar ((c & 7) + '0');
    }
  else
    putchar (c);
}

error (string)
     char *string;
{
  puts (string);
  exit (1);
}
#endif /* test */



/*
Local variables:
make-backup-files: t
version-control: t
trim-versions-without-asking: nil
End:
*/
