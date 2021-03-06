#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "tokens.h"
#include <tcl.h>

const char *irc_server_hostname = "irc.chat.twitch.tv";
const char *bot_nickname = "sumcademy";
const char *channel_name = "#sumcademy";
const char *robot_emoji = "\xf0\x9f\xa4\x96"; // 🤖

// TODO: respond to PINGs
// TODO: how to fix `unknown to read line from socket: Illegal seek` happening on startup
// TODO: how to fix `unknown to read line from socket: Connection reset by peer`


void w(FILE *file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    fflush(file);
    va_end(args);
}

void send_message(FILE *write_stream, char *msg) {
    // TODO: probably extract this into a socket writing function
    printf("> PRIVMSG %s :%s %s\n", channel_name, robot_emoji, msg);
    w(write_stream, "PRIVMSG %s :%s %s\n", channel_name, robot_emoji, msg);
}

int tcl_send_message(FILE *write_stream, Tcl_Interp *interp, int argc, const char *argv[]) {
    if (argc != 2) {
        Tcl_SetResult(interp, "wrong number of arguments", TCL_STATIC);
        return TCL_ERROR;
    }
    send_message(write_stream, (char *) argv[1]);
    Tcl_SetResult(interp, "", TCL_STATIC);
    return TCL_OK;
}


Tcl_Interp *init_tcl(FILE *write_stream) {
    Tcl_Interp *interp = Tcl_CreateInterp();
    if (Tcl_Init(interp) != TCL_OK) {
        fprintf(stderr, "error initializing Tcl: %s\n", Tcl_GetStringResult(interp));
        exit(1);
    }

    Tcl_CreateCommand(interp,
                      "send_message",
                      (void *) tcl_send_message,
                      write_stream,
                      NULL);

    Tcl_EvalFile(interp, "main.tcl");
    printf("Loaded main.tcl: |%s|\n", Tcl_GetStringResult(interp));

    return interp;
}

// TODO: make sure not to overflow objv
int chop_string_to_tcl_objs(char *string, char delim, Tcl_Obj **objv) {
    char strtok_delim[2] = {delim, 0};
    char *first_word = strtok(string, strtok_delim);
    Tcl_Obj* obj = Tcl_NewStringObj(first_word, -1);
    int objc = 0;
    objv[objc++] = obj;
    Tcl_IncrRefCount(obj);
    while (true) {
        char *next = strtok(NULL, strtok_delim);
        if (next == NULL) {
            break;
        }
        obj = Tcl_NewStringObj(next, -1);
        Tcl_IncrRefCount(obj);
        objv[objc++] = obj;
    }
    return objc;

}

int main(void) {
    int sock = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (sock < 0) {
        w(stderr, "error creating socket: %m\n");
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
    w(write_stream, "PASS %s\n", token);
    w(write_stream, "NICK %s\n", bot_nickname);

    // join the channel (configured by channel_name)
    w(write_stream, "JOIN %s\n", channel_name);
    fflush(write_stream);

    Tcl_Interp *interp = init_tcl(write_stream);

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
        int scanned = sscanf(read_buffer, ":%*s PRIVMSG %s :%[^\r\n]\r\n", target, message);
        // if we detect a message that starts with !, we're going to try and call a Tcl
        // command with that corresponding name. for example, if someone types !project,
        if (scanned == 2 && message[0] == '!') {
            printf(">>> %s\n", message);

            Tcl_Obj *objv[1024];
            int objc = chop_string_to_tcl_objs(message, ' ', objv);
            Tcl_EvalObjv(interp, objc, objv, 0);
            for (int i = 0; i < objc; i++) Tcl_DecrRefCount(objv[i]);

            Tcl_Obj *result = Tcl_GetObjResult(interp);
            if (result->typePtr != NULL && strcmp(result->typePtr->name, "list") == 0) {
                int objc;
                Tcl_Obj **objv;
                Tcl_ListObjGetElements(interp, result, &objc, &objv);
                for (int i = 0; i < objc; i++) {
                    send_message(write_stream, Tcl_GetString(objv[i]));
                }
            } else {
                send_message(write_stream, (char *) Tcl_GetStringResult(interp));
            }
        }
    }

    free(read_buffer);
    free(target);
    free(message);

    fclose(write_stream);
    fclose(read_stream);
    close(sock);
    return 0;
}

