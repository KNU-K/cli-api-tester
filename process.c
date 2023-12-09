#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include "network.h"
#include "loglib.h"
LogMode logMode;
#define ROWS 1000
#define COLS 1000
#define MAX_BUFFER_SIZE 1024
Target getTarget();
static char *hostname;
static char *path;
static int port;
static char **params;
static int numParams;
char jsonBody[ROWS][COLS];

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
// param쪼갠거 사용하기, body 넣어서 보내기
char *join_strings_with_separator(char **strings, size_t num_strings, const char *separator)
{
    size_t total_length = 0;

    // Calculate the total length of the concatenated string
    for (size_t i = 0; i < num_strings; ++i)
    {
        total_length += strlen(strings[i]);
    }

    // Add space for separator characters
    total_length += (num_strings - 1) * strlen(separator);

    // Allocate memory for the concatenated string
    char *result = (char *)malloc(total_length + 1); // +1 for the null terminator
    if (result == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Concatenate each string in the array with the separator
    result[0] = '\0'; // Initialize with an empty string
    for (size_t i = 0; i < num_strings; ++i)
    {
        strcat(result, strings[i]);
        if (i < num_strings - 1)
        {
            strcat(result, separator);
        }
    }

    return result;
}
void handleGET()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        error("Error opening socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        error("Error connecting to the server");
    }

    // HTTP GET 요청 생성
    char request[BUFSIZ];
    if (numParams == 0)
    {
        sprintf(request, "GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, hostname, port);
    }
    else
    {
        char *queryString = join_strings_with_separator(params, numParams, "&");
        sprintf(request, "GET %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, queryString, hostname, port);
    }
    // 서버로 요청 전송
    if (write(sockfd, request, strlen(request)) == -1)
    {
        error("Error writing to socket");
    }

    // 서버로부터 응답 읽기
    char buffer[4096];
    ssize_t bytes_received;

    printf("[ result reuestedBody ]\n");
    int cnt = 0;
    time_t rawtime;
    struct tm *info;
    char timeBuffer[80];

    time(&rawtime);
    info = localtime(&rawtime);
    char methodBuffer[512];
    strftime(timeBuffer, sizeof(timeBuffer), "\n\n[%Y-%m-%d %H:%M:%S]_log\n", info);

    while ((bytes_received = read(sockfd, buffer, sizeof(buffer) - 1)) > 0)
    {

        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
        if (logMode == LOG_MODE_ON)
        {
            int logfd = open("./log.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if (cnt == 0)
            {

                write(logfd, timeBuffer, strlen(timeBuffer));
                sprintf(methodBuffer, "method : %s\n", getTarget().method);
                write(logfd, methodBuffer, strlen(methodBuffer));
                cnt++;
            }
            write(logfd, buffer, bytes_received);

            close(logfd);
        }
    }

    if (bytes_received == -1)
    {
        error("Error reading from socket");
    }
    printf("\n\n Pressed Enter, go main menu .. ");
    close(sockfd);
}

void handlePOST()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        error("Error opening socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        error("Error connecting to the server");
    }

    // HTTP POST 요청 생성
    char *json_data = (char *)malloc(ROWS * COLS);
    if (json_data == NULL)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    // Use a single memcpy to copy the entire 2D array
    memcpy(json_data, jsonBody, sizeof(jsonBody) + 1);

    // Null-terminate the string
    json_data[sizeof(jsonBody)] = '\0';

    char request[BUFSIZ];
    if (numParams == 0)
    {
        if (strlen(json_data) == 0)
        {
            sprintf(request, "POST %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, hostname, port);
        }
        else
        {
            sprintf(request, "POST %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", path, hostname, port, strlen(json_data), json_data);
        }
    }
    else
    {
        char *queryString = join_strings_with_separator(params, numParams, "&");
        if (strlen(json_data) == 0)
        {
            sprintf(request, "POST %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, queryString, hostname, port);
        }
        else
        {
            sprintf(request, "POST %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", path, queryString, hostname, port, strlen(json_data), json_data);
        }
    }

    // 서버로 요청 전송
    if (write(sockfd, request, strlen(request)) == -1)
    {
        error("Error writing to socket");
    }

    // 서버로부터 응답 읽기
    char buffer[4096];
    ssize_t bytes_received;

    printf("[ result reuestedBody ]\n");
    int cnt = 0;
    time_t rawtime;
    struct tm *info;
    char timeBuffer[80];

    time(&rawtime);
    info = localtime(&rawtime);
    char methodBuffer[512];
    strftime(timeBuffer, sizeof(timeBuffer), "[%Y-%m-%d %H:%M:%S]_log\n", info);

    while ((bytes_received = read(sockfd, buffer, sizeof(buffer) - 1)) > 0)
    {

        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
        if (logMode == LOG_MODE_ON)
        {
            int logfd = open("./log.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if (cnt == 0)
            {

                write(logfd, timeBuffer, strlen(timeBuffer));
                sprintf(methodBuffer, "method : %s\n", getTarget().method);
                write(logfd, methodBuffer, strlen(methodBuffer));
                cnt++;
            }
            write(logfd, buffer, bytes_received);

            close(logfd);
        }
    }

    if (bytes_received == -1)
    {
        error("Error reading from socket");
    }
    printf("\n\n Pressed Enter, go main menu .. ");
    close(sockfd);
}

void handlePUT()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        error("Error opening socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        error("Error connecting to the server");
    }

    // HTTP POST 요청 생성
    char *json_data = (char *)malloc(ROWS * COLS);
    if (json_data == NULL)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    // Use a single memcpy to copy the entire 2D array
    memcpy(json_data, jsonBody, sizeof(jsonBody) + 1);

    // Null-terminate the string
    json_data[sizeof(jsonBody)] = '\0';

    char request[BUFSIZ];
    if (numParams == 0)
    {
        if (strlen(json_data) == 0)
        {
            sprintf(request, "PUT %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, hostname, port);
        }
        else
        {
            sprintf(request, "PUT %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", path, hostname, port, strlen(json_data), json_data);
        }
    }
    else
    {
        char *queryString = join_strings_with_separator(params, numParams, "&");
        if (strlen(json_data) == 0)
        {
            sprintf(request, "PUT %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, queryString, hostname, port);
        }
        else
        {
            sprintf(request, "PUT %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", path, queryString, hostname, port, strlen(json_data), json_data);
        }
    }
    // 서버로 요청 전송
    if (write(sockfd, request, strlen(request)) == -1)
    {
        error("Error writing to socket");
    }

    // 서버로부터 응답 읽기
    char buffer[4096];
    ssize_t bytes_received;

    printf("[ result reuestedBody ]\n");
    int cnt = 0;
    time_t rawtime;
    struct tm *info;
    char timeBuffer[80];

    time(&rawtime);
    info = localtime(&rawtime);
    char methodBuffer[512];
    strftime(timeBuffer, sizeof(timeBuffer), "[%Y-%m-%d %H:%M:%S]_log\n", info);

    while ((bytes_received = read(sockfd, buffer, sizeof(buffer) - 1)) > 0)
    {

        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
        if (logMode == LOG_MODE_ON)
        {
            int logfd = open("./log.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if (cnt == 0)
            {

                write(logfd, timeBuffer, strlen(timeBuffer));
                sprintf(methodBuffer, "method : %s\n", getTarget().method);
                write(logfd, methodBuffer, strlen(methodBuffer));
                cnt++;
            }
            write(logfd, buffer, bytes_received);

            close(logfd);
        }
    }

    if (bytes_received == -1)
    {
        error("Error reading from socket");
    }
    printf("\n\n Pressed Enter, go main menu .. ");
    close(sockfd);
}

void handleDELETE()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        error("Error opening socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        error("Error connecting to the server");
    }
    // HTTP POST 요청 생성
    char *json_data = (char *)malloc(ROWS * COLS);
    if (json_data == NULL)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    // Use a single memcpy to copy the entire 2D array
    memcpy(json_data, jsonBody, sizeof(jsonBody) + 1);

    // Null-terminate the string
    json_data[sizeof(jsonBody)] = '\0';

    char request[BUFSIZ];
    if (numParams == 0)
    {
        if (strlen(json_data) == 0)
        {
            sprintf(request, "DELETE %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, hostname, port);
        }
        else
        {
            sprintf(request, "DELETE %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", path, hostname, port, strlen(json_data), json_data);
        }
    }
    else
    {
        char *queryString = join_strings_with_separator(params, numParams, "&");
        if (strlen(json_data) == 0)
        {
            sprintf(request, "DELETE %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, queryString, hostname, port);
        }
        else
        {
            sprintf(request, "DELETE %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", path, queryString, hostname, port, strlen(json_data), json_data);
        }
    }
    // 서버로 요청 전송
    if (write(sockfd, request, strlen(request)) == -1)
    {
        error("Error writing to socket");
    }

    // 서버로부터 응답 읽기
    char buffer[4096];
    ssize_t bytes_received;

    printf("[ result reuestedBody ]\n");
    int cnt = 0;
    time_t rawtime;
    struct tm *info;
    char timeBuffer[80];

    time(&rawtime);
    info = localtime(&rawtime);
    char methodBuffer[512];
    strftime(timeBuffer, sizeof(timeBuffer), "[%Y-%m-%d %H:%M:%S]_log\n", info);

    while ((bytes_received = read(sockfd, buffer, sizeof(buffer) - 1)) > 0)
    {

        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
        if (logMode == LOG_MODE_ON)
        {
            int logfd = open("./log.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if (cnt == 0)
            {

                write(logfd, timeBuffer, strlen(timeBuffer));
                sprintf(methodBuffer, "method : %s\n", getTarget().method);
                write(logfd, methodBuffer, strlen(methodBuffer));
                cnt++;
            }
            write(logfd, buffer, bytes_received);

            close(logfd);
        }
    }

    if (bytes_received == -1)
    {
        error("Error reading from socket");
    }
    printf("\n\n Pressed Enter, go main menu .. ");
    close(sockfd);
}

void parseQueryString(const char *queryString)
{
    char *token = strtok((char *)queryString, "&");
    while (token != NULL)
    {
        // 매개변수를 출력 또는 원하는 작업 수행
        params[numParams++] = strdup(token);
        token = strtok(NULL, "&");
    }
}
void splitHostUrl(char *url)
{
    const char *protocolSeparator = "://";
    const char *protocolEnd = strstr(url, protocolSeparator);

    if (protocolEnd != NULL)
    {
        protocolEnd += strlen(protocolSeparator);

        const char *pathSeparator = "/";
        const char *pathStart = strstr(protocolEnd, pathSeparator);
        if (pathStart != NULL)
        {
            size_t hostnamePortLength = pathStart - protocolEnd + 1; // +1 for null character
            char hostnamePort[hostnamePortLength];
            strncpy(hostnamePort, protocolEnd, hostnamePortLength - 1);
            hostnamePort[hostnamePortLength - 1] = '\0';

            const char *portSeparator = ":";
            char *portStr = strstr(hostnamePort, portSeparator);
            if (portStr != NULL)
            {
                *portStr = '\0';
                portStr++;
                port = atoi(portStr);
            }
            else
            {
                port = 80;
            }

            const char *querySeparator = "?";
            char *queryString = strstr(pathStart, querySeparator);
            if (queryString != NULL)
            {
                *queryString = '\0';
                queryString++;
                params = malloc(sizeof(char *) * 10); // 예시로 초기 크기를 10으로 지정
                numParams = 0;
                parseQueryString(queryString);
            }

            hostname = strdup(hostnamePort);
            path = strdup(pathStart);
        }
    }
}

void process()
{
    HostAddr url = getTarget().hostAddr;

    char *url2 = strdup(url); // Allocate memory for a duplicate of the string
    if (url2 == NULL)
    {
        error("Memory allocation error");
    }
    Method method = getTarget().method;
    system("clear");
    splitHostUrl(url2);

    int row = 0, col = 0;

    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        error("Fork failed");
    }

    if (child_pid == 0)
    {
        // Child process
        char ch;
        printf("###### input jsonBody ########\n");
        printf("* Press Ctrl-D if you do not want to enter body!\n");
        while ((ch = getchar()) != EOF)
        {
            // 1000x1000 배열에 문자 저장
            jsonBody[row][col] = ch;
            // 다음 열로 이동
            col++;
            // 열이 배열의 한 행을 초과하면 다음 행으로 이동
            if (col == COLS)
            {
                col = 0;
                row++;
                // 행이 배열의 한 열을 초과하면 입력 종료
                if (row == ROWS)
                {
                    error("out of bound error");
                }
            }
        }

        if (strcmp(method, "GET") == 0)
        {
            handleGET();
        }
        else if (strcmp(method, "POST") == 0)
        {

            handlePOST();
        }
        else if (strcmp(method, "PUT") == 0)
        {
            handlePUT();
        }
        else if (strcmp(method, "DELETE") == 0)
        {
            handleDELETE();
        }
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Parent process
        // Wait for the child process to complete
        wait(NULL);
        getchar();
        return;
    }
}
