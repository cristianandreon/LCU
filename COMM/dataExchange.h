#ifndef DATA_EXCHANGE_H

#ifdef EXTERN
#define DATA_EXCHANGE_H extern
#else
#define DATA_EXCHANGE_H 
#endif



// N.B.: File di Configurazione diversi per permetere l'esecuzione nella stessa macchina di più Applicativi

#ifdef xBM_COMPILE

    #define IO_BOARD_CFG_FILE   "XPIO.cfg"
    #define CU_ADDR             200
    #define START_IO_ADDR       201
    #define END_IO_ADDR         205


    #define SCR_BOARD_CFG_FILE   "XPSCR.cfg"
    #define START_SCR_ADDR       210
    #define END_SCR_ADDR         213


    #define SER_BOARD_CFG_FILE  "XPSER.cfg"

    #define CAN_BOARD_CFG_FILE  "XPCAN.cfg"
    #define START_CAN_ADDR       220
    #define END_CAN_ADDR         221

    #define USB_BOARD_CFG_FILE  "XPUSB.cfg"

#elif xCNC_COMPILE

    #define IO_BOARD_CFG_FILE   "xCNC_IO.cfg"
    #define CU_ADDR             200
    #define START_IO_ADDR       201
    #define END_IO_ADDR         205


    #define SCR_BOARD_CFG_FILE   "xCNC_SCR.cfg"
    #define START_SCR_ADDR       210
    #define END_SCR_ADDR         213


    #define SER_BOARD_CFG_FILE  "xCNC_SER.cfg"

    #define CAN_BOARD_CFG_FILE  "xCNC_CAN.cfg"
    #define START_CAN_ADDR       220
    #define END_CAN_ADDR         221

    #define USB_BOARD_CFG_FILE  "xCNC_USB.cfg"

#endif






// Timeout ciclo comunicazioni    
#define APP_TIMEOUT_SEC             30

// Keep Alive interfaccia utente
#define KEEPALIVE_TIMEOUT_MSEC      10000

// Watch dog stream IO (genera emergenza macchina)
#define IO_WATCHDOG_TIMEOUT_MSEC    250

// Watch dog SCR (genera emergenza macchina, o warning)
#define SCR_WATCHDOG_TIMEOUT_MSEC    5000


// Segnale KeepAlive inviato alla scheda IO
#define IO_KEEPALIVE_TIMEOUT_MSEC   2500

// Timeout attesa avvio stream e risoluzione ARP iniziale
#define IO_PENDING_TIMEOUT_MSEC     3000

// Timeout attesa avvio stream e risoluzione ARP iniziale
#define SCR_PENDING_TIMEOUT_MSEC     3000


// Timeout comunizazione seriale (modbus)
#define MODBUS_TIMEOUT_SEC          0
#define MODBUS_TIMEOUT_USEC         250*1000

// Timeout streaming comunizazione seriale (modbus)
#define MODBUS_STREAM_TIMEOUT_MS   350
#define MODBUS_STREAM_PRE_TIMEOUT_MS   125

#define MODBUS_WAIT_BEFOR_RECONNECT_MS  500
#define MODBUS_LOOP_TIME_MS 0
#define MAX_SERIAL_ATTEMPS  3
#define MAX_SERIAL_LOOP_ERROR   3


// Timeout streaming comunizazione seriale (modbus)
#define CANBUS_STREAM_TIMEOUT_MS        5000
#define CANBUS_STREAM_PRE_TIMEOUT_MS    125
#define CANBUS_LOOP_TIME_MS             0
#define MAX_CANOPEN_ATTEMPS             3
#define MAX_CANOPEN_LOOP_ERROR          3





DATA_EXCHANGE_H int GLTestSerial;
DATA_EXCHANGE_H int GLTestCAN;

DATA_EXCHANGE_H IOBoard *GLIOBoard;
DATA_EXCHANGE_H uint16_t GLNumIOBoardAllocated;

DATA_EXCHANGE_H SCRBoard *GLSCRBoard;
DATA_EXCHANGE_H uint16_t GLNumSCRBoardAllocated;



DATA_EXCHANGE_H uint16_t GLListeningPort;
DATA_EXCHANGE_H uint16_t GLIOIpAddrPort;
DATA_EXCHANGE_H uint16_t GLSCRIpAddrPort;
DATA_EXCHANGE_H uint16_t GLCANIpAddrPort;

// Statistiche
DATA_EXCHANGE_H uint16_t GLMStatIOTime;
DATA_EXCHANGE_H uint16_t GLMaxIOTime;

DATA_EXCHANGE_H char *dataExchangeGetStat(int32_t state);
DATA_EXCHANGE_H int dataExchangeInit(int32_t);
DATA_EXCHANGE_H int dataExchangeLoop(int32_t);
DATA_EXCHANGE_H int dataExchangeClose();
DATA_EXCHANGE_H int dataExchangeReset();
DATA_EXCHANGE_H int dataExchangeDump(char *msg, size_t msg_size);
DATA_EXCHANGE_H int dataExchangeDumpSCR(char *msg, size_t msg_size);



DATA_EXCHANGE_H int dataExchangeInitSCR(int32_t);
DATA_EXCHANGE_H int dataExchangeLoopSCR();
DATA_EXCHANGE_H int dataEchangeResetSCR();


DATA_EXCHANGE_H char *readConsole();
DATA_EXCHANGE_H int setupConsole();
DATA_EXCHANGE_H int addToConsole(char *Message);




// Esportazione in C

#ifdef __cplusplus
extern "C" {
#endif





#ifdef __cplusplus
}
#endif



#endif

