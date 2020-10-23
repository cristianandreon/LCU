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



// Macro controllo stringa in uscita
#define DEUBG_CHECK_STRING


/// #define DEBUG_PRINTF

int handle_xproject_command(uint8_t *recvBuffer, uint32_t nReciv, uint8_t *out_str, uint32_t *out_str_size) {

    if (recvBuffer && nReciv && out_str && out_str_size) {
        uint32_t varIndex = 0;
        int Mode = 0 + 0;
        int res = 0, start_index = 0;
        char str[256];

        if (out_str) {

            /*
            if (isdigit((int)recvBuffer[0])) {
                varIndex = (uint16_t)atoi((char*)&recvBuffer[0]);
            }
             */


            // { printf("[handle_xproject_command(varName):%s-%u]\n", (char*)recvBuffer, nReciv); fflush (stdout); }

            //////////////////////////////////
            // Passa il comando alla logica
            //

            start_evalute_protocol:

            if (recvBuffer[start_index] == '!') {
                // Esecuzione / aggiornamento varibile per nome 
                Mode |= 0;
                res = 0;

            } else if (recvBuffer[start_index] == '?') {
                // Risoluzione nome variabile in ID (e messa in cache della risposta)
                Mode |= 1;
                res = 1;               
                
            } else if (recvBuffer[start_index] == '@') {
                // Risoluzione variabile per ID dalla cache
                Mode |= 2;
                res = 2;

            } else if (recvBuffer[start_index] == '>') {
                // Pacchetto da inoltrate (collegamento remoto)
                // vDisplayMessage((char*)recvBuffer);
                Mode |= 16;
                res = 16;
                
                start_index++;
                goto start_evalute_protocol;

            } else if (recvBuffer[start_index] == '#') {
                // Autenticazione
                // vDisplayMessage((char*)recvBuffer);
                Mode |= 8;
                res = 8;

            } else {
                // Modalita non ricinosciuta
                res = -1;
            }


            if (res < 0) {
                // snprintf((char*)str, sizeof(str), "unk cmd=%s", (char *)sanitizeString((char*)recvBuffer));
                snprintf((char*) str, sizeof (str), "[%sunk cmd%s]", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(str);
                out_str[0] = 0;
                
            } else {
                //////////////////////////////////////////////////////////////
                // snprintf((char*) str, sizeof (str), "[Mode:%d]", Mode);
                // vDisplayMessage(str);
                // vDisplayMessage((char*)& recvBuffer[start_index]);
                
                // Mode = 0+0   ->  Esecuzione
                // Mode = 0+1   ->  Risoluzione nome variabile in ID (e messa in cache della risposta)
                // Mode = 0+2   ->  Risoluzione variabile per ID dalla cache
                // Mode = 0+4   ->  Risevato (Scrittura variabile)
                // Mode = 0+8   ->  Autenticazione utente
                // Mode = 0+16   ->  Pacchetto da inoltrare (trasparente alla UI)
                res = logic_get_value((uint8_t *) & recvBuffer[start_index+1], out_str, out_str_size, Mode);
                if (res < 0) {
                    snprintf((char*) str, sizeof (str), "LogicError=%d", res);
                    vDisplayMessage(str);
                    out_str[0] = 0;
                } else {
                    /// printf("[RTC logic result:%s]", (char*)out_str); fflush (stdout);
                }
                
                // snprintf((char*) str, sizeof (str), "\n[OUT]");
                // vDisplayMessage(str);
                // vDisplayMessage((char*)out_str);
                
            }

            
            
            
            *out_str_size = strlen((char*) out_str);

            
            
            
            if (recvBuffer[start_index] == '!') {
#ifdef DEUBG_CHECK_STRING
                if (!checkString((char*) out_str)) {
                    snprintf((char*) str, sizeof (str), "Incorrect exec string:%s", out_str);
                    vDisplayMessage(str);
                }
#endif

            } else if (recvBuffer[start_index] == '?') {
#ifdef DEUBG_CHECK_STRING
                if (!checkString((char*) out_str)) {
                    snprintf((char*) App.Msg, App.MsgSize, "Incorrect resolved string:%s %s %s\n", (char*)ANSI_COLOR_RED, out_str, (char*)ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }
#endif

                
            } else if (recvBuffer[start_index] == '@') {
                // Utilizzo delle stringhe (lento x debug)
#ifdef DEUBG_CHECK_STRING                
                sanitizeString((char*) out_str);
#endif

            }


#ifdef DEBUG_PRINTF
            {
                printf("[handle_xproject_command OUT:'%s' - %u]\n", (char*) out_str, *out_str_size);
                fflush(stdout);
            }
#endif
        }

    } else {
        if (out_str)
            out_str[0] = 0;
        if (out_str_size)
            *out_str_size = 0;
    }


    return 1;
}



// Allocazione statica della cache delle variabili

int initVarToCache() {

    GLCUNumVarsAllocated = 256;
    GLCUNumVars = 0;
    GLCUVars = (cu_vars_cache *) calloc(sizeof (cu_vars_cache) * GLCUNumVarsAllocated, 1);
    if (GLCUVars) {
        return 1;
    } else {
        return -1;
    }
}

int addVarToCache(void *varAddr, char typeOf, int *Id) {
    int retVal = 0;

    if ((int)varAddr < 1024 && (int)varAddr > 0) {
        if (generate_alarm((char*) "Invalid Variable Address\n", 9801, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
        }
        return -1;
    }
    
    for (int i = 0; i < GLCUNumVars; i++) {
        if (GLCUVars[i].varAddr == varAddr) {
            if (GLCUVars[i].typeOf == typeOf) {
                if (Id)
                    *Id = GLCUVars[i].Id;
                return i + 1;
            }
        }
    }

    if (GLCUNumVars < GLCUNumVarsAllocated) {
    } else {
        // riallocazione
        if (check_general_structure_allocated(0, (void**) &GLCUVars, sizeof (GLCUVars[0]), GLCUNumVars, &GLCUNumVarsAllocated, 32, (char*) "Adding variable's cache", (HWND) NULL) < 0) {
        }
    }

    if (GLCUNumVars < GLCUNumVarsAllocated) {

        // Debug
        // printf("[addVarToCache() addr:%d type:=%d]\n", (int)varAddr, (int)typeOf); fflush (stdout);

        GLCUVars[GLCUNumVars].varAddr = varAddr;
        GLCUVars[GLCUNumVars].typeOf = typeOf;
        GLCUVars[GLCUNumVars].Id = GLCUNumVars + 1;

        if (Id)
            *Id = GLCUVars[GLCUNumVars].Id;

        GLCUNumVars++;
        retVal = GLCUNumVars;
    } else {
        if (generate_alarm((char*) "Variable cache out of allocation\n", 9900, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
        }
        retVal = -1;
    }

    return retVal;
}



static char GLShowErrorSize = 0;

int getVarFromCache(int Id, char **str, uint32_t *str_size) {
    int retVal = 0;

    
    try {
        
   
        for (int i = 0; i < GLCUNumVars; i++) {

            if (GLCUVars[i].Id == Id) {

                if (str) {

                    if (str[0]) {

                        if (GLCUVars[i].typeOf == TYPE_STRING) {
                            char **ppStr = (char **) GLCUVars[i].varAddr;
                            if (ppStr) {
                                strncpy((char*) str[0], (char*) (*ppStr?*ppStr:""), str_size[0]);
                            }
                        } else if (GLCUVars[i].typeOf == TYPE_SRC_STRING) {
                            char ***pppStr = (char ***) GLCUVars[i].varAddr;
                            if (pppStr) {
                                char **ppStr = (char **) pppStr[0];
                                if (ppStr) {
                                    strncpy((char*) str[0], (char*) (*ppStr?*ppStr:""), str_size[0]);
                                } else {
                                    strncpy((char*) str[0], (char*) "", str_size[0]);                                    
                                }
                            }
                        } else if (GLCUVars[i].typeOf == TYPE_TEXT) {
                            char *pStr = (char *) GLCUVars[i].varAddr;
                            strncpy((char*) str[0], (char*) (pStr?pStr:""), str_size[0]);
                            
                        } else if (GLCUVars[i].typeOf == TYPE_INT) {
                            snprintf((char*) str[0], str_size[0], "%d", *((int*) GLCUVars[i].varAddr));
                            
                        } else if (GLCUVars[i].typeOf == TYPE_INT_PTR) {
                            int **pInt = (int**) GLCUVars[i].varAddr;
                            if (pInt) {
                                snprintf((char*) str[0], str_size[0], "%d", pInt[0][0]);
                            }

                        } else if (GLCUVars[i].typeOf == TYPE_FLOAT3) {
                            snprintf((char*) str[0], str_size[0], "%0.3f", *((float*) GLCUVars[i].varAddr));
                            auto_size_string(str[0], 0);

                        } else if (GLCUVars[i].typeOf == TYPE_FLOAT2) {
                            snprintf((char*) str[0], str_size[0], "%0.2f", *((float*) GLCUVars[i].varAddr));
                            auto_size_string(str[0], 0);

                        } else if (GLCUVars[i].typeOf == TYPE_FLOAT1) {
                            snprintf((char*) str[0], str_size[0], "%0.1f", *((float*) GLCUVars[i].varAddr));
                            auto_size_string(str[0], 0);

                        } else if (GLCUVars[i].typeOf == TYPE_FIXED_FLOAT3) {
                            snprintf((char*) str[0], str_size[0], "%0.3f", *((float*) GLCUVars[i].varAddr));
                            auto_size_string(str[0], 3);

                        } else if (GLCUVars[i].typeOf == TYPE_FIXED_FLOAT2) {
                            snprintf((char*) str[0], str_size[0], "%0.2f", *((float*) GLCUVars[i].varAddr));
                            auto_size_string(str[0], 2);

                        } else if (GLCUVars[i].typeOf == TYPE_FIXED_FLOAT1) {
                            snprintf((char*) str[0], str_size[0], "%0.1f", *((float*) GLCUVars[i].varAddr));
                            auto_size_string(str[0], 1);

                        
                        
                        } else if (GLCUVars[i].typeOf == TYPE_FLOAT1_TEMPERATURE) {
                            snprintf((char*) str[0], str_size[0], "%0.1f", (float) ((float) ((unsigned int *) GLCUVars[i].varAddr)[0] / 1000.0f));
                            auto_size_string(str[0], 1);


                       } else if (GLCUVars[i].typeOf == TYPE_FLOAT3_UINT) {
                            snprintf((char*) str[0], str_size[0], "%0.3f", (float) ((float) ((unsigned int *) GLCUVars[i].varAddr)[0] / 65535.0f));
                            auto_size_string(str[0], 0);

                        } else if (GLCUVars[i].typeOf == TYPE_FLOAT2_UINT) {
                            snprintf((char*) str[0], str_size[0], "%0.2f", (float) ((float) ((unsigned int *) GLCUVars[i].varAddr)[0] / 65535.0f));
                            auto_size_string(str[0], 0);

                        } else if (GLCUVars[i].typeOf == TYPE_MSEC) {
                            snprintf((char*) str[0], str_size[0], "%0.3f", (float) (*((unsigned int*) GLCUVars[i].varAddr)) / 1000.0f);
                            auto_size_string(str[0], 0);

                        } else if (GLCUVars[i].typeOf == TYPE_SEC) {
                            snprintf((char*) str[0], str_size[0], "%d", (uint32_t) (*((unsigned int*) GLCUVars[i].varAddr)) );
                            auto_size_string(str[0], 0);

                        } else if (GLCUVars[i].typeOf == TYPE_SEC_STR) {
                            uint32_t time_sec = (*((unsigned int*) GLCUVars[i].varAddr));
                            format_time_run(time_sec, (char*)str[0], str_size[0]);
                            // auto_size_string(str[0], 0);

                        } else if (GLCUVars[i].typeOf == TYPE_PERCENT) {
                            snprintf((char*) str[0], str_size[0], "%0.1f", (float) (*((float*) GLCUVars[i].varAddr)) * 100.0f);
                            auto_size_string(str[0], 1);

                        } else if (GLCUVars[i].typeOf == TYPE_BOOL) {
                            snprintf((char*) str[0], str_size[0], "%s", *((int*) GLCUVars[i].varAddr) == 0 ? "OFF" : "ON");

                        } else if (GLCUVars[i].typeOf == TYPE_ENUM) {
                            unsigned char *penum = (unsigned char *) GLCUVars[i].varAddr;
                            snprintf((char*) str[0], str_size[0], "%d", (unsigned char) penum[0]);

                        } else if (GLCUVars[i].typeOf == TYPE_CHAR) {
                            char *pChar = (char *) GLCUVars[i].varAddr;
                            snprintf((char*) str[0], str_size[0], "%d", (int) pChar[0]);

                        } else if (GLCUVars[i].typeOf == TYPE_MAC_STAT) {
                            snprintf((char*) str[0], str_size[0], "%s", (char*) (get_machine_status_desc()));

                            /////////////////////////////
                            // Tipi speciali (runtime)
                            //
                        } else if (GLCUVars[i].typeOf == TYPE_SYS_TICK) {
                            snprintf((char*) str[0], str_size[0], "%d", (uint32_t) (xTaskGetTickCount()));

                        } else if (GLCUVars[i].typeOf == TYPE_SYS_TIME) {
                            snprintf((char*) str[0], str_size[0], "%0.3f", (float) (xTaskGetTickCount() * portTICK_RATE_MS) / 1000.0f);

                        } else if (GLCUVars[i].typeOf == TYPE_MAC_ADDR) {
                            snprintf((char*) str[0], str_size[0], "%x.%x.%x.%x.%x.%x", App.MacAddress[0], App.MacAddress[1], App.MacAddress[2], App.MacAddress[3], App.MacAddress[4], App.MacAddress[5]);

                        } else if (GLCUVars[i].typeOf == TYPE_ACTUATOR_STATUS) {
                            LP_ACTUATOR pAct = (LP_ACTUATOR) GLCUVars[i].varAddr;                        
                            char *step_name = get_actuator_step((void *)pAct);
                            snprintf((char*) str[0], str_size[0], "%s", step_name?step_name:"");

                        } else if (GLCUVars[i].typeOf == TYPE_ACTUATOR_DRIVER_STATUS) {
                            LP_ACTUATOR pAct = (LP_ACTUATOR) GLCUVars[i].varAddr;                        
                            char *act_driver_stat = get_actuator_driver_status((void *)pAct);
                            snprintf((char*) str[0], str_size[0], "%s", act_driver_stat?act_driver_stat:"");


                        } else if (GLCUVars[i].typeOf == TYPE_TRACE_NUM_DATA) {
                            LPP_ACTUATOR_TRACE ppTrace = (LPP_ACTUATOR_TRACE) GLCUVars[i].varAddr;
                            LP_ACTUATOR_TRACE pTrace = (LP_ACTUATOR_TRACE) ppTrace[0];
                            if (pTrace) {
                                snprintf((char*) str[0], str_size[0], "%d", (int)pTrace->num_data);
                            } else {
                                str[0][0] = 0;
                            }

                        } else if (GLCUVars[i].typeOf == TYPE_TRACE_HEADER) {
                            LPP_ACTUATOR_TRACE ppTrace = (LPP_ACTUATOR_TRACE) GLCUVars[i].varAddr;
                            LP_ACTUATOR_TRACE pTrace = (LP_ACTUATOR_TRACE) ppTrace[0];
                            if (pTrace) {
                                size_t outputHeaderLength = 0, length = sizeof (pTrace[0]);
                                char *encodedDataHeader = NewBase64Encode((void *) pTrace, length, FALSE, &outputHeaderLength);

                                strncpy((char*) str[0], encodedDataHeader, str_size[0]);

                                if (encodedDataHeader)
                                    free(encodedDataHeader);
                            } else {
                                str[0][0] = 0;
                            }
                        } else if (GLCUVars[i].typeOf == TYPE_TRACE_POS) {
                            LPP_ACTUATOR_TRACE ppTrace = (LPP_ACTUATOR_TRACE) GLCUVars[i].varAddr;
                            LP_ACTUATOR_TRACE pTrace = (LP_ACTUATOR_TRACE) ppTrace[0];
                            if (pTrace) {
                                size_t outputBodyLength = 0, length = sizeof (pTrace->pos[0]) * pTrace->num_data;

                                if (length > 0 && pTrace->Status == 3) {
                                    char *encodedDataBody = NewBase64Encode((void *) pTrace->pos, length, FALSE, &outputBodyLength);


                                    // Evita rischieste multiple
                                    pTrace->Status = 4;

                                    if (outputBodyLength > str_size[0]) {
                                        str_size[0] = outputBodyLength + 16;
                                        str[0] = (char*) realloc(str[0], str_size[0]);
                                        if (!str[0])
                                            str_size[0] = 0;
                                    }
                                    if (outputBodyLength > str_size[0]) {
                                        if (encodedDataBody)
                                            encodedDataBody[0] = 0;
                                        if (!GLShowErrorSize) {
                                            fprintf(stdout, "[Error] not enough room for track data :%d/%d", outputBodyLength, str_size[0]);
                                            GLShowErrorSize = 1;
                                        }
                                    }

                                    strncpy((char*) str[0], encodedDataBody, outputBodyLength + 1);

                                    if (encodedDataBody)
                                        free(encodedDataBody);
                                } else {
                                    str[0][0] = 0;
                                }
                            } else {
                                str[0][0] = 0;
                            }

                        } else if (GLCUVars[i].typeOf == TYPE_CONSOLE) {

                            char *consoleContent = readConsole();

                            if (consoleContent) {
                                uint32_t consoleLength = strlen(consoleContent);

                                if (consoleLength > str_size[0]) {
                                    str_size[0] = consoleLength + 16;
                                    str[0] = (char*) realloc(str[0], str_size[0]);
                                    if (!str[0])
                                        str_size[0] = 0;
                                }

                                if (consoleLength > str_size[0]) {
                                    snprintf(App.Msg, App.MsgSize, "[Error] not enough room for consol data :%d/%d", consoleLength, str_size[0]);
                                    vDisplayMessage(App.Msg);
                                }

                                strncpy((char*) str[0], consoleContent, consoleLength + 1);

                                // Buffer gloable
                                // free(consoleContent);
                                // consoleContent = NULL;
                            }



                            // Elenco allarmi
                        } else if (GLCUVars[i].typeOf == TYPE_ALARM_LIST || GLCUVars[i].typeOf == TYPE_LAST_ALARM_LIST) {
                            bool proceedAlarmList = true;

                            if (machine.sequence==0 || machine.sequence==WAIT_FOR_RESUME_CYCLE) {
                                int lb = 1;
                            }

                            if (GLCUVars[i].typeOf == TYPE_LAST_ALARM_LIST) {
                                if (machine.lastAlarmListId == machine.lastAlarmId && machine.lastAlarmListId!= 0xFFFFFFFF) {
                                    proceedAlarmList = false;
                                } else {
                                    /*
                                    snprintf(App.Msg, App.MsgSize, "{LAST_ALARM_LIST - lastId/Id:%d/%d}", machine.lastAlarmListId, machine.lastAlarmId);
                                    vDisplayMessage(App.Msg);
                                     */
                                    // DEBUG
                                    if (machine.numAlarmList) {
                                        // App.dumpWebSocket = 3;
                                    }
                                    proceedAlarmList = true;
                                }
                            } else if (GLCUVars[i].typeOf == TYPE_ALARM_LIST) {
                                if (machine.curAlarmList > machine.numAlarmList) {
                                    // reset : interfaccia non allineata
                                    machine.lastAlarmListId = 0;
                                    machine.curAlarmList = 0;
                                    machine.rebuildAlarmList = 1;
                                    // snprintf(App.Msg, App.MsgSize, "curAlarmList resetted\n" ); 
                                    // vDisplayMessage(App.Msg);
                                }

                                if (machine.curAlarmList == machine.numAlarmList && machine.rebuildAlarmList == 0) {
                                    proceedAlarmList = false;
                                } else {
                                    /*
                                    snprintf(App.Msg, App.MsgSize, "{ALARM_LIST - cur/num/rebuild:%d/%d/%d}", machine.curAlarmList, machine.numAlarmList, machine.rebuildAlarmList);
                                    vDisplayMessage(App.Msg);
                                    */
                                    proceedAlarmList = true;
                                }
                            }
                            

                            if (proceedAlarmList) {
                                char *serializedJSON = serialize_alarm((GLCUVars[i].typeOf == TYPE_ALARM_LIST ? machine.curAlarmList : (machine.numAlarmList < 3 ? 0 : (machine.numAlarmList - 3))), GLCUVars[i].typeOf);

                                if (serializedJSON) {
                                    uint32_t outputBodyLength = 0;

                                    /*
                                    snprintf(App.Msg, App.MsgSize, "[serializedJSON:");
                                    vDisplayMessage(App.Msg);
                                    vDisplayMessage(serializedJSON);
                                    snprintf(App.Msg, App.MsgSize, "]");
                                    vDisplayMessage(App.Msg);
                                    */
                                    
                                    char *encodedDataBody = NewBase64Encode((void *) serializedJSON, strlen(serializedJSON), FALSE, &outputBodyLength);


                                    if (outputBodyLength > str_size[0]) {
                                        str_size[0] = outputBodyLength + 16;
                                        str[0] = (char*) realloc(str[0], str_size[0]);
                                        if (!str[0])
                                            str_size[0] = 0;
                                    }
                                    if (outputBodyLength > str_size[0]) {
                                        if (encodedDataBody)
                                            encodedDataBody[0] = 0;
                                        if (!GLShowErrorSize) {
                                            snprintf(App.Msg, App.MsgSize, "[Error] not enough room for alarms :%d/%d", outputBodyLength, str_size[0]);
                                            vDisplayMessage(App.Msg);
                                            GLShowErrorSize = 1;
                                        }
                                    }

                                    strncpy((char*) str[0], encodedDataBody, outputBodyLength + 1);

                                    
                                    // tutti gli allarmi sono stati inviati
                                    if (GLCUVars[i].typeOf == TYPE_ALARM_LIST) {
                                        machine.curAlarmList = machine.numAlarmList;
                                    } else if (GLCUVars[i].typeOf == TYPE_LAST_ALARM_LIST) {
                                        machine.lastAlarmListId = machine.lastAlarmId;
                                    }

                                    if (encodedDataBody)
                                        free(encodedDataBody);
                                } else {
                                    str[0][0] = 0;
                                    if (machine.numAlarmList) {
                                        snprintf(App.Msg, App.MsgSize, "[Error failed to serialize alarms!]");
                                        vDisplayMessage(App.Msg);
                                    }
                                }
                            } else {
                                str[0][0] = 0;
                            }



                        } else {
                            snprintf((char*) str[0], str_size[0], "?");
                        }


                        checkString(str[0]);

                        retVal = 1;
                    }
                }
                break;
            }
        }
        
        return retVal;
        
    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "getVarFromCache() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "getVarFromCache() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }   
    
    return -1;
}

int updateVar(void *varAddr, char typeOf, char *newValue) {
    int retVal = 0;

    if (varAddr) {

        if (newValue) {

            if (newValue[0] == '=') {
                int i = 1;

                if (typeOf == TYPE_STRING) {
                    char **pString = (char **) varAddr;

                    pString[0] = (char*) realloc(pString[0], strlen(newValue) + 1);
                    strcpy((char*) pString[0], &newValue[1]);

                } else if (typeOf == TYPE_SRC_STRING) {
                    char ***pppString = (char ***) varAddr;
                    if (pppString) {
                        char **ppString = (char **) pppString;

                        ppString[0] = (char*) realloc(ppString[0], strlen(newValue) + 1);
                        strcpy((char*) ppString[0], &newValue[1]);
                    }
                    
                } else if (typeOf == TYPE_INT) {
                    int *pInt = (int*) varAddr;
                    pInt[0] = atoi((char*) &newValue[1]);
                    
                } else if (typeOf == TYPE_INT_PTR) {
                    int **ppInt = (int**) varAddr;
                    if (ppInt) {
                        int *pInt = (int*) ppInt[0];
                        if (pInt) {
                            pInt[0] = atoi((char*) &newValue[1]);
                        }
                    }
                    

                } else if (typeOf == TYPE_FLOAT3 || typeOf == TYPE_FLOAT2 || typeOf == TYPE_FLOAT3_UINT || typeOf == TYPE_FLOAT2_UINT) {
                    float *pFloat = (float*) varAddr;

                    while (isdigit(newValue[i]) || newValue[i] == '+' || newValue[i] == '-' || newValue[i] == '.' || newValue[i] == ',') i++;
                    newValue[i] = 0;
                    pFloat[0] = atof((char*) &newValue[1]);

                } else if (typeOf == TYPE_MSEC) {
                    int *pInt = (int*) varAddr;

                    pInt[0] = atoi((char*) &newValue[1]);

                } else if (typeOf == TYPE_BOOL) {
                    int *pInt = (int*) varAddr;
                    pInt[0] = atoi((char*) &newValue[1]);

                } else if (typeOf == TYPE_ENUM) {


                } else if (typeOf == TYPE_CHAR) {
                    char *pChar = (char*) varAddr;
                    pChar[0] = newValue[1];

                } else if (typeOf == TYPE_SYS_TICK) {
                } else if (typeOf == TYPE_SYS_TIME) {
                } else if (typeOf == TYPE_TRACE_HEADER) {
                } else if (typeOf == TYPE_TRACE_POS) {
                } else if (typeOf == TYPE_TRACE_NUM_DATA) {
                }
            }
        }
    }

    return retVal;
}

int handleUpdateVarFailed(char *newValue) {
    int retVal = 0;

    fprintf(stderr, "[Unable to update variable:%s%s%s]", (char*) ANSI_COLOR_RED, newValue != NULL ? newValue : "[NULL]", (char*) ANSI_COLOR_RESET);

    return retVal;
}

int auto_size_string(char *str, int nDigits) {
    if (str) {
        int len = strlen(str), len2 = 0;
        if (len) {
            len--;
            while (str[len] == '0' && len > 0)
                len--;
            len2 = len;
            if (str[len] == '.' || str[len] == ',') {
                if (nDigits > 0 && nDigits <= 12) {
                    str[len+1] = 0;
                    for (int32_t i=0; i<nDigits; i++) {
                        strcat(str, "0");
                    }
                } else {
                    str[len] = 0;
                    return 1;
                }
            }
            while (str[len] != '.' && str[len] != ',' && len > 0)
                len--;
            if (str[len] == ',' || str[len] == '.') {
                // Trovato il separatore di decimali
                if (nDigits > 0 && nDigits <= 12 && nDigits >= (len2-len) ) {
                    str[len2+1] = 0;
                    for (int32_t i=0; i<nDigits-(len2-len); i++) {
                        strcat(str, "0");
                    }
                } else if (nDigits > 0 && nDigits <= 12 && nDigits < (len2-len) ) {
                    str[len+nDigits+1] = 0;
                    return 1;
                } else {
                    str[len2+1] = 0;
                    return 1;
                }
            }
            return 1;
        }
    }
    return 0;
}