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



int recover(void) {
    int res;
    char str[256];

    try {
        
        if (machine.status == INITIALIZED) {
            // Inizio azione di ripristino
            machine.status = RECOVERING;

        } else if (machine.status == READY) {
            // Ripristino non necessario
            return 0;

        } else if (machine.status == AUTOMATIC) {
            // Ripristino in automatico
            return 0;

        } else if (machine.status == MANUAL) {
            // Ripristino in manuale : non necessario
            return 0;

        } else if (machine.status == STEP_BY_STEP) {
            // Ripristino in passo/passo
            return 0;


        } else if (machine.status == RECOVERING) {
            // Modalita ripristino

            if (!machine.power_on_request) {
                // Manca potenza
                strncpy(machine.status_message, "Mancanza potenza", machine.status_message_size);
                return 0;
            }

            if (machine.sequence == INIT_SEQUENCE) {

                strncpy(machine.status_message, "RECOVERY mode", machine.status_message_size);


                ////////////////////////////////////////////////////////////////
                // Avvia e Inizializza gli assi con l'eventuale azzeramento
                //
                res = process_actuators_initialize( true );

                if (res >= 0) {

                    machine.sequence = 10;

                } else {

                    ////////////////////////////
                    // Generazione Allarme                
                    //
                    strncpy(machine.status_message, "RECOVERY mode FAILED!", machine.status_message_size);

                    snprintf(str, sizeof (str), (char*) "Actuator initialize error:%d", res);
                    if (generate_alarm((char*) str, 9001, 0, (int) ALARM_ERROR, 0+1) < 0) {
                    }

                }

            } else if (machine.sequence == 10) {


                
                if (stop_spindle() < 0) {
                    // ...
                }

                if (stop_cooler_I() < 0) {
                    // ...
                }

                if (stop_cooler_II() < 0) {
                    // ...
                }

                if (stop_X() < 0) {
                    // ...
                }
                if (stop_Y() < 0) {
                    // ...
                }
                if (stop_Z() < 0) {
                    // ...
                }
                if (stop_W() < 0) {
                    // ...
                }
                if (stop_T() < 0) {
                    // ...
                }
                
                machine.sequence = END_SEQUENCE;

                
            } else if (machine.sequence == END_SEQUENCE) {

                // Fine ripristino
                machine.status = READY;
                machine.sequence = 0;
                

            } else {
                // Sequenza non contigua
                machine.sequence++;
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
