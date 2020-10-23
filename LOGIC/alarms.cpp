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



// 70xx = allarmi comandi manuali
// 60xx = allarmi comunicazione seriale
// 50xx = allarmi comunicazione I/O
// 20xx = log eventi comunicazioni

int reset_alarms(void) {
    uint32_t i, fatalErrors = 0;
    
    // Ricerca errori fatali
    for (i=0; i<machine.numAlarmList; i++) {
        if (machine.alarmList[i].Type == ALARM_FATAL_ERROR) 
            fatalErrors++;                
        }

    if (fatalErrors) {
        snprintf(machine.status_message, machine.status_message_size, "%d Fatal error(s) detected! Should reboot the system...", fatalErrors);
        snprintf(App.Msg, App.MsgSize, "[%s%s%s]", (char*)ANSI_COLOR_RED, (char*)machine.status_message, (char*)ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);
        
        if (App.SimulateMode) {
            snprintf(App.Msg, App.MsgSize, "[%s%s%s]", (char*)ANSI_COLOR_MAGENTA, (char*)"Alarms resetted due to SimulateMode active", (char*)ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            goto handle_reset_alarms;
        }
        
    } else {
         handle_reset_alarms:
        if (machine.status == EMERGENCY) {
            machine.status = INITIALIZED;
            on_machine_initialized();
        }
        strncpy(machine.status_message, "", machine.status_message_size);
        machine.sequence = ALARM_RESETTED_SEQUENCE;
        machine.numAlarmList = 0;
        machine.curAlarmList = 0;
        machine.lastAlarmListId = -1;
        machine.rebuildAlarmList = 1;
    }

    bool reinitActuators = false;
    for (i = 0; i < machine.num_actuator; i++) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) & machine.actuator[i];
        pActuator->error = 0;
        if (pActuator->step == STEP_UNINITIALIZED || pActuator->step == STEP_STOPPED || pActuator->step == STEP_ERROR) {
            reinitActuators = true;
        }
    }
    
    if (reinitActuators) {
        process_actuators_initialize(false);
    }
    
    return 0;
}


static char GLAlarmString[512];

// checkDupMode & 1 ->  Controlla la duplicazione per Code, Type, Desc
// checkDupMode & 2 ->  Controlla la duplicazione per Code, Type
// checkDupMode & 4 ->  Controlla la duplicazione per Code, Type, Desc su tutti gli allarmi

