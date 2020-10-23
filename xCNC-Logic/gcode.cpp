#define EXTERN


/////////////////////////
// RT Kernel includes
//
#ifdef WATCOM
#include "FreeRTOS.h"
#include "task.h"
#else
#include <string>

#include "./../RTLinux/RTLinux.h"
#endif







bool gcode_init_display_rows ( uint32_t nRows ) {
    uint32_t numDisplayRowsAlloc = machine.App.GCode.numDisplayRowsAlloc;

    if (check_general_structure_allocated(0, (void**) &machine.App.GCode.displayRows, sizeof (machine.App.GCode.Rows[0]), machine.App.GCode.numDisplayRows+1, &machine.App.GCode.numDisplayRowsAlloc, 1, (char*) "Adding gcode display row", (HWND) NULL) < 0) {
        return false;        
    }    
    
    machine.App.GCode.numDisplayRowsAlloc = numDisplayRowsAlloc;    
    if (check_general_structure_allocated(0, (void**) &machine.App.GCode.displayRowsOptions, sizeof (machine.App.GCode.Rows[0]), machine.App.GCode.numDisplayRows+1, &machine.App.GCode.numDisplayRowsAlloc, 1, (char*) "Adding gcode display row", (HWND) NULL) < 0) {
        return false;        
    }    
    
    return true;
}



// aggiorna le righe dal testo
bool gcode_update_content ( char *GCodeContent ) {
    if (GCodeContent) {
        uint32_t GCodeContentSize = strlen(GCodeContent);
        
        if (GCodeContent != machine.App.GCode.Content)
            machine.App.GCode.Content = (char *)realloc(machine.App.GCode.Content, GCodeContentSize+32);
        
        if (machine.App.GCode.Content) {
            char last_char = 0, *last_pstr = machine.App.GCode.Content, *pstr = machine.App.GCode.Content;
            uint32_t sizeofSep = 2;
            
            if (GCodeContent != machine.App.GCode.Content)
                strcpy(machine.App.GCode.Content, GCodeContent);
            
            gcode_reset_content();
                
            
            while (pstr) { 
                sizeofSep = 2;
                pstr = strstr(pstr, "\r\n");
                if (!pstr) {
                    pstr = last_pstr;
                    pstr = strstr(pstr, "\\r\\n");
                    sizeofSep = 4;
                }
                
                if (pstr) {
                    last_char = *pstr;
                    *pstr = 0;
                    
                    if (last_pstr) {
                        if (!gcode_add_content ( last_pstr )) {
                            return -1;
                        }
                    }
                    
                    *pstr = last_char;
                    pstr += sizeofSep;
                } else {
                    if (last_pstr) {
                        if (last_pstr[0]) {
                            if (!gcode_add_content ( last_pstr )) {
                                return -1;
                            }
                        }
                    }
                }

                last_pstr = pstr;
            }
            
            // Contenuto appena aggiornato
            machine.App.GCode.ContentChanged = 0;
            
            return 1;
        }
    }    
}


bool gcode_update_start_row ( uint32_t newStartRow ) {
    machine.App.GCode.startRow = newStartRow;    
    if (machine.App.GCode.startRow >= machine.App.GCode.numRows - machine.App.GCode.numDisplayRows)
        if (machine.App.GCode.numRows > machine.App.GCode.numDisplayRows) {
            machine.App.GCode.startRow = machine.App.GCode.numRows - machine.App.GCode.numDisplayRows;
        } else {
            machine.App.GCode.startRow = 0;
        }
    
    for (uint32_t i=0; i<machine.App.GCode.numDisplayRows; i++) {
        if (i+machine.App.GCode.startRow < machine.App.GCode.numRows) {
            machine.App.GCode.displayRows[i] = (char*)machine.App.GCode.Rows[i+machine.App.GCode.startRow];
        } else {
            machine.App.GCode.displayRows[i] = NULL;
        }        
        machine.App.GCode.displayRowsOptions[i] = &machine.App.GCode.RowsOptions[i+machine.App.GCode.startRow];
    }
    return true;
}


#define MAX_GCODE_VIEW_ROWS 1024

bool gcode_update_no_view_row ( uint32_t newNoViewRow ) {
    if (newNoViewRow < MAX_GCODE_VIEW_ROWS) {
        if (machine.App.GCode.numDisplayRows != newNoViewRow) {
            machine.App.GCode.numDisplayRows = newNoViewRow;
            if (!gcode_init_display_rows ( machine.App.GCode.numDisplayRows )) {
                return false;
            }
            gcode_update_start_row ( machine.App.GCode.startRow );
        }
    }
    return true;
}


bool gcode_reset_content ( void ) {
    for (uint32_t i=0; i<machine.App.GCode.numRowsAlloc; i++) {
        FREE_POINTER(machine.App.GCode.Rows[i]);
    }
    machine.App.GCode.startRow = 0;
    machine.App.GCode.numRows = 0;
    return true;
}

bool gcode_srep_content () {
    bool retVal = true;
    
    if (machine.App.GCode.SearchStr && machine.App.GCode.ReplaceStr) {
        for (uint32_t i=0; i<machine.App.GCode.numRows; i++) {
            std::string c = std::string(machine.App.GCode.Rows[i]);
            c.replace( c.begin(), c.end(), machine.App.GCode.SearchStr, machine.App.GCode.ReplaceStr );
            if (c.compare(machine.App.GCode.Rows[i]) == 0) {
                if (!gcode_update_row ( (char*) c.c_str(), i )) {
                    retVal = false;
                }
            }
        }
    
        gcode_rebuld_content_from_rows();
    }
    
    FREE_POINTER(machine.App.GCode.SearchStr);
    FREE_POINTER(machine.App.GCode.ReplaceStr);
    
    return retVal;
}



bool gcode_add_content ( char *GCodeRowContent ) {
    uint32_t numRowsAlloc = machine.App.GCode.numRowsAlloc;
    if (check_general_structure_allocated(0, (void**) &machine.App.GCode.Rows, sizeof (machine.App.GCode.Rows[0]), machine.App.GCode.numRows+1, &machine.App.GCode.numRowsAlloc, 512, (char*) "Adding gcode row", (HWND) NULL) < 0) {
        return false;        
    }    
    machine.App.GCode.numRowsAlloc = numRowsAlloc;
    if (check_general_structure_allocated(0, (void**) &machine.App.GCode.RowsOptions, sizeof (machine.App.GCode.RowsOptions[0]), machine.App.GCode.numRows+1, &machine.App.GCode.numRowsAlloc, 512, (char*) "Adding gcode row", (HWND) NULL) < 0) {
        return false;
    }    
    
    COPY_POINTER(machine.App.GCode.Rows[machine.App.GCode.numRows], GCodeRowContent);
    machine.App.GCode.numRows++;            

    // Contenuto modificato
    machine.App.GCode.ContentChanged++;

    return true;
}


bool gcode_insert_content ( char *GCodeRowContent, uint32_t rowPos ) {

    if (general_structure_operation( 0, (void **) &machine.App.GCode.Rows,
            sizeof(machine.App.GCode.Rows[0]), &machine.App.GCode.numRows, &machine.App.GCode.numRowsAlloc,
            7, (char*)"Insert gcode",
            (void *)NULL, 1, rowPos,
            GENERAL_STRUCTURE_INSERT, NULL) < 0) {        
    }
        
    COPY_POINTER(machine.App.GCode.Rows[rowPos], GCodeRowContent);
    machine.App.GCode.RowsOptions[rowPos] = 0;

    // Contenuto modificato
    machine.App.GCode.ContentChanged++;

    return true;
}


