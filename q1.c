#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int x = 100;

	printf("[before fork] pid=%d, x=%d\n", getpid(), x);
	pid_t rc = fork();
	if (rc < 0) {
		// fork failure
		perror("fork");
		exit(1);

	} else if (rc == 0) {
		// child (new process)
		x += 1;
		printf("[child ] pid=%d, x=%d\n", getpid(), x);
	} else {
		// parent goes down this path (og)
		x -= 1;
		printf("[parent] pid=%d, x=%d (child pid=%d)\n", getpid(), x, rc);
	}
	printf("[final ] pid=%d, x=%d\n", getpid(), x);
	return 0;
}
