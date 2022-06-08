#include "webserver.h"
#include <unistd.h>

int main(int argc, char*argv[]){

    Webserver ws(stoi(argv[1]), (string)argv[2]);

    ws.setup();

    ws.serverLoop();

    return 1;
}