bool gcode_delete_content ( uint32_t rowPos ) {

    if (general_structure_operation( 0, (void **) &machine.App.GCode.Rows,
            sizeof(machine.App.GCode.Rows[0]), &machine.App.GCode.numRows, &machine.App.GCode.numRowsAlloc,
            7, (char*)"Delete gcode",
            (void *)NULL, -1, rowPos,
            GENERAL_STRUCTURE_SHIFT, NULL) < 0) {        
    }

    // Contenuto modificato
    machine.App.GCode.ContentChanged++;

    return true;
}



bool gcode_update_row ( char *GCodeRowContent, uint32_t rowPos ) {

    if (rowPos >= machine.App.GCode.numRowsAlloc) {
        uint32_t numRowsAlloc = machine.App.GCode.numRowsAlloc;
        if (check_general_structure_allocated(0, (void**) &machine.App.GCode.Rows, sizeof (machine.App.GCode.Rows[0]), rowPos+1, &machine.App.GCode.numRowsAlloc, 512, (char*) "Adding gcode row", (HWND) NULL) < 0) {
            return false;
        }    
        machine.App.GCode.numRowsAlloc = numRowsAlloc;
        if (check_general_structure_allocated(0, (void**) &machine.App.GCode.RowsOptions, sizeof (machine.App.GCode.RowsOptions[0]), rowPos+1, &machine.App.GCode.numRowsAlloc, 512, (char*) "Adding gcode row", (HWND) NULL) < 0) {
            return false;
        }    
    }
    
    
    
    if (rowPos < machine.App.GCode.numRows) {
        COPY_POINTER(machine.App.GCode.Rows[rowPos], GCodeRowContent);        
        
    } else if (rowPos >= machine.App.GCode.numRows) {
        for (uint32_t i=machine.App.GCode.numRows; i<rowPos+1; i++) {
            FREE_POINTER(machine.App.GCode.Rows[i]);
            machine.App.GCode.RowsOptions[i] = 0;
        }

        COPY_POINTER(machine.App.GCode.Rows[rowPos], GCodeRowContent);
        machine.App.GCode.RowsOptions[rowPos] = 0;
        machine.App.GCode.numRows = rowPos+1;
        
        // Contenuto modificato
        machine.App.GCode.ContentChanged++;

        return true;
    }
            
    return false;
}


int32_t gcode_rebuld_content_from_rows ( void ) {
    char *Content = NULL;
    uint32_t  ContentAlloc = 0;
    
    for (uint32_t i=0; i<machine.App.GCode.numRows; i++) {
        if (i)
            AddStr(&Content, (char*)"\r\n", &ContentAlloc);
        AddStr(&Content, machine.App.GCode.Rows[i], &ContentAlloc);
    }
    
    FREE_POINTER(machine.App.GCode.Content);
    machine.App.GCode.Content = Content;
    
    // Contenuto appena aggiornato
    machine.App.GCode.ContentChanged = 0;
    
    return 1;
}

    
int32_t process_gcode_row ( char *GCodeRowContent, int32_t *rowOptions, uint32_t *index, void *pvGcodeCmd ) {
    int32_t retVal = 0, len = 0;
    char *pstr = NULL, *last_pstr = NULL, last_char = 0;
    LP_GCODE_CMD pGcodeCmd = NULL;

    if (pvGcodeCmd) {
        pGcodeCmd = (LP_GCODE_CMD)pvGcodeCmd;
    } else {
        pGcodeCmd = (LP_GCODE_CMD)&machine.App.GCodeCmd;
    }

        
    if (GCodeRowContent) {
        len = strlen(GCodeRowContent);
        if (index) {
            if (*index < len) {
                if (GCodeRowContent[*index]) {
                    last_pstr = pstr = &GCodeRowContent[*index];

                    while (*last_pstr == ' ')
                        last_pstr++;

                    while (last_pstr) {

                        if (*last_pstr == ';') {
                            // riga commento
                            last_char = *last_pstr;
                        } else {
                            // ricerca dello spazio vuoto come delimitatore
                            pstr = strstr(last_pstr, " ");
                            if (pstr) {
                                last_char = *pstr;
                                *pstr = 0;
                            }
                        while (*last_pstr == ' ') last_pstr++;
                        }

                        switch(process_gcode_item( last_pstr, rowOptions, pvGcodeCmd)) {
                            case -1:
                                return -1;
                                break;
                            case 0:
                                // retVal = 0;
                                break;
                            case 1:
                                retVal = 1;
                                break;
                            case 999:
                                retVal = 999;
                                break;
                        }

                        if (pstr) {
                            *pstr = last_char;                    
                            pstr++;
                        }

                        last_pstr = pstr;

                        if(last_pstr)
                            *index = (uint32_t)last_pstr - (uint32_t)GCodeRowContent;
                        else 
                            *index = len;

                        if (*index >= len) {
                            return retVal;
                        }
                        if (retVal == 999) {
                            return 999;
                        }
                    }
                }
            }
        }
    }
    
    return retVal;
}




