//
// Created by Seledets Vadim on 9/9/18.
//

#ifndef FILEGETTER_TYPES_H
#define FILEGETTER_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>

struct Sockets
{
	fd_set allSockets;
	int currentSocket;
	int maxSocketDescriptor;
};

struct TaskParameters
{
	struct Sockets *sockets;
	char *directory;
};

#endif //FILEGETTER_TYPES_H
