#include <fstream>
#include <iostream>
#include <string>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <chrono>
#include <fcntl.h>
#include <netdb.h>

#include "utils.h"

using namespace std;

#define BUFFER_SIZE 10000000
#define ERROR(str) { fprintf(stderr, "%s: %s\n", str, strerror(errno)); exit(EXIT_FAILURE); }


class Webserver
{
public:
    Webserver(int port, string dir);
    void setup();

private:
    int port;
    string dir;
    string separator = "\r\n\r\n";
    int counter = 0;

public:
    void serverLoop();

private:
    ssize_t receivePacket(int fd, u_int8_t *buffer, size_t buffer_size, unsigned int timeout);
    HTTPHeaders readHeaders(char* recv_buffer);

private:
    int sockfd;
    struct sockaddr_in server_address;
    void socketSetup();
    void serverAddressSetup();
};