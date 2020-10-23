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




extern "C" int search_for_update_value(char *actProp, uint32_t *i, bool *updateValue, bool *isValueReversed, int *newValue, char *newStringValue, int newStringValueSize) {
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
                    if (isValueReversed) *isValueReversed = true;
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
// auxMode & 2 ->  Interpreta newValue come base64 encoded
// auxMode & 1 ->  Incrementa il contantore di caricamento del workSet
//
extern "C" int std_getset_var(char *var_prefix, char *actName, unsigned int *i, int keyLen,
        bool *isValueUpdated, bool *isValueReversed, char *newValue, int newValueSize,
        float *maxFValue, float *minFValue, int32_t *maxIValue, int32_t *minIValue, uint32_t *maxUIValue, uint32_t *minUIValue,
        int *Mode, int TYPE_OF, char *str, int str_size, void *sourceVar, int sourceVarSize, int auxMode) {

    int32_t retVal = 0;
    int32_t cacheVarId = 0;
    uint32_t decoded_data_length = 0;
    char *decoded_data = NULL;

    
    // Ricerca valore da assegnare
    *i = keyLen;

    isValueUpdated[0] = false;
    isValueReversed[0] = false;
    if (newValue)
        newValue[0] = 0;

    if (search_for_update_value(actName, i, isValueUpdated, isValueReversed, NULL, newValue, newValueSize) > 0) {
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

        if (auxMode & 2) {
            // auxMode & 2 ->  Interpreta newValue come base64 encoded
            decoded_data = (char*)NewBase64Decode((char*)newValue, (size_t)strlen((char*)newValue), (size_t*)&decoded_data_length);
            decoded_data[decoded_data_length] = 0;            
            newValue = (char*)decoded_data;
        }
        
        if (isValueUpdated) {
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
                            retVal = 1;
                        }
                    }
                } else {
                    // Puntatore
                    if (sourceStringVar) {
                        if (TYPE_OF == TYPE_INT_PTR) {
                            int32_t newIValue = atoi(newValue);
                            int32_t **ppSourceIntVar = (int32_t **)sourceVar;
                            if (ppSourceIntVar) {
                                int32_t *pSourceIntVar = (int32_t *)ppSourceIntVar[0];
                                if (pSourceIntVar) {
                                    if ( pSourceIntVar[0] != newIValue) {
                                        pSourceIntVar[0] = newIValue;
                                        retVal = 1;
                                    }
                                }
                            }
                        } else {
                            if (check_stri((char*)sourceStringVar[0], (char*)newValue)) {
                                sourceStringVar[0] = (char*)realloc(sourceStringVar[0], strlen(newValue)+32);
                                if (sourceStringVar[0]) {
                                    strncpy((char*)sourceStringVar[0], (char*)newValue, strlen(newValue)+1 );
                                    retVal = 1;
                                }
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
            } else if (TYPE_OF == TYPE_INT_PTR) {                
                snprintf((char*) str, str_size, "%d", (int)(((int32_t**)sourceVar)[0])[0] );
            } else if (TYPE_OF == TYPE_MSEC) {
                snprintf((char*) str, str_size, "%0.3f", (float)( (float)(((int32_t*)sourceVar)[0]) / 1000.0f) );
            } else if (TYPE_OF == TYPE_STRING) {
                snprintf((char*) str, str_size, "%s", (char*)((char**)sourceVar)[0] );
            } else if (TYPE_OF == TYPE_SRC_STRING) {
                char ***pppStr = (char ***)sourceVar;
                if(pppStr) {
                    snprintf((char*) str, str_size, "%s", (char*)((char**)pppStr[0])[0] );
                }
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

    if (decoded_data) free(decoded_data);
    
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

void handle_gcode_update(void *var, char *newValue, void *userData) {
    gcode_update_content ( newValue );
}


void handle_gcode_start_row_update(void *var, char *newValue, void *userData) {
    if(newValue)
        gcode_update_start_row ( (uint32_t)atoi (newValue) );
}

void handle_gcode_no_view_row_update(void *var, char *newValue, void *userData) {
    if(newValue)
        gcode_update_no_view_row ( (uint32_t)atoi (newValue) );
}

void handle_gcode_crow_update(void *var, char *newValue, void *userData) {
    if(newValue) {
        if (machine.App.GCode.cRow < machine.App.GCode.numRows) {
            BIT_OFF(machine.App.GCode.RowsOptions[machine.App.GCode.cRow], BIT28);
        }
        if (atoi (newValue) != machine.App.GCode.cRow) {
            machine.App.GCode.cRow = atoi (newValue);
            if (machine.App.GCode.cRow > machine.App.GCode.numRows) {
                if (machine.App.GCode.numRows) {
                    machine.App.GCode.cRow = machine.App.GCode.numRows - 1;
                } else {
                    machine.App.GCode.cRow = -1;
                }
            }

            if (machine.App.GCode.cRow < machine.App.GCode.numRows) {
                BIT_ON(machine.App.GCode.RowsOptions[machine.App.GCode.cRow], BIT28);
            }
        } else {
            machine.App.GCode.cRow = -1;
        }
    }
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
void handle_speed_lin3_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            // vel. lineare / perimetro nominale * rapp. rid.
            // pActuator->speed_auto2 = atof(newValue) * 60.0f / pActuator->cam_ratio;
            pActuator->speed_lin_auto3 = atof(newValue);
            actuator_linear_to_speed((void *)pActuator, atof(newValue), &pActuator->speed_auto3);
        }
    }
}

void handle_speed3_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            // giri/sec * perimetro nominale / rapp. rid.
            // pActuator->speed_lin_auto1 = atof(newValue) / 60.0f * pActuator->cam_ratio;
            pActuator->speed_auto3 = atof(newValue);
            actuator_speed_to_linear((void *)pActuator, atof(newValue), &pActuator->speed_lin_auto3);
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

void handle_speed1_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            // pActuator->speed_lin_auto2 = atof(newValue) / 60.0f * pActuator->cam_ratio;
            pActuator->speed_auto1 = atof(newValue);
            actuator_speed_to_linear((void *)pActuator, atof(newValue), &pActuator->speed_lin_auto1);
        }
    }
}



