#include <stdio.h>
#include <cstdio>
#include "thread.h"
#include "networking.h"
#include "parse.h"

#define SERV_ADDR "192.168.47.1"
#define SERV_PORT 6667
#define RECONNECT_TIME 5000
#define MAX_TIME_WAIT 15
#define CHAN "#recruits"
#define MAX_LINE_SIZE 512
#define RECV_BUFFER_SIZE 512
#define MAX_WORDS 5

