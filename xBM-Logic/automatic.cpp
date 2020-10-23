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

#define MAX_WAITING_MOLD_SEQUENCE_MS    3000


#define MAX_PRODUCTION_CYCLES_PER_HOUR  2500
#define MIN_PRODUCTION_CYCLES_PER_HOUR  10


#define MOULD_SEQ_WAITING_FOR_TRASFERITOR_X     20
#define MOULD_SEQ_WAITING_FOR_TRASFERITOR_Y     11
#define MOULD_SEQ_WAITING_FOR_CHAIN_STEP                12
#define MOULD_SEQ_WAITING_FOR_TRASF_INSIDE                13
#define PREFORM_STEPPER_SEQ_WAITING_PREFORM_FRONT 20
#define START_TRANSFERITOR_RETURN_SEQUENCE      50


#define CYCLE_TIME_MS_TOLL_TO_WARNING   150


///////////////////////////////////////////////////////////////////////////////////////////
// Prospettuta dei movimenti :
//
//      Le posizioni dentro /fuori, Alto / Basso sori riferite all'oggetto fisico
//      NON allo stelo del cilindro o allo zero asse
//
///////////////////////////////////////////////////////////////////////////////////////////


int is_blow_cycle_ended(void) {
    if (machine.App.production_cycle_time_ms >= machine.workSet.discharge_air_time_ms) {
        if (machine.App.mold_cycle_time_ms >= machine.App.production_cycle_time_ms - machine.workSet.discharge_air_time_ms) {
            return 1;
        }

    } else {                
        strncpy(machine.status_message, (char*)"Tempo ciclo non valido", machine.status_message_size);
        return 1;
    }
    return 0;
}

            
            
            
int automatic( uint32_t *pSequence, void *pvCmd ) {
    char str[256];
    
    try {
        
        
        // N.B.. Ricorsività non implementata
     if (pSequence) {
        } else {
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
            machine.App.mold_sequence = 0;
            machine.status = AUTOMATIC;



        } else if (machine.status == AUTOMATIC || machine.status == STEP_BY_STEP) {

            // Richiesta passaggio in Automatico
    #ifdef _DEBUG_MODE
            if (PUT_IN_AUTOMATIC()) {
                if (machine.status == STEY_BY_STEP) {
                    machine.status = AUTOMATIC;
                }
            }
    #endif




            /*
            __________________________________

            Sequenza principale
            __________________________________                                                                                                                                                                                                                                                                                                              */

            if (machine.sequence == INIT_SEQUENCE) {

                // Sequenza pressa
                machine.App.mold_sequence = INIT_SEQUENCE;

                // Sequenza del carico preforma
                machine.App.load_sequence = INIT_SEQUENCE;

                // Sequenza del pettine
                machine.App.manip_pref_sequence = INIT_SEQUENCE;

                // Sequenza dell' espulsore bottiglia
                machine.App.bottle_eject_sequence = INIT_SEQUENCE;

                // Sequenza del blocco/sblocco pista
                machine.App.pit_lock_sequence = INIT_SEQUENCE;

                // Sequenza dei forni
                machine.App.owens_sequence = INIT_SEQUENCE;



                machine.sequence = RUN_SEQUENCE;

                strncpy(machine.status_message, (char*)"AUTOMATIC mode", machine.status_message_size);



            } else if (machine.sequence == RUN_SEQUENCE) {
                // macchina in run
                if (IS_IN_MANUAL_REQUEST()) {
                    // Passaggio in manuale : messa in stop della macchina ???
                    PUT_STOP_CYCLE();
                }


            } else if (machine.sequence == END_SEQUENCE) {
                // macchina in stop
                if (IS_IN_MANUAL_REQUEST()) {
                    ///////////////////////
                    // Stato in Manuale
                    //
                    put_machine_in_manual_mode();
                }

            } else {
            }




            /*
            __________________________________

            Sequenza stampo / soffiaggio
            __________________________________                                                                                                                                                                                                                                                                                                              */

            if (machine.App.mold_sequence == INIT_SEQUENCE) {

                // Partenza sequenza principale

                // Controlli generali ....

                // Avvio sequenza
                machine.App.mold_sequence = START_SEQUENCE;



                // Partenza sequenza forni
                if (machine.App.heat_on) {
                    machine.App.owens_sequence = START_SEQUENCE;
                }




            } else if (machine.App.mold_sequence == START_SEQUENCE) {

                machine.App.mold_cycle_time_ms = 1;

                if (machine.workSet.productionRecomputed) {
                    machine.workSet.production = machine.workSet.productionRecomputed;
                    machine.workSet.productionRecomputed = 0;
                    snprintf((char*) App.Msg, App.MsgSize, (char*) "Recomputed production rate to %d", machine.workSet.production);
                    if (generate_alarm((char*) App.Msg, 7700, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                }

                // Calcolo durata del ciclo (msec)
                if ((int) machine.workSet.production < MIN_PRODUCTION_CYCLES_PER_HOUR) {
                    snprintf((char*) App.Msg, App.MsgSize, (char*) "Production out of range : %d", machine.workSet.production);
                    if (generate_alarm((char*) App.Msg, 7701, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                    machine.workSet.production = MIN_PRODUCTION_CYCLES_PER_HOUR;
                } else if ((int) machine.workSet.production > MAX_PRODUCTION_CYCLES_PER_HOUR) {
                    snprintf((char*) App.Msg, App.MsgSize, (char*) "Production out of range : %d", machine.workSet.production);
                    if (generate_alarm((char*) App.Msg, 7702, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                    machine.workSet.production = MAX_PRODUCTION_CYCLES_PER_HOUR;                
                }

                machine.App.production_cycle_time_ms = (uint32_t) (3600.0f * 1000.0f / (float) machine.workSet.production);

                if (machine.App.production_cycle_time_ms < 1000) {
                    snprintf(machine.status_message, 256, "WARNING : Tempo ciclo non valido:%d", machine.App.production_cycle_time_ms);
                    machine.workSet.production = machine.workSet.production * 0.9f;
                    machine.App.mold_sequence = INIT_SEQUENCE;

                } else {

                    memset(machine.once_actions, 0, sizeof (machine.once_actions));

                    machine.App.mold_sequence = MOULD_SEQ_WAITING_FOR_TRASFERITOR_Y;
                }







            } else if (machine.App.mold_sequence == MOULD_SEQ_WAITING_FOR_TRASFERITOR_Y) {


                ////////////////////////////////////////////////////////////////////////
                //
                // Punto Zero del ciclo : attesa inserimento Trasferitore stampo
                // N.B.: Dipendente dall'architettura del sigillo disoffiaggio
                //

                if (IS_STOP_CYCLE_REQUESTED()) {
                    // Arresto macchina

                    machine.status = READY;
                    machine.sequence = END_SEQUENCE;
                    strncpy(machine.status_message, (char*)"READY", machine.status_message_size);
                    
                    PUT_HEAT_OFF();

                } else {


                    if (machine.App.preform_registry[machine.App.BLOW_MOLD_POS] != EMPTY) {
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        // Preforma nello stampo presente : sequanza di soffiaggio e avvio sequenza manipolatore
                        //

                        // Stampo in asstesa Trasferitore dentro : Avvia della sequanza Trasferimento
                        machine.App.transferitor_sequence = START_SEQUENCE;

                        machine.App.mold_sequence = 13;



                    } else {
                        ///////////////////////////////////////////////////////////////////////////////
                        // Stampo in standby : avvio sequenza manipolatore catena
                        //
                        if (machine.App.manip_pref_sequence == END_SEQUENCE || machine.App.manip_pref_sequence == INIT_SEQUENCE) {
                            strncpy(machine.status_message, (char*)"", machine.status_message_size);
                            machine.App.manip_pref_sequence = START_SEQUENCE;
                            machine.App.mold_sequence = MOULD_SEQ_WAITING_FOR_CHAIN_STEP;

                        } else {
                        }
                    }
                }



            } else if (machine.App.mold_sequence == MOULD_SEQ_WAITING_FOR_CHAIN_STEP) {
                // Stampo in standby : attesa sequenza avanzamento mandrini.....


            } else if (machine.App.mold_sequence == MOULD_SEQ_WAITING_FOR_TRASF_INSIDE) {
                // Attesa trasferitore dentro per aprire lo stampo
                if (machine.actuator[TRANSFERITOR_Y].position == INSIDE) {
                    if (machine.actuator[TRANSFERITOR_X].position == BACK) {
                        machine.App.mold_sequence = 14;
                    } else {
                        recompute_prod(2, machine.App.mold_sequence, 0);
                    }
                }





            } else if (machine.App.mold_sequence == 14) {
                // Comando Apertura stampo, alza stazione blocco, alza manipolatore pref.

                if (machine.actuator[STRETCH].position == STRETCH_UP) { 
                    // Aste alte
                    if (machine.actuator[TRANSFERITOR_Y].position == INSIDE) { 
                        // Trasferitore stampo dentro
                        if (machine.actuator[TRANSFERITOR_X].position == BACKWARD) { 
                            // Trasferitore indietro

                            ///////////////////////////////////////////
                            // Controllo pressione residua
                            //
                            if (machine.actuator[BLOW_PRESSURE].cur_rpos <= machine.workSet.max_pressure_in_mold) {

                                // Apertura stampo
                                if (open_mold() > 0) {

                                    // Comando salita stazioni blocco
                                    if (up_holding_station() > 0) {

                                        // Attende la sequenza del manipolatore (che dovrebbe esse gia' pronta)
                                        machine.App.mold_sequence = 15;
                                    }

                                } else {
                                    // int dbg = 1;
                                    strncpy(machine.status_message, (char*)"WARNING : pressione nello stampo", machine.status_message_size);
                                }

                            } else {
                                /////////////////////////////////////////
                                // Presenza pressione residua
                                //
                                // Controllo del tempo di scarico e correione parametri
                                //
                                snprintf(machine.status_message, machine.status_message_size, "WARNING : pressione nello Stampo (%0.1f bar)", machine.actuator[BLOW_PRESSURE].cur_rpos);
                            }

                        } else {
                            strncpy(machine.status_message, (char*)"WARNING : Trasferitore NON INDIETRO", machine.status_message_size);
                        }
                    } else {
                        strncpy(machine.status_message, (char*)"WARNING : Trasferitore NON DENTRO", machine.status_message_size);
                    }
                } else {
                    strncpy(machine.status_message, (char*)"WARNING : Aste NON ALTE", machine.status_message_size);
                }


            } else if (machine.App.mold_sequence == 15) {

                machine.App.mold_sequence = 16;


            } else if (machine.App.mold_sequence == 16) {

                //////////////////////////////////////////////////////////////////////////////////////////////////////
                // Solleva il manipolatore pref asse Z e quindi Avvia la sequenza manipolatore pref
                //
                if (machine.App.manip_pref_sequence == END_SEQUENCE || machine.App.manip_pref_sequence == INIT_SEQUENCE) {
                    // strncpy(machine.status_message, (char*)"", machine.status_message_size);
                    machine.App.manip_pref_sequence = START_SEQUENCE;
                    machine.App.mold_sequence = 17;
                    // Ricalcola nel caso...
                } else {
                    //
                    // Trasferitore in ritardo
                    // imposta le basi per il ricalcolo ( a conteggio ciclo completo)
                    //
                    strncpy(machine.status_message, (char*)"Attesa Trasferitore preforme", machine.status_message_size);

                    if (machine.App.preform_registry[machine.App.BLOW_MOLD_POS] == PREFORM_OK) {
                        /*
                        snprintf((char*) App.Msg, App.MsgSize, (char*) "Preforms Transferitor late");
                        if (generate_alarm((char*) App.Msg, 7770, MOLD, (int) ALARM_WARNING) < 0) {
                        }
                        */
                    }
                }




            } else if (machine.App.mold_sequence == 17) {

                if (machine.actuator[MOLD].position == OPEN) {
                    // Stampo aperto : Pronto per attendere il trasferitore stampo asse x
                    machine.App.mold_sequence = MOULD_SEQ_WAITING_FOR_TRASFERITOR_X;
                }



            } else if (machine.App.mold_sequence == MOULD_SEQ_WAITING_FOR_TRASFERITOR_X) {
                // Attesa Avanzamento e inserimento Trasferitore
                if (machine.actuator[TRANSFERITOR_Y].position == INSIDE) {
                    if (machine.actuator[TRANSFERITOR_X].position == FRONT) {
                        // Trasferitore dentro e avanti
                        machine.App.mold_sequence = 30;
                    } else {
                        strncpy(machine.status_message, (char*)"Attesa Trasferitore avanti", machine.status_message_size);
                    }
                } else {
                    strncpy(machine.status_message, (char*)"Attesa Trasferitore dentro", machine.status_message_size);
                }

            } else if (machine.App.mold_sequence == 30) {
                // Chiusura stampo
                if (close_mold() > 0) {
                    machine.App.mold_sequence = 31;
                }


            } else if (machine.App.mold_sequence == 31) {
                // Abbassamento stazione blocco
                if (down_holding_station() > 0) {
                    machine.App.mold_sequence = 35;
                }


            } else if (machine.App.mold_sequence == 35) {
                // Attesa stampo chiuso
                if (machine.actuator[MOLD].position == CLOSE) {
                    machine.App.mold_sequence = 40;
                } else {
                    strncpy(machine.status_message, (char*)"Attesa chiusura stampo", machine.status_message_size);
                }


            } else if (machine.App.mold_sequence == 40) {

                // Avvio Sequenza ritorno Trasferitore
                machine.App.transferitor_sequence = START_TRANSFERITOR_RETURN_SEQUENCE;

                machine.App.mold_sequence = 41;



            } else if (machine.App.mold_sequence == 41) {

                if (machine.App.preform_registry[machine.App.BLOW_MOLD_POS] == PREFORM_OK) {
                    // Preforma presente e soffiabile : sequanza di soffiaggio
                    if (machine.App.blow_on) {
                        // soffiaggio abilitato
                        machine.App.mold_sequence = 50;
                    } else {
                        // Fine ciclo di soffiaggio
                        machine.App.mold_sequence = 90;
                    }
                } else {
                    // Preforma assente : salta la sequenza di soffiaggio e attende la fine del ciclo
                    if (machine.App.production_cycle_time_ms > machine.workSet.discharge_air_time_ms) {
                        if (machine.App.mold_cycle_time_ms >= machine.App.production_cycle_time_ms - machine.workSet.discharge_air_time_ms) {
                            // Fine ciclo di soffiaggio
                            machine.App.mold_sequence = 90;
                        }
                    } else {
                        // Fine ciclo di soffiaggio
                        machine.App.mold_sequence = 90;
                    }
                }


            } else if (machine.App.mold_sequence == 50) {

                // Attesa chiusura stampo
                if (machine.actuator[MOLD].position == CLOSE) {
                    machine.App.mold_sequence = 60;

                    ///////////////////////////////////////////////
                    // Verifiche ulteriori pre Ciclo Soffiaggio
                    // ...
                } else {
                    strncpy(machine.status_message, (char*)"WARNING : Attesa chiusura stampo", machine.status_message_size);
                }




                //////////////////////////
                // Inizio Stiro
                //
            } else if (machine.App.mold_sequence == 60) {

                if (machine.App.preform_registry[machine.App.BLOW_MOLD_POS] == PREFORM_OK) {
                    
                    if (machine.actuator[LINE_PRESSURE].cur_rpos >= machine.workSet.pressure_min) {
                
                        // Comando discesa asta
                        if (down_stretch_rod() > 0) {

                            // Comando chiusura valvole soffiaggio
                            close_discharge_air();

                            // Comando chiusura valvole recupero
                            close_recovery_air();

                            machine.App.mold_sequence = 61;
                            
                            machine.App.mold_sequence_start_time = 0;

                        } else {
                            strncpy(machine.status_message, (char*)"Attesa comando Aste basse", machine.status_message_size);
                        }
                        
                    } else {
                        machine.statistic.discharged++;
                        machine.rt_statistic.discharged++;

                        ///////////////////////////////////////////////////////////////////////
                        // Mancanza pressione -> Fine soffiaggio : su asta e scarico aria
                        //
                        machine.App.mold_sequence = 77;
                        
                    }
                    
                } else if (machine.App.preform_registry[machine.App.BLOW_MOLD_POS] != EMPTY) {
                    
                    machine.statistic.discharged++;
                    machine.rt_statistic.discharged++;
                    
                    //////////////////////////////////////////////////////////////////////////
                    // Preforma da scaratare -> Fine soffiaggio : su asta e scarico aria
                    //
                    machine.App.mold_sequence = 77;

                } else {

                    ///////////////////////////////////////////////////////////////////
                    // Preforma assente -> Fine soffiaggio : su asta e scarico aria
                    //
                    machine.App.mold_sequence = 77;
                    
                }
                

            } else if (machine.App.mold_sequence == 61) {

                machine.App.mold_sequence = 70;



            } else if (machine.App.mold_sequence == 70) {

                // Comando Apertura Aria Primaria
                if (machine.actuator[STRETCH].cur_rpos >= machine.workSet.primary_air_gap_mm) {
                    open_primary_air();
                    machine.App.mold_sequence = 71;
                    
                    machine.statistic.preforms_blowed++;
                    machine.rt_statistic.preforms_blowed++;


                } else if (machine.actuator[STRETCH].position == machine.actuator[STRETCH].target_position ||
                        machine.actuator[STRETCH].cur_rpos >= machine.actuator[STRETCH].end_rpos-0.05 ) {                
                    // Finecorsa stiro (Anticipato
                    machine.App.mold_sequence = 71;
                    machine.statistic.preforms_blowed++;
                    machine.rt_statistic.preforms_blowed++;
                } else {
                    if (machine.App.mold_sequence_start_time == 0) {
                        machine.App.mold_sequence_start_time = GLCurTimeMs;
                    } else if (GLCurTimeMs - machine.App.mold_sequence_start_time >= MAX_WAITING_MOLD_SEQUENCE_MS) {
                        snprintf((char*) str, sizeof(str), (char*) "Mold Sequence Timeout at %d", machine.App.mold_sequence);
                        if (generate_alarm((char*) str, 7770, MOLD, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    } else {
                        strncpy(machine.status_message, (char*)"Attesa Aste basse", machine.status_message_size);
                    }
                }





            } else if (machine.App.mold_sequence == 71) {

                // Chiusura aria primaria
                if (machine.actuator[STRETCH].position == STRETCH_DOWN) {


                    machine.App.mold_sequence = 72;


                    /////////////////////////////////////////////////////////////
                    // N.B.: ritardo aria secondaria in msec
                    //
                    machine.App.secondary_air_on_time_ms = machine.App.mold_cycle_time_ms + machine.workSet.secondary_air_gap_ms;


                    ///////////////////////////////////////////////////////////
                    // Base per il ricalcolo del tempo ciclo (asta bassa)
                    //
                    machine.App.reference_mold_cycle_time_ms = machine.App.mold_cycle_time_ms;


                } else {
                    if (machine.App.mold_sequence_start_time == 0) {
                        machine.App.mold_sequence_start_time = GLCurTimeMs;
                    } else if (GLCurTimeMs - machine.App.mold_sequence_start_time >= MAX_WAITING_MOLD_SEQUENCE_MS) {
                        snprintf((char*) str, sizeof(str), (char*) "Mold Sequence Timeout at %d", machine.App.mold_sequence);
                        if (generate_alarm((char*) str, 7771, MOLD, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    } else {
                        strncpy(machine.status_message, (char*)"Attesa Aste basse", machine.status_message_size);
                    }
                }

                // Pressione raqgiunta aria primaria
                machine.App.primmary_air_press = machine.actuator[BLOW_PRESSURE].cur_rpos;




            } else if (machine.App.mold_sequence == 72) {

                // Attesa ritado aria secondaria
                if (machine.App.mold_cycle_time_ms >= machine.App.secondary_air_on_time_ms) {

                    // Chiusura Aria primaria
                    close_primary_air();


                    machine.App.mold_sequence = 73;

                } else {
                    strncpy(machine.status_message, (char*)"Attesa ritardo AriaII", machine.status_message_size);
                }

                // calcolo tempo aria secondaria
                if (machine.App.secondary_air_persistence_start > 0)
                    machine.App.secondary_air_persistence_time_ms = xTaskGetTickCount() - machine.App.secondary_air_persistence_start;

                // Pressione raqgiunta aria primaria
                machine.App.primmary_air_press = machine.actuator[BLOW_PRESSURE].cur_rpos;

                // Pressione raqgiunta aria scondaria
                machine.App.secondary_air_press = machine.actuator[BLOW_PRESSURE].cur_rpos;



            } else if (machine.App.mold_sequence == 73) {

                if (is_blow_cycle_ended() > 0) {                
                    strncpy(machine.status_message, (char*)"Premature blow cycle end", machine.status_message_size);
                    /*
                    snprintf((char*) str, sizeof(str), (char*) "Premature blow cycle end SEQ:%d", machine.App.mold_sequence);
                    if (generate_alarm((char*) str, 7772, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                    */
                    
                // Chiusura valvola aria secondaria, salita asta
                if (close_secondary_air() > 0) {
                    // Chiusura Aria secondaria e Apertura scarico
                    machine.App.mold_sequence = 78;
                }

                } else {

                    // Attesa avvenuta chiusura aria primaria
                    if (machine.actuator[PRIMARY_AIR].position == OFF) {
                        // Apertura valvola secondaria
                        machine.App.mold_sequence = 74;
                    } else {
                        strncpy(machine.status_message, (char*)"Attesa chiusura AriaI", machine.status_message_size);
                    }
                }

                // calcolo tempo aria secondaria
                if (machine.App.secondary_air_persistence_start > 0)
                    machine.App.secondary_air_persistence_time_ms = xTaskGetTickCount() - machine.App.secondary_air_persistence_start;

                // Pressione raqgiunta aria scondaria
                machine.App.secondary_air_press = machine.actuator[BLOW_PRESSURE].cur_rpos;




            } else if (machine.App.mold_sequence == 74) {

                // Attesa ritado aria secondaria
                if (machine.App.mold_cycle_time_ms >= machine.App.secondary_air_on_time_ms) {

                    // Apertura valvola secondaria
                    open_secondary_air();

                    // Avvio sezione temporale persistenza aria secondaria
                    machine.App.secondary_air_persistence_start = xTaskGetTickCount();

                    machine.App.mold_sequence = 77;

               } else {
                    strncpy(machine.status_message, (char*)"Attesa ritardo AriaII", machine.status_message_size);
               }


                if (is_blow_cycle_ended() > 0) {                
                    // Chiusura valvola aria secondaria, salita asta
                    if (close_secondary_air() > 0) {
                        // Chiusura Aria secondaria e Apertura scarico
                        machine.App.mold_sequence = 78;
                    }
                }

                // calcolo tempo aria secondaria
                if (machine.App.secondary_air_persistence_start > 0)
                    machine.App.secondary_air_persistence_time_ms = xTaskGetTickCount() - machine.App.secondary_air_persistence_start;

                // Pressione raqgiunta aria scondaria
                machine.App.secondary_air_press = machine.actuator[BLOW_PRESSURE].cur_rpos;


            } else if (machine.App.mold_sequence == 77) {

                //////////////////////////////////////////////////////////
                // Fine soffiaggio : su asta e scarico aria
                //
                if (is_blow_cycle_ended() > 0) {                
                    // Chiusura valvola aria secondaria, salita asta
                    if (close_secondary_air() > 0) {
                        // Chiusura Aria secondaria e Apertura scarico
                        machine.App.mold_sequence = 78;
                    }
                } else {
                    snprintf(machine.status_message, machine.status_message_size, (char*)"Attesa Fine ciclo %0.2f/%0.2f", (float)machine.App.mold_cycle_time_ms / 1000.0f, (float)(machine.App.production_cycle_time_ms - machine.workSet.discharge_air_time_ms)/1000.0f );
                }

                // calcolo tempo aria secondaria
                if (machine.App.secondary_air_persistence_start > 0)
                    machine.App.secondary_air_persistence_time_ms = xTaskGetTickCount() - machine.App.secondary_air_persistence_start;

                // Pressione raqgiunta aria scondaria
                machine.App.secondary_air_press = machine.actuator[BLOW_PRESSURE].cur_rpos;





                // Chiusura Aria secondaria e Apertura scarico
            } else if (machine.App.mold_sequence == 78) {

                // Attesa chiusura secondaria primaria
                if (machine.actuator[SECONDARY_AIR].position == OFF) {

                    // Apertura valvola di Scarico
                    if (open_discharge_air() > 0) {
                        
                        machine.App.mold_sequence = 79;
                    
                        // Inizio calcolo scarico tempo aria necessario
                        machine.App.discharge_needed_start_time_ms = xTaskGetTickCount();
                        
                    }

                } else {
                    strncpy(machine.status_message, (char*)"Attesa chiususa AriaS", machine.status_message_size);
                }

                // calcolo tempo aria secondaria
                if (machine.App.secondary_air_persistence_start > 0)
                    machine.App.secondary_air_persistence_time_ms = xTaskGetTickCount() - machine.App.secondary_air_persistence_start;

                // Pressione raqgiunta aria scondaria
                machine.App.secondary_air_press = machine.actuator[BLOW_PRESSURE].cur_rpos;



            } else if (machine.App.mold_sequence == 79) {


                // Comando Salita Asta
                if (up_stretch_rod() > 0) {
                    machine.App.mold_sequence = 80;
                    machine.App.mold_sequence_start_time = 0;
                } else {
                    strncpy(machine.status_message, (char*)"Attesa CMD Aste su", machine.status_message_size);                
                }


                // Fine calcolo durata aria II
                machine.App.secondary_air_persistence_start = 0;

                // Calcolo tempo necessatio scarico aria
                if (machine.App.discharge_needed_start_time_ms > 0) {
                    machine.App.discharge_needed_time_ms = xTaskGetTickCount() - machine.App.discharge_needed_start_time_ms;
                    if (machine.actuator[BLOW_PRESSURE].cur_rpos <= machine.workSet.max_pressure_in_mold) {
                        machine.App.discharge_needed_start_time_ms = 0;
                    }
                }






            } else if (machine.App.mold_sequence == 80) {

                // Attesa Salita Asta
                if (machine.actuator[STRETCH].position == STRETCH_UP) {

                    if (machine.App.mold_cycle_time_ms > machine.App.production_cycle_time_ms) {
                        // Fine ciclo di soffiaggio : Asta alta in ritardo
                        snprintf(machine.status_message, machine.status_message_size, (char*)"Aste stiro in ritardo:%dms", machine.App.mold_cycle_time_ms - machine.App.production_cycle_time_ms);
                    }


                    if (machine.actuator[STRETCH].pTrace) {
                        // fprintf(stdout, "[TRK:%d-(%0.2f,%0.2f,%0.2f)]", machine.actuator[STRETCH].pTrace->num_data, machine.actuator[STRETCH].pTrace->pos[0], machine.actuator[STRETCH].pTrace->pos[machine.actuator[STRETCH].pTrace->num_data / 2], machine.actuator[STRETCH].pTrace->pos[machine.actuator[STRETCH].pTrace->num_data - 1]);
                    }


                    // Statistiche                
                    machine.statistic.bottles++;
                    machine.rt_statistic.bottles++;

                    machine.App.mold_sequence = 90;

                } else {
                    if (machine.App.mold_sequence_start_time == 0) {
                        machine.App.mold_sequence_start_time = GLCurTimeMs;
                    } else if (GLCurTimeMs - machine.App.mold_sequence_start_time >= MAX_WAITING_MOLD_SEQUENCE_MS) {
                        snprintf((char*) str, sizeof(str), (char*) "Mold Sequence Timeout at %d", machine.App.mold_sequence);
                        if (generate_alarm((char*) str, 7773, MOLD, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    } else {
                        strncpy(machine.status_message, (char*)"Attesa Aste su", machine.status_message_size);                
                    }
                }

                
                // Calcolo tempo necessatio scarico aria
                if (machine.App.discharge_needed_start_time_ms > 0) {
                    machine.App.discharge_needed_time_ms = xTaskGetTickCount() - machine.App.discharge_needed_start_time_ms;
                    if (machine.actuator[BLOW_PRESSURE].cur_rpos <= machine.workSet.max_pressure_in_mold) {
                        machine.App.discharge_needed_start_time_ms = 0;
                    }
                }



            } else if (machine.App.mold_sequence == 90) {

                if (machine.App.transferitor_sequence == END_SEQUENCE) {
                    // Attesa ritorno Trasferitore Stampo
                    machine.App.mold_sequence = 100;
                }




            } else if (machine.App.mold_sequence == 100) {

                //////////////////////////////////////////////////
                // Fine sequenza : Verifiche e correzzioni....
                //
                if (check_premature_cicle_end(true, machine.App.mold_sequence) > 0) {
                }            

                machine.App.mold_sequence = END_SEQUENCE;




            } else if (machine.App.mold_sequence == END_SEQUENCE) {
                // Fine sequenza


                if (machine.App.mold_cycle_time_ms >= machine.App.production_cycle_time_ms && machine.App.mold_cycle_time_ms <= machine.App.production_cycle_time_ms+50) {
                    // Ciclo in tolleranza
                    
                    // Contatore cicli stampo
                    machine.rt_statistic.mold_cycles++;
                    machine.statistic.mold_cycles++;
                    machine.App.mold_cycles++;
                    
                } else if (machine.App.mold_cycle_time_ms < machine.App.production_cycle_time_ms - CYCLE_TIME_MS_TOLL_TO_WARNING) {
                    // Ciclo in anticipo
                    if (machine.App.preform_registry[machine.App.BLOW_MOLD_POS] == PREFORM_OK) {
                        snprintf(machine.status_message, machine.status_message_size, (char*)"Cycle time not optimized:%dms.. [Discharge air time:%0.2fs] [Rod up time/Discharge needed:%0.2fs - %0.2fs]"
                                ,(int32_t)(machine.App.production_cycle_time_ms - machine.App.mold_cycle_time_ms)
                                ,(float)machine.workSet.discharge_air_time_ms / 1000.0f
                                ,(float)machine.actuator[STRETCH].time_ms2 / 1000.0f
                                ,(float)machine.App.discharge_needed_time_ms / 1000.0f
                                );
                        // checkDupMode & 1 ->  Controlla la duplicazione per Code, Type, Desc
                        // checkDupMode & 2 ->  Controlla la duplicazione per Code, Type
                        if (generate_alarm((char*) machine.status_message, 7771, MOLD, (int) ALARM_WARNING, 0+2) < 0) {
                        }
                    }
                    
                } else if (machine.App.mold_cycle_time_ms > machine.App.production_cycle_time_ms + CYCLE_TIME_MS_TOLL_TO_WARNING) {
                    // Ciclo in ritardo
                    if (machine.App.preform_registry[machine.App.BLOW_MOLD_POS] == PREFORM_OK) {
                        snprintf(machine.status_message, machine.status_message_size, (char*)"Cycle time too short:%dms.. [Discharge air time:%0.2fs] [Rod up time/Discharge needed:%0.2fs - %0.2fs]"
                                ,(int32_t)(machine.App.mold_cycle_time_ms - machine.App.production_cycle_time_ms)
                                ,(float)machine.workSet.discharge_air_time_ms / 1000.0f
                                ,(float)machine.actuator[STRETCH].time_ms2 / 1000.0f
                                ,(float)machine.App.discharge_needed_time_ms / 1000.0f
                                );
                        strncpy(App.Msg, machine.status_message, App.MsgSize);
                        // checkDupMode & 1 ->  Controlla la duplicazione per Code, Type, Desc
                        // checkDupMode & 2 ->  Controlla la duplicazione per Code, Type
                        if (generate_alarm((char*) App.Msg, 7772, MOLD, (int) ALARM_WARNING, 0+2) < 0) {
                        }
                    }
                }
                
                // Riavvolgimento sequenza
                machine.App.mold_sequence = START_SEQUENCE;


            } else {

                // Sequenza non contigua : incremente fino a trovare il prossimo step
                machine.App.mold_sequence++;
            }







            /*
            ____________________________________________

            Registrazione Traccia degli attuatori
            ____________________________________________                                                                                                                                                                                                                    */

            if (machine.App.mold_sequence == 61) {
                // Avvio traccia stiro
                if (start_track(&machine.actuator[STRETCH], 10, 4 * 1000) <= 0) {
                    if (generate_alarm((char*) "WARNING : track STRETCH failed", 7300, 0, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                }
            }
            if (machine.App.mold_sequence > 61 && machine.App.mold_sequence <= 76) {
                // traccia dello stiro
                if (record_track(&machine.actuator[STRETCH]) < 0) {
                    snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[T]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET); 
                    fprintf(stdout, "%s", App.Msg);
                    // vDisplayMessage(App.Msg);
                    if (end_track(&machine.actuator[STRETCH]) <= 0) {
                        snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[End Track Error]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                    
                    }
                } else {
                    // DEBUG
                    /*
                    if (machine.actuator[STRETCH].pTrace) {
                        if (machine.actuator[STRETCH].pTrace->num_data >= 2) {
                            fprintf(stdout, "[TRK:%d-(%0.2f,%0.2f,%0.2f)]", machine.actuator[STRETCH].pTrace->num_data
                                    , machine.actuator[STRETCH].pTrace->pos[0]
                                    , machine.actuator[STRETCH].pTrace->pos[machine.actuator[STRETCH].pTrace->num_data / 2]
                                    , machine.actuator[STRETCH].pTrace->pos[machine.actuator[STRETCH].pTrace->num_data - 1]
                                    );
                        }
                    } 
                    */               
                }
            }
            if (machine.App.mold_sequence == 80) {
                // fine dello stiro
                if (end_track(&machine.actuator[STRETCH]) <= 0) {
                    snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[End Track Error]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                    
                }
            }


            if (machine.App.mold_sequence == 15) {
                // Avvio traccia stampo
                if (start_track(&machine.actuator[MOLD], 10, 4 * 1000) <= 0) {
                    if (generate_alarm((char*) "WARNING : track MOLD failed", 7310, 0, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                }
            }
            if (machine.App.mold_sequence >= 16 && machine.App.mold_sequence <= 17) {
                // traccia dello stampo
                if (record_track(&machine.actuator[MOLD]) < 0) {
                    snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[T]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                    fprintf(stdout, "%s", App.Msg);
                    // vDisplayMessage(App.Msg);
                    if (end_track(&machine.actuator[MOLD]) < 0) {
                        snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[End Track Error]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                    
                    }
                }
            }
            if (machine.App.mold_sequence == MOULD_SEQ_WAITING_FOR_TRASFERITOR_X) {
                // fine dello stampo
                if (machine.actuator[MOLD].position == OPEN) {            
                    if (end_track(&machine.actuator[MOLD]) < 0) {
                        snprintf((char*) App.Msg, App.MsgSize, (char*) "%s[End Track Error]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                    
                    }
                }
            }








            /*
            ____________________________________________

            Sequenza Ritorno del Trasferitore stampo
            ____________________________________________                                                                                                                                                                                                                                                                                                            */

            if (machine.App.transferitor_sequence == INIT_SEQUENCE) {

                machine.App.transferitor_cycle_time_ms = 0;


            } else if (machine.App.transferitor_sequence == START_SEQUENCE) {

                // Comando dentro traferitore
                if (inside_transferitor() > 0) {
                    machine.App.transferitor_sequence = 21;
                    // Avvio conteggio temporale
                    machine.App.transferitor_cycle_time_ms = 1;
                }


            } else if (machine.App.transferitor_sequence == 21) {
                // Attesa Trasferitore Stampo dentro
                if (machine.actuator[TRANSFERITOR_Y].position == INSIDE) {
                    machine.App.transferitor_sequence = 30;
                    machine.App.transferitor_sequence_start_time = 0;
                }




            } else if (machine.App.transferitor_sequence == 30) {
                // Attesa stampo aperto etc...
                if (machine.App.mold_sequence == MOULD_SEQ_WAITING_FOR_TRASFERITOR_X || machine.App.mold_sequence == 16 || machine.App.mold_sequence == 17) {
                    if (machine.actuator[MOLD].position == OPEN) {
                        if (machine.actuator[BOTTLE_PREF_HOLDER].position == UP) {
                            // Blocco pref/bot su
                            if (machine.actuator[LOAD_UNLOAD_Z].position == UP || machine.actuator[LOAD_UNLOAD_X].position != FRONT) {
                                // Manipolatore alto o NON avanti
                                
                                // Comando traferitore avanti
                                if (front_transferitor() > 0) {
                                    machine.App.transferitor_sequence = 31;
                                }
                            } else {
                                // Attesa : controllo timeput
                                if (machine.App.transferitor_sequence_start_time == 0) {
                                    machine.App.transferitor_sequence_start_time = GLCurTimeMs;
                                } else if (GLCurTimeMs - machine.App.transferitor_sequence_start_time >= MAX_WAITING_MOLD_SEQUENCE_MS) {
                                    snprintf((char*) str, sizeof(str), (char*) "Trasferitor Sequence Timeout at %d", machine.App.mold_sequence);
                                    if (generate_alarm((char*) str, 7774, MOLD, (int) ALARM_ERROR, 0+1) < 0) {
                                    }
                                } else {
                                    strncpy(machine.status_message, (char*)"Attesa MANIP_Z Alto", machine.status_message_size);
                                }
                            }
                        } else {
                            // Attesa : controllo timeput
                            if (machine.App.transferitor_sequence_start_time == 0) {
                                machine.App.transferitor_sequence_start_time = GLCurTimeMs;
                            } else if (GLCurTimeMs - machine.App.transferitor_sequence_start_time >= MAX_WAITING_MOLD_SEQUENCE_MS) {
                                snprintf((char*) str, sizeof(str), (char*) "Trasferitor Sequence Timeout at %d", machine.App.mold_sequence);
                                if (generate_alarm((char*) str, 7770, MOLD, (int) ALARM_ERROR, 0+1) < 0) {
                                }
                            } else {
                                strncpy(machine.status_message, (char*)"Attesa BOTT/PREF HOLDER Alto", machine.status_message_size);
                            }
                        }
                    }
                }

            } else if (machine.App.transferitor_sequence == 31) {
                // Attesa Trasferitore Stampo avanti
                if (machine.actuator[TRANSFERITOR_X].position == FRONT) {
                    machine.App.transferitor_sequence = 40;
                }


            } else if (machine.App.transferitor_sequence == 40) {

                // Attesa stampo chiuso e blocco bott./pref. basso 
                // La eequenza verra sbloccata dallo stampo




                // Sequenza Ritorno del Trasferitore stampo
            } else if (machine.App.transferitor_sequence == START_TRANSFERITOR_RETURN_SEQUENCE) {
                // Comando Trasferitore Stampo fuori
                if (outside_transferitor() > 0) {
                    machine.App.transferitor_sequence = 51;
                }



            } else if (machine.App.transferitor_sequence == 51) {
                // Attesa Trasferitore Stampo fuori
                if (machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) {
                    machine.App.transferitor_sequence = 60;
                }



            } else if (machine.App.transferitor_sequence == 60) {
                // Comando Trasferitore Stampo indietro
                if (back_transferitor() > 0) {
                    machine.App.transferitor_sequence = 61;
                }


            } else if (machine.App.transferitor_sequence == 61) {
                // Attesa Trasferitore Stampo indietro
                if (machine.actuator[TRANSFERITOR_X].position == BACK) {
                    // Trasferitore indietro : fine sequenza
                    machine.App.transferitor_sequence = END_SEQUENCE;
                }

            } else if (machine.App.transferitor_sequence == END_SEQUENCE) {

            } else {
                machine.App.transferitor_sequence++;
            }









            /*
            __________________________________

            Sequenza del Trasferitore Preforme
            __________________________________                                                                                                                                                                                                                                                                                                              */

            if (machine.App.manip_pref_sequence == INIT_SEQUENCE) {

                machine.App.manip_pref_cycle_time_ms = 0;

                // Controllo posizioni iniziali
                if (machine.actuator[TRANSFERITOR_Y].target_position != OUTSIDE) {
                    // Trasferitore NON fuori
                } else {
                    if (machine.actuator[TRANSFERITOR_X].target_position != BACK) {
                        // Trasferitore NON Indietro
                    } else {
                        // machine.App.manip_pref_sequence = START_SEQUENCE;
                    }
                }


            } else if (machine.App.manip_pref_sequence == START_SEQUENCE) {

                machine.App.manip_pref_cycle_time_ms = 1;

                // Comando salita manipolatore pref.
                if (up_preform_manipolator() > 0) {

                    // Comando indietro step catena
                    if (back_heating_chain() > 0) {

                        machine.App.manip_pref_sequence = 21;
                    }
                }



            } else if (machine.App.manip_pref_sequence == 21) {
                if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                    machine.App.manip_pref_sequence = 30;
                }


            } else if (machine.App.manip_pref_sequence == 30) {
                // Comando indietro manipolatore pref.
                back_preform_manipolator();
                machine.App.manip_pref_sequence = 31;

            } else if (machine.App.manip_pref_sequence == 31) {
                if (machine.actuator[LOAD_UNLOAD_X].position == BACK) {
                    // Attesa manipolatore indietro (verso carico)
                    if (machine.actuator[HEATING_CHAIN1].position == INSIDE) {
                        // Attesa stepper catena indietro
                        machine.App.manip_pref_sequence = 40;
                    }
                }


            } else if (machine.App.manip_pref_sequence == 40) {
                // Comando abbassa manipolatore pref.
                down_preform_manipolator();
                machine.App.manip_pref_sequence = 41;


            } else if (machine.App.manip_pref_sequence == 41) {
                if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                    machine.App.manip_pref_sequence = 50;
                }



            } else if (machine.App.manip_pref_sequence == 50) {
                // Comando unpick manipolatore pref.
                unpick_preform_manipolator();
                machine.App.manip_pref_sequence = 51;



            } else if (machine.App.manip_pref_sequence == 51) {
                if (machine.actuator[LOAD_UNLOAD_PICK].position == OPEN) {
                    machine.App.manip_pref_sequence = 60;
                }




            } else if (machine.App.manip_pref_sequence == 60) {
                // Comando salita manipolatore pref.
                up_preform_manipolator();
                machine.App.manip_pref_sequence = 61;



            } else if (machine.App.manip_pref_sequence == 61) {
                if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                    machine.App.manip_pref_sequence = 70;
                }




            } else if (machine.App.manip_pref_sequence == 70) {
                // Comando avanti manipolatore pref. (verso stampo)
                front_preform_manipolator();

                // Comando Avanzamento Catena riscaldamento
                step_heating_chain();

                machine.App.manip_pref_sequence = 71;



            } else if (machine.App.manip_pref_sequence == 71) {

                if (machine.actuator[LOAD_UNLOAD_X].position == FRONT) {

                    // Esecuzione shift registro di carico preforme
                    do_preform_registry_shift();

                    // Carica il buffer preforma
                    if (machine.App.load_on) {
                        machine.App.preform_registry[0] = PREFORM_LOADED;
                    }

                    read_preform_temperature();

                    machine.statistic.preforms_loaded++;
                    machine.rt_statistic.preforms_loaded++;
                    

                    machine.App.manip_pref_sequence = 80;
                }



            } else if (machine.App.manip_pref_sequence == 80) {
                // Comando abbassa manipolatore pref.
                down_preform_manipolator();
                machine.App.manip_pref_sequence = 81;



            } else if (machine.App.manip_pref_sequence == 81) {
                if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                    machine.App.manip_pref_sequence = 90;
                }


            } else if (machine.App.manip_pref_sequence == 90) {
                // Comando pick manipolatore pref.
                pick_preform_manipolator();
                machine.App.manip_pref_sequence = 91;



            } else if (machine.App.manip_pref_sequence == 91) {
                if (machine.actuator[LOAD_UNLOAD_PICK].position == CLOSE) {
                    machine.App.manip_pref_sequence = END_SEQUENCE;
                }



            } else if (machine.App.manip_pref_sequence == END_SEQUENCE) {
                // Fine sequenza

                if (machine.App.mold_sequence == MOULD_SEQ_WAITING_FOR_CHAIN_STEP) {
                    // Riavvolgimento sequanza stampo poich� sta attendendo questa sequenza
                    machine.App.mold_sequence = START_SEQUENCE;
                }


            } else {
                // Sequenza non contigua
                machine.App.manip_pref_sequence++;
            }













            /*
            __________________________________

            Sequenza del carico scarico
            __________________________________                                                                                                                                                                                                                                                                                                              */

            if (machine.App.load_sequence == INIT_SEQUENCE) {

                machine.App.load_cycle_time = 0;


            } else if (machine.App.load_sequence == START_SEQUENCE) {

                machine.App.load_cycle_time = 1;

                if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                    if (machine.actuator[LOAD_UNLOAD_X].position == FRONT) {
                        // Comando discesa manipolatore carico scarico
                        down_preform_manipolator();
                        machine.App.load_sequence = 20;
                    } else {
                        // Trasferitore carico scarico NON AVANTI
                    }
                } else {
                    // Trasferitore carico scarico NON ALTO
                }


            } else if (machine.App.load_sequence == 20) {
                // Attesa discesa manip
                if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                    machine.App.load_sequence = 30;
                }

            } else if (machine.App.load_sequence == 30) {
                // Comando chiusura pinza
                pick_preform_manipolator();
                machine.App.load_sequence = 40;

            } else if (machine.App.load_sequence == 40) {
                // Attesa chiusura pinza
                if (machine.actuator[LOAD_UNLOAD_PICK].position == CLOSE) {
                    machine.App.load_sequence = 50;
                }

            } else if (machine.App.load_sequence == 50) {
                // Comando salita manipolatore
                up_preform_manipolator();

                machine.App.load_sequence = 60;


            } else if (machine.App.load_sequence == 60) {
                // Attesa salita manipolatore
                if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                    machine.App.load_sequence = 70;
                }


            } else if (machine.App.load_sequence == 70) {
                // Comando indietro manipolatore
                // Verificare se l'avanzatore catena � indietro
                back_preform_manipolator();
                machine.App.load_sequence = 80;



            } else if (machine.App.load_sequence == 80) {
                // Attesa indietro manipolatore
                if (machine.actuator[LOAD_UNLOAD_X].position == BACK) {
                    machine.App.load_sequence = 90;
                }


            } else if (machine.App.load_sequence == 90) {
                // Comando discesa manipolatore
                down_preform_manipolator();
                machine.App.load_sequence = 100;


            } else if (machine.App.load_sequence == 100) {
                // Attesa gi�  manipolatore
                if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {

                    // Assegnamento stato preforma caricata
                    machine.App.preform_registry[0] = PREFORM_LOADED;

                    machine.App.load_sequence = 110;
                }



            } else if (machine.App.load_sequence == 110) {
                // Comando apertura pinza manipolatore
                unpick_preform_manipolator();
                machine.App.load_sequence = 120;


            } else if (machine.App.load_sequence == 120) {
                // Attesa pinza aperta
                if (machine.actuator[LOAD_UNLOAD_PICK].position == OPEN) {
                    machine.App.load_sequence = 130;
                }


            } else if (machine.App.load_sequence == 130) {
                // Comando salita manipolatore
                up_preform_manipolator();
                machine.App.load_sequence = 140;


            } else if (machine.App.load_sequence == 140) {
                // Attesa salita manipolatore
                if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                    machine.App.load_sequence = 150;
                }



            } else if (machine.App.load_sequence == 150) {
                // Comando avanti manipolatore
                front_preform_manipolator();
                machine.App.load_sequence = 160;


            } else if (machine.App.load_sequence == 160) {
                // Attesa avanti manipolatore
                if (machine.actuator[LOAD_UNLOAD_X].position == FRONT) {
                    machine.App.load_sequence = END_SEQUENCE;
                }


                /*

                } else if (machine.App.load_sequence == 170) {
                // Comando discesa manipolatore
                down_preform_manipolator();
                machine.App.load_sequence = 180;


                } else if (machine.App.load_sequence == 180) {
                // Attesa discesa manipolatore
                if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                        machine.actuator[LOAD_UNLOAD_Z].step = STEP_READY;
                        machine.App.load_sequence = 190;
                        }



                } else if (machine.App.load_sequence == 190) {
                // Comando chiusura pinza manipolatore
                pick_preform_manipolator()
                machine.actuator[LOAD_UNLOAD_PICK].step = STEP_SEND_CMD;
                machine.App.load_sequence = 200;


                } else if (machine.App.load_sequence == 200) {
                // Attesa pinza aperta
                if (machine.actuator[LOAD_UNLOAD_PICK].position == CLOSE) {
                        machine.App.load_sequence = END_SEQUENCE;
                        }
                 */



            } else if (machine.App.load_sequence == END_SEQUENCE) {
                // Fine sequenza

                machine.App.load_sequence = 0;


                // Partenza sequenza sblocco pista
                machine.App.pit_lock_sequence = START_SEQUENCE;


            } else {
                // Sequenza non contigua
                machine.App.load_sequence++;
            }





            /*
            __________________________________

            Sequenza dell' espulore bottiglia
            __________________________________                                                                                                                                                                                                                                                                                                              */

            if (machine.App.bottle_eject_sequence == INIT_SEQUENCE) {

            } else if (machine.App.bottle_eject_sequence == START_SEQUENCE) {
                machine.App.bottle_eject_sequence = 20;

            } else if (machine.App.bottle_eject_sequence == 20) {
                machine.App.bottle_eject_sequence = END_SEQUENCE;

            } else if (machine.App.bottle_eject_sequence == END_SEQUENCE) {
                // Fine sequenza

                machine.App.bottle_eject_sequence = 0;
                machine.App.bottle_eject_cycle_time = 0;


            } else {
                // Sequenza non contigua
                machine.App.bottle_eject_sequence++;
            }



            /*
            __________________________________

            Sequenza del blocco/sblocco pista
            __________________________________                                                                                                                                                                                                                                                                                                              */

            if (machine.App.pit_lock_sequence == INIT_SEQUENCE) {
                machine.App.pit_lock_cycle_time_ms = 0;

            } else if (machine.App.pit_lock_sequence == START_SEQUENCE) {
                // Comando SBlocco pista
                machine.actuator[PIT_LOCK].target_position = INSIDE;
                machine.actuator[PIT_LOCK].step = STEP_SEND_CMD;
                machine.App.pit_lock_sequence = 20;

                machine.App.pit_lock_cycle_time_ms = 10;



            } else if (machine.App.pit_lock_sequence == 20) {
                // Attesa SBlocco pista
                if (machine.actuator[PIT_LOCK].position == machine.actuator[PIT_LOCK].target_position) {
                    machine.App.pit_lock_sequence = 30;
                }


            } else if (machine.App.pit_lock_sequence == 30) {
                // Attesa Tempo sBlocco pista
                if (GLCurTimeMs - machine.actuator[PIT_LOCK].start_time >= machine.workSet.pit_unlock_time_ms) {
                    machine.App.pit_lock_sequence = 40;
                }

            } else if (machine.App.pit_lock_sequence == 40) {
                // Comando Blocco pista
                machine.actuator[PIT_LOCK].target_position = OUTSIDE;
                machine.actuator[PIT_LOCK].step = STEP_SEND_CMD;
                machine.App.pit_lock_sequence = 50;


            } else if (machine.App.pit_lock_sequence == 50) {
                // Attesa Blocco pista
                if (machine.actuator[PIT_LOCK].position == machine.actuator[PIT_LOCK].target_position) {
                    machine.actuator[PIT_LOCK].step = STEP_READY;
                    machine.App.pit_lock_sequence = END_SEQUENCE;
                }

            } else if (machine.App.pit_lock_sequence == END_SEQUENCE) {
                // Fine sequenza

                machine.App.pit_lock_sequence = 0;

            } else {
                // Sequenza non contigua
                machine.App.pit_lock_sequence++;
            }





            /*
            __________________________________

                Sequenza dei forni
            __________________________________                                                                                                                                                                                                                                                                                                              */

            if (machine.App.owens_sequence == INIT_SEQUENCE) {

            } else if (machine.App.owens_sequence == START_SEQUENCE) {
                // Comando Acensione forni, ventilazione, aspirazione

                machine.actuator[OWENS].target_position = OUTSIDE;
                machine.actuator[OWENS].step = STEP_SEND_CMD;


                machine.actuator[VENTILATOR].target_position = OUTSIDE;
                machine.actuator[VENTILATOR].step = STEP_SEND_CMD;


                machine.actuator[ASPIRATOR].target_position = OUTSIDE;
                machine.actuator[ASPIRATOR].step = STEP_SEND_CMD;


                machine.actuator[OWENS_TEMPERATURE1].target_position = OUTSIDE;
                machine.actuator[OWENS_TEMPERATURE1].step = STEP_SEND_CMD;

                machine.actuator[OWENS_TEMPERATURE2].target_position = OUTSIDE;
                machine.actuator[OWENS_TEMPERATURE2].step = STEP_SEND_CMD;



                machine.App.owens_sequence = 20;


            } else if (machine.App.owens_sequence == 20) {
                // Preriscaldamento
                if (machine.actuator[OWENS_TEMPERATURE1].position == machine.actuator[OWENS_TEMPERATURE1].target_position) {
                    if (machine.actuator[OWENS_TEMPERATURE2].position == machine.actuator[OWENS_TEMPERATURE2].target_position) {
                        machine.App.owens_sequence = 30;
                    }
                }


            } else if (machine.App.owens_sequence == 30) {
                // Rampa riscaldamento
                // machine.work_param.global_heat_ratio1;

                if (machine.actuator[OWENS].position == machine.actuator[OWENS].target_position) {
                    machine.actuator[OWENS].step = STEP_READY;
                    machine.App.owens_sequence = END_SEQUENCE;
                } else {
                }


                if (machine.App.mold_sequence == END_SEQUENCE) {
                    machine.App.owens_cycles++;
                }


            } else if (machine.App.owens_sequence == END_SEQUENCE) {
                // Fine sequenza

                machine.App.owens_sequence = 0;
                machine.App.owens_cycle_time_ms = 0;


            } else {
                // Sequenza non contigua
                machine.App.bottle_eject_sequence++;
            }



            ////////////////////////////////////////////////////
            // Aggiornamento contatori temporali
            //

            {
                if (machine.App.mold_cycle_time_ms > 0 && machine.App.mold_sequence != END_SEQUENCE) machine.App.mold_cycle_time_ms += REAL_TASK_TIME;
                if (machine.App.manip_pref_cycle_time_ms > 0 && machine.App.manip_pref_sequence != END_SEQUENCE) machine.App.manip_pref_cycle_time_ms += REAL_TASK_TIME;
                if (machine.App.transferitor_cycle_time_ms > 0 && machine.App.transferitor_sequence != END_SEQUENCE) machine.App.transferitor_cycle_time_ms += REAL_TASK_TIME;
                if (machine.App.pit_lock_cycle_time_ms > 0 && machine.App.pit_lock_sequence != END_SEQUENCE) machine.App.pit_lock_cycle_time_ms += REAL_TASK_TIME;
                if (machine.App.load_cycle_time > 0 && machine.App.load_sequence != END_SEQUENCE) machine.App.load_cycle_time += REAL_TASK_TIME;


                snprintf(machine.time_message, machine.time_message_size, "[%d msec]", REAL_TASK_TIME);
            }


        } else {
            // Stato macchina non valido
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






////////////////////////////////////////////////
// Controllo Comando apertura stampo
//
int check_open_mold_condition() {
    char str[256];
    if (machine.actuator[BLOW_PRESSURE].cur_rpos <= machine.workSet.max_pressure_in_mold) {
        if (machine.actuator[STRETCH].position == STRETCH_UP) { // aste alte
            if (machine.actuator[DISCHARGE_AIR].position == OFF) { // scarico aperto
                if (machine.actuator[PRIMARY_AIR].position == OFF) { // aria I� chiusa
                    if (machine.actuator[SECONDARY_AIR].position == OFF) { // aria II� chiusa
                        return 1;
                    } else {
                        if (!machine.actuator[MOLD].error) {
                            machine.actuator[MOLD].error = 7900;
                            if (generate_alarm((char*) "SECONDARY_AIR not closed", 7010, 0, (int) ALARM_WARNING, 0+1) < 0) {
                            }
                        }
                    }
                } else {
                    if (!machine.actuator[MOLD].error) {
                        machine.actuator[MOLD].error = 7900;
                        if (generate_alarm((char*) "PRIMARY_AIR not closed", 7011, 0, (int) ALARM_WARNING, 0+1) < 0) {
                        }
                    }
                }
            } else {
                if (!machine.actuator[MOLD].error) {
                    machine.actuator[MOLD].error = 7900;
                    if (generate_alarm((char*) "DISCHARGE AIR not open", 7012, 0, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                }
            }
        } else {
            if (!machine.actuator[MOLD].error) {
                machine.actuator[MOLD].error = 7900;
                if (generate_alarm((char*) "STRETCH not UP", 7013, 0, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    } else {
        if (!machine.actuator[MOLD].error) {
            machine.actuator[MOLD].error = 7900;
            snprintf(str, sizeof (str), "BLOW_PRESSURE not zero (%0.3f)", machine.actuator[BLOW_PRESSURE].cur_rpos);
            if (generate_alarm((char*) str, 7014, 0, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
    
}

//////////////////////////////////////////
// Comando Homing stampo
//
int home_mold() {
    char str[256];
    
    if (check_open_mold_condition()) {
        if (machine.actuator[MOLD].step == STEP_READY) {
            machine.actuator[MOLD].step = STEP_SEND_HOMING;
            return 1;
        } else {
            if (!machine.actuator[MOLD].error) {
                machine.actuator[MOLD].error = 7900;
                snprintf((char*) str, sizeof(str), (char*) "MOLD Actuator not ready for homing(%s:%0.3f/%0.3f)"
                        , get_actuator_step(&machine.actuator[MOLD])
                        , machine.actuator[MOLD].cur_rpos
                        , machine.actuator[MOLD].end_rpos ); 
                if (generate_alarm((char*) str, 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    }
    return 0;
}

////////////////////////////////////////////////
// Comando apertura stampo
//
int open_mold() {
    char str[256];
    
    if (check_open_mold_condition()) {                        
        if (machine.actuator[MOLD].step == STEP_READY) {
            machine.actuator[MOLD].target_position = OPEN;
            machine.actuator[MOLD].step = STEP_SEND_CMD;
            return 1;
        } else {
            if (!machine.actuator[MOLD].error) {
                machine.actuator[MOLD].error = 7900;
                snprintf((char*) str, sizeof(str), (char*) "MOLD Actuator not ready for open(%s:%0.3f/%0.3f)"
                        , get_actuator_step(&machine.actuator[MOLD])
                        , machine.actuator[MOLD].cur_rpos
                        , machine.actuator[MOLD].end_rpos ); 
                if (generate_alarm((char*) str, 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    }
    return 0;
}




//////////////////////////////////////////
// Comando chiusura stampo stampo
//

int close_mold() {
    char str[256];
    if (machine.actuator[TRANSFERITOR_Y].position == OUTSIDE
            || (machine.actuator[TRANSFERITOR_Y].position == INSIDE && machine.actuator[TRANSFERITOR_X].position == FRONT)
            || (machine.actuator[TRANSFERITOR_Y].position == INSIDE && machine.actuator[TRANSFERITOR_X].position == BACK)) {

        if (machine.actuator[STRETCH].position == STRETCH_UP) { // aste alte
            if (machine.actuator[MOLD].step == STEP_READY) {
                machine.actuator[MOLD].target_position = CLOSE;
                machine.actuator[MOLD].step = STEP_SEND_CMD;
                return 1;
            } else {
                if (!machine.actuator[MOLD].error) {
                    machine.actuator[MOLD].error = 7900;
                    snprintf((char*) str, sizeof(str), (char*) "MOLD Actuator not ready for close(%s:%0.3f/%0.3f)"
                            , get_actuator_step(&machine.actuator[MOLD])
                            , machine.actuator[MOLD].cur_rpos
                            , machine.actuator[MOLD].start_rpos ); 
                    if (generate_alarm((char*) str, 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                }
            }

        } else {
            // Trasferitore NON in posizione
            if (!machine.actuator[MOLD].error) {
                machine.actuator[MOLD].error = 7900;
                if (generate_alarm((char*) "STRETCH NOT UP", 7020, STRETCH, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    } else {
        // Trasferitore NON in posizione
        if (!machine.actuator[MOLD].error) {
            machine.actuator[MOLD].error = 7900;
            if (generate_alarm((char*) "TRANSFERITOR_Y out of position", 7021, TRANSFERITOR_Y, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}






//////////////////////////////////////////
// Comando Homing Aste
//
int home_stretch_rod() {
    char str[256];
    if (machine.actuator[STRETCH].step == STEP_READY) {
        machine.actuator[STRETCH].step = STEP_SEND_HOMING;
        return 1;
    } else {
        if (!machine.actuator[STRETCH].error) {
            machine.actuator[STRETCH].error = 7900;
            snprintf((char*) str, sizeof(str), (char*) "STRETCH Actuator not ready for homing (%s:%0.3f/%0.3f) (reads:%d) (Pos:%s - Tgt:%s) SEQ:%d"
                    , get_actuator_step(&machine.actuator[STRETCH])
                    , machine.actuator[STRETCH].cur_rpos
                    , machine.actuator[STRETCH].start_rpos 
                    , machine.actuator[STRETCH].readCounter
                    , (char*)(machine.actuator[STRETCH].position == STRETCH_UP ? "UP" : "DOWN"), (char*)(machine.actuator[STRETCH].target_position == STRETCH_UP ? "UP" : "DOWN")
                    , machine.App.mold_sequence ); 
            vDisplayMessage(App.Msg);
            if (generate_alarm((char*) str, 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}


//////////////////////////////////////////
// Comando Salita Aste
//
int up_stretch_rod() {
    char str[256];
    if (machine.actuator[STRETCH].step == STEP_READY) {
        machine.actuator[STRETCH].target_position = STRETCH_UP;
        machine.actuator[STRETCH].step = STEP_SEND_CMD;
        return 1;
    } else {
        if (!machine.actuator[STRETCH].error) {
            machine.actuator[STRETCH].error = 7900;
            snprintf((char*) str, sizeof(str), (char*) "STRETCH Actuator not ready for up (%s:%0.3f/%0.3f) (reads:%d) (Pos:%s - Tgt:%s) SEQ:%d"
                    , get_actuator_step(&machine.actuator[STRETCH])
                    , machine.actuator[STRETCH].cur_rpos
                    , machine.actuator[STRETCH].start_rpos 
                    , machine.actuator[STRETCH].readCounter
                    , (char*)(machine.actuator[STRETCH].position == STRETCH_UP ? "UP" : "DOWN"), (char*)(machine.actuator[STRETCH].target_position == STRETCH_UP ? "UP" : "DOWN")
                    , machine.App.mold_sequence ); 
            vDisplayMessage(App.Msg);
            if (generate_alarm((char*) str, 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}


//////////////////////////////////////////
// Comando Discesa Aste
//

int down_stretch_rod() {
    char str[256];
    if (machine.actuator[MOLD].position == CLOSE || machine.status == MANUAL) {
        if (machine.actuator[STRETCH].step == STEP_READY) {
            machine.actuator[STRETCH].target_position = STRETCH_DOWN;
            machine.actuator[STRETCH].step = STEP_SEND_CMD;
            return 1;
        } else {
            if (!machine.actuator[STRETCH].error) {
                machine.actuator[STRETCH].error = 7900;
                snprintf((char*) str, sizeof(str), (char*) "STRETCH Actuator not ready for down(%s:%0.3f/%0.3f)  (reads:%d)  (Pos:%d - Tgt:%d) SEQ:%d"
                        , get_actuator_step(&machine.actuator[STRETCH])
                        , machine.actuator[STRETCH].cur_rpos
                        , machine.actuator[STRETCH].end_rpos
                        , machine.actuator[STRETCH].readCounter
                        , machine.actuator[STRETCH].position, machine.actuator[STRETCH].target_position
                        , machine.App.mold_sequence );
                vDisplayMessage(App.Msg);
                if (generate_alarm((char*) str, 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    } else {
        // Stampo NON chiuso
        if (!machine.actuator[MOLD].error) {
            machine.actuator[MOLD].error = 7900;
            if (generate_alarm((char*) "MOLD not closed", 7030, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}






//////////////////////////////////////////
// Controllo Comando Trasferitore stampo
//

int check_transferitor_condition() {    
    
    if (machine.actuator[MOLD].position == OPEN || machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) { // Stampo apero o Trasf Fuori        
        if (machine.actuator[STRETCH].position == STRETCH_UP || machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) { // aste alte o Trasf Fuori
            if (machine.actuator[BOTTLE_PREF_HOLDER].position == UP || machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) { // Holder alto o Trasf Fuori
                if (machine.actuator[LOAD_UNLOAD_Z].position == UP || machine.actuator[TRANSFERITOR_Y].position == OUTSIDE
                        || (machine.actuator[LOAD_UNLOAD_Z].position == DOWN && machine.actuator[LOAD_UNLOAD_X].position == BACK) // Manip. Pref. Z alto o indietro verso carico 
                        ) { // Mani. pref. Z alto o Trasf Fuori
                    return 1;
                } else {
                    // Manipolatore Z ( Alto/Basso Trasferitore catena) non alto e non verso carico
                    if (!machine.actuator[TRANSFERITOR_X].error) {
                        machine.actuator[TRANSFERITOR_X].error = 7900;
                        if (generate_alarm((char*) "LOAD_UNLOAD_Z not up/LOAD_UNLOAD_X not back", 7043, LOAD_UNLOAD_Z, (int) ALARM_WARNING, 0+1) < 0) {                        
                        }
                    }
                }
            } else {
                // blocco pref. non alto
                if (!machine.actuator[TRANSFERITOR_X].error) {
                    machine.actuator[TRANSFERITOR_X].error = 7900;
                    if (generate_alarm((char*) "BOTTLE_PREF_HOLDER not up/TRANSFERITOR_Y not outside", 7042, BOTTLE_PREF_HOLDER, (int) ALARM_WARNING, 0+1) < 0) {                        
                    }
                }
            }
        } else {
            // aste non alte
            if (!machine.actuator[TRANSFERITOR_X].error) {
                machine.actuator[TRANSFERITOR_X].error = 7900;
                if (generate_alarm((char*) "STRETCH not up/TRANSFERITOR_Y not outside", 7041, STRETCH, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    } else {
        // Stampo NON chiuso
        if (!machine.actuator[TRANSFERITOR_X].error) {
            machine.actuator[TRANSFERITOR_X].error = 7900;
            if (generate_alarm((char*) "MOLD not open/TRANSFERITOR_Y not outside", 7040, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }      
    return 0;
}

int home_transferitor() {
    if (check_transferitor_condition() > 0) {
        if (machine.actuator[TRANSFERITOR_X].step == STEP_READY) {
            machine.actuator[TRANSFERITOR_X].step = STEP_SEND_HOMING;
            return 1;
        } else {
            if (!machine.actuator[TRANSFERITOR_X].error) {
                machine.actuator[TRANSFERITOR_X].error = 7900;
                if (generate_alarm((char*) "TRANSFERITOR_X Actuator not ready for homing", 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    }
    return 0;
}



int front_transferitor() {
    if (check_transferitor_condition() > 0) {
        if (machine.actuator[TRANSFERITOR_X].step == STEP_READY) {
            machine.actuator[TRANSFERITOR_X].target_position = FRONT;
            machine.actuator[TRANSFERITOR_X].step = STEP_SEND_CMD;
            return 1;
        } else {
            if (!machine.actuator[TRANSFERITOR_X].error) {
                machine.actuator[TRANSFERITOR_X].error = 7900;
                if (generate_alarm((char*) "TRANSFERITOR_X Actuator not ready", 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    }
    return 0;
}

int back_transferitor() {
    if (machine.actuator[TRANSFERITOR_Y].position == OUTSIDE
            || (machine.actuator[MOLD].position == OPEN && machine.actuator[TRANSFERITOR_Y].position == INSIDE)) {
        if (machine.actuator[BOTTLE_PREF_HOLDER].position == UP || machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) {
            
            if (machine.actuator[TRANSFERITOR_X].step == STEP_READY) {
                machine.actuator[TRANSFERITOR_X].target_position = BACK;
                machine.actuator[TRANSFERITOR_X].step = STEP_SEND_CMD;
                return 1;
            } else {
                if (!machine.actuator[TRANSFERITOR_X].error) {
                    machine.actuator[TRANSFERITOR_X].error = 7900;
                    if (generate_alarm((char*) "TRANSFERITOR_X Actuator not ready", 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                }
            }
        } else {
            // blocco pref. non alto
            if (!machine.actuator[TRANSFERITOR_X].error) {
                machine.actuator[TRANSFERITOR_X].error = 7052;
                if (generate_alarm((char*) "BOTTLE_PREF_HOLDER not up", 7052, BOTTLE_PREF_HOLDER, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    } else {
        // Stampo NON chiuso
        if (!machine.actuator[TRANSFERITOR_X].error) {
            machine.actuator[TRANSFERITOR_X].error = 7050;
            if (generate_alarm((char*) "MOLD not open/TRANSFERITOR_Y not outside", 7050, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}

int inside_transferitor() {
    if (machine.actuator[TRANSFERITOR_X].position == BACK || machine.actuator[TRANSFERITOR_X].position == FRONT) {
        if (machine.actuator[TRANSFERITOR_Y].step == STEP_READY) {
            machine.actuator[TRANSFERITOR_Y].target_position = INSIDE;
            machine.actuator[TRANSFERITOR_Y].step = STEP_SEND_CMD;
            return 1;
        } else {
            if (!machine.actuator[TRANSFERITOR_Y].error) {
                machine.actuator[TRANSFERITOR_Y].error = 7900;
                if (generate_alarm((char*) "TRANSFERITOR_Y Actuator not ready", 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    } else {
        // Trasferitore non in posione
        if (generate_alarm((char*) "TRANSFERITOR_X not back/front", 7060, TRANSFERITOR_X, (int) ALARM_WARNING, 0+1) < 0) {
        }
    }
    return 0;
}

int outside_transferitor() {
    if (machine.actuator[TRANSFERITOR_Y].step == STEP_READY) {
        machine.actuator[TRANSFERITOR_Y].target_position = OUTSIDE;
        machine.actuator[TRANSFERITOR_Y].step = STEP_SEND_CMD;
        return 1;
    } else {
        if (!machine.actuator[TRANSFERITOR_Y].error) {
            machine.actuator[TRANSFERITOR_Y].error = 7900;
            if (generate_alarm((char*) "TRANSFERITOR_Y Actuator not ready", 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}

int down_holding_station() {
    if (machine.actuator[TRANSFERITOR_Y].target_position == OUTSIDE
            || (machine.actuator[TRANSFERITOR_Y].target_position == INSIDE && (machine.actuator[TRANSFERITOR_X].position == BACK || machine.actuator[TRANSFERITOR_X].position == FRONT))) {
        if (machine.actuator[BOTTLE_PREF_HOLDER].step == STEP_READY) {
            machine.actuator[BOTTLE_PREF_HOLDER].target_position = DOWN;
            machine.actuator[BOTTLE_PREF_HOLDER].step = STEP_SEND_CMD;
            return 1;
        } else {
            if (!machine.actuator[BOTTLE_PREF_HOLDER].error) {
                machine.actuator[BOTTLE_PREF_HOLDER].error = 7900;
                if (generate_alarm((char*) "BOTTLE_PREF_HOLDER Actuator not ready", 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        }
    } else {
        // Trasferitore non in posione
        if (!machine.actuator[BOTTLE_PREF_HOLDER].error) {
            machine.actuator[BOTTLE_PREF_HOLDER].error = 7900;
            if (generate_alarm((char*) "TRANSFERITOR_X not back/front", 7060, TRANSFERITOR_X, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}

int up_holding_station() {
    if (machine.actuator[BOTTLE_PREF_HOLDER].step == STEP_READY) {
        machine.actuator[BOTTLE_PREF_HOLDER].target_position = UP;
        machine.actuator[BOTTLE_PREF_HOLDER].step = STEP_SEND_CMD;
        return 1;
    } else {
        if (!machine.actuator[BOTTLE_PREF_HOLDER].error) {
            machine.actuator[BOTTLE_PREF_HOLDER].error = 7900;
            if (generate_alarm((char*) "BOTTLE_PREF_HOLDER Actuator not ready", 7900, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
    }
    return 0;
}

int open_primary_air() {
    machine.actuator[PRIMARY_AIR].target_position = ON;
    machine.actuator[PRIMARY_AIR].step = STEP_SEND_CMD;
    return 1;
}

int close_primary_air() {
    machine.actuator[PRIMARY_AIR].target_position = OFF;
    machine.actuator[PRIMARY_AIR].step = STEP_SEND_CMD;
    return 1;
}

int open_secondary_air() {
    machine.actuator[SECONDARY_AIR].target_position = ON;
    machine.actuator[SECONDARY_AIR].step = STEP_SEND_CMD;
    return 1;
}

int close_secondary_air() {
    machine.actuator[SECONDARY_AIR].target_position = OFF;
    machine.actuator[SECONDARY_AIR].step = STEP_SEND_CMD;
    return 1;
}

int open_discharge_air() {
    machine.actuator[DISCHARGE_AIR].target_position = OFF;
    machine.actuator[DISCHARGE_AIR].step = STEP_SEND_CMD;
    return 1;
}

int close_discharge_air() {
    machine.actuator[DISCHARGE_AIR].target_position = ON;
    machine.actuator[DISCHARGE_AIR].step = STEP_SEND_CMD;
    return 1;
}

int open_recovery_air() {
    machine.actuator[RECOVERY_AIR].target_position = OFF;
    machine.actuator[RECOVERY_AIR].step = STEP_SEND_CMD;
    return 1;
}

int close_recovery_air() {
    machine.actuator[RECOVERY_AIR].target_position = ON;
    machine.actuator[RECOVERY_AIR].step = STEP_SEND_CMD;
    return 1;
}


int down_preform_manipolator() {
    if (machine.actuator[LOAD_UNLOAD_Z].step == STEP_READY) {
        machine.actuator[LOAD_UNLOAD_Z].target_position = DOWN;
        machine.actuator[LOAD_UNLOAD_Z].step = STEP_SEND_CMD;
        return 1;
    } else {
    }
    return 0;
}

int up_preform_manipolator() {
    if (machine.actuator[LOAD_UNLOAD_Z].step == STEP_READY) {
        machine.actuator[LOAD_UNLOAD_Z].target_position = UP;
        machine.actuator[LOAD_UNLOAD_Z].step = STEP_SEND_CMD;
        return 1;
    } else {
    }
    return 0;
}

int pick_preform_manipolator() {
    machine.actuator[LOAD_UNLOAD_PICK].target_position = CLOSE;
    machine.actuator[LOAD_UNLOAD_PICK].step = STEP_SEND_CMD;
    return 1;
}

int unpick_preform_manipolator() {
    machine.actuator[LOAD_UNLOAD_PICK].target_position = OPEN;
    machine.actuator[LOAD_UNLOAD_PICK].step = STEP_SEND_CMD;
    return 1;
}

int front_preform_manipolator() {
    machine.actuator[LOAD_UNLOAD_X].target_position = FRONT;
    machine.actuator[LOAD_UNLOAD_X].step = STEP_SEND_CMD;
    return 1;
}

int back_preform_manipolator() {
    machine.actuator[LOAD_UNLOAD_X].target_position = BACK;
    machine.actuator[LOAD_UNLOAD_X].step = STEP_SEND_CMD;
    return 1;
}

int step_heating_chain() {
    machine.actuator[HEATING_CHAIN1].target_position = OUTSIDE;
    machine.actuator[HEATING_CHAIN1].step = STEP_SEND_CMD;
    return 1;
}

int back_heating_chain() {
    machine.actuator[HEATING_CHAIN1].target_position = INSIDE;
    machine.actuator[HEATING_CHAIN1].step = STEP_SEND_CMD;
    return 1;
}

int check_premature_cicle_end(int32_t recomputeProd, int32_t Sequence) {
    bool recalProd = false;
    uint32_t production_cycle_time_ms = machine.App.production_cycle_time_ms;
    
    if (machine.workSet.min_secondary_air_time_ms >= 0) {
        if (machine.App.secondary_air_persistence_time_ms <= machine.workSet.min_secondary_air_time_ms) {
            // Rettifica il tempo ciclo
            production_cycle_time_ms = machine.App.reference_mold_cycle_time_ms + (machine.workSet.min_secondary_air_time_ms + machine.workSet.secondary_air_gap_ms + 50) + machine.workSet.discharge_air_time_ms;
            recalProd = true;
            
            snprintf((char*) App.Msg, App.MsgSize, (char*) "Not enough secondary time %d/%dmsec...recomputing cycle time to %0.2fsec"
                    , machine.App.secondary_air_persistence_time_ms
                    , machine.workSet.min_secondary_air_time_ms
                    , (float)production_cycle_time_ms / 1000.0f );
            vDisplayMessage(App.Msg);
            if (generate_alarm((char*) App.Msg, 7710, MOLD, (int) ALARM_WARNING, 0+1) < 0) {
            }            
        }        
    }

    if(recalProd) {
        recompute_prod(3, Sequence, production_cycle_time_ms);
        return 1;
    }
    
    return 0;
}


#define MIN_CYCLE_TIME_MS   ((int32_t)(3600.0f / (float)MAX_PRODUCTION_CYCLES_PER_HOUR * 1000.0f))

int recompute_prod(int32_t case_number, int32_t Sequence, uint32_t production_cycle_time_ms) {    

    // Ricalcola la produzione
    if (production_cycle_time_ms > 0) {
        if (production_cycle_time_ms >= MIN_CYCLE_TIME_MS) {
            machine.workSet.productionRecomputed = 3600.0f / (float)((production_cycle_time_ms + 10) / 1000.0f);
        } else {
            machine.workSet.productionRecomputed = MAX_PRODUCTION_CYCLES_PER_HOUR;
        }
    } else {
        machine.workSet.productionRecomputed = machine.workSet.production  * 0.9;
    }    
    
    // Azzera se livello prod. pià alto
    if (machine.workSet.productionRecomputed >= machine.workSet.production) {
        machine.workSet.productionRecomputed = 0;        
    }
            
    switch (case_number) {

        case 0:
            // snprintf(machine.status_message, machine.status_message_size, "Sequenza Trasferitore NON pronta [%dmsec]!", machine.App.manip_pref_cycle_time_ms);
            snprintf(machine.status_message, machine.status_message_size, "Sequenza Trasferitore NON pronta (SEQ:%d)..new cycle time:%0.2f", Sequence, (float)production_cycle_time_ms / 1000.0f );
            if (!machine.once_actions[0]) {
                machine.once_actions[0] = 1;
                if (generate_alarm((char*) machine.status_message, 7800, 0, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
            break;

        case 1:
            if (machine.once_actions[0]) {
                // snprintf(machine.status_message, machine.status_message_size, "Sequenza Trasferitore NON pronta [Ciclo minimo:%dmsec]!", machine.App.manip_pref_cycle_time_ms);
                snprintf(machine.status_message, machine.status_message_size, "Sequenza Trasferitore NON pronta (SEQ:%d)..new cycle time:%0.2f", Sequence, (float)production_cycle_time_ms / 1000.0f );
                if (!machine.once_actions[1]) {
                    machine.once_actions[1] = 1;
                    if (generate_alarm((char*) machine.status_message, 7801, 0, (int) ALARM_WARNING, 0+1) < 0) {
                    }
                }
            }
            break;

        case 2:
            // snprintf(machine.status_message, machine.status_message_size, "Sequenza Traferitore NON pronta [%dmsec]!", machine.App.transferitor_cycle_time_ms);
            snprintf(machine.status_message, machine.status_message_size, "Sequenza Traferitore NON pronta (SEQ:%d)..new cycle time:%0.2f", Sequence, (float)production_cycle_time_ms / 1000.0f );
            if (!machine.once_actions[0]) {
                machine.once_actions[0] = 1;
                if (generate_alarm((char*) machine.status_message, 7802, 0, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
            break;

        case 3:
            // snprintf(machine.status_message, machine.status_message_size, "Sequenza Soffiaggio troppo corta [%dmsec]!", machine.App.mold_cycle_time_ms);
            snprintf(machine.status_message, machine.status_message_size, "Sequenza Soffiaggio troppo corta (SEQ:%d)..new cycle time:%0.2f", Sequence, (float)production_cycle_time_ms / 1000.0f );
            if (!machine.once_actions[3]) {
                machine.once_actions[3] = 1;
                if (generate_alarm((char*) machine.status_message, 7803, 0, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
            break;
            
        default:
            break;
    }
    return 1;
}
