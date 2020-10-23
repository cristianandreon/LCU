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



#include <exception>




#define ENABLE_TRACE

int start_track(LP_ACTUATOR pActuator, int resolution_ms, uint32_t max_time_ms) {

    try {
        
    #ifdef ENABLE_TRACE

        if (pActuator) {
            uint32_t num_data = 0;

            if (!pActuator->pTrace) {
                pActuator->pTrace = (LP_ACTUATOR_TRACE) calloc(sizeof (ACTUATOR_TRACE), 1);
            }

            if (!pActuator->pTrace) {
                return -1;
            }

            pActuator->pTrace->start_tick = xTaskGetTickCount();
            pActuator->pTrace->end_tick = 0;
            pActuator->pTrace->resolution_ms = resolution_ms > 0 ? resolution_ms : 1;
            pActuator->pTrace->next_tick = pActuator->pTrace->start_tick + resolution_ms;

            pActuator->pTrace->num_data = 0;
            pActuator->pTrace->Status = 1;

            num_data = max_time_ms / resolution_ms;

            if (pActuator->pTrace->num_data_allocated < num_data) {
                pActuator->pTrace->num_data_allocated = num_data;

                if (pActuator->pTrace->pos) {
                    free(pActuator->pTrace->pos);
                    pActuator->pTrace->pos = NULL;
                }
                if (pActuator->pTrace->speed) {
                    free(pActuator->pTrace->speed);
                    pActuator->pTrace->speed = NULL;
                }
                if (pActuator->pTrace->acc) {
                    free(pActuator->pTrace->acc);
                    pActuator->pTrace->acc = NULL;
                }
                if (pActuator->pTrace->torque) {
                    free(pActuator->pTrace->torque);
                    pActuator->pTrace->torque = NULL;
                }

                pActuator->pTrace->pos = (float*) calloc(sizeof (float) * pActuator->pTrace->num_data_allocated, 1);
                pActuator->pTrace->speed = (float*) calloc(sizeof (float) * pActuator->pTrace->num_data_allocated, 1);
                pActuator->pTrace->acc = (float*) calloc(sizeof (float) * pActuator->pTrace->num_data_allocated, 1);
                pActuator->pTrace->torque = (float*) calloc(sizeof (float) * pActuator->pTrace->num_data_allocated, 1);
            }

            // Fallita allocazione ?
            if (!pActuator->pTrace->pos || !pActuator->pTrace->speed || !pActuator->pTrace->acc || !pActuator->pTrace->torque) {
                free(pActuator->pTrace);
                pActuator->pTrace = NULL;
                return -2;
            }



        } else {
            return -1;
        }

    #endif

    
    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "start_track() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "start_track() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }   
    
    return 1;
}

int record_track(LP_ACTUATOR pActuator) {

    try {
        
    #ifdef ENABLE_TRACE

        if (pActuator) {

            if (!pActuator->pTrace) {
                snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[No actuator trace allocated]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                    
                return -1;
            }

            if (pActuator->pTrace->Status == 1 || pActuator->pTrace->Status == 2) {
                // In fase di registrazione

                pActuator->pTrace->Status = 2;

                if (xTaskGetTickCount() >= pActuator->pTrace->next_tick) {

                    pActuator->pTrace->next_tick += pActuator->pTrace->resolution_ms;

                    if (pActuator->pTrace->num_data < pActuator->pTrace->num_data_allocated) {

                        pActuator->pTrace->pos[pActuator->pTrace->num_data] = pActuator->cur_rpos;
                        pActuator->pTrace->speed[pActuator->pTrace->num_data] = pActuator->speed;
                        pActuator->pTrace->acc[pActuator->pTrace->num_data] = pActuator->pTrace->num_data > 0 ? ((pActuator->pTrace->speed[pActuator->pTrace->num_data] - pActuator->pTrace->speed[pActuator->pTrace->num_data-1]) * 1000.0f / pActuator->pTrace->resolution_ms) : 0.0f;
                        pActuator->pTrace->torque[pActuator->pTrace->num_data] = pActuator->torque;

                        pActuator->pTrace->num_data++;

                        return 1;

                    } else {
                        // snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[No room for trace data:%d/%d]%s", (char*) ANSI_COLOR_RED, pActuator->pTrace->num_data , pActuator->pTrace->num_data_allocated, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                    
                        return -2;
                    }
                }
            }

        } else {
            snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[No actuator for trace data]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                    
            return -1;
        }

    #endif

    
    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "record_track() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "record_track() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }   
    
    return 0;
}





int end_track(LP_ACTUATOR pActuator) {

#ifdef ENABLE_TRACE

    if (pActuator) {

        if (!pActuator->pTrace) {
            return -1;
        }

        pActuator->pTrace->end_tick = xTaskGetTickCount();

        pActuator->pTrace->Status = 3;


    } else {
        return -1;
    }


#endif

    return 1;
}
