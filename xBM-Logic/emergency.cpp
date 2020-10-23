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



int emergency(void) {

    try {

        if (machine.status != EMERGENCY) {

            snprintf(App.Msg, App.MsgSize, "[%s Entering EMERGENCY%s]", (char*)ANSI_COLOR_MAGENTA, (char*)ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);

            ///////////////////////////////
            // Stop di tutti i movimenti
            //


            // Comando Chiusura Aria primaria
            if (close_primary_air() < 0) {

            }

            // Comando Chiusura Aria secondaria
            if (close_secondary_air() < 0) {

            }

            // Comando Apertura Scarico Aria
            if (open_discharge_air() < 0) {

            }


            if (stop_mold() < 0) {

            }

            if (stop_stretch_rod() < 0) {

            }

            if (stop_trasferitor() < 0) {

            }



            PUT_HEAT_OFF();





            // Marca il buffer preforme come fuori uso
            // ... se trascorso un certo tempo ???
            set_preform_registry_as_damaged();



            // Disattiva la potenza e Arresta i servo motori     
            // .. ritardo disattivazione ?
            if (App.DebugMode) {
                snprintf(App.Msg, App.MsgSize, "%s[POWER Still active due to Debug Mode]%s", (char*) ANSI_COLOR_MAGENTA, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            } else {
                PUT_POWER_OFF();
            }


            machine.start_request = 0;

            machine.status = EMERGENCY;


            snprintf(App.Msg, App.MsgSize, "EMERGENCY mode [%s]", machine.status_message);
            strncpy(machine.status_message, (char*)App.Msg, machine.status_message_size);

        } else {

            // strncpy(machine.status_message, "EMERGENCY mode");

        }



        
    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "automatic() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "automatic() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }
        
    return 1;
}



// Comando stop stampo

int stop_mold() {
    machine.actuator[MOLD].target_position = -1;
    machine.actuator[MOLD].step = STEP_STOPPED;
    return 1;
}

int stop_stretch_rod() {
    machine.actuator[STRETCH].target_position = -1;
    machine.actuator[STRETCH].step = STEP_STOPPED;
    return 1;
}

int stop_trasferitor() {
    machine.actuator[TRANSFERITOR_X].target_position = -1;
    machine.actuator[TRANSFERITOR_X].step = STEP_STOPPED;
    return 1;
}
