//
// Created by Seledets Vadim on 9/9/18.
//

#ifndef FILEGETTER_LOG_H
#define FILEGETTER_LOG_H

#include "Types.h"

#define LOG_FILE_PATH "FileGetter.log"
#define TIME_STRING_SIZE 32

void writeLog(const char *logFilePath, int argc, ...);

#endif //FILEGETTER_LOG_H
