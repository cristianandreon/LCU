
/*
________________________________________________________________________________________

TITOLO SEZIONE :        Definizione strutture dati
AUTORE :                        Cristian Andreon
DATA :                  6-5-2007
________________________________________________________________________________________                                                                                                                                                                                                                                                                                                            */


#ifndef __CONSTANT_KEY

#define __CONSTANT_KEY

// #pragma message ("__CONSTANT_KEY DEFINED")


#ifdef WATCOM
#ifdef __cplusplus
#define _far
#else
#define _far    __far
#endif
#else
#define _far
#endif






#ifndef NULL
#define NULL (void)0
#endif


/*
 
#ifdef WATCOM
    #ifndef uint32_t
        #define uint32_t long int unsigned _far
    #endif

    #ifndef uint32_t
        #define uint32_t long int unsigned _far
    #endif

#else

    #ifndef uint32_t
        #define uint32_t int unsigned _far
    #endif

    #ifndef int32_t
        #define int32_t int _far
    #endif

#endif


#ifndef uint16_t
#define uint16_t short unsigned _far
#endif

#ifndef int16_t
#define int16_t short _far
#endif

#ifndef UCHAR
#define UCHAR char unsigned _far
#endif

#ifndef uint8_t
#define uint8_t char unsigned _far
#endif


#ifndef int8_t
#define int8_t char _far
#endif

*/


#ifndef USHORT
    #define USHORT uint16_t
#endif

#ifndef UINT
    #define UINT uint32_t
#endif


#ifndef HWND
#define HWND void *
#endif


#ifndef BOOL
#define BOOL char
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif


#define MDB_CHAR        char

#define GENERAL_STRUCTURE_INSERT    2
#define GENERAL_STRUCTURE_SHIFT     3
#define GENERAL_STRUCTURE_MOVE_UP   4
#define GENERAL_STRUCTURE_MOVE_DOWN 5



#define strncpy_s(__tgt, __tgt_size, __src)    strncpy(__tgt, __src, __tgt_size)
#define strcpy_s(__tgt, __tgt_size, __src)    strncpy(__tgt, __src, __tgt_size)
#define strcat_s(__tgt, __tgt_size, __src)    strncat(__tgt, __src, __tgt_size)
#define sprintf_s   snprintf
#define strcmpi		strcasecmp

#define IDYES   1

#define DELTA_STRING_DIM        512
#define SAFE_STRING_SIZE        32

#define DEFAULT_FOLDER_SEP  '/'


#define COPY_POINTER(__to_ptr, __from_ptr)   \
        if (__from_ptr) { \
            uint32_t __local_string_needed_size = strlen ((char*)__from_ptr)+1;\
            __to_ptr = (MDB_CHAR*)realloc ((void*)__to_ptr, __local_string_needed_size); \
            if (__to_ptr) {\
                strncpy ((char*)__to_ptr, ( const char * )__from_ptr, ( unsigned short )__local_string_needed_size); \
            } else {\
                __to_ptr=NULL;\
            }\
        } else { \
            MDB_CHAR *__old_ptr = (MDB_CHAR *)__to_ptr;\
            __to_ptr = NULL; \
            if (__old_ptr) {\
                free (__old_ptr); \
            }\
        }

#define COPY_ARRAY(to_array, __from_ptr)   \
                if (__from_ptr) {\
                        strcpy_s ((char*)to_array, sizeof(to_array), (const char*)__from_ptr);\
			} else {\
			to_array[0] = (char)0;\
			}


#define COPY_CARRAY(to_array, __from_ptr)   \
                if (strlen ((char*)__from_ptr) >= sizeof (to_array)) {\
			strncpy_s ((char*)to_array, sizeof(to_array), (const char*)__from_ptr, sizeof(to_array));\
			to_array[sizeof(to_array)-1] = (MDB_CHAR)0;\
			} else {\
			strcpy_s ((char*)to_array, sizeof(to_array), (const char*)__from_ptr);\
			}


