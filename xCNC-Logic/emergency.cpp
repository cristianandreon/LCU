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
            if (stop_spindle() < 0) {
            }

            // Comando Chiusura Aria secondaria
            if (stop_cooler_I() < 0) {
            }

            // Comando Apertura Scarico Aria
            if (stop_cooler_II() < 0) {
            }


            if (stop_X() < 0) {
            }
            if (stop_Z() < 0) {
            }
            if (stop_Y() < 0) {
            }            
            if (stop_W() < 0) {
            }
            if (stop_T() < 0) {
            }


            process_actuators_terminate();


            // Disattiva la potenza e Arresta i servo motori     
            // .. ritardo disattivazione ?
            if (App.DebugMode) {
                snprintf(App.Msg, App.MsgSize, "%s[POWER Still active due to Debug Mode]%s", (char*) ANSI_COLOR_MAGENTA, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            } else {
                PUT_POWER_OFF();
            }


            machine.start_request = 0;
            machine.simulate_request = 0;

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


