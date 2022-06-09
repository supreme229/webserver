#include "utils.h"

bool readFileIntoString(const string& path, vector<char>& message,
                        string type) {
    ifstream input_file;

    if (type.find("image/") != string::npos) {
        input_file.open(path, ios::binary);
    } else {
        input_file.open(path);
    }

    if (!input_file.is_open()) {
        return false;
    }

    vector<char> vec;

    if (!input_file.eof() && !input_file.fail()) {
        input_file.seekg(0, std::ios_base::end);
        std::streampos fileSize = input_file.tellg();
        vec.resize(fileSize);

        input_file.seekg(0, std::ios_base::beg);
        input_file.read(&vec[0], fileSize);
    }

    message = vec;

    return true;
}

bool fileExists(string path) {
    ifstream input_file(path);

    if (!input_file.is_open()) {
        return false;
    }

    input_file.close();

    return true;
}

int getFileSize(const string& path) {
    ifstream input_file(path, ios::binary);
    input_file.seekg(0, ios::end);
    return input_file.tellg();
}

ResponseType getResponseType(HTTPHeaders header) {
    if (!regex_match (header.proto, regex("GET \\/.* HTTP\\/1\\.1$") )){
        return NOT_IMPLEMENTED_501;
    }
    
    if (header.address == "/") {
        return MOVED_PERMANENTLY_301;
    }

    string not_resolved = "./webpages/" + header.host + header.address;
    char result[255] = "";
    realpath(not_resolved.c_str(), result);

    string file_path = (string)result;

    if (file_path.find("/webpages/" + header.host) == string::npos){
        return FORBIDDEN_403;
    }

    if (fileExists(file_path)) {
        return OK_200;

    } else {
        return ERROR_404;
    }

    return NOT_IMPLEMENTED_501;
}

string getType(string address) {
    string type = address.substr(address.find_last_of(".") + 1);

    if (type == "css" || type == "txt" || type == "html") {
        return "text/" + type;
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
