/*
** popen.h -- prototypes for pipe functions
*/
#if !defined (__DJGPP__)
# if defined (popen)
#  undef popen
#  undef pclose
# endif
# define popen(c, m)	os_popen(c, m)
# define pclose(f)	os_pclose(f)
  extern FILE *os_popen( const char *, const char * );
  extern int  os_pclose( FILE * );
# if defined __MINGW32__ || defined _MSC_VER
#  define system(c)	os_system(c)
   extern int os_system( const char * );
#  define SIGKILL	9
   extern int kill( int, int );
   extern char *quote_cmd( const char * );
# endif	 /* __MINGW32__ || _MSC_VER */
#endif	/* !__DJGPP__ */
