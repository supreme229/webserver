#include "webserver.h"


Webserver::Webserver(int port, string dir) : port(port), dir(dir), separator("\r\n\r\n"), separator_counter(0), timeout(true),  recv_buffer(new uint8_t[BUFFER_SIZE + 1]) {}

void Webserver::setup() {
    socketSetup();
    serverAddressSetup();
}

void Webserver::socketSetup() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        ERROR("socket error");

    int reuse_option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_option, sizeof(int));
}

void Webserver::serverAddressSetup() {
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&server_address,
             sizeof(server_address)) < 0)
        ERROR("bind error");
}

HTTPHeaders Webserver::readHeaders(char* recv_buffer) {
    HTTPHeaders header;

    size_t idx = string(recv_buffer).find("\n");
    if (idx != string::npos) {
        header.proto = string(recv_buffer).substr(0, idx - 1);
    }

    char* pch;
    pch = strtok((char*)recv_buffer, "\r\n ");
    string previous_word = "";

    while (pch != NULL) {
        string curr_word = (string)pch;
        if (previous_word == "Connection:") {
            header.connection = curr_word;
        } else if (previous_word == "GET") {
            header.address = curr_word;
        } else if (previous_word == "Host:") {
            header.host = curr_word.substr(0, curr_word.find(":"));
        }

        previous_word = curr_word;

        pch = strtok(NULL, "\r\n ");
    }

    return header;
}

ResponseType Webserver::getResponseType(HTTPHeaders header) {
    if (!regex_match (header.proto, regex("GET \\/.* HTTP\\/1\\.1$") )){
        return NOT_IMPLEMENTED_501;
    }
    
    if (header.address == "/") {
        return MOVED_PERMANENTLY_301;
    }

    string not_resolved = "./" + dir + "/" + header.host + header.address;
    char result[255] = "";
    realpath(not_resolved.c_str(), result);

    string file_path = (string)result;

    if (file_path.find("/" + dir + "/" + header.host) == string::npos){
        return FORBIDDEN_403;
    }

    if (fileExists(file_path)) {
        return OK_200;

    } else {
        return ERROR_404;
    }

    return NOT_IMPLEMENTED_501;
}

string Webserver::getType(string address) {
    string type = address.substr(address.find_last_of(".") + 1);

    if (type == "css" || type == "txt" || type == "html") {
        return "text/" + type + type == "html" ? ";charset=utf-8" : "";
    }

    if (type == "jpeg" || type == "jpg") {
        return "image/jpeg";
    }

    if (type == "png") {
        return "image/png";
    }

    if (type == "pdf") {
        return "application/pdf";
    }

    return "application/octet-stream";
}

vector<char> Webserver::packetBuilder(HTTPHeaders header) {

    vector<char> packet;

    string response_str;
    ResponseType response = getResponseType(header);
    string content_type = getType(header.address);

    string not_resolved = "./" + dir + "/" + header.host + header.address;
    char result[255] = "";
    realpath(not_resolved.c_str(), result);

    string file_path = (string)result;

    switch (response) {
    case OK_200: {
        response_str =
            "HTTP/1.1 200 OK\nContent-Type: " + content_type +
            "\nContent-Length: " + to_string(getFileSize(file_path)) + "\n\n";
        break;
    }
    case MOVED_PERMANENTLY_301:
        response_str = "HTTP/1.1 301 Moved Permanently\nLocation: http://" +
                       header.host + ":" + to_string(port) + "/index.html\n\n";
        break;
    case ERROR_404: {
        string error_message =
            "<html><head><title>404 Not "
            "found</title></head><body><center><h1>404 Not "
            "found</h1></center><hr><center> nginx</ center></body></html>";
        response_str = "HTTP/1.1 404 Not Found\nContent-Type: "
                       "text/html\nContent-Length: " +
                       to_string(error_message.size()) + "\n\n" + error_message;
        break;
    }
    case FORBIDDEN_403: {
        string error_message =
            "<html><head><title>403 Forbidden "
            "</title></head><body><center><h1>403 Forbidden "
            "</h1></center><hr><center> nginx</ center></body></html>";
        response_str = "HTTP/1.1 403 Forbidden\nContent-Type: "
                       "text/html\nContent-Length: " +
                       to_string(error_message.size()) + "\n\n" + error_message;
        break;
    }
    case NOT_IMPLEMENTED_501:
        string error_message =
            "<html><head><title>501 Not implemented "
            "</title></head><body><center><h1>501 Not implemented "
            "</h1></center><hr><center> nginx</ center></body></html>";
        response_str = "HTTP/1.1 501 Not implemented\nContent-Type: "
                       "text/html\nContent-Length: " +
                       to_string(error_message.size()) + "\n\n" + error_message;
        break;
    }

    packet = vector<char>(response_str.begin(), response_str.end());

    if (response == OK_200) {
        vector<char> message;
        readFileIntoString(file_path, message, content_type);
        packet.insert(packet.end(), message.begin(), message.end());
    }

    return packet;
}



ssize_t Webserver::receivePacket(int fd, size_t buffer_size) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    bool separator_found = false;

    size_t total_bytes_read = 0;
    while (total_bytes_read < buffer_size) {
        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(fd, &descriptors);
        int ready = select(fd + 1, &descriptors, NULL, NULL, &tv);

        if (ready < 0)
            ERROR("select error");
        if (ready == 0)
            return -2;

        ssize_t bytes_read = recv(fd, recv_buffer + total_bytes_read,
                                  buffer_size - total_bytes_read, 0);

        if (bytes_read < 0)
            ERROR("recv error");
        if (bytes_read == 0)
            return 0;

        for (int i = 0; i < bytes_read; i++) {
            if (recv_buffer[total_bytes_read + i] == separator[separator_counter]) {
                if (separator_counter == 3) {
                    separator_found = true;
                }
                separator_counter++;
            } else {
                separator_counter = 0;
            }
        }

        total_bytes_read += bytes_read;

        if (separator_found)
            return total_bytes_read;
    }

    return 0;
}

void Webserver::serverLoop() {
    if (listen(sockfd, 64) < 0)
        ERROR("listen error");

    for (;;) {
        if (timeout) {
            connected_sockfd = accept(sockfd, NULL, NULL);
        }

        if (connected_sockfd < 0)
            ERROR("accept error")

        ssize_t bytes_read =
            receivePacket(connected_sockfd, BUFFER_SIZE);

        if (bytes_read == -2) {
            timeout = true;
        } else if (bytes_read == 0) {
            timeout = true;
        } else {
            timeout = false;
            recv_buffer[bytes_read] = '\0';
            HTTPHeaders header = readHeaders((char*)recv_buffer);

            vector<char> packet = packetBuilder(header);

            if (write(connected_sockfd, &packet[0], packet.size()) < 0) {
                ERROR("write error");
            }
        }

        if (timeout) {
            if (close(connected_sockfd) < 0)
                ERROR("close error");
        }
    }
}