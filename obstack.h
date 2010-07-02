/* obstack.h - object stack macros
   Copyright (c) 1986 Free Software Foundation, Inc.

Summary:

All the apparent functions defined here are macros. The idea
is that you would use these pre-tested macros to solve a
very specific set of problems, and they would run fast.
Caution: no side-effects in arguments please!! They may be
evaluated MANY times!!

These macros operate a stack of objects.  Each object starts life
small, and may grow to maturity.  (Consider building a word syllable
by syllable.)  An object can move while it is growing.  Once it has
been "finished" it never changes address again.  So the "top of the
stack" is typically an immature growing object, while the rest of the
stack is of mature, fixed size and fixed address objects.

These routines grab large chunks of memory, using a function you
supply, called `obstack_chunk_alloc'.  On occasion, they free chunks,
by calling `obstack_chunk_free'.  You must define them and declare
them before using any obstack macros.

Each independent stack is represented by a `struct obstack'.
Each of the obstack macros expects a pointer to such a structure
as the first argument.

One motivation for this package is the problem of growing char strings
in symbol tables.  Unless you are "facist pig with a read-only mind"
[Gosper's immortal quote from HAKMEM item 154, out of context] you
would not like to put any arbitrary upper limit on the length of your
symbols.

In practice this often means you will build many short symbols and a
few long symbols.  At the time you are reading a symbol you don't know
how long it is.  One traditional method is to read a symbol into a
buffer, realloc()ating the buffer every time you try to read a symbol
that is longer than the buffer.  This is beaut, but you still will
want to copy the symbol from the buffer to a more permanent
symbol-table entry say about half the time.

With obstacks, you can work differently.  Use one obstack for all symbol
names.  As you read a symbol, grow the name in the obstack gradually.
When the name is complete, finalize it.  Then, if the symbol exists already,
free the newly read name.

The way we do this is to take a large chunk, allocating memory from
low addresses.  When you want to build a aymbol in the chunk you just
add chars above the current "high water mark" in the chunk.  When you
have finished adding chars, because you got to the end of the symbol,
you know how long the chars are, and you can create a new object.
Mostly the chars will not burst over the highest address of the chunk,
because you would typically expect a chunk to be (say) 100 times as
long as an average object.

In case that isn't clear, when we have enough chars to make up
the object, THEY ARE ALREADY CONTIGUOUS IN THE CHUNK (guaranteed)
so we just point to it where it lies.  No moving of chars is
needed and this is the second win: potentially long strings need
never be explicitly shuffled. Once an object is formed, it does not
change its address during its lifetime.

When the chars burst over a chunk boundary, we allocate a larger
chunk, and then copy the partly formed object from the end of the old
chunk to the beggining of the new larger chunk.  We then carry on
accreting characters to the end of the object as we normaly would.

A special macro is provided to add a single char at a time to a
growing object.  This allows the use of register variables, which
break the ordinary 'growth' macro.

Summary:
	We allocate large chunks.
	We carve out one object at a time from the current chunk.
	Once carved, an object never moves.
	We are free to append data of any size to the currently
	  growing object.
	Exactly one object is growing in an obstack at any one time.
	You can run one obstack per control block.
	You may have as many control blocks as you dare.
	Because of the way we do it, you can `unwind' a obstack
	  back to a previous state. (You may remove objects much
	  as you would with a stack.)
*/

#ifndef obstackH
#define obstackH
				/* these #defines keep it brief */
#define _Ll struct obstack_chunk
#define _LL (8)			/* _L length in chars */

struct obstack_chunk		/* Lives at front of each chunk. */
{
  char  *obstack_l_limit;	/* 1 past end of this chunk */
  _Ll	*obstack_l_prev;	/* address of prior chunk or NULL */
  char	obstack_l_0[4];		/* objects begin here */
};

#if 0
This function, called like malloc but not returning on failure,
must return a chunk of the size given to it as argument,
aligned on a boundary of 2**OBSTACK_LOG_DEFAULT_ALIGNMENT bytes.

struct obstack_chunk * obstack_chunk_alloc();
#endif /* 0 */

