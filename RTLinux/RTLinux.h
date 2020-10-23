/*
 * lcu.h
 *
 *  Created on: 04 mag 2017
 *      Author: ubuntu
 */

#ifndef RTLINUX_H_

    #ifdef EXTERN
    	#ifdef __cplusplus
        	#define RTLINUX_H_ extern "C"
    	#else
        	#define RTLINUX_H_ extern
    	#endif
    #else
        #define RTLINUX_H_
    #endif



    #include <stdio.h>
    #include <stdlib.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <inttypes.h>
    #include <ctype.h>
    #include <unistd.h>
    #include <string.h>
    #include <math.h>



    #include <sys/stat.h>
    #include <sys/utsname.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <sys/fcntl.h>
    #include <sys/mman.h>

    #include <netinet/in.h>
    #include <net/if.h>
    #include <netdb.h>
    #include <arpa/inet.h>

    #include <pthread.h>
    #include <dirent.h>
    #include <termios.h>

    #include <errno.h>






    #define _OLD_POSIX_C_SOURCE _POSIX_C_SOURCE
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L
    // #define HAVE_STRUCT_TIMESPEC

    //#define timespec linux_timespec
    #include <time.h>
    //#undef timespec

    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE _OLD_POSIX_C_SOURCE




    typedef uint8_t	IpAddr_t[4];
    typedef uint8_t	EthAddr_t[6];
    typedef int 	TcpSocket;
    typedef int 	SOCKET;


    typedef void lp_udp_callback(IpAddr_t, uint16_t, uint16_t, int8_t *, uint16_t);
    typedef void *lp_thread_routine(void *);



    #include "./../RTLinux/ControlUnit.h"



    //////////////////////////////
    // Modulo logica macchina
    //
    #ifdef xBM_COMPILE
        #include "./../xBM-Logic/logic_precomp.h"
    #elif xCNC_COMPILE
        #include "./../xCNC-Logic/logic_precomp.h"
    #endif

    
    ////////////////////////////////////
    // Modbus
    //
    #include "../libmodbus-3.1.4/src/modbus.h"


    
    
    ////////////////////////////////////
    // Persistenza (impostazioni)
    //
    #include "../libconfig-1.5/lib/persistManager.h"


    

    ///////////////////////////
    // Modulo comunicazioni
    //
    #include "./../COMM/dataExchange.h"
    #include "./../COMM/dataExchangeIO.h"
    #include "./../COMM/dataExchangeSER.h"
    #include "./../COMM/dataExchangeSCR.h"
    #include "./../COMM/dataExchangeCAN.h"
    #include "./../COMM/dataExchangeUSB.h"
    #include "./../COMM/LinuxARP.h"
    #include "./../COMM/webSocket.h"
    #include "./../COMM/modbusCommands.h"
    #include "./../COMM/canbusCommands.h"
    #include "./../COMM/xProjectCommand.h"
    #include "./../COMM/actuatorCommand.h"
    #include "./../COMM/xrt_modbus.h"
    #include "./../COMM/base64.h"
    #include "./../COMM/sha1.h"


    ////////////////////////////////
    // Modulo Gestione memoria
    //
    #include "./../RTLinux/MemoryManager.h"


    ////////////////////////////////
    // Modulo HTTP
    //
    #include "./../COMM/HTTPRequest.h"
    #include "./../COMM/HTTPServer.h"



    
    ////////////////////////////////////
    // Utilita / Miscellanea
    //
    #include "./../RTLinux/Utility/utility.h"







    #define MY_PRIORITY (49) /* we use 49 as the PRREMPT_RT use 50 as the priority of kernel tasklets and interrupt handler by default */
    #define NSEC_PER_SEC    (1000*1000*1000) /* The number of nsecs per sec. */

    RTLINUX_H_ struct timespec GLTimeSpec;
    RTLINUX_H_ int GLInterval_ns;

    RTLINUX_H_ void stack_prefault(void);
    
    RTLINUX_H_ int xrt_init ();

    RTLINUX_H_ void taskENTER_CRITICAL ();
    RTLINUX_H_ void taskEXIT_CRITICAL ();
    RTLINUX_H_ uint32_t xTaskGetTickCount ();
    RTLINUX_H_ uint32_t TIMER_MS_TO_TICKS ( unsigned int ms );
    RTLINUX_H_ void xprojctTaskGetRunTimeStats();

    RTLINUX_H_ int myTaskDelayUntil( portTickType *xExpectedWakeTime, portTickType xLoopRate, portTickType *tOut );
    RTLINUX_H_ int vTaskDelayUntil( uint32_t *xLastWakeTime, uint32_t sleep_ms );

    RTLINUX_H_ void disable_ints();
    RTLINUX_H_ int vTaskSuspendAll();
    RTLINUX_H_ int xTaskResumeAll();
    RTLINUX_H_ void enable_ints();



    RTLINUX_H_ int _bios_keybrd(int a);


    RTLINUX_H_ int xrt_socket ( );
    RTLINUX_H_ int xrt_bind ( int Socket, int Port );
    RTLINUX_H_ int xrt_bind_to ( int Socket, struct sockaddr_in *serv_addr );
    RTLINUX_H_ int xrt_listen ( int Socket, int n );
    RTLINUX_H_ int xrt_select (int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds, fd_set *__restrict __exceptfds, struct timeval *__restrict __timeout);
    RTLINUX_H_ int xrt_accept( int Socket, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len );
    RTLINUX_H_ int xrt_connect ( int Socket, const struct sockaddr *addr, socklen_t addrlen );
    RTLINUX_H_ int xrt_connect_ex ( int Socket, const struct sockaddr *addr, socklen_t addrlen, int timeout );
    RTLINUX_H_ int xrt_shutdown (int *Socket );

    RTLINUX_H_ unsigned short xrt_ntohs( unsigned short port );

    RTLINUX_H_ int xrt_recv ( int Socket, void *recvBuffer, uint32_t recvBufferSize );
    RTLINUX_H_ int xrt_recv_async ( int Socket, void *recvBuffer, uint32_t recvBufferSize );
    RTLINUX_H_ int xrt_send ( int Socket, void *sendBuffer, uint32_t sendBufferSize );

    RTLINUX_H_ int xrt_connect_udp( IpAddr_t ip, uint16_t sport, uint16_t dport );
    RTLINUX_H_ int xrt_send_udp ( IpAddr_t ip, uint16_t sport, uint16_t dport, uint8_t *buf, uint16_t buf_size, uint8_t flag );
    RTLINUX_H_ int xrt_recive_udp( IpAddr_t ip, uint16_t sport, uint16_t dport, uint8_t *buf, uint16_t buf_size, uint8_t flag );
    RTLINUX_H_ int xrt_callback_udp ( uint16_t port, lp_udp_callback udp_callback );

    RTLINUX_H_ int xrt_get_ip( IpAddr_t *ip  );
    RTLINUX_H_ int xrt_get_mac_addr( char *mac_address );
    RTLINUX_H_ int xrt_arp_resolve( IpAddr_t ip, EthAddr_t *eth_dest );

    RTLINUX_H_ int xrt_pthread_create ( pthread_t *__restrict __newthread, const pthread_attr_t *__restrict __attr, void *(*__start_routine) (void *), void *__restrict __arg );


    RTLINUX_H_ uint32_t xrt_set_delta_time(uint32_t end_time_ms, uint32_t start_time_ms);

    RTLINUX_H_ int has_user_permission(int level);
    RTLINUX_H_ int is_machine_stopped();    
    RTLINUX_H_ void xrt_reboot();

    RTLINUX_H_ int format_time_run( uint32_t machine_running, char *machine_elapsed_string, int32_t machine_elapsed_string_size);


    typedef struct tag_udp_host {

        IpAddr_t host;
        uint16_t sport, dport;

        SOCKET sock;
        
        // char *buf;
        // uint32_t bufSize;
                
        lp_udp_callback *UdpCallback;

    } UPD_HOST, *LP_UDP_HOST;

    
    

    typedef struct tag_udp_callback {

        uint16_t port;
               
        lp_udp_callback *func;

    } UPD_CALLBACK, *LP_UDP_CALLBACK;

    
    #define MAX_UPD_CALLBACK    8

    RTLINUX_H_ UPD_CALLBACK GLUdpCallback[MAX_UPD_CALLBACK];
    RTLINUX_H_ uint32_t GLNumUdpCallback;

    
    #define MAX_UDP_HOST        64
    #define MAX_UDP_BUF_SIZE    4096

    RTLINUX_H_ UPD_HOST GLUdpHost[MAX_UDP_HOST];
    RTLINUX_H_ uint32_t GLNumUdpHost;




    #define NOT_SET_HIGH_VALUE  1000000

    
    RTLINUX_H_ uint32_t GLIOMaxDataPerSec, GLIOMinDataPerSec, GLIOLastDataPerSec, GLIODataPerSec;
    RTLINUX_H_ uint32_t GLUIMaxDataPerSec, GLUIMinDataPerSec, GLUILastDataPerSec, GLUIDataPerSec;    
    RTLINUX_H_ uint32_t GLSERMaxDataPerSec, GLSERMinDataPerSec, GLSERLastDataPerSec, GLSERDataPerSec;    
    RTLINUX_H_ uint32_t GLCANMaxDataPerSec, GLCANMinDataPerSec, GLCANLastDataPerSec, GLCANDataPerSec;
    RTLINUX_H_ uint32_t GLUSBMaxDataPerSec, GLUSBMinDataPerSec, GLUSBLastDataPerSec, GLUSBDataPerSec;

    RTLINUX_H_ uint32_t GLLogicErr, GLmaxtOut;

    
    
    #define TIMER_GET_CURRENT	xTaskGetTickCount
    #define portTICK_RATE_MS    1
    #define configTICK_RATE_HZ  1000


    #define RECV_BUFFER_SIZE    	(1024*1024)
    #define RECV_BUFFER_SIZE_MAX	(1024*1024)





    #define strnicmp	strncasecmp
    #define strcmpi		strcasecmp
    #define getch		getchar


    #define ANSI_COLOR_RED     "\x1b[31m"
    #define ANSI_COLOR_GREEN   "\x1b[32m"
    #define ANSI_COLOR_YELLOW  "\x1b[33m"
    #define ANSI_COLOR_BLUE    "\x1b[34m"
    #define ANSI_COLOR_MAGENTA "\x1b[35m"
    #define ANSI_COLOR_CYAN    "\x1b[36m"
    #define ANSI_COLOR_RESET   "\x1b[0m"


    #define LOWORD(l) ((int16_t)(l))
    #define HIWORD(l) ((int16_t)(((int16_t)(l) >> 16) & 0xFFFF))
    #define LOBYTE(w) ((int8_t)(w))
    #define HIBYTE(w) ((int8_t)(((int16_t)(w) >> 8) & 0xFF))

    #define MAKEWORD(lw,hw)   ((uint16_t)(((uint8_t)(lw))|(((uint16_t)((uint8_t)(hw)))<<8)))
    #define MAKEDWORD(lw, hw)   ((int32_t)(((int16_t)(lw)) | (((int32_t)((int16_t)(hw))) << 16)))

    #define SWAPWORD(w) MAKEWORD(HIBYTE(w),LOBYTE(w))

    #define HEX_TO_INT(__x,__y) ((__x*16)+(__y))
    



    #define DETECT_FILE_EXTENSION(__file_name, __file_ext, __file_ext_size)\
            { uint32_t __local_len, __local_i;\
                    GET_POINTER_LEN (__file_name, __local_len);\
                    if (__local_len) {\
                            for (__local_i=__local_len; __local_i>=0; __local_i--) {\
                                    if (__file_name[__local_i] == '.') {\
                                            strcpy_s(__file_ext, __file_ext_size, ((MDB_CHAR*)&__file_name[__local_i]));\
                                            break;\
                                            }\
                                    if (!__local_i) break;\
                                    }\
                            }\
                    }

    #define STRIP_LAST_BLANK_SPACE(__ptr_string, __ptr_string_size)\
            if (__ptr_string) {\
                    uint32_t local_str_len = strlen ((char*)__ptr_string);\
                    if (local_str_len) {\
                            local_str_len--;\
                            while (__ptr_string[local_str_len] == ' ' && local_str_len) local_str_len--;\
                            __ptr_string[local_str_len+1] = 0;\
                            }\
                    }

    #define STRIP_LAST_BLANK_SPACE2(__ptr_string, ptr_char_value, ptr_char_position)\
            if (__ptr_string) {\
                    uint32_t local_str_len = strlen ((char*)__ptr_string);\
                    if (local_str_len) {\
                            local_str_len--;\
                            while (__ptr_string[local_str_len] == ' ' && local_str_len) local_str_len--;\
                            ptr_char_value = __ptr_string[local_str_len+1];\
                            ptr_char_position = local_str_len+1;\
                            __ptr_string[local_str_len+1] = 0;\
                            } else {\
                            ptr_char_value = 0;\
                            ptr_char_position = 0;\
                            }\
                    }

    #define STRIP_LAST_BLANK_SPACE_FULL STRIP_LAST_BLANK_SPACE2

    #define STRIP_FIRST_BLANK_SPACE(__ptr_string, __ptr_string_size)\
            if (__ptr_string) {\
                    uint32_t local_str_len = 0;\
                    while (__ptr_string[local_str_len] == ' ') local_str_len++;\
                    if (local_str_len) {\
                            strcpy_s (__ptr_string, __ptr_string_size, (MDB_CHAR*)&__ptr_string[local_str_len]);\
                            }\
                    }




    #define STRIP_FIRST_USER_CHAR(__ptr_string, __ptr_user_char, __ptr_string_size )\
            if (__ptr_string) {\
                    uint32_t local_str_len = 0;\
                    while (__ptr_string[local_str_len] == __ptr_user_char) local_str_len++;\
                    if (local_str_len) {\
                            strcpy_s (__ptr_string, __ptr_string_size, (char*)&__ptr_string[local_str_len]);\
                            }\
                    }

    #define STRIP_LAST_USER_CHAR(__ptr_string, __ptr_user_char, __ptr_string_size)\
            if (__ptr_string) {\
                    uint32_t local_str_len = strlen ((char*)__ptr_string);\
                    if (local_str_len) {\
                            local_str_len--;\
                            while (__ptr_string[local_str_len] == __ptr_user_char && local_str_len) local_str_len--;\
                            __ptr_string[local_str_len+1] = 0;\
                            }\
                    }


    #define BIT_ON(ptr, bit) \
            if (!(ptr & bit)) {\
                    ptr = ptr + bit;\
                    }

    #define BIT_OFF(ptr, bit) \
            if (ptr & bit) {\
                    ptr = ptr - bit;\
                    }


    #define ALIGN_BIT(source, target, bit) \
            if (source & bit) {\
                    if (!(target & bit)) {\
                            target = target + bit;\
                            }\
                    } else {\
                    if (target & bit) {\
                            target = target - bit;\
                            }\
                    }\


    
    
#endif /* RTLINUX_H_ */
