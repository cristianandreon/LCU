/*
 * RTLinux
 *
 *  Created on: May 5, 2017
 *      Author: ubuntu
 */
#include "./RTLinux.h"


//////////////////////////////
// Timer globale
//
struct timespec GLTimeSpec = {0};
int GLInterval_ns = 1000 * 1000; /* 1ms = 1000 us = 1000000 ns */



#define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is guaranteed safe to access without faulting */

void stack_prefault(void) {
    unsigned char dummy[MAX_SAFE_STACK];
    memset(dummy, 0, MAX_SAFE_STACK);
    return;
}



uint32_t GLFaultCounter = 0;
#define MAX_FAULT_PER_SESSION   32

void xrt_sig_handler() {
    char msg[256];
    
    GLFaultCounter++;
    if (GLFaultCounter < MAX_FAULT_PER_SESSION) {
        snprintf(msg, sizeof(msg), "xrt_sig_handler() : Internal Error...please reboot the system");
        if (generate_alarm((char*) msg, 8880, 0, (int) ALARM_WARNING, 0+1) < 0) {
        }    
    } else if (GLFaultCounter == MAX_FAULT_PER_SESSION) {
        snprintf(msg, sizeof(msg), "xrt_sig_handler() : Internal Error...please reboot the system");
        if (generate_alarm((char*) msg, 8880, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
        }            
    } else {
    }
}



int xrt_init() {
    struct sched_param param = {0};

    /*
    cap_t caps = cap_get_proc();
    cap_value_t cap_list[1];
    cap_list[0] = CAP_NET_RAW;
    if (cap_set_flag(caps, CAP_EFFECTIVE, 1, cap_list, CAP_SET) == -1) {
        printf("cap_set_flag error");
    }
    if (cap_set_proc(caps) == -1) {
        printf("cap_set_proc error");
    }
     */


    struct utsname u;
    int crit1, crit2 = 0;
    FILE *fd;

    uname(&u);
    crit1 = (int) strcasestr(u.version, "PREEMPT RT");

    if ((fd = fopen("/sys/kernel/realtime", "r")) != NULL) {
        int flag;
        crit2 = ((fscanf(fd, "%d", &flag) == 1) && (flag == 1));
        fclose(fd);
    }

    char str[512], str_err[512];

    snprintf(str, sizeof (str), "%s%s%s", ANSI_COLOR_GREEN, "PREEMPT RT", ANSI_COLOR_RESET);
    snprintf(str_err, sizeof (str_err), "%s%s%s", ANSI_COLOR_RED, u.version, ANSI_COLOR_RESET);
    snprintf(App.KernelString, sizeof (App.KernelString), "KERNEL : %s \n", (crit1 && crit2) ? str : str_err);

    snprintf(str, sizeof (str), "%s%s%s", "<span style=\"color:darkGreen\"><b>", "PREEMPT RT", "</b></span>");
    snprintf(str_err, sizeof (str_err), "%s%s%s", "<span style=\"color:darkRed\"><b>", u.version, "</b></span>");
    snprintf(App.KernelHTMLString, sizeof (App.KernelHTMLString), "KERNEL : %s \n", (crit1 && crit2) ? str : str_err);
    

            
    if (crit1 && crit2) {
        App.RTKernelOK = 1;
    } else {
        App.RTKernelOK = 0;
    }



    memset(&GLUdpHost, 0, sizeof (GLUdpHost));
    GLNumUdpHost = 0;

    
    
    memset(&GLUdpCallback, 0, sizeof (GLUdpCallback));
    GLNumUdpCallback = 0;

    
    

    GLIOMaxDataPerSec = 0;
    GLIOMinDataPerSec = 1000000;
    GLIOLastDataPerSec = 0;
    GLIODataPerSec = 0;

    GLUIMaxDataPerSec = 0;
    GLUIMinDataPerSec = 1000000;
    GLUIDataPerSec = 0;

    GLSERMinDataPerSec = 1000000;
    GLSERLastDataPerSec = 0;
    GLSERDataPerSec = 0;


    GLLogicErr = 0;
    GLmaxtOut = 0;



    /* Declare ourself as a real time task */
    // printf("sched_setscheduler()...\n");

    if (App.RTKernelOK) {
        param.sched_priority = 49; // sched_get_priority_max(SCHED_RR); // MY_PRIORITY;
        if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
            perror("[WARING:sched_setscheduler failed:");
            fprintf(stderr, "]\n");
            // exit(-1);
        }
    }




    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = xrt_sig_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGSEGV, &sigIntHandler, NULL);

    // sigaction(SIGSTKFLT, &sigIntHandler, NULL);

    // Ignore sigpipe (causa crash)
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGILL, SIG_IGN);
    signal(SIGTRAP, SIG_IGN);
    signal(SIGABRT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGKILL, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
    signal(SIGBUS, SIG_IGN);
    // signal(SIGFAULT, SIG_IGN);
    

    // #define	SIGHUP		1	/* Hangup (POSIX).  */
    // #define	SIGINT		2	/* Interrupt (ANSI).  */
    // #define	SIGQUIT		3	/* Quit (POSIX).  */
    // #define	SIGILL		4	/* Illegal instruction (ANSI).  */
    // #define	SIGTRAP		5	/* Trace trap (POSIX).  */
    // #define	SIGABRT		6	/* Abort (ANSI).  */
    // #define	SIGIOT		6	/* IOT trap (4.2 BSD).  */
    // #define	SIGBUS		7	/* BUS error (4.2 BSD).  */
    // #define	SIGFPE		8	/* Floating-point exception (ANSI).  */
    // #define	SIGKILL		9	/* Kill, unblockable (POSIX).  */
    // #define	SIGUSR1		10	/* User-defined signal 1 (POSIX).  */
    // #define	SIGSEGV		11	/* Segmentation violation (ANSI).  */
    // #define	SIGUSR2		12	/* User-defined signal 2 (POSIX).  */
    // #define	SIGPIPE		13	/* Broken pipe (POSIX).  */
    // #define	SIGALRM		14	/* Alarm clock (POSIX).  */
    // #define	SIGTERM		15	/* Termination (ANSI).  */
    // #define	SIGSTKFLT	16	/* Stack fault.  */
    // #define	SIGCLD		SIGCHLD	/* Same as SIGCHLD (System V).  */
    // #define	SIGCHLD		17	/* Child status has changed (POSIX).  */
    // #define	SIGCONT		18	/* Continue (POSIX).  */
    // #define	SIGSTOP		19	/* Stop, unblockable (POSIX).  */
    // #define	SIGTSTP		20	/* Keyboard stop (POSIX).  */
    // #define	SIGTTIN		21	/* Background read from tty (POSIX).  */
    // #define	SIGTTOU		22	/* Background write to tty (POSIX).  */
    // #define	SIGURG		23	/* Urgent condition on socket (4.2 BSD).  */
    // #define	SIGXCPU		24	/* CPU limit exceeded (4.2 BSD).  */
    // #define	SIGXFSZ		25	/* File size limit exceeded (4.2 BSD).  */
    // #define	SIGVTALRM	26	/* Virtual alarm clock (4.2 BSD).  */
    // #define	SIGPROF		27	/* Profiling alarm clock (4.2 BSD).  */
    // #define	SIGWINCH	28	/* Window size change (4.3 BSD, Sun).  */
    // #define	SIGPOLL		SIGIO	/* Pollable event occurred (System V).  */
    // #define	SIGIO		29	/* I/O now possible (4.2 BSD).  */
    // #define	SIGPWR		30	/* Power failure restart (System V).  */
    // #define SIGSYS		31	/* Bad system call.  */

    
    
    

    /* Lock memory */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("mlockall failed");
        // exit(-2);
    }

    /* Pre-fault our stack */
    stack_prefault();



    clock_gettime(CLOCK_MONOTONIC, &GLTimeSpec);

    // GLTimeSpec.tv_nsec = 0;
    // GLTimeSpec.tv_nsec = 0;


    App.KeyboardxTime = xTaskGetTickCount();


    return 1;
}