#define ADD_ARRAY(to_array, __from_ptr)   \
                if (__from_ptr) {\
			if (strlen ((char*)to_array)+strlen ((char*)__from_ptr) >= sizeof (to_array)) {\
				uint32_t local_to_array_len = strlen ((char*)to_array);\
				uint32_t local_to_array_size = sizeof (to_array);\
				if (local_to_array_len+1 < local_to_array_size) {\
					uint32_t local_num_char_to_copy = local_to_array_size-local_to_array_len - 1;\
					memcpy((void*)&to_array[local_to_array_len], __from_ptr, local_num_char_to_copy);\
					to_array[local_to_array_len+local_num_char_to_copy] = 0;\
					}\
				} else {\
				strcat_s (to_array, sizeof(to_array), (char*)__from_ptr);\
				}\
			}


// Allocazione puntatore senza recupero dati esistenti
#define ALLOCATE_POINTER(ptr, length, allocated, cast)   \
                        if (length > allocated) {\
				ptr = (cast)realloc((void*)ptr, length+1);\
				if (ptr) {\
					allocated = length+1;\
					} else {\
					allocated = 0;\
					}\
				} else {\
				if (length) {\
					if (!ptr) {\
						ptr = (cast)malloc(length+1);\
						if (ptr) {\
							allocated = length+1;\
							} else {\
							allocated = 0;\
							}\
						}\
					}\
				}




// Allocazione puntatore con recupero dati esistenti
#define CHECK_ALLOCATE_POINTER(__ptr, __needed_items, __items_allocated, __cast) \
                        { void *__local_old_ptr = (void*)__ptr;\
				uint32_t __local_old_items_allocated = __items_allocated;\
				MDB_CHAR *__local_new_ptr = (MDB_CHAR *)NULL;\
				\
				if (__needed_items > __items_allocated) {\
					__local_new_ptr = (MDB_CHAR *)malloc((__needed_items)*sizeof(__ptr[0])+1);\
					if (__local_new_ptr) {\
						__items_allocated = __needed_items+1;\
						if (__local_old_ptr) {\
							memcpy ((void*)__local_new_ptr, (void*)__local_old_ptr, __local_old_items_allocated*sizeof(__ptr[0]));\
							if (__local_old_items_allocated > __items_allocated) {\
								memset ((void*)&__local_new_ptr[__local_old_items_allocated*sizeof(__ptr[0])], 0, (__items_allocated-__local_old_items_allocated)*sizeof(__ptr[0]));\
								}\
							} else {\
							memset ((void*)__local_new_ptr, 0, (__items_allocated)*sizeof(__ptr[0]));\
							}\
						} else {\
						__items_allocated = 0;\
						}\
					__ptr = (__cast)__local_new_ptr;\
					} else {\
					if (__needed_items) {\
						if (!__ptr) {\
							__local_new_ptr = (MDB_CHAR *)malloc((__needed_items)*sizeof(__ptr[0])+1);\
							if (__local_new_ptr) {\
								__items_allocated = __needed_items;\
								memset ((void*)__local_new_ptr, 0, (__items_allocated)*sizeof(__ptr[0]));\
								} else {\
								__items_allocated = 0;\
								}\
							__ptr = (__cast)__local_new_ptr;\
							}\
						}\
					}\
				if (__local_old_ptr) {\
					if (__local_old_ptr != __ptr) {\
						free (__local_old_ptr);\
						}\
					}\
				}



#define FREE_POINTER(__ptr)   \
                if (__ptr) {\
			void *__local_freeing_pointer = (void *)__ptr;\
			__ptr = NULL;\
			free (__local_freeing_pointer); \
                        }



    // Posizione lettore temparatura IR
    /*
    #define TEMPERATURE_READER_POS          48
    #define BLOW_MOLD_POS                   51
    #define NUM_PREFORM_REGISTRY            52
     */



	#define APPEND_CHAR_IF_NOT(__string, char_to_append, __string_size)   \
		if (__string) {\
			uint32_t LocalLen = strlen ((char*)__string);\
			if (LocalLen) {\
				if (LocalLen < __string_size) {\
					if (__string[LocalLen-1] != char_to_append) {\
						__string[LocalLen] = char_to_append;\
						__string[LocalLen+1] = 0;\
						}\
					}\
				} else {\
				__string[LocalLen] = char_to_append;\
				__string[LocalLen+1] = 0;\
				}\
			}



