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





int do_preform_registry_shift(void) {
    uint32_t i;

    if (machine.status == AUTOMATIC || machine.status == STEP_BY_STEP || machine.status == RECOVERING) {
        for (i = machine.App.NUM_PREFORM_REGISTRY; i >= 1; i--) {
            machine.App.preform_registry[i] = machine.App.preform_registry[i - 1];
        }

        machine.App.preform_registry[0] = EMPTY;

    } else {
        // Stato macchina non valido
    }

    return 1;
}

int read_preform_temperature(void) {


    // lettura temperatura
    if (machine.App.preform_registry[machine.App.TEMPERATURE_READER_POS] == PREFORM_LOADED) {

        if (machine.actuator[PREFORM_TEMPERATURE].cur_rpos >= machine.workSet.preform_temp1 - machine.workSet.preform_temp_gap1 &&
                machine.actuator[PREFORM_TEMPERATURE].cur_rpos <= machine.workSet.preform_temp1 + machine.workSet.preform_temp_gap1) {
            machine.App.preform_registry[machine.App.TEMPERATURE_READER_POS] = PREFORM_OK;

        } else if (machine.actuator[PREFORM_TEMPERATURE].cur_rpos < machine.workSet.preform_temp1 - machine.workSet.preform_temp_gap1) {

            machine.App.preform_registry[machine.App.TEMPERATURE_READER_POS] = PREFORM_UNDER_LIMIT;

        } else if (machine.actuator[PREFORM_TEMPERATURE].cur_rpos <= machine.workSet.preform_temp1 + machine.workSet.preform_temp_gap1) {

            machine.App.preform_registry[machine.App.TEMPERATURE_READER_POS] = PREFORM_OVER_LIMIT;

        }
    }


    return 1;
}

int check_dirty_preform_registry(void) {
    uint32_t i;

    for (i = 0; i < machine.App.NUM_PREFORM_REGISTRY; i++) {
        if (machine.App.preform_registry[i] != EMPTY) {
            return TRUE;
        }
    }

    return FALSE;
}

int set_preform_registry_as_damaged(void) {
    uint32_t i;
    int retVal = 0;

    for (i = 0; i < machine.App.NUM_PREFORM_REGISTRY; i++) {
        if (machine.App.preform_registry[i] != EMPTY) {
            machine.App.preform_registry[i] = PREFORM_DAMAGED;
            retVal = 1;
        }
    }
    return retVal;
}