uint32_t xrt_set_delta_time(uint32_t end_time_ms, uint32_t start_time_ms) {
    if (end_time_ms >= start_time_ms) {
        return (end_time_ms - start_time_ms);
    } else {
        return (end_time_ms + (OVERFLOW_TICK - start_time_ms));
    }
}







/////////////////////////////
// Funzione attesa timer
//

/* inline */
inline int myTaskDelayUntil(portTickType *xExpectedWakeTime, portTickType xLoopRate, portTickType * tOut) {

    int retVal = TIMER_RUN;
    volatile portTickType xTime = xTaskGetTickCount();

    if (xTime >= xExpectedWakeTime[0]) {
        if (xTime > xExpectedWakeTime[0]) {
            if (tOut)
                tOut[0] = xTime - xExpectedWakeTime[0];
            retVal = TIMER_OUT;
        } else {
            retVal = TIMER_DONE;
        }


        if ((xExpectedWakeTime[0] + xLoopRate) < OVERFLOW_TICK) {
            xExpectedWakeTime[0] = xTime + xLoopRate;
        } else {
            // Wrap to zero
            // printf( "[WRAP]"); fflush(stdout);
            // xTaskResetTickCount();
            xExpectedWakeTime[0] = xLoopRate - (OVERFLOW_TICK - xExpectedWakeTime[0]);
        }
    }

    return retVal;
}

