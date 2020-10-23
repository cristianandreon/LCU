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




            
            
            
int automatic ( uint32_t *pSequence, void *pvGcodeCmd ) {
    uint32_t XAxisIndex = 0, YAxisIndex = 0, ZAxisIndex = 0;
    LP_GCODE_CMD pGcodeCmd = NULL;
    char str[512];

    try {
        
        if (pSequence) {
        } else {
            pSequence = &machine.sequence;
        }
                
        if (pvGcodeCmd) {
            pGcodeCmd = (LP_GCODE_CMD)pvGcodeCmd;
        } else {
            pGcodeCmd = (LP_GCODE_CMD)&machine.App.GCodeCmd;
        }
        
        
        
        ////////////////////////////////////////
        // Richiesta caduta potenza potenza
        //
        if (!machine.power_on_request) {
            // Manca potenza
            strncpy(machine.status_message, (char*)"Potenza disattivata", machine.status_message_size);
            // Generazione allarme per traccia
            if (generate_alarm((char*) "Power disabled by user", 9998, 0, (int32_t) ALARM_ERROR, 0+1) < 0) {
            }            
            emergency();
            return 0;
        }
        
        ///////////////////////////////
        // Analisi presenza potenza
        //


        

        if (machine.status == READY) {

            // Passaggio in automatico
            machine.status = AUTOMATIC;



        } else if (machine.status == AUTOMATIC || machine.status == STEP_BY_STEP) {

            /////////////////////////////////////////////////////////////////////////////////
            // Richiesta passaggio in MNAULE
            // Solo in debug : l'arresto può avenire solo in emergenza o stop ciclo
            //
    
#ifdef _DEBUG_MODE
            if (App.DebugMode) {
                if (PUT_IN_AUTOMATIC()) {
                    if (machine.status == STEY_BY_STEP) {
                        machine.status = AUTOMATIC;
                    }
                }
            }
#endif




                                                                                                                                                                                                    /*
            __________________________________

                Sequenza principale
            __________________________________                                                                                                                                                                                                                                                                                                              */


            if (pSequence[0] == ALARM_RESETTED_SEQUENCE) {
                // Reset allarmi   
                pSequence[0] = WAIT_FOR_RESUME_CYCLE;
                        
            } else if (pSequence[0] == INIT_SEQUENCE) {

                pSequence[0] = WAIT_FOR_START_CYCLE;
                strncpy(machine.status_message, (char*)"AUTOMATIC mode", machine.status_message_size);

                // pGcodeCmd->curRow = 0;
                // pGcodeCmd->curRowChar = 0;
                
                for (int32_t iRow = 0; iRow<machine.App.GCode.numRows; iRow++) {
                    machine.App.GCode.RowsOptions[iRow] = 0;
                }
                
                
            } else if (pSequence[0] == WAIT_FOR_START_CYCLE) {
                // macchina in attesa start-ciclo (passo passo)
                if (IS_IN_MANUAL_REQUEST()) {
                    // Passaggio in manuale : messa in stop della macchina ???
                    PUT_STOP_CYCLE();
                }

                if (IS_START_CYCLE_REQUESTED()) {
                    pSequence[0] = RUN_SEQUENCE;
                    machine.App.GCodeSetup.simulateMode = false;
                    pGcodeCmd->Result = -999;
                    
                    strncpy(machine.status_message, (char*)"AUTOMATIC mode", machine.status_message_size);
                }


                if (IS_SIMULATE_CYCLE_REQUESTED()) {
                    pSequence[0] = RUN_SEQUENCE;
                    machine.App.GCodeSetup.simulateMode = true;
                    strncpy(machine.status_message, (char*)"AUTOMATIC mode", machine.status_message_size);            
                }



                
            } else if (pSequence[0] == WAIT_FOR_RESUME_CYCLE) {
                // macchina in attesa start-ciclo (passo passo)
                if (IS_IN_MANUAL_REQUEST()) {
                    // Passaggio in manuale : messa in stop della macchina ???
                    PUT_STOP_CYCLE();
                }

                if (IS_START_CYCLE_REQUESTED()) {
                    pSequence[0] = RUN_SEQUENCE;
                }
                


                
            } else if (pSequence[0] == RUN_SEQUENCE) {
                                
                // macchina in run
                if (IS_IN_MANUAL_REQUEST()) {
                    // Passaggio in manuale : messa in stop della macchina ???
                    PUT_STOP_CYCLE();
                }                

                // Stop : ferma il flosso d'esecuzione
                if (IS_STOP_CYCLE_REQUESTED()) {
                    pSequence[0] = WAIT_FOR_START_CYCLE;
                    return 1;
                }
                
                // BIT29 ->  Break point
                if (machine.App.GCode.RowsOptions[pGcodeCmd->curRow] & BIT29) {
                    pSequence[0] = WAIT_FOR_RESUME_CYCLE;
                    snprintf (machine.status_message, machine.status_message_size, (char*)"BREAK POINT at row %d", pGcodeCmd->curRow + 1);
                    
                } else {
                        

                    /////////////////////////////////////
                    // Processa la riga del codice
                    //
                    if (pGcodeCmd->curRow < machine.App.GCode.numRows) {
                        
                        pGcodeCmd->Result = process_gcode_row ( machine.App.GCode.Rows[pGcodeCmd->curRow], &machine.App.GCode.RowsOptions[pGcodeCmd->curRow], &pGcodeCmd->curRowChar, pGcodeCmd );

                        // Rende il codice visibile
                        if (pGcodeCmd->curRow >= machine.App.GCode.startRow+machine.App.GCode.numDisplayRows) {
                            gcode_update_start_row ( pGcodeCmd->curRow );
                        }
                        

                        switch (pGcodeCmd->Result) {

                            // Errore
                            case -1:
                                pSequence[0] = DONE_SEQUENCE;
                            break;

                            // Nessun comando
                            case 0:
                                pSequence[0] = DONE_SEQUENCE;
                            break;

                            // Riga completa
                            case 1:
                                // Opzione riga generante il comendo
                                pGcodeCmd->RowOptions = machine.App.GCode.RowsOptions[pGcodeCmd->curRow];

                                if(pGcodeCmd->gotoRow1B) {
                                    pSequence[0] = END_SEQUENCE;
                                } else {
                                    // processa il comando di cambio stato macchina
                                    if(pGcodeCmd->nextSequence) {
                                        pSequence[0] = pGcodeCmd->nextSequence;
                                    }
                                }

                            break;

                            // Parziale
                            case 999:
                                // processa il comando e continua il parsing

                                // Opzione riga generante il comendo
                                pGcodeCmd->RowOptions = machine.App.GCode.RowsOptions[pGcodeCmd->curRow];

                                // processa il comando di cambio stato macchina
                                if(pGcodeCmd->nextSequence)
                                    pSequence[0] = pGcodeCmd->nextSequence;
                            break;
                        }


                        
                        
                        ////////////////////////////////////////////////////////////////////////
                        // Camandi ad esecuzione immediata (rotazione mandrino / cooler ...
                        //                            
                        if (pGcodeCmd->parserResult == 1 || pGcodeCmd->parserResult == 999) {
                            float speed_rpm = 0.0f;

                            if (pGcodeCmd->RowOptions & BIT25) {
                                 // M spingle code setted    
                                if (pGcodeCmd->RowOptions & BIT20) {
                                    // S spedd setted
                                    speed_rpm = pGcodeCmd->spindle_speed_rpm;
                                } else {
                                    speed_rpm = machine.settings.spindle_speed;
                                }

                                if (fabs(pGcodeCmd->spindle_speed_rpm - machine.actuator[SPINDLE].speed) > machine.settings.spindle_speed_toll) {
                                }

                                if (pGcodeCmd->spindleCMD == 0) {
                                    if (machine.App.GCodeSetup.simulateMode) {
                                        // simulazione
                                    } else {
                                        if (stop_spindle() < 0) {
                                        }
                                    }
                                } else if (pGcodeCmd->spindleCMD == CLOCKWISE) {
                                    if (machine.App.GCodeSetup.simulateMode) {
                                        // simulazione
                                    } else {
                                        if (start_spindle(0.0, speed_rpm, CLOCKWISE) < 0) {
                                        }
                                    }
                                } else if (pGcodeCmd->spindleCMD == COUNTERCLOCKWISE) {
                                    if (machine.App.GCodeSetup.simulateMode) {
                                        // simulazione
                                    } else {
                                        if (start_spindle(0.0, speed_rpm, CLOCKWISE) < 0) {
                                        }
                                    }
                                }
                            }



                            if (pGcodeCmd->RowOptions & BIT26) {
                                  // M cooler code setted
                                if (pGcodeCmd->CoolerCMD == 1) {
                                    // start cooler
                                    if (machine.App.GCodeSetup.simulateMode) {
                                        // simulazione
                                    } else {
                                        start_cooler_I();
                                    }
                                } else if (pGcodeCmd->CoolerCMD == 0) {
                                    // stop cooler
                                    if (machine.App.GCodeSetup.simulateMode) {
                                        // simulazione
                                    } else {
                                        stop_cooler_I();
                                    }
                                }
                            }
                        }
                        
                        
                    } else {
                        //////////////////////
                        // Fine programma 
                        //
                        if (generate_alarm((char*) "Program terminated", 2220, 1, (int) ALARM_LOG, 0+1) < 0) {
                        }
                        
                        PUT_MANUAL();
                                
                        pSequence[0] = END_SEQUENCE;
                    }                
                }
                
                /////////////////////////////////////////////////
                // Movimento radipo interpolazione lineare
                //
            } else if (pSequence[0] == START_RAPID_MOVE) {               
                if (machine.App.GCodeSetup.simulateMode) {
                    // simulazione
                } else {
                    // Movimento non interpolato in rapido
                    CANSlot *pCANSlot = (CANSlot *) NULL;
                    float rapid_feed_XYZ = 0.0f;
                    int retVal = 0;
                    
                    if (pGcodeCmd->RowOptions & BIT5) {
                        if (!pCANSlot) {
                            pCANSlot = (CANSlot *)machine.actuator[X].pCANSlot;
                        } else {
                            if (pCANSlot != machine.actuator[X].pCANSlot) {
                                // Asse in schede diverse
                            }
                        }
                    } else {
                        pGcodeCmd->targetX = machine.actuator[X].cur_rpos;
                    }
                    
                    if (pGcodeCmd->RowOptions & BIT6) {
                        if (!pCANSlot) {
                            pCANSlot = (CANSlot *)machine.actuator[Y].pCANSlot;
                        } else {
                            if (pCANSlot != machine.actuator[Y].pCANSlot) {
                                // Asse in schede diverse
                            }
                        }
                    } else {
                        pGcodeCmd->targetY = machine.actuator[Y].cur_rpos;
                    }

                    if (pGcodeCmd->RowOptions & BIT7) {
                        if (!pCANSlot) {
                            pCANSlot = (CANSlot *)machine.actuator[Z].pCANSlot;
                        } else {
                            if (pCANSlot != machine.actuator[Z].pCANSlot) {
                                // Asse in schede diverse
                            }
                        }
                    } else {
                        pGcodeCmd->targetZ = machine.actuator[Z].cur_rpos;
                    }
                    
                    rapid_feed_XYZ = sqrt (machine.settings.rapid_feed_X*machine.settings.rapid_feed_X + machine.settings.rapid_feed_Y*machine.settings.rapid_feed_Y + machine.settings.rapid_feed_Z*machine.settings.rapid_feed_Z);
                    if (machine.App.GCodeSetup.simulateMode) {
                        // simulazione
                    } else {
                        if ((retVal = gcode_act_free_linear_move ( (void*)pCANSlot, (void*)&machine.actuator[X], (void*)&machine.actuator[Y], (void*)&machine.actuator[Z], 
                            pGcodeCmd->targetX, pGcodeCmd->targetY, pGcodeCmd->targetZ, rapid_feed_XYZ, pGcodeCmd->Precision )) > 0) {
                            pSequence[0] = WAIT_RAPID_MOVE;
                        } else {
                            snprintf(str, sizeof(str), "Unable to send multi-axis command to CANBUS");
                            if (generate_alarm((char*) str, 8010, 0, (int) ALARM_ERROR, 0+1) < 0) {
                            }
                        }
                    }                    
                }
                
                
            } else if (pSequence[0] == WAIT_RAPID_MOVE) {
                BOOL procedToNextStep = true;
                
                if (pGcodeCmd->RowOptions & BIT5) {
                    if (machine.actuator[X].step != STEP_READY) {
                        procedToNextStep = false;
                        } else {
                        if (machine.actuator[X].position != machine.actuator[X].target_position) {
                            procedToNextStep = false;
                        } else {
                        }
                    }
                }                
                if (pGcodeCmd->RowOptions & BIT6) {
                    if (machine.actuator[Y].step != STEP_READY) {
                        procedToNextStep = false;
                        } else {
                        if (machine.actuator[Y].position != machine.actuator[Y].target_position) {
                            procedToNextStep = false;
                        } else {
                        }
                    }
                }
                if (pGcodeCmd->RowOptions & BIT7) {
                    if (machine.actuator[Z].step != STEP_READY) {
                        procedToNextStep = false;
                        } else {
                        if (machine.actuator[Z].position != machine.actuator[Z].target_position) {
                            procedToNextStep = false;
                        } else {
                        }
                    }
                }
                
                if(procedToNextStep) pSequence[0] = DONE_RAPID_MOVE;                            
                
            } else if (pSequence[0] == DONE_RAPID_MOVE) {
                pSequence[0] = DONE_SEQUENCE;


                
                ///////////////////////////////////////////////////////////////
                // Movimento fresatura interpolazione linear/circolare
                //
            } else if (pSequence[0] == START_MILL_MOVE) {

                if (machine.App.GCodeSetup.simulateMode) {
                    // simulazione
                } else {
                    CANSlot *pCANSlot = (CANSlot *) NULL;
                    float speed_base_feed_mm_min = 0.0f;                    
                    int32_t retVal = 0;

                    
                    switch(pGcodeCmd->moveType) {

                        case 1:
                            ///////////////////////////////////////////////
                            // interpolazione lineare in fresatura
                            //
                            if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.XMap, 'X', &XAxisIndex, NULL) < 0) {
                            } else {
                            }
                            if (pGcodeCmd->RowOptions & BIT5) {
                                if (!pCANSlot) {
                                    pCANSlot = (CANSlot *)machine.actuator[XAxisIndex].pCANSlot;
                                } else {
                                    if (pCANSlot != machine.actuator[XAxisIndex].pCANSlot) {
                                        // Asse in schede diverse
                                    }
                                }
                            } else {
                                pGcodeCmd->targetX = machine.actuator[XAxisIndex].cur_rpos;
                            }
                            
                            
                            if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.YMap, 'Y', &YAxisIndex, NULL) < 0) {
                            } else {
                            }
                            if (pGcodeCmd->RowOptions & BIT6) {
                                if (!pCANSlot) {
                                    pCANSlot = (CANSlot *)machine.actuator[YAxisIndex].pCANSlot;
                                } else {
                                    if (pCANSlot != machine.actuator[YAxisIndex].pCANSlot) {
                                        // Asse in schede diverse
                                    }
                                }
                            } else {
                                pGcodeCmd->targetY = machine.actuator[YAxisIndex].cur_rpos;
                            }
                            

                            if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                            } else {
                            }
                            if (pGcodeCmd->RowOptions & BIT7) {
                                if (!pCANSlot) {
                                    pCANSlot = (CANSlot *)machine.actuator[ZAxisIndex].pCANSlot;
                                } else {
                                    if (pCANSlot != machine.actuator[ZAxisIndex].pCANSlot) {
                                        // Asse in schede diverse
                                    }
                                }
                            } else {
                                pGcodeCmd->targetZ = machine.actuator[ZAxisIndex].cur_rpos;
                            }
                            


                            // Velocità avanzamento
                            speed_base_feed_mm_min = gcode_get_mill_feed ( 10.0f, (void*)pGcodeCmd );
                            
                           
                            

                            if (machine.App.GCodeSetup.simulateMode) {
                                // simulazione
                            } else {
                                if ((retVal = gcode_act_interpolated_linear_move ( (void*)pCANSlot, (void*)&machine.actuator[XAxisIndex], (void*)&machine.actuator[YAxisIndex], (void*)&machine.actuator[ZAxisIndex], 
                                    pGcodeCmd->targetX, pGcodeCmd->targetY, pGcodeCmd->targetZ, speed_base_feed_mm_min, pGcodeCmd->Precision )) > 0) {
                                    pSequence[0] = WAIT_MILL_MOVE;
                                } else {
                                    snprintf(str, sizeof(str), "Unable to send multi-axis command G1, error:%d", retVal);
                                    if (generate_alarm((char*) str, 8011, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                    }
                                }
                            }                    
                            break;
                            

                            
                        case 2:
                        case 3: {
                            CANSlot *pCANSlot = (CANSlot *) NULL;
                            float speed_base_rpm, speed_base_feed_mm_min;
                            uint32_t XAxisIndex = 0, YAxisIndex = 0, ZAxisIndex = 0;
                            

                            ///////////////////////////////////////////////////
                            // interpolazione circolare CW (orario)
                            // interpolazione circolare CCW (anti-orario)
                            //
                            if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.XMap, 'X', &XAxisIndex, (void**)NULL) < 0) {
                            } else {
                            }
                            if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.YMap, 'Y', &YAxisIndex, (void**)NULL) < 0) {
                            } else {
                            }
                            if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, (void**)NULL) < 0) {
                            } else {
                            }

                            if (pGcodeCmd->RowOptions & BIT5) {
                                if (!pCANSlot) {
                                    pCANSlot = (CANSlot *)machine.actuator[XAxisIndex].pCANSlot;
                                } else {
                                    if (pCANSlot != machine.actuator[XAxisIndex].pCANSlot) {
                                        // Asse in schede diverse
                                    }
                                }
                            }
                            if (pGcodeCmd->RowOptions & BIT6) {
                                if (!pCANSlot) {
                                    pCANSlot = (CANSlot *)machine.actuator[YAxisIndex].pCANSlot;
                                } else {
                                    if (pCANSlot != machine.actuator[YAxisIndex].pCANSlot) {
                                        // Asse in schede diverse
                                    }
                                }
                            }
                            if (pGcodeCmd->RowOptions & BIT7) {
                                if (!pCANSlot) {
                                    pCANSlot = (CANSlot *)machine.actuator[ZAxisIndex].pCANSlot;
                                } else {
                                    if (pCANSlot != machine.actuator[ZAxisIndex].pCANSlot) {
                                        // Asse in schede diverse
                                    }
                                }
                            }

                            /////////////////////////////////////////////
                            // Coordinate del centro
                            //
                            // BIT12 ->  I setted
                            // BIT13 ->  J setted
                            // BIT14 ->  K setted

                            if (pGcodeCmd->RowOptions & BIT12) {
                                if (pGcodeCmd->abs_arc_coord) {
                                    pGcodeCmd->centerX = pGcodeCmd->I;
                                } else {
                                    pGcodeCmd->centerX = machine.actuator[XAxisIndex].cur_rpos + pGcodeCmd->I;
                                }
                            } else {                            
                                pGcodeCmd->centerX = machine.actuator[XAxisIndex].cur_rpos;
                            }

                            if (pGcodeCmd->RowOptions & BIT13) {
                                if (pGcodeCmd->abs_arc_coord) {
                                    pGcodeCmd->centerY = pGcodeCmd->J;
                                } else {
                                    pGcodeCmd->centerY = machine.actuator[YAxisIndex].cur_rpos + pGcodeCmd->J;
                                }
                            } else {                            
                                pGcodeCmd->centerY = machine.actuator[YAxisIndex].cur_rpos;
                            }

                            if (pGcodeCmd->RowOptions & BIT14) {
                                if (pGcodeCmd->abs_arc_coord) {
                                    pGcodeCmd->centerZ = pGcodeCmd->K;
                                } else {
                                    pGcodeCmd->centerZ = machine.actuator[ZAxisIndex].cur_rpos + pGcodeCmd->K;
                                }
                            } else {                            
                                pGcodeCmd->centerZ = machine.actuator[ZAxisIndex].cur_rpos;
                            }


                            // Velocità avanzamento
                            speed_base_feed_mm_min = gcode_get_mill_feed ( 10.0f, (void*)pGcodeCmd );
                            
                            
                            // Calcolo angolo iniziale/finale
                            float deltaX = machine.actuator[XAxisIndex].cur_rpos - pGcodeCmd->centerX;
                            float deltaY = machine.actuator[YAxisIndex].cur_rpos - pGcodeCmd->centerY;
                            float deltaZ = machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->centerZ;

                            float startAngRad = atan2(machine.actuator[YAxisIndex].cur_rpos - pGcodeCmd->centerY, machine.actuator[XAxisIndex].cur_rpos - pGcodeCmd->centerX);
                            float endAngRad = atan2(pGcodeCmd->targetY - pGcodeCmd->centerY, pGcodeCmd->targetX - pGcodeCmd->centerX);
                            float RadiusMM = sqrt( deltaX*deltaX + deltaY*deltaY ); // Raggio interpolazione circolare

                            if (RadiusMM < EPSILON) {

                                // Errore                                                              
                                snprintf(str, sizeof(str), "Invalid Radius");
                                if (generate_alarm((char*) str, 8015, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                }
                                
                            } else {
                            

                                if (pGcodeCmd->Precision < EPSILON) {
                                    pGcodeCmd->Precision = machine.settings.InterpolationPrecisionMM;
                                }

     
                                        
                                if (pGcodeCmd->RowOptions & BIT5 || pGcodeCmd->RowOptions & BIT6 || pGcodeCmd->RowOptions & BIT7) {
                                    
                                    // Avanzamento in Z (???)
                                    float feedZ = (pGcodeCmd->RowOptions & BIT7) ? (machine.settings.mill_feed_mm_min_Z) : (0.0f);

                                    ///////////////////////////////////////////////////
                                    // Impostazione degli assi all'interpolazione
                                    //
                                    if ((retVal = gcode_act_circular_move( (void*)pCANSlot, (void *)&machine.actuator[XAxisIndex], (void *)&machine.actuator[YAxisIndex], (void *)&machine.actuator[ZAxisIndex],
                                            pGcodeCmd->centerX, pGcodeCmd->centerY,
                                            RadiusMM, startAngRad, endAngRad, pGcodeCmd->Precision,
                                            pGcodeCmd->moveType,
                                            pGcodeCmd->targetX, pGcodeCmd->targetY, pGcodeCmd->targetZ, 
                                            speed_base_feed_mm_min, feedZ )) > 0) {

                                        pSequence[0] = WAIT_MILL_MOVE;

                                    } else {
                                        snprintf(str, sizeof(str), "Unable to start multi-axis interpolation G2/G3, error:%d", retVal);
                                        if (generate_alarm((char*) str, 8010, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                        }
                                    }

                                } else {
                                    // Non supportato                                                                
                                    snprintf(str, sizeof(str), "Unsupported multi-axis Interpolation plane");
                                    if (generate_alarm((char*) str, 8020, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                    }
                                    pCANSlot = NULL;
                                }                            
                            }
                             
                            break;
                        }
                    }
                }
                
            } else if (pSequence[0] == WAIT_MILL_MOVE) {
                BOOL procedToNextStep = true;
                float target_rpos_toll = 0.0f;
                
                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.XMap, 'X', &XAxisIndex, NULL) < 0) {
                } else {
                }
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.YMap, 'Y', &YAxisIndex, NULL) < 0) {
                } else {
                }
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                }
 
                if (pGcodeCmd->RowOptions & BIT5) {
                    if (machine.actuator[XAxisIndex].step != STEP_READY) {
                        procedToNextStep = false;
                        } else {
                        if (machine.actuator[XAxisIndex].position != machine.actuator[XAxisIndex].target_position) {
                            procedToNextStep = false;
                        } else {
                            target_rpos_toll = MAX(machine.actuator[XAxisIndex].start_rpos_toll, machine.actuator[XAxisIndex].end_rpos_toll);
                            if (target_rpos_toll <= 0.0f)
                                target_rpos_toll = machine.App.Epsilon;
                            if ( fabs(machine.actuator[XAxisIndex].cur_rpos - machine.actuator[XAxisIndex].target_rpos) > target_rpos_toll) {
                                procedToNextStep = false;
                            }
                        }
                    }
                }                
                if (pGcodeCmd->RowOptions & BIT6) {
                    if (machine.actuator[YAxisIndex].step != STEP_READY) {
                        procedToNextStep = false;
                        } else {
                        if (machine.actuator[YAxisIndex].position != machine.actuator[YAxisIndex].target_position) {
                            procedToNextStep = false;
                        } else {
                            target_rpos_toll = MAX(machine.actuator[YAxisIndex].start_rpos_toll, machine.actuator[YAxisIndex].end_rpos_toll);
                            if (target_rpos_toll <= 0.0f)
                                target_rpos_toll = machine.App.Epsilon;
                            if ( fabs(machine.actuator[YAxisIndex].cur_rpos - machine.actuator[YAxisIndex].target_rpos) > target_rpos_toll) {
                                procedToNextStep = false;
                            }
                        }
                    }
                }
                if (pGcodeCmd->RowOptions & BIT7) {
                    if (machine.actuator[ZAxisIndex].step != STEP_READY) {
                        procedToNextStep = false;
                        } else {
                        if (machine.actuator[ZAxisIndex].position != machine.actuator[ZAxisIndex].target_position) {
                            procedToNextStep = false;
                        } else {
                            target_rpos_toll = MAX(machine.actuator[ZAxisIndex].start_rpos_toll, machine.actuator[ZAxisIndex].end_rpos_toll);
                            if (target_rpos_toll <= 0.0f)
                                target_rpos_toll = machine.App.Epsilon;
                            if ( fabs(machine.actuator[ZAxisIndex].cur_rpos - machine.actuator[ZAxisIndex].target_rpos) > target_rpos_toll) {
                                procedToNextStep = false;
                            }
                        }
                    }
                }
                
                if(procedToNextStep) pSequence[0] = DONE_MILL_MOVE;                            
                
            } else if (pSequence[0] == DONE_MILL_MOVE) {
                
                machine.App.SectionTime = xTaskGetTickCount();
                pSequence[0] = DONE_MILL_WAIT;

            } else if (pSequence[0] == DONE_MILL_WAIT) {
                
                if (xTaskGetTickCount() - machine.App.SectionTime >= machine.App.AfterMillWaitTimeMS) {
                    pSequence[0] = DONE_SEQUENCE;
                }

                ///////////////////////////
                // Avvio rafreddatore I
                //
            } else if (pSequence[0] == START_COOLER_I) {
            } else if (pSequence[0] == WAIT_COOLER_I) {
            } else if (pSequence[0] == DONE__COOLER_I) {

                ///////////////////////////
                // Avvio rafreddatore II
                //
            } else if (pSequence[0] == START_COOLER_II) {                
            } else if (pSequence[0] == WAIT_COOLER_II) {
            } else if (pSequence[0] == DONE__COOLER_II) {

                ///////////////////////////////
                // Arresto rafreddatore II
                //
            } else if (pSequence[0] == STOP_COOLER_I) {
            } else if (pSequence[0] == STOP_COOLER_II) {


            } else if (pSequence[0] == SET_AXIS_MAP) {
                switch (pGcodeCmd->curPlane) {                    
                        // XY-plane (G17) ) (modalità faccia a 90 verso il basso)
                    case 17:
                        machine.App.GCodeSetup.XMap = 'X';
                        machine.App.GCodeSetup.YMap = 'Y';
                        machine.App.GCodeSetup.ZMap = 'Z';                        
                        break;
                        // ZX-plane (G18) (modalita faccia laterale alesatrice con testa a 90 dx o sx)
                        machine.App.GCodeSetup.XMap = 'X';
                        machine.App.GCodeSetup.YMap = 'Z';
                        machine.App.GCodeSetup.ZMap = 'Y';                        
                    case 18:
                        break;
                        // YZ-plane (G19) (modalita alesatice)
                        machine.App.GCodeSetup.XMap = 'Z';
                        machine.App.GCodeSetup.YMap = 'X';
                        machine.App.GCodeSetup.ZMap = 'Y';                        
                    case 19:
                        break;
                }
                
                pSequence[0] = DONE_SEQUENCE;
                
                

                
                
                
                
                
                ////////////////////////////////////////////////////////
                // Ciclo di foratura, G81, G82, G83
                //
                // Ciclo alesatura : 
                //      G85 Boring Cycle, Feed Out
                //      G86 Boring Cycle, Spindle Stop, Rapid Move Out
                //      G88 Boring Cycle, Spindle Stop, Manual Out
                //      G89 Boring Cycle, Dwell, Feed Out
                //
                // pGcodeCmd->moveType = tipo foratura
                // pGcodeCmd->R = Z Escape
                // pGcodeCmd->Q = Delta
                // pGcodeCmd->targetZ = quota prof. foratura
                // machine.App.GCodeSetup.ZMap = asse foratura
            } else if (pSequence[0] == START_DRILL_CYCLE || pSequence[0] == START_BORE_CYCLE) {
                                
                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.XMap, 'X', &XAxisIndex, NULL) < 0) {
                } else {
                }
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.YMap, 'Y', &YAxisIndex, NULL) < 0) {
                } else {
                }
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                }

                pGcodeCmd->drillDepth = 0;
                
                if (pGcodeCmd->RowOptions & BIT7) {
                } else {
                    pGcodeCmd->targetZ = machine.actuator[Z].cur_rpos;
                }
                
                if (machine.actuator[ZAxisIndex].step != STEP_READY) {
                    // Errore
                } else {
                    switch(pGcodeCmd->moveType) {
                        case 81:
                            pGcodeCmd->drillTarget = pGcodeCmd->targetZ;
                            pSequence[0] = RUNNING_DRILL_CYCLE;
                            break;
                        case 82:
                            pGcodeCmd->drillTarget = pGcodeCmd->targetZ;
                            pSequence[0] = RUNNING_DRILL_CYCLE;
                            break;
                        case 83:
                            if (fabs(pGcodeCmd->targetZ) - fabs(pGcodeCmd->drillDepth) > fabs(pGcodeCmd->Q)) {
                                pGcodeCmd->drillTarget = (fabs(pGcodeCmd->drillDepth) + fabs(pGcodeCmd->Q)) * fabs(pGcodeCmd->targetZ) / pGcodeCmd->targetZ;
                            } else {
                                pGcodeCmd->drillTarget = pGcodeCmd->targetZ;
                            }
                            pSequence[0] = RUNNING_DRILL_CYCLE;
                            break;
                        case 85:
                        case 86:
                        case 88:
                        case 89:
                            pGcodeCmd->drillTarget = pGcodeCmd->targetZ;
                            pSequence[0] = RUNNING_BORE_CYCLE;
                            break;
                        default:
                            // Errore
                            break;
                    }
                }
                
                // Verifica posizionamento
                if (machine.actuator[XAxisIndex].step == STEP_READY) {                    
                    if (fabs(machine.actuator[XAxisIndex].cur_rpos - pGcodeCmd->targetX) < machine.App.toll) {
                    } else {
                        pSequence[0] = POS_XY_DRILL_CYCLE;
                    }
                }
                if (machine.actuator[YAxisIndex].step == STEP_READY) {                    
                    if (fabs(machine.actuator[YAxisIndex].cur_rpos - pGcodeCmd->targetY) < machine.App.toll) {
                    } else {
                        pSequence[0] = POS_XY_DRILL_CYCLE;
                    }
                }
                
                
            } else if (pSequence[0] == POS_XY_DRILL_CYCLE) {
                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.XMap, 'X', &XAxisIndex, NULL) < 0) {
                } else {
                }
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.YMap, 'Y', &YAxisIndex, NULL) < 0) {
                } else {
                }
                
                float rapid_feed_XYZ = sqrt (machine.settings.rapid_feed_X*machine.settings.rapid_feed_X + machine.settings.rapid_feed_Y*machine.settings.rapid_feed_Y + machine.settings.rapid_feed_Z*machine.settings.rapid_feed_Z);
                if (machine.App.GCodeSetup.simulateMode) {
                    // simulazione
                } else {
                    if (gcode_act_free_linear_move ( (void*)machine.actuator[XAxisIndex].pCANSlot, (void*)&machine.actuator[XAxisIndex], (void*)&machine.actuator[YAxisIndex], (void*)NULL, 
                        pGcodeCmd->targetX, pGcodeCmd->targetY, 0.0f, rapid_feed_XYZ, pGcodeCmd->Precision ) > 0) {
                        pSequence[0] = WAIT_RAPID_MOVE;
                    } else {
                        snprintf(str, sizeof(str), "Unable to send multi-axis command to CANBUS");
                        if (generate_alarm((char*) str, 8010, 0, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    }
                }                    

                pSequence[0] = WAIT_POS_XY_DRILL_CYCLE;
                
                

            } else if (pSequence[0] == WAIT_POS_XY_DRILL_CYCLE) {                                
                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.XMap, 'X', &XAxisIndex, NULL) < 0) {
                } else {
                }
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.YMap, 'Y', &YAxisIndex, NULL) < 0) {
                } else {
                }


                if (machine.actuator[XAxisIndex].step == STEP_READY) {                    
                    if (fabs(machine.actuator[XAxisIndex].cur_rpos - pGcodeCmd->targetX) < machine.App.toll) {
                        if (machine.actuator[YAxisIndex].step == STEP_READY) {                    
                            if (fabs(machine.actuator[YAxisIndex].cur_rpos - pGcodeCmd->targetY) < machine.App.toll) {
                                // Verifica quota posizione Z iniziale (piano R)
                                if (machine.actuator[ZAxisIndex].step == STEP_READY) {                    
                                    if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->R) < machine.App.toll) {
                                        pSequence[0] = RUNNING_DRILL_CYCLE;                        
                                    } else {
                                        pSequence[0] = POS_Z_DRILL_CYCLE;
                                    }
                                }
                            }
                        }
                    }
                }
                




            } else if (pSequence[0] == POS_Z_DRILL_CYCLE) {
                
                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                }
                float rapid_feed_XYZ = sqrt (machine.settings.rapid_feed_X*machine.settings.rapid_feed_X + machine.settings.rapid_feed_Y*machine.settings.rapid_feed_Y + machine.settings.rapid_feed_Z*machine.settings.rapid_feed_Z);
                if (machine.App.GCodeSetup.simulateMode) {
                    // simulazione
                } else {
                    if (gcode_act_free_linear_move ( (void*)machine.actuator[ZAxisIndex].pCANSlot, (void*)NULL, (void*)NULL, (void*)&machine.actuator[ZAxisIndex], 
                        0.0f, 0.0f, pGcodeCmd->R, rapid_feed_XYZ, pGcodeCmd->Precision ) > 0) {
                        pSequence[0] = WAIT_POS_Z_DRILL_CYCLE;
                    } else {
                        snprintf(str, sizeof(str), "Unable to send multi-axis command to CANBUS");
                        if (generate_alarm((char*) str, 8010, 0, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    }
                }                
                

            } else if (pSequence[0] == WAIT_POS_Z_DRILL_CYCLE) {

                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                }

                if (machine.actuator[ZAxisIndex].step == STEP_READY) {                    
                    if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->R) < machine.App.toll) {
                        switch(pGcodeCmd->moveType) {
                            case 81:
                            case 82:
                            case 83:
                                pSequence[0] = RUNNING_DRILL_CYCLE;
                                break;
                            case 85:
                            case 86:
                            case 88:
                            case 89:
                                pSequence[0] = RUNNING_BORE_CYCLE;
                                break;
                            default:
                                // Errore
                                break;
                        }
                    }
                }

                

                ////////////////////////////////////////////////////////                
                // Passa il comando di Foratura/Alesatura all'asse Z
                //
            } else if (pSequence[0] == RUNNING_DRILL_CYCLE || pSequence[0] == RUNNING_BORE_CYCLE) {

                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                    // Velocità avanzamento
                    float feed_mm_min = gcode_get_mill_feed ( 10.0f, (void*)pGcodeCmd );
                    if (gcode_act_free_linear_move ( (void*)machine.actuator[ZAxisIndex].pCANSlot, (void*)NULL, (void*)NULL, (void*)&machine.actuator[ZAxisIndex], 
                            0.0f, 0.0f, pGcodeCmd->drillTarget, feed_mm_min, pGcodeCmd->Precision ) > 0) {
                        switch(pGcodeCmd->moveType) {
                            case 81:
                            case 82:
                            case 83:
                                pSequence[0] = WATING_DRILL_CYCLE;
                                break;
                            case 85:
                            case 86:
                            case 88:
                            case 89:
                                pSequence[0] = WAITING_BORE_CYCLE;
                                break;
                            default:
                                // Errore
                                break;
                        }
                        
                    } else {
                        snprintf(str, sizeof(str), "Unable to send multi-axis command to CANBUS");
                        if (generate_alarm((char*) str, 8010, 0, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    }
                }
                        
            } else if (pSequence[0] == WATING_DRILL_CYCLE || pSequence[0] == WAITING_BORE_CYCLE) {
                uint32_t ZAxisIndex = 0;                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                }
                if (machine.actuator[ZAxisIndex].step == STEP_READY) {                    
                    if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->drillTarget) < machine.App.toll) {
                        switch(pGcodeCmd->moveType) {
                            case 81:
                                pSequence[0] = DONE_SEQUENCE;
                                break;
                            case 82:
                                pGcodeCmd->WaitTimeMS = pGcodeCmd->P * 1000.0f;
                                machine.App.SectionTime = xTaskGetTickCount();
                                pSequence[0] = WAIT_DONE_DRILL_CYCLE;
                                break;
                            case 83:
                                if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->targetZ) < machine.App.toll) {
                                    pSequence[0] = WAIT_DONE_DRILL_CYCLE;
                                    machine.App.SectionTime = xTaskGetTickCount();
                                    pGcodeCmd->WaitTimeMS = 0;
                                } else {
                                    // Scarico trucciolo e ripresa foratura
                                    pSequence[0] = MOVING_OUT_DRILL_CYCLE;
                                }
                                break;                            
                            case 85:
                                if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->targetZ) < machine.App.toll) {
                                    // Scarico trucciolo e ripresa foratura
                                    pSequence[0] = MOVING_OUT_BORE_CYCLE;
                                }
                                break;                            
                            case 86:
                            case 88:
                                if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->targetZ) < machine.App.toll) {
                                    // Scarico trucciolo e ripresa foratura
                                    pSequence[0] = SPINDLE_OFF_BORE_CYCLE;
                                }
                                break;                            

                            case 89:
                                if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->targetZ) < machine.App.toll) {
                                    // Scarico trucciolo e ripresa foratura
                                    machine.App.SectionTime = xTaskGetTickCount();
                                    pGcodeCmd->WaitTimeMS = pGcodeCmd->P * 1000.0f;
                                    pSequence[0] = WAIT_BORE_CYCLE;
                                }
                                break;                            
                        }
                    }
                }

                

                
                /////////////////////////////////////////////                
                // Passa il comando di Arresto mandrino
                //
            } else if (pSequence[0] == SPINDLE_OFF_BORE_CYCLE) {
                if (stop_spindle() >= 0) {
                    switch(pGcodeCmd->moveType) {
                        case 86:
                            pSequence[0] = WAIT_SPINDLE_OFF;
                            break;
                        default:
                            // Errore
                            break;
                    }                            
                }

                
            } else if (pSequence[0] == WAIT_SPINDLE_OFF) {
                if (machine.actuator[SPINDLE].step == STEP_READY) {
                    if (fabs(machine.actuator[SPINDLE].speed) < machine.App.Epsilon) {
                        switch(pGcodeCmd->moveType) {
                            case 86:
                                pSequence[0] = RAPID_OUT_BORE_CYCLE;
                                break;
                            case 88:
                                pSequence[0] = DONE_SEQUENCE;
                                put_machine_in_manual_mode();
                                break;
                            default:
                                // Errore
                                break;
                        }                            
                    }
                }

                

                /////////////////////////////////////
                // Attesa fine ciclo alesatura G89
                //
            } else if (pSequence[0] == WAIT_BORE_CYCLE) {
                
                if (xTaskGetTickCount() - machine.App.SectionTime >= pGcodeCmd->WaitTimeMS) {
                    switch(pGcodeCmd->moveType) {
                        case 89:
                            // G89 Boring Cycle, Dwell, Feed Out
                            pSequence[0] = MOVING_OUT_BORE_CYCLE;
                            break;
                        default:
                            pSequence[0] = DONE_SEQUENCE;
                            break;
                    }                            
                }
                
                        
                
                ///////////////////////////////////////////////////////////                
                // Passa il comando di uscita dal foro all'asse G83, G85
                //
                // (pGcodeCmd->R = Z Escape)
                //
            } else if (pSequence[0] == MOVING_OUT_DRILL_CYCLE || pSequence[0] == MOVING_OUT_BORE_CYCLE  || pSequence[0] == RAPID_OUT_BORE_CYCLE) {
                uint32_t ZAxisIndex = 0;                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                    // Velocità avanzamento
                    float move_out_mm_min = 0.0f;
                    
                    if (pSequence[0] == MOVING_OUT_DRILL_CYCLE || pSequence[0] == MOVING_OUT_BORE_CYCLE) {
                        move_out_mm_min = gcode_get_mill_feed ( 10.0f, (void*)pGcodeCmd );
                    } else if (pSequence[0] == RAPID_OUT_BORE_CYCLE) {
                        move_out_mm_min = gcode_get_rapid_feed (machine.App.GCodeSetup.ZMap, (void *)pGcodeCmd );
                    }
                    if (gcode_act_free_linear_move ( (void*)machine.actuator[ZAxisIndex].pCANSlot, (void*)NULL, (void*)NULL, (void*)&machine.actuator[ZAxisIndex], 
                            0.0f, 0.0f, pGcodeCmd->R, move_out_mm_min, pGcodeCmd->Precision ) > 0) {
                        switch(pGcodeCmd->moveType) {
                            case 81:
                            case 82:
                            case 83:
                                pSequence[0] = WAITING_OUT_DRILL_CYCLE;
                                break;
                            case 85:
                            case 86:
                            case 88:
                            case 89:
                                pSequence[0] = WAITING_OUT_BORE_CYCLE;
                                break;
                            default:
                                // Errore
                                break;
                        }
                        
                    } else {
                        snprintf(str, sizeof(str), "Unable to send multi-axis command to CANBUS");
                        if (generate_alarm((char*) str, 8010, 0, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    }                    
                }
                
            } else if (pSequence[0] == WAITING_OUT_DRILL_CYCLE || pSequence[0] == WAITING_OUT_BORE_CYCLE) {
                uint32_t ZAxisIndex = 0;                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                    if (machine.actuator[ZAxisIndex].step == STEP_READY) {
                        if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->R) < machine.App.toll) {
                            switch(pGcodeCmd->moveType) {
                                case 81:
                                case 82:
                                    pSequence[0] = DONE_SEQUENCE;
                                    break;
                                case 83:
                                    pSequence[0] = MOVING_IN_DRILL_CYCLE;
                                    break;
                                case 85:
                                case 86:
                                case 88:
                                case 89:
                                    pSequence[0] = DONE_SEQUENCE;
                                    break;
                                default:
                                    // Errore
                                    break;
                            }                            
                        }
                    }
                }
                

                //////////////////////////////////////////////////////
                // Passa il comando di ingresso nel foro all'asse
            } else if (pSequence[0] == MOVING_IN_DRILL_CYCLE) {
                uint32_t ZAxisIndex = 0;                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                    // Velocità avanzamento
                    float rapid_feed_mm_min = gcode_get_rapid_feed (machine.App.GCodeSetup.ZMap, (void *)pGcodeCmd);
                    
                    pGcodeCmd->drillTarget -= (pGcodeCmd->drillTarget/fabs(pGcodeCmd->drillTarget) * 0.25f);
                    
                    if (gcode_act_free_linear_move ( (void*)machine.actuator[ZAxisIndex].pCANSlot, (void*)NULL, (void*)NULL, (void*)&machine.actuator[ZAxisIndex], 
                            0.0f, 0.0f, pGcodeCmd->drillTarget, rapid_feed_mm_min, pGcodeCmd->Precision ) > 0) {
                        pSequence[0] = WAITING_IN_DRILL_CYCLE;                        
                    } else {
                        snprintf(str, sizeof(str), "Unable to send multi-axis command to CANBUS");
                        if (generate_alarm((char*) str, 8010, 0, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    }                    
                }

                //////////////////////////////////////////                
                // Attesa posizionamento dentro foro
                //
            } else if (pSequence[0] == WAITING_IN_DRILL_CYCLE) {
                uint32_t ZAxisIndex = 0;                            
                if (gcode_get_mapped_actuator ( machine.App.GCodeSetup.ZMap, 'Z', &ZAxisIndex, NULL) < 0) {
                } else {
                    if (machine.actuator[ZAxisIndex].step == STEP_READY) {
                        if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->drillTarget) < machine.App.toll) {
                            // Ripartenza foratura
                            switch(pGcodeCmd->moveType) {
                                case 83:
                                    if (fabs(machine.actuator[ZAxisIndex].cur_rpos - pGcodeCmd->targetZ) < machine.App.toll) {
                                        pSequence[0] = WAIT_DONE_DRILL_CYCLE;
                                        machine.App.SectionTime = xTaskGetTickCount();
                                        pGcodeCmd->WaitTimeMS = 100;
                                    } else {
                                        pGcodeCmd->drillDepth += fabs(pGcodeCmd->Q);
                                        if (fabs(pGcodeCmd->targetZ) - fabs(pGcodeCmd->drillDepth) > fabs(pGcodeCmd->Q)) {
                                            pGcodeCmd->drillTarget = (fabs(pGcodeCmd->drillDepth) + fabs(pGcodeCmd->Q)) * fabs(pGcodeCmd->targetZ) / pGcodeCmd->targetZ;
                                        } else {
                                            pGcodeCmd->drillTarget = pGcodeCmd->targetZ;
                                        }
                                        pSequence[0] = RUNNING_DRILL_CYCLE;
                                    }
                                    break;
                                default:
                                    // Errore
                                    break;
                            }
                        }
                    }
                }
                

                ///////////////////////////////////
                // Attesa fine ciclo foratura
                //
            } else if (pSequence[0] == WAIT_DONE_DRILL_CYCLE) {
                
                if (xTaskGetTickCount() - machine.App.SectionTime >= pGcodeCmd->WaitTimeMS) {
                    pSequence[0] = DONE_SEQUENCE;
                }

                
                
            } else if (pSequence[0] == SET_WAIT_SEQUENCE) {
                machine.App.SectionTime = xTaskGetTickCount();
                pSequence[0] = WAIT_SEQUENCE;
                
            } else if (pSequence[0] == WAIT_SEQUENCE) {
                if (xTaskGetTickCount() - machine.App.SectionTime >= pGcodeCmd->WaitTimeMS) {
                    pSequence[0] = DONE_SEQUENCE;
                }
                

                
                


                
                

                ////////////////////////////////////////////////////////////
                //
                // Processa il risultato dopo aver teminato la riga
                //
                //
                
            } else if (pSequence[0] == DONE_SEQUENCE) {
                BOOL IsNewRow = false;
                
                switch (pGcodeCmd->Result) {

                    // Errore
                    case -1:
                        BIT_ON(machine.App.GCode.RowsOptions[pGcodeCmd->curRow], BIT2); // Riga con errore
                        BIT_OFF(machine.App.GCode.RowsOptions[pGcodeCmd->curRow], BIT1);
                        pGcodeCmd->curRow++;
                        pGcodeCmd->curRowChar = 0;
                        IsNewRow = true;
                    break;

                    // Nessun comando
                    case 0:
                        BIT_ON(machine.App.GCode.RowsOptions[pGcodeCmd->curRow], BIT3); // Riga processata
                        BIT_OFF(machine.App.GCode.RowsOptions[pGcodeCmd->curRow], BIT1);
                        pGcodeCmd->curRow++;
                        pGcodeCmd->curRowChar = 0;
                        IsNewRow = true;
                    break;

                    // Riga completa
                    case 1:
                        BIT_ON(machine.App.GCode.RowsOptions[pGcodeCmd->curRow], BIT3); // BIT3 ->  Riga processata
                        BIT_OFF(machine.App.GCode.RowsOptions[pGcodeCmd->curRow], BIT1);

                        if(pGcodeCmd->gotoRow1B) {
                            if (pGcodeCmd->gotoRow1B == (uint32_t)-1) {
                                // Fine programma
                                strncpy(machine.status_message, (char*)"PROGRAM TERMINATED", machine.status_message_size);
                                pSequence[0] = END_SEQUENCE;
                                return 1;
                            } else {
                                pGcodeCmd->curRow = pGcodeCmd->gotoRow1B-1;
                                IsNewRow = true;
                            }
                        } else {
                            pGcodeCmd->curRow++;
                            IsNewRow = true;
                        }
                        pGcodeCmd->curRowChar = 0;                        

                    break;

                    // Parziale
                    case 999:
                    break;
                }

                if (IsNewRow) {
                    machine.App.GCode.RowsOptions[pGcodeCmd->curRow] = BIT1;
                    
                    if (pGcodeCmd->curRow == machine.App.GCode.cRow) {
                        BIT_ON(machine.App.GCode.RowsOptions[pGcodeCmd->curRow], BIT28);                        
                    }
                
                    // BIT1 ->  Riga in processo
                    // BIT2 ->  Riga con errore
                    // BIT3 ->  Riga processata
                    // BIT4 ->  Riga commento
                    // ----------------------------
                    // BIT5 ->  X setted
                    // BIT6 ->  Y setted
                    // BIT7 ->  Z setted
                    // BIT8 ->  W setted
                    // BIT9 ->  T setted                
                    // ----------------------------
                    // BIT10 ->  P setted
                    // BIT11 ->  R setted
                    // BIT12 ->  I setted
                    // BIT13 ->  J setted
                    // BIT14 ->  K setted
                    // BIT15 ->  Q setted
                    // BIT20 ->  S speed setted    
                
                    pGcodeCmd->nextSequence = 0;
                    pGcodeCmd->spindleCMD = 0;
                    pGcodeCmd->CoolerCMD = 0;
                    pGcodeCmd->gotoRow1B = 0;
                }
                
                pSequence[0] = RUN_SEQUENCE;
                
                
                

            } else if (pSequence[0] == END_SEQUENCE) {
                // macchina in stop
                if (IS_IN_MANUAL_REQUEST()) {
                    ///////////////////////
                    // Stato in Manuale
                    //
                    put_machine_in_manual_mode();
                }

                if (IS_START_CYCLE_REQUESTED()) {
                    pSequence[0] = INIT_SEQUENCE;
                    strncpy(machine.status_message, (char*)"AUTOMATIC mode", machine.status_message_size);
                } else {
                }
                
            } else {
                pSequence[0] = END_SEQUENCE;
                snprintf(str, sizeof(str), "Invalid main sequence");
                if (generate_alarm((char*) str, 8010, 0, (int) ALARM_ERROR, 0+1) < 0) {
                }
            }
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





