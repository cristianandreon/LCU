#ifndef __LOGIC_PRECOMP

    #ifdef PLC_SIMULATOR
        #include <windows.h>
    #endif


    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stddef.h>
    #include <stdint.h>

    #include "./app_define.h"

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
    

    /*
    __LOGIC_PRECOMP char *ACTUATORS_NAME_LIST[256];
    __LOGIC_PRECOMP char *ACTUATORS_IN_POSITION_NAME_LIST[256];
    __LOGIC_PRECOMP char *ACTUATORS_OUT_POSITION_NAME_LIST[256];
    __LOGIC_PRECOMP char *ERRORS_LIST[512];
    __LOGIC_PRECOMP char *WARNINGS_LIST[512];
    __LOGIC_PRECOMP char *MANUAL_COMMANDS_LIST[256];
    __LOGIC_PRECOMP char *USER_INTERFACE_CONTROL_LIST[256];
    __LOGIC_PRECOMP char *QUICK_TIPS_LIST[256];
    */
    
    
    __LOGIC_PRECOMP void PUT_AUTOMATIC();
    __LOGIC_PRECOMP void PUT_MANUAL();
    __LOGIC_PRECOMP void PUT_POWER_ON();
    __LOGIC_PRECOMP void PUT_POWER_OFF();
    __LOGIC_PRECOMP void PUT_LOAD_ON();
    __LOGIC_PRECOMP void PUT_LOAD_OFF();
    __LOGIC_PRECOMP void PUT_SINGLE_LOAD_ON();
    __LOGIC_PRECOMP void PUT_SINGLE_LOAD_OFF();
    __LOGIC_PRECOMP void PUT_HEAT_ON();
    __LOGIC_PRECOMP void PUT_HEAT_OFF();
    __LOGIC_PRECOMP void PUT_BLOW_ON();
    __LOGIC_PRECOMP void PUT_BLOW_OFF();
    __LOGIC_PRECOMP void PUT_INLINE_OUTPUT_ON();
    __LOGIC_PRECOMP void PUT_INLINE_OUTPUT_OFF();
    
    __LOGIC_PRECOMP void PUT_START_CYCLE();
    __LOGIC_PRECOMP void PUT_STOP_CYCLE();
    __LOGIC_PRECOMP void PUT_EMERGENCY();
    __LOGIC_PRECOMP void PUT_RESET_ALARMS();
    
    __LOGIC_PRECOMP int IS_RESET_ALARMS_REQUEST ( void );
    __LOGIC_PRECOMP int IS_IN_EMERGENCY_REQUEST ( void );
    __LOGIC_PRECOMP int IS_IN_AUTOMATIC_REQUEST ( void );
    __LOGIC_PRECOMP int IS_IN_MANUAL_REQUEST( void );
    __LOGIC_PRECOMP int IS_STOP_CYCLE_REQUESTED ( void );
    __LOGIC_PRECOMP int IS_START_CYCLE_REQUESTED ( void );
    
    __LOGIC_PRECOMP int on_machine_initialized ( void );

    __LOGIC_PRECOMP int logic_init ( void );
    __LOGIC_PRECOMP int logic_post_init ( void );
    __LOGIC_PRECOMP int logic_loop ( int Mode );
    
    
    __LOGIC_PRECOMP int do_preform_registry_shift ( void );
    __LOGIC_PRECOMP int check_dirty_preform_registry ( void );
    __LOGIC_PRECOMP int set_preform_registry_as_damaged ( void );        
    __LOGIC_PRECOMP int simulate_actuators_move ( void );
    __LOGIC_PRECOMP int read_inputs ( void );
    __LOGIC_PRECOMP int write_outputs ( void );
    __LOGIC_PRECOMP int check_actuators_status ( void );
    __LOGIC_PRECOMP int check_timeouts ( void );
    __LOGIC_PRECOMP int manual ( void );
    __LOGIC_PRECOMP int emergency();
    __LOGIC_PRECOMP int recover();
    __LOGIC_PRECOMP int automatic( uint32_t *pSequence, void *pvCmd );
    __LOGIC_PRECOMP int check_manual_cmd_status ();    
    __LOGIC_PRECOMP int close_primary_air();
    __LOGIC_PRECOMP int close_secondary_air();
    __LOGIC_PRECOMP int open_discharge_air();
    __LOGIC_PRECOMP int stop_stretch_rod();
    __LOGIC_PRECOMP int stop_trasferitor();
    __LOGIC_PRECOMP int pick_preform_manipolator();
    __LOGIC_PRECOMP int read_preform_temperature();
    __LOGIC_PRECOMP int step_heating_chain();
    __LOGIC_PRECOMP int front_preform_manipolator();
    __LOGIC_PRECOMP int unpick_preform_manipolator();
    __LOGIC_PRECOMP int down_preform_manipolator();
    __LOGIC_PRECOMP int back_preform_manipolator();
    __LOGIC_PRECOMP int back_heating_chain();
    __LOGIC_PRECOMP int up_preform_manipolator();
    
    __LOGIC_PRECOMP int outside_transferitor();
    __LOGIC_PRECOMP int inside_transferitor();
    
    __LOGIC_PRECOMP int home_transferitor();
    __LOGIC_PRECOMP int back_transferitor();
    __LOGIC_PRECOMP int front_transferitor();
    __LOGIC_PRECOMP int stop_transferitor();

    __LOGIC_PRECOMP int open_secondary_air();
    __LOGIC_PRECOMP int open_primary_air ();
    __LOGIC_PRECOMP int check_premature_cicle_end(int32_t, int32_t);
    __LOGIC_PRECOMP int close_discharge_air();
    __LOGIC_PRECOMP int open_recovery_air();
    __LOGIC_PRECOMP int close_recovery_air();
    
    __LOGIC_PRECOMP int home_mold();
    __LOGIC_PRECOMP int open_mold();
    __LOGIC_PRECOMP int close_mold();
    __LOGIC_PRECOMP int stop_mold();
    
    __LOGIC_PRECOMP int home_stretch_rod();
    __LOGIC_PRECOMP int down_stretch_rod();
    __LOGIC_PRECOMP int up_stretch_rod();
    __LOGIC_PRECOMP int stop_stretch_rod();

    __LOGIC_PRECOMP int up_holding_station();
    __LOGIC_PRECOMP int down_holding_station();
    
    __LOGIC_PRECOMP int recompute_prod(int32_t, int32_t, uint32_t);
    
    __LOGIC_PRECOMP int put_machine_in_manual_mode();
    
    __LOGIC_PRECOMP char *login_user ( char *user, char *password, char *token );
    
#ifdef __cplusplus
        // }
#endif

#endif