void handle_working_offset_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            actuator_set_working_offset((void *)pActuator, atof(newValue));
        }
    }
}


void handle_dist_update(void *var, char *newValue, void *userData) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) userData;
    if (pActuator) {
        if (newValue) {
            pActuator->target_rpos = pActuator->cur_rpos + atof(newValue);
            if (pActuator->target_rpos > pActuator->end_rpos) {
                pActuator->target_rpos = pActuator->end_rpos;
            }
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
            /* Operatore omesso : ritorno del valore della variabile*/    \
            /* snprintf((char*) str, str_size, "%s%s=%d", var_prefix, actName, (int)__structName); */ \
        }   \
    }


    /*
    if (__typeOf == TYPE_INT) {   \
        snprintf((char*) str, str_size, "%d", (int)__structName); \
    } else if (__typeOf == TYPE_MSEC) {   \
        snprintf((char*) str, str_size, "%0.3f", (float)(__structName / 1000.0f) ); \
    } else if (__typeOf == TYPE_CHAR) {   \
        snprintf((char*) str, str_size, "%d", (int)(__structName)); \
    } else if (__typeOf == TYPE_STRING) {   \
        snprintf((char*) str, str_size, "%s", (char*)(__structName)); \
    } else if (__typeOf == TYPE_FLOAT3) {    \
        snprintf((char*) str, str_size, "%0.3f", (float)(__structName) ); \
    } else if (__typeOf == TYPE_FLOAT2) {    \
        snprintf((char*) str, str_size, "%0.2f", (float)(__structName) ); \
    } else if (__typeOf == TYPE_FLOAT1) {    \
        snprintf((char*) str, str_size, "%0.1f", (float)(__structName) ); \
    } else if (__typeOf == TYPE_PERCENT) {    \
        snprintf((char*) str, str_size, "%0.1f", (float)(__structName * 100.0f) ); \
    }   \
    */