int generate_alarm(char *Desc, int Code, int actuatorId, int Type, int checkDupMode) {
    ALARM localAlarm = {0};
    time_t rawtime;
    struct tm * timeinfo;
    char DateTime[80];
    int res = 0;

    try {

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(DateTime, sizeof (DateTime), "%d-%m-%Y %I:%M:%S", timeinfo);
        
        if (Type == ALARM_FATAL_ERROR || Type == ALARM_ERROR) {
            if (machine.status == AUTOMATIC) {
                machine.rt_statistic.machine_stops++;
                machine.statistic.machine_stops++;
            }
	}        

        if (Type == ALARM_FATAL_ERROR || Type == ALARM_ERROR) {
            // Emerganza macchina
            emergency();
        }
        
        // Anti-duplicazione
        if (checkDupMode & 4) {
            // Controlla la duplicazione per Code, Type su tutti gli allarmi
            if (machine.alarmList) {
                for (uint32_t i=0; i<machine.numAlarmList; i++) {
                    if (machine.alarmList[machine.numAlarmList-1].Type == Type) {
                        if (machine.alarmList[machine.numAlarmList-1].Code == Code) {
                            if (machine.alarmList[machine.numAlarmList-1].Desc && Desc) {
                                if (strcmpi(machine.alarmList[machine.numAlarmList-1].Desc, Desc)) {
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        } else if (checkDupMode > 0) {
            if (machine.alarmList) {
                if (machine.numAlarmList>0) {
                    if (machine.alarmList[machine.numAlarmList-1].Type == Type) {
                        if (machine.alarmList[machine.numAlarmList-1].Code == Code) {
                            if (!(checkDupMode & 2)) {
                                if (machine.alarmList[machine.numAlarmList-1].Desc && Desc) {
                                    if (strcmpi(machine.alarmList[machine.numAlarmList-1].Desc, Desc)) {
                                        // Aggiorna la data
                                        COPY_POINTER(machine.alarmList[machine.numAlarmList-1].DateTime, DateTime);
                                        // snprintf(App.Msg, App.MsgSize, "[%sALARN filtered out #1%s]", (char*)ANSI_COLOR_RED, (char*)ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                                        return 0;
                                    } else {
                                        // Continua
                                    }
                                }
                            } else {
                                // Allarme già presente
                                // snprintf(App.Msg, App.MsgSize, "[%sALARN filtered out #2%s]", (char*)ANSI_COLOR_RED, (char*)ANSI_COLOR_RESET);                                 vDisplayMessage(App.Msg);
                                return 0;
                            }
                        }
                    }
                }
            }
        }

        
        if (Type == ALARM_FATAL_ERROR || Type == ALARM_ERROR) {
            // Mostra l'allarme nella consolle
            if (Type == ALARM_FATAL_ERROR) {
                snprintf(GLAlarmString, sizeof(GLAlarmString), "[%sERROR %d:%s%s]\n", (char*) ANSI_COLOR_MAGENTA, Code, Desc, (char*) ANSI_COLOR_RESET);
            } else if (Type == ALARM_ERROR) {
                snprintf(GLAlarmString, sizeof(GLAlarmString), "[%sERROR %d:%s%s]\n", (char*) ANSI_COLOR_RED, Code, Desc, (char*) ANSI_COLOR_RESET);
            }
            vDisplayMessage(GLAlarmString);
            machine.statistic.errors++;

        } else if (Type == ALARM_WARNING) {
            if (App.DebugMode) {
                snprintf(GLAlarmString, sizeof(GLAlarmString), "[%sWARNING %d:%s%s]\n", (char*) ANSI_COLOR_YELLOW, Code, Desc, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(GLAlarmString);
            }
            machine.statistic.warnings++;
           
        } else if (Type == ALARM_LOG) {
            snprintf(GLAlarmString, sizeof(GLAlarmString), "[%sLOG %d:%s%s]\n", (char*) ANSI_COLOR_BLUE, Code, Desc, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(GLAlarmString);
        }

        
        
        // STatistiche
        if (Type == ALARM_FATAL_ERROR) {
            machine.rt_statistic.fatal_errors++;
            machine.statistic.fatal_errors++;
        } else if (Type == ALARM_ERROR) {
            machine.rt_statistic.errors++;
            machine.statistic.errors++;
        } else if (Type == ALARM_WARNING) {
            machine.rt_statistic.warnings++;
            machine.statistic.warnings++;
        }
                

                
                
                
        localAlarm.Desc = Desc;
        localAlarm.Code = Code;
        localAlarm.actuatorId = actuatorId;
        localAlarm.Type = (ERRORS_TYPE_ENUM)Type;
        localAlarm.id = ++machine.lastAlarmId;


        localAlarm.DateTime = (char*) DateTime;

        res =  add_alarm(&localAlarm, 0+0);

        if (res < 0) {
            snprintf(App.Msg, App.MsgSize, "[%s Unable to add alarm %s]\n", (char*)ANSI_COLOR_MAGENTA, (char*)ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }
        
        return res;

    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "generate_alarm() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "generate_alarm() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }    
}



int add_alarm(LP_ALARM pAlarm, int Mode) {

    if (!machine.alarmList)
        machine.numAlarmListAllocated = 0;

    if (machine.numAlarmList <= machine.numAlarmListAllocated) {
        if (check_general_structure_allocated(0, (void**)&machine.alarmList, sizeof(ALARM), machine.numAlarmList, &machine.numAlarmListAllocated, 1024, (char*)"Adding alarm", (HWND)NULL ) < 0) {
            return -1;
        }
    }


    if (machine.alarmList) {
        machine.alarmList[machine.numAlarmList].Code = pAlarm->Code;
        COPY_POINTER(machine.alarmList[machine.numAlarmList].Desc, pAlarm->Desc);
        COPY_POINTER(machine.alarmList[machine.numAlarmList].DateTime, pAlarm->DateTime);
        machine.alarmList[machine.numAlarmList].Type = pAlarm->Type;
        machine.alarmList[machine.numAlarmList].actuatorId = pAlarm->actuatorId;
        machine.numAlarmList++;
    } else {
        return -1;
    }

    // curalarm;

    return machine.numAlarmList;
}

/*
 * Oggetto JSON
alarmList = {
    "count":30,
    "alarms": [
       [  "Type", "Code", "Desc", "DateTime","actuatorId" ],
       [  "Type", "Code", "Desc", "DateTime","actuatorId" ],
       [ ... ]
    ]
 }
 */





char *serialize_alarm(uint32_t iStartAlarm, int typeOf) {
    
    uint32_t startAlarm = iStartAlarm;
    char *jsonStr = NULL;
    
    if (startAlarm > machine.numAlarmList) startAlarm = machine.numAlarmList;
        
    {   uint32_t jsonStrSize = 256 + (machine.numAlarmList-startAlarm) * 256;
        int rebuild = 0;
        uint32_t i = 0;
        char str[512];


        
        jsonStr = (char*)malloc(jsonStrSize);

        if (!jsonStr)
            return (char*)NULL;
        
        jsonStr[0] = 0;

        if (startAlarm > machine.numAlarmList) {
            startAlarm = 0;
            rebuild = 1;
        }


        if (typeOf == TYPE_LAST_ALARM_LIST) {
        } else if (typeOf == TYPE_ALARM_LIST) {
            if (machine.rebuildAlarmList) {
                machine.rebuildAlarmList = 0;
                rebuild = 1;
                startAlarm = 0;
            }
        }
        
        if (machine.alarmList) {

            // N.B.: anche l'elenco vuoto dev'essere
            if (machine.numAlarmList >= 0) {
                
                
                if (startAlarm <= machine.numAlarmList) {

                    CpyStr(&jsonStr, (char*)"{\"alarmList\":{\"tot\":", &jsonStrSize);
                    snprintf(str, sizeof (str), "%d", machine.numAlarmList);
                    AddStr(&jsonStr, str, &jsonStrSize);

                    AddStr(&jsonStr, (char*)",\"count\":", &jsonStrSize);
                    snprintf(str, sizeof (str), "%d", (machine.numAlarmList-startAlarm));
                    AddStr(&jsonStr, str, &jsonStrSize);
                    
                    AddStr(&jsonStr, (char*)",\"rebuild\":", &jsonStrSize);
                    snprintf(str, sizeof (str), "%d", rebuild);
                    AddStr(&jsonStr, str, &jsonStrSize);
                    
                    if (typeOf == TYPE_LAST_ALARM_LIST) {
                        AddStr(&jsonStr, (char*)",\"lastAlarms\":", &jsonStrSize);
                        snprintf(str, sizeof (str), "%d", 1);
                        AddStr(&jsonStr, str, &jsonStrSize);
                    } else {
                    }
                            
                    AddStr(&jsonStr, (char*)",\"alarms\":[", &jsonStrSize);


                    for (i = startAlarm; i < machine.numAlarmList; i++) {

                        if (i>startAlarm) {
                            AddStr(&jsonStr, (char*)",[", &jsonStrSize);
                        } else {
                            AddStr(&jsonStr, (char*)"[", &jsonStrSize);
                        }

                        snprintf(str, sizeof (str), "\"%d\"", machine.alarmList[i].Type);
                        AddStr(&jsonStr, str, &jsonStrSize);

                        snprintf(str, sizeof (str), ",\"%d\"", machine.alarmList[i].Code);
                        AddStr(&jsonStr, str, &jsonStrSize);

                        if (machine.alarmList[i].Desc) {
                            for (int ic=0; ic<strlen(machine.alarmList[i].Desc); ic++) {
                                if (machine.alarmList[i].Desc[ic] == '"') {
                                    machine.alarmList[i].Desc[ic] = '\'';
                                }
                                if (machine.alarmList[i].Desc[ic] == '\n') {
                                    machine.alarmList[i].Desc[ic] = ' ';
                                }
                                if (machine.alarmList[i].Desc[ic] == '\r') {
                                    machine.alarmList[i].Desc[ic] = ' ';
                                }
                            }
                        }
                        AddStr(&jsonStr, (char*)",\"", &jsonStrSize);
                        AddStr(&jsonStr, (char*)(machine.alarmList[i].Desc ? machine.alarmList[i].Desc : ""), &jsonStrSize);
                        AddStr(&jsonStr, (char*)"\"", &jsonStrSize);

                        AddStr(&jsonStr, (char*)",\"", &jsonStrSize);
                        AddStr(&jsonStr, machine.alarmList[i].DateTime, &jsonStrSize);
                        AddStr(&jsonStr, (char*)"\"", &jsonStrSize);

                        snprintf(str, sizeof (str), ",\"%d\"", machine.alarmList[i].actuatorId);
                        AddStr(&jsonStr, str, &jsonStrSize);

                        AddStr(&jsonStr, (char*)"]", &jsonStrSize);
                    }
                    AddStr(&jsonStr, (char*)"]}}", &jsonStrSize);
                }
            } else {
                goto handle_no_alarms;
            }
        } else {
            handle_no_alarms:
            CpyStr(&jsonStr, (char*)"{\"alarmList\":{\"tot\":0,\"count\":0,\"rebuild\":", &jsonStrSize);
            snprintf(str, sizeof (str), "%d", rebuild);
            AddStr(&jsonStr, str, &jsonStrSize);
            AddStr(&jsonStr, (char*)"}}", &jsonStrSize);
        }
    }
    
    // curalarm;

    return (char*)jsonStr;
}