#define BIT1 1
#define BIT2 2
#define BIT3 4
#define BIT4 8
#define BIT5 16
#define BIT6 32
#define BIT7 64
#define BIT8 128
#define BIT9 256
#define BIT10 512
#define BIT11 1024
#define BIT12 2048
#define BIT13 4096
#define BIT14 8192
#define BIT15 16384
#define BIT16 32768
#define BIT17 65536
#define BIT18 131072
#define BIT19 262144
#define BIT20 524288
#define BIT21 1048576
#define BIT22 2097152
#define BIT23 4194304
#define BIT24 8388608
#define BIT25 16777216
#define BIT26 33554432
#define BIT27 67108864
#define BIT28 134217728
#define BIT29 268435456
#define BIT30 536870912
#define BIT31 1073741824
#define BIT32 2147483648



#define DPIGRECO 3.14159265354
#define PIGRECO 3.14159265354f
#define EPSILON	0.0000000001f


// Valore lingua
#define ENG_LANG 1
#define ITA_LANG 2


#define GET_POINTER_LEN(ptr, length)   \
                if (ptr) { \
                        length = strlen((char*)ptr); \
                        } else { \
                        length = 0; \
                        }

#define GET_POINTER_LEN_1B(ptr, length)   \
                if (ptr) { \
                        length = strlen((char*)ptr)+1; \
                        } else { \
                        length = 1; \
                        }
#define GET_POINTER_LEN_1BASED GET_POINTER_LEN_1B
#define GET_POINTER_LEN1B GET_POINTER_LEN_1B



// Definizione forni
#define MAX_OWENS               3
#define MAX_OWEN_ROWS           16

// Intervallo esecuzione (msec)
#define TASK_TIME                       5

// Valori notevoli sequenze
#define ALARM_RESETTED_SEQUENCE 0
#define INIT_SEQUENCE           1
#define START_SEQUENCE          10
#define RUN_SEQUENCE            20
#define WAIT_FOR_START_CYCLE    30
#define WAIT_FOR_RESUME_CYCLE   40
#define START_RAPID_MOVE        100
#define WAIT_RAPID_MOVE         110
#define DONE_RAPID_MOVE         120
#define START_MILL_MOVE        200
#define WAIT_MILL_MOVE         210
#define DONE_MILL_MOVE         220
#define DONE_MILL_WAIT         221
#define START_COOLER_I        300
#define WAIT_COOLER_I         310
#define DONE__COOLER_I         320
#define START_COOLER_II        400
#define WAIT_COOLER_II         410
#define DONE__COOLER_II         420
#define STOP_COOLER_I           400
#define STOP_COOLER_II          410
#define SET_AXIS_MAP            420

#define START_DRILL_CYCLE       426
#define POS_XY_DRILL_CYCLE      427
#define WAIT_POS_XY_DRILL_CYCLE 428
#define POS_Z_DRILL_CYCLE       429
#define WAIT_POS_Z_DRILL_CYCLE  430
#define RUNNING_DRILL_CYCLE     431
#define WATING_DRILL_CYCLE      432
#define MOVING_OUT_DRILL_CYCLE  433
#define WAITING_OUT_DRILL_CYCLE 434
#define MOVING_IN_DRILL_CYCLE   435
#define WAITING_IN_DRILL_CYCLE  436
#define WAIT_DONE_DRILL_CYCLE   437

#define START_BORE_CYCLE        440
#define RUNNING_BORE_CYCLE      441
#define WAITING_BORE_CYCLE      442
#define MOVING_OUT_BORE_CYCLE   443
#define SPINDLE_OFF_BORE_CYCLE  444
#define WAIT_SPINDLE_OFF        445
#define RAPID_OUT_BORE_CYCLE    446
#define WAITING_OUT_BORE_CYCLE  447
#define WAIT_BORE_CYCLE         448

#define SET_WAIT_SEQUENCE       970
#define WAIT_SEQUENCE           980
#define DONE_SEQUENCE           990
#define END_SEQUENCE            999

#define CLOCKWISE           2
#define COUNTERCLOCKWISE    3
#define CW                  2
#define CCW                 3


