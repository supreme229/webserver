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
#include <fcntl.h>
#include <netdb.h>
#include <chrono>
#include <ctime>
#include <cmath>
#include <algorithm>

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
    string separator;
    int separator_counter;
    int connected_sockfd;

public:
    void serverLoop();

private:
    ssize_t receivePacket(int fd, size_t buffer_size);
    HTTPHeaders readHeaders(char* recv_buffer);
    vector<char> packetBuilder(HTTPHeaders header);
    bool timeout;
    u_int8_t *recv_buffer;

private:
    int sockfd;
    struct sockaddr_in server_address;
    void socketSetup();
    void serverAddressSetup();

private:
    string getType(string address);
    ResponseType getResponseType(HTTPHeaders header);
};