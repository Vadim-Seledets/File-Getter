#include "HttpParser.h"

struct HttpRequest *httpParse(char *receivedData, size_t dataLength)
{
	struct HttpRequest *httpRequest = (struct HttpRequest *)malloc(sizeof(struct HttpRequest));

	httpRequest->header = getNextHeader(&receivedData);

	struct HttpHeader *header = httpRequest->header;
	while ((header->next = getNextHeader(&receivedData)) != NULL)
		header = header->next;

	header = httpRequest->header;
	while (strcmp(header->parameter, "Content-Type") != 0)
		header = header->next;

	if (strstr(header->value, "multipart/form-data") != NULL)
	{
		char *boundary = getBoundary(header->value);

		httpRequest->chunk = getNextChunk(&receivedData, boundary);

		struct HttpChunk *httpChunk = httpRequest->chunk;
		while ((httpChunk->next = getNextChunk(&receivedData, boundary)) != NULL)
			httpChunk = httpChunk->next;
	}
	else
	{
		return NULL;
	}

	return httpRequest;
}

struct HttpChunk *getNextChunk(char **restOfData, char *boundary)
{
	struct HttpChunk *httpChunk = (struct HttpChunk *)malloc(sizeof(struct HttpChunk));

	*restOfData += 2;

	if (isLastChunk(*restOfData, boundary))
		return NULL;

	httpChunk->header = getNextHeader(restOfData);

	if (httpChunk->header == NULL)
		return NULL;

	struct HttpHeader *header = httpChunk->header;
	while ((header->next = getNextHeader(restOfData)) != NULL)
		header = header->next;

	httpChunk->body = getBody(restOfData, boundary, &httpChunk->bodyLength);

	httpChunk->next = NULL;

	return httpChunk;
}

struct HttpHeader *getNextHeader(char **restOfData)
{
	struct HttpHeader *httpHeader = (struct HttpHeader *)malloc(sizeof(struct HttpHeader));

	char *httpHeaderLine = getNextHeaderLine(restOfData);

	if (httpHeaderLine == NULL)
		return NULL;

	char *delimiter = strchr(httpHeaderLine, ':');

	if (delimiter != NULL)
	{
		size_t difference = delimiter - httpHeaderLine;

		httpHeader->parameter = (char *) malloc(difference + 1);
		strncpy(httpHeader->parameter, httpHeaderLine, difference);
		httpHeader->parameter[difference] = '\0';

		httpHeader->value = (char *) malloc(strlen(httpHeaderLine) - difference - 1);
		strncpy(httpHeader->value, delimiter + 2, strlen(httpHeaderLine) - difference - 1);
	}
	else
	{
		return NULL;
	}

	httpHeader->next = NULL;

	return httpHeader;
}

char *getNextHeaderLine(char **restOfData)
{
	if ((*restOfData = strstr(*restOfData, CRLF)) != NULL)
	{
		*restOfData += 2;

		char *lastChar;
		if ((lastChar = strstr(*restOfData, CRLF)) != NULL)
		{
			if (lastChar - *restOfData > 0) 
			{
				char *header = (char *) malloc(lastChar - *restOfData + 1);
				strncpy(header, *restOfData, lastChar - *restOfData);
				header[lastChar - *restOfData] = '\0';

				return header;
			} 
			else
			{
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

int isLastChunk(char *restOfData, char *boundary)
{
	char *ch = restOfData + strlen(boundary) - 2;

	return ch[0] == '-' ? 1 : 0;
}

char *getBody(char **restOfData, char *boundary, size_t *bodyLength)
{
	char *body;
	char *firstChar = *restOfData + 2;

	size_t i = 0;
	while (strstr(*restOfData + i, boundary) == NULL)
		++i;

	*restOfData = strstr(*restOfData + i, boundary);

	if (*restOfData != NULL)
	{
		*bodyLength = *restOfData - firstChar;
		body = (char *) malloc(*bodyLength * sizeof(char));
		bcopy(firstChar, body, *bodyLength);
	}
	else
	{
		return NULL;
	}

	return body;
}

char *getBoundary(char *value)
{
	char *boundary;
	char *firstChar = strstr(value, "=") + 1;

	if (firstChar != NULL)
	{
		size_t difference = firstChar - value;

		boundary = (char *) malloc(strlen(value) - difference + 5);
		strcpy(boundary, CRLF);
		strcat(boundary, "--");
		strncpy(boundary + 4, firstChar, strlen(value) - difference + 1);

		return boundary;
	}
	else
	{
		return NULL;
	}
}

char *getParameterName(char *value)
{
	char *parameterName;
	char *firstChar = strstr(value, "name=") + 6;
	char *lastChar = strchr(firstChar, '\"');

	if (firstChar != NULL)
	{
		size_t length = lastChar - firstChar;
		parameterName = (char *) malloc(length + 1);
		strncpy(parameterName, firstChar, length);
		parameterName[length] = '\0';

		return parameterName;
	}
	else
	{
		return NULL;
	}
}

struct HttpData *getHttpData(const struct HttpRequest *httpRequest)
{
	struct HttpData *httpData = (struct HttpData *)malloc(sizeof(struct HttpData));

	struct HttpChunk *httpChunk = httpRequest->chunk;
	while (httpChunk != NULL)
	{
		struct HttpHeader *httpHeader = httpChunk->header;
		while (strcmp(httpHeader->parameter, "Content-Disposition") != 0)
			httpHeader = httpHeader->next;

		char *parameterName = getParameterName(httpHeader->value);

		if (strcmp(parameterName, MD5_PARAMETER_NAME) == 0)
		{
			httpData->md5 = (char *)malloc(httpChunk->bodyLength + 1);
			strncpy(httpData->md5, httpChunk->body, httpChunk->bodyLength);
			httpData->md5[httpChunk->bodyLength] = '\0';
		} else if (strcmp(parameterName, FILENAME_PARAMETER_NAME) == 0)
		{
			httpData->fileName = (char *)malloc(httpChunk->bodyLength + 1);
			strncpy(httpData->fileName, httpChunk->body, httpChunk->bodyLength);
			httpData->fileName[httpChunk->bodyLength] = '\0';
		}
		else
		{
			httpData->fileData = (struct FileData *)malloc(sizeof(struct FileData));
			httpData->fileData->data = httpChunk->body;
			httpData->fileData->size = httpChunk->bodyLength;
		}

		httpChunk = httpChunk->next;
	}

	return httpData;
}

void httpHeaderFree(struct HttpHeader **httpHeader)
{
	if ((*httpHeader)->next != NULL)
		httpHeaderFree(&(*httpHeader)->next);

	free(*httpHeader);
}

void httpChunkFree(struct HttpChunk **httpChunk)
{
	if ((*httpChunk)->next != NULL)
		httpChunkFree(&(*httpChunk)->next);

	httpHeaderFree(&(*httpChunk)->header);
	free((*httpChunk)->body);
	free(*httpChunk);
}

void httpRequestFree(struct HttpRequest **httpRequest)
{
	httpHeaderFree(&(*httpRequest)->header);
	httpChunkFree(&(*httpRequest)->chunk);
	free(*httpRequest);
}

void httpDataFree(struct HttpData **httpData)
{
	free((*httpData)->fileData);
	free((*httpData)->fileName);
	free((*httpData)->md5);
	free(*httpData);
}