void taskENTER_CRITICAL() {
}

void taskEXIT_CRITICAL() {
}

inline uint32_t xTaskGetTickCount() {

    // NO : n° di clock spesi nel programma
    // clock_t c = clock();

    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint32_t) ((t.tv_sec - GLTimeSpec.tv_sec) * 1000 + (t.tv_nsec /*- GLTimeSpec.tv_nsec*/) / 1000 / 1000);

}

uint32_t TIMER_MS_TO_TICKS(unsigned int ms) {
    return ms;
}

// Stampa lo stato dei Task

void xprojctTaskGetRunTimeStats() {

}

int vTaskDelayUntil(uint32_t *xLastWakeTime, uint32_t sleep_ms) {

    // wait until next shot
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);

    t.tv_nsec += sleep_ms * 1000 * 1000;
    if (t.tv_nsec >= 1000 * 1000 * 1000) {
        t.tv_sec++;
        t.tv_nsec -= 1000 * 1000 * 1000;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

    // usleep(sleep_ms * 1000);
    // clock_t sc = clock();

    /*
     * 
     * uint32_t dc_ms = 0;
    
    while (dc_ms < sleep_ms) {
        dc_ms = (uint32_t)((clock() - sc) / (CLOCKS_PER_SEC / 1000));
    }
    
     * 
     */

    if (xLastWakeTime)
        *xLastWakeTime += sleep_ms;

    return 1;
}

void disable_ints() {
}

int vTaskSuspendAll() {
    return 0;
}

int xTaskResumeAll() {
    return 0;
}

void enable_ints() {
}

int xrt_socket() {
    return socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
}

