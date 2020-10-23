#ifndef CONTROL_UNIT_H


#ifdef EXTERN
#ifdef __cplusplus
#define CONTROL_UNIT_H extern "C"
#else
#define CONTROL_UNIT_H extern
#endif
#else
#define CONTROL_UNIT_H
#endif




typedef unsigned int portTickType;
typedef unsigned int TickType_t;
typedef unsigned int clockTicks_t;
typedef int SOCKET;


#define OVERFLOW_TICK   (0xFFFFFFFF)

enum VAR_TYPE {
    TYPE_STRING = 1,    // char *
    TYPE_SRC_STRING,    // char **
    TYPE_TEXT,          // char[]
    TYPE_INT,
    TYPE_INT_PTR,
    TYPE_SHORT,
    TYPE_FLOAT3,
    TYPE_FLOAT2,
    TYPE_FLOAT1,
    TYPE_FIXED_FLOAT3,
    TYPE_FIXED_FLOAT2,
    TYPE_FIXED_FLOAT1,
    TYPE_FLOAT3_UINT,
    TYPE_FLOAT2_UINT,
    TYPE_FLOAT1_TEMPERATURE,
    TYPE_MSEC,
    TYPE_SEC,
    TYPE_SEC_STR,
    TYPE_BOOL,
    TYPE_ENUM,
    TYPE_CHAR,
    TYPE_MAC_STAT,
    TYPE_PERCENT,
    TYPE_SYS_TICK,
    TYPE_SYS_TIME,
    TYPE_MAC_ADDR,
    TYPE_TRACE_NUM_DATA,
    TYPE_TRACE_HEADER,
    TYPE_TRACE_POS,
    TYPE_TRACE_SPEED,
    TYPE_TRACE_ACC,
    TYPE_TRACE_TORQUE,
    TYPE_CONSOLE,
    TYPE_ALARM_LIST,
    TYPE_LAST_ALARM_LIST,
    TYPE_ACTUATOR_STATUS,
    TYPE_ACTUATOR_DRIVER_STATUS,
    TYPE_UNK
};

typedef struct tag_cu_vars_cache {
    void *varAddr;
    char typeOf; // 1=string   2=int 3=float(3digit) 4=float(2digit)
    int Id;
} cu_vars_cache;

enum TIMER_STATE {
    TIMER_RUN = 0,
    TIMER_DONE,
    TIMER_OUT
};

enum GLSATES {
    STATE_UNINIT = 0,
    STATE_INIT,
    STATE_START,
    STATE_PENDING,
    STATE_WAITING_FOR_STREAM,
    STATE_LISTEN,
    STATE_CONNETED,
    STATE_WEBSOCKET_PARSING,
    STATE_REPLYNG,
    STATE_WEBSOCKET_REPLYNG,
    STATE_WEBSOCKET_SINGLE_REPLYNG,
    STATE_WEBSOCKET_REPLYNG_DATA,
    STATE_WEBSOCKET_SINGLE_REPLYNG_DATA,
    STATE_CLOSING,
    STATE_CLOSED,
    STATE_TESTING,
    STATE_MAX
};



// Stati comunicazione seriale

enum GL_DEVICE_STATES {
    DEV_STATE_UNINIT = 0,
    DEV_STATE_INIT,
    DEV_STATE_WAITING,
    DEV_STATE_CONNECTING,
    
    DEV_STATE_HOMING = 10,
    DEV_STATE_HOMING_INIT,
    DEV_STATE_HOMING_SEND,
    DEV_STATE_HOMING_RECV,
    DEV_STATE_HOMING_DONE,
            
