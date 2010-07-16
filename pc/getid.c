#ifdef _MSC_VER

#ifdef OS2
# define INCL_DOSPROCESS
# include <os2.h>
# if _MSC_VER == 510
#  define DosGetPID DosGetPid
# endif 
#else
# include <process.h>
#endif

#ifdef OS2
int getpid(void)
{
	PIDINFO	PidInfo;

	DosGetPID(&PidInfo);
	return(PidInfo.pid);
}
#endif

int getppid(void)
{
#ifdef OS2
	PIDINFO	PidInfo;

	DosGetPID(&PidInfo);
	return(PidInfo.pidParent);
#else
	return(0);
#endif
}

unsigned int getuid (void)
{
  return (0);                   /* root! */
}


unsigned int geteuid (void)
{
  return (0);
}


unsigned int getgid (void)
{
  return (0);
}


unsigned int getegid (void)
{
  return (0);
}


char *getlogin (void)
{
  return ("root");
}

#endif

int getpgrp(void)
{
  return (0);
}