struct obstack		/* control current object in current chunk */
{
  long	chunk_size;		/* preferred size to allocate chunks in */
  _Ll*	chunk;			/* address of current struct obstack_chunk */
  char	*object_base;		/* address of object we are building */
  char	*next_free;		/* where to add next char to current object */
  char	*chunk_limit;		/* address of char after current chunk */
  int	temp;			/* Temporary for some macros.  */
  int   alignment_mask;		/* Mask of alignment for each object. */
};

/* Pointer to beginning of object being allocated or to be allocated next.
   Note that this might not be the final address of the object
   because a new chunk might be needed to hold the final size.  */

#define obstack_base(h) ((h)->object_base)

/* Pointer to next byte not yet allocated in current chunk.  */

#define obstack_next_free(h)	((h)->next_free)

/* Size of object currently growing */

#define obstack_object_size(h)  ((h)->next_free - (h)->object_base)

/* Mask specifying low bits that should be clear in address of an object.  */

#define obstack_alignment_mask(h) ((h)->alignment_mask)

#define obstack_init(h) obstack_begin (h, 4096 - 4 - _LL)

#define obstack_begin(h,try_length)					\
((h)->chunk_size = (try_length) + (_LL),				\
 (h)->alignment_mask = ((1 << 2) - 1),					\
 _obstack_begin ((h), obstack_chunk_alloc))

#define obstack_grow(h,where,length)					\
( (h)->temp = (length),							\
  (((h)->next_free + (h)->temp > (h)->chunk_limit)			\
   ? _obstack_newchunk ((h), obstack_chunk_alloc, (h)->temp) : 0),	\
  bcopy (where, (h)->next_free, (h)->temp),				\
  (h)->next_free += (h)->temp)

#define obstack_grow0(h,where,length)					\
( (h)->temp = (length),							\
  (((h)->next_free + (h)->temp + 1 > (h)->chunk_limit)			\
   ? _obstack_newchunk ((h), obstack_chunk_alloc, (h)->temp + 1) : 0),	\
  bcopy (where, (h)->next_free, (h)->temp),				\
  (h)->next_free += (h)->temp,						\
  *((h)->next_free)++ = 0)

#define obstack_1grow(h,datum)						\
( (((h)->next_free + 1 > (h)->chunk_limit)				\
   ? _obstack_newchunk ((h), obstack_chunk_alloc, 1) : 0),		\
  *((h)->next_free)++ = (datum))

#define obstack_blank(h,length)						\
( (h)->temp = (length),							\
  (((h)->next_free + (h)->temp > (h)->chunk_limit)			\
   ? _obstack_newchunk ((h), obstack_chunk_alloc, (h)->temp) : 0),	\
  (h)->next_free += (h)->temp)

#define obstack_alloc(h,length)						\
 (obstack_blank ((h), (length)), obstack_finish (h))

#define obstack_copy(h,where,length)					\
 (obstack_grow ((h), (where), (length)), obstack_finish (h))

#define obstack_copy0(h,where,length)					\
 (obstack_grow0 ((h), (where), (length)), obstack_finish (h))

#define obstack_room(h) ((long unsigned int)				\
 ((h)->chunk_limit - (h)->next_free))

#define obstack_1grow_fast(h,achar) (*((h)->next_free)++ = achar)

#define obstack_blank_fast(h,n) ((h)->next_free += (n))

#define obstack_finish(h)  						\
 ((h)->temp = (int) (h)->object_base,					\
  (h)->next_free							\
    = (char*)((int)((h)->next_free+(h)->alignment_mask)			\
	      & ~ ((h)->alignment_mask)),				\
  (((h)->next_free - (char *)(h)->chunk					\
    > (h)->chunk_limit - (char *)(h)->chunk)				\
   ? (h)->next_free = (h)->chunk_limit : 0),				\
  (h)->object_base = (h)->next_free,					\
  (char *) (h)->temp)

#define obstack_free(h,obj)						\
(((h)->temp = (char *)(obj) - (char *) (h)->chunk),			\
 (((h)->temp >= 0 && (h)->temp < (h)->chunk_limit - (char *) (h)->chunk)\
  ? (int) ((h)->next_free = (h)->object_base				\
	   = (h)->temp + (char *) (h)->chunk)				\
  : (int) _obstack_free ((h), obstack_chunk_free,			\
			 (h)->temp + (char *) (h)->chunk)))

#endif				/* #ifndef obstackH */