    DEV_STATE_INIT_STREAM = 20,
    DEV_STATE_STREAMING_SEND,
    DEV_STATE_STREAMING_RECV,
    DEV_STATE_STREAMING_DONE,
    DEV_STATE_STREAMING_POST_ACT = 40,
    DEV_STATE_STREAMING_POST_WAIT,
    DEV_STATE_SERVICE = 50,
    DEV_STATE_SERVICE_SETUP,
    DEV_STATE_SERVICE_SETUP_I,
    DEV_STATE_SERVICE_SETUP_II,
    DEV_STATE_SERVICE_SETUP_III,
    DEV_STATE_SERVICE_SETUP_IV,
    DEV_STATE_SERVICE_SETUP_SPEED_ACC,
    DEV_STATE_SERVICE_FIRST_SETUP,
    DEV_STATE_SERVICE_STOP,
    DEV_STATE_SERVICE_OUT,
    DEV_STATE_SERVICE_RESET,
    DEV_STATE_CMD_INIT = 200,
    DEV_STATE_CMD_INIT_SEND,
    DEV_STATE_CMD_INIT_RECV,
    DEV_STATE_CMD_FEEDBACK_SEND,
    DEV_STATE_CMD_FEEDBACK_RECV,
    DEV_STATE_CMD_DONE,
    DEV_STATE_CMD_DONE_FEEDBACK,
    DEV_STATE_CMD_ERROR,
    DEV_STATE_CLOSING,
    DEV_STATE_MAX
};

typedef struct {
    
    uint8_t id;
    uint8_t mac[6];
    uint8_t ip[4];
    uint16_t port;

    uint32_t tickTocIOKeepAliveTimeout;
    uint32_t tickTocIOWatchDogTimeout;
    uint32_t tickTocIOPendingTimeout;

    uint32_t tickTocIOWatchDogCurrent;
    uint32_t tickTocIOPendingCounter;
    uint32_t tickTocIOWatchDog;
    uint32_t tickTocIOKeepAlive;
    uint32_t tickTocReadi2c;
    uint32_t Readi2cIntervalMs;

    uint32_t timeout_count;

    uint8_t commState;

    uint8_t buf[4096];
    uint16_t buf_size;

    int32_t i2cValues[255];
    uint32_t i2cNumValues;

    int32_t Mode;
    
    uint8_t disabled;

} IOBoard;


typedef struct {
    uint8_t id;
    uint8_t mac[6];
    uint8_t ip[4];
    uint16_t port;
    uint16_t starIndex;
    uint16_t endIndex;


    // valori correnti
#define MAX_SCR_ROWS    16

    int numRows;
    int Rows[MAX_SCR_ROWS];

    float AnalogVoltage[3];
    float TollRefChange;

    uint16_t SessionID;
    uint16_t KALoopTimeMs;
    uint16_t MaxErr;
    uint16_t StatusCode, StatusDataSize;
    uint8_t StatusData[256];

    uint32_t tickTocWatchDog;
    uint32_t tickTocWatchDogCurrent;
    uint32_t tickTocWatchDogTimeout;
    uint32_t tickTocPendingTimeout;
    uint32_t tickTocPendingCounter;
    uint32_t tickTocReadi2c;
    uint32_t tickTocReadScr;

    uint32_t timeout_count;

    uint8_t commState;
    int ShouldInit;

    uint8_t buf[512];
    uint16_t buf_size;

    int i2cValues[255];
    unsigned int i2cNumValues;

    int Mode;

    int8_t disabled;
    
} SCRBoard;


#define INIT_AND_RUN    2