int32_t process_gcode_item ( char *GCodeItem, int32_t *rowOptions, void *pvGcodeCmd ) {
    int32_t retVal = 0;
    LP_GCODE_CMD pGcodeCmd = NULL;

    if (pvGcodeCmd) {
        pGcodeCmd = (LP_GCODE_CMD)pvGcodeCmd;
    } else {
        pGcodeCmd = (LP_GCODE_CMD)&machine.App.GCodeCmd;
    }

    
    if (GCodeItem) {

        if (GCodeItem[0]) {

            ///////////////////
            // Heidenhain
            //
            if (strnicmp(GCodeItem, "BEGIN PGM", 9) == 0) {
            } else if (strnicmp(GCodeItem, "PGM", 3) == 0) {
            } else if (strnicmp(GCodeItem, "TOOL CALL", 9) == 0) {
            } else if (strnicmp(GCodeItem, "CYCLE CALL", 10) == 0) {
            } else if (strnicmp(GCodeItem, "CYCLE CALL", 10) == 0) {

            } else {
                ////////////////
                // GCODE
                //
                    
                switch (GCodeItem[0]) {

                ////////////////
                // Comando G
                //
                case 'G': {
                    switch (atoi(&GCodeItem[1])) {
                        case 0:
                            // Rapid
                            // targetX, targetY, targetZ, targetW, targetT;
                            pGcodeCmd->moveType = 0;  // 0 = interpolazione lineare rapido
                            pGcodeCmd->nextSequence = START_RAPID_MOVE;
                            retVal = 1;
                            break;
                        case 1:
                            // Mill
                            // targetX, targetY, targetZ, targetW, targetT;
                            pGcodeCmd->moveType = 1;  // 1 = interpolazione lineare                             
                            pGcodeCmd->nextSequence = START_MILL_MOVE;
                            retVal = 1;
                            break;
                            
                        case 2:
                            // Mill
                            // targetX, targetY, targetZ, targetW, targetT;
                            // centerX, centerY, centerZ, radius, startAng, endAng;
                            pGcodeCmd->moveType = 2;  // 2 = interpolazione circolare CW (orario)
                            pGcodeCmd->nextSequence = START_MILL_MOVE;
                            retVal = 1;
                            break;
                            
                        case 3:
                            // Mill
                            // targetX, targetY, targetZ, targetW, targetT;
                            // centerX, centerY, centerZ, radius, startAng, endAng;
                            pGcodeCmd->moveType = 3;  // 3 = interpolazione circolare CCW (anti-orario)
                            pGcodeCmd->nextSequence = START_MILL_MOVE;
                            retVal = 1;
                            break;
                            
                            
                        case 4:
                            // Dwell : wait time
                            pGcodeCmd->WaitTimeMS = (uint32_t)(pGcodeCmd->P * 1000.0f);
                            pGcodeCmd->moveType = 0;
                            pGcodeCmd->nextSequence = SET_WAIT_SEQUENCE;
                            retVal = 1;
                            break;

                            
                        case 17:
                            // XY-plane (G17)
                            pGcodeCmd->curPlane = 17;
                            pGcodeCmd->nextSequence = SET_AXIS_MAP;
                            retVal = 1;
                            break;
        
                        case 18:
                            // XY-plane (G18)
                            pGcodeCmd->curPlane = 18;
                            pGcodeCmd->nextSequence = SET_AXIS_MAP;
                            retVal = 1;
                            break;

                        case 19:
                            // YZ-plane (G19)
                            pGcodeCmd->curPlane = 19;
                            pGcodeCmd->nextSequence = SET_AXIS_MAP;
                            retVal = 1;
                            break;

                        case 81:                           
                        case 82:
                        case 83:
                            pGcodeCmd->moveType = atoi(&GCodeItem[1]);
                            pGcodeCmd->nextSequence = START_DRILL_CYCLE;
                            retVal = 1;
                            break;

                        case 85:
                        case 86:
                        case 88:
                        case 89:
                            pGcodeCmd->moveType = atoi(&GCodeItem[1]);
                            pGcodeCmd->nextSequence = START_BORE_CYCLE;
                            retVal = 1;
                            break;

                        case 90:
                            if (strncmp(&GCodeItem[1], "90.1", 4) == 0) {
                                // G90.1, Arc Distance Mode
                                pGcodeCmd->abs_arc_coord = true;
                            } else if (strncmp(&GCodeItem[1], "90", 2) == 0) {
                                // G90 (set absolute distance mode)
                                pGcodeCmd->abs_coord = true;
                            } else {
                                retVal = -1;
                            }
                            break;

                        case 91:
                            if (strncmp(&GCodeItem[1], "91.1", 4) == 0) {
                                // G91.1 Arc Distance Mode
                                pGcodeCmd->abs_arc_coord = false;
                            } else if (strncmp(&GCodeItem[1], "91", 2) == 0) {
                                // G91 (set incremental distance mode)
                                pGcodeCmd->abs_coord = false;
                            } else {
                                retVal = -1;
                            }
                            break;
                                   
                        default:
                            retVal = 0;
                            break;
                        }
                    break;
                }

                //////////////////////
                // Comando Mandrino
                //
                case 'M': {
                    switch (atoi(&GCodeItem[1])) {
                        case 0:
                            // M0	Stop program run (Spindle STOP, Coolant OFF)
                            BIT_ON(rowOptions[0], BIT25); // M spindle code setted    
                            BIT_ON(rowOptions[0], BIT26); // M cooler code setted    
                            pGcodeCmd->spindleCMD = 0;
                            pGcodeCmd->CoolerCMD = 0;
                            retVal = 999;
                            break;
                        case 1:
                            // M1	Optional program STOP (Spindle STOP, Coolant OFF)
                            BIT_ON(rowOptions[0], BIT25); // M spindle code setted    
                            BIT_ON(rowOptions[0], BIT26); // M cooler code setted    
                            pGcodeCmd->spindleCMD = 0;
                            pGcodeCmd->CoolerCMD = 0;
                            retVal = 999;
                            break;
                        case 2:
                        case 30:
                            // M2	Stop program run (Spindle STOP,Coolant OFF,Go to block 1,Clear the status display(depending on machine parameter))
                            // M30	Same as M2
                            BIT_ON(rowOptions[0], BIT25); // M spindle code setted   
                            BIT_ON(rowOptions[0], BIT26); // M cooler code setted    
                            pGcodeCmd->spindleCMD = 0;
                            pGcodeCmd->CoolerCMD = 0;
                            pGcodeCmd->gotoRow1B = -1;
                            retVal = 999;
                            break;
                        case 3:
                            // M3	Spindle ON clockwise
                            BIT_ON(rowOptions[0], BIT25); // M spindle code setted                                
                            pGcodeCmd->spindleCMD = 3;
                            pGcodeCmd->CoolerCMD = 0;
                            pGcodeCmd->gotoRow1B = 1;
                            retVal = 999;
                            break;
                        case 4:
                            // M4	Spindle ON counterclockwise
                            BIT_ON(rowOptions[0], BIT25); // M code setted    
                            pGcodeCmd->spindleCMD = 4;
                            retVal = 999;
                            break;
                        case 5:
                            // M5	Spindle STOP
                            BIT_ON(rowOptions[0], BIT25); // M code setted    
                            pGcodeCmd->spindleCMD = 0;
                            retVal = 999;
                            break;
                        case 6:
                            // M6	Tool change (STOP program run (depending on machine parameter),Spindle STOP)
                            BIT_ON(rowOptions[0], BIT25); // M code setted
                            BIT_ON(rowOptions[0], BIT26); // M cooler code setted    
                            pGcodeCmd->spindleCMD = 0;
                            pGcodeCmd->CoolerCMD = 0;
                            retVal = 999;
                            break;
                        case 8:
                            // M8	Coolant ON
                            BIT_ON(rowOptions[0], BIT26); // M cooler code setted
                            pGcodeCmd->CoolerCMD = 1;
                            retVal = 999;
                            break;
                        case 9:
                            // M9	Coolant OFF
                            BIT_ON(rowOptions[0], BIT26); // M cooler code setted    
                            pGcodeCmd->CoolerCMD = 0;
                            retVal = 999;
                            break;
                            
                        case 13:
                            // M13	Spindle ON clockwise (Coolant ON)
                            BIT_ON(rowOptions[0], BIT25); // M spingle code setted    
                            BIT_ON(rowOptions[0], BIT26); // M cooler code setted    
                            pGcodeCmd->spindleCMD = 3;
                            pGcodeCmd->CoolerCMD = 1;
                            retVal = 999;
                            break;
                        case 14:
                            // M14	Spindle ON counterclockwise (Coolant ON)
                            BIT_ON(rowOptions[0], BIT25); // M spingle code setted    
                            BIT_ON(rowOptions[0], BIT26); // M cooler code setted    
                            pGcodeCmd->spindleCMD = 4;
                            pGcodeCmd->CoolerCMD = 1;
                            retVal = 999;
                            break;
                        default:
                            retVal = 0;
                            break;
                        }
                    break;
                }

                //////////////////////
                // Asse X...
                //
                case 'X': {
                    uint32_t XAxisIndex = 0;                            
                    if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.XMap, 'X', &XAxisIndex, NULL) < 0) {
                    } else {
                        if (pGcodeCmd->abs_coord) {
                            pGcodeCmd->targetX = atof(&GCodeItem[1]);
                        } else {
                            pGcodeCmd->targetX = atof(&GCodeItem[1]) + machine.actuator[XAxisIndex].cur_rpos;
                        }
                        BIT_ON(rowOptions[0], BIT5);
                    }
                    break;
                }
                case 'Y': {
                    uint32_t YAxisIndex = 0;                            
                    if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.YMap, 'Y', &YAxisIndex, NULL) < 0) {
                    } else {
                        if (pGcodeCmd->abs_coord) {
                            pGcodeCmd->targetY = atof(&GCodeItem[1]);
                        } else {
                            pGcodeCmd->targetY = atof(&GCodeItem[1]) + machine.actuator[YAxisIndex].cur_rpos;
                        }
                        BIT_ON(rowOptions[0], BIT6);
                    }
                    break;
                }
                case 'Z': {
                    uint32_t ZAxisIndex = 0;                            
                    if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                    } else {
                        if (pGcodeCmd->abs_coord) {
                            pGcodeCmd->targetZ = atof(&GCodeItem[1]);
                        } else {
                            pGcodeCmd->targetZ = atof(&GCodeItem[1]) + machine.actuator[ZAxisIndex].cur_rpos;
                        }
                        BIT_ON(rowOptions[0], BIT7);
                    }
                    break;
                }
                case 'W': {
                    uint32_t WAxisIndex = 0;                            
                    if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.WMap, 'W', &WAxisIndex, NULL) < 0) {
                    } else {
                        if (pGcodeCmd->abs_coord) {
                            pGcodeCmd->targetW = atof(&GCodeItem[1]);
                        } else {
                            pGcodeCmd->targetW = atof(&GCodeItem[1]) + machine.actuator[WAxisIndex].cur_rpos;
                        }
                        BIT_ON(rowOptions[0], BIT8);
                    }
                    break;
                }
                case 'T': {
                    uint32_t TAxisIndex = 0;                            
                    if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.TMap, 'T', &TAxisIndex, NULL) < 0) {
                    } else {
                        if (pGcodeCmd->abs_coord) {
                            pGcodeCmd->targetT = atof(&GCodeItem[1]);
                        } else {
                            pGcodeCmd->targetT = atof(&GCodeItem[1]) + machine.actuator[TAxisIndex].cur_rpos;
                        }
                        BIT_ON(rowOptions[0], BIT9);
                    }
                    break;
                }
                    
                
                case 'P':
                    pGcodeCmd->P = atof(&GCodeItem[1]);
                        // BIT10 ->  P setted
                    if(rowOptions) {
                        BIT_ON(rowOptions[0], BIT10);
                    }
                    break;
                case 'R':
                    pGcodeCmd->R = atof(&GCodeItem[1]);
                        // BIT11 ->  R setted
                    if(rowOptions) {
                        BIT_ON(rowOptions[0], BIT11);
                    }
                    break;
                    
                case 'I': {
                    uint32_t XAxisIndex = 0;                            
                    if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.XMap, 'X', &XAxisIndex, NULL) < 0) {
                    } else {
                        pGcodeCmd->I = atof(&GCodeItem[1]);
                            // BIT12 ->  I setted
                        if(rowOptions) {
                            BIT_ON(rowOptions[0], BIT12);
                        }
                    }
                    break;
                    }
                case 'J': {
                    uint32_t YAxisIndex = 0;                            
                    if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.YMap, 'Y', &YAxisIndex, NULL) < 0) {
                    } else {
                        pGcodeCmd->J = atof(&GCodeItem[1]);
                            // BIT13 ->  J setted
                        if(rowOptions) {
                            BIT_ON(rowOptions[0], BIT13);
                        }
                    }
                    break;
                    }
                case 'K': {
                    uint32_t ZAxisIndex = 0;                            
                    if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                    } else {
                        pGcodeCmd->K = atof(&GCodeItem[1]);
                            // BIT14 ->  K setted
                        if(rowOptions) {
                            BIT_ON(rowOptions[0], BIT14);
                        }
                    }
                    break;
                    }
                    
                case 'Q':
                    pGcodeCmd->Q = atof(&GCodeItem[1]);
                        // BIT15 ->  Q setted
                    if(rowOptions) {
                        BIT_ON(rowOptions[0], BIT15);
                    }
                    break;

        
                //////////////////////
                // Feeed...
                //
                case 'F': {
                    float feed = atof(&GCodeItem[1]);
                    pGcodeCmd->speed_mm_min = feed / 1.0f;
                    if(rowOptions) {
                        BIT_ON(rowOptions[0], BIT21); // BIT21 ->  F feed setted
                    }
                    break;
                }


                //////////////////////
                // Speed...
                //
                case 'S': {
                    float speed = atof(&GCodeItem[1]);
                    pGcodeCmd->spindle_speed_rpm = speed / 1.0f;
                    if(rowOptions) {
                        BIT_ON(rowOptions[0], BIT20); // BIT20 ->  S speed setted                    
                    }
                    break;
                }
                    
                /////////////////////////////////////////////
                // Commento o Commento con parole chiave
                //
                case ';': {

                    GCodeItem++;
                    while (*GCodeItem == ' ' || *GCodeItem == ';') GCodeItem++;

                    //////////////////////////////////////////
                    // Protocollo xProject
                    //
                    if (strnicmp(&GCodeItem[0], "xp://", 5) == 0) {
                        switch (process_gcode_xproject ( &GCodeItem[6], rowOptions, pvGcodeCmd )) {
                            case -1:
                                break;
                            case 0:
                                break;
                            case 1:
                                break;
                            case 999:
                                break;
                        }
                        
                        BIT_ON(rowOptions[0], BIT31);
                            // BIT31 ->  Riga commento con chiave XP



                        ////////////////////////////////////////////                        
                        // Commento per CutViewer_Mill ?
                        //
                    } else if (strnicmp(&GCodeItem[0], "tool/drill", 10) == 0 || strnicmp(&GCodeItem[0], "tool/mill", 9) == 0 ||
                                strnicmp(&GCodeItem[0], "color", 6) == 0 || strnicmp(&GCodeItem[0], "(block/stock", 12) == 0
                            ) {
                        
                        switch (process_gcode_comment ( &GCodeItem[0], rowOptions, pvGcodeCmd )) {
                            case -1:
                                break;
                            case 0:
                                break;
                            case 1:
                                break;
                            case 999:
                                break;
                        }
                        
                    } else {
                        BIT_ON(rowOptions[0], BIT4);
                            // BIT4 ->  Riga commento
                    }
                    break;
                }
                }
            }
        }
    }
    
    return retVal;
}



