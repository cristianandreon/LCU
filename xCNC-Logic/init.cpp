#define EXTERN


/////////////////////////
// RT Kernel includes
//
#ifdef WATCOM
#include "FreeRTOS.h"
#include "task.h"
#else
#include "./../RTLinux/RTLinux.h"
#endif


#include "./logic_precomp.h"


#include <exception>



// #define DEBUF_PRINTF



/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inizializza la logica della macchina (Eseguita prima della lettura delle impostazioni persistenti
//

int logic_init ( void ) {
    uint32_t i;


    try {

        GLLastCurrentTime = 0;
        GLCurTimeMs = 0;


        // Dal modulo FreeRTOS
        REAL_TASK_TIME = portTICK_RATE_MS;


        ////////////////////////////////////
        // Inizializzazione Utenti
        //
        memset (GLUsers, 0, sizeof(GLUsers));

        {   uint8_t *pass;

            GLNumUsers = 0;
            strcpy (GLUsers[GLNumUsers].user_name, (char*)"Admin");
            pass = (uint8_t*)"Admin73";
            sha1(pass, (size_t)strlen((char*)pass), (uint8_t*)GLUsers[GLNumUsers].password);
            GLUsers[GLNumUsers].user_id = 1;
            GLUsers[GLNumUsers].level = 2;
            GLNumUsers++;

            strcpy (GLUsers[GLNumUsers].user_name, (char*)"Operator");
            pass = (uint8_t*)"1234";
            sha1(pass, (size_t)strlen((char*)pass), (uint8_t*)GLUsers[GLNumUsers].password);
            GLUsers[GLNumUsers].user_id = 2;
            GLUsers[GLNumUsers].level = 1;
            GLNumUsers++;

            strcpy (GLUsers[GLNumUsers].user_name, (char*)"Guest");
            pass = (uint8_t*)"";
            sha1(pass, (size_t)strlen((char*)pass), (uint8_t*)GLUsers[GLNumUsers].password);
            GLUsers[GLNumUsers].user_id = 3;
            GLUsers[GLNumUsers].level = 0;
            GLNumUsers++;

            GLLoggedUser1B = 0;
           }


        memset (&machine, 0, sizeof(machine));

        machine.prgVer = 73;

        machine.status = PENDING_INIT;


        ////////////////////////////////
        // Righe visualizzabile GCode
        //
        machine.App.GCode.numDisplayRows = 64;
        if (!gcode_init_display_rows ( machine.App.GCode.numDisplayRows )) {
            return -1;
        }        

        
        // machine.App.toll = 0.005f;
        machine.App.toll = 0.010f;
        
        machine.App.Epsilon = 0.0001f;
        machine.App.dEpsilon = 0.0001;

        machine.App.AfterMillWaitTimeMS = 500;

        
        machine.settings.TaylorC = 250.0f;
        machine.settings.TaylorN = 0.122f;

        // Precisione della lavorazione (interpolazione)
        machine.settings.InterpolationPrecisionMM = 0.10f;
        
        // Periodo Interpolazione
        machine.settings.InterpolationPeriodMS = 5;
        

        
        // N.B.: L'interpolazione richiede un tempo di risposta più lungp
        App.CanBusFullCmdFeedbackTimeoutMS = 500;
        
        
        
        //////////////////////
        // Schede di IO
        //

        // Numero di Shede di IO
        machine.numIOBoardSlots = (unsigned int)0;

        machine.numIOBoardSlotsAllocated = 6;
        machine.ioBoardSlots = (IOBoardSlot *)calloc(sizeof(IOBoardSlot) * machine.numIOBoardSlotsAllocated +1, 1);

        if (!machine.ioBoardSlots) {
            machine.numIOBoardSlotsAllocated = 0;
            return -1;
        } else {
            for (i=0; i<machine.numIOBoardSlotsAllocated; i++) {
                machine.ioBoardSlots[i].id = i+1;
                machine.ioBoardSlots[i].numDigitalIN = 20;
                machine.ioBoardSlots[i].digitalIN = (uint8_t*)calloc(sizeof(machine.ioBoardSlots[0].digitalIN[0]) * machine.ioBoardSlots[i].numDigitalIN, 1);
                if (!machine.ioBoardSlots[i].digitalIN) {
                    return -10;
                }

                machine.ioBoardSlots[i].numDigitalOUT = 12;
                machine.ioBoardSlots[i].digitalOUT = (uint8_t*)calloc(sizeof(machine.ioBoardSlots[0].digitalOUT[0]) * machine.ioBoardSlots[i].numDigitalOUT, 1);
                if (!machine.ioBoardSlots[i].digitalOUT) {
                    return -11;
                }
                machine.ioBoardSlots[i].digitalOUTBK = (uint8_t*)calloc(sizeof(machine.ioBoardSlots[0].digitalOUTBK[0]) * machine.ioBoardSlots[i].numDigitalOUT, 1);
                if (!machine.ioBoardSlots[i].digitalOUTBK) {
                    return -12;
                }

                machine.ioBoardSlots[i].numAnalogIN = 12;
                machine.ioBoardSlots[i].analogIN = (unsigned int *)calloc(sizeof(machine.ioBoardSlots[0].analogIN[0]) * machine.ioBoardSlots[i].numAnalogIN, 1);
                if (!machine.ioBoardSlots[i].analogIN) {
                    return -13;
                }

                machine.ioBoardSlots[i].numAnalogOUT = 12;
                machine.ioBoardSlots[i].analogOUT = (unsigned int *)calloc(sizeof(machine.ioBoardSlots[0].analogOUT[0]) * machine.ioBoardSlots[i].numAnalogOUT, 1);
                if (!machine.ioBoardSlots[i].analogOUT) {
                    return -14;
                }
                machine.ioBoardSlots[i].analogOUTBK = (unsigned int *)calloc(sizeof(machine.ioBoardSlots[0].analogOUTBK[0]) * machine.ioBoardSlots[i].numAnalogOUT, 1);
                if (!machine.ioBoardSlots[i].analogOUTBK) {
                    return -15;
                }

            }
        }





        // Timeout Comunicazione IO
        machine.io_timeout_ms = 250;
        machine.io_timeout_count = 0;

        // Timeout Comunicazione SCR
        machine.scr_timeout_ms = 5000;
        machine.scr_timeout_count = 0;


        // Timeout Comunicazione UI
        machine.ui_timeout_ms = 10000;
        machine.ui_timeout_count = 0;




        #ifdef SIMULATE_MODE                        
            create_main_font ( &machine.hfont, &machine.hsmallfont, GetFocus(), 0+0 );
            #endif


        machine.status_message_size = 1024;
        machine.status_message = (char*)calloc(machine.status_message_size, 1);
        if (!machine.status_message) {
            return -20;
        }

        machine.time_message_size = 1024;
        machine.time_message = (char*)calloc(machine.time_message_size, 1);
        if (!machine.time_message) {
            return -21;
        }





        // Assegnamnto dati attuatori
        machine.num_actuator = MAX_ACTUATOR_NAME;

        machine.actuator = (LP_ACTUATOR)calloc(sizeof(machine.actuator[0]), machine.num_actuator+1);



        for (i=0; i<machine.num_actuator; i++) {
            machine.actuator[i].Id = i+1;
            machine.actuator[i].position = INDETERMINATE;
            machine.actuator[i].cur_rpos = 500.0f;
            machine.actuator[i].start_rpos = 0.0f;
            machine.actuator[i].end_rpos = 1000.0f;
            machine.actuator[i].cur_vpos = 500.0f;
            
            // Tolleranza posizionamento asse
            // 0.01 causa il timeout..
            machine.actuator[i].end_rpos_toll = 0.03f;
            machine.actuator[i].start_rpos_toll = 0.03f;
            
            machine.actuator[i].speed_auto1 = 25.0f;    // rpm
            machine.actuator[i].speed_auto2 = 25.0f;
            machine.actuator[i].acc_auto1 = 314.0f;     // rad/sec2
            machine.actuator[i].acc_auto2 = 314.0f;     // rad/sec2
            machine.actuator[i].dec_auto1 = 314.0f;     // rad/sec2
            machine.actuator[i].dec_auto2 = 314.0f;
            machine.actuator[i].timeout1_ms = 1000;
            machine.actuator[i].timeout2_ms = 1000;
            machine.actuator[i].timewarn1_ms = 800;
            machine.actuator[i].timewarn2_ms = 800;

            machine.actuator[i].homing_speed_rpm = 10.0f;
            machine.actuator[i].homing_rated_torque = 20.0f;
            machine.actuator[i].homing_position = 0;
            machine.actuator[i].homing_timeout_ms = 15*1000;
            
            machine.actuator[i].step = STEP_UNINITIALIZED;
        }




        // Stringhe di debug
        memset (&machine.debug_string, 0, sizeof(machine.debug_string));









            // Asse X
        {   float gearRatio = 1.0f;
            float mm_per_turn = 14.0f;

            COPY_POINTER(machine.actuator[X].name, "X")
            COPY_POINTER(machine.actuator[X].pos_name[0], "Start")
            COPY_POINTER(machine.actuator[X].pos_name[1], "End")
            machine.actuator[X].start_rpos = 0.0f;
            machine.actuator[X].end_rpos = 1600.0f;
            machine.actuator[X].cur_rpos = (machine.actuator[X].start_rpos - machine.actuator[X].start_rpos) / 2.0f;
            machine.actuator[X].speed_auto1 = machine.actuator[X].speed_auto2 = 40.0f;

            machine.actuator[X].start_rpos_toll = 0.005;
            machine.actuator[X].end_rpos_toll = 0.005;
                    
            machine.actuator[X].protocol = CANOPEN_AC_SERVO_DELTA;
            machine.actuator[X].boardId = 1;              // 1° scheda CANBUS
            machine.actuator[X].stationId = 0x2;        // Default del driver DELTA ASD-A2 = 0x7B

            /*
            machine.actuator[X].protocol = MODBUS_AC_SERVO_DELTA;
            machine.actuator[X].boardId = 2;              // 2° scheda SERIALE
            machine.actuator[X].stationId = 0x2; // 0x7F = Default del driver DELTA ASD-A2
            */

            // DEBUG
            // machine.actuator[X].protocol = PROTOCOL_NONE;



            machine.actuator[X].pulsesPerTurn = 1280000;
            machine.actuator[X].pulsesOverflow = 1280000;
            machine.actuator[X].cam_ratio = 1.0f / gearRatio * mm_per_turn;

            machine.actuator[X].open_func = (lp_cmd_func)NULL;
            machine.actuator[X].close_func = (lp_cmd_func)NULL;
            machine.actuator[X].home_func = (lp_cmd_func)home_X;
            machine.actuator[X].jog_plus_func = (lp_cmd_func)jog_plus_X;
            machine.actuator[X].jog_minus_func = (lp_cmd_func)jog_minus_X;

            // Posizione finale in unità encoder
            {   int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0;

                actuator_position_to_encoder(&machine.actuator[X], machine.actuator[X].end_rpos, &targetTurnsPPT, &targetPulsesPPT);
                actuator_position_to_encoder(&machine.actuator[X], machine.actuator[X].start_rpos, &targetTurnsPPT2, &targetPulsesPPT2);
            }
            
            
            machine.actuator[X].homingTorqueMode = true;
            machine.actuator[X].homing_speed_rpm = 10;
            machine.actuator[X].homing_rated_torque = 20.0f;
            machine.actuator[X].homing_position = 0;                // Direzione di azzeramento 
            machine.actuator[X].homingBoardId = 1;                // Ingresso #2, scheda 1
            machine.actuator[X].homingDI = 2;         
            machine.actuator[X].homingDIvalue = 1;
            
            // debug
            machine.actuator[X].homingDone = true;
            
            machine.actuator[X].pTrace = NULL;
            
        }



            // Asse Y
        {   float gearRatio = 1.0f;
            float mm_per_turn = 14.0f;

            COPY_POINTER(machine.actuator[Y].name, "Y")
            COPY_POINTER(machine.actuator[Y].pos_name[0], "Start")
            COPY_POINTER(machine.actuator[Y].pos_name[1], "End")
            machine.actuator[Y].start_rpos = 0.0f;
            machine.actuator[Y].end_rpos = 1100.0f;
            machine.actuator[Y].cur_rpos = (machine.actuator[Y].start_rpos - machine.actuator[Y].start_rpos) / 2.0f;
            machine.actuator[Y].speed_auto1 = machine.actuator[Y].speed_auto2 = 40.0f;

            machine.actuator[Y].start_rpos_toll = 0.005;
            machine.actuator[Y].end_rpos_toll = 0.005;

            machine.actuator[Y].protocol = CANOPEN_AC_SERVO_DELTA;
            machine.actuator[Y].boardId = 1;              // 1° scheda CANBUS
            machine.actuator[Y].stationId = 0x2;        // Default del driver DELTA ASD-A2 = 0x7B

            /*
            machine.actuator[Y].protocol = MODBUS_AC_SERVO_DELTA;
            machine.actuator[Y].boardId = 2;              // 2° scheda SERIALE
            machine.actuator[Y].stationId = 0x2; // 0x7F = Default del driver DELTA ASD-A2
            */

            // DEBUG
            machine.actuator[Y].protocol = PROTOCOL_NONE;



            machine.actuator[Y].pulsesPerTurn = 1280000;
            machine.actuator[Y].pulsesOverflow = 1280000;
            machine.actuator[Y].cam_ratio = 1.0f / gearRatio * mm_per_turn;

            machine.actuator[Y].open_func = (lp_cmd_func)NULL;
            machine.actuator[Y].close_func = (lp_cmd_func)NULL;
            machine.actuator[Y].home_func = (lp_cmd_func)NULL;
            machine.actuator[Y].jog_plus_func = (lp_cmd_func)jog_plus_Y;
            machine.actuator[Y].jog_minus_func = (lp_cmd_func)jog_minus_Y;

            // Posizione finale in unità encoder
            {   int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0;

                actuator_position_to_encoder(&machine.actuator[Y], machine.actuator[Y].end_rpos, &targetTurnsPPT, &targetPulsesPPT);
                actuator_position_to_encoder(&machine.actuator[Y], machine.actuator[Y].start_rpos, &targetTurnsPPT2, &targetPulsesPPT2);
            }
            
            
            machine.actuator[Y].homingTorqueMode = true;
            machine.actuator[Y].homing_speed_rpm = 10;
            machine.actuator[Y].homing_rated_torque = 20.0f;
            machine.actuator[Y].homing_position = 0;            // Direzione di azzeramento 
            machine.actuator[Y].homingBoardId = 1;                // Ingresso #2, scheda 1
            machine.actuator[Y].homingDI = 2;         
            machine.actuator[Y].homingDIvalue = 1;
            
        }

        
        
            // Asse Z
        {   float gearRatio = 1.0f;
            float mm_per_turn = 14.0f;

            COPY_POINTER(machine.actuator[Z].name, "Z")
            COPY_POINTER(machine.actuator[Z].pos_name[0], "Start")
            COPY_POINTER(machine.actuator[Z].pos_name[1], "End")
            machine.actuator[Z].start_rpos = 0.0f;
            machine.actuator[Z].end_rpos = 900.0f;
            machine.actuator[Z].cur_rpos = (machine.actuator[Z].start_rpos - machine.actuator[Z].start_rpos) / 2.0f;
            machine.actuator[Z].speed_auto1 = machine.actuator[Z].speed_auto2 = 40.0f;

            machine.actuator[Z].start_rpos_toll = 0.005;
            machine.actuator[Z].end_rpos_toll = 0.005;

            machine.actuator[Z].protocol = CANOPEN_AC_SERVO_DELTA;
            machine.actuator[Z].boardId = 1;              // 1° scheda CANBUS
            machine.actuator[Z].stationId = 0x2;        // Default del driver DELTA ASD-A2 = 0x7B

            /*
            machine.actuator[Z].protocol = MODBUS_AC_SERVO_DELTA;
            machine.actuator[Z].boardId = 2;              // 2° scheda SERIALE
            machine.actuator[Z].stationId = 0x2; // 0x7F = Default del driver DELTA ASD-A2
            */

            // DEBUG
            machine.actuator[Z].protocol = PROTOCOL_NONE;



            machine.actuator[Z].pulsesPerTurn = 1280000;
            machine.actuator[Z].pulsesOverflow = 1280000;
            machine.actuator[Z].cam_ratio = 1.0f / gearRatio * mm_per_turn;

            machine.actuator[Z].open_func = (lp_cmd_func)NULL;
            machine.actuator[Z].close_func = (lp_cmd_func)NULL;
            machine.actuator[Z].home_func = (lp_cmd_func)NULL;
            machine.actuator[Z].jog_plus_func = (lp_cmd_func)jog_plus_Z;
            machine.actuator[Z].jog_minus_func = (lp_cmd_func)jog_minus_Z;

            // Posizione finale in unità encoder
            {   int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0;

                actuator_position_to_encoder(&machine.actuator[Z], machine.actuator[Z].end_rpos, &targetTurnsPPT, &targetPulsesPPT);
                actuator_position_to_encoder(&machine.actuator[Z], machine.actuator[Z].start_rpos, &targetTurnsPPT2, &targetPulsesPPT2);
            }
            
            
            machine.actuator[Z].homingTorqueMode = true;
            machine.actuator[Z].homing_speed_rpm = 10;
            machine.actuator[Z].homing_rated_torque = 20.0f;
            machine.actuator[Z].homing_position = 0;            // Direzione di azzeramento 
            machine.actuator[Z].homingBoardId = 1;                // Ingresso #2, scheda 1
            machine.actuator[Z].homingDI = 2;         
            machine.actuator[Z].homingDIvalue = 1;
            
        }
        
        
        

        COPY_POINTER(machine.actuator[W].name, "W");
        COPY_POINTER(machine.actuator[W].pos_name[BACK], "OFF");
        COPY_POINTER(machine.actuator[W].pos_name[FRONT], "ON");
        machine.actuator[W].speed_auto1 = machine.actuator[W].speed_auto2 = 25.0f;
        machine.actuator[W].protocol = ADC_INPUT;
        
        COPY_POINTER(machine.actuator[T].name, "T");
        COPY_POINTER(machine.actuator[T].pos_name[BACK], "OFF");
        COPY_POINTER(machine.actuator[T].pos_name[FRONT], "ON");
        machine.actuator[T].speed_auto1 = machine.actuator[T].speed_auto2 = 25.0f;
        machine.actuator[T].protocol = ADC_INPUT;

        
        
        
        COPY_POINTER(machine.actuator[SPINDLE].name, "Spindle");
        COPY_POINTER(machine.actuator[SPINDLE].pos_name[BACK], "OFF");
        COPY_POINTER(machine.actuator[SPINDLE].pos_name[FRONT], "ON");
        machine.actuator[SPINDLE].speed_auto1 = machine.actuator[COOLER_I].speed_auto2 = 25.0f;
        machine.actuator[SPINDLE].protocol = MONOSTABLE_PN_VALVE;

        

        COPY_POINTER(machine.actuator[COOLER_I].name, "Cooler_I");
        COPY_POINTER(machine.actuator[COOLER_I].pos_name[BACK], "OFF");
        COPY_POINTER(machine.actuator[COOLER_I].pos_name[FRONT], "ON");
        machine.actuator[COOLER_I].speed_auto1 = machine.actuator[COOLER_I].speed_auto2 = 25.0f;
        machine.actuator[COOLER_I].protocol = BISTABLE_PN_VALVE;


        COPY_POINTER(machine.actuator[COOLER_II].name, "Cooler_II");
        COPY_POINTER(machine.actuator[COOLER_II].pos_name[BACK], "OFF");
        COPY_POINTER(machine.actuator[COOLER_II].pos_name[FRONT], "ON");
        machine.actuator[COOLER_II].speed_auto1 = machine.actuator[COOLER_II].speed_auto2 = 25.0f;
        machine.actuator[COOLER_II].protocol = BISTABLE_PN_VALVE;

        
        COPY_POINTER(machine.actuator[AIR_PRESS].name, "AirPress");
        COPY_POINTER(machine.actuator[AIR_PRESS].pos_name[BACK], "OFF");
        COPY_POINTER(machine.actuator[AIR_PRESS].pos_name[FRONT], "ON");
        machine.actuator[AIR_PRESS].speed_auto1 = machine.actuator[AIR_PRESS].speed_auto2 = 25.0f;
        machine.actuator[AIR_PRESS].protocol = ADC_INPUT;

        
        
        do_acctuator_mirror ();


        ////////////////////////////////
        //  Impostazioni
        //

        machine.settings.spindle_speed = 120.0f;
        machine.settings.spindle_power = 7500.0;
        machine.settings.mill_feed_mm_min_X = 240.0;
        machine.settings.mill_feed_mm_min_Y = 240.0;
        machine.settings.mill_feed_mm_min_Z = 240.0;
        machine.settings.mill_feed_mm_min_W = 240.0;
        machine.settings.mill_feed_mm_min_T = 240.0;
        
        machine.settings.rapid_feed_X = 2000.0;
        machine.settings.rapid_feed_Y = 2000.0;
        machine.settings.rapid_feed_Z = 2000.0;
        machine.settings.rapid_feed_W = 2000.0;
        machine.settings.rapid_feed_T = 2000.0;
        
        machine.settings.max_weight = 2500.0;
        machine.settings.max_X = 1600.0;
        machine.settings.max_Y = 1200.0;
        machine.settings.max_Z = 900.0;

        machine.settings.cam_ratio_X = 1.0f;
        machine.settings.cam_ratio_Y = 1.0f;
        machine.settings.cam_ratio_Z = 1.0f;
        machine.settings.cam_ratio_W = 1.0f;
        machine.settings.cam_ratio_T = 1.0f;
     



        machine.rebuildAlarmList = 1;


    #ifdef DEBUG

        if (App.SimulateMode) {
            for (i=0; i<machine.num_actuator; i++) {
                if (!machine.actuator[i].pSerialSlot && !machine.actuator[i].pIOSlot &&  !machine.actuator[i].pCANSlot) {
                    machine.actuator[i].speed_auto1 /= 16.0f;
                    machine.actuator[i].speed_auto2 /= 16.0f;
                }
            }
        }

        #else
        /*

        for (i=0; i<MAX_ACTUATOR_NAME; i++) {
                if (ACTUATORS_NAME_LIST[i]) {
                        strncpy(machine.actuator[i].name, ACTUATORS_NAME_LIST[i], sizeof(machine.actuator[0].name));
                        strncpy(machine.actuator[i].pos_name[0], ACTUATORS_IN_POSITION_NAME_LIST[i], sizeof(machine.actuator[0].pos_name[0]));
                        strncpy(machine.actuator[i].pos_name[1], ACTUATORS_OUT_POSITION_NAME_LIST[i], sizeof(machine.actuator[0].pos_name[0]));
                        }
                }
                */
        #endif


  } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "login_user() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "login_user() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }
            
    
    
return 1;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inizializza la logica della macchina (Eseguita prima della lettura delle impostazioni persistenti
//

int logic_post_init ( void ) {
    
    return 1;
}


////////////////////////////////////////////
// callback macchina inizializzata
//
int on_machine_initialized ( void ) {
    

    if (stop_spindle() < 0) {
    }

    if (stop_cooler_I() < 0) {
    }

    if (stop_cooler_II() < 0) {
    }
                           
    if (generate_alarm((char*) "Machine initialized", 2200, 1, (int) ALARM_LOG, 0+1) < 0) {
    }
    
    return 1;
}



