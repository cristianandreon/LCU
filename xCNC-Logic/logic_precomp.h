#ifndef __LOGIC_PRECOMP

    #ifdef PLC_SIMULATOR
        #include <windows.h>
    #endif


    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stddef.h>
    #include <stdint.h>

    #include "app_define.h"
    #include "gcode.h"

    #include "./../LOGIC/constant.h"
    #include "./../LOGIC/alarms.h"
    #include "./../LOGIC/track.h"



    #ifdef __cplusplus
        #if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )
            extern "C" {
        #endif
    #endif



    #ifdef EXTERN
        #ifdef __cplusplus
            #if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )
                #define __LOGIC_PRECOMP extern
            #else
                #define __LOGIC_PRECOMP  extern "C"
            #endif
        #else
            #define __LOGIC_PRECOMP extern
        #endif
    #else
        #define __LOGIC_PRECOMP 
    #endif

                
                                                                                                                                                                                                                                                                                                                                                                                    /*
    ______________________________________________________________________________________
    
            TITOLO SEZIONE :        Definizione variabili globali
            AUTORE :                        Cristian Andreon
            DATA :                  6-5-2007
    ______________________________________________________________________________________                                                                                                                                                                                                                                                                                                            */
    
                
       
#ifdef __cplusplus
    // extern "C" {
#endif
                
    
    // Struttura principale
                
    
    // Struttura principale
    __LOGIC_PRECOMP MACHINE machine;
    

    
#define MAX_USERS   8
    
    __LOGIC_PRECOMP USERS GLUsers[MAX_USERS];
    __LOGIC_PRECOMP uint32_t GLNumUsers;
    __LOGIC_PRECOMP uint32_t GLLoggedUser1B;
    
    
    
    // Contatore tempo corrente
    __LOGIC_PRECOMP uint32_t GLCurTimeMs;
    
    __LOGIC_PRECOMP uint32_t REAL_TASK_TIME;
    
    __LOGIC_PRECOMP uint32_t GLLastCurrentTime;
    
                                                                                                                                                                                                                                                                                                                                                                                    /*
    _________________________________________________________________
    
            TITOLO SEZIONE :        Definizione stringhe globali
            AUTORE :                        Cristian Andreon
            DATA :                  6-5-2007
    _________________________________________________________________                                                                                                                                                                                                                                                                                                            */
    

    
    
    __LOGIC_PRECOMP void PUT_AUTOMATIC();
    __LOGIC_PRECOMP void PUT_MANUAL();
    __LOGIC_PRECOMP void PUT_POWER_ON();
    __LOGIC_PRECOMP void PUT_POWER_OFF();
    
    __LOGIC_PRECOMP void PUT_START_CYCLE();
    __LOGIC_PRECOMP void PUT_STOP_CYCLE();
    __LOGIC_PRECOMP void PUT_SIMULATE_CYCLE();
    __LOGIC_PRECOMP void PUT_EMERGENCY();
    __LOGIC_PRECOMP void PUT_RESET_ALARMS();
    
    __LOGIC_PRECOMP int IS_RESET_ALARMS_REQUEST ( void );
    __LOGIC_PRECOMP int IS_IN_EMERGENCY_REQUEST ( void );
    __LOGIC_PRECOMP int IS_IN_AUTOMATIC_REQUEST ( void );
    __LOGIC_PRECOMP int IS_IN_MANUAL_REQUEST( void );
    __LOGIC_PRECOMP int IS_STOP_CYCLE_REQUESTED ( void );
    __LOGIC_PRECOMP int IS_START_CYCLE_REQUESTED ( void );
    __LOGIC_PRECOMP int IS_SIMULATE_CYCLE_REQUESTED ( void );
    
    __LOGIC_PRECOMP int on_machine_initialized ( void );

    __LOGIC_PRECOMP int logic_init ( void );
    __LOGIC_PRECOMP int logic_post_init ( void );
    __LOGIC_PRECOMP int logic_loop ( int Mode );
    
    
    __LOGIC_PRECOMP int home_X();
    __LOGIC_PRECOMP int home_Y();
    __LOGIC_PRECOMP int home_Z();
    __LOGIC_PRECOMP int jog_plus_X();
    __LOGIC_PRECOMP int jog_minus_X();
    __LOGIC_PRECOMP int jog_plus_Y();
    __LOGIC_PRECOMP int jog_minus_Y();
    __LOGIC_PRECOMP int jog_plus_Z();
    __LOGIC_PRECOMP int jog_minus_Z();
    __LOGIC_PRECOMP int start_spindle(float position, float speed_rpm , int direction);
    __LOGIC_PRECOMP int stop_spindle();
    __LOGIC_PRECOMP int start_cooler_I();
    __LOGIC_PRECOMP int stop_cooler_I();
    __LOGIC_PRECOMP int start_cooler_II();
    __LOGIC_PRECOMP int stop_cooler_II();
    __LOGIC_PRECOMP int stop_X();
    __LOGIC_PRECOMP int stop_Y();
    __LOGIC_PRECOMP int stop_Z();
    __LOGIC_PRECOMP int stop_W();
    __LOGIC_PRECOMP int stop_T();
 

    __LOGIC_PRECOMP int automatic ( uint32_t *pSequence, void *pvGcodeCmd );
    __LOGIC_PRECOMP int recover();
    __LOGIC_PRECOMP int simulate_actuators_move();

    __LOGIC_PRECOMP int emergency();
    __LOGIC_PRECOMP int check_timeouts();
    __LOGIC_PRECOMP int check_actuators_status();
    __LOGIC_PRECOMP int manual(void);

    __LOGIC_PRECOMP int put_machine_in_manual_mode();
    __LOGIC_PRECOMP int check_manual_cmd_status();
    
    __LOGIC_PRECOMP char *login_user ( char *user, char *password, char *token );
    
#ifdef __cplusplus
        // }
#endif

#endif
