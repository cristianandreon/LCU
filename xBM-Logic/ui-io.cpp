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



// DEBUG : stampa la variazione del campo
int32_t GLPrintChangedVar = 0;
char GLLoginUser[256];
char GLLoginPassword[256];
char GLLoginToken[256];




extern "C" int search_for_update_value(char *actProp, uint32_t *i, bool *updateValue, bool *reverseValue, int *newValue, char *newStringValue, int newStringValueSize) {
    int retVal = 0;

    if (actProp && i) {
        uint32_t i2 = *i, i3 = *i;

        if (actProp[*i] != 0) {

            while (actProp[*i] == ' ')
                *i = *i + 1;

            if (actProp[*i] == '=') {

                retVal = 1;

                if (updateValue)
                    *updateValue = true;

                *i = *i + 1;
                i3++;

                while (actProp[*i] != 0 && (isdigit(actProp[*i]) || actProp[*i] == '-' || actProp[*i] == '+' || actProp[*i] == '.' || actProp[*i] == ',') ) {
                    *i = *i + 1;
                }


                if (actProp[*i] == '!') {
                    if (reverseValue) *reverseValue = true;
                } else {
                    if (newValue)
                        *newValue = atoi((char*) &actProp[i3]);
                }


                if (newStringValue) {
                    strncpy((char*) newStringValue, (char*) &actProp[i3], (size_t) newStringValueSize);
                }

            }

        }

        // Tronca a partire dal segno
        if (retVal > 0)
            actProp[i2] = 0;
    }

    return retVal;
}


extern "C" int is_var_equal(char *actProp, char *varNameString ) {
    if (actProp && varNameString) {
        int len = strlen(varNameString);
        if (strnicmp(actProp, varNameString, len) == 0) {
            if (actProp[len] == 0 || actProp[len] == '=' || actProp[len] == ' ') {
                return 1;
            }
        }
    }
    return 0;
}


//
// auxMode & 1 ->  Incrementa il contantore di caricamento del workSet
//
extern "C" int std_getset_var(char *var_prefix, char *actName, unsigned int *i, int keyLen,
        bool *updateValue, bool *reverseValue, char *newValue, int newValueSize,
        float *maxFValue, float *minFValue, int32_t *maxIValue, int32_t *minIValue, uint32_t  *maxUIValue, uint32_t *minUIValue,
        int *Mode, int TYPE_OF, char *str, int str_size, void *sourceVar, int sourceVarSize, int auxMode) {

    int retVal = 0;
    int cacheVarId = 0;

    // Ricerca valore da assegnare
    *i = keyLen;

    updateValue[0] = false;
    reverseValue[0] = false;
    if (newValue)
        newValue[0] = 0;

    if (search_for_update_value(actName, i, updateValue, reverseValue, NULL, newValue, newValueSize) > 0) {
        // forza la modalità scrittura
        Mode[0] |= 4;
    }


    if (Mode[0] & 1) {
        // Risoluzione nome variabile in ID (e messa in cache della risposta)
        if (addVarToCache((void*) sourceVar, TYPE_OF, &cacheVarId) > 0) {
            snprintf((char*) str, str_size, "%d", (int) cacheVarId);
        } else {
            snprintf((char*) str, str_size, "!");
        }
    } else if (Mode[0] & 4) {
        // Scrittura variabile
        if (updateValue) {
            if (maxFValue || minFValue) {
                float newFValue = atof(newValue);

                if (maxFValue)
                    if (newFValue > *maxFValue)
                        newFValue = *maxFValue;

                if (minFValue)
                    if (newFValue < *minFValue)
                        newFValue = *minFValue;


                if (sourceVar) {
                    if (sourceVarSize > 0 && sourceVarSize <= 512) {
                        if (TYPE_OF == TYPE_MSEC) {
                            newFValue = (float)(atof(newValue) * 1000.0);
                        }
                        memcpy((void*) sourceVar, (void*) &newFValue, 4);
                        retVal = 1;
                    } else {
                        retVal = -1;
                    }
                }

            } else if (maxIValue || minIValue) {
                int32_t newIValue = atoi(newValue);

                if (maxIValue)
                    if (newIValue > *maxIValue)
                        newIValue = *maxIValue;

                if (minIValue)
                    if (newIValue < *minIValue)
                        newIValue = *minIValue;


                if (sourceVar) {
                    if (sourceVarSize > 0 && sourceVarSize <= 512) {
                        if (TYPE_OF == TYPE_MSEC) {
                            newIValue = (int32_t)(atof(newValue) * 1000.0);
                        }
                        memcpy((void*) sourceVar, (void*) &newIValue, 4);
                        retVal = 1;
                    } else {
                        retVal = -1;
                    }
                }

            } else if (maxUIValue || minUIValue) {
                uint32_t newUIValue = atoi(newValue);

                if (maxUIValue)
                    if (newUIValue > *maxUIValue)
                        newUIValue = *maxUIValue;

                if (minUIValue)
                    if (newUIValue < *minUIValue)
                        newUIValue = *minUIValue;


                if (sourceVar) {
                    if (sourceVarSize > 0 && sourceVarSize <= 512) {
                        if (TYPE_OF == TYPE_MSEC) {
                            float f = (float)atof(newValue);
                            float d = (double)atof(newValue);
                            newUIValue = (int32_t)(atof(newValue) * 1000.0);
                        }
                        memcpy((void*) sourceVar, (void*) &newUIValue, 4);
                        retVal = 1;
                    } else {
                        retVal = -1;
                    }
                }

            } else {
                // Stringa (Array o Puntatore)
                char **sourceStringVar = (char **)sourceVar;
                if (sourceVarSize > 0) {
                    // Array
                    if (sourceStringVar) {
                        if (check_stri((char*)&sourceStringVar[0], (char*)newValue)) {
                            strncpy((char*)&sourceStringVar[0], (char*)newValue, MIN(sourceVarSize, strlen(newValue)) );
                        }
                    }
                } else {
                    // Puntatore
                    if (sourceStringVar) {
                        if (check_stri((char*)sourceStringVar[0], (char*)newValue)) {
                            sourceStringVar[0] = (char*)realloc(sourceStringVar[0], strlen(newValue)+32);
                            if (sourceStringVar[0]) {
                                strncpy((char*)sourceStringVar[0], (char*)newValue, strlen(newValue) );
                            }
                        }
                    }                
                }            
            }
        } else {
            handle_param_value:
            // Operatore non specificato : ritorna il valore della vaiabile
            if (TYPE_OF == TYPE_INT) {
                snprintf((char*) str, str_size, "%d", (int)((int32_t*)sourceVar)[0]);
            } else if (TYPE_OF == TYPE_MSEC) {
                snprintf((char*) str, str_size, "%0.3f", (float)( (float)(((int32_t*)sourceVar)[0]) / 1000.0f) );
            } else if (TYPE_OF == TYPE_STRING) {
                snprintf((char*) str, str_size, "%s", (char*)((char**)sourceVar)[0] );
            } else if (TYPE_OF == TYPE_FLOAT3) {
                snprintf((char*) str, str_size, "%0.3f", (float)((float*)sourceVar)[0] );
            } else if (TYPE_OF == TYPE_FLOAT2) {
                snprintf((char*) str, str_size, "%0.2f", (float)((float*)sourceVar)[0] );
            } else if (TYPE_OF == TYPE_FLOAT1) {
                snprintf((char*) str, str_size, "%0.1f", (float)((float*)sourceVar)[0] );
            } else if (TYPE_OF == TYPE_PERCENT) {
                snprintf((char*) str, str_size, "%0.1f", (float)((float*)sourceVar)[0] * 100.0f );                
            } 
        }
        
    if (auxMode & 1) {
        // Incrementa il contantore di caricamento del workSet
        machine.workSet.loadCount++;
    }
        
    } else {
        goto handle_param_value;
    }

    return retVal;
}


//////////////////////////////////////////////////////
// Calback aggiornamento campo
//
void handle_cur_alarm_update(void *var, char *newValue, void *userData) {
    if (newValue) {
        int *curAlarmList = (int*)var;
        
        machine.curAlarmList = atoi(newValue);

        if (machine.curAlarmList > machine.numAlarmList) {
            // reset : interfaccia non allineata
            machine.curAlarmList = 0;
            machine.rebuildAlarmList = 1;
        }

        // Azzera comunque la cache lastAlarm (permettere alla UI di non azzerare splicitamente, svuotanto l'eventuale lista presente)
        machine.lastAlarmListId = -1;
    }
}

void handle_dummy_update(void *var, char *newValue, void *userData) {
}


void handle_speed_lin1_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            // vel. lineare / perimetro nominale * rapp. rid.
            // pActuator->speed_auto1 = atof(newValue) * 60.0f / pActuator->cam_ratio;
            pActuator->speed_lin_auto1 = atof(newValue);
            actuator_linear_to_speed((void *)pActuator, atof(newValue), &pActuator->speed_auto1);
        }
    }
}

void handle_speed_lin2_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            // vel. lineare / perimetro nominale * rapp. rid.
            // pActuator->speed_auto2 = atof(newValue) * 60.0f / pActuator->cam_ratio;
            pActuator->speed_lin_auto2 = atof(newValue);
            actuator_linear_to_speed((void *)pActuator, atof(newValue), &pActuator->speed_auto2);
        }
    }
}

void handle_speed1_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            // giri/sec * perimetro nominale / rapp. rid.
            // pActuator->speed_lin_auto1 = atof(newValue) / 60.0f * pActuator->cam_ratio;
            pActuator->speed_auto1 = atof(newValue);
            actuator_speed_to_linear((void *)pActuator, atof(newValue), &pActuator->speed_lin_auto1);
        }
    }
}

void handle_speed2_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            // pActuator->speed_lin_auto2 = atof(newValue) / 60.0f * pActuator->cam_ratio;
            pActuator->speed_auto2 = atof(newValue);
            actuator_speed_to_linear((void *)pActuator, atof(newValue), &pActuator->speed_lin_auto2);
        }
    }
}






#define PROCESS_READONLY_VAR(__varNameString,__structName,__typeOf) \
    if (is_var_equal((char*)actProp, (char*)__varNameString)) {  \
        if (Mode & 1) { \
            if (addVarToCache((void*) &__structName, __typeOf, &cacheVarId) > 0) {  \
                snprintf((char*) str, str_size, "%d", (int) cacheVarId);    \
            } else {    \
                snprintf((char*) str, str_size, "!");   \
            }   \
        } else if (Mode & 4) {  \
        } else {    \
            /* snprintf((char*) str, str_size, "%s%s=%d", var_prefix, actName, (int)__structName); */ \
        }   \
    }

