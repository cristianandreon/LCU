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





int manual(void) {

    try {
        
        if (machine.status == MANUAL) {
            // Modalit� manuale
            BOOL IsMoveDone = FALSE;

            strncpy(machine.status_message, "MANUAL mode", machine.status_message_size);




            // Debug
            machine.should_recover = TRUE;

            // Richiesta passaggio in Automatico
            if (IS_IN_AUTOMATIC_REQUEST()) {
                if (machine.should_recover) {
                    // Ripristino necessario (Alterazione stato, apertura protezioni, ...)
                    machine.status = INITIALIZED;
                    on_machine_initialized();
                } else {
                    machine.status = READY;
                }
            }
        }


    
       } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "manual() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "manual() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }   
    
    return 1;
}

int put_machine_in_manual_mode() {
    int res = 0, retVal = 0;
    char str[256];

            
    
    res = 1;

    if (res >= 0) {

        machine.sequence = 10;

    } else {

        ////////////////////////////
        // Generazione Allarme                
        //
        strncpy(machine.status_message, (char*)"MANUAL mode FAILED!", machine.status_message_size);
        snprintf(str, sizeof (str), (char*) "Actuator initialize error:%d", res);
        if (generate_alarm((char*) str, 9001, 0, (int) ALARM_ERROR, 0+1) < 0) {
        }
    }

    machine.status = MANUAL;
    return retVal;
}



int check_manual_cmd_status() {
    if (machine.status == MANUAL) {
        return 1;
    } else {
        if (generate_alarm((char*) "Machine not in MANUAL Mode", 9910, 0, (int) ALARM_WARNING, 0+1) < 0) {
        }
        return 0;
    }
}