enum MACHINE_STATUS_ENUM {
    UNINITIALIZED = 0,      // NON Inizializzata
    PENDING_INIT,           // Attesa IO e perifariche
    INITIALIZED,            // Inizializzata, necessita ripristino
    RECOVERING,             // In fase di ripristino
    READY,                  // Ripristinata o a fine ciclo
    MANUAL,                 // Modalita manuale
    AUTOMATIC,              // Modalita automatica
    STEP_BY_STEP,           // Modalita passo/passo (debug)
    EMERGENCY,              // Modalita emergenza
    MAX_MACHINE_STATUS
};

enum STEP_ENUM {
    STEP_UNINITIALIZED = 0, // NON Inizializzata
    STEP_INITIALIZED,
    STEP_READY,
    STEP_SEND_CMD,
    STEP_MOVING,
    STEP_DONE,
    STEP_ERROR,
    STEP_STOPPED,
    STEP_SEND_HOMING,
    STEP_HOMING,
    MAX_STEP
};


enum ACTUATOR_PROTOCOL_ENUM {
    PROTOCOL_NONE = 0
    , ADC_INPUT = 10 // Valore del trasduttore
    , VARIABLE_ON_OFF = 20

    , BISTABLE_PN_VALVE = 50
    , MONOSTABLE_PN_VALVE = 51

    , AC_INVERTER = 100
    , ASYNC_MOTOTR = 110 // Motore asincrono trifase

    , VIRTUAL_AC_SERVO
    , MODBUS_AC_SERVO_LICHUAN = 200
    , MODBUS_AC_SERVO_DELTA
    , CANOPEN_AC_SERVO_DELTA

    , MAX_ACTUATOR_PROTOCOL
};

enum ERRORS_TYPE_ENUM {
    ALARM_ERR_NONE = 0,
    ALARM_LOG,
    ALARM_WARNING,
    ALARM_ERROR,
    ALARM_FATAL_ERROR,
    MAX_ALARM_ERROR
};



// enum POSITION_ENUM {
#define INSIDE          0
#define DOWN                    0
#define OPEN                    0
#define BACKWARD                0
#define BACK                    0
#define OFF                     0
#define ON                      1
#define OUTSIDE         1
#define UP                      1
#define CLOSE                   1
#define FORWARD         1
#define FRONT                   1
#define USER_POSITION           2
#define INTERPOLATE_POSITION    3
        

// Upside down
#define STRETCH_UP      0
#define STRETCH_DOWN    1

#define INDETERMINATE      -1
#define UNDERSTROKE      -10
#define OVERSTROKE      10


// Anticipi
#define NEAR_IN                 2
#define NEAR_DOWN                       2
#define NEAR_OPEN                       2
#define NEAR_BACKWARD           2
#define NEAR_OUT                        3
#define NEAR_UP                 3
#define NEAR_CLOSE                      3
#define NEAR_FORWARD            3
#define MAX_POSIZION    4






/*
___________________________________________________________________
    
TITOLO SEZIONE :        Definizione strutture dati
AUTORE :                        Cristian Andreon
DATA :                  6-5-2007
___________________________________________________________________                                                                                                                                                                                                                                                                                                            */

#ifndef BASE_STRUCTURES

#define BASE_STRUCTURES

// Struttura traccia attuattori

typedef struct tag_actuator_trace {
    short resolution_ms; // Millisecondi risoluzione

    uint32_t start_tick; // Tick iniziale
    uint32_t next_tick; // Prossimo tick
    uint32_t end_tick; // Tick finale

    float *torque; // Array valori torcente
    float *acc; // Array valori accelerazione
    float *speed; // Array valori velocit�
    float *pos; // Array valori posizione
    uint32_t num_data_allocated;
    uint32_t num_data;

    char Status;
    // 0 = init
    // 1 = started
    // 2 = running
    // 3 = done
    // 4 = read from ui
} ACTUATOR_TRACE, *LP_ACTUATOR_TRACE, **LPP_ACTUATOR_TRACE;




typedef int32_t(*lp_cmd_func)(void);


// Struttura attuattori