#define PROCESS_READWRITE_VAR(__varNameString,__structName,__typeOf,__use_update_func,__update_func,__user_data) \
    if (is_var_equal((char*)actProp, (char*)__varNameString)) {  \
        /* Ricerca valore da assegnare */\
        isValueReversed = false;   \
        isValueUpdated = false;    \
        i = strlen(__varNameString);  \
        if (search_for_update_value(actProp, &i, &isValueUpdated, &isValueReversed, &newValue, newSValue, sizeof(newSValue)) > 0) {    \
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
            if (isValueUpdated) {  \
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
                    } else if (__typeOf == TYPE_SRC_STRING) {   \
                        char **__sstructNameString = (char**)((int)__structName); \
                        if(__sstructNameString) {    \
                            char *__structNameString = (char*)(__sstructNameString[0]); \
                            if(__structNameString){ \
                                strncpy((char*)__structNameString, (char*)newSValue, sizeof(__structName));    \
                            } \
                        }\
                    } else if (__typeOf == TYPE_TEXT) {   \
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
// Mode = 0+16   ->  Pacchetto da inoltrare (trasparente alla logica)

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

    uint32_t str_size = var_name ? (strlen((char*)var_name) + 256) : 256;
    char *str = (char *) calloc(str_size+1, 1);
    char var_prefix[256];

    size_t debug_str_size = 256;

    int j;
    char str2[32];

    char newSValue[512];

    bool isValueReversed = false;
    bool isValueUpdated = false;

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
            snprintf((char*) machine.debug_string[0], debug_str_size, "(%d)",
                    (int) machine.sequence );

            // fprintf(stderr, "{%s}\n", machine.debug_string[0]); fflush (stderr);
        }


        if (!machine.debug_string[1])
            machine.debug_string[1] = (char*)calloc(debug_str_size, 1);



        if (machine.debug_string[1]) {
            // Registro preforme (debug)

            snprintf((char*) machine.debug_string[1], debug_str_size, "[");
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
                                    
                                    
                            else PROCESS_READONLY_VAR("task_cycles", machine.task_cycles, TYPE_INT)
                                    
                                    

                                ///////////////////////////////////////// 
                                // Parametri lavoro    
                                //  



                            
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

                            } else if (strcmpi(actName, "SIMULATE") == 0) {
                                PUT_SIMULATE_CYCLE();
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


                            } else if (strcmpi(actName, "RESET_ALARM") == 0) {
                                PUT_RESET_ALARMS();
                                str[0] = 0;

                            } else if (strcmpi(actName, "AUTOSTART") == 0 || strcmpi(actName, "AUTO_START") == 0) {
                                PUT_RESET_ALARMS();
                                PUT_AUTOMATIC();
                                PUT_RESET_ALARMS();
                                PUT_POWER_ON();
                                PUT_START_CYCLE();
                                
                                str[0] = 0;

                            } else if (strcmpi(actName, "testA") == 0) {
                                // G2 test
                                float centerX = machine.actuator[X].cur_rpos + 100.0f;
                                float centerY = machine.actuator[Y].cur_rpos + 100.0f;
                                float Radius = sqrt(100.0f*100.0f + 100.0f*100.0f);
                                float startAngRad = 225.0f / 180.0 * PIGRECO;
                                float endAngRad = 45.0f / 180.0 * PIGRECO;
                                float feed_mm_min = 90.0;
                                float feedZ = 0.0f;
                                uint8_t moveType = 2; // G2 = CW
                                float targetX = centerX + Radius * cosf(endAngRad);
                                float targetY = centerY + Radius * sinf(endAngRad);
                                
                                if (gcode_act_circular_move( (void*)machine.actuator[X].pCANSlot, (void *)&machine.actuator[X], (void *)&machine.actuator[Y], (void *)&machine.actuator[Z],
                                        centerX, centerY, 
                                        Radius, startAngRad, endAngRad, 0.10f,
                                        moveType,
                                        targetX, targetY, machine.actuator[Z].cur_rpos, 
                                        feed_mm_min, feedZ ) > 0) {
                                } else {
                                    snprintf(str, str_size, (char*) "gcode_act_circular_move failed");
                                    if (generate_alarm((char*) str, 7101, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                    }                                    
                                }
                                
                                str[0] = 0;

                            } else if (strcmpi(actName, "testB") == 0) {
                                // G1 test
                                float targetX = machine.actuator[X].cur_rpos + 200.0f;
                                float targetY = machine.actuator[Y].cur_rpos + 200.0f;
                                float targetZ = machine.actuator[Z].cur_rpos + 200.0f;
                                float rapid_feed = 0.0f;
                                
                                if (machine.actuator[X].cur_vpos > 800) {
                                    targetX = 100.0f;
                                }
                                if (machine.actuator[X].cur_vpos > 800) {
                                    targetY = 100.0f;
                                }
                                if (machine.actuator[X].cur_vpos > 800) {
                                    targetZ = 100.0f;
                                }
                                
                                rapid_feed = sqrt(machine.settings.rapid_feed_X * machine.settings.rapid_feed_X + machine.settings.rapid_feed_Y*machine.settings.rapid_feed_Y + machine.settings.rapid_feed_Z*machine.settings.rapid_feed_Z);
                                
                                if (gcode_act_linear_move ( (void*)machine.actuator[X].pCANSlot,
                                        (void*)&machine.actuator[X], (void*)&machine.actuator[Y], (void*)&machine.actuator[Z], 
                                        targetX, targetY, targetZ,
                                        rapid_feed,
                                        0.10f, false ) > 0) {
                                } else {
                                    snprintf(str, str_size, (char*) "gcode_act_linear_move failed");
                                    if (generate_alarm((char*) str, 7102, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                    }                                                                       
                                }
                                
                                str[0] = 0;

                            } else if (strcmpi(actName, "testC") == 0) {
                                // G3 test
                                
                            } else if (strcmpi(actName, "testD") == 0) {
                                // G0 test


                                
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
                            isValueReversed = false;
                            isValueUpdated = false;

                            i = 0;
                            if (search_for_update_value(actProp, &i, &isValueUpdated, &isValueReversed, &newValue, NULL, 0) > 0) {
                                // forza la modalità scrittura
                                Mode |= 4;
                            }


                            if (strcmpi(actName, "...") == 0) {

                            } else if (strnicmp(actName, "spindle_speed", 13) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.spindle_speed, sizeof (machine.settings.spindle_speed), 0+0);
                            } else if (strnicmp(actName, "spindle_power", 13) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT1, str, str_size, (void *) &machine.settings.spindle_power, sizeof (machine.settings.spindle_power), 0+0);
                            } else if (strnicmp(actName, "rapid_feed_X", 12) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.rapid_feed_X, sizeof (machine.settings.rapid_feed_X), 0+0);
                            } else if (strnicmp(actName, "rapid_feed_Y", 12) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.rapid_feed_Y, sizeof (machine.settings.rapid_feed_Y), 0+0);
                            } else if (strnicmp(actName, "rapid_feed_Z", 12) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.rapid_feed_Z, sizeof (machine.settings.rapid_feed_Z), 0+0);
                            } else if (strnicmp(actName, "rapid_feed_W", 12) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.rapid_feed_W, sizeof (machine.settings.rapid_feed_W), 0+0);
                            } else if (strnicmp(actName, "rapid_feed_T", 12) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.rapid_feed_T, sizeof (machine.settings.rapid_feed_T), 0+0);

                            } else if (strnicmp(actName, "mill_feed_X", 11) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.mill_feed_mm_min_X, sizeof (machine.settings.mill_feed_mm_min_X), 0+0);
                            } else if (strnicmp(actName, "mill_feed_Y", 11) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.mill_feed_mm_min_Y, sizeof (machine.settings.mill_feed_mm_min_Y), 0+0);
                            } else if (strnicmp(actName, "mill_feed_Z", 11) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.mill_feed_mm_min_Z, sizeof (machine.settings.mill_feed_mm_min_Z), 0+0);
                            } else if (strnicmp(actName, "mill_feed_W", 11) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.mill_feed_mm_min_W, sizeof (machine.settings.mill_feed_mm_min_W), 0+0);
                            } else if (strnicmp(actName, "mill_feed_T", 11) == 0) {
                                maxFValue = 1000000000.0f, minFValue = 1.0f;
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        &maxFValue, &minFValue, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.mill_feed_mm_min_T, sizeof (machine.settings.mill_feed_mm_min_T), 0+0);
                            
                            } else if (strnicmp(actName, "max_weight", 10) == 0) {
                                maxIValue = 1000000000.0f, minIValue = 0.0f;
                                std_getset_var(var_prefix, actName, &i, 10,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, &maxIValue, &minIValue, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.max_weight, sizeof (machine.settings.max_weight), 0+0);
                            } else if (strnicmp(actName, "max_X", 5) == 0) {
                                std_getset_var(var_prefix, actName, &i, 5,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.max_X, sizeof (machine.settings.max_X), 0+0);
                            } else if (strnicmp(actName, "max_Y", 5) == 0) {
                                std_getset_var(var_prefix, actName, &i, 5,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.max_Y, sizeof (machine.settings.max_Y), 0+0);
                            } else if (strnicmp(actName, "max_Z", 5) == 0) {
                                std_getset_var(var_prefix, actName, &i, 5,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.max_Z, sizeof (machine.settings.max_Z), 0+0);

                            } else if (strnicmp(&actName[i], "TaylorC", 7) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 7,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.TaylorC, 0, 0+0);
                                // PROCESS_READWRITE_VAR("toll", machine.App.TaylorC, TYPE_FLOAT, true, handle_dummy_update, (void*)NULL)
                                
                            } else if (strnicmp(&actName[i], "TaylorN", 7) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 7,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.settings.TaylorN, 0, 0+0);
                                // PROCESS_READWRITE_VAR("toll", machine.App.TaylorN, TYPE_FLOAT, true, handle_dummy_update, (void*)NULL)

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
                            isValueReversed = false;
                            isValueUpdated = false;

                            if (Mode == 0) {
                                int lb = 1;
                            }
                            
                            i = 0;
                            if (search_for_update_value(actProp, &i, &isValueUpdated, &isValueReversed, &newValue, NULL, 0) > 0) {
                                // forza la modalità scrittura
                                Mode |= 4;
                            }


                            if (strcmpi(actName, "...") == 0) {


                            } else if (strnicmp(actName, "loadCount", 9) == 0) {
                                std_getset_var(var_prefix, actName, &i, 9,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.workSet.loadCount, sizeof (machine.workSet.loadCount), 0+0);
                            } else if (strnicmp(actName, "name", 4) == 0) {
                                std_getset_var(var_prefix, actName, &i, 4,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.workSet.name, sizeof (machine.workSet.name), 0+1);
                            } else if (strnicmp(actName, "description", 11) == 0) {
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.workSet.description, sizeof (machine.workSet.description), 0+0);
                            } else if (strnicmp(actName, "rapid_feed", 10) == 0) {
                                std_getset_var(var_prefix, actName, &i, 10,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.rapid_feed, sizeof (machine.workSet.rapid_feed), 0+0);                                
                            } else if (strnicmp(actName, "mill_feed", 9) == 0) {
                                std_getset_var(var_prefix, actName, &i, 9,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.mill_feed, sizeof (machine.workSet.mill_feed), 0+0);
                            } else if (strnicmp(actName, "spindle_speed", 13) == 0) {
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.spindle_speed, sizeof (machine.workSet.spindle_speed), 0+0);
                            } else if (strnicmp(actName, "current_tool", 12) == 0) {
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.workSet.current_tool, sizeof (machine.workSet.current_tool), 0+0);
                            } else if (strnicmp(actName, "tool_name", 9) == 0) {
                                std_getset_var(var_prefix, actName, &i, 9,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.workSet.tool_name, sizeof (machine.workSet.tool_name), 0+0);
                            } else if (strnicmp(actName, "tool_diam", 9) == 0) {
                                std_getset_var(var_prefix, actName, &i, 9,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.tool_diam, sizeof (machine.workSet.tool_diam), 0+0);
                            } else if (strnicmp(actName, "tool_height", 11) == 0) {
                                std_getset_var(var_prefix, actName, &i, 11,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.workSet.tool_height, sizeof (machine.workSet.tool_height), 0+0);
                            } else if (strnicmp(actName, "tool_time", 9) == 0) {
                                std_getset_var(var_prefix, actName, &i, 9,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_SEC_STR, str, str_size, (void *) &machine.workSet.tool_time, sizeof (machine.workSet.tool_time), 0+0);
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

                            } else if (strnicmp(actName, "collisions", 10) == 0) {
                                std_getset_var(var_prefix, actName, &i, 10,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.collisions, sizeof (machine.statistic.collisions), 0+0);

                            } else if (strnicmp(actName, "fatal_errors", 12) == 0) {
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.fatal_errors, sizeof (machine.statistic.fatal_errors), 0+0);

                            } else if (strnicmp(actName, "errors", 6) == 0) {
                                std_getset_var(var_prefix, actName, &i, 6,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.errors, sizeof (machine.statistic.errors), 0+0);

                            } else if (strnicmp(actName, "warnings", 8) == 0) {
                                std_getset_var(var_prefix, actName, &i, 8,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.warnings, sizeof (machine.statistic.warnings), 0+0);
                                
                            } else if (strnicmp(actName, "machine_stops", 13) == 0) {
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.machine_stops, sizeof (machine.statistic.machine_stops), 0+0);

                            } else if (strnicmp(actName, "machine_elapsed_string", 22) == 0) {
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.statistic.machine_elapsed_string, sizeof (machine.statistic.machine_elapsed_string), 0+0);
                            
                            } else if (strnicmp(actName, "machine_running_string", 22) == 0) {
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.statistic.machine_running_string, sizeof (machine.statistic.machine_running_string), 0+0);

                            } else if (strnicmp(actName, "machine_elapsed", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.statistic.machine_elapsed, sizeof (machine.statistic.machine_elapsed), 0+0);

                            } else if (strnicmp(actName, "machine_running", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
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

                            } else if (strnicmp(actName, "collisions", 10) == 0) {
                                std_getset_var(var_prefix, actName, &i, 10,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.collisions, sizeof (machine.rt_statistic.collisions), 0+0);

                            } else if (strnicmp(actName, "fatal_errors", 12) == 0) {
                                std_getset_var(var_prefix, actName, &i, 12,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.fatal_errors, sizeof (machine.rt_statistic.fatal_errors), 0+0);

                            } else if (strnicmp(actName, "errors", 6) == 0) {
                                std_getset_var(var_prefix, actName, &i, 6,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.errors, sizeof (machine.rt_statistic.errors), 0+0);

                            } else if (strnicmp(actName, "warnings", 8) == 0) {
                                std_getset_var(var_prefix, actName, &i, 8,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.warnings, sizeof (machine.rt_statistic.warnings), 0+0);

                            } else if (strnicmp(actName, "machine_stops", 13) == 0) {
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.machine_stops, sizeof (machine.rt_statistic.machine_stops), 0+0);

                            } else if (strnicmp(actName, "machine_elapsed_string", 22) == 0) {
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.rt_statistic.machine_elapsed_string, sizeof (machine.rt_statistic.machine_elapsed_string), 0+0);
                            
                            } else if (strnicmp(actName, "machine_running_string", 22) == 0) {
                                std_getset_var(var_prefix, actName, &i, 22,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_TEXT, str, str_size, (void *) &machine.rt_statistic.machine_running_string, sizeof (machine.rt_statistic.machine_running_string), 0+0);

                            } else if (strnicmp(actName, "machine_elapsed", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_INT, str, str_size, (void *) &machine.rt_statistic.machine_elapsed, sizeof (machine.rt_statistic.machine_elapsed), 0+0);

                            } else if (strnicmp(actName, "machine_running", 15) == 0) {
                                std_getset_var(var_prefix, actName, &i, 15,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
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
                                isValueReversed = false;
                                isValueUpdated = false;

                                if (search_for_update_value(actProp, &i, &isValueUpdated, &isValueReversed, &newValue, NULL, 0) > 0) {
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
                                            
                                            {
    
                                                machine.manual_move++;
                                                machine.should_recover = TRUE;
                                            
                                                if (IOCase == 0) {

                                                } else if (IOCase == 1) {
                                                    // Digital out
                                                    if (isValueUpdated) {
                                                        if (isValueReversed) {
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
                                                    if (isValueUpdated) {
                                                        if (isValueReversed) {
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
                            // Gestione GCODE
                            //

                        } else if (strnicmp(svar_name, "GCODE.", 6) == 0) {
                            uint32_t foundRow1B = 0;
                            char Label[32];

                            i = 0;
                            found1B = 0;
                            actName = svar_name + 6;
                            actProp = NULL;

                            strncpy(var_prefix, svar_name, 6);

                            if (strnicmp(&actName[i], "nROWs", 5) == 0) {                                
                                actProp = actName;
                                PROCESS_READONLY_VAR("nROWs", machine.App.GCode.numRows, TYPE_INT)
                                
                            } else if (strnicmp(&actName[i], "cROW", 4) == 0) {
                                actProp = actName;
                                PROCESS_READWRITE_VAR("cROW", machine.App.GCode.cRow, TYPE_INT, true, handle_gcode_crow_update, (void*)NULL)

                                        
                            } else if (strnicmp(&actName[i], "vROWs", 5) == 0) {
                                actProp = actName;
                                PROCESS_READWRITE_VAR("vROWs", machine.App.GCode.numDisplayRows, TYPE_INT, true, handle_gcode_no_view_row_update, (void*)NULL)
                                
                            } else if (strnicmp(&actName[i], "sROW", 4) == 0) {
                                actProp = actName;
                                PROCESS_READWRITE_VAR("sROW", machine.App.GCode.startRow, TYPE_INT, true, handle_gcode_start_row_update, (void*)NULL)
                                                
                            } else if (strnicmp(&actName[i], "ROW.", 4) == 0) {
                                i+=4;

                                // Ricerca dell' indirizzo della riga
                                actProp = (char *) &actName[i + 0];
                                j = 0;
                                while (actProp[j] != 0 && isdigit(actProp[j])) {
                                    j++;
                                    i++;
                                }
                                char lastChar = actProp[j];
                                actProp[j] = 0;
                                foundRow1B = atoi((char*) actProp);
                                actProp[j] = lastChar;

                                snprintf(Label, sizeof(Label), "Row%d", foundRow1B);
                                
                                if (foundRow1B <= machine.App.GCode.numDisplayRows) {
                                    // PROCESS_READWRITE_VAR(Label, machine.App.GCode.Rows[foundRow1B-1], TYPE_STRING, false, handle_dummy_update, (void*)NULL)

                                    if (actName[i]=='=') {
                                        if (std_getset_var(var_prefix, actName, &i, i,
                                                &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                                NULL, NULL, NULL, NULL, NULL, NULL,
                                                &Mode, TYPE_STRING, str, str_size, (void *) &machine.App.GCode.Rows[foundRow1B-1+machine.App.GCode.startRow], 0, 0+0) > 0) {

                                        // relink del contenuto
                                        gcode_update_start_row ( machine.App.GCode.startRow );
                                        // Contenuto modificato
                                        machine.App.GCode.ContentChanged++;
                                        }
                                    } else {
                                        if (foundRow1B == 12) {
                                            int lb = 1;
                                        }
                                        if (std_getset_var(var_prefix, actName, &i, i,
                                                &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                                NULL, NULL, NULL, NULL, NULL, NULL,
                                                &Mode, TYPE_STRING, str, str_size, (void *) &machine.App.GCode.displayRows[foundRow1B-1], 0, 0+0) > 0) {
                                        }
                                    }
                                }
                                        
                            } else if (strnicmp(&actName[i], "RowOpt.", 7) == 0) {                           
                                i+=7;

                                // Ricerca dell' indirizzo della riga
                                actProp = (char *) &actName[i + 0];
                                j = 0;
                                while (actProp[j] != 0 && isdigit(actProp[j])) {
                                    j++;
                                    i++;
                                }
                                char lastChar = actProp[j];
                                actProp[j] = 0;
                                foundRow1B = atoi((char*) actProp);
                                actProp[j] = lastChar;

                                snprintf(Label, sizeof(Label), "RowOpt%d", foundRow1B);

                                if (foundRow1B <= machine.App.GCode.numDisplayRows) {
                                    // PROCESS_READWRITE_VAR(Label, machine.App.GCode.Rows[foundRow1B-1], TYPE_STRING, false, handle_dummy_update, (void*)NULL)

                                    if (actName[i]=='=') {
                                        maxIValue = BIT31, minIValue = 0.0f;
                                        if (std_getset_var(var_prefix, actName, &i, i,
                                                &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                                NULL, NULL, NULL, NULL, NULL, NULL,
                                                &Mode, TYPE_INT_PTR, str, str_size, (void *) &machine.App.GCode.displayRowsOptions[foundRow1B-1], 0, 0+0) > 0) {
                                        }
                                    } else {                                    
                                        if (std_getset_var(var_prefix, actName, &i, i,
                                                &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                                NULL, NULL, NULL, NULL, NULL, NULL,
                                                &Mode, TYPE_INT_PTR, str, str_size, (void *) &machine.App.GCode.displayRowsOptions[foundRow1B-1], 0, 0+0) > 0) {
                                        }
                                    }
                                }
                                
                            } else if (strnicmp(&actName[i], "CONTENT", 7) == 0) {                           
                                // PROCESS_READWRITE_VAR("CONTENT", machine.App.GCode.Content, TYPE_STRING, false, handle_gcode_update, (void*)NULL)
                                
                                // Ricostruisce il contenuto
                                if (machine.App.GCode.ContentChanged) {
                                    gcode_rebuld_content_from_rows ();
                                }
                                
                                i = 0;                                   
                                std_getset_var(var_prefix, actName, &i, 7,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_STRING, str, str_size, (void *) &machine.App.GCode.Content, 0, 0+0);


                            } else if (strnicmp(&actName[i], "goto", 4) == 0) {
                                if (machine.sequence == WAIT_FOR_START_CYCLE || machine.status == MANUAL) {
                                    // Macchina in mnuale o ciclo in attesa
                                    if (machine.App.GCode.cRow < machine.App.GCode.numRows) {
                                        if (machine.App.GCodeCmd.curRow < machine.App.GCode.numRows) {
                                            BIT_OFF(machine.App.GCode.RowsOptions[machine.App.GCodeCmd.curRow], BIT1);
                                        }
                                        machine.App.GCodeCmd.curRow = machine.App.GCode.cRow;
                                        BIT_ON(machine.App.GCode.RowsOptions[machine.App.GCodeCmd.curRow], BIT1);
                                    }
                                }                                
                            } else if (strnicmp(&actName[i], "insRow", 6) == 0) {
                                
                                if (machine.App.GCode.cRow == machine.App.GCode.numRows-1) {
                                    gcode_add_content ( (char*)"" );
                                } else if (machine.App.GCode.cRow < machine.App.GCode.numRows) {
                                    gcode_insert_content ( (char*)"", machine.App.GCode.cRow );
                                }
                                // relink del contenuto
                                gcode_update_start_row ( machine.App.GCode.startRow );
                                        
                            } else if (strnicmp(&actName[i], "delRow", 6) == 0) {
                                if (machine.App.GCode.cRow < machine.App.GCode.numRows) {
                                    gcode_delete_content ( machine.App.GCode.cRow );
                                }
                                // relink del contenuto
                                gcode_update_start_row ( machine.App.GCode.startRow );
                                
                            } else if (strnicmp(&actName[i], "search", 6) == 0) {
                                // PROCESS_READWRITE_VAR("search", machine.App.GCode.SearchStr, TYPE_STRING, true, handle_dummy_update, (void*)NULL)
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 6,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_STRING, str, str_size, (void *) &machine.App.GCode.SearchStr, 0, 0+0);

                                
                            } else if (strnicmp(&actName[i], "replace", 7) == 0) {
                                // PROCESS_READWRITE_VAR("search", machine.App.GCode.ReplaceStr, TYPE_STRING, true, handle_dummy_update, (void*)NULL)
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 7,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_STRING, str, str_size, (void *) &machine.App.GCode.ReplaceStr, 0, 0+0);
                                        
                            } else if (strnicmp(&actName[i], "do_replace", 10) == 0) {
                                gcode_srep_content ();


                            } else if (strnicmp(&actName[i], "toll", 4) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 4,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_FLOAT3, str, str_size, (void *) &machine.App.toll, 0, 0+0);
                                // PROCESS_READWRITE_VAR("toll", machine.App.toll, TYPE_FLOAT, true, handle_dummy_update, (void*)NULL)

                            } else if (strnicmp(&actName[i], "AfterMillWait", 13) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 13,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_MSEC, str, str_size, (void *) &machine.App.AfterMillWaitTimeMS, 0, 0+0);
                                // PROCESS_READWRITE_VAR("toll", machine.App.AfterMillWaitTimeMS, TYPE_MSEC, true, handle_dummy_update, (void*)NULL)

                            } else if (strnicmp(&actName[i], "XMap", 4) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 4,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_CHAR, str, str_size, (void *) &machine.App.GCodeSetup.XMap, 0, 0+0);

                            } else if (strnicmp(&actName[i], "YMap", 4) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 4,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_CHAR, str, str_size, (void *) &machine.App.GCodeSetup.YMap, 0, 0+0);

                            } else if (strnicmp(&actName[i], "ZMap", 4) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 4,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_CHAR, str, str_size, (void *) &machine.App.GCodeSetup.ZMap, 0, 0+0);

                            } else if (strnicmp(&actName[i], "WMap", 4) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 4,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_CHAR, str, str_size, (void *) &machine.App.GCodeSetup.WMap, 0, 0+0);

                            } else if (strnicmp(&actName[i], "TMap", 4) == 0) {
                                i = 0;                                   
                                actProp = actName;
                                std_getset_var(var_prefix, actName, &i, 4,
                                        &isValueUpdated, &isValueReversed, newSValue, sizeof (newSValue),
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        &Mode, TYPE_CHAR, str, str_size, (void *) &machine.App.GCodeSetup.TMap, 0, 0+0);

                            } else {
                                if (Mode & 1) {
                                    snprintf(str, str_size, (char*) "GCODE's property not found:'%s'", (char*) actProp);
                                    if (generate_alarm((char*) str, 7008, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                    }
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

                                if (machine.actuator[i_act].name && strcmpi(machine.actuator[i_act].name, actName) == 0) {
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
                                        else PROCESS_READONLY_VAR("rPos", machine.actuator[i_act].cur_rpos, TYPE_FIXED_FLOAT3)        // start_pos ... .. end_pos
                                        else PROCESS_READONLY_VAR("cPos", machine.actuator[i_act].position, TYPE_INT)           // 0 / 1
                                        else PROCESS_READONLY_VAR("tgPos", machine.actuator[i_act].target_position, TYPE_INT)    // 0 / 1
                                        else PROCESS_READONLY_VAR("sPos", machine.actuator[i_act].start_rpos, TYPE_FLOAT3)      
                                        else PROCESS_READONLY_VAR("ePos", machine.actuator[i_act].end_rpos, TYPE_FLOAT3)

                                        // else PROCESS_READONLY_VAR("dist", machine.actuator[i_act].dist, TYPE_FLOAT3)
                                        // else PROCESS_READONLY_VAR("rDist", machine.actuator[i_act].dist, TYPE_FIXED_FLOAT3)
                                        else PROCESS_READWRITE_VAR("dist", machine.actuator[i_act].dist, TYPE_FLOAT3, true, handle_dist_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("rDist", machine.actuator[i_act].dist, TYPE_FIXED_FLOAT3, true, handle_dist_update, (void*)&machine.actuator[i_act])

                                        else PROCESS_READWRITE_VAR("workingOffset", machine.actuator[i_act].workingOffset, TYPE_FLOAT3, true, handle_working_offset_update, (void*)&machine.actuator[i_act])
                                                
                                        else PROCESS_READWRITE_VAR("ACC_AUTO1", machine.actuator[i_act].acc_auto1, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("DEC_AUTO1", machine.actuator[i_act].dec_auto1, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("SPEED_AUTO1", machine.actuator[i_act].speed_auto1, TYPE_FLOAT3, true, handle_speed1_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("SPEED_LIN_AUTO1", machine.actuator[i_act].speed_lin_auto1, TYPE_FLOAT1, true, handle_speed_lin1_update, (void*)&machine.actuator[i_act] )
                                        else PROCESS_READWRITE_VAR("FORCE_AUTO1", machine.actuator[i_act].force_auto1, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("SPEED_MAN1", machine.actuator[i_act].speed_man1, TYPE_FLOAT3, false, handle_dummy_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("SPEED_MAN2", machine.actuator[i_act].speed_man2, TYPE_FLOAT3, false, handle_dummy_update, (void*)&machine.actuator[i_act])

                                        else PROCESS_READWRITE_VAR("ACC_AUTO2", machine.actuator[i_act].acc_auto2, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("DEC_AUTO2", machine.actuator[i_act].dec_auto2, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                        else PROCESS_READWRITE_VAR("SPEED_AUTO2", machine.actuator[i_act].speed_auto2, TYPE_FLOAT3, true, handle_speed2_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("SPEED_LIN_AUTO2", machine.actuator[i_act].speed_lin_auto2, TYPE_FLOAT1, true, handle_speed_lin2_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("FORCE_AUTO2", machine.actuator[i_act].force_auto2, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                                
                                        else PROCESS_READWRITE_VAR("SPEED_AUTO3", machine.actuator[i_act].speed_auto3, TYPE_FLOAT3, true, handle_speed3_update, (void*)&machine.actuator[i_act])
                                        else PROCESS_READWRITE_VAR("SPEED_LIN_AUTO3", machine.actuator[i_act].speed_lin_auto3, TYPE_FLOAT1, true, handle_speed_lin3_update, (void*)&machine.actuator[i_act])

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
                                                
                                        else PROCESS_READWRITE_VAR("tgrPos", machine.actuator[i_act].target_rpos, TYPE_FLOAT3, false, handle_dummy_update, (void*)NULL)
                                                


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
                                            
                                        } else if (strcmpi(actProp, "JogStop") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (check_manual_cmd_status()) {
                                                    if (handle_actuator_speed_mode( (void *)&machine.actuator[i_act], (int)0) < 0) {
                                                    }
                                                    machine.manual_move++;
                                                    machine.should_recover = TRUE;
                                                }                                               
                                            } else {
                                                snprintf(str, str_size, (char*) "Machine must be in MANUAL mode");
                                                if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }                                                
                                            }

                                        } else if (strcmpi(actProp, "MOVE") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (check_manual_cmd_status()) {
                                                    // Avvio del movimento
                                                    if (machine.actuator[i_act].step == STEP_READY) {
                                                        if (machine.actuator[i_act].target_rpos >= machine.actuator[i_act].start_rpos && 
                                                            machine.actuator[i_act].target_rpos <= machine.actuator[i_act].end_rpos ) {
                                                            actuator_speed_to_linear( (void *)&machine.actuator[i_act], machine.actuator[i_act].speed_auto1, &machine.actuator[i_act].speed_lin_auto1);
                                                            actuator_speed_to_linear( (void *)&machine.actuator[i_act], machine.actuator[i_act].speed_auto2, &machine.actuator[i_act].speed_lin_auto2);
                                                            // machine.actuator[X].target_position = ...;
                                                            machine.actuator[i_act].position = INDETERMINATE;
                                                            machine.actuator[i_act].target_position = USER_POSITION;
                                                            machine.actuator[i_act].step = STEP_SEND_CMD;
                                                            machine.actuator[i_act].AxisSinCosLin = 2;
                                                        }
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
                                                machine.should_recover = TRUE;
                                                
                                            } else {
                                                snprintf(str, str_size, (char*) "Machine must be in MANUAL mode");
                                                if (generate_alarm((char*) str, 7002, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                }                                                
                                            }
                                            
                                        } else if (strcmpi(actProp, "SETUP") == 0) {
                                            if (machine.status == MANUAL) {
                                                if (check_manual_cmd_status()) {
                                                    if (machine.actuator[i_act].pCANSlot) {
                                                        ((LP_CANSlot)machine.actuator[i_act].pCANSlot)->setupRequest = 1000;
                                                    } else if (machine.actuator[i_act].pSerialSlot) {
                                                    } else {
                                                    }
                                                    machine.should_recover = TRUE;
                                                }
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
                                    // checkDupMode & 4 ->  Controlla la duplicazione per Code, Type, Desc su tutti gli allarmi
                                    if (generate_alarm((char*) str, 7007, 0, (int) ALARM_WARNING, 0+4) < 0) {
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
                                //
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

