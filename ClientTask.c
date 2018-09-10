#include "ClientTask.h"

void *clientsRequest(void *params)
{
	struct TaskParameters *taskParameters = (struct TaskParameters*)(params);

	char *receivedData;
	size_t dataLength = readData(taskParameters->sockets->currentSocket, &receivedData);
	struct HttpRequest *httpRequest = httpParse(receivedData, dataLength);
	free(receivedData);

	struct HttpData *httpData = getHttpData(httpRequest);

	char *fileHash = getFileHash(httpData->fileData);

	if (strcmp(fileHash, httpData->md5) == 0)
	{
		char *path = (char *)malloc(strlen(taskParameters->directory) + strlen(httpData->fileName) + 2);
		strcpy(path, taskParameters->directory);
		if (taskParameters->directory[strlen(taskParameters->directory) - 1] != '/')
			strcat(path, "/");
		strcat(path, httpData->fileName);

		saveFile(path, httpData->fileData);

		writeLog(LOG_FILE_PATH, 5,
		         "File name: ",
		         httpData->fileName,
		         "\t",
		         "The file has been saved to ",
		         path);
	}
	else
	{
		writeLog(LOG_FILE_PATH, 9,
				"File name: ",
				httpData->fileName,
				"\t",
				"Hashes are not the same: ",
				"[",
				 httpData->md5,
				"] : [",
				 fileHash,
				 "]");
	}

	free(fileHash);
	httpDataFree(&httpData);
	httpRequestFree(&httpRequest);

	return 0;
}

char *getFileHash(struct FileData *fileData)
{
	char *md5 = (char *)malloc(33);
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5_CTX mdContext;

	MD5_Init (&mdContext);

	size_t total = 0;
	size_t bytesRead = 0;
	size_t size = fileData->size;
	while (size > total)
	{
		if (total + BLOCK_SIZE < size)
			bytesRead = BLOCK_SIZE;
		else
			bytesRead = size - total;

		MD5_Update(&mdContext, fileData->data + total, bytesRead);

		total += bytesRead;
	}

	MD5_Final (digest, &mdContext);

	int i;
	for(i = 0; i < MD5_DIGEST_LENGTH; ++i)
		sprintf(&md5[i*2], "%02x", (unsigned int)digest[i]);

	return md5;
}

void saveFile(const char *path, const struct FileData *fileData)
{
	int file = open(path, O_WRONLY | O_CREAT | O_TRUNC);
	if (file == -1)
	{
		int errorNumber = errno;
		writeLog(LOG_FILE_PATH, 2, "open: ", strerror(errorNumber));
		exit(errorNumber);
	}

	write(file, fileData->data, fileData->size);
	close(file);
}