typedef struct tag_actuator {
    int32_t Id;
    char *name;

    // Indirizzi uscita
    uint32_t out_address[2];
    uint32_t in_address[2];

    // Nomi posizioni notevoli
    char *pos_name[4];

    int32_t version;

    int32_t options;

    int32_t position;
    int32_t target_position;
    int32_t positionReached; // RUNTIME : indice la posizione raggiunta con almeno una lettura dal driver

    float cur_rpos;         // posizione corrente
    float start_rpos;       // posizione minima
    float end_rpos;         // psoizione massima
    float target_rpos;      // Posizione utente
    
    float end_rpos_toll;
    float start_rpos_toll;
    float dist;

    uint32_t readCounter; // conteggio letture nell'ambito del movimento
    uint32_t readPositionCounter; // conteggio letture posizione nell'ambito del movimento
    
    
    uint32_t CANSeqID; // Progressivo pacchetto CAN

    
    float cam_ratio;            // Rapporto encoder (pulses) / posizione (unità misura utente)
    float workingOffset;        // Offset nella fase di lavoro (zero intermedio)
    uint32_t pulsesPerTurn;     // Numero inpulsi al giro per il calcolo ella poszione dell'albero (1 = 1giro)
    uint32_t pulsesOverflow;    // Numero di Pulsazioni in un giro conteggiate dal driver (es. rotazione a 16bit)
    
    float cur_vpos;             // Posizione virtuale 0...1000.0



    // Gestioni enticipi (per assi)
    float near_start_rposition;
    float near_end_rposition;

    float speed_lin;    // Velocità lineare
    float speed;
    float max_speed;
    float min_speed;

    float torque;
    float rated_torque;
    float max_torque;
    float min_torque;
    
    float temp;

    float speed_auto1;
    float speed_lin_auto1;
    float acc_auto1;
    float dec_auto1;
    float force_auto1;

    float speed_auto2;
    float speed_lin_auto2;
    float acc_auto2;
    float dec_auto2;
    float force_auto2;

    float speed_auto3;
    float speed_lin_auto3;

    float speed_man1;
    float acc_man1;
    float dec_man1;
    float force_man1;

    float speed_man2;
    float acc_man2;
    float dec_man2;
    float force_man2;

    // Gestione sezioni temporali
    uint32_t start_time, // istante partenza (tick msec)
    start_time1, // istante inizio movimento
    start_time2, // istante fine movimento
    start_time3; // // istante attivazione sensore

    uint32_t timeout1_ms; // (msec timeout)
    uint32_t timewarn1_ms; // (msec warning)
    uint32_t timeout2_ms; // (msec timeout)
    uint32_t timewarn2_ms; // (msec warning)
    
    uint32_t timeout3_ms;   // Timeout il movimento a posizione utente (Runtime)
    uint32_t timeout4_ms;   // Timeout per l'interpolazione  (Runtime)

    uint32_t time_ms1; // tempo movimento msec
    uint32_t time_ms11; // tempo inizio movimento msec
    uint32_t time_ms12; // tempo corpo movimento msec
    uint32_t time_ms13; // tempo coda movimento msec

    uint32_t time_ms2; // tempo movimento msec
    uint32_t time_ms21; // tempo inizio movimento msec
    uint32_t time_ms22; // tempo corpo movimento msec
    uint32_t time_ms23; // tempo coda movimento msec

    int32_t rtOptions;
    // BT1 ->   waring 1 già emesso
    // BT2 ->   waring 2 già emesso

    LP_ACTUATOR_TRACE pTrace;


    enum STEP_ENUM step;

    enum ACTUATOR_PROTOCOL_ENUM protocol; // Protocollo gestione

    int32_t boardId, stationId;
    int32_t IOindex1, IOindex2;
    
    void *pUSBSlot;
    void *pCANSlot;
    void *pSerialSlot;
    void *pIOSlot;

    // Uscite ausiliarie (tipo pilotaggio AC servo) (link al sensore)
    int32_t auxSCRId;
    int32_t auxBoardId;
    int32_t auxDO1, auxDO2;
    int32_t auxI2C;
    
    // codice errore
    int32_t error;
    
    // Conteggio errori lettura fuori range
    int32_t outStrokeError;

    // Controlla il timeout sulla comunicazione durante il movimento
    uint8_t CheckFeedbackTimeout;
    
    // Risposta dal driver
    int32_t driverStatus;
        // BIT1 ->  Comando avvio ricevuto (Status Word)
        // BIT2 ->  Comando posizione arrivo ricevuto
        // BIT3 ->  Comando velocita ricevuto
        // BIT4 ->  Comando accelerazione ricevuto
        // BIT5 ->  Comando decelerazione ricevuto
    
        // BIT11 ->  Comando avvio letto dal driver (Status Word)
        // BIT12 ->  Comando posizione arrivo letto dal driver
        // BIT13 ->  Comando velocita letto dal driver
        // BIT14 ->  Comando accelerazione letto dal driver
        // BIT15 ->  Comando decelerazione letto dal driver
    char driverStatusDesc[64];
    
    // errore inseguimento
    float follow_error1, follow_error2;
    float cur_follow_error1, cur_follow_error2;

    int32_t IDLECounter;
    int32_t IDLESeq;
    
    // Posizione inviata al driver
    uint32_t lastTargetPos;
    
    int32_t driverTemp;
    int32_t driverMotorTemp;
    int32_t driverPosition;
    int32_t driverTurns;
    int32_t driverPulses;
    int32_t driverSpeed;
    int32_t driverAcc;
    uint32_t driverLastTime;
    uint32_t driverSpeedCount;
    uint32_t driverPosCount;
            
    int32_t homingTorqueMode;
    int32_t homingRequest;
    int32_t homingDone;
    int32_t homingPulsesPPT;    // Rotazione in PulsePerTurn
    int32_t homingTurnsPPT;     // Rotazione in PulsePerTurn
    int32_t homingSeq;
    uint32_t homingStartTime;
    uint32_t homingLastSendReqTime;
    uint32_t homingZeroCounter;
    
    float homing_offset_mm;
    float homing_speed_rpm;
    float homing_rated_torque;
    uint32_t homing_timeout_ms;
    int32_t homing_position;
    int32_t homingBoardId;
    int32_t homingDI;
    int32_t homingDIvalue;
            
    bool disabled;
    
#ifdef SIMULATE_MODE
    int32_t PosX, PosY;
#endif

    lp_cmd_func close_func;
    lp_cmd_func open_func;
    lp_cmd_func home_func;
    lp_cmd_func jog_plus_func;
    lp_cmd_func jog_minus_func;

    void *actuator_mirror;

    
    // Tipo Interpolazione : Seno / Coseno / Lineare
    uint8_t AxisSinCosLin;
    
} ACTUATOR, *LP_ACTUATOR, **LPP_ACTUATOR;


