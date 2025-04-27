#include <easylogger.h>
#include <easynet.h>

int main(int argc, char** argv) {
    EZ_INFO("Hello!");

    EZ_INIT_NETWORK();
    ez_Server* server = EZ_GENERATE_SERVER();
    EZ_OPEN_SERVER(server, 5000);
    ez_Connection* connection = EZ_SERVER_ACCEPT(server);
    EZ_INFO("Got a connection!");
    EZ_CLEAN_NETWORK();

    return 0;
}