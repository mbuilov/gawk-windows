/*

** popen.h -- prototypes for pipe functions

*/

#if !defined(FILE)

#include <stdio.h>

#endif



extern FILE *os_popen( char *, char * );

extern int  os_pclose( FILE * );

#define popen os_popen
#define pclose os_close
