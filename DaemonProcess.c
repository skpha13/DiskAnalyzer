#include <stdio.h>
#include <unistd.h>	//for fork()
#include <stdlib.h>	//for exit()
#include <sys/stat.h>	//for umask
#include <sys/types.h>
#include <signal.h>
#include <syslog.h>


// Message added in syslog later
#define LOGGING  "Start logging my task = %d\n"

int main(){
	pid_t pid;
	int x_fd;

	/* Fork off the parent process */
	pid = fork();
	
	/* Error */
	if(pid < 0)
		exit(EXIT_FAILURE);

	/* Let parent terminate */
	if(pid > 0)
		exit(EXIT_SUCCESS);

	/* On success: The child process becomes session leader and process group leader */
	if(setsid() < 0)
		exit(EXIT_FAILURE);

	/* Catch, ignore and handle signals */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	/* Fork off the second time */
	pid = fork();

	/* Error */
	if(pid < 0)
		exit(EXIT_FAILURE);

	/* Let parent terminate */
	if(pid > 0)
		exit(EXIT_SUCCESS);

	/* Allow read, write, execute permission for the file's owner only, created by daemon */
	umask(077);


	/* Change the working directory to the root directory
	 * if the current directory is on some mounted file system,
	 * so daemon process will not let the mounted file system to unmount */
	chdir("/");

	/* Close all open file descriptors */
	for(x_fd = sysconf(_SC_OPEN_MAX) ; x_fd >= 0 ; x_fd--)
		close(x_fd);



	/* Logging Erros/Info in the syslog system */

	// log into syslog a message every second	

	 int count = 0;
	openlog("Logs", LOG_PID, LOG_USER);
	while(1){
		sleep(1);
	//	syslog(LOG_INFO, LOGGING, count++);
	}
	closelog();

	
	
	return 1;
}
