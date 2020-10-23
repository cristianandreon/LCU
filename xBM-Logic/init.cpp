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
            
            machine.actuator[i].speed_auto1 = 1500.0f;    // rpm
            machine.actuator[i].speed_auto2 = 1500.0f;
            machine.actuator[i].acc_auto1 = 300.0f;     // rad/sec2
            machine.actuator[i].acc_auto2 = 300.0f;     // rad/sec2
            machine.actuator[i].dec_auto1 = 300.0f;     // rad/sec2
            machine.actuator[i].dec_auto2 = 300.0f;
            machine.actuator[i].timeout1_ms = 3000;
            machine.actuator[i].timeout2_ms = 3000;
            machine.actuator[i].timewarn1_ms = 2500;
            machine.actuator[i].timewarn2_ms = 2500;

            machine.actuator[i].homing_speed_rpm = 10.0f;
            machine.actuator[i].homing_rated_torque = 20.0f;
            machine.actuator[i].homing_position = STRETCH_UP;
            machine.actuator[i].homing_timeout_ms = 15*1000;
            
            machine.actuator[i].step = STEP_UNINITIALIZED;
        }




        // Stringhe di debug
        memset (&machine.debug_string, 0, sizeof(machine.debug_string));


        machine.workSet.production = 500;
        machine.workSet.primary_air_gap_mm = 500.0f;        // mm posizione asta
        machine.workSet.secondary_air_gap_ms = 900;         // msec da asta bassa
        machine.workSet.min_secondary_air_time_ms = 200;    // minimo aria recondaria
        machine.workSet.discharge_air_time_ms = 2000;
        machine.workSet.pit_unlock_time_ms = 100;




        machine.App.Epsilon = 0.0001f;

        machine.App.NUM_PREFORM_REGISTRY = 5;
        machine.App.TEMPERATURE_READER_POS = machine.App.NUM_PREFORM_REGISTRY-2;
        machine.App.BLOW_MOLD_POS = machine.App.NUM_PREFORM_REGISTRY-1;



        machine.workSet.preform_temp1  = 100.0f;
        machine.workSet.preform_temp2  = 99.0f;
        machine.workSet.preform_temp3  = 95.0f;

        machine.workSet.preform_temp_gap1 = 100.0f;
        machine.workSet.preform_temp_gap2 = 100.0f;
        machine.workSet.preform_temp_gap3 = 1.0f;



        for (i=0; i<machine.App.NUM_PREFORM_REGISTRY; i++) {
            machine.App.preform_registry[i] = EMPTY;
        }

        // DEBUG
        // machine.App.preform_registry[40] = 1;
        // machine.App.preform_registry[45] = 1;


        machine.App.preform_temp = 90.0f;





        machine.workSet.max_pressure_in_mold = 0.4f;
        
        machine.workSet.pressure_min = 4.0f;
        
        machine.workSet.pressure_max = 41.0f;







        {   float gearRatio = 21.0f;
            float  primPerimeter = 360.000f; // N.B.: Gradi Manovella per giro albero


            COPY_POINTER(machine.actuator[MOLD].name, "stampo")
            COPY_POINTER(machine.actuator[MOLD].pos_name[OPEN], "aperto")
            COPY_POINTER(machine.actuator[MOLD].pos_name[CLOSE], "chiuso")
            machine.actuator[MOLD].start_rpos = 0.0f;
            machine.actuator[MOLD].end_rpos = 179.0f;
            machine.actuator[MOLD].cur_rpos = (machine.actuator[MOLD].start_rpos - machine.actuator[MOLD].start_rpos) / 2.0f;
            machine.actuator[MOLD].speed_auto1 = machine.actuator[MOLD].speed_auto2 = 1200.0; //rpm // (machine.actuator[MOLD].end_rpos-machine.actuator[MOLD].start_rpos) / 22.0f;

            machine.actuator[MOLD].acc_auto1 = 350.0f;
            machine.actuator[MOLD].acc_auto2 = 350.0f;

            machine.actuator[MOLD].end_rpos_toll = 20.0f;
            machine.actuator[MOLD].start_rpos_toll = 20.0f;

            machine.actuator[MOLD].timeout1_ms = 5000;
            machine.actuator[MOLD].timeout2_ms = 5000;
            machine.actuator[MOLD].timewarn1_ms = 4000;
            machine.actuator[MOLD].timewarn2_ms = 4000;
            machine.actuator[MOLD].protocol = MODBUS_AC_SERVO_LICHUAN;
            machine.actuator[MOLD].boardId = 1; // Diver #1
            machine.actuator[MOLD].stationId = 1;
            machine.actuator[MOLD].auxBoardId = 1;  // Mosfet sulla Scheda 1
            machine.actuator[MOLD].auxDO1 = 9;      // N.B.: 9 = Pin 30 : Uscita 9
            machine.actuator[MOLD].auxDO2 = 0;
            machine.actuator[MOLD].pulsesPerTurn = 10000;
            machine.actuator[MOLD].pulsesOverflow = 65535;
            machine.actuator[MOLD].cam_ratio = 1.0f / gearRatio * primPerimeter;
            machine.actuator[MOLD].open_func = (lp_cmd_func)open_mold;
            machine.actuator[MOLD].close_func = (lp_cmd_func)close_mold;
            machine.actuator[MOLD].home_func = (lp_cmd_func)home_mold;

            machine.actuator[MOLD].homingTorqueMode = true;
            machine.actuator[MOLD].homing_offset_mm = 15.0f; // Offset dal trigger discesa Ingresso digitale
            machine.actuator[MOLD].homing_speed_rpm = 10;
            machine.actuator[MOLD].homing_rated_torque = 30.0f;
            machine.actuator[MOLD].homing_timeout_ms = 15*1000;
            machine.actuator[MOLD].homing_position = OFF;   // Azzeramento in apertura
            machine.actuator[MOLD].homingBoardId = 1;       // Ingresso #1, scheda 1
            machine.actuator[MOLD].homingDI = 1;
            machine.actuator[MOLD].homingDIvalue = 1;

                                                
            // machine.actuator[MOLD].disabled = true;
            
            // Posizione finale in unità encoder
            {   int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0;

                actuator_position_to_encoder(&machine.actuator[MOLD], machine.actuator[MOLD].end_rpos, &targetTurnsPPT, &targetPulsesPPT);
                actuator_position_to_encoder(&machine.actuator[MOLD], machine.actuator[MOLD].start_rpos, &targetTurnsPPT2, &targetPulsesPPT2);
            }
        }





        {   float gearRatio = 8.0f;
            float primPerimeter = 97.000f * 3.14159265358f; // N.B.: mm asta di stiro

            COPY_POINTER(machine.actuator[STRETCH].name, "stiro")
            COPY_POINTER(machine.actuator[STRETCH].pos_name[STRETCH_UP], "alto")
            COPY_POINTER(machine.actuator[STRETCH].pos_name[STRETCH_DOWN], "basso")
            machine.actuator[STRETCH].start_rpos = 0.0f;
            machine.actuator[STRETCH].end_rpos = 750.0f;
            machine.actuator[STRETCH].cur_rpos = (machine.actuator[STRETCH].start_rpos - machine.actuator[STRETCH].start_rpos) / 2.0f;
            machine.actuator[STRETCH].speed_auto1 = machine.actuator[STRETCH].speed_auto2 = 3600.0f;
            machine.actuator[STRETCH].acc_auto1 = machine.actuator[STRETCH].acc_auto2 = 7000.0f;
            machine.actuator[STRETCH].end_rpos_toll = 0.5f;
            machine.actuator[STRETCH].start_rpos_toll = 0.5f;


            machine.actuator[STRETCH].protocol = CANOPEN_AC_SERVO_DELTA;
            machine.actuator[STRETCH].boardId = 1;              // 1° scheda CANBUS
            machine.actuator[STRETCH].stationId = 0x2;        // Default del driver DELTA ASD-A2 = 0x7B

            /*
            machine.actuator[STRETCH].protocol = MODBUS_AC_SERVO_DELTA;
            machine.actuator[STRETCH].boardId = 2;              // 2° scheda SERIALE
            machine.actuator[STRETCH].stationId = 0x2; // 0x7F = Default del driver DELTA ASD-A2
            */

            // DEBUG
            // machine.actuator[STRETCH].protocol = PROTOCOL_NONE;



            machine.actuator[STRETCH].pulsesPerTurn = 1280000;
            machine.actuator[STRETCH].pulsesOverflow = 1280000; // 214748364;
            machine.actuator[STRETCH].cam_ratio = 1.0f / gearRatio * primPerimeter;

            machine.actuator[STRETCH].open_func = (lp_cmd_func)down_stretch_rod;
            machine.actuator[STRETCH].close_func = (lp_cmd_func)up_stretch_rod;
            machine.actuator[STRETCH].home_func = (lp_cmd_func)home_stretch_rod;

            // Posizione finale in unità encoder
            {   int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0;

                actuator_position_to_encoder(&machine.actuator[STRETCH], machine.actuator[STRETCH].end_rpos, &targetTurnsPPT, &targetPulsesPPT);
                actuator_position_to_encoder(&machine.actuator[STRETCH], machine.actuator[STRETCH].start_rpos, &targetTurnsPPT2, &targetPulsesPPT2);
            }
            
            
            machine.actuator[STRETCH].homingTorqueMode = true;
            machine.actuator[STRETCH].homing_speed_rpm = 10;
            machine.actuator[STRETCH].homing_rated_torque = 10.0f;
            machine.actuator[STRETCH].homing_timeout_ms = 15*1000;
            machine.actuator[STRETCH].homing_position = STRETCH_UP;     // Azzeramento verso l'altro
            machine.actuator[STRETCH].homingBoardId = 1;                // Ingresso #2, scheda 1
            machine.actuator[STRETCH].homingDI = 2;         
            machine.actuator[STRETCH].homingDIvalue = 1;
            
        }



        COPY_POINTER(machine.actuator[PRIMARY_AIR].name, "ariaI")
        COPY_POINTER(machine.actuator[PRIMARY_AIR].pos_name[OFF], "chiusa")
        COPY_POINTER(machine.actuator[PRIMARY_AIR].pos_name[ON], "aperta")
        machine.actuator[PRIMARY_AIR].speed_auto1 = machine.actuator[PRIMARY_AIR].speed_auto2 = 25000.0f;
        machine.actuator[PRIMARY_AIR].protocol = BISTABLE_PN_VALVE;
        machine.actuator[PRIMARY_AIR].open_func = (lp_cmd_func)open_primary_air;
        machine.actuator[PRIMARY_AIR].close_func = (lp_cmd_func)close_primary_air;
        machine.actuator[PRIMARY_AIR].position = OFF;

        COPY_POINTER(machine.actuator[SECONDARY_AIR].name, "ariaII")
        COPY_POINTER(machine.actuator[SECONDARY_AIR].pos_name[OFF], "chiusa")
        COPY_POINTER(machine.actuator[SECONDARY_AIR].pos_name[ON], "aperta")
        machine.actuator[SECONDARY_AIR].speed_auto1 = machine.actuator[SECONDARY_AIR].speed_auto2 = 25000.0f;
        machine.actuator[SECONDARY_AIR].protocol = BISTABLE_PN_VALVE;
        machine.actuator[SECONDARY_AIR].open_func = (lp_cmd_func)open_secondary_air;
        machine.actuator[SECONDARY_AIR].close_func = (lp_cmd_func)close_secondary_air;
        machine.actuator[SECONDARY_AIR].position = OFF;

        COPY_POINTER(machine.actuator[DISCHARGE_AIR].name, "ariaS");
        COPY_POINTER(machine.actuator[DISCHARGE_AIR].pos_name[OFF], "aperta");
        COPY_POINTER(machine.actuator[DISCHARGE_AIR].pos_name[ON], "chiusa");
        machine.actuator[DISCHARGE_AIR].speed_auto1 = machine.actuator[DISCHARGE_AIR].speed_auto2 = 25000.0f;
        machine.actuator[DISCHARGE_AIR].protocol = BISTABLE_PN_VALVE;
        machine.actuator[DISCHARGE_AIR].open_func = (lp_cmd_func)open_discharge_air;
        machine.actuator[DISCHARGE_AIR].close_func = (lp_cmd_func)close_discharge_air;
        machine.actuator[DISCHARGE_AIR].position = ON;

        COPY_POINTER(machine.actuator[RECOVERY_AIR].name, "ariaR");
        COPY_POINTER(machine.actuator[RECOVERY_AIR].pos_name[OFF], "aperta");
        COPY_POINTER(machine.actuator[RECOVERY_AIR].pos_name[ON], "chiusa");
        machine.actuator[RECOVERY_AIR].speed_auto1 = machine.actuator[RECOVERY_AIR].speed_auto2 = 25000.0f;
        machine.actuator[RECOVERY_AIR].protocol = BISTABLE_PN_VALVE;
        machine.actuator[RECOVERY_AIR].open_func = (lp_cmd_func)open_recovery_air;
        machine.actuator[RECOVERY_AIR].close_func = (lp_cmd_func)close_recovery_air;
        machine.actuator[RECOVERY_AIR].position = OFF;

        COPY_POINTER(machine.actuator[TRANSFERITOR_Y].name, "trasfY");
        COPY_POINTER(machine.actuator[TRANSFERITOR_Y].pos_name[INSIDE], "dentro");
        COPY_POINTER(machine.actuator[TRANSFERITOR_Y].pos_name[OUTSIDE], "fuori");
        machine.actuator[TRANSFERITOR_Y].speed_auto1 = machine.actuator[TRANSFERITOR_Y].speed_auto2 = 2500.0f;
        machine.actuator[TRANSFERITOR_Y].protocol = BISTABLE_PN_VALVE;
        machine.actuator[TRANSFERITOR_Y].open_func = (lp_cmd_func)outside_transferitor;
        machine.actuator[TRANSFERITOR_Y].close_func = (lp_cmd_func)inside_transferitor;
        machine.actuator[TRANSFERITOR_Y].position = BACK;

        COPY_POINTER(machine.actuator[TRANSFERITOR_X].name, "trasfX");
        COPY_POINTER(machine.actuator[TRANSFERITOR_X].pos_name[FRONT], "avanti");
        COPY_POINTER(machine.actuator[TRANSFERITOR_X].pos_name[BACK], "indietro");
        machine.actuator[TRANSFERITOR_X].speed_auto1 = machine.actuator[TRANSFERITOR_X].speed_auto2 = 3000.0f;
        machine.actuator[TRANSFERITOR_X].protocol = BISTABLE_PN_VALVE;
        machine.actuator[TRANSFERITOR_X].open_func = (lp_cmd_func)front_transferitor;
        machine.actuator[TRANSFERITOR_X].close_func = (lp_cmd_func)back_transferitor;
        machine.actuator[TRANSFERITOR_X].position = BACK;


        COPY_POINTER(machine.actuator[HEATING_CHAIN1].name, "catena_risc_step1");
        COPY_POINTER(machine.actuator[HEATING_CHAIN1].pos_name[BACK], "indietro");
        COPY_POINTER(machine.actuator[HEATING_CHAIN1].pos_name[FRONT], "avanti");
        machine.actuator[HEATING_CHAIN1].speed_auto1 = machine.actuator[HEATING_CHAIN1].speed_auto2 = 2000.0f;
        machine.actuator[HEATING_CHAIN1].protocol = BISTABLE_PN_VALVE;


        COPY_POINTER(machine.actuator[HEATING_CHAIN2].name, "catena_risc_step2");
        COPY_POINTER(machine.actuator[HEATING_CHAIN2].pos_name[BACK], "indietro");
        COPY_POINTER(machine.actuator[HEATING_CHAIN2].pos_name[FRONT], "avanti");
        machine.actuator[HEATING_CHAIN2].speed_auto1 = machine.actuator[HEATING_CHAIN2].speed_auto2 = 2000.0f;
        machine.actuator[HEATING_CHAIN2].protocol = BISTABLE_PN_VALVE;


        COPY_POINTER(machine.actuator[HEATING_CHAIN3].name, "catena_risc_step3");
        COPY_POINTER(machine.actuator[HEATING_CHAIN3].pos_name[BACK], "indietro");
        COPY_POINTER(machine.actuator[HEATING_CHAIN3].pos_name[FRONT], "avanti");
        machine.actuator[HEATING_CHAIN3].speed_auto1 = machine.actuator[HEATING_CHAIN3].speed_auto2 = 2000.0f;
        machine.actuator[HEATING_CHAIN3].protocol = BISTABLE_PN_VALVE;


        COPY_POINTER(machine.actuator[LOAD_UNLOAD_X].name, "manipX");
        COPY_POINTER(machine.actuator[LOAD_UNLOAD_X].pos_name[BACK], "indietro");
        COPY_POINTER(machine.actuator[LOAD_UNLOAD_X].pos_name[FRONT], "avanti");
        machine.actuator[LOAD_UNLOAD_X].speed_auto1 = machine.actuator[LOAD_UNLOAD_X].speed_auto2 = 2500.0f;
        machine.actuator[LOAD_UNLOAD_X].protocol = BISTABLE_PN_VALVE;

        COPY_POINTER(machine.actuator[LOAD_UNLOAD_Z].name, "manipZ");
        COPY_POINTER(machine.actuator[LOAD_UNLOAD_Z].pos_name[UP], "alto");
        COPY_POINTER(machine.actuator[LOAD_UNLOAD_Z].pos_name[DOWN], "basso");
        machine.actuator[LOAD_UNLOAD_Z].speed_auto1 = machine.actuator[LOAD_UNLOAD_Z].speed_auto2 = 3500.0f;
        machine.actuator[LOAD_UNLOAD_Z].protocol = BISTABLE_PN_VALVE;

        COPY_POINTER(machine.actuator[LOAD_UNLOAD_PICK].name, "manipP");
        COPY_POINTER(machine.actuator[LOAD_UNLOAD_PICK].pos_name[OPEN], "aperta");
        COPY_POINTER(machine.actuator[LOAD_UNLOAD_PICK].pos_name[CLOSE], "chiusa");
        machine.actuator[LOAD_UNLOAD_PICK].speed_auto1 = machine.actuator[LOAD_UNLOAD_PICK].speed_auto2 = 3500.0f;
        machine.actuator[LOAD_UNLOAD_PICK].protocol = BISTABLE_PN_VALVE;


        COPY_POINTER(machine.actuator[BOTTLE_PREF_HOLDER].name, "bloccoBP");
        COPY_POINTER(machine.actuator[BOTTLE_PREF_HOLDER].pos_name[UP], "alto");
        COPY_POINTER(machine.actuator[BOTTLE_PREF_HOLDER].pos_name[DOWN], "basso")
        machine.actuator[BOTTLE_PREF_HOLDER].open_func = (lp_cmd_func)down_holding_station;
        machine.actuator[BOTTLE_PREF_HOLDER].close_func = (lp_cmd_func)up_holding_station;
        machine.actuator[BOTTLE_PREF_HOLDER].speed_auto1 = machine.actuator[BOTTLE_PREF_HOLDER].speed_auto2 = 3500.0f;
        machine.actuator[BOTTLE_PREF_HOLDER].protocol = BISTABLE_PN_VALVE;


        COPY_POINTER(machine.actuator[BLOW_PRESSURE].name, "pressBott")
        COPY_POINTER(machine.actuator[BLOW_PRESSURE].pos_name[UP], "40")
        COPY_POINTER(machine.actuator[BLOW_PRESSURE].pos_name[DOWN], "0")
        machine.actuator[BLOW_PRESSURE].speed_auto1 = machine.actuator[LINE_PRESSURE].speed_auto2 = 500.0f;
        machine.actuator[BLOW_PRESSURE].cur_rpos = 12.5f;
        machine.actuator[BLOW_PRESSURE].start_rpos = 0.0f;
        machine.actuator[BLOW_PRESSURE].end_rpos = 40.0f;
        update_actuator_virtual_pos((void *)&machine.actuator[BLOW_PRESSURE]);    
        machine.actuator[BLOW_PRESSURE].protocol = ADC_INPUT;

        COPY_POINTER(machine.actuator[LINE_PRESSURE].name, "pressLinea")
        COPY_POINTER(machine.actuator[LINE_PRESSURE].pos_name[UP], "40")
        COPY_POINTER(machine.actuator[LINE_PRESSURE].pos_name[DOWN], "0")
        machine.actuator[LINE_PRESSURE].speed_auto1 = machine.actuator[LINE_PRESSURE].speed_auto2 = 500.0f;
        machine.actuator[LINE_PRESSURE].cur_rpos = 36.0f;
        machine.actuator[LINE_PRESSURE].start_rpos = 0.0f;
        machine.actuator[LINE_PRESSURE].end_rpos = 40.0f;
        update_actuator_virtual_pos((void *)&machine.actuator[LINE_PRESSURE]);
        machine.actuator[LINE_PRESSURE].protocol = ADC_INPUT;



        COPY_POINTER(machine.actuator[PIT_LOCK].name, "blocco_pista")
        COPY_POINTER(machine.actuator[PIT_LOCK].pos_name[INSIDE], "sbloccato")
        COPY_POINTER(machine.actuator[PIT_LOCK].pos_name[OUTSIDE], "bloccato")
        machine.actuator[PIT_LOCK].speed_auto1 = machine.actuator[PIT_LOCK].speed_auto2 = 3000.0f;
        machine.actuator[PIT_LOCK].protocol = BISTABLE_PN_VALVE;

        COPY_POINTER(machine.actuator[PREF_LOAD_SUPP].name, "supp_carico_pref")
        COPY_POINTER(machine.actuator[PREF_LOAD_SUPP].pos_name[INSIDE], "dentro")
        COPY_POINTER(machine.actuator[PREF_LOAD_SUPP].pos_name[OUTSIDE], "fuori")
        machine.actuator[PREF_LOAD_SUPP].speed_auto1 = machine.actuator[PIT_LOCK].speed_auto2 = 3000.0f;
        machine.actuator[PREF_LOAD_SUPP].protocol = BISTABLE_PN_VALVE;


        COPY_POINTER(machine.actuator[OWENS].name, "forni")
        COPY_POINTER(machine.actuator[OWENS].pos_name[OFF], "spenti")
        COPY_POINTER(machine.actuator[OWENS].pos_name[ON], "accesi")
        machine.actuator[OWENS].protocol = VARIABLE_ON_OFF;

        COPY_POINTER(machine.actuator[OWENS_TEMPERATURE1].name, "temperatura_forno1")
        COPY_POINTER(machine.actuator[OWENS_TEMPERATURE1].pos_name[DOWN], "cold")
        COPY_POINTER(machine.actuator[OWENS_TEMPERATURE1].pos_name[UP], "ok")
        machine.actuator[OWENS_TEMPERATURE1].protocol = ADC_INPUT;

        COPY_POINTER(machine.actuator[OWENS_TEMPERATURE2].name, "temperatura_forno2")
        COPY_POINTER(machine.actuator[OWENS_TEMPERATURE2].pos_name[DOWN], "cold")
        COPY_POINTER(machine.actuator[OWENS_TEMPERATURE2].pos_name[UP], "ok")
        machine.actuator[OWENS_TEMPERATURE2].protocol = ADC_INPUT;

        COPY_POINTER(machine.actuator[PREFORM_TEMPERATURE].name, "temperatura_preforma")
        COPY_POINTER(machine.actuator[PREFORM_TEMPERATURE].pos_name[DOWN], "out")
        COPY_POINTER(machine.actuator[PREFORM_TEMPERATURE].pos_name[UP], "ok")
        machine.actuator[PREFORM_TEMPERATURE].start_rpos  = 0.0f;
        machine.actuator[PREFORM_TEMPERATURE].cur_rpos  = 102.7f;
        machine.actuator[PREFORM_TEMPERATURE].end_rpos  = 150.0f;
        update_actuator_virtual_pos((void *)&machine.actuator[LINE_PRESSURE]);
        machine.actuator[PREFORM_TEMPERATURE].protocol = ADC_INPUT;
        machine.actuator[PREFORM_TEMPERATURE].auxSCRId = 1;
        machine.actuator[PREFORM_TEMPERATURE].auxI2C = 0;


        COPY_POINTER(machine.actuator[VENTILATOR].name, "ventalazione")
        COPY_POINTER(machine.actuator[VENTILATOR].pos_name[OFF], "spenta")
        COPY_POINTER(machine.actuator[VENTILATOR].pos_name[ON], "accesa")
        machine.actuator[VENTILATOR].protocol = AC_INVERTER;


        COPY_POINTER(machine.actuator[ASPIRATOR].name, "aspirazione")
        COPY_POINTER(machine.actuator[ASPIRATOR].pos_name[OFF], "spenta")
        COPY_POINTER(machine.actuator[ASPIRATOR].pos_name[ON], "accesa")
        machine.actuator[ASPIRATOR].protocol = ASYNC_MOTOTR;


        COPY_POINTER(machine.actuator[COOLING_WATER1].name, "raff_forni")
        COPY_POINTER(machine.actuator[COOLING_WATER1].pos_name[OFF], "spento")
        COPY_POINTER(machine.actuator[COOLING_WATER1].pos_name[ON], "acceso")
        machine.actuator[COOLING_WATER1].protocol = BISTABLE_PN_VALVE;

        COPY_POINTER(machine.actuator[COOLING_WATER2].name, "raff_stampo")
        COPY_POINTER(machine.actuator[COOLING_WATER2].pos_name[OFF], "spento")
        COPY_POINTER(machine.actuator[COOLING_WATER2].pos_name[ON], "acceso")
        machine.actuator[COOLING_WATER2].protocol = BISTABLE_PN_VALVE;

        COPY_POINTER(machine.actuator[OWEN_AIR].name, "aria_forni")
        COPY_POINTER(machine.actuator[OWEN_AIR].pos_name[OFF], "spenta")
        COPY_POINTER(machine.actuator[OWEN_AIR].pos_name[ON], "accesa")
        machine.actuator[OWEN_AIR].protocol = BISTABLE_PN_VALVE;


        COPY_POINTER(machine.actuator[ELEV_UNJAM].name, "elevUnjam")
        COPY_POINTER(machine.actuator[ELEV_UNJAM].pos_name[OFF], "spento")
        COPY_POINTER(machine.actuator[ELEV_UNJAM].pos_name[ON], "acceso")
        machine.actuator[ELEV_UNJAM].protocol = MONOSTABLE_PN_VALVE;

        COPY_POINTER(machine.actuator[ELEV_MOTOR].name, "elevMotor")
        COPY_POINTER(machine.actuator[ELEV_MOTOR].pos_name[OFF], "spento")
        COPY_POINTER(machine.actuator[ELEV_MOTOR].pos_name[ON], "acceso")
        machine.actuator[ELEV_MOTOR].protocol = ASYNC_MOTOTR;

        COPY_POINTER(machine.actuator[ORIENT_FAN].name, "orientFan")
        COPY_POINTER(machine.actuator[ORIENT_FAN].pos_name[OFF], "spento")
        COPY_POINTER(machine.actuator[ORIENT_FAN].pos_name[ON], "acceso")
        machine.actuator[ORIENT_FAN].protocol = ASYNC_MOTOTR;

        COPY_POINTER(machine.actuator[ORIENT_ROLL].name, "orientRoll")
        COPY_POINTER(machine.actuator[ORIENT_ROLL].pos_name[OFF], "spento")
        COPY_POINTER(machine.actuator[ORIENT_ROLL].pos_name[ON], "acceso")
        machine.actuator[ORIENT_ROLL].protocol = ASYNC_MOTOTR;



        do_acctuator_mirror ();


        ////////////////////////////////
        //  Impostazioni
        //

        machine.settings.startup_owens_cycles = 10;
        machine.settings.startup_owens_delay_ms = 0;

        // Numero cicli attesa speginmento forni
        machine.settings.turnoff_owens_cycles = 7;
        machine.settings.turnoff_owens_delay_ms = 0;

        // Numero di cicli senza carico innesco standby forni
        machine.settings.owens_standby_cycles = 3;
        machine.settings.owens_standby_delay_ms = 0;
        machine.settings.owens_towork_cycles = 3;
        machine.settings.owens_towork_delay_ms = 0;

        machine.settings.initial_owens_cycles = 3;
        machine.settings.initial_owens_ratio = 1.2;


        machine.settings.chain_stepper1_pause_ms = 100;
        machine.settings.chain_stepper2_pause_ms = 100;
        machine.settings.chain_stepper3_pause_ms = 100;

        machine.settings.trasf_x_forward_pause_ms = 100;
        machine.settings.chain_trasf_z_down_pause_ms = 100;

        machine.settings.chain_picker_open_pause_ms = 100;
        machine.settings.chain_picker_close_pause_ms = 100;

        machine.settings.pref_load_inside_pause_ms = 100;
        machine.settings.pref_load_outside_pause_ms = 100;

        machine.settings.pit_stopper_inside_pause_ms = 100;
        machine.settings.pit_stopper_outside_pause_ms = 100;

        machine.settings.aspirator_delay_ms = 5 * 60 * 1000;







        machine.rebuildAlarmList = 1;


    #ifdef DEBUG

        if (App.SimulateMode) {
            for (i=0; i<machine.num_actuator; i++) {
                if (!machine.actuator[i].pSerialSlot && !machine.actuator[i].pIOSlot &&  !machine.actuator[i].pCANSlot) {
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
    

    // Comando Chiusura Aria primaria
    if (close_primary_air() < 0) {

    }

    // Comando Chiusura Aria secondaria
    if (close_secondary_air() < 0) {

    }

    // Comando Apertura Scarico Aria
    if (open_discharge_air() < 0) {

    }
                           
    if (generate_alarm((char*) "Machine initialized", 2200, 1, (int) ALARM_LOG, 0+1) < 0) {
    }
    
    return 1;
}



