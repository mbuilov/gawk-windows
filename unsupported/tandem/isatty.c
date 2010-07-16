#include <cextdecs(FILE_GETINFO_)>
#include <stdioh>
#include <talh>

int isatty(int fd)
{
	short cret,sfd,typ[5];
	sfd = (short) fd;
	cret = FILE_GETINFO_(sfd,,,,,&typ[0]);
	if(typ[0] == 6)
		return (1);
	else
		return (0);
}
int dup(int fd)
{
	return (fd);
}
int dup2(int fd, int fd2)
{
	return (0);
}