//////////////////////////////////////////
// Comando Homing Asse X
//
int home_X() {
    char str[256];
    if (machine.actuator[X].step == STEP_READY) {
        machine.actuator[X].step = STEP_SEND_HOMING;
        return 1;
    } else {
        if (!machine.actuator[X].error) {
            machine.actuator[X].error = 7900;
            snprintf((char*) str, sizeof(str), (char*) "X Actuator not ready for homing (%s:%0.3f/%0.3f) (reads:%d) (Pos:%s - Tgt:%s)"
                    , get_actuator_step(&machine.actuator[X])
                    , machine.actuator[X].cur_rpos
                    , machine.actuator[X].start_rpos 
                    , machine.actuator[X].readCounter
                    , (char*)(machine.actuator[X].position == ON ? "UP" : "DOWN"), (char*)(machine.actuator[X].target_position == ON ? "UP" : "DOWN")
                    ); 
            vDisplayMessage(App.Msg);
            if (generate_alarm((char*) str, 7900, X, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}

int home_Y() {
    char str[256];
    if (machine.actuator[Y].step == STEP_READY) {
        machine.actuator[Y].step = STEP_SEND_HOMING;
        return 1;
    } else {
        if (!machine.actuator[Y].error) {
            machine.actuator[Y].error = 7900;
            snprintf((char*) str, sizeof(str), (char*) "Y Actuator not ready for homing (%s:%0.3f/%0.3f) (reads:%d) (Pos:%s - Tgt:%s)"
                    , get_actuator_step(&machine.actuator[Y])
                    , machine.actuator[Y].cur_rpos
                    , machine.actuator[Y].start_rpos 
                    , machine.actuator[Y].readCounter
                    , (char*)(machine.actuator[Y].position == ON ? "UP" : "DOWN"), (char*)(machine.actuator[Y].target_position == ON ? "UP" : "DOWN")
                    ); 
            vDisplayMessage(App.Msg);
            if (generate_alarm((char*) str, 7900, Y, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}

int home_Z() {
    char str[256];
    if (machine.actuator[Z].step == STEP_READY) {
        machine.actuator[Z].step = STEP_SEND_HOMING;
        return 1;
    } else {
        if (!machine.actuator[Z].error) {
            machine.actuator[Z].error = 7900;
            snprintf((char*) str, sizeof(str), (char*) "Z Actuator not ready for homing (%s:%0.3f/%0.3f) (reads:%d) (Pos:%s - Tgt:%s)"
                    , get_actuator_step(&machine.actuator[Z])
                    , machine.actuator[Z].cur_rpos
                    , machine.actuator[Z].start_rpos 
                    , machine.actuator[Z].readCounter
                    , (char*)(machine.actuator[Z].position == ON ? "UP" : "DOWN"), (char*)(machine.actuator[Z].target_position == ON ? "UP" : "DOWN")
                    ); 
            vDisplayMessage(App.Msg);
            if (generate_alarm((char*) str, 7900, Z, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}





//////////////////////////////////////////
// Comando Jog Asse X
//
int jog_plus_X() {
    char str[256];
    if (machine.actuator[X].step == STEP_READY) {
        // machine.actuator[X].step = STEP_SEND_HOMING;
        if (machine.actuator[X].speed_man1 <= 0) machine.actuator[X].speed_man1 = 1;
        if (handle_actuator_speed_mode( (void *)&machine.actuator[X], (int32_t)fabs(machine.actuator[X].speed_man1)) < 0) {            
            return -1;
        }
        return 1;
    }
    return 0;
}

int jog_minus_X() {
    char str[256];
    if (machine.actuator[X].step == STEP_READY) {
        // machine.actuator[X].step = STEP_SEND_HOMING;
        if (machine.actuator[X].speed_man2 <= 0) machine.actuator[X].speed_man2 = 1;
        if (handle_actuator_speed_mode( (void *)&machine.actuator[X], (int32_t)(fabs(machine.actuator[X].speed_man2)*-1.0f)) < 0) {            
            return -1;
        }
        return 1;
    }
    return 0;
}


int jog_plus_Y() {
    char str[256];
    if (machine.actuator[Y].step == STEP_READY) {
        // machine.actuator[Y].step = STEP_SEND_HOMING;
        if (machine.actuator[Y].speed_man1 <= 0) machine.actuator[Y].speed_man1 = 1;
        if (handle_actuator_speed_mode( (void *)&machine.actuator[Y], (int32_t)fabs(machine.actuator[Y].speed_man1)) < 0) {            
            return -1;
        }
        return 1;
    }
    return 0;
}

int jog_minus_Y() {
    char str[256];
    if (machine.actuator[Y].step == STEP_READY) {
        // machine.actuator[Y].step = STEP_SEND_HOMING;
        if (machine.actuator[Y].speed_man2 <= 0) machine.actuator[Y].speed_man2 = 1;
        if (handle_actuator_speed_mode( (void *)&machine.actuator[Y], (int32_t)(fabs(machine.actuator[Y].speed_man2)*-1.0f)) < 0) {            
            return -1;
        }
        return 1;
    }
    return 0;
}


int jog_plus_Z() {
    char str[256];
    if (machine.actuator[Z].step == STEP_READY) {
        // machine.actuator[Z].step = STEP_SEND_HOMING;
        if (machine.actuator[Z].speed_man1 <= 0) machine.actuator[Z].speed_man1 = 1;
        if (handle_actuator_speed_mode( (void *)&machine.actuator[Z], (int32_t)fabs(machine.actuator[Z].speed_man1)) < 0) {            
            return -1;
        }
        return 1;
    }
    return 0;
}

int jog_minus_Z() {
    char str[256];
    if (machine.actuator[Z].step == STEP_READY) {
        // machine.actuator[Z].step = STEP_SEND_HOMING;
        if (machine.actuator[Z].speed_man2 <= 0) machine.actuator[Z].speed_man2 = 1;
        if (handle_actuator_speed_mode( (void *)&machine.actuator[Z], (int32_t)(fabs(machine.actuator[Z].speed_man2)*-1.0f)) < 0) {            
            return -1;
        }
        return 1;
    }
    return 0;
}







////////////////////////////////////////////////
// Comando apertura stampo
//
int start_spindle(float position, float speed_rpm , int direction) {
    return 0;
}

int stop_spindle( ) {
    return 0;
}

int stop_X(  ) {
    return 0;
}

int stop_Y(  ) {
    return 0;
}

int stop_Z(  ) {
    return 0;
}

int stop_W(  ) {
    return 0;
}

int stop_T(  ) {
    return 0;
}


int start_cooler_I(  ) {
    return 0;
}

int stop_cooler_I(  ) {
    return 0;
}

int start_cooler_II(  ) {
    return 0;
}

int stop_cooler_II(  ) {
    return 0;
}


int check_timeouts() {
    return 0;    
}
    

int check_actuators_status() {
    return 0;
}


