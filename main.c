//
// Created by Seledets Vadim on 9/9/18.
//

#include "Types.h"
#include "TCPServer.h"
#include "thpool.h"
#include "ClientTask.h"
#include "Log.h"

#define PORT "9034"
#define BACKLOG 10

void daemonRoutine(char *path);

int main(int argc, char **argv) {
	if (argc < 2) {
		printf( "Usage: %s receives files via HTTP/POST", argv[0]);
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();

	if (pid == 0)
	{
		umask(0);
		setsid();

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		writeLog(LOG_FILE_PATH, 1, "Child process has been created");
		daemonRoutine(argv[1]);
	} else if (pid == -1)
	{
		writeLog(LOG_FILE_PATH, 1, "Could not create a child process");
	}
	else
		return 0;
}

void daemonRoutine(char *path)
{
	struct TaskParameters *taskParameters = (struct TaskParameters *)malloc(sizeof(struct TaskParameters));;
	taskParameters->sockets = (struct Sockets *)malloc(sizeof(struct Sockets));
	taskParameters->directory = path;

	threadpool threadPool = thpool_init(8);
	fd_set readSocketDescriptors;

	int *listener = createTCPServer(PORT, BACKLOG);

	FD_ZERO(&taskParameters->sockets->allSockets);
	FD_ZERO(&readSocketDescriptors);
    	FD_SET(*listener, &taskParameters->sockets->allSockets);

	taskParameters->sockets->maxSocketDescriptor = *listener;

    while (1)
    {
	readSocketDescriptors = taskParameters->sockets->allSockets;

        if (select(taskParameters->sockets->maxSocketDescriptor + 1, &readSocketDescriptors, NULL, NULL, NULL) == -1)
        {
	        int errorNumber = errno;
	        writeLog(LOG_FILE_PATH, 1, strerror(errorNumber));
	        exit(errorNumber);
        }

        int i;
        for(i = 0; i <= taskParameters->sockets->maxSocketDescriptor; ++i)
        {
            if (FD_ISSET(i, &readSocketDescriptors))
            {
                if (i == *listener)
                {
                    addNewClient(*listener, &taskParameters->sockets->allSockets, &taskParameters->sockets->maxSocketDescriptor);
                }
                else
               	{
	                taskParameters->sockets->currentSocket = i;
	                FD_CLR(i, &taskParameters->sockets->allSockets);
			thpool_add_work(threadPool, (void *)clientsRequest, (void *)taskParameters);
                }
            }
        }
    }
}