// macro / utilities
#define MAX_ACT_POS(__i) (machine.actuator[__i].end_rpos-machine.actuator[__i].start_rpos)

// determina se l'attuatore è collegato ad una scheda
#define IS_ACTUATOR_LINKED(__i) (machine.actuator[__i].pUSBSlot || machine.actuator[__i].pCANSlot || machine.actuator[__i].pSerialSlot || machine.actuator[__i].pIOSlot)






// Struttura comani manuali

typedef struct tag_errors {
    int32_t id;

    int32_t Code;

    char *Desc;

    char *DateTime;

    int32_t actuatorId;

    enum ERRORS_TYPE_ENUM Type;

} ALARM, *LP_ALARM, **LPP_ALARM;






#define MAX_PREFORM_REGISTRY    256

typedef struct tag_IOBoardSlot {
    
    int32_t id;
    uint8_t *digitalIN;
    uint32_t numDigitalIN;

    uint32_t *analogIN;
    uint32_t numAnalogIN;


    uint8_t *digitalOUT;
    uint8_t *digitalOUTBK;
    uint32_t numDigitalOUT;

    uint32_t *analogOUT;
    uint32_t *analogOUTBK;
    uint32_t numAnalogOUT;

    int32_t i2cValues[255];
    uint32_t i2cNumValues;
    
    uint8_t IODataOut[256];
    uint32_t NumIODataOut;
    
    uint8_t disabled;
            
} IOBoardSlot;