int32_t process_gcode_xproject ( char *GCodeItem, int32_t *rowOptions, void *pvGcodeCmd ) {    
    int32_t retVal;
    LP_GCODE_CMD pGcodeCmd = NULL;

    if (pvGcodeCmd) {
        pGcodeCmd = (LP_GCODE_CMD)pvGcodeCmd;
    } else {
        pGcodeCmd = (LP_GCODE_CMD)&machine.App.GCodeCmd;
    }
    
    if (GCodeItem) {
        if (GCodeItem[0]) {
            // MATERIAL=%MATERIAL%,MATERIAL_CUT_SPEED=%MATERIAL_CUT_SPEED%,MATERIAL_SIGMA=%MATERIAL_SIGMA%\r\n"
            // DEPTH_Z=%FEATURE_STEP_Z% CUT_SPEED=%STOCK_TOOL_CUT_SPEED%
            // RAD=%STOCK_TOOL_RADIUS%,NO_CUTTING_EDGE=%STOCK_TOOL_NO_CUTTING_EDGE%Z,POS=%STOCK_TOOL_POS%
            MDB_CHAR **itemsArray = NULL;
            MDB_CHAR *SeparatorChar = (MDB_CHAR *)",";
            uint32_t NumSeparatorChar = 1;
            uint32_t NumItemsArray = 0, NumItemsArrayAllocated = 0;
            HWND ptr_hwnd = NULL;
            int32_t i;
                    
            if (create_array_from_string ( GCodeItem, &itemsArray, &NumItemsArray, &NumItemsArrayAllocated, SeparatorChar, NumSeparatorChar, ptr_hwnd, 0+0 ) < 0) {
                
            } else {
                for (i=0; i<NumItemsArray; i++) {
                    char *pstr = strstr(itemsArray[i], "=");
                    if (pstr) {
                        pstr[0] = 0;
                        pstr++;
                        if (strcmpi((char*)itemsArray[i], "MATERIAL") == 0) {
                            COPY_POINTER (machine.App.GCodeSetup.Material, pstr);
                        } else if (strcmpi((char*)itemsArray[i], "MATERIAL_CUT_SPEED") == 0) {
                            machine.App.GCodeSetup.MaterialCurSpeed = atof(pstr);
                        } else if (strcmpi((char*)itemsArray[i], "MATERIAL_SIGMA") == 0) {
                            machine.App.GCodeSetup.MaterialSigma = atof(pstr);
                        } else if (strcmpi((char*)itemsArray[i], "DEPTH_Z") == 0) {
                            // Profondita di passata
                            pGcodeCmd->DepthStep = atof(pstr);
                        } else if (strcmpi((char*)itemsArray[i], "CUT_SPEED") == 0) {
                            // Velocità taglio utensile
                            pGcodeCmd->ToolCutSpeed = atof(pstr);
                        } else if (strcmpi((char*)itemsArray[i], "RAD") == 0) {
                            // %STOCK_TOOL_RADIUS%
                            pGcodeCmd->ToolRad = atof(pstr);
                        } else if (strcmpi((char*)itemsArray[i], "NO_CUTTING_EDGE") == 0) {
                            // %STOCK_TOOL_NO_CUTTING_EDGE%Z
                            pGcodeCmd->ToolNumCuttingEdge = atoi(pstr);
                        } else if (strcmpi((char*)itemsArray[i], "POS") == 0) {
                             // %STOCK_TOOL_POS%
                            pGcodeCmd->ToolPos = atoi(pstr);
                        } else {
                        }
                    }
                }
            }
            
            free_fields_array(&itemsArray, NumItemsArrayAllocated);
        }
    }
    
    return retVal;
}




