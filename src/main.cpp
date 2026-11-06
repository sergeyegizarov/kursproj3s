#include "Server.h"
int main(int argc, char** argv)
{
    UserInterface ui;
    ui.readArgs(argc, argv);
    Server server(ui);
    server.start();
    return 0;
}