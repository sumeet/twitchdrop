#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "tokens.h"

const char* irc_server_hostname = "irc.chat.twitch.tv";
const char* bot_nickname = "futuresmt";
const char* channel_name = "#futuresmt";
const char* robot_emoji = "\xf0\x9f\xa4\x96"; // 🤖

void send_message(FILE *write_stream, char *msg) {
    fprintf(write_stream, "PRIVMSG %s :%s %s\n", channel_name, robot_emoji, msg);
}

int main(void) {
    int sock = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (sock < 0) {
        fprintf(stderr, "error creating socket: %m\n");
        exit(1);
    }

    // TODO XXX WARNING: this seems to be leaking memory, according to valgrind
    // freehostent doesn't seem to work, and neither does free
    struct hostent *host = gethostbyname(irc_server_hostname);
    if (host == NULL) {
        fprintf(stderr, "unknown host: %s\n", irc_server_hostname);
        exit(1);
    }

    struct sockaddr_in saddr = {0};
    saddr.sin_family = AF_INET;
    memcpy(&saddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    saddr.sin_port = htons(6667);

    if (connect(sock, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        fprintf(stderr, "unknown to connect: %m\n");
        exit(1);
    }

    // XXX TODO: check resource allocation, see if we're leaking
    FILE *f = fdopen(dup(sock), "w");
    fprintf(f, "PASS %s\n", token);
    fprintf(f, "NICK %s\n", bot_nickname);
    fflush(f);
    fprintf(f, "JOIN %s\n", channel_name);
    send_message(f, "hello from C");

    fclose(f);
    close(sock);
}

