#if defined(__MINGW32__) || defined(_MSC_VER)

#include <process.h>

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

#endif	/* __MINGW32__ || _MSC_VER */

int getpgrp(void)
{
  return (0);
}

#if defined(__DJGPP__) || defined(__MINGW32__) || defined(_MSC_VER)
int getppid(void)
{
  return (0);
}
#endif	/* __DJGPP__ || __MINGW32__ || _MSC_VER */
