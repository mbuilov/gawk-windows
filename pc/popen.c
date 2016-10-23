#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <process.h>
#include <errno.h>
#include "popen.h"
#undef popen
#undef pclose
#undef system

#ifndef _NFILE
#define _NFILE 40
#endif

static struct {
  char *command;
  char *name;
  char pmode[4];
} pipes[_NFILE];


/*
 * For systems where system() and popen() do not follow SHELL:
 *  1. Write command to temp file.  Temp filename must have slashes
 *     compatible with SHELL (if set) or COMSPEC.
 *  2. Convert slashes in SHELL (if present) to be compatible with COMSPEC.
 * Currently, only MSC (running under DOS) and MINGW versions are managed.
 */

#if defined(__MINGW32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if 0
static int
unixshell(char *p)
{
  static char *shell[] = {"sh", "bash", "csh", "tcsh", "sh32", "sh16", "ksh", NULL};
  char **shellp = shell, *s, *q;

  if (p == NULL) return (0);
  s = p = strdup(p);
  if ((q = strrchr(p, '\\')) != NULL)
    p = q + 1;
  if ((q = strrchr(p, '/')) != NULL)
    p = q + 1;
  if ((q = strchr(p, '.')) != NULL)
    *q = '\0';
  strlwr(p);
  do {
    if (strcmp(*shellp, p) == 0) break;
  } while (*++shellp);
  free(s);
  return(*shellp ? 1 : 0);
}

static char *
slashify(char *p, char *s)
{
  if (unixshell(s))
    while (s = strchr(p, '\\')) *s = '/';
  else
    while (s = strchr(p, '/')) *s = '\\';
  return(p);
}

static char *
scriptify(const char *command)
{
  FILE *fp;
  char *cmd, *name, *s, *p;
  int i;

  if((name = tempnam(".", "pip")) == NULL) return(NULL);
  p = getenv("COMSPEC"); s = getenv("SHELL");
  cmd = malloc(strlen(name) + (s ? strlen(s) : 0) + 9); *cmd = '\0';
  if (s) {
    slashify(strcpy(cmd, s), p);
    p = s;
  }
  slashify(name, p);
  if (! (i = unixshell(p))) {
    char *p = (char *) realloc(name, strlen(name) + 5);
    if (p == NULL) {
	free(cmd);
	return NULL;
    }
    name = p;
    strcat(name, ".bat");
  }
  if (s) sprintf(cmd + strlen(cmd), " %cc ", unixshell(s) ? '-' : '/');
  strcpy(p = cmd + strlen(cmd), name); free(name);

  if ((fp = fopen(p, i ? "wb" : "w")) != NULL) {
    if (! i) fputs("@echo off\n", fp);
    i = strlen(command);
    if ((fwrite(command, 1, i, fp) < i) || (fputc('\n', fp) == EOF)) {
      free(cmd);
      cmd = NULL;
    }
  } else {
    free(cmd);
    cmd = NULL;
  }
  if (fp) fclose(fp);
  return(cmd);
}

static void
unlink_and_free(char *cmd)
{
  char *s;

  if (s = strrchr(cmd, ' '))
    s++;
  else
    s = cmd;
  unlink(s); free(cmd);
}
#endif

int
os_system(const char *cmd)
{
  char *cmdexe = getenv("ComSpec");
  char *cmd1 = quote_cmd(cmd);
  int i = spawnl(P_WAIT, cmdexe, "cmd.exe", "/c", cmd1, NULL);

  free(cmd1);
  return(i);
}

#ifndef PIPES_SIMULATED
int
kill (int pid, int sig)
{
  HANDLE ph;
  int retval = 0;

  /* We only support SIGKILL.  */
  if (sig != SIGKILL)
    {
      errno = ENOSYS;
      return -1;
    }

  ph = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
  if (ph)
    {
      BOOL status = TerminateProcess(ph, -1);

      if (!status)
	{
	  errno = EPERM;
	  retval = -1;
	}
    }
  else
    {
      /* If we cannot open the process, it means we eaither aren't
	 allowed to (e.g., a process of another user), or such a
	 process doesn't exist.  */
      switch (GetLastError ())
	{
	  case ERROR_ACCESS_DENIED:
	    errno = EPERM;
	    break;
	  default:
	    errno = ESRCH;
	    break;
	}
      retval = -1;
    }
  CloseHandle (ph);
  return retval;
}

char *
quote_cmd(const char *cmd)
{
  char *quoted;

  /* The command will be invoked via cmd.exe, whose behavior wrt
     quoted commands is to remove the first and the last quote
     characters, and leave the rest (including any quote characters
     inside the outer pair) intact.  */
  quoted = malloc(strlen (cmd) + 2 + 1);
  sprintf(quoted, "\"%s\"", cmd);

  return quoted;
}
#endif

#else  /* !__MINGW32__ */
#define os_system(cmd) system(cmd)
#endif


FILE *
os_popen(const char *command, const char *mode )
{
    FILE *current;
    char *name;
    int cur;
    char curmode[4];

    if (*mode != 'r' && *mode != 'w')
      return NULL;
    strncpy(curmode, mode, 3); curmode[3] = '\0';

#if defined(__MINGW32__)
    current = popen(command, mode);
    cur = fileno(current);
    strcpy(pipes[cur].pmode, curmode);
    return(current);
#endif

    /*
    ** get a name to use.
    */
    if((name = tempnam(".","pip"))==NULL)
        return NULL;
    /*
    ** If we're reading, just call system to get a file filled with
    ** output.
    */
    if (*curmode == 'r') {
        FILE *fp;
        if ((cur = dup(fileno(stdout))) == -1)
	    return NULL;
	*curmode = 'w';
        if ((current = freopen(name, curmode, stdout)) == NULL)
	    return NULL;
        os_system(command);
        if (dup2(cur, fileno(stdout)) == -1)
	    return NULL;
	close(cur);
	*curmode = 'r';
        if ((current = fopen(name, curmode)) == NULL)
            return NULL;
    } else {
        if ((current = fopen(name, curmode)) == NULL)
            return NULL;
    }
    cur = fileno(current);
    pipes[cur].name = name;
    strcpy(pipes[cur].pmode, curmode);
    pipes[cur].command = strdup(command);
    return current;
}

int
os_pclose( FILE * current)
{
    int cur = fileno(current);
    int fd, rval;

#if defined(__MINGW32__)
    rval = pclose(current);
    *pipes[cur].pmode = '\0';
    return rval;
#endif

    /*
    ** check for an open file.
    */
    switch (*pipes[cur].pmode) {
    case 'r':
        /*
        ** input pipes are just files we're done with.
        */
        rval = fclose(current);
        unlink(pipes[cur].name);
	break;
    case 'w':
        /*
        ** output pipes are temporary files we have
        ** to cram down the throats of programs.
        */
        fclose(current);
	rval = -1;
	if ((fd = dup(fileno(stdin))) != -1) {
	  char *mode = pipes[cur].pmode; *mode = 'r';
	  if (current = freopen(pipes[cur].name, mode, stdin)) {
	    rval = os_system(pipes[cur].command);
	    fclose(current);
	    if (dup2(fd, fileno(stdin)) == -1) rval = -1;
	    close(fd);
	  }
	}
        unlink(pipes[cur].name);
	break;
    default:
      return -1;
    }
    /*
    ** clean up current pipe.
    */
    *pipes[cur].pmode = '\0';
    free(pipes[cur].name);
    free(pipes[cur].command);
    return rval;
}
