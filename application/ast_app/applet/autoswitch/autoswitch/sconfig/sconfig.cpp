#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "cmdline.h"
#include "utilities.h"

#define CMD_SOCKET    "/tmp/sconfig"

#ifndef MAIN_SOCKET_NAME
#define MAIN_SOCKET_NAME    "/tmp/mainswitch"
#endif

using namespace std;

static int sock = -1;

bool sendEvent(int argc, char *argv[])
{
    bool ret = false;
    if (argc != 4) {
        std::cout << "sconfig --event {plugin|plugout|signal-valid|signal-invalid} HDMI[1-3]\n";
        return ret;
    }

    if (!(!strcasecmp(argv[2], "plugin")
         || !strcasecmp(argv[2], "plugout")
         || !strcasecmp(argv[2], "signal-valid")
          || !strcasecmp(argv[2], "signal-invalid"))) {
        std::cout << "sconfig --event {plugin|plugout|signal-valid|signal-invalid} HDMI[1-3]\n";
        return ret;
    }

    if (!(!strcasecmp(argv[3], "hdmi1")
         || !strcasecmp(argv[3], "hdmi2")
         || !strcasecmp(argv[3], "hdmi3"))) {
        std::cout << "sconfig --event {plugin|plugout|signal-valid|signal-invalid} HDMI[1-3]\n";
        return ret;
    }

    std::string msg("event ");
    msg += argv[2];
    msg += " ";
    msg += argv[3];

    ssize_t nbytes = unixsock_send_message(sock, MAIN_SOCKET_NAME, msg.c_str(), msg.length());
    if (nbytes > 0)
        ret = true;

    return ret;
}


//sconfig --mode {FILO|priority|manual}
bool setWorkMode(int argc, char *argv[])
{
    bool ret = false;
    if (!(!strcasecmp(argv[2], "FILO")
         || !strcasecmp(argv[2], "priority")
         || !strcasecmp(argv[2], "manual"))) {
        std::cout << "sconfig --mode {FILO|priority|manual}\n";
        return ret;
    }
    std::string msg("set mode ");
    msg += argv[2];
    ssize_t nbytes = unixsock_send_message(sock, MAIN_SOCKET_NAME, msg.c_str(), msg.length());
    if (nbytes > 0)
        ret = true;
    return ret;
}

//sconfig --priority {HDMI1 HDMI2 HDMI3}
bool setPriority(int argc, char *argv[])
{
    bool ret = false;
    if (argc != 5) {
        std::cout << "sconfig --priority {HDMI1 HDMI2 HDMI3}\n";
        return ret;
    }

    if (!(!strncasecmp(argv[2], "hdmi", 4)
         || !strncasecmp(argv[3], "hdmi", 4)
         || !strncasecmp(argv[4], "hdmi", 4))) {
        std::cout << "sconfig --priority {FILO|priority|manual}\n";
        return ret;
    }
    std::string msg("set priority ");
    msg += argv[2];
    msg += " ";
    msg += argv[3];
    msg += " ";
    msg += argv[4];
    ssize_t nbytes = unixsock_send_message(sock, MAIN_SOCKET_NAME, msg.c_str(), msg.length());
    if (nbytes > 0)
        ret = true;
    return ret;
}

//sconfig --input {HDMI1 | HDMI2 | HDMI3}
bool setInputSource(int argc, char *argv[])
{
    bool ret = false;
    if (!(!strcasecmp(argv[2], "hdmi1")
         || !strcasecmp(argv[2], "hdmi2")
         || !strcasecmp(argv[2], "hdmi3"))) {
        std::cout << "sconfig --input {HDMI1 | HDMI2 | HDMI3}\n";
        return ret;
    }

    std::string msg("set input ");
    msg += argv[2];
    ssize_t nbytes = unixsock_send_message(sock, MAIN_SOCKET_NAME, msg.c_str(), msg.length());
    if (nbytes > 0)
        ret = true;
    return ret;
}

bool showInformation(int argc, char *argv[])
{
    bool ret = false;
    if (!(!strcasecmp(argv[2], "mode")
         || !strcasecmp(argv[2], "priority")
         || !strcasecmp(argv[2], "input"))) {
        return ret;
    }

    std::string msg("get ");
    msg += argv[2];
    msg += " ";
    msg += "nouse";
    ssize_t nbytes = unixsock_send_message(sock, MAIN_SOCKET_NAME, msg.c_str(), msg.length());
    if (nbytes > 0)
        ret = true;

    char buff[BUFSIZ] = {0};
    int numBytes = recvfrom(sock, buff, BUFSIZ, 0, NULL, NULL);
    if (numBytes == -1) {
        perror("recvfrom");
    } else {
        std::cout << buff << "\n";
    }

    return ret;
}

int main(int argc, char *argv[])
{
    cmdline::parser parser;

    //sconfig --event {plugin|plugout|signal-valid|signal-invalid} HDMI[1-3]
    parser.add<string>("event", 0, "source event", false, "");

    //sconfig --mode {FILO|priority|manual}
    parser.add<string>("mode", 0, "switch mode", false, "");

    //sconfig --priority {HDMI1 HDMI2 HDMI3}
    parser.add<string>("priority", 0, "set priority", false, "");

    //sconfig --input {HDMI1 | HDMI2 | HDMI3}
    parser.add<string>("input", 0, "set current input", false, "");

    //sconfig --show {mode | priority | input}
    parser.add<string>("show", 0, "show information", false, "");

    parser.add("help", 0, "print this message");

    bool status = parser.parse(argc, argv);
    if (argc == 1 || parser.exist("help") || !strcasecmp(argv[1], "?")) {
        cerr << parser.usage();
        return 0;
    }

    if (!status) {
        cerr << parser.error() << endl;
        cerr << parser.usage();
        return 0;
    }

    sock = create_unixsocket(CMD_SOCKET);
    if (sock == -1) {
        perror("unix socker error.");
        return -1;
    }

    if (parser.exist("event")) {
        sendEvent(argc, argv);
    }

    if (parser.exist("mode")) {
        setWorkMode(argc, argv);
    }

    if (parser.exist("priority")) {
        setPriority(argc, argv);
    }

    if (parser.exist("input")) {
        setInputSource(argc, argv);
    }

    if (parser.exist("show")) {
        showInformation(argc, argv);
    }

    if (sock != -1) {
        close(sock);
    }
	return 0;
}