#define PROCESS_READWRITE_VAR(__varNameString,__structName,__typeOf,__use_update_func,__update_func,__user_data) \
    if (is_var_equal((char*)actProp, (char*)__varNameString)) {  \
        /* Ricerca valore da assegnare */\
        reverseValue = false;   \
        updateValue = false;    \
        i = strlen(__varNameString);  \
        if (search_for_update_value(actProp, &i, &updateValue, &reverseValue, &newValue, newSValue, sizeof(newSValue)) > 0) {    \
            /* forza la modalità scrittura */   \
            Mode |= 4;  \
        }   \
        if (Mode & 1) { \
            if (addVarToCache((void*) &__structName, __typeOf, &cacheVarId) > 0) {  \
                snprintf((char*) str, str_size, "%d", (int) cacheVarId);    \
            } else {    \
                snprintf((char*) str, str_size, "!");   \
            }   \
        } else if (Mode & 4) {  \
            /* Scrittura variabile */   \
            if (updateValue) {  \
                if (__use_update_func) {    \
                    __update_func( (void*)&__structName, newSValue, __user_data);    \
                    /* DEBUG*/\
                    if (GLPrintChangedVar) { fprintf(stderr, "[__update_func(%s,%s)]", #__structName, newSValue); fflush(stderr); } \
                } else {    \
                    if (__typeOf == TYPE_INT) {   \
                        __structName = atoi(newSValue);    \
                    /* DEBUG*/\
                    if (GLPrintChangedVar) { fprintf(stderr, "[INT(%s<-%d)]", #__structName, atoi(newSValue)); fflush(stderr); }   \
                    } else if (__typeOf == TYPE_MSEC) {   \
                        __structName = (uint32_t)(atof(newSValue?newSValue:"0") * 1000.0f);    \
                    } else if (__typeOf == TYPE_STRING) {   \
                        char *__structNameString = (char*)((int)__structName); \
                        if(__structNameString)    \
                            strncpy((char*)__structNameString, (char*)newSValue, sizeof(__structName));    \
                    } else if (__typeOf == TYPE_FLOAT3 || __typeOf == TYPE_FLOAT2 || __typeOf == TYPE_FLOAT1) {    \
                        __structName = atof(newSValue);    \
                    /* DEBUG*/\
                    if (GLPrintChangedVar) { fprintf(stderr, "[FLOAT(%s<-%0.3f)]", #__structName, atof(newSValue)); fflush(stderr); }  \
                    } else if (__typeOf == TYPE_PERCENT) {    \
                        __structName = atof(newSValue) / 100.0f;    \
                    /* DEBUG*/\
                    if (GLPrintChangedVar) { fprintf(stderr, "[PERCENT(%s<-%0.3f)]", #__structName, atof(newSValue) / 100.0f); fflush(stderr); }  \
                    }   \
                }   \
            } else {    \
                /* Operatore omesso : ritorno del valore della variabile*/    \
                if (__typeOf == TYPE_INT) {   \
                    snprintf((char*) str, str_size, "%d", (int)__structName); \
                } else if (__typeOf == TYPE_MSEC) {   \
                    snprintf((char*) str, str_size, "%0.3f", (float)(__structName / 1000.0f) ); \
                } else if (__typeOf == TYPE_STRING) {   \
                    snprintf((char*) str, str_size, "%s", (char*)((int)__structName)); \
                } else if (__typeOf == TYPE_FLOAT3) {    \
                    snprintf((char*) str, str_size, "%0.3f", (float)(__structName) ); \
                } else if (__typeOf == TYPE_FLOAT2) {    \
                    snprintf((char*) str, str_size, "%0.2f", (float)(__structName) ); \
                } else if (__typeOf == TYPE_FLOAT1) {    \
                    snprintf((char*) str, str_size, "%0.1f", (float)(__structName) ); \
                } else if (__typeOf == TYPE_PERCENT) {    \
                    snprintf((char*) str, str_size, "%0.1f", (float)(__structName * 100.0f) ); \
                }   \
            }   \
        } else {    \
            /* snprintf((char*) str, str_size, "%s%s=%d", var_prefix, actName, (int)__structName); */ \
        }   \
    }







//////////////////////////////////////////////////////////////////////////////////////////////
// Ritorna il valore della variabile
//
// Mode = 0+0   ->  Esecuzione
// Mode = 0+1   ->  Risoluzione nome variabile in ID (e messa in cache della risposta)
// Mode = 0+2   ->  Risoluzione variabile per ID dalla cache
// Mode = 0+4   ->  Riservato (Scrittura variabile)
// Mode = 0+8   ->  Autenticazione utente
// Mode = 0+16   ->  Pacchetto da inoltrare (trasparente alla UI)

extern "C" int logic_get_value(uint8_t *var_name, uint8_t *out_str, uint32_t *out_str_size, int Mode) {
    int retVal = 0;
    int cacheVarId = 0;
    uint32_t nVar = 0;
    size_t len = strlen((char*) out_str), lenAdding = 0;
    char *svar_name = (char *) var_name, *pvar_name = NULL;

    uint32_t i = 0, found1B = 0;
    int newValue = 0;

    char *actName = NULL;
    char *actProp = NULL;

    uint32_t str_size = var_name ? (strlen((char*)var_name) + 64) : 32;
    char *str = (char *) calloc(str_size, 1);
    char *var_prefix = (char *) calloc(255, 1);

    size_t debug_str_size = 256;

    int j;
    char str2[32];

    char newSValue[512];

    bool reverseValue = false;
    bool updateValue = false;

    int rowPos = 0;

    float maxFValue = 0.0f, minFValue = 0.0f;
    int32_t maxIValue = 0, minIValue = 0;
    uint32_t maxUIValue = 0, minUIValue = 0;



    try {


        /// printf("logic_get_value() IN:'%s'\n", var_name); fflush (stdout);

        if (out_str)
            *out_str = 0;

        if (!str)
            return -11;

        if (!var_prefix) {
            if (str)
                free(str);
            return -12;
        }


        str[0] = 0;
        var_prefix[0] = 0;



        //////////////////////////////////////////////
        // Risoluzione delle stringhe di debug
        //


        // Registro sequenze macchina (debug)
        if (!machine.debug_string[0])
            machine.debug_string[0] = (char*)calloc(debug_str_size, 1);

        if (machine.debug_string[0]) {
            // snprintf( (char*)machine.debug_string[0], debug_str_size, "XXX");
            snprintf((char*) machine.debug_string[0], debug_str_size, "(%d)[%d][%d][%d](%d)(%d)(%d)",
                    (int) machine.sequence,
                    (int) machine.App.mold_sequence, (int) machine.App.transferitor_sequence, (int) machine.App.load_sequence,
                    (int) machine.App.manip_pref_sequence, (int) machine.App.pit_lock_sequence, (int) machine.App.owens_sequence);

            // fprintf(stderr, "{%s}\n", machine.debug_string[0]); fflush (stderr);
        }


        if (!machine.debug_string[1])
            machine.debug_string[1] = (char*)calloc(debug_str_size, 1);



        if (machine.debug_string[1]) {
            // Registro preforme (debug)

            snprintf((char*) machine.debug_string[1], debug_str_size, "[");
            for (j = 0; j < machine.App.NUM_PREFORM_REGISTRY; j++) {
                snprintf((char*) str2, sizeof (str2), "%d", machine.App.preform_registry[j]);
                strncat((char*) machine.debug_string[1], str2, (size_t) debug_str_size - (size_t) strlen(machine.debug_string[1]) - (size_t) strlen(str2));
            }
            strncat((char*) machine.debug_string[1], "]", (size_t) debug_str_size - (size_t) strlen(machine.debug_string[1]) - (size_t) 1);

            // fprintf(stderr, "{%s}\n", machine.debug_string[1]); fflush (stderr);
        }





        // Stampa le variabili registrate
        // if (Mode & 1) fprintf(stdout, "[PARSE:%s]", svar_name); fflush (stdout);


        while (svar_name) {

            pvar_name = strstr((char*) svar_name, (char*) App.RowSep);

            if (pvar_name)
                *pvar_name = 0;


            if (svar_name) {

                if (svar_name[0]) {

                    // Debug
                    // if (Mode & 1) printf("[svar_name:%s]", (char*)svar_name);

                    str[0] = 0;
                    var_prefix[0] = 0;


                    if (Mode & 2) {
                        // Risoluzione variabile per ID dalla cache
                        getVarFromCache(atoi(svar_name), &str, &str_size);

                        // Debug
                        // printf("[CACHE:%d=%s]", (int)atoi(svar_name), str); fflush (stdout);

                    } else {
                        // Risoluzione per nome o Risoluzione + archivio nella cache

                        if (Mode & 1) {
                            int lb = 1;
                        }


                        if (strnicmp(svar_name, "mac.", 4) == 0) {
                            i = 0;
                            found1B = 0;
                            actName = svar_name + 4;
                            actProp = actName;

                            strncpy(var_prefix, svar_name, 4);

                            while (actName[i] != 0 && actName[i] != '.' && actName[i] != '=' && actName[i] != ' ') {
                                i++;
                            }


                            // N.B.: Non supporta la modalita diretta (obsoleta)

                            PROCESS_READONLY_VAR("prgVer", machine.prgVer, TYPE_CHAR)
                            else PROCESS_READONLY_VAR("stat", machine.status, TYPE_MAC_STAT)
                            else PROCESS_READONLY_VAR("stat_seq", machine.debug_string[0], TYPE_STRING)
                            else PROCESS_READONLY_VAR("stat_time", machine.time_message, TYPE_STRING)
                            else PROCESS_READONLY_VAR("stat_msg", machine.status_message, TYPE_STRING)
                            else PROCESS_READONLY_VAR("nAct", machine.num_actuator, TYPE_INT)
                                    
                            else PROCESS_READONLY_VAR("power", machine.power_on_request, TYPE_BOOL)
                            else PROCESS_READONLY_VAR("load", machine.App.load_on, TYPE_BOOL)
                            else PROCESS_READONLY_VAR("single_load", machine.App.single_load_on, TYPE_BOOL)
                            else PROCESS_READONLY_VAR("heat", machine.App.heat_on, TYPE_BOOL)
                            else PROCESS_READONLY_VAR("blow", machine.App.blow_on, TYPE_BOOL)                                   
                            else PROCESS_READONLY_VAR("inline_output", machine.App.inline_output_on, TYPE_BOOL)
                                    
                            else PROCESS_READONLY_VAR("NumPrefReg", machine.App.NUM_PREFORM_REGISTRY, TYPE_INT)
                            else PROCESS_READONLY_VAR("PrefReg", machine.debug_string[1], TYPE_STRING)

                                    
                            else PROCESS_READONLY_VAR("task_cycles", machine.task_cycles, TYPE_INT)
                                    
                                    

                                ///////////////////////////////////////// 
                                // Parametri lavoro    
                                //  
                            else PROCESS_READONLY_VAR("moldCycleTime", machine.App.mold_cycle_time_ms, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("prodCycleTime", machine.App.production_cycle_time_ms, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("transferitorCycleTime", machine.App.transferitor_cycle_time_ms, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("manipPrefCycleTime", machine.App.manip_pref_cycle_time_ms, TYPE_MSEC)

                            else PROCESS_READONLY_VAR("airTime", machine.App.secondary_air_persistence_time_ms, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("secondaryAirTime", machine.App.secondary_air_persistence_time_ms, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("primaryAirPress", machine.App.primmary_air_press, TYPE_FLOAT1)
                            else PROCESS_READONLY_VAR("secondaryAirPress", machine.App.secondary_air_press, TYPE_FLOAT1)
                            else PROCESS_READONLY_VAR("dischargeAirTime", machine.App.discharge_needed_time_ms, TYPE_MSEC)


                            
                            else PROCESS_READONLY_VAR("IOTimeout", machine.io_timeout_ms, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("IOTimeoutCount", machine.io_timeout_count, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("SCRTimeout", machine.scr_timeout_ms, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("SCRTimeoutCount", machine.scr_timeout_count, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("UITimeout", machine.ui_timeout_ms, TYPE_MSEC)
                            else PROCESS_READONLY_VAR("UITimeoutCount", machine.ui_timeout_count, TYPE_INT)





                                ///////////////////////
                                // Gestione allarmi
                                //
                            else PROCESS_READONLY_VAR("alarmCount", machine.numAlarmList, TYPE_INT)
                            else PROCESS_READONLY_VAR("lastAlarms", machine.alarmList, TYPE_LAST_ALARM_LIST)
                            else PROCESS_READONLY_VAR("alarmList", machine.alarmList, TYPE_ALARM_LIST)
                            else PROCESS_READWRITE_VAR("curAlarm", machine.curAlarmList, TYPE_INT, true, handle_cur_alarm_update, NULL)






                                //////////////////////////
                                // Azioni
                                //
                            else if (strcmpi(actName, "AUTO") == 0) {
                                PUT_AUTOMATIC();
                                str[0] = 0;

                            } else if (strcmpi(actName, "MAN") == 0) {
                                PUT_MANUAL();
                                str[0] = 0;

                            } else if (strcmpi(actName, "EMERGENCY") == 0) {
                                PUT_EMERGENCY();
                                str[0] = 0;

                            } else if (strcmpi(actName, "START") == 0) {
                                PUT_START_CYCLE();
                                str[0] = 0;

                            } else if (strcmpi(actName, "STOP") == 0) {
                                PUT_STOP_CYCLE();
                                str[0] = 0;


                            } else if (strcmpi(actName, "POWER_ON") == 0) {
                                PUT_POWER_ON();
                                str[0] = 0;

                            } else if (strcmpi(actName, "POWER_OFF") == 0) {
                                PUT_POWER_OFF();
                                str[0] = 0;

                            } else if (strcmpi(actName, "LOAD_ON") == 0) {
                                PUT_LOAD_ON();
                                str[0] = 0;

                            } else if (strcmpi(actName, "LOAD_OFF") == 0) {
                                PUT_LOAD_OFF();
                                str[0] = 0;

                            } else if (strcmpi(actName, "SINGLE_LOAD_ON") == 0) {
                                PUT_SINGLE_LOAD_ON();
                                str[0] = 0;

                            } else if (strcmpi(actName, "SINGLE_LOAD_OFF") == 0) {
                                PUT_SINGLE_LOAD_OFF();
                                str[0] = 0;


                            } else if (strcmpi(actName, "BLOW_ON") == 0) {
                                PUT_BLOW_ON();
                                str[0] = 0;

                            } else if (strcmpi(actName, "BLOW_OFF") == 0) {
                                PUT_BLOW_OFF();
                                str[0] = 0;

                            } else if (strcmpi(actName, "HEAT_ON") == 0) {
                                PUT_HEAT_ON();
                                str[0] = 0;

                            } else if (strcmpi(actName, "HEAT_OFF") == 0) {
                                PUT_HEAT_OFF();
                                str[0] = 0;

                            } else if (strcmpi(actName, "INLINE_OUTPUT_ON") == 0) {
                                PUT_INLINE_OUTPUT_ON();
                                str[0] = 0;

                            } else if (strcmpi(actName, "INLINE_OUTPUT_OFF") == 0) {
                                PUT_INLINE_OUTPUT_OFF();
                                str[0] = 0;

                            } else if (strcmpi(actName, "RESET_ALARM") == 0) {
                                PUT_RESET_ALARMS();
                                str[0] = 0;

                            } else if (strcmpi(actName, "AUTOSTART") == 0) {
                                PUT_RESET_ALARMS();
                                PUT_AUTOMATIC();
                                PUT_RESET_ALARMS();
                                PUT_HEAT_ON();                               
                                PUT_POWER_ON();
                                PUT_LOAD_ON();
                                PUT_SINGLE_LOAD_ON();
                                PUT_BLOW_ON();
                                PUT_START_CYCLE();
                                
                                str[0] = 0;

                            } else {
                                // snprintf( (char*)str, str_size, "%s%s=[!]", var_prefix, (actName?actName:""));
                                snprintf((char*) str, str_size, "%s=[!]", svar_name);
                            }




                            /////////////////////////////////////    
                            // Gestione Parametri ricetta
                            //
                        } else if (strnicmp(svar_name, "settings.", 9) == 0) {

                            i = 0;
                            found1B = 0;
                            actName = svar_name + 9;
                            actProp = NULL;

                            strncpy(var_prefix, svar_name, 9);

                            while (actName[i] != 0 && actName[i] != '.' && actName[i] != '=' && actName[i] != ' ') {
                                i++;
                            }

                            if (actName[i] == '.') {
                                actName[i] = 0;
                                actProp = actName;
                                actProp += i + 1;
                            }

                            // Ricerca valore da assegnare
                            reverseValue = false;
                            updateValue = false;

                            i = 0;
                            if (search_for_update_value(actProp, &i, &updateValue, &reverseValue, &newValue, NULL, 0) > 0) {
                                // forza la modalità scrittura
                                Mode |= 4;
                            }


                            if (strcmpi(actName, "...") == 0) {

                            } else if (strnicmp(actName, "startup_owens_cycles", 20) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 20,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.settings.startup_owens_cycles, sizeof (machine.settings.startup_owens_cycles), 0+0);
                            } else if (strnicmp(actName, "startup_owens_delay_ms", 22) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.startup_owens_delay_ms, sizeof (machine.settings.startup_owens_delay_ms), 0+0);
                            } else if (strnicmp(actName, "turnoff_owens_cycles", 20) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 20,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.turnoff_owens_cycles, sizeof (machine.settings.turnoff_owens_cycles), 0+0);
                            } else if (strnicmp(actName, "turnoff_owens_delay_ms", 22) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.turnoff_owens_delay_ms, sizeof (machine.settings.turnoff_owens_delay_ms), 0+0);
                            } else if (strnicmp(actName, "owens_standby_cycles", 20) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 20,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.settings.owens_standby_cycles, sizeof (machine.settings.owens_standby_cycles), 0+0);
                            } else if (strnicmp(actName, "owens_standby_delay_ms", 22) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.owens_standby_delay_ms, sizeof (machine.settings.owens_standby_delay_ms), 0+0);
                            } else if (strnicmp(actName, "owens_towork_cycles", 19) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 19,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.owens_towork_cycles, sizeof (machine.settings.owens_towork_cycles), 0+0);
                            } else if (strnicmp(actName, "owens_towork_delay_ms", 21) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 21,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.owens_towork_delay_ms, sizeof (machine.settings.owens_towork_delay_ms), 0+0);
                            } else if (strnicmp(actName, "initial_owens_cycles", 20) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 20,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.settings.initial_owens_cycles, sizeof (machine.settings.initial_owens_cycles), 0+0);
                            } else if (strnicmp(actName, "initial_owens_ratio", 19) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 19,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_FLOAT2, str, str_size, (void *) &machine.settings.initial_owens_ratio, sizeof (machine.settings.initial_owens_ratio), 0+0);    
                            } else if (strnicmp(actName, "chain_stepper1_pause_ms", 23) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 23,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.chain_stepper1_pause_ms, sizeof (machine.settings.chain_stepper1_pause_ms), 0+0);
                            } else if (strnicmp(actName, "chain_stepper2_pause_ms", 23) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 23,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.chain_stepper2_pause_ms, sizeof (machine.settings.chain_stepper2_pause_ms), 0+0);
                            } else if (strnicmp(actName, "chain_stepper3_pause_ms", 23) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 23,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.chain_stepper3_pause_ms, sizeof (machine.settings.chain_stepper3_pause_ms), 0+0);
                            } else if (strnicmp(actName, "trasf_x_forward_pause_ms", 24) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 24,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.trasf_x_forward_pause_ms, sizeof (machine.settings.trasf_x_forward_pause_ms), 0+0);
                            } else if (strnicmp(actName, "chain_trasf_z_down_pause_ms", 27) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 27,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.chain_trasf_z_down_pause_ms, sizeof (machine.settings.chain_trasf_z_down_pause_ms), 0+0);
                            } else if (strnicmp(actName, "chain_picker_open_pause_ms", 26) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 26,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.chain_picker_open_pause_ms, sizeof (machine.settings.chain_picker_open_pause_ms), 0+0);
                            } else if (strnicmp(actName, "chain_picker_close_pause_ms", 25) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 25,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.chain_picker_close_pause_ms, sizeof (machine.settings.chain_picker_close_pause_ms), 0+0);
                            } else if (strnicmp(actName, "pref_load_inside_pause_ms", 27) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 27,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.pref_load_inside_pause_ms, sizeof (machine.settings.pref_load_inside_pause_ms), 0+0);
                            } else if (strnicmp(actName, "pref_load_outside_pause_ms", 26) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 26,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.pref_load_outside_pause_ms, sizeof (machine.settings.pref_load_outside_pause_ms), 0+0);
                            } else if (strnicmp(actName, "pit_stopper_inside_pause_ms", 25) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 25,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.pit_stopper_inside_pause_ms, sizeof (machine.settings.pit_stopper_inside_pause_ms), 0+0);
                            } else if (strnicmp(actName, "pit_stopper_outside_pause_ms", 26) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 26,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.pit_stopper_outside_pause_ms, sizeof (machine.settings.pit_stopper_outside_pause_ms), 0+0);
                            } else if (strnicmp(actName, "aspirator_delay_ms", 18) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 18,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.settings.aspirator_delay_ms, sizeof (machine.settings.aspirator_delay_ms), 0+0);
                            }




                            /////////////////////////////////////    
                            // Gestione Parametri ricetta
                            //
                        } else if (strnicmp(svar_name, "workSet.", 8) == 0) {

                            i = 0;
                            found1B = 0;
                            actName = svar_name + 8;
                            actProp = NULL;

                            strncpy(var_prefix, svar_name, 8);

                            while (actName[i] != 0 && actName[i] != '.' && actName[i] != '=' && actName[i] != ' ') {
                                i++;
                            }

                            if (actName[i] == '.') {
                                actName[i] = 0;
                                actProp = actName;
                                actProp += i + 1;
                            }

                            // Ricerca valore da assegnare
                            reverseValue = false;
                            updateValue = false;

                            if (Mode == 0) {
                                int lb = 1;
                            }
                            
                            i = 0;
                            if (search_for_update_value(actProp, &i, &updateValue, &reverseValue, &newValue, NULL, 0) > 0) {
                                // forza la modalità scrittura
                                Mode |= 4;
                            }


                            if (strcmpi(actName, "...") == 0) {


                            } else if (strnicmp(actName, "loadCount", 9) == 0) {
                                std_getset_var(var_prefix, actName, &i, 9,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.workSet.loadCount, sizeof (machine.workSet.loadCount), 0+0);
                            } else if (strnicmp(actName, "name", 4) == 0) {
                                std_getset_var(var_prefix, actName, &i, 4,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.workSet.name, sizeof (machine.workSet.name), 0+1);
                            } else if (strnicmp(actName, "description", 11) == 0) {
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.workSet.description, sizeof (machine.workSet.description), 0+0);

                            } else if (strnicmp(actName, "global_heat_ratio1", 18) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 18,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.global_heat_ratio1, sizeof (machine.workSet.global_heat_ratio1), 0+0);

                            } else if (strnicmp(actName, "global_heat_ratio2", 18) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 18,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.global_heat_ratio2, sizeof (machine.workSet.global_heat_ratio2), 0+0);

                            } else if (strnicmp(actName, "stretch_bottom_gap", 18) == 0) {
                                maxFValue = 50.0f, minFValue = -1.0f;
                                std_getset_var(var_prefix, actName, &i, 18,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.stretch_bottom_gap_mm, sizeof (machine.workSet.stretch_bottom_gap_mm), 0+0);

                            } else if (strnicmp(actName, "primary_air_gap", 15) == 0) {
                                maxFValue = 750.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.primary_air_gap_mm, sizeof (machine.workSet.primary_air_gap_mm), 0+0);

                            } else if (strnicmp(actName, "secondary_air_gap", 17) == 0) {
                                maxUIValue = 999999, minUIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 17,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.workSet.secondary_air_gap_ms, sizeof (machine.workSet.secondary_air_gap_ms), 0+0);
        
                            } else if (strnicmp(actName, "min_secondary_air_time", 22) == 0) {
                                maxUIValue = 999999, minUIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.workSet.min_secondary_air_time_ms, sizeof (machine.workSet.min_secondary_air_time_ms), 0+0);

                            } else if (strnicmp(actName, "discharge_air_time", 18) == 0) {
                                maxUIValue = 999999; minUIValue = 0;
                                std_getset_var(var_prefix, actName, &i, 18,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.workSet.discharge_air_time_ms, sizeof (machine.workSet.discharge_air_time_ms), 0+0);

                            } else if (strnicmp(actName, "owen1_min_temp", 14) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 14,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.owen1_min_temp, sizeof (machine.workSet.owen1_min_temp), 0+0);
                            } else if (strnicmp(actName, "owen2_min_temp", 14) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 14,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.owen2_min_temp, sizeof (machine.workSet.owen2_min_temp), 0+0);
                            } else if (strnicmp(actName, "owen1_max_temp", 14) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 14,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.owen1_max_temp, sizeof (machine.workSet.owen1_max_temp), 0+0);
                            } else if (strnicmp(actName, "owen2_max_temp", 14) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 14,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.owen2_max_temp, sizeof (machine.workSet.owen2_max_temp), 0+0);



                            } else if (strnicmp(actName, "preform_temp1", 13) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.preform_temp1, sizeof (machine.workSet.preform_temp1), 0+0);
                            } else if (strnicmp(actName, "preform_temp2", 13) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.preform_temp2, sizeof (machine.workSet.preform_temp2), 0+0);
                            } else if (strnicmp(actName, "preform_temp3", 13) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.preform_temp3, sizeof (machine.workSet.preform_temp3), 0+0);
                                
                            } else if (strnicmp(actName, "preform_temp_gap1", 17) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 17,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.preform_temp_gap1, sizeof (machine.workSet.preform_temp_gap1), 0+0);
                            } else if (strnicmp(actName, "preform_temp_gap2", 17) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 17,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.preform_temp_gap2, sizeof (machine.workSet.preform_temp_gap2), 0+0);
                            } else if (strnicmp(actName, "preform_temp_gap3", 17) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 17,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.preform_temp_gap3, sizeof (machine.workSet.preform_temp_gap3), 0+0);


                            } else if (strnicmp(actName, "init_heat_ratio1", 16) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 16,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.init_heat_ratio1, sizeof (machine.workSet.init_heat_ratio1), 0+0);
                            } else if (strnicmp(actName, "init_heat_ratio2", 16) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 16,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.init_heat_ratio2, sizeof (machine.workSet.init_heat_ratio2), 0+0);

                            } else if (strnicmp(actName, "global_heat_ratio1", 16) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 16,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.global_heat_ratio1, sizeof (machine.workSet.global_heat_ratio1), 0+0);
                            } else if (strnicmp(actName, "global_heat_ratio2", 18) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 18,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.global_heat_ratio2, sizeof (machine.workSet.global_heat_ratio2), 0+0);

                            } else if (strnicmp(actName, "standby_heat_ratio1", 19) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 19,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.standby_heat_ratio1, sizeof (machine.workSet.standby_heat_ratio1), 0+0);
                            } else if (strnicmp(actName, "standby_heat_ratio2", 19) == 0) {
                                maxFValue = 1000.0f, minFValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 19,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.workSet.standby_heat_ratio2, sizeof (machine.workSet.standby_heat_ratio2), 0+0);
                            } else if (strnicmp(actName, "fan_motor_time_msec", 19) == 0) {
                                maxUIValue = 999999; minUIValue = 0;
                                std_getset_var(var_prefix, actName, &i, 19,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.workSet.fan_motor_time_msec, sizeof (machine.workSet.fan_motor_time_msec), 0+0);
                            } else if (strnicmp(actName, "roll_motor_time_msec", 20) == 0) {
                                maxUIValue = 999999; minUIValue = 0;
                                std_getset_var(var_prefix, actName, &i, 20,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.workSet.roll_motor_time_msec, sizeof (machine.workSet.roll_motor_time_msec), 0+0);
                            } else if (strnicmp(actName, "aspiration_persistence_time_msec", 32) == 0) {
                                minUIValue = 0;
                                std_getset_var(var_prefix, actName, &i, 32,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.workSet.aspiration_persistence_time_msec, sizeof (machine.workSet.aspiration_persistence_time_msec), 0+0);
                            } else if (strnicmp(actName, "preform_elevator_time_msec", 16) == 0) {
                                maxUIValue = 999999; minUIValue = 0;
                                std_getset_var(var_prefix, actName, &i, 16,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.workSet.preform_elevator_time_msec, sizeof (machine.workSet.preform_elevator_time_msec), 0+0);
                            } else if (strnicmp(actName, "unjammer_time_msec", 18) == 0) {
                                maxUIValue = 999999; minUIValue = 0;
                                std_getset_var(var_prefix, actName, &i, 18,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.workSet.unjammer_time_msec, sizeof (machine.workSet.unjammer_time_msec), 0+0);



                            } else if (strnicmp(actName, "production", 10) == 0) {
                                maxUIValue = 2500, minUIValue = 1;
                                std_getset_var(var_prefix, actName, &i, 10,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, &maxUIValue, &minUIValue,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.workSet.production, sizeof (machine.workSet.production), 0+0);



                            } else if (strcmpi(actName, "rRow") == 0) {
                                // Ratio irraggiatore

                                if (Mode == 0) {
                                    int lb = 1;
                                }
                                if (strnicmp(actProp, "1.", 2) == 0 || strnicmp(actProp, "2.", 2) == 0) {
                                    int curOwen = 0;
                                    float *owen_row = NULL;

                                    if (strnicmp(actProp, "1.", 2) == 0) {
                                        // 1.x
                                        curOwen = 1;
                                        owen_row = (float*)machine.workSet.owen1_row;
                                    } else if (strnicmp(actProp, "2.", 2) == 0) {
                                        curOwen = 2;                                                
                                        owen_row = (float*)machine.workSet.owen2_row;
                                    }
    
                                    // ricerca riga oggetto
                                    i = 2;
                                    while (actProp[i] != '.' && actProp[i] != '=' && actProp[i] != ' ' && actProp[i] != '\0')
                                        i++;
                                    
                                    if (actProp[i] == '=') {                                        
                                        actProp[i] = 0;
                                        rowPos = atoi((char*) &actProp[2]);
                                        actProp[i] = '=';
                                        if (search_for_update_value(actProp, &i, &updateValue, &reverseValue, &newValue, NULL, 0) > 0) {
                                            // forza la modalità scrittura
                                            Mode |= 4;
                                        }
                                    } else {
                                        actProp[i] = 0;
                                        rowPos = atoi((char*) &actProp[2]);                                        
                                    }
                                    
                                    if ( (curOwen == 1 || curOwen == 2) && rowPos > 0 && rowPos <= MAX_OWENS_ROWS) {

                                        if (Mode & 1) {
                                            // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                            if (addVarToCache((void*) &owen_row[rowPos-1], TYPE_PERCENT, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (Mode & 4) {
                                            // Scrittura variabile
                                            if (updateValue) {
                                                if (reverseValue) {
                                                    // achine.work_param.owen1_row[rowPos] = ???
                                                } else {
                                                    owen_row[rowPos-1] = newValue / 100.0f;
                                                }
                                            } else {
                                                snprintf((char*) str, str_size, "workSet.%s.%s=%0.1f", actName, actProp, owen_row[rowPos-1] * 100.0f);
                                            }
                                        }
                                    }                                    
                                }

                            } else if (strcmpi(actName, "pRow") == 0) {
                                // Potenza irraggiatore

                                if (Mode == 0) {
                                    int lb = 1;
                                }
                                if (strnicmp(actProp, "1.", 2) == 0 || strnicmp(actProp, "2.", 2) == 0) {
                                    int curOwen = 0;
                                    float *owen_power = NULL;

                                    if (strnicmp(actProp, "1.", 2) == 0) {
                                        // 1.x
                                        curOwen = 1;
                                        owen_power = (float*)machine.workSet.owen1_power;
                                    } else if (strnicmp(actProp, "2.", 2) == 0) {
                                        curOwen = 2;                                                
                                        owen_power = (float*)machine.workSet.owen2_power;
                                    }
    
                                    // ricerca riga oggetto
                                    i = 2;
                                    while (actProp[i] != '.' && actProp[i] != '=' && actProp[i] != ' ' && actProp[i] != '\0')
                                        i++;
                                    
                                    if (actProp[i] == '=') {                                        
                                        actProp[i] = 0;
                                        rowPos = atoi((char*) &actProp[2]);
                                        actProp[i] = '=';
                                        if (search_for_update_value(actProp, &i, &updateValue, &reverseValue, &newValue, NULL, 0) > 0) {
                                            // forza la modalità scrittura
                                            Mode |= 4;
                                        }
                                    } else {
                                        actProp[i] = 0;
                                        rowPos = atoi((char*) &actProp[2]);                                        
                                    }
                                    
                                    if (curOwen == 1 || curOwen == 2) {                                   

                                        if (Mode & 1) {
                                            // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                            if (addVarToCache((void*) &owen_power[rowPos], TYPE_FLOAT1, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (Mode & 4) {
                                            // Scrittura variabile
                                            if (updateValue) {
                                                if (reverseValue) {
                                                    // achine.work_param.owen1_row[rowPos] = ???
                                                } else {
                                                    owen_power[rowPos] = newValue;
                                                }
                                            } else {
                                                snprintf((char*) str, str_size, "workSet.%s.%s=%0.3f", actName, actProp, owen_power[rowPos]);
                                            }
                                        }
                                    }
                                }
                            }


                            /////////////////////////////////////    
                            // Statistiche
                            //

                        } else if (strnicmp(svar_name, "STAT.", 5) == 0) {

                            i = 0;
                            found1B = 0;
                            actName = svar_name + 5;
                            actProp = NULL;

                            strncpy(var_prefix, svar_name, 5);

                            while (actName[i] != 0 && actName[i] != '.' && actName[i] != '=' && actName[i] != ' ') {
                                i++;
                            }

                            if (actName[i] == '.') {
                                actName[i] = 0;
                                actProp = actName;
                                actProp += i + 1;
                            }


                            if (strcmpi(actName, "...") == 0) {

                            } else if (strnicmp(actName, "preforms_loaded", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.preforms_loaded, sizeof (machine.statistic.preforms_loaded), 0+0);

                            } else if (strnicmp(actName, "preforms_blowed", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.preforms_blowed, sizeof (machine.statistic.preforms_blowed), 0+0);

                            } else if (strnicmp(actName, "bottles", 7) == 0) {
                                std_getset_var(var_prefix, actName, &i, 7,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.bottles, sizeof (machine.statistic.bottles), 0+0);

                            } else if (strnicmp(actName, "discharged", 10) == 0) {
                                std_getset_var(var_prefix, actName, &i, 10,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.discharged, sizeof (machine.statistic.discharged), 0+0);


                            } else if (strnicmp(actName, "mold_cycles", 11) == 0) {
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.mold_cycles, sizeof (machine.statistic.mold_cycles), 0+0);

                            } else if (strnicmp(actName, "fatal_errors", 12) == 0) {
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.fatal_errors, sizeof (machine.statistic.fatal_errors), 0+0);

                            } else if (strnicmp(actName, "errors", 6) == 0) {
                                std_getset_var(var_prefix, actName, &i, 6,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.errors, sizeof (machine.statistic.errors), 0+0);

                            } else if (strnicmp(actName, "warnings", 8) == 0) {
                                std_getset_var(var_prefix, actName, &i, 8,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.warnings, sizeof (machine.statistic.warnings), 0+0);
                                
                            } else if (strnicmp(actName, "machine_stops", 13) == 0) {
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.machine_stops, sizeof (machine.statistic.machine_stops), 0+0);

                            } else if (strnicmp(actName, "machine_elapsed_string", 22) == 0) {
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.statistic.machine_elapsed_string, sizeof (machine.statistic.machine_elapsed_string), 0+0);
                            
                            } else if (strnicmp(actName, "machine_running_string", 22) == 0) {
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.statistic.machine_running_string, sizeof (machine.statistic.machine_running_string), 0+0);

                            } else if (strnicmp(actName, "machine_elapsed", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.machine_elapsed, sizeof (machine.statistic.machine_elapsed), 0+0);

                            } else if (strnicmp(actName, "machine_running", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.machine_running, sizeof (machine.statistic.machine_running), 0+0);
                            }


                            
                            
                        } else if (strnicmp(svar_name, "RT_STAT.", 8) == 0) {

                            i = 0;
                            found1B = 0;
                            actName = svar_name + 8;
                            actProp = NULL;

                            strncpy(var_prefix, svar_name, 8);

                            while (actName[i] != 0 && actName[i] != '.' && actName[i] != '=' && actName[i] != ' ') {
                                i++;
                            }

                            if (actName[i] == '.') {
                                actName[i] = 0;
                                actProp = actName;
                                actProp += i + 1;
                            }


                            if (strcmpi(actName, "...") == 0) {

                            } else if (strnicmp(actName, "preforms_loaded", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.preforms_loaded, sizeof (machine.rt_statistic.preforms_loaded), 0+0);

                            } else if (strnicmp(actName, "preforms_blowed", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.preforms_blowed, sizeof (machine.rt_statistic.preforms_blowed), 0+0);

                            } else if (strnicmp(actName, "bottles", 7) == 0) {
                                std_getset_var(var_prefix, actName, &i, 7,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.bottles, sizeof (machine.rt_statistic.bottles), 0+0);

                            } else if (strnicmp(actName, "discharged", 10) == 0) {
                                std_getset_var(var_prefix, actName, &i, 10,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.discharged, sizeof (machine.rt_statistic.discharged), 0+0);


                            } else if (strnicmp(actName, "mold_cycles", 11) == 0) {
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.mold_cycles, sizeof (machine.rt_statistic.mold_cycles), 0+0);

                            } else if (strnicmp(actName, "fatal_errors", 12) == 0) {
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.fatal_errors, sizeof (machine.rt_statistic.fatal_errors), 0+0);

                            } else if (strnicmp(actName, "errors", 6) == 0) {
                                std_getset_var(var_prefix, actName, &i, 6,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.errors, sizeof (machine.rt_statistic.errors), 0+0);

                            } else if (strnicmp(actName, "warnings", 8) == 0) {
                                std_getset_var(var_prefix, actName, &i, 8,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.warnings, sizeof (machine.rt_statistic.warnings), 0+0);

                            } else if (strnicmp(actName, "machine_stops", 13) == 0) {
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.machine_stops, sizeof (machine.rt_statistic.machine_stops), 0+0);

                            } else if (strnicmp(actName, "machine_elapsed_string", 22) == 0) {
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.rt_statistic.machine_elapsed_string, sizeof (machine.rt_statistic.machine_elapsed_string), 0+0);
                            
                            } else if (strnicmp(actName, "machine_running_string", 22) == 0) {
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.rt_statistic.machine_running_string, sizeof (machine.rt_statistic.machine_running_string), 0+0);

                            } else if (strnicmp(actName, "machine_elapsed", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.machine_elapsed, sizeof (machine.rt_statistic.machine_elapsed), 0+0);

                            } else if (strnicmp(actName, "machine_running", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &updateValue, &reverseValue, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.machine_running, sizeof (machine.rt_statistic.machine_running), 0+0);
                            }
                                


                            


                            /////////////////////////////////////    
                            // Gestione IO
                            //

                        } else if (strnicmp(svar_name, "DI.", 3) == 0 || strnicmp(svar_name, "DO.", 3) == 0
                                || strnicmp(svar_name, "AI.", 3) == 0 || strnicmp(svar_name, "AO.", 3) == 0
                                || strnicmp(svar_name, "I2C.", 4) == 0
                                ) {
                            
                            ////////////////////////////
                            // Digital/Analog IN/OUT
                            //
                            unsigned int num_io = 0;
                            char IOCase = 0;


                            i = 0;
                            found1B = 0;
                            actName = svar_name + 3;
                            actProp = NULL;

                            strncpy(var_prefix, svar_name, 3);

                            // Ricerca dell' indirizzo della scheda
                            while (actName[i] != 0 && isdigit(actName[i])) {
                                i++;
                            }

                            actName[i] = 0;
                            found1B = atoi((char*) (actName));

                            if (found1B > 0 && found1B <= machine.numIOBoardSlots) {
                                unsigned int foundIO1B = 0;

                                // Ricerca dell' indirizzo dell'IO
                                actProp = (char *) &actName[i + 1];
                                i = 0;
                                while (actProp[i] != 0 && isdigit(actProp[i])) {
                                    i++;
                                }


                                // Ricerca valore da assegnare
                                reverseValue = false;
                                updateValue = false;

                                if (search_for_update_value(actProp, &i, &updateValue, &reverseValue, &newValue, NULL, 0) > 0) {
                                    // forza la modalità scrittura
                                    Mode |= 4;
                                }

                                foundIO1B = atoi((char*) actProp);

                                if (strnicmp(svar_name, "DI.", 3) == 0) {
                                    num_io = machine.ioBoardSlots[found1B - 1].numDigitalIN;
                                    IOCase = 0;
                                } else if (strnicmp(svar_name, "DO.", 3) == 0) {
                                    num_io = machine.ioBoardSlots[found1B - 1].numDigitalOUT;
                                    IOCase = 1;
                                } else if (strnicmp(svar_name, "AI.", 3) == 0) {
                                    num_io = machine.ioBoardSlots[found1B - 1].numAnalogIN;
                                    IOCase = 2;
                                } else if (strnicmp(svar_name, "AO.", 3) == 0) {
                                    num_io = machine.ioBoardSlots[found1B - 1].numAnalogOUT;
                                    IOCase = 3;
                                } else if (strnicmp(svar_name, "I2C.", 4) == 0) {
                                    num_io = machine.ioBoardSlots[found1B - 1].i2cNumValues;
                                    IOCase = 4;
                                }

                                if (foundIO1B > 0 && foundIO1B <= num_io) {

                                    if (Mode & 1) {
                                        // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                        if (IOCase == 0) {
                                            if (addVarToCache((void*) &machine.ioBoardSlots[found1B - 1].digitalIN[foundIO1B - 1], TYPE_CHAR, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (IOCase == 1) {
                                            if (addVarToCache((void*) &machine.ioBoardSlots[found1B - 1].digitalOUT[foundIO1B - 1], TYPE_CHAR, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (IOCase == 2) {
                                            if (addVarToCache((void*) &machine.ioBoardSlots[found1B - 1].analogIN[foundIO1B - 1], TYPE_FLOAT3_UINT, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (IOCase == 3) {
                                            if (addVarToCache((void*) &machine.ioBoardSlots[found1B - 1].analogOUT[foundIO1B - 1], TYPE_FLOAT3_UINT, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (IOCase == 4) {
                                            if (addVarToCache((void*) &machine.ioBoardSlots[found1B - 1].i2cValues[foundIO1B - 1], TYPE_FLOAT1_TEMPERATURE, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        }

                                    } else if (Mode & 4) {
                                        // Scrittura variabile

                                        if (check_manual_cmd_status()) {
                                            
                                            if (machine.actuator[BLOW_PRESSURE].cur_rpos > machine.workSet.max_pressure_in_mold) {
                                                
                                                snprintf(App.Msg, App.MsgSize, "BLOW_PRESSURE not zero (%0.3f)", machine.actuator[BLOW_PRESSURE].cur_rpos);
                                                if (generate_alarm((char*) App.Msg, 7014, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }
                                                
                                            } else {
    
                                                machine.manual_move++;
                                                machine.should_recover = TRUE;
                                            
                                                if (IOCase == 0) {

                                                } else if (IOCase == 1) {
                                                    // Digital out
                                                    if (updateValue) {
                                                        if (reverseValue) {
                                                            machine.ioBoardSlots[found1B - 1].digitalOUT[foundIO1B - 1] = !machine.ioBoardSlots[found1B - 1].digitalOUT[foundIO1B - 1];
                                                        } else {
                                                            machine.ioBoardSlots[found1B - 1].digitalOUT[foundIO1B - 1] = (newValue == 0 ? 0 : 1);
                                                        }
                                                        // forza la modalità scrittura
                                                        Mode |= 4;
                                                    }

                                                } else if (IOCase == 2) {
                                                } else if (IOCase == 3) {
                                                    // Analog out
                                                    if (updateValue) {
                                                        if (reverseValue) {
                                                            machine.ioBoardSlots[found1B - 1].analogOUT[foundIO1B - 1] = (machine.ioBoardSlots[found1B - 1].analogOUT[foundIO1B - 1] > 0 ? 0 : 4095);
                                                        } else {
                                                            machine.ioBoardSlots[found1B - 1].analogOUT[foundIO1B - 1] = newValue;
                                                        }
                                                    }
                                                }
                                            }
                                            
                                        } else {
                                            
                                        }

                                    } else {
                                        if (IOCase == 0) {
                                            snprintf((char*) str, str_size, "DI.%d.%d=%d", (int) (found1B - 1), (int) foundIO1B, (int) machine.ioBoardSlots[found1B - 1].digitalIN[found1B - 1]);
                                        } else if (IOCase == 1) {
                                            snprintf((char*) str, str_size, "DO.%d.%d=%d", (int) (found1B - 1), (int) foundIO1B, (int) machine.ioBoardSlots[found1B - 1].digitalOUT[found1B - 1]);
                                        } else if (IOCase == 2) {
                                            snprintf((char*) str, str_size, "AI.%d.%d=%0.3f", (int) (found1B - 1), (int) foundIO1B, (float) machine.ioBoardSlots[found1B - 1].analogIN[found1B - 1]);
                                        } else if (IOCase == 3) {
                                            snprintf((char*) str, str_size, "AO.%d.%d=%0.3f", (int) (found1B - 1), (int) foundIO1B, (float) machine.ioBoardSlots[found1B - 1].analogOUT[found1B - 1]);
                                        } else if (IOCase == 4) {
                                            snprintf((char*) str, str_size, "I2C.%d.%d=%0.3f", (int) (found1B - 1), (int) foundIO1B, (float) machine.ioBoardSlots[found1B - 1].i2cValues[found1B - 1]);
                                        } else {
                                            snprintf((char*) str, str_size, "X=!");
                                        }
                                    }
                                } else {
                                    // Fuori range
                                    if (Mode & 1) {
                                        // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                        // snprintf( (char*)str, str_size, "![%d/%d/%s/%s/%s]", foundIO1B, num_io, svar_name, actName, actProp);
                                        snprintf((char*) str, str_size, "!");
                                    } else {
                                        snprintf((char*) str, str_size, "!");
                                        /*
                                        if (IOCase == 0) {
                                            snprintf( (char*)str, str_size, "DI=![%d/%d]", foundIO1B, num_io);
                                        } else if (IOCase == 1) {
                                            snprintf( (char*)str, str_size, "DO=![%d/%d]", foundIO1B, num_io);
                                        } else if (IOCase == 2) {
                                            snprintf( (char*)str, str_size, "AI=![%d/%d]", foundIO1B, num_io);
                                        } else if (IOCase == 3) {
                                            snprintf( (char*)str, str_size, "AO=![%d/%d]", foundIO1B, num_io);
                                        } else {
                                            snprintf( (char*)str, str_size, "XX=![%d/%d]", foundIO1B, num_io);
                                        }
                                         */
                                    }
                                }

                            } else {
                                // Fuori range
                                if (Mode & 1) {
                                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                    // snprintf( (char*)str, str_size, "!foundB");
                                    snprintf((char*) str, str_size, "!");
                                } else {
                                    snprintf((char*) str, str_size, "!");
                                    /*
                                    if (IOCase == 0) {
                                        snprintf( (char*)str, str_size, "DI=!foundB");
                                    } else if (IOCase == 1) {
                                        snprintf( (char*)str, str_size, "DO=!foundB");
                                    } else if (IOCase == 2) {
                                        snprintf( (char*)str, str_size, "AI=!foundB");
                                    } else if (IOCase == 3) {
                                        snprintf( (char*)str, str_size, "AO=!foundB");
                                    } else {
                                        snprintf( (char*)str, str_size, "XX=!foundB");
                                    }
                                     */
                                }
                            }


                            
                            

                            /////////////////////////////////////    
                            // Gestione SCR
                            //

                        } else if (strnicmp(svar_name, "SCR.", 4) == 0) {
                            
                            ////////////////////////////
                            // Digital/Analog IN/OUT
                            //
                            unsigned int num_io = 0;
                            char IOCase = 0;


                            i = 0;
                            found1B = 0;
                            actName = svar_name + 4;
                            actProp = NULL;

                            strncpy(var_prefix, svar_name, 4);

                            // Ricerca dell' indirizzo della scheda
                            while (actName[i] != 0 && isdigit(actName[i])) {
                                i++;
                            }


                            actName[i] = 0;
                            found1B = atoi((char*) (actName));

                            if (found1B > 0 && found1B <= App.NumSCRBoard) {
                                unsigned int foundIO1B = 0;

                                i++;

                                // Ricerca proprietà dell' SCR
                                if (strnicmp(&actName[i], "ROW.", 4) == 0) {
                                    num_io = GLSCRBoard[found1B - 1].numRows;
                                    IOCase = 0;
                                    i+=4;
                                } else if (strnicmp(&actName[i], "I2C.", 4) == 0) {
                                    num_io = GLSCRBoard[found1B - 1].i2cNumValues;
                                    IOCase = 1;
                                    i+=4;
                                } else if (strnicmp(&actName[i], "REF.", 4) == 0) {
                                    num_io = 1;
                                    IOCase = 2;
                                    i+=4;
                                } else if (strnicmp(&actName[i], "TOLL", 4) == 0) {
                                    num_io = 1;
                                    IOCase = 3;
                                    i+=4;
                                }

                                // Ricerca dell' indirizzo dell'IO
                                actProp = (char *) &actName[i + 0];
                                i = 0;
                                while (actProp[i] != 0 && isdigit(actProp[i])) {
                                    i++;
                                }
                                foundIO1B = atoi((char*) actProp);

                                
                                // Ricerca valore da assegnare
                                reverseValue = false;
                                updateValue = false;

                                if (search_for_update_value(actProp, &i, &updateValue, &reverseValue, &newValue, newSValue, 0) > 0) {
                                    // forza la modalità scrittura
                                    Mode |= 4;
                                }


                                if (foundIO1B > 0 && foundIO1B <= num_io) {


                                    if (Mode & 1) {
                                        // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                        if (IOCase == 0) {
                                            if (addVarToCache((void*) &GLSCRBoard[found1B - 1].Rows[foundIO1B - 1], TYPE_INT, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (IOCase == 1) {
                                            if (addVarToCache((void*) &GLSCRBoard[found1B - 1].i2cValues[foundIO1B - 1], TYPE_FLOAT1_TEMPERATURE, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (IOCase == 2) {
                                            if (addVarToCache((void*) &GLSCRBoard[found1B - 1].AnalogVoltage[foundIO1B - 1], TYPE_FLOAT3, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (IOCase == 3) {
                                            if (addVarToCache((void*) &GLSCRBoard[found1B - 1].TollRefChange, TYPE_FLOAT3, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        }

                                    } else if (Mode & 4) {
                                        // Scrittura variabile

                                        if (check_manual_cmd_status()) {
                                            
                                            if (IOCase == 0) {

                                            } else if (IOCase == 1) {
                                                // Digital out
                                                if (updateValue) {
                                                    if (reverseValue) {
                                                        // GLSCRBoard[found1B - 1].Rows[foundIO1B - 1] = ...
                                                    } else {
                                                        GLSCRBoard[found1B - 1].Rows[foundIO1B - 1] = newValue;
                                                        if (GLSCRBoard[found1B - 1].Rows[foundIO1B - 1] < 0)
                                                            GLSCRBoard[found1B - 1].Rows[foundIO1B - 1] = 0;
                                                        if (GLSCRBoard[found1B - 1].Rows[foundIO1B - 1] > 100)
                                                            GLSCRBoard[found1B - 1].Rows[foundIO1B - 1] = 100;
                                                    }
                                                    // forza la modalità scrittura
                                                    Mode |= 4;
                                                }

                                            } else if (IOCase == 2) {
                                                // Analog voltage
                                                if (updateValue) {
                                                    if (reverseValue) {
                                                        // GLSCRBoard[found1B - 1].AnalogVoltage
                                                    } else {
                                                        GLSCRBoard[found1B - 1].AnalogVoltage[foundIO1B - 1] = atof(newSValue);
                                                    }
                                                }
                                                
                                            } else if (IOCase == 3) {
                                                // Tollerance
                                                if (updateValue) {
                                                    if (reverseValue) {
                                                        // GLSCRBoard[found1B - 1].AnalogVoltage
                                                    } else {
                                                        GLSCRBoard[found1B - 1].TollRefChange = atof(newSValue);
                                                        if (GLSCRBoard[found1B - 1].TollRefChange < 0.0f)
                                                            GLSCRBoard[found1B - 1].TollRefChange = 0.0f;
                                                    }
                                                }
                                            }
                                            
                                        } else {                                            
                                        }

                                    } else {
                                        if (IOCase == 0) {
                                            snprintf((char*) str, str_size, "SCR.%d.ROW.%d=%d", (int) (found1B - 1), (int) foundIO1B, (int) GLSCRBoard[found1B - 1].Rows[foundIO1B - 1] );
                                        } else if (IOCase == 1) {
                                            snprintf((char*) str, str_size, "SCR.%d.I2C.%d=%0.1f", (int) (found1B - 1), (int) foundIO1B, (float) GLSCRBoard[found1B - 1].i2cValues[foundIO1B - 1] / 1000.0f );
                                        } else if (IOCase == 2) {
                                            snprintf((char*) str, str_size, "SRC.%d.REF.%d=%0.3f", (int) (found1B - 1), (int) foundIO1B, (float) GLSCRBoard[found1B - 1].AnalogVoltage[0]);
                                        } else if (IOCase == 3) {
                                            snprintf((char*) str, str_size, "SRC.%d.TOLL=%0.3f", (int) (found1B - 1), (float) GLSCRBoard[found1B - 1].TollRefChange);
                                        } else {
                                            snprintf((char*) str, str_size, "!");
                                        }
                                    }
                                } else {
                                    // Fuori range
                                    if (Mode & 1) {
                                        // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                        // snprintf( (char*)str, str_size, "![%d/%d/%s/%s/%s]", foundIO1B, num_io, svar_name, actName, actProp);
                                        snprintf((char*) str, str_size, "!");
                                    } else {
                                        snprintf((char*) str, str_size, "!");
                                        /*
                                        if (IOCase == 0) {
                                            snprintf( (char*)str, str_size, "DI=![%d/%d]", foundIO1B, num_io);
                                        } else if (IOCase == 1) {
                                            snprintf( (char*)str, str_size, "DO=![%d/%d]", foundIO1B, num_io);
                                        } else if (IOCase == 2) {
                                            snprintf( (char*)str, str_size, "AI=![%d/%d]", foundIO1B, num_io);
                                        } else if (IOCase == 3) {
                                            snprintf( (char*)str, str_size, "AO=![%d/%d]", foundIO1B, num_io);
                                        } else {
                                            snprintf( (char*)str, str_size, "XX=![%d/%d]", foundIO1B, num_io);
                                        }
                                         */
                                    }
                                }

                            } else {
                                // Fuori range
                                if (Mode & 1) {
                                    snprintf((char*) str, str_size, "!");
                                } else {
                                    snprintf((char*) str, str_size, "!");
                                }
                            }

                            
                            

                            //////////////////////////////////////////////                            
                            // Gestione attuatori
                            //                            

                        } else if (strnicmp(svar_name, "act.", 4) == 0) {
                            int i_act = 0;

                            i = 0;
                            found1B = 0;
                            actName = svar_name + 4;
                            actProp = NULL;

                            strncpy(var_prefix, svar_name, 4);

                            while (actName[i] != 0 && actName[i] != '.' && actName[i] != '=' && actName[i] != ' ') {
                                i++;
                            }

                            if (actName[i] == '.') {
                                actName[i] = 0;
                                actProp = actName + i + 1;
                            }

                            for (i_act = 0; i_act < machine.num_actuator; i_act++) {

                                if (strcmpi(machine.actuator[i_act].name, actName) == 0) {
                                    found1B = i_act + 1;


                                    if (!actProp) {
                                        actProp = (char*)"name";
                                        if (Mode & 1) {
                                            // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                            if (addVarToCache((void*) &machine.actuator[i_act].Id, TYPE_INT, &cacheVarId) > 0) {
                                                snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                                            } else {
                                                snprintf((char*) str, str_size, "!");
                                            }
                                        } else if (Mode & 4) {
                                            // Scrittura variabile
                                        } else {
                                            snprintf((char*) str, str_size, "act.%s.%s=%d", machine.actuator[i_act].name, actProp, machine.actuator[i_act].Id);
                                        }

                                    } else {



                                        PROCESS_READONLY_VAR("Pos", machine.actuator[i_act].cur_vpos, TYPE_FLOAT3)              // 0 ... 1000
                                        else PROCESS_READONLY_VAR("rPos", machine.actuator[i_act].cur_rpos, TYPE_FLOAT3)        // start_pos ... .. end_pos
                                        else PROCESS_READONLY_VAR("cPos", machine.actuator[i_act].position, TYPE_INT)           // 0 / 1
                                        else PROCESS_READONLY_VAR("tgPos", machine.actuator[i_act].target_position, TYPE_INT)    // 0 / 1
                                        else PROCESS_READONLY_VAR("sPos", machine.actuator[i_act].start_rpos, TYPE_FLOAT3)      
                                        else PROCESS_READONLY_VAR("ePos", machine.actuator[i_act].end_rpos, TYPE_FLOAT3)


                                        else PROCESS_READWRITE_VAR("ACC_AUTO1", machine.actuator[i_act].acc_auto1, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("DEC_AUTO1", machine.actuator[i_act].dec_auto1, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("SPEED_AUTO1", machine.actuator[i_act].speed_auto1, TYPE_FLOAT3, true, handle_speed1_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("SPEED_LIN_AUTO1", machine.actuator[i_act].speed_lin_auto1, TYPE_FLOAT1, true, handle_speed_lin1_update, (void*)&machine.actuator[i_act] )
                                        else PROCESS_READWRITE_VAR("FORCE_AUTO1", machine.actuator[i_act].force_auto1, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)

                                        else PROCESS_READWRITE_VAR("ACC_AUTO2", machine.actuator[i_act].acc_auto2, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("DEC_AUTO2", machine.actuator[i_act].dec_auto2, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("SPEED_AUTO2", machine.actuator[i_act].speed_auto2, TYPE_FLOAT3, true, handle_speed2_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("SPEED_LIN_AUTO2", machine.actuator[i_act].speed_lin_auto2, TYPE_FLOAT1, true, handle_speed_lin2_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("FORCE_AUTO2", machine.actuator[i_act].force_auto2, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                                
                                        else PROCESS_READWRITE_VAR("start_rpos_toll", machine.actuator[i_act].start_rpos_toll, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("end_rpos_toll", machine.actuator[i_act].end_rpos_toll, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)


                                        else PROCESS_READONLY_VAR("step", machine.actuator[i_act].step, TYPE_INT)
                                        else PROCESS_READONLY_VAR("stat", machine.actuator[i_act], TYPE_ACTUATOR_STATUS)
                                        else PROCESS_READONLY_VAR("dstat", machine.actuator[i_act], TYPE_ACTUATOR_DRIVER_STATUS)
                                                
                                        else PROCESS_READONLY_VAR("speed", machine.actuator[i_act].speed, TYPE_FLOAT1)
                                        else PROCESS_READONLY_VAR("speed_lin", machine.actuator[i_act].speed_lin, TYPE_FLOAT1)
                                        else PROCESS_READONLY_VAR("min_speed", machine.actuator[i_act].min_speed, TYPE_FLOAT1)
                                        else PROCESS_READONLY_VAR("max_speed", machine.actuator[i_act].max_speed, TYPE_FLOAT1)

                                        else PROCESS_READONLY_VAR("torque", machine.actuator[i_act].torque, TYPE_FLOAT3)
                                        else PROCESS_READONLY_VAR("rated_torque", machine.actuator[i_act].rated_torque, TYPE_PERCENT)
                                        else PROCESS_READONLY_VAR("min_torque", machine.actuator[i_act].min_torque, TYPE_FLOAT3)
                                        else PROCESS_READONLY_VAR("max_torque", machine.actuator[i_act].max_torque, TYPE_FLOAT3)
                                                
                                        else PROCESS_READONLY_VAR("temp", machine.actuator[i_act].temp, TYPE_FLOAT3)

                                        else PROCESS_READONLY_VAR("TIME1", machine.actuator[i_act].time_ms1, TYPE_MSEC)
                                        else PROCESS_READONLY_VAR("TIME11", machine.actuator[i_act].time_ms11, TYPE_MSEC)
                                        else PROCESS_READONLY_VAR("TIME12", machine.actuator[i_act].time_ms12, TYPE_MSEC)
                                        else PROCESS_READONLY_VAR("TIME13", machine.actuator[i_act].time_ms13, TYPE_MSEC)

                                        else PROCESS_READONLY_VAR("TIME2", machine.actuator[i_act].time_ms2, TYPE_MSEC)
                                        else PROCESS_READONLY_VAR("TIME21", machine.actuator[i_act].time_ms21, TYPE_MSEC)
                                        else PROCESS_READONLY_VAR("TIME22", machine.actuator[i_act].time_ms22, TYPE_MSEC)
                                        else PROCESS_READONLY_VAR("TIME23", machine.actuator[i_act].time_ms23, TYPE_MSEC)


                                        else PROCESS_READWRITE_VAR("TOUT1", machine.actuator[i_act].timeout1_ms, TYPE_MSEC, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("TWARN1", machine.actuator[i_act].timewarn1_ms, TYPE_MSEC, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("TOUT2", machine.actuator[i_act].timeout2_ms, TYPE_MSEC, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("TWARN2", machine.actuator[i_act].timewarn2_ms, TYPE_MSEC, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("FERR1", machine.actuator[i_act].follow_error1, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("FERR2", machine.actuator[i_act].follow_error2, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)

                                        else PROCESS_READWRITE_VAR("curFERR1", machine.actuator[i_act].cur_follow_error1, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("curFERR2", machine.actuator[i_act].cur_follow_error2, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)

                                        else PROCESS_READONLY_VAR("ERR", machine.actuator[i_act].error, TYPE_INT)

                                        else PROCESS_READONLY_VAR("Trace", machine.actuator[i_act].pTrace, TYPE_TRACE_NUM_DATA)
                                        else PROCESS_READONLY_VAR("TPos", machine.actuator[i_act].pTrace, TYPE_TRACE_POS)
                                        else PROCESS_READONLY_VAR("HTPos", machine.actuator[i_act].pTrace, TYPE_TRACE_HEADER)

                                                
                                        else PROCESS_READWRITE_VAR("homing_speed_rpm", machine.actuator[i_act].homing_speed_rpm, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("homing_rated_torque", machine.actuator[i_act].homing_rated_torque, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("homing_offset", machine.actuator[i_act].homing_offset_mm, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("homing_timeout", machine.actuator[i_act].homing_timeout_ms, TYPE_MSEC, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READONLY_VAR("homing_position", machine.actuator[i_act].homing_position, TYPE_INT)
                                                
                                                


                                        else if (strcmpi(actProp, "TraceSpeed") == 0) {
                                        } else if (strcmpi(actProp, "TraceAcc") == 0) {
                                        } else if (strcmpi(actProp, "TraceTorque") == 0) {


                                            ////////////////////
                                            // Posizionamento
                                            //
                                        } else if (strcmpi(actProp, "OPEN") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (machine.actuator[i_act].open_func) {
                                                    if (check_manual_cmd_status()) {
                                                        if (machine.actuator[i_act].open_func() < 0) {                                                            
                                                        }
                                                        machine.manual_move++;
                                                        machine.should_recover = TRUE;
                                                    }
                                                } else {
                                                    snprintf(str, str_size, (char*) "Actuator %s has no open callback defined", machine.actuator[i_act].name);
                                                    if (generate_alarm((char*) str, 7001, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                    }
                                                }
                                            } else {
                                                snprintf(str, str_size, (char*) "Machine must be in MANUAL mode");
                                                if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }                                                
                                            }
                                        } else if (strcmpi(actProp, "CLOSE") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (machine.actuator[i_act].close_func) {
                                                    if (check_manual_cmd_status()) {
                                                        if (machine.actuator[i_act].close_func() < 0) {                                                            
                                                        }
                                                        machine.manual_move++;
                                                        machine.should_recover = TRUE;
                                                    }
                                                } else {
                                                    snprintf(str, str_size, (char*) "Actuator %s has no close callback defined", machine.actuator[i_act].name);
                                                    if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                    }
                                                }
                                            } else {
                                                snprintf(str, str_size, (char*) "Machine must be in MANUAL mode");
                                                if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }                                                
                                            }
                                        } else if (strcmpi(actProp, "HOME") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (machine.actuator[i_act].home_func) {
                                                    if (check_manual_cmd_status()) {
                                                        if (machine.actuator[i_act].home_func() < 0) {                                                            
                                                        }
                                                        machine.manual_move++;
                                                        machine.should_recover = TRUE;
                                                    }
                                                } else {
                                                    snprintf(str, str_size, (char*) "Actuator %s has no home callback defined", machine.actuator[i_act].name);
                                                    if (generate_alarm((char*) str, 7004, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                    }
                                                }
                                            } else {
                                                snprintf(str, str_size, (char*) "Machine must be in MANUAL mode");
                                                if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }                                                
                                            }
                                        } else if (strcmpi(actProp, "JOG+") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (machine.actuator[i_act].jog_plus_func) {
                                                    if (check_manual_cmd_status()) {
                                                        if (machine.actuator[i_act].jog_plus_func() < 0) {                                                            
                                                        }
                                                        machine.manual_move++;
                                                        machine.should_recover = TRUE;
                                                    }
                                                } else {
                                                    snprintf(str, str_size, (char*) "Actuator %s has no jog_plus callback defined)", machine.actuator[i_act].name);
                                                    if (generate_alarm((char*) str, 7005, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                    }
                                                }
                                            } else {
                                                snprintf(str, str_size, (char*) "Machine must be in MANUAL mode");
                                                if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }                                                
                                            }
                                        } else if (strcmpi(actProp, "JOG-") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (machine.actuator[i_act].jog_minus_func) {
                                                    if (check_manual_cmd_status()) {
                                                        if (machine.actuator[i_act].jog_minus_func() < 0) {
                                                        }
                                                        machine.manual_move++;
                                                        machine.should_recover = TRUE;
                                                    }
                                                } else {
                                                    snprintf(str, str_size, (char*) "Actuator %s has no jog_minus callback defined", machine.actuator[i_act].name);
                                                    if (generate_alarm((char*) str, 7006, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                    }
                                                }
                                            } else {
                                                snprintf(str, str_size, (char*) "Machine must be in MANUAL mode");
                                                if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }                                                
                                            }
                                            
                                        } else if (strcmpi(actProp, "RESET") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (machine.actuator[i_act].pCANSlot) {
                                                    ((LP_CANSlot)machine.actuator[i_act].pCANSlot)->resetRequest = 1;                                                
                                                } else if (machine.actuator[i_act].pSerialSlot) {
                                                    ((LP_SerialSlot)machine.actuator[i_act].pSerialSlot)->resetRequest = 1;                                                
                                                } else {
                                                }
                                                machine.manual_move++;
                                                machine.should_recover = TRUE;
                                                
                                            } else {
                                                snprintf(str, str_size, (char*) "Machine must be in MANUAL mode");
                                                if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }                                                
                                            }
                                            
                                                    
                                        } else {
                                            if (Mode & 1) {
                                                snprintf(str, str_size, (char*) "Actuator's property not found:'%s'", (char*) actProp);
                                                if (generate_alarm((char*) str, 7008, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }
                                            }
                                        }
                                    }

                                    break;
                                }
                            }

                            if (!found1B) {
                                if (Mode & 1) {
                                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                    snprintf(str, str_size, (char*) "Actuator not found:'%s'", (char*) actName);
                                    if (generate_alarm((char*) str, 7007, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                    }
                                    str[0] = 0;

                                } else if (Mode & 4) {
                                    // Scrittura variabile
                                    str[0] = 0;

                                } else {
                                    snprintf((char*) str, str_size, "%s.%s=[!]", actName, (actProp != NULL ? actProp : "name"));
                                }
                            }

                            
                            
                            /////////////////////////////////////////////
                            // Gestione Autentuicazione
                            //
                        } else if (strnicmp(svar_name, "user=", 5) == 0) {
                            strncpy(GLLoginUser, (char*)&svar_name[5], sizeof(GLLoginUser));

                        } else if (strnicmp(svar_name, "password=", 9) == 0) {
                            strncpy(GLLoginPassword, (char*)&svar_name[9], sizeof(GLLoginPassword));

                        } else if (strnicmp(svar_name, "token=", 6) == 0) {
                            strncpy(GLLoginToken, (char*)&svar_name[6], sizeof(GLLoginToken));

                        } else {

                            // if (Mode & 1) fprintf(stdout, "[SYS:%s-id:%d]", svar_name, (int)cacheVarId); fflush (stdout);

                            ////////////////////////////////
                            // Variabile di sistema ?
                            //
                            if (system_get_value((uint8_t*)svar_name, (uint8_t*)str, str_size, Mode) < 0) {
                                if (Mode & 1) {
                                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                    snprintf((char*) str, str_size, "!");

                                    snprintf(str, str_size, (char*) "Variable not found:'%s'", (char*) svar_name);
                                    if (generate_alarm((char*) str, 7009, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                    }

                                } else if (Mode & 4) {
                                    // Scrittura variabile
                                } else {
                                    snprintf((char*) str, str_size, "%s.=[!]", svar_name);
                                }

                                /*
                                strcpy((char*) str, "error=UNK:");
                                sanitizeString(svar_name);
                                strncat((char*) str, svar_name, (size_t) str_size - (size_t) strlen(str) - (size_t) strlen(svar_name));
                                 */

                            } else {
                                // fprintf(stdout, "[STORE OK:%s]\n", str); fflush (stdout);
                            }
                        }
                    }


                    // Stampa le variabili registrate
                    // if (Mode & 1) fprintf(stdout, "[STORE:%s-id:%d]", svar_name, (int)cacheVarId); fflush (stdout);


                    if (out_str) {

                        if (str) {

                            // Anche la stringa vuota va aggiunta per rispettare la sequenza della domanda/risposta
                            // if (str[0]) {

                            len = strlen((char*) out_str);
                            lenAdding = strlen((char*) str);

                            ////////////////////////////////////////
                            // Codice identificativo trasmissione
                            //
                            if (nVar == 0) {
                              
                                if (Mode & 16) {
                                    // Pacchetto da inoltrare (trasparente alla logica)
                                    // Appende il codice di inoltro
                                    strncat((char*) out_str, "<", (size_t) out_str_size[0] - 2);
                                    len++;
                                }
                                
                                if (Mode & 1) {
                                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                                    strncat((char*) out_str, "=", (size_t) out_str_size[0] - 2);
                                    len++;
                                    
                                } else if (Mode & 2) {
                                    // Risoluzione variabile per ID dalla cache
                                    strncat((char*) out_str, "@", (size_t) out_str_size[0] - 2);
                                    len++;

                                } else if (Mode & 4) {
                                    // Scrittura variabile
                                    strncat((char*) out_str, "!", (size_t) out_str_size[0] - 2);
                                    len++;
                                    
                                } else if (Mode & 8) {
                                    // Autenticazione utente 
                                    // Output Riscritto alla fine della funzione
                                    strncat((char*) out_str, "#", (size_t) out_str_size[0] - 2);
                                    len++;

                                } else {
                                    // Esecuzione
                                    strncat((char*) out_str, "!", (size_t) out_str_size[0] - 2);
                                    len++;
                                }
                            }


                            // Debug : stampa la risposta
                            // printf("[%s]", str);


                            if (nVar) {
                                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                // !! FUCKOFF WATCOM COMPILER 
                                // if (len < *out_str_size) {
                                if ((size_t) len + 1 < (size_t) out_str_size[0]) {
                                    strncat((char*) out_str, App.RowSep, (size_t) out_str_size[0] - len);
                                    len++;
                                }
                            }

                            if ((size_t) len + lenAdding < (size_t) out_str_size[0]) {
                                strncat((char*) out_str, str, (size_t) out_str_size[0] - len);
                            }

                            nVar++;
                        }
                    }
                }
            }


            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // FACKYOU STUPID COMPILER!!!
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // svar_name = pvar_name?(pvar_name+1):NULL;


            svar_name = pvar_name;
            if (svar_name)
                svar_name++;
        }



        ////////////////////////////////
        // Autenticazione utente
        //
        if (Mode & 8) {
            char *token = login_user(GLLoginUser, GLLoginPassword, GLLoginToken);
            
            out_str[0] = 0;
            
            if (token == NULL || (token && token[0] == 0)) {
                snprintf((char*) App.Msg, sizeof (App.MsgSize), "[%sLogin failed for user:%s-%s%s]", (char*) ANSI_COLOR_RED, (char*) GLLoginUser, (char*) GLLoginToken, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            } else {
                snprintf((char*) App.Msg, sizeof (App.MsgSize), "[%sUser:%s logged in...Token:%s%s]", (char*) ANSI_COLOR_GREEN, (char*) GLLoginUser, (char*) GLLoginToken, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);                
            }
                    
            if (Mode & 16) {
                // Pacchetto da inoltrare (trasparente alla logica)
                // Appende il codice di inoltro
                strncat((char*) out_str, "<", (size_t) out_str_size[0] - 2);
                len++;
            }
            
            strncat((char*) out_str, "#", (size_t) out_str_size[0] - 16 );
            if (token)
                strncat((char*) out_str, token, (size_t) out_str_size[0] - strlen(token) );

            // snprintf((char*) App.Msg, sizeof (App.MsgSize), "[%s%s%s]", (char*) ANSI_COLOR_RED, (char*) out_str, (char*) ANSI_COLOR_RESET);
            // vDisplayMessage(App.Msg);
        }


        if (str)
            free(str);

        if (var_prefix)
            free(var_prefix);


        
        
    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "logic_get_value() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "logic_get_value() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }
        

    return retVal;
}