typedef struct tag_app {
    
    bool Initialized;
    
    uint8_t CANOK, SEROK, SCROK, USBOK, IOOK;
    
    uint8_t SERDetectError, SCRDetectError, IODetectError, CANDetectError, USBDetectError;
    
    uint32_t MajVer, MinVer;
    char KernelString[256], KernelHTMLString[256];

    volatile portTickType KeyboardxTime;

    int RTKernelOK;
    char ETHInterfaceName[256];
    
    char TerminateProgram;
    char escCounter;

    // Nemero di schede di IO rilevate
    uint32_t NumIOBoard, NumSCRBoard;

    uint8_t MyIpAddr[4];
    uint8_t Gateway[4];
    uint8_t Netmask[4];    
    uint8_t MacAddress[6];

    
    pthread_t LogicThreadId, CommThreadId, HTTPServicesThreadId, WatchDogThreadId;



    int RTRun;
    int RTCommRun;
    int RTCommInit;
    int RTHttpInit;
    int CheckRTTickts;


    // Contatore errori stack
    uint32_t StackOverlow;

    int RTCRunning, HTTPRunning, SerialRunning;

    // Stato interfaccia Utente
    enum GLSATES UICommState;
    uint32_t SendWebSocketError;


    uint32_t UIRunLoop;
    uint32_t UIIDLECounter;

    uint32_t UIMaxStatTime[16];
    uint32_t UIMaxTime[16];

    
    uint32_t UIRecvBufferSize;
    uint8_t *UIRecvBuffer;
    uint32_t UIIndexRecvBuffer;
    uint32_t UIIndexWebSocketBuffer;
    int UIRecvBufferLastResp;

    uint32_t webSocketHeaderSize;
    uint8_t webSocketHeader[64];
            
    uint32_t UISendBufferSize;
    uint8_t *UISendBuffer;

    
    
    char HTTPRunLoop;
    uint32_t HTTPIDLECounter;
    enum GLSATES HTTPServerState;

    uint32_t recvBufferSize;
    uint8_t *recvBuffer;

    uint32_t sendBufferSize;
    uint8_t *sendBuffer;

    uint16_t HTTPLListeningPort;

    uint32_t *HTTPSocket;
    char *HTTPBufferToSend;
    uint32_t HTTPBufferToSendSize;

    uint32_t HTTPMaxTime[16];
    
    
    
    uint32_t SERMeasurePostTimeout;
    uint16_t SERRunLoop;
    bool SERReadPosIDLE;
    
    uint32_t CANMeasurePostTimeout;
    uint16_t CANRunning;
    uint16_t CANRunLoop;
    bool CANReadPosIDLE;

    // Modalita simulazione movimenti
    int SimulateMode;


    char ExecFilePath[256];

    char *Msg;
    uint32_t MsgSize;

    uint32_t TrackPacketRecived;

    int numNewIO, numUnresolvedIO, numDuplicateIpIO, numIOOK;

    int numNewSCR, numUnresolvedSCR, numDuplicateIpSCR, numSCROK;

    int numNewCanbus, numUnresolvedCanbus, numDuplicateCanbus, numCanbusOK;

    
    bool DebugMode;
    bool DebugModbusParam;
    
    uint32_t dumpWebSocket;
    
    
    uint32_t LogicWaitUSec;
    uint32_t LogicLastTimeNanoSec, LogicMaxTimeNanoSec, LogicMinTimeNanoSec;
    
    
    // Test : comando unico di partenza
    //  N.B: dimuisce i tempi di latanza ??? ma sbaglia verso (manca la pusa fra le due contro word???)
    bool CanBusUseFullCommand;
    int32_t CanBusFullCmdFeedbackTimeoutMS;
    int32_t CanBusFullCmdMacAttemps;
    int32_t CanOpenVersion;
    int32_t CanBusStreamTimeoutMS;
            
    int32_t CurLanguage;
    
    // Separatore di record nello stream
    char RowSep[2];
    
    
    // Connessione corrente
    char UIConnectedIp[256];
    uint16_t UIConnectedPort;

    
} APP, *LP_APP;




#ifdef __cplusplus
#endif

CONTROL_UNIT_H void terminate_program(bool bExit);

CONTROL_UNIT_H char *get_machine_status_desc();

// Visualizzazione messaggi
CONTROL_UNIT_H void vDisplayMessage(const char * const pcMessageToPrint);


// Callback


CONTROL_UNIT_H void process_keyboard_input(char *msg, uint32_t msg_size);


CONTROL_UNIT_H int system_get_value(uint8_t *var_name, uint8_t *out_str, uint32_t out_str_size, int Mode);


CONTROL_UNIT_H char *sanitizeString(char *str);
CONTROL_UNIT_H int checkString(char *str);



CONTROL_UNIT_H APP App;

CONTROL_UNIT_H struct timespec GLTimeSpec;


#ifdef __cplusplus
#endif

#endif

