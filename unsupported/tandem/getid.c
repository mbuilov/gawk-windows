#if defined _MSC_VER || defined TANDEM

#ifndef TANDEM
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
        PIDINFO PidInfo;

        DosGetPID(&PidInfo);
        return(PidInfo.pid);
}
#endif

int getppid(void)
{
#ifdef OS2
        PIDINFO PidInfo;

        DosGetPID(&PidInfo);
        return(PidInfo.pidParent);
#else
        return(0);
#endif
}

#endif  /* TANDEM */
#ifdef TANDEM
unsigned int getuid (void)
{
	short cret;
	short cwd,pwd;

	cret = PROCESS_GETINFO_(,,,,,,,,,,&cwd,&pwd);
	return ((unsigned int) (cwd & 255));
}

unsigned int geteuid (void)
{
	short cret;
	short cwd,pwd;

	cret = PROCESS_GETINFO_(,,,,,,,,,,&cwd,&pwd);
	return ((unsigned int) (pwd & 255));
}

unsigned int getgid (void)
{
	short cret;
	short cwd,pwd;

	cret = PROCESS_GETINFO_(,,,,,,,,,,&cwd,&pwd);
	return ((unsigned int) ((cwd >> 8)  & 255));
}

unsigned int getegid (void)
{
	short cret;
	short cwd,pwd;

	cret = PROCESS_GETINFO_(,,,,,,,,,,&cwd,&pwd);
	return ((unsigned int) ((pwd >> 8) & 255));
}

int getpid(void)
{
	return (0);
}

int getppid(void)
{
	return (0);
}

#else
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

#endif  /* TANDEM */
#endif

int getpgrp(void)
{
	return (0);
}
