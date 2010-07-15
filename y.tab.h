
typedef union  {
	long lval;
	AWKNUM fval;
	NODE *nodeval;
	NODETYPE nodetypeval;
	char *sval;
	NODE *(*ptrval)();
} YYSTYPE;
extern YYSTYPE yylval;
# define FUNC_CALL 257
# define NAME 258
# define REGEXP 259
# define ERROR 260
# define NUMBER 261
# define YSTRING 262
# define RELOP 263
# define APPEND_OP 264
# define ASSIGNOP 265
# define MATCHOP 266
# define NEWLINE 267
# define CONCAT_OP 268
# define LEX_BEGIN 269
# define LEX_END 270
# define LEX_IF 271
# define LEX_ELSE 272
# define LEX_RETURN 273
# define LEX_DELETE 274
# define LEX_WHILE 275
# define LEX_DO 276
# define LEX_FOR 277
# define LEX_BREAK 278
# define LEX_CONTINUE 279
# define LEX_PRINT 280
# define LEX_PRINTF 281
# define LEX_NEXT 282
# define LEX_EXIT 283
# define LEX_FUNCTION 284
# define LEX_GETLINE 285
# define LEX_IN 286
# define LEX_AND 287
# define LEX_OR 288
# define INCREMENT 289
# define DECREMENT 290
# define LEX_BUILTIN 291
# define LEX_LENGTH 292
# define UNARY 293
