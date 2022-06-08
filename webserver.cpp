#include "webserver.h"

u_int8_t recv_buffer[BUFFER_SIZE+1];

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

}

void Webserver::serverAddressSetup(){
 struct sockaddr_in server_address;
    bzero (&server_address, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
        ERROR("bind error");
}

void Webserver::serverLoop()
{
    if (listen (sockfd, 64) < 0)
        ERROR("listen error");


    for (;;) {

        int connected_sockfd = accept (sockfd, NULL, NULL);
        if (connected_sockfd < 0)
            ERROR("accept error")

        ssize_t bytes_read = receivePacket(connected_sockfd, recv_buffer, BUFFER_SIZE, 10, 'X');
        if (bytes_read == -2) {
            printf ("Timeout\n");
        } else if (bytes_read == 0) {
            printf ("Client closed connection");
        } else {
            recv_buffer[bytes_read] = '\0';
            u_int8_t reply_buffer[21];
            strncpy((char*)reply_buffer, (char*)recv_buffer, 20);
            reply_buffer[20] = '\0';
            printf ("%ld bytes read, first 20 bytes: '%s'\n", bytes_read, reply_buffer);
            //send_all (connected_sockfd, reply_buffer, strlen((char*)reply_buffer));
        }

        if (close (connected_sockfd) < 0)
            ERROR("close error");
    }
}

ssize_t Webserver::receivePacket(int fd, u_int8_t *buffer, size_t buffer_size, unsigned int timeout, char separator)
{
    struct timeval tv; tv.tv_sec = timeout; tv.tv_usec = 0;
    bool separator_found = false;
    size_t total_bytes_read = 0;
    while (total_bytes_read < buffer_size) {
        printf("DEBUG: Current value of tv = %.3f\n", (double)tv.tv_sec + (double)tv.tv_usec / 1000000);

        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(fd, &descriptors);
        int ready = select(fd + 1, &descriptors, NULL, NULL, &tv);
        if (ready < 0)
            ERROR("select error");
        if (ready == 0)
            return -2;

        ssize_t bytes_read = recv(fd, buffer + total_bytes_read, buffer_size - total_bytes_read, 0);
        if (bytes_read < 0)
            ERROR("recv error");
        if (bytes_read == 0)
            return 0;

        for (int i = 0; i < bytes_read; i++) {
            if (buffer[total_bytes_read + i] == separator)
                separator_found = true;
        }

        printf("DEBUG: %ld bytes read\n", bytes_read);
        total_bytes_read += bytes_read;

        if (separator_found)
            return total_bytes_read;
    }
    
    return 0;
}