int32_t process_gcode_comment ( char *GCodeItem, int32_t *rowOptions, void *pvGcodeCmd ) {    
    int32_t retVal;
    LP_GCODE_CMD pGcodeCmd = NULL;

    if (pvGcodeCmd) {
        pGcodeCmd = (LP_GCODE_CMD)pvGcodeCmd;
    } else {
        pGcodeCmd = (LP_GCODE_CMD)&machine.App.GCodeCmd;
    }

    
    if (GCodeItem) {
        if (GCodeItem[0]) {
            // TOOL/DRILL,diam,angle,height
            // TOOL/MILL,diam,angle,height
            // COLOR,r,g,b
            // (BLOCK/STOCK,wx,wy,wz,ox,oy,oz)
            MDB_CHAR **itemsArray = NULL;
            MDB_CHAR *SeparatorChar = (MDB_CHAR *)",";
            uint32_t NumSeparatorChar = 1;
            uint32_t NumItemsArray = 0, NumItemsArrayAllocated = 0;
            HWND ptr_hwnd = NULL;
            int32_t i;
                    
            if (create_array_from_string ( GCodeItem, &itemsArray, &NumItemsArray, &NumItemsArrayAllocated, SeparatorChar, NumSeparatorChar, ptr_hwnd, 0+0 ) < 0) {
                
            } else {
                for (i=0; i<NumItemsArray; i++) {
                    char *pstr = strstr(itemsArray[i], "=");
                    if (pstr) {
                        pstr[0] = 0;
                        pstr++;
                        if (strcmpi((char*)itemsArray[i], "TOOL/DRILL") == 0) {
                            // foratura : taglio frontale
                            COPY_POINTER (machine.App.GCodeSetup.Material, pstr);
                        } else if (strcmpi((char*)itemsArray[i], "TOOL/MILL") == 0) {
                            // fresatura : taglio laterale
                            machine.App.GCodeSetup.MaterialCurSpeed = atof(pstr);
                        } else if (strcmpi((char*)itemsArray[i], "COLOR") == 0) {
                            // Colore
                            machine.App.GCodeSetup.MaterialSigma = atof(pstr);
                        } else if (strcmpi((char*)itemsArray[i], "(BLOCK/STOCK") == 0 || strcmpi((char*)itemsArray[i], "(LOCK/STOCK") == 0) {
                            // Grezzo
                        } else {
                        }
                    }
                }
            }
            
            free_fields_array(&itemsArray, NumItemsArrayAllocated);
        }
    }
    
    return retVal;
}


float get_timeout_by_feed ( float deltaXYZ, float feedMMMin) {
    float dTimeMins = deltaXYZ / (feedMMMin > 0.0f ? feedMMMin : 1.0f);
    return dTimeMins * 60.0f * 1.10f * 1000.0f + 2500.0f;
}

int32_t gcode_act_free_linear_move(void *pvCANSlot, void *pvActuatorX, void *pvActuatorY, void *pvActuatorZ, float targetX, float targetY, float targetZ, float feedMMMin, float precisionMM ) {
    return gcode_act_linear_move(pvCANSlot, pvActuatorX, pvActuatorY, pvActuatorZ, targetX, targetY, targetZ, feedMMMin, precisionMM, true );
}

int32_t gcode_act_interpolated_linear_move(void *pvCANSlot, void *pvActuatorX, void *pvActuatorY, void *pvActuatorZ, float targetX, float targetY, float targetZ, float feedMMMin, float precisionMM ) {
    return gcode_act_linear_move(pvCANSlot, pvActuatorX, pvActuatorY, pvActuatorZ, targetX, targetY, targetZ, feedMMMin, precisionMM, false );
}

