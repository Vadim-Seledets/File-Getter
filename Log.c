#include "Log.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char *getCurrentTime()
{
	char *currentTime = (char *)malloc(TIME_STRING_SIZE);
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);

	size_t length = strftime(currentTime, TIME_STRING_SIZE, "%Y-%m-%d %H:%M:%S", lt);
	currentTime[length] = '\0';

	return currentTime;
}

void writeLog(const char *logFilePath, int argc, ...)
{
	pthread_mutex_lock(&mutex);
	FILE *file = fopen(logFilePath, "a");

	if (file == NULL)
	{
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	char *currentTime = getCurrentTime();

	va_list args;
	va_start(args, argc);

	fprintf(file, "[%s]\t Thread: %d\t ", currentTime, (int)pthread_self());
	while (argc--)
	{
		char *data = va_arg(args, char *);
		fprintf(file, "%s", data);
	}
	fprintf(file, "\n");

	va_end(args);
	fclose(file);
	pthread_mutex_unlock(&mutex);
}
