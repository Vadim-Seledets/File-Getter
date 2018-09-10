//
// Created by Seledets Vadim on 9/9/18.
//

#ifndef FILEGETTER_TCPSERVER_H
#define FILEGETTER_TCPSERVER_H

#include "Types.h"
#include "Log.h"

#define BUFFER_SIZE 16384

int *createTCPServer(const char *port, int backlog);
void addNewClient(int listener, fd_set *sockets, int *maxSocketDescriptor);
void *get_in_addr(struct sockaddr *sa);
size_t readData(int socket, char **buf);
int isDataAvaliable(int socket);

#endif //FILEGETTER_TCPSERVER_H