typedef struct tag_SerialSlot {
    void *modbus_ctx;

    char *device;
    int32_t baud;
    char parity;
    int32_t data_bit;
    int32_t stop_bit;

    int32_t boardId; // Id della scheda
    int32_t stationId; // Id della stazione

    void *pActuator; // Linl all'attuatore che riceve il dato

    uint16_t data[64];
    int32_t dataOffset;

    int32_t driverPosition, driverTurns, driverPulses;
    int32_t driverSpeed;
    int32_t driverAcc;

    int32_t testCommand;
    int32_t pendingCommand; // Comando in attesa di esecuzione
    int32_t runningCommand; // Comando in esecuzione
    int32_t doneCommand; // Comando eseguito
    int32_t stopRequest; // richiesta di stop
    int32_t setupRequest; // richiesta di setup
    int32_t homingRequest;  // richiesta di homing
    int32_t resetRequest; // richiesta di reset
    
    int32_t setupDone;
    
    
    

    int32_t state;

    uint32_t start_time;
    uint32_t tStat;
    uint32_t preTimeout;

    int32_t messageState;
    int32_t timeoutCount;
    int32_t readCount;
    int32_t streamErrorCount;
    int32_t waitingRespSize;

    int32_t controlMode1B;
    
    uint32_t ReadSinglePos;

#define SPEED_1 0
#define SPEED_2 1
#define ACC_1   2
#define ACC_2   3
#define F_ERR1  4
#define F_ERR2  5

    char paramToUpdate[32];
    
    uint8_t disabled;

} SerialSlot, *LP_SerialSlot;




    #define MAX_CANBUS_ACTUATORS    32

typedef struct tag_CANSlot {
    
    void *can_ctx;
    
    uint32_t tickTocIOWatchDog;

    char device[32];
    uint8_t mac[6], ip[4];
    int16_t port;
    int16_t Kbps;
    
    int32_t boardId; // Id della scheda
    int32_t stationId; // Id del Driver

    void *pActuators[MAX_CANBUS_ACTUATORS]; // Linl all'attuatore che riceve il dato
    uint32_t nActuators;

    char *data;
    int32_t data_size;
    int32_t data_available;


    int32_t pendingCommand; // Comando in attesa di esecuzione
    int32_t runningCommand; // Comando in esecuzione
    int32_t doneCommand; // Comando eseguito
    int32_t stopRequest; // richiesta di stop
    int32_t setupRequest; // richiesta di setup
    int32_t homingRequest; // richoesta di homing
    int32_t resetRequest; // richiesta di reset
    
    int32_t setupDone;

    int32_t state;

    uint32_t t1;
    uint32_t start_time;
    uint32_t tStat;
    uint32_t preTimeout;

    int32_t messageState;
    int32_t timeoutCount;
    int32_t readCount;
    int32_t readPDOCount;
    int32_t statReadCount;
    int32_t streamErrorCount;
    int32_t waitingRespSize;

    int32_t controlMode1B;

#define SPEED_1 0
#define SPEED_2 1
#define ACC_1   2
#define ACC_2   3

    char paramToUpdate[32];
    
    BOOL StreamingMode;
    uint32_t ReadSinglePos;
    
    // Posizione passata al canbus
    uint32_t lastTargetPos;
    
    // Timeout Feedback fullCMD format
    uint32_t fullCMDFeedbackStartTime;
    uint8_t fullCMDFeedbackError;
    
    // Backup comando inviato
#define FULL_START_CMD_SIZE 64
    uint8_t fullStartCMD[FULL_START_CMD_SIZE];
    uint32_t nfullStartCMD;
    
    // Intervallo minimp esecuzione
    uint8_t si_ms, ri_ms;


    // Dati Interpolazione    
    float StartAngleRad, EndAngleRad;   // Angolo iniziale e finale
    float RadiusMM;                       // Raggio interpolazione circolare

    uint8_t Direction;                  // 0 = CW orario, 1 = CCW antiorario
    float FeedMMMin;                  // Avanzamento in mm/sec
    float PrecisionMM;                  // Precisione in mm (corda massima)
    int32_t PrecisionNSteps;            // Precisione in numero di suddivisioni
    uint32_t PeriodMsec;                 // Periodo in msec
    
    
    uint8_t disabled;
    
} CANSlot, *LP_CANSlot;