int xrt_udp_socket() {
    return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

int xrt_bind(int Socket, int Port) {

    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(Port);

    /* Now bind the host address using bind() call.*/
    return bind(Socket, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
}

int xrt_bind_to(int listeningSocket, struct sockaddr_in * serv_addr) {
    return bind(listeningSocket, (struct sockaddr *) serv_addr, sizeof (struct sockaddr));
}

int xrt_listen(int Socket, int n) {
    return listen(Socket, n);
}

int xrt_select(int __nfds, fd_set * __restrict __readfds, fd_set * __restrict __writefds, fd_set * __restrict __exceptfds, struct timeval * __restrict __timeout) {
    return select(__nfds, __readfds, __writefds, __exceptfds, __timeout);
}

int xrt_accept(int Socket, __SOCKADDR_ARG __addr, socklen_t * __restrict __addr_len) {
    return accept(Socket, __addr, __addr_len);
}

int xrt_connect(int Socket, const struct sockaddr *addr, socklen_t addrlen) {
    return connect(Socket, addr, addrlen);
}


int xrt_connect_ex(int Socket, const struct sockaddr *addr, socklen_t addrlen, int timeout) {
    struct timeval Timeout;
    Timeout.tv_sec = timeout;
    Timeout.tv_usec = 0;
 
    //set the socket in non-blocking
    unsigned long iMode = 1;
    int iResult = ioctl(Socket, FIONBIO, &iMode);
    if (iResult != 0) {	
        printf("ioctlsocket failed with error: %d\n", iResult);
    }
	    
    if(connect(Socket,(struct sockaddr *)addr, addrlen)==false) {	
        return -1;
    }	
 
    // restart the socket mode
    iMode = 0;
    iResult = ioctl(Socket, FIONBIO, &iMode);
    if (iResult != 0) {	
        printf("ioctlsocket failed with error: %d\n", iResult);
    }
 
    fd_set Read, Write, Err;
    
    FD_ZERO(&Read);
    FD_ZERO(&Write);
    FD_ZERO(&Err);
    
    FD_SET(Socket, &Read);
    FD_SET(Socket, &Write);
    FD_SET(Socket, &Err);
 
    // check if the socket is ready
    select(Socket+1, &Read, &Write, &Err, &Timeout);			
    if(FD_ISSET(Socket, &Write) || FD_ISSET(Socket, &Read)) {	
        return 1;
    }
 
    if(FD_ISSET(Socket, &Err)) {	
        return -1;
    }
     
    return 0;
}



int xrt_shutdown(int *Socket) {
    if (Socket) {
        if (*Socket) {

            // 0 - Stop receiving data for this socket. If further data arrives, reject it.
            // 1 - Stop trying to transmit data from this socket. Discard any data waiting to be sent. Stop looking for acknowledgement of data already sent; don’t retransmit it if it is lost.
            // 2 - Stop both reception and transmission.
            switch (shutdown(*Socket, 2)) {

                case EBADF:
                    // socket is not a valid file descriptor.
                    break;

                case ENOTSOCK:
                    // socket is not a socket.
                    break;

                case ENOTCONN:
                    // socket is not connected.
                    break;
            }
        }
        *Socket = 0;
    }
    return 0;
}

unsigned short xrt_ntohs(unsigned short port) {
    return ntohs(port);
}

int xrt_recv(int Socket, void *recvBuffer, uint32_t recvBufferSize) {
    return recv(Socket, recvBuffer, recvBufferSize, 0);
}

int xrt_recv_async(int Socket, void *recvBuffer, uint32_t recvBufferSize) {
    int bytes_available = 0;
    ioctl(Socket, FIONREAD, &bytes_available);
    if (bytes_available > 0) {
        return recv(Socket, recvBuffer, recvBufferSize, MSG_DONTWAIT );
    } else {
        return 0;
    }
}

int xrt_send(int Socket, void *sendBuffer, uint32_t sendBufferSize) {
    return send(Socket, sendBuffer, sendBufferSize, 0);
}


// Invia il pacchetto di dati udp (se necessario crea la connessione)

int xrt_send_udp(IpAddr_t ip, uint16_t sport, uint16_t dport, uint8_t *buf, uint16_t buf_size, uint8_t flag) {
    int index1B = xrt_connect_udp(ip, sport, dport);
    if (index1B > 0) {
        if (GLUdpHost[index1B - 1].sock > 0) {
            struct sockaddr_in addr;
            char sAddr[256];
            snprintf(sAddr, sizeof (sAddr), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(sAddr);
            addr.sin_port = htons(sport);

            // return send(GLUdpHost[index1B-1].sock, buf, buf_size, 0);
            int retVal = sendto(GLUdpHost[index1B - 1].sock, buf, buf_size, 0, (struct sockaddr*) &addr, sizeof (addr));
            if (retVal < 0) {
                perror("xrt_send_udp():");
                xrt_shutdown(&GLUdpHost[index1B - 1].sock);
                return -1;
            }
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

int xrt_recive_udp(IpAddr_t ip, uint16_t sport, uint16_t dport, uint8_t *buf, uint16_t buf_size, uint8_t flag) {
    ///////////////////////////////
    // recezione pacchetto udp
    //
    int retVal = -1, index1B = xrt_connect_udp(ip, sport, dport);
    if (index1B > 0) {
        if (GLUdpHost[index1B - 1].sock > 0) {
            unsigned long int bytes_available = 0;
            int rc = ioctl(GLUdpHost[index1B - 1].sock, FIONREAD, &bytes_available);
            if (rc < 0) {
            } else {
                if (bytes_available > 0) {
                    if (buf) {
                        retVal = xrt_recv(GLUdpHost[index1B - 1].sock, buf, buf_size);
                        // callback
                        if (retVal > 0) {
                            if (GLNumUdpCallback > 0) {
                                for (int i=0; i<GLNumUdpCallback; i++) {
                                    if (GLUdpCallback[i].port == sport) {
                                        if (GLUdpCallback[i].func) {
                                            GLUdpCallback[i].func(ip, sport, dport, buf, retVal);
                                        } else {
                                            vDisplayMessage("No udp callback");
                                        }
                                    }
                                }
                            } else {
                                // vDisplayMessage("No udp callbacks");
                            }
                        }
                    }
                } else if (bytes_available == 0) {
                    retVal = 0;
                }
            }
        }
    }

    return retVal;
}

int xrt_connect_udp(IpAddr_t ip, uint16_t sport, uint16_t dport) {
    int index1B = 0;

    for (uint32_t i = 0; i < GLNumUdpHost; i++) {
        if (GLUdpHost[i].host[0] == ip[0] && GLUdpHost[i].host[1] == ip[1] && GLUdpHost[i].host[2] == ip[2] && GLUdpHost[i].host[3] == ip[3]) {
            if (GLUdpHost[i].sport == sport) {
                if (GLUdpHost[i].dport == dport) {
                    index1B = i + 1;
                    break;
                }
            }
        }
    }

    if (!index1B) {

        // qualche posto libero ?
        for (uint32_t i = 0; i < GLNumUdpHost; i++) {
            if (GLUdpHost[i].host[0] == 0 && GLUdpHost[i].host[1] == 0 && GLUdpHost[i].host[2] == 0 && GLUdpHost[i].host[3] == 0) {
                index1B = i + 1;
            }
        }

        if (!index1B)
            if (GLNumUdpHost < MAX_UDP_HOST)
                index1B = ++GLNumUdpHost;

        if (index1B < MAX_UDP_HOST) {
            GLUdpHost[index1B - 1].host[0] = ip[0];
            GLUdpHost[index1B - 1].host[1] = ip[1];
            GLUdpHost[index1B - 1].host[2] = ip[2];
            GLUdpHost[index1B - 1].host[3] = ip[3];
            GLUdpHost[index1B - 1].sport = sport;
            GLUdpHost[index1B - 1].dport = dport;


            GLUdpHost[index1B - 1].sock = xrt_udp_socket();
            if (GLUdpHost[index1B - 1].sock < 0) {
                perror("[ERROR] UDP socket failed:");
            } else {

                int yes = 1;
                setsockopt(GLUdpHost[index1B - 1].sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int));
                struct sockaddr_in addr;
                char sAddr[256];

                snprintf(sAddr, sizeof (sAddr), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = inet_addr(sAddr);
                addr.sin_port = 0; // htons(sport);
                if (xrt_bind_to(GLUdpHost[index1B - 1].sock, &addr) < 0) {
                    // perror("[ERROR] UDP bind failed:");
                } else {
                }
            }
        } else {
            // tutto pieno
        }
    }

    return index1B;
}

int xrt_callback_udp(uint16_t port, lp_udp_callback * udp_callback) {
    if (GLNumUdpCallback < MAX_UPD_CALLBACK) {
        for (int i=0; i<GLNumUdpCallback; i++) {
            if (GLUdpCallback[i].port == port) {
                if (GLUdpCallback[i].func == (lp_udp_callback *)udp_callback) {
                    return i+1;
                } else {
                    GLUdpCallback[i].func = (lp_udp_callback *)udp_callback;
                    return i+1;
                }
            }
        }
        GLUdpCallback[GLNumUdpCallback].port = port;
        GLUdpCallback[GLNumUdpCallback].func = (lp_udp_callback *)udp_callback;
        GLNumUdpCallback++;
        return GLNumUdpCallback;
    } else {
        return 0;
    }
}




#include <ifaddrs.h>

int xrt_get_ip(IpAddr_t * ip) {
    int isIpSet = 0;

    if (ip) {

        struct ifaddrs * ifAddrStruct = NULL;
        struct ifaddrs * ifa = NULL;
        void * tmpAddrPtr = NULL;

        ip[0][0] = 0;
        ip[0][1] = 0;
        ip[0][2] = 0;
        ip[0][3] = 0;


        getifaddrs(&ifAddrStruct);

        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) {
                continue;
            }
            if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
                // is a valid IP4 Address

                tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                // printf("[DEBUG] %s IP Address %s\n", ifa->ifa_name, addressBuffer); 

                if (strncasecmp((char*) ifa->ifa_name, "eth", 3) == 0 ||
                        strncasecmp((char*) ifa->ifa_name, "enp1s", 5) == 0) {

                    strcpy(App.ETHInterfaceName, ifa->ifa_name);

                    // ip adress eth0
                    if (!isIpSet && (ifa->ifa_addr->sa_data[2] || ifa->ifa_addr->sa_data[3] || ifa->ifa_addr->sa_data[4] || ifa->ifa_addr->sa_data[5])) {
                        ip[0][0] = ifa->ifa_addr->sa_data[2];
                        ip[0][1] = ifa->ifa_addr->sa_data[3];
                        ip[0][2] = ifa->ifa_addr->sa_data[4];
                        ip[0][3] = ifa->ifa_addr->sa_data[5];
                        isIpSet = 1;
                    }
                }

            } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
                // is a valid IP6 Address
                tmpAddrPtr = &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;
                char addressBuffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                // printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
                if (strncasecmp((char*) ifa->ifa_name, "eth", 3) == 0 ||
                        strncasecmp((char*) ifa->ifa_name, "enp1s", 5) == 0) {
                    // mac address
                    // fprintf(stdout, "[DEBUG] eth0 mac.addr.: %d.%d.%d.%d.%d.%d", ifa->ifa_addr->sa_data[2], ifa->ifa_addr->sa_data[3], ifa->ifa_addr->sa_data[4], ifa->ifa_addr->sa_data[5], ifa->ifa_addr->sa_data[6], ifa->ifa_addr->sa_data[7]);
                } else if (strcmpi((char*) ifa->ifa_name, "eth1") == 0) {
                    // fprintf(stdout, "[DEBUG] eth1 mac.addr.: %x.%x.%x.%x.%x.%x\n", ifa->ifa_addr->sa_data[2], ifa->ifa_addr->sa_data[3], ifa->ifa_addr->sa_data[4], ifa->ifa_addr->sa_data[5], ifa->ifa_addr->sa_data[6], ifa->ifa_addr->sa_data[7]);
                }
            } else {
                // printf("[DEBUG] Undetected Family : %hu\n", ifa->ifa_addr->sa_family); 
            }
        }

        if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
    }

    return isIpSet;
}

int xrt_arp_resolve(IpAddr_t ip, EthAddr_t * eth_dest) {
    return -1;
}


int xrt_get_mac_addr( char *mac_address ) {
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[512];
    int success = 0;

    if (mac_address)
        mac_address[0] = 0;
    
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) { /* handle error*/ };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */ }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else { /* handle error */ }
    }

    // unsigned char mac_address[6];

    if (success && mac_address) memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
    
    xrt_shutdown(&sock);
}