int32_t gcode_act_linear_move(void *pvCANSlot, void *pvActuatorX, void *pvActuatorY, void *pvActuatorZ, float targetX, float targetY, float targetZ, float feedMMMin, float precisionMM, int32_t FreeMode ) {
    int retVal = -1;
    CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
    LP_ACTUATOR pActuatorX = (LP_ACTUATOR) pvActuatorX;
    LP_ACTUATOR pActuatorY = (LP_ACTUATOR) pvActuatorY;
    LP_ACTUATOR pActuatorZ = (LP_ACTUATOR) pvActuatorZ;
    float deltaX = 0.0f, deltaY = 0.0f, deltaZ = 0.0f, deltaXY = 0.0f, deltaXZ = 0.0f, deltaXYZ = 0.0f, angleXY = 0.0f, angleXZ = 0.0f;
    float speed_rpm_X = 0.0f, speed_rpm_Y = 0.0f, speed_rpm_Z = 0.0f;
    float speed_base_rpm;
    float feedXMMMin = 0.0f, feedYMMMin = 0.0f, feedZMMMin = 0.0f;
    bool busNeeded = false;

        
    deltaX = pActuatorX ? targetX - pActuatorX->cur_rpos : 0.0f;
    deltaY = pActuatorY ? targetY - pActuatorY->cur_rpos : 0.0f;
    deltaZ = pActuatorZ ? targetZ - pActuatorZ->cur_rpos : 0.0f;

    deltaXYZ = sqrt (deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);
    deltaXY = sqrt (deltaX*deltaX + deltaY*deltaY);
    deltaXZ = sqrt (deltaX*deltaX + deltaZ*deltaZ);
    
    if (deltaXYZ >= 0.0f && deltaXYZ < machine.App.Epsilon) {
        deltaXYZ = 0.0f;
    }
    if (deltaXY >= 0.0f && deltaXY < machine.App.Epsilon) {
        deltaXY = 0.0f;
    }
    if (deltaXZ >= 0.0f && deltaXZ < machine.App.Epsilon) {
        deltaXY = 0.0f;
    }
    
    angleXY = atan2f(deltaY, deltaX ); // * 180.0 / DPIGRECO;
    angleXZ = atan2f(deltaZ, deltaX ); // * 180.0 / DPIGRECO;

    float cX = (float)cosf(angleXY);
    float cY = (float)sinf(angleXY);
    float cZ = (float)sinf(angleXZ);


    // Calcolo del numero di passi basato sulla velocità
    int32_t nSteps = 0, nStepsX = 0, nStepsY = 0, nStepsZ = 0;
    float dTimeMins = deltaXYZ / feedMMMin;
    float PeriodSec = (float)machine.settings.InterpolationPeriodMS / 1000.0f;

    // nSteps = (int32_t)(deltaXYZ / (pCANSlot->PrecisionMM > 0.0f ? pCANSlot->PrecisionMM : machine.settings.Precision)) + 1;
    nSteps = dTimeMins * 60.0f / PeriodSec;

    if (nSteps <= 0)
        nSteps = 1;
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Ricalcolo numero passi per contenere il passo delle pusazioni 16bit signed nel PDO del canopen
    //
    float deltaPulseX = pActuatorX ? ((float)deltaX / pActuatorX->cam_ratio * (float)pActuatorX->pulsesPerTurn) : 0.0f;
    float deltaPulse = deltaPulseX / (float)nSteps;
    if (deltaPulse > 32767) {
        deltaPulse = 32000.0;
        nStepsX = deltaPulseX / deltaPulse;
    } else if (deltaPulse < -32767) {
        deltaPulse = -32000.0;
        nStepsX = deltaPulseX / deltaPulse;
    }
    
    float deltaPulseY = pActuatorY ? ((float)deltaY / pActuatorY->cam_ratio * (float)pActuatorY->pulsesPerTurn) : 0.0f;
    deltaPulse = deltaPulseY / (float)nSteps;
    if (deltaPulse > 32767) {
        deltaPulse = 32000;
        nStepsY = deltaPulseY / deltaPulse;
    } else if (deltaPulse < -32767) {
        deltaPulse = -32000;
        nStepsY = deltaPulseY / deltaPulse;
    }

    float deltaPulseZ = pActuatorZ ? ((float)deltaZ / pActuatorZ->cam_ratio * (float)pActuatorZ->pulsesPerTurn) : 0.0f;
    deltaPulse = deltaPulseZ / (float)nSteps;
    if (deltaPulse > 32767) {
        deltaPulse = 32000;
        nStepsZ = deltaPulseZ / deltaPulse;
    } else if (deltaPulse < -32767) {
        deltaPulse = -32000;
        nStepsZ = deltaPulseZ / deltaPulse;
    }

    nSteps = MAX(nSteps, nStepsX);
    nSteps = MAX(nSteps, nStepsY);
    nSteps = MAX(nSteps, nStepsZ);

    // Durata interpolazione
    float dTimeMins2 = (float)machine.settings.InterpolationPeriodMS * (float)nSteps / 1000.0f / 60.0f;

    /////////////////////////
    // Ricalcolo avanzamenti
    //
    if (dTimeMins2 > EPSILON) { 
        feedMMMin = deltaXYZ / dTimeMins2;
        feedXMMMin = fabs(deltaX) / dTimeMins2;
        feedYMMMin = fabs(deltaY) / dTimeMins2;
        feedZMMMin = fabs(deltaZ) / dTimeMins2;
    } else {        
        return -99;
    }
    
    speed_rpm_X = pActuatorX ? (feedXMMMin / ( pActuatorX ? pActuatorX->cam_ratio : 1.0f )) : 0.0f;
    speed_rpm_Y = pActuatorY ? (feedYMMMin / ( pActuatorY ? pActuatorY->cam_ratio : 1.0f )) : 0.0f;
    speed_rpm_Z = pActuatorZ ? (feedZMMMin / ( pActuatorZ ? pActuatorZ->cam_ratio : 1.0f )) : 0.0f;

    
    retVal = 0;

    if (pActuatorX) {
        if (fabs(pActuatorX->cur_rpos - targetX) > machine.App.Epsilon) {
            if (pActuatorX->step == STEP_READY) {
                if (pActuatorX->homingDone) {
                    pActuatorX->speed_lin_auto3 = feedXMMMin / 60.0f;
                    actuator_linear_to_speed( (void *)pActuatorX, pActuatorX->speed_lin_auto3, &pActuatorX->speed_auto3);
                    pActuatorX->target_rpos = targetX;
                    pActuatorX->target_position = FreeMode ? USER_POSITION : INTERPOLATE_POSITION;
                    pActuatorX->position = INDETERMINATE;
                    pActuatorX->step = STEP_SEND_CMD;
                    pActuatorX->AxisSinCosLin = 2;
                    pActuatorX->timeout4_ms = get_timeout_by_feed (deltaXYZ, feedMMMin);
                    
                    if (pActuatorX->protocol == PROTOCOL_NONE || pActuatorX->protocol == VIRTUAL_AC_SERVO) {
                    } else {
                        busNeeded = true;
                    }
                    
                    retVal++;
                } else {
                    return -5;
                }
            } else {
                return -7;
            }
        } else {
            // gia in posizione
            pActuatorX->position = pActuatorX->target_position;
        }
    }        

    if (pActuatorY) {
        if (fabs(pActuatorY->cur_rpos - targetY) > machine.App.Epsilon) {
            if (pActuatorY->step == STEP_READY) {
                if (pActuatorY->homingDone) {
                    pActuatorY->speed_lin_auto3 = feedYMMMin / 60.0f;
                    actuator_linear_to_speed( (void *)pActuatorY, pActuatorY->speed_lin_auto3, &pActuatorY->speed_auto3);
                    pActuatorY->target_rpos = targetY;
                    pActuatorY->target_position = FreeMode ? USER_POSITION : INTERPOLATE_POSITION;
                    pActuatorY->position = INDETERMINATE;
                    pActuatorY->step = STEP_SEND_CMD;
                    pActuatorY->AxisSinCosLin = 2;
                    pActuatorY->timeout4_ms = get_timeout_by_feed (deltaXYZ, feedMMMin);

                    if (pActuatorY->protocol == PROTOCOL_NONE || pActuatorY->protocol == VIRTUAL_AC_SERVO) {
                    } else {
                        busNeeded = true;
                    }
                    
                    retVal++;
                } else {
                    return -5;
                }
            } else {
                return -8;
            }
        } else {
            // gia in posizione
            pActuatorY->position = pActuatorY->target_position;
        }
    }        

    if (pActuatorZ) {
        if (fabs(pActuatorZ->cur_rpos - targetZ) > machine.App.Epsilon) {
            if (pActuatorZ->step == STEP_READY) {
                if (pActuatorZ->homingDone) {
                    pActuatorZ->speed_lin_auto3 = feedZMMMin / 60.0f;
                    actuator_linear_to_speed( (void *)pActuatorZ, pActuatorZ->speed_lin_auto3, &pActuatorZ->speed_auto3);
                    pActuatorZ->target_rpos = targetZ;
                    pActuatorZ->target_position = FreeMode ? USER_POSITION : INTERPOLATE_POSITION;
                    pActuatorZ->position = INDETERMINATE;
                    pActuatorZ->step = STEP_SEND_CMD;
                    pActuatorZ->AxisSinCosLin = 2;
                    pActuatorZ->timeout4_ms = get_timeout_by_feed (deltaXYZ, feedMMMin);

                    if (pActuatorZ->protocol == PROTOCOL_NONE || pActuatorZ->protocol == VIRTUAL_AC_SERVO) {
                    } else {
                        busNeeded = true;
                    }

                    retVal++;
                } else {
                    return -5;
                }
            } else {
                return -9;
            }
        } else {
            // gia in posizione
            pActuatorZ->position = pActuatorZ->target_position;
        }
    }        


    if (FreeMode == 0) {
        if (pCANSlot) {
            pCANSlot->StartAngleRad = 0.0f;                     // Angolo iniziale mm
            pCANSlot->EndAngleRad = 0.0f;                       // Angolo finale mm
            pCANSlot->RadiusMM = 0.0f;                          // Raggio in mm

            pCANSlot->Direction = -1;                           // 0 = uso CW orario, 1 = CCW antiorario
            pCANSlot->FeedMMMin = feedMMMin;                    // Avanzamento in mm/sec           
            pCANSlot->PrecisionMM = precisionMM;                // Precisione in mm
            pCANSlot->PrecisionNSteps = nSteps;
            pCANSlot->PeriodMsec = machine.settings.InterpolationPeriodMS;
            retVal = 1;                        
        } else {
            if (busNeeded)
                retVal = -1;
            else
                retVal = 1;
        }
    } else {
        retVal = 1;
    }

    return retVal;
}




