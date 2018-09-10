//
// Created by Seledets Vadim on 9/9/18.
//

#ifndef FILEGETTER_CLIENTTASK_H
#define FILEGETTER_CLIENTTASK_H

#include <openssl/md5.h>
#include "Types.h"
#include "TCPServer.h"
#include "HttpParser.h"
#include "Log.h"

#define BLOCK_SIZE 16384

void *clientsRequest(void *params);

char *getFileHash(struct FileData *fileData);
void saveFile(const char *path, const struct FileData *fileData);

#endif //FILEGETTER_CLIENTTASK_H
