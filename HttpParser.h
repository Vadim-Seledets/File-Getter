//
// Created by Seledets Vadim on 9/9/18.
//

#ifndef FILEGETTER_HTTPPARSER_H
#define FILEGETTER_HTTPPARSER_H

#include "Types.h"

#define CRLF "\r\n"
#define MD5_PARAMETER_NAME "md5"
#define FILENAME_PARAMETER_NAME "filename"

struct FileData
{
	char *data;
	size_t size;
};

struct HttpData
{
	char *md5;
	char *fileName;
	struct FileData *fileData;
};

struct HttpHeader
{
	char *parameter;
	char *value;
	struct HttpHeader *next;
};

struct HttpChunk
{
	struct HttpHeader *header;
	char *body;
	size_t bodyLength;
	struct HttpChunk *next;
};

struct HttpRequest
{
	struct HttpHeader *header;
	struct HttpChunk *chunk;
};

struct HttpRequest *httpParse(char *receivedData, size_t dataLength);
struct HttpChunk *getNextChunk(char **restOfData, char *boundary);
struct HttpHeader *getNextHeader(char **restOfData);
char *getNextHeaderLine(char **restOfData);
int isLastChunk(char *restOfData, char *boundary);
char *getBody(char **restOfData, char *boundary, size_t *bodyLength);
char *getBoundary(char *value);
char *getParameterName(char *value);

void httpRequestFree(struct HttpRequest **httpRequest);
void httpDataFree(struct HttpData **httpData);

struct HttpData *getHttpData(const struct HttpRequest *httpRequest);

#endif //FILEGETTER_HTTPPARSER_H