int32_t gcode_act_circular_move ( void *pvCANSlot, void *pvActuatorX, void *pvActuatorY, void *pvActuatorZ,
        float centerX, float centerY, 
        float RadiusMM, float startAngRad, float endAngRad, 
        float PrecisionMM,
        uint8_t moveType,
        float targetX, float targetY, float targetZ, 
        float feedXY, float feedZMMMin ) {
    
    int retVal = -1;
    
    if (pvActuatorX && pvActuatorY && pvActuatorZ) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        LP_ACTUATOR pActuatorX = (LP_ACTUATOR) pvActuatorX;
        LP_ACTUATOR pActuatorY = (LP_ACTUATOR) pvActuatorY;
        LP_ACTUATOR pActuatorZ = (LP_ACTUATOR) pvActuatorZ;
        uint8_t Direction = -1;
        float arcLen = 0.0f, dTimeMins = 0.0f;

        char msg[256];
        int32_t msg_size = sizeof(msg);
        
        
                                
        if (pActuatorX->step == STEP_READY && pActuatorY->step == STEP_READY) {

            if (RadiusMM < EPSILON) {
                return -9;
            }

        float checkTargetX = centerX + RadiusMM * cosf(endAngRad);
        float checkTargetY = centerY + RadiusMM * sinf(endAngRad);
        float checkCurX = centerX + RadiusMM * cosf(startAngRad);
        float checkCurY = centerY + RadiusMM * sinf(startAngRad);
        float errPosMM = fabs (checkTargetX - targetX);
        
        if (errPosMM > machine.App.toll) {
            snprintf(msg, msg_size, (char*) "Wrong target X position:%0.3f / %0.3f, error:%0.3f\n", targetX, checkTargetX, errPosMM);
            // vDisplayMessage(msg);
            if (generate_alarm((char*) msg, 8110, 0, (int) ALARM_WARNING, 0+1) < 0) {
            }
            return -10;
        }
        errPosMM = fabs (checkTargetY - targetY);
        if (errPosMM > machine.App.toll) {
            snprintf(msg, msg_size, (char*) "Wrong target Y position:%0.3f / %0.3f, error:%0.3f\n", targetY, checkTargetY, errPosMM);
            // vDisplayMessage(msg);
            if (generate_alarm((char*) msg, 8110, 0, (int) ALARM_WARNING, 0+1) < 0) {
            }
            return -11;
        }
        errPosMM = fabs (checkCurX - pActuatorX->cur_rpos);
        if (errPosMM > machine.App.toll) {
            snprintf(msg, msg_size, (char*) "Wrong start X position:%0.3f / %0.3f, error:%0.3f\n", checkCurX, pActuatorX->cur_rpos, errPosMM);
            // vDisplayMessage(msg);
            if (generate_alarm((char*) msg, 8110, 0, (int) ALARM_WARNING, 0+1) < 0) {
            }
            return -12;
        }
        errPosMM = fabs (checkCurY - pActuatorY->cur_rpos);
        if (errPosMM > machine.App.toll) {
            snprintf(msg, msg_size, (char*) "Wrong startY position:%0.3f / %0.3f, error:%0.3f\n", checkCurY, pActuatorY->cur_rpos, errPosMM);
            // vDisplayMessage(msg);
            if (generate_alarm((char*) msg, 8110, 0, (int) ALARM_WARNING, 0+1) < 0) {
            }
            return -13;
        }
                
            
            //////////////////////////////////////////////////
            // Impostazione Seno/coseno asse piano X/Y
            //
            
            // 0 = interpolazione lineare rapido
            // 1 = interpolazione lineare
            // 2 = interpolazione circolare CW (orario)
            // 3 = interpolazione circolare CCW (anti-orario)            
            
            switch(moveType) {
                case 0:
                case 1:
                    // Non valido
                    return -1;
                    break;
                case 2:
                case 3:
                    Direction = (moveType == 2 ? 0 : 1);
                    arcLen = cal_arc_length( startAngRad, endAngRad, RadiusMM, Direction);
                    dTimeMins = feedXY > 0.0f ? (arcLen / feedXY * 60.0f) : 0.0f;
                    pActuatorX->AxisSinCosLin = 0; // seno
                    pActuatorY->AxisSinCosLin = 1; // coseno
                    break;
            }
            

            if (feedZMMMin > 0.00f) {
                // Z setted
                pActuatorZ->AxisSinCosLin = 2;
            } else {
                pActuatorZ->AxisSinCosLin = -1;
            }
            
            // Numero di step (interpolazioni)
            float nSteps = (int32_t)(arcLen / PrecisionMM) + 1;
                    
            // Passo o corda in mm
            float pitchMM = arcLen / nSteps;
            
            // Passo in pulsazioni
            float pitchPulses = pitchMM / pActuatorX->cam_ratio * (float)pActuatorX->pulsesPerTurn;
            
            // Controllo overflow pulsazioni limite del PDO a 16 bit signed
            if (pitchPulses > 32767) {
                // Ricalcolo basato sulla corda massima (variazione pulse 16 bit signed)
                pitchPulses = 32000;
                float pitchMMmax = pitchPulses * pActuatorX->cam_ratio / (float)pActuatorX->pulsesPerTurn;
                nSteps = (int32_t)(arcLen / pitchMMmax) + 1;
                feedXY = arcLen / ( nSteps * machine.settings.InterpolationPeriodMS );
            } else if (pitchPulses < -32767) {
                // Ricalcolo basato sulla corda massima (variazione pulse 16 bit signed)
                pitchPulses = -32000;
                float pitchMMmax = pitchPulses * pActuatorX->cam_ratio / (float)pActuatorX->pulsesPerTurn;
                nSteps = (int32_t)(arcLen / pitchMMmax) + 1;
                feedXY = arcLen / ( nSteps * machine.settings.InterpolationPeriodMS );
                
            }
            


            ///////////////////////////////
            // Parametri per il CANBUS
            //                            
            if (pCANSlot) {
                // Angolo iniziale e finale
                pCANSlot->StartAngleRad = startAngRad;
                pCANSlot->EndAngleRad = endAngRad;   
                pCANSlot->RadiusMM = RadiusMM;

                
                // Direzione
                pCANSlot->Direction = Direction;    // 0 = uso CW orario, 1 = CCW antiorario

                    
                    
                pCANSlot->FeedMMMin = feedXY;           // Avanzamento in mm/sec           
                pCANSlot->PrecisionMM = PrecisionMM;    // Precisione in MM
                pCANSlot->PrecisionNSteps = (int32_t)nSteps;
                pCANSlot->PeriodMsec = machine.settings.InterpolationPeriodMS;


                //////////////////////////////////////////
                // Lancia il movimento degli assi
                //


                // Disabilita la sceda per avere la garanzia di recezione di tutti gli assi in oggetto

                pCANSlot->disabled = true;                                

                if (machine.App.GCodeSetup.simulateMode) {
                    // simulazione
                } else {
                    pActuatorX->position = INDETERMINATE;
                    pActuatorX->target_position = INTERPOLATE_POSITION;
                    pActuatorX->target_rpos = targetX;
                    pActuatorX->step = STEP_SEND_CMD;
                    pActuatorX->timeout4_ms = get_timeout_by_feed (arcLen, feedXY);
                }
                if (machine.App.GCodeSetup.simulateMode) {
                    // simulazione
                } else {
                    pActuatorY->position = INDETERMINATE;
                    pActuatorY->target_position = INTERPOLATE_POSITION;
                    pActuatorY->target_rpos = targetY;
                    pActuatorY->step = STEP_SEND_CMD;
                    pActuatorY->timeout4_ms = get_timeout_by_feed (arcLen, feedXY);
                }

                if (fabs(pActuatorZ->cur_rpos-targetZ) < EPSILON) {
                    if (machine.App.GCodeSetup.simulateMode) {
                        // simulazione
                    } else {
                        pActuatorZ->position = INDETERMINATE;
                        pActuatorZ->target_position = INTERPOLATE_POSITION;
                        pActuatorZ->target_rpos  = targetZ;
                        pActuatorZ->step = STEP_SEND_CMD;
                        pActuatorZ->timeout3_ms = get_timeout_by_feed (arcLen, feedXY);
                    }
                } else {
                }

                retVal = 1;
                
                // Riabilita la sceda
                pCANSlot->disabled = false;

            } else {
            }
            
        }
    }
    return retVal;
}




