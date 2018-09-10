#include "TCPServer.h"

int *createTCPServer(const char *port, int backlog)
{
	int *listener = (int *)malloc(sizeof(int));

	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *addressInfoList;
	int status;
	if ((status = getaddrinfo(NULL, port, &hints, &addressInfoList)) != 0) {
		writeLog(LOG_FILE_PATH, 1, gai_strerror(status));
		exit(status);
	}

	struct addrinfo *addressInfo;
	for(addressInfo = addressInfoList; addressInfo != NULL; addressInfo = addressInfo->ai_next)
	{
		*listener = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
		if (*listener < 0)
		{
			continue;
		}

		int isActive = 1;
		setsockopt(*listener, SOL_SOCKET, SO_REUSEADDR, &isActive, sizeof(int));

		if (bind(*listener, addressInfo->ai_addr, addressInfo->ai_addrlen) < 0)
		{
			close(*listener);
			continue;
		}

		struct sockaddr_in *addr_in = (struct sockaddr_in *)addressInfo->ai_addr;
		writeLog(LOG_FILE_PATH, 2,
				"The server has been started: ",
				 inet_ntoa(addr_in->sin_addr));

		break;
	}

	if (addressInfo == NULL) {
		int errorNumber = errno;
		writeLog(LOG_FILE_PATH, 1, strerror(errorNumber));
		exit(errorNumber);
	}

	freeaddrinfo(addressInfoList);

	if (listen(*listener, backlog) == -1) {
		int errorNumber = errno;
		writeLog(LOG_FILE_PATH, 1, strerror(errorNumber));
		exit(errorNumber);
	}

	return listener;
}

void addNewClient(int listener, fd_set *sockets, int *maxSocketDescriptor)
{
	int newSocket;
	struct sockaddr_storage clientAddress;
	socklen_t addrlen = sizeof clientAddress;
	char remoteIP[INET6_ADDRSTRLEN];

	newSocket = accept(listener, (struct sockaddr *)&clientAddress, &addrlen);

	if (newSocket == -1)
	{
		int errorNumber = errno;
		writeLog(LOG_FILE_PATH, 1, strerror(errorNumber));
	}
	else
	{
		int flags = fcntl(newSocket, F_GETFL, 0);
		fcntl(newSocket, F_SETFL, flags | O_NONBLOCK);

		FD_SET(newSocket, sockets);

		if (newSocket > *maxSocketDescriptor)
		{
			*maxSocketDescriptor = newSocket;
		}

		writeLog(LOG_FILE_PATH, 2,
				"New connection from ",
		        inet_ntop(clientAddress.ss_family, get_in_addr((struct sockaddr*)&clientAddress), remoteIP, INET6_ADDRSTRLEN));
	}
}

size_t readData(int socket, char **buf)
{
	size_t total = 0;
	ssize_t bytesRead = 0;

	*buf = (char *)malloc(BUFFER_SIZE);

	size_t i = 1;
	while (isDataAvaliable(socket))
	{
		bytesRead = recv(socket, *buf + total, BUFFER_SIZE, 0);

		total += bytesRead;
		*buf = (char *)realloc(*buf, BUFFER_SIZE * ++i);
	}

	if (bytesRead <= 0)
	{
		if (bytesRead == 0)
		{
			close(socket);
		}
		else
		{
			int errorNumber = errno;
			writeLog(LOG_FILE_PATH, 1, strerror(errorNumber));
		}
	}
	else
		close(socket);

	return total;
}

int isDataAvaliable(int socket)
{
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(socket, &readfds);

	struct timeval time;
	time.tv_sec = 6;
	time.tv_usec = 0;

	if (select(socket + 1, &readfds, NULL, NULL, &time) == -1)
	{
		int errorNumber = errno;
		writeLog(LOG_FILE_PATH, 1, strerror(errorNumber));
		exit(errorNumber);
	}

	return FD_ISSET(socket, &readfds) ? 1 : 0;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}