typedef struct tag_USBSlot {
    void *usb_ctx;

    char device[32], device_addr[32];

    
    int32_t boardId; // Id della scheda
    int32_t stationId; // Id del Driver

    void *pActuator; // Linl all'attuatore che riceve il dato

    char *data;
    int32_t data_size;

    int32_t driverPosition;
    int32_t driverSpeed;
    int32_t driverAcc;

    int32_t pendingCommand; // Comando in attesa di esecuzione
    int32_t runningCommand; // Comando in esecuzione
    int32_t doneCommand; // Comando eseguito
    int32_t stopRequest; // richiesta di stop
    int32_t setupRequest; // richioesta di setup

    int32_t state;

    uint32_t start_time;
    uint32_t tStat;

    int32_t messageState;
    int32_t timeoutCount;
    int32_t readCount;
    int32_t streamErrorCount;
    int32_t waitingRespSize;

    int32_t controlMode1B;

#define SPEED_1 0
#define SPEED_2 1
#define ACC_1   2
#define ACC_2   3
#define FERR_1   4
#define FERR_2   5

    char paramToUpdate[32];

    uint8_t disabled;
    
} USBSlot, *LP_USBSlot;







// Struttura gestione macchina

typedef struct tag_machine {
    char prgVer;

    int32_t options;

    // Id utente autenticato
    int32_t user_id;


    // Schede di IO
    IOBoardSlot *ioBoardSlots;
    uint32_t numIOBoardSlots, numIOBoardSlotsAllocated;

    // Shede Seriali
    SerialSlot *serialSlots;
    uint32_t numSerialSlots, numSerialSlotsAllocated;

    // Shede CANOpen
    CANSlot *CANSlots;
    uint32_t numCANSlots, numCANSlotsAllocated;

    // Shede USB
    USBSlot *USBSlots;
    uint32_t numUSBSlots, numUSBSlotsAllocated;

    

    // Attuattori
    LP_ACTUATOR actuator;
    uint32_t num_actuator;


    uint32_t manual_move;
    BOOL should_recover;







    // Parametri lavoro
    WORK_SET workSet;


    // Parametri Macchina
    SETTINGS settings;




    // Statistiche globali (su ssam)
    STATISTIC statistic;
    STATISTIC rt_statistic;



    // stato macchina
    enum MACHINE_STATUS_ENUM status;

    MDB_CHAR *status_message; // [256];
    uint32_t status_message_size;

    MDB_CHAR *time_message; // [64];
    uint32_t time_message_size;



    // Azione un volta al ciclo macchina (azioni correttive)
    char once_actions[32];



    // Sequenza corrente macchina
    uint32_t sequence;

    
    
    

    // Riciesta reset allarmi
    int32_t reset_alarms_request;

    // Gestione Emergenza
    int32_t emergency_request;

    // Gestione potenza
    int32_t power_on_request;



    // Selettore 1=Automatico/0=Manuale
    int32_t machine_mode_request;

    // Richiesta start ciclo
    int32_t start_request;
    
    // Richiesta simulazione ciclo
    int32_t simulate_request;

    // Richiesta fine ciclo
    int32_t stop_request;



    // Timeout Comunicazione Interfaccia Utente
    uint32_t ui_timeout_ms;
    uint32_t ui_timeout_count;

    uint32_t io_timeout_ms;
    uint32_t io_timeout_count;

    uint32_t scr_timeout_ms;
    uint32_t scr_timeout_count;


    // Stringhe composte per il debug e la messa in cache
    MDB_CHAR *debug_string[4];


    LP_ALARM alarmList;
    uint32_t numAlarmList;
    uint32_t numAlarmListAllocated;
    uint32_t curAlarmList;
    uint32_t lastAlarmListId;

    uint32_t lastAlarmId;
    int32_t rebuildAlarmList;
    
    int32_t task_cycles;
    
    bool start_after_recovery;


    // Struttura dipendente dall' applicazione    
    APP_SPECIFIC App;

#ifdef SIMULATE_MODE

    BOOL simulate_pause;

    HFONT hfont, hsmallfont;

    HANDLE thread_handle;
    int32_t thread_id;

    HWND hwnd;

#endif

} MACHINE, *LP_MACHINE, **LPP_MACHINE;



#endif

typedef struct tag_users {
    int32_t user_id;
    char user_name[256];
    char password[256];
    char token[256];

    int32_t level;  // 0 = guest, 1 = user, 2 = admin

} USERS, *LP_USERS, **LPP_USERS;


#endif

