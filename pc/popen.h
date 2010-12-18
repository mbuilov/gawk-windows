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
#endif	/* !__DJGPP__ */