float gcode_get_mill_feed ( float default_min_feed, void *pvGcodeCmd ) {
    LP_GCODE_CMD pGcodeCmd = NULL;
    float speed_base_feed_mm_min = 0.0f;

    if (pvGcodeCmd) {
        pGcodeCmd = (LP_GCODE_CMD)pvGcodeCmd;
    } else {
        pGcodeCmd = (LP_GCODE_CMD)&machine.App.GCodeCmd;
    }    
    
    if (pGcodeCmd->speed_mm_min > 0.0f) {
        speed_base_feed_mm_min = pGcodeCmd->speed_mm_min;
    } else {
        speed_base_feed_mm_min = sqrt( 
                (machine.settings.mill_feed_mm_min_X*machine.settings.mill_feed_mm_min_X) 
                + (machine.settings.mill_feed_mm_min_Y*machine.settings.mill_feed_mm_min_Y)
                + (machine.settings.mill_feed_mm_min_Z*machine.settings.mill_feed_mm_min_Z) );
        if (speed_base_feed_mm_min <= 0.0f) {
            speed_base_feed_mm_min = default_min_feed;
        }
    }
    return speed_base_feed_mm_min;
}




float gcode_get_rapid_feed ( uint8_t Axis, void *pvGcodeCmd ) {
    float speed_base_feed_mm_min = 0.0f;
    LP_GCODE_CMD pGcodeCmd = NULL;

    if (pvGcodeCmd) {
        pGcodeCmd = (LP_GCODE_CMD)pvGcodeCmd;
    } else {
        pGcodeCmd = (LP_GCODE_CMD)&machine.App.GCodeCmd;
    }    
    
    switch(Axis) {
        case 'x':
        case 'X':
            return machine.settings.rapid_feed_X;
            break;
        case 'y':
        case 'Y':
            return machine.settings.rapid_feed_Y;
            break;
        case 'z':
        case 'Z':
            return machine.settings.rapid_feed_Z;
            break;
    }
    
    return 0.0f;
}




int32_t gcode_get_mapped_actuator ( uint8_t Axis, uint8_t defaultAxis, uint32_t *AxisIndex, void **ppActuator) {
    
    switch (Axis) {
        case 'x':
        case 'X':
            if (AxisIndex) *AxisIndex = X;
            if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[X];
            return X;
            break;
        case 'y':
        case 'Y':
            if (AxisIndex) *AxisIndex = Y;
            if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[Y];
            return Y;
            break;
        case 'z':
        case 'Z':
            if (AxisIndex) *AxisIndex = Z;
            if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[Z];
            return Z;
            break;
        case 'w':
        case 'W':
            if (AxisIndex) *AxisIndex = W;
            if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[W];
            return W;
            break;
        case 't':
        case 'T':
            if (AxisIndex) *AxisIndex = T;
            if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[T];
            return T;
            break;
            
        default:        
            switch (defaultAxis) {
                case 'x':
                case 'X':
                    if (AxisIndex) *AxisIndex = X;
                    if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[X];
                    return X;
                    break;
                case 'y':
                case 'Y':
                    if (AxisIndex) *AxisIndex = Y;
                    if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[Y];
                    return Y;
                    break;
                case 'z':
                case 'Z':
                    if (AxisIndex) *AxisIndex = Z;
                    if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[Z];
                    return Z;
                    break;
                case 'w':
                case 'W':
                    if (AxisIndex) *AxisIndex = W;
                    if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[W];
                    return W;
                    break;
                case 't':
                case 'T':
                    if (AxisIndex) *AxisIndex = T;
                    if (ppActuator) ((LPP_ACTUATOR)ppActuator)[0] = (LP_ACTUATOR) &machine.actuator[T];
                    return T;
                    break;
                default:
                    return -1;
                    break;
                break;        
            }
        
            break;    
    }
        

    return 0;
}