#pragma once

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
#include <vector>
#include <sstream>

#define ERROR(str) { fprintf(stderr, "%s: %s\n", str, strerror(errno)); exit(EXIT_FAILURE); }

using namespace std;

struct HTTPHeaders{
    string address;
    string host;
    string connection;
};

bool readFileIntoString(const string& path, vector<char> &message, string type);

int getFileSize(const string& path);

vector<char> packetBuilder(HTTPHeaders header);

string getType(string address);
