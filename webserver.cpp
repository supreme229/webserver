#include "webserver.h"

u_int8_t recv_buffer[BUFFER_SIZE + 1];

Webserver::Webserver(int port, string dir) : port(port), dir(dir) {}

void Webserver::setup()
{
    socketSetup();
    serverAddressSetup();
}

void Webserver::socketSetup()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        ERROR("socket error");
    int val = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
}

void Webserver::serverAddressSetup()
{
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) <
        0)
        ERROR("bind error");
}

HTTPHeaders Webserver::readHeaders(char *recv_buffer)
{
    char *pch;
    pch = strtok((char *)recv_buffer, "\r\n ");
    string previous_word = "";
    HTTPHeaders headers;
    while (pch != NULL)
    {
        string curr_word = (string)pch;

        if (previous_word == "Connection:")
        {
            headers.connection = curr_word;
        }
        else if (previous_word == "GET")
        {
            headers.address = curr_word;
        }
        else if (previous_word == "Host:")
        {
            headers.host = curr_word.substr(0, curr_word.find(":"));
        }

        previous_word = curr_word;
        pch = strtok(NULL, "\r\n ");
    }
    return headers;
}

void Webserver::serverLoop()
{
    if (listen(sockfd, 64) < 0)
        ERROR("listen error");

    for (;;)
    {
        int connected_sockfd = accept(sockfd, NULL, NULL);
        if (connected_sockfd < 0)
            ERROR("accept error")

        ssize_t bytes_read =
            receivePacket(connected_sockfd, recv_buffer, BUFFER_SIZE, 1);

        if (bytes_read == -2)
        {
            printf("Timeout\n");
        }
        else if (bytes_read == 0)
        {
            printf("Client closed connection");
        }
        else
        {
            recv_buffer[bytes_read] = '\0';
            u_int8_t reply_buffer[21];
            strncpy((char *)reply_buffer, (char *)recv_buffer, 20);
            reply_buffer[20] = '\0';
        }

        HTTPHeaders headers = readHeaders((char *)recv_buffer);

        if (headers.address.size())
        {
            cout << "Address: " << headers.address << endl;
            cout << "Host: " << headers.host << endl;
            cout << "Connection: " << headers.connection << endl;
            cout << endl;

            vector<char> packet = packetBuilder(headers);

            if(write(connected_sockfd, &packet[0], packet.size()) < 0)
                ERROR("write error");
        }

        if (close(connected_sockfd) < 0)
            ERROR("close error");
    }
}

ssize_t Webserver::receivePacket(int fd, u_int8_t *buffer, size_t buffer_size,
                                 unsigned int timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    bool separator_found = false;

    size_t total_bytes_read = 0;
    while (total_bytes_read < buffer_size)
    {
        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(fd, &descriptors);
        int ready = select(fd + 1, &descriptors, NULL, NULL, &tv);
        
        
        if (ready < 0)
            ERROR("select error");
        if (ready == 0)
            return -2;
        
        cout << "test1" << endl;

        ssize_t bytes_read =
            recv(fd, recv_buffer + total_bytes_read, buffer_size - total_bytes_read, 0);

        cout << "test2" << endl;
        cout << bytes_read << endl;

        if (bytes_read < 0)
            ERROR("recv error");
        if (bytes_read == 0)
            return 0;

        for (int i = 0; i < bytes_read; i++)
        {
            if (recv_buffer[total_bytes_read + i] == separator[counter])
            {
                if (counter == 3)
                {
                    separator_found = true;
                }
                counter++;
            }
            else
            {
                counter = 0;
            }
        }

        total_bytes_read += bytes_read;

        if (separator_found)
            return total_bytes_read;
    }

    return 0;
}