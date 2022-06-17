#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "tokens.h"

const char *irc_server_hostname = "irc.chat.twitch.tv";
const char *bot_nickname = "futuresmt";
const char *channel_name = "#futuresmt";
const char *robot_emoji = "\xf0\x9f\xa4\x96"; // 🤖


#define MAX_CURRENT_PROJECT_SIZE 2048
char current_project[MAX_CURRENT_PROJECT_SIZE] = "Writing a Twitch bot in Pure C -- https://github.com/sumeet/twitchdrop";

void send_message(FILE *write_stream, char *msg) {
    // TODO: probably extract this into a socket writing function
    printf("> PRIVMSG %s :%s %s\n", channel_name, robot_emoji, msg);
    fprintf(write_stream, "PRIVMSG %s :%s %s\n", channel_name, robot_emoji, msg);
    fflush(write_stream);
}

int main(void) {
    int sock = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (sock < 0) {
        fprintf(stderr, "error creating socket: %m\n");
        exit(1);
    }

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

    FILE *write_stream = fdopen(dup(sock), "w");

    // authenticate to twitch
    fprintf(write_stream, "PASS %s\n", token);
    fprintf(write_stream, "NICK %s\n", bot_nickname);

    // join the channel (configured by channel_name)
    fprintf(write_stream, "JOIN %s\n", channel_name);

    // send the welcome message
    send_message(write_stream, "hello from C");
    fflush(write_stream);

    FILE *read_stream = fdopen(dup(sock), "r");
    size_t read_buffer_size = 2048;
    char *read_buffer = malloc(read_buffer_size);
    char *target = malloc(1024);
    char *message = malloc(1024);

    while (true) {
        printf("< ");
        fflush(stdout);

        // read line delineated messages from the IRC server
        if (getline(&read_buffer, &read_buffer_size, read_stream) < 0) {
            fprintf(stderr, "unknown to read line from socket: %m\n");
            break;
        }

        printf("%s", read_buffer);
        int scanned = sscanf(read_buffer, ":%*s PRIVMSG %s :%s\n", target, message);
        if (scanned == 2) {
            if (strcmp(message, "!project") == 0 || strcmp(message, "!today") == 0) {
                send_message(write_stream, current_project);
            }
        }
    }

    fclose(write_stream);
    fclose(read_stream);
    close(sock);
    return 0;
}