// Thread utility

int xrt_pthread_create(pthread_t * __restrict __newthread, const pthread_attr_t * __restrict __attr, void *(*__start_routine) (void *), void *__restrict __arg) {
    return pthread_create(__newthread, __attr, __start_routine, __arg);
}




// livel = 2    ->  (admin) read-write-setup
// livel = 1    ->  (user) read-write
// livel = 0    ->  (guest) read-only
int has_user_permission(int level) {
    if (GLLoggedUser1B>0 && GLLoggedUser1B <= GLNumUsers) {
        if (GLUsers[GLLoggedUser1B-1].level >= level) {
            return 1;
        }
    }
    return 0;
}

int is_machine_stopped() {
    if (machine.status == READY
            || machine.status == EMERGENCY
            || machine.status == INITIALIZED
            || machine.status == MANUAL
            ) {
        return 1;
    } else {
        return 0;
    }
}

#include <unistd.h>
#include <sys/reboot.h>

void xrt_reboot() {
    if (is_machine_stopped()) {
        if (has_user_permission(0)) {
            terminate_program(false);
            sync();
            reboot(RB_AUTOBOOT);
        } else {
            if (generate_alarm((char*) "Permission denied", 9901, 0, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    } else {
        if (generate_alarm((char*) "Cannot reboot until machine is running", 9902, 0, (int) ALARM_WARNING, 0+1) < 0) {
        }    
    }
}















// #define _XOPEN_SOURCE 700
// #define _POSIX_C_SOURCE	199309L

int _bios_keybrd(int a) {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof (struct termios));
    new_term_attr.c_lflag &= ~(ECHO | ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}




int format_time_run( uint32_t machine_running, char *machine_elapsed_string, int32_t machine_elapsed_string_size) {
    
    int32_t sec = machine_running % 60;
    int32_t min = (machine_running % 3600) / 60;
    int32_t hour = (machine_running % (3600*24)) / 3600;
    int32_t days = (machine_running / (3600*24));

    if (machine_elapsed_string) {
        if (days > 0) {
            snprintf(machine_elapsed_string, machine_elapsed_string_size, "%dd %02dh %02dm %02ds", days, hour, min, sec);
        } else {
            if (hour > 0) {
                snprintf(machine_elapsed_string, machine_elapsed_string_size, "%02dh %02dm %02ds", hour, min, sec);
            } else {            
                if (min > 0) {
                    snprintf(machine_elapsed_string, machine_elapsed_string_size, "%02dm %02ds", min, sec);
                } else {
                    snprintf(machine_elapsed_string, machine_elapsed_string_size, "%02ds", sec);
                }
            }
        }
    }
    
    return 1;
}            
