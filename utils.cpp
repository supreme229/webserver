#include "utils.h"

bool readFileIntoString(const string &path, vector<char> &message, string type)
{
    ifstream input_file;

    if (type.find("image/") != string::npos)
    {
        input_file.open(path, ios::binary);
    }else{
        input_file.open(path);
    }

    if (!input_file.is_open())
    {
        return false;
    }

    vector<char> vec;

    if (!input_file.eof() && !input_file.fail())
    {
        input_file.seekg(0, std::ios_base::end);
        std::streampos fileSize = input_file.tellg();
        vec.resize(fileSize);

        input_file.seekg(0, std::ios_base::beg);
        input_file.read(&vec[0], fileSize);
    }

    message = vec;

    return true;
}

int getFileSize(const string &path)
{
    ifstream input_file(path, ios::binary);
    input_file.seekg(0, ios::end);
    return input_file.tellg();
}

string getType(string address)
{
    string type = address.substr(address.find_last_of(".") + 1);

    if (type == "css" || type == "txt" || type == "html")
    {
        return "text/" + type;
    }

    if (type == "jpeg" || type == "jpg")
    {
        return "image/jpeg";
    }

    if (type == "png")
    {
        return "image/png";
    }

    if (type == "pdf")
    {
        return "application/pdf";
    }

    return "application/octet-stream";
}

vector<char> packetBuilder(HTTPHeaders header)
{

    vector<char> packet;

    string content_type = getType(header.address);

    if (header.address == "/")
    {
        printf("here\n");
        string s = "HTTP/1.1 301 Moved Permanently\nLocation: http://" + header.host + ":8888" + "/index.html\n\n";
        packet = vector<char>(s.begin(), s.end());
    }
    else
    {
        string file_path = "./webpages/" + header.host + header.address;
        vector<char> temp;

        int correct_file = readFileIntoString(file_path, packet, content_type);

        if (!correct_file)
        {
            string error_message = "<html><head><title>404 Not found</title></head><body><center><h1>404 Not found</h1></center><hr><center> nginx</ center></body></html>";
            string packet_str = "HTTP/1.1 404 NOT FOUND\nContent-Type: text/html\nContent-Length: " + to_string(error_message.size()) + "\n\n" + error_message;
            packet = vector<char>(packet_str.begin(), packet_str.end());

        }
        else
        {
            string packet_str = "HTTP/1.1 200 OK\nContent-Type: " + content_type + "\nContent-Length: " +
                     to_string(getFileSize(file_path)) + "\n\n";

            vector<char> temp(packet_str.begin(), packet_str.end());
            vector<char> result;

            result.reserve( temp.size() + packet.size() ); // preallocate memory
            result.insert( result.end(), temp.begin(), temp.end() );
            result.insert( result.end(), packet.begin(), packet.end() );

            return result;
        }
    }

    for(auto lett : packet){
        cout << lett;
    }

    return packet;
}