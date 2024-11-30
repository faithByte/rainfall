#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int ac, char **av) {
	int		nbr = atoi(av[1]);
	char	*args[] = { "/usr/bin/sh", NULL };
	gid_t	egid;
	uid_t	euid;

	if (nbr != 423)
		printf("No !\n");
	else {
		egid = getegid();
		euid = geteuid();
        setresgid(egid, egid, egid);
        setresuid(euid, euid, euid);
		execve(args[0], args, NULL);
		perror("execve");
	}
	
	return (0);
}
