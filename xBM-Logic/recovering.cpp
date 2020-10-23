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


                // Comando Blocco pista
                machine.actuator[PIT_LOCK].target_position = UP;
                machine.actuator[PIT_LOCK].step = STEP_SEND_CMD;

                // Comando Chiusura Aria primaria
                if (close_primary_air() < 0) {
                    // ...
                }

                // Comando Chiusura Aria secondaria
                close_secondary_air();


                // Comando Apertura Scarico Aria
                open_discharge_air();

                // Comando Apertura Scarico Aria
                open_recovery_air();



                // Comando Spegnimento Forni
                machine.actuator[OWENS].target_position = OFF;
                machine.actuator[OWENS].step = STEP_SEND_CMD;

                // Comando Spegnimento Ventilazione
                machine.actuator[VENTILATOR].target_position = OFF;
                machine.actuator[VENTILATOR].step = STEP_SEND_CMD;

                // Comando Spegnimento Ventilazione
                machine.actuator[ASPIRATOR].target_position = OFF;
                machine.actuator[ASPIRATOR].step = STEP_SEND_CMD;

                machine.sequence = 15;

                
            } else if (machine.sequence == 15) {
                // Imposta l'attesa scaricamento aria
                machine.App.secondary_air_persistence_start = xTaskGetTickCount();
                machine.sequence = 16;

            } else if (machine.sequence == 16) {
                // Attesa sciricamento aria
                if (xTaskGetTickCount() - machine.App.secondary_air_persistence_start >= 2000) {
                    machine.App.secondary_air_persistence_start = 0;
                    machine.sequence = 17;
                }


                
                
                
                ///////////////////////
                // Azzeramento ASTE
                //                
            } else if (machine.sequence == 17) {
                // Necessario l'azzeramento ?
                if (!machine.actuator[STRETCH].homingDone) {
                    // Imposta la richiesta di homing
                    switch (process_actuator_homing_request( (void *)&machine.actuator[STRETCH])) {
                        case -1:
                            machine.sequence = 18;
                            break;
                        case 0:
                            machine.sequence = 19;
                            break;
                        case 1:
                            machine.sequence = 20;
                            break;
                    }
                } else {
                    machine.sequence = 200;
                }

            } else if (machine.sequence == 18) {
                if (machine.actuator[STRETCH].step == STEP_READY) {
                    machine.sequence = 17;
                } else {
                    strncpy(machine.status_message, "Waiting STRETCH ready", machine.status_message_size);
                }
                
            } else if (machine.sequence == 19) {
                if (machine.actuator[STRETCH].step == STEP_READY) {
                    machine.sequence = 17;
                } else {
                    strncpy(machine.status_message, "Waiting STRETCH ready", machine.status_message_size);
                }
            
            } else if (machine.sequence == 20) {
                if (machine.actuator[STRETCH].homingDone) {
                    machine.sequence = 200;
                } else {
                    strncpy(machine.status_message, "Waiting STRETCH homing", machine.status_message_size);
                }
                
                
                
                ///////////////////////
                // Azzeramento ASTE
                //                
            } else if (machine.sequence == 200) {
                // Controllo stato Aste            
                if (machine.actuator[STRETCH].position != STRETCH_UP) {
                    // machine.actuator[STRETCH].speed = 1;
                    up_stretch_rod();
                    machine.sequence = 210;
                    strncpy(machine.status_message, "Comando Aste su", machine.status_message_size);
                } else {
                    machine.sequence = 250;
                }

            } else if (machine.sequence == 210) {
                // Attesa movimento aste            
                if (machine.actuator[STRETCH].position == STRETCH_UP) {
                    machine.sequence = 220;
                } else {
                    strncpy(machine.status_message, "Attesa Aste su", machine.status_message_size);
                }

            } else if (machine.sequence == 220) {
                // Attesa pressione su stampop assente       
                if (machine.actuator[BLOW_PRESSURE].cur_rpos <= machine.workSet.max_pressure_in_mold) {
                    machine.sequence = 250;
                } else {
                    strncpy(machine.status_message, "Attesa scaricamento pressione", machine.status_message_size);
                }

                

                
                
                
                ///////////////////////
                // Azzeramento stampo
                //
            } else if (machine.sequence == 250) {
                machine.sequence = 257;

                
            } else if (machine.sequence == 257) {
                // Necessario l'azzeramento ?
                if (!machine.actuator[MOLD].homingDone) {
                    // Imposta la richiesta di homing
                    switch (process_actuator_homing_request( (void *)&machine.actuator[MOLD])) {
                        case -1:
                            machine.sequence = 258;
                            break;
                        case 0:
                            machine.sequence = 259;
                            break;
                        case 1:
                            machine.sequence = 260;
                            break;
                    }
                } else {
                    machine.sequence = 270;
                }
                
            } else if (machine.sequence == 258) {
                if (machine.actuator[MOLD].step == STEP_READY) {
                    machine.sequence = 257;
                } else {
                    strncpy(machine.status_message, "Waiting MOLD ready", machine.status_message_size);
                }
                
            } else if (machine.sequence == 259) {
                if (machine.actuator[MOLD].step == STEP_READY) {
                    machine.sequence = 257;
                } else {
                    strncpy(machine.status_message, "Waiting MOLD ready", machine.status_message_size);
                }
            
            } else if (machine.sequence == 260) {
                if (machine.actuator[MOLD].homingDone) {
                    machine.sequence = 270;
                } else {
                    strncpy(machine.status_message, "Waiting MOLD homing", machine.status_message_size);
                }
                
            } else if (machine.sequence == 270) {
                // Controllo stato STAMPO
                if (machine.actuator[MOLD].position != OPEN) {
                    open_mold();
                    machine.sequence = 280;
                    strncpy(machine.status_message, "Opening MOLD", machine.status_message_size);
                } else {
                    machine.sequence = 300;
                }

            } else if (machine.sequence == 280) {
                // Attesa movimento stampo
                if (machine.actuator[MOLD].position == OPEN) {
                    machine.sequence = 300;
                } else {
                    strncpy(machine.status_message, "Waiting MOLD open", machine.status_message_size);
                }


                
                
                


                
            } else if (machine.sequence == 300) {
                // Controllo trasferitore
                if (machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) {
                    if (machine.actuator[TRANSFERITOR_X].position == BACK) {
                        // OK trasferitore fuori e indietro : Comando chiusura stampo inserimento Trasferitore
                        if (machine.actuator[MOLD].position != CLOSE) {
                            inside_transferitor();
                            machine.sequence = 310;
                        } else {
                            inside_transferitor();
                            machine.sequence = 310;
                        }
                    } else if (machine.actuator[TRANSFERITOR_X].position == FRONT) {
                        // NO : trasferitore fuori e avanti
                        machine.actuator[TRANSFERITOR_X].target_position = BACK;
                        machine.actuator[TRANSFERITOR_X].step = STEP_SEND_CMD;
                        machine.sequence = 320;
                    } else {
                        // NO : trasferitore fuori e fuori posizione
                        machine.actuator[TRANSFERITOR_X].target_position = BACK;
                        machine.actuator[TRANSFERITOR_X].step = STEP_SEND_CMD;
                        machine.sequence = 320;
                    }

                } else if (machine.actuator[TRANSFERITOR_Y].position == INSIDE) {
                    if (machine.actuator[TRANSFERITOR_X].position == FRONT) {
                        // trasferitore dentro e avanti : chiude lo stampo abbassa i blocchi pref./bott.
                        close_mold();
                        down_holding_station();
                        machine.sequence = 330;
                    } else if (machine.actuator[TRANSFERITOR_X].position == BACK) {
                        // OK : trasferitore dentro e indietro
                        machine.sequence = 400;
                    } else {
                        // trasferitore dentro e fuori posizione : apre lo stampo
                        if (open_mold() > 0) {
                            up_holding_station();
                            machine.sequence = 350;
                        } else {
                            snprintf(str, sizeof (str), (char*) "Mold open failed");
                            if (generate_alarm((char*) str, 9002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }
                    }
                } else {
                    // stato indeterminato ???
                    outside_transferitor();
                    machine.sequence = 340;
                }


            } else if (machine.sequence == 310) {
                // Attesa inserimento trasferitore
                strncpy(machine.status_message, "Attesa INSERIMENTO TRASF Y", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_Y].position == INSIDE) {
                    // Ritorna alla sequenza analizzatrice
                    machine.sequence = 300;
                }

            } else if (machine.sequence == 320) {
                // Attesa trasferitore indietro
                strncpy(machine.status_message, "Attesa INDIETRO TRASF X", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_X].position == machine.actuator[TRANSFERITOR_X].target_position) {
                    // Ritorna alla sequenza analizzatrice
                    machine.sequence = 300;
                }

            } else if (machine.sequence == 330) {
                // Attesa stampo chiuso e blocchi pref./bott. gi�
                strncpy(machine.status_message, "Attesa STAMPO CHIUSO", machine.status_message_size);
                if (machine.actuator[MOLD].position == CLOSE) {
                    strncpy(machine.status_message, "Attesa GIU BLOCCO PREF.", machine.status_message_size);
                    if (machine.actuator[BOTTLE_PREF_HOLDER].position == DOWN) {
                        // Comando indietro strasferitore
                        outside_transferitor();
                        machine.sequence = 340;
                    }
                }


            } else if (machine.sequence == 340) {
                // Attesa diinserimento trasferitore
                strncpy(machine.status_message, "Attesa FUORI TRASF Y", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) {
                    // Ritorna alla sequenza analizzatrice
                    machine.sequence = 300;
                }

            } else if (machine.sequence == 350) {
                // Attesa stampo aperto e blocchi pref./bott. su
                strncpy(machine.status_message, "Attesa STAMPO APERTO", machine.status_message_size);
                if (machine.actuator[MOLD].position == OPEN) {
                    strncpy(machine.status_message, "Attesa SU BLOCCO PREF.", machine.status_message_size);
                    if (machine.actuator[BOTTLE_PREF_HOLDER].position == UP) {
                        // Comando strasferitore avanti (completa il traslo)
                        front_transferitor();
                        machine.sequence = 360;
                    }
                }

            } else if (machine.sequence == 360) {
                // Attesa trasferitore avanti
                strncpy(machine.status_message, "Attesa AVANTI TRASF. X", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_X].position == FRONT) {
                    // Ritorna alla sequenza analizzatrice
                    machine.sequence = 300;
                }

            } else if (machine.sequence == 370) {
                // Attesa chiusura stampo
                strncpy(machine.status_message, "Attesa CHIUSURA STAMPO", machine.status_message_size);
                if (machine.actuator[MOLD].position == CLOSE) {
                    // Ritorna alla sequenza analizzatrice
                    machine.sequence = 300;
                }







            } else if (machine.sequence == 400) {
                ///////////////////////////////////////////////////////////////////////////////////////////
                // Esegue il completamento della traslazione delle pref. nello stampo
                //

                // Controllo stato stampo : apre lo stampo e abbassa le stazioni bi blocco
                if (machine.actuator[MOLD].position != OPEN) {
                    if (open_mold() > 0) {
                        // Comando accettato : Comando Salita blocchi preforme
                        up_holding_station();

                        // Comando Salita manipolatore preforme
                        up_preform_manipolator();

                        machine.sequence = 410;

                    } else {
                        // Comando fallito
                        strncpy(machine.status_message, "RECOVERY mode error : impossibile aprire lo stampo", machine.status_message_size);
                        snprintf(str, sizeof (str), (char*) "Mold open failed");
                        if (generate_alarm((char*) str, 9003, 0, (int) ALARM_ERROR, 0+1) < 0) {
                        }
                    }

                } else {

                    // Comando Salita blocchi preforme
                    up_holding_station();

                    // Comando Salita manipolatore preforme
                    up_preform_manipolator();

                    machine.sequence = 410;
                }


            } else if (machine.sequence == 410) {
                // Attesa movimento stampo e stati blocco bott./pref.
                strncpy(machine.status_message, "Attesa APERTURA STAMPO", machine.status_message_size);
                if (machine.actuator[MOLD].position == OPEN) {
                    strncpy(machine.status_message, "Attesa SU BLOCCO PREF.", machine.status_message_size);
                    if (machine.actuator[BOTTLE_PREF_HOLDER].position == UP) {
                        strncpy(machine.status_message, "Attesa SU MANIP.PREF.Z", machine.status_message_size);
                        if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                            machine.sequence = 420;
                        }
                    }
                }


            } else if (machine.sequence == 420) {

                // Comando avanti trasferitore stampo
                if (front_transferitor() > 0) {

                    // Comando indietro manipolatore pref.
                    if (back_preform_manipolator() > 0) {

                        machine.sequence = 430;
                    }
                }


            } else if (machine.sequence == 430) {
                // Attesa movimento stampo e stati blocco bott./pref.
                strncpy(machine.status_message, "Attesa AVANTI TRASF.X", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_X].position == FRONT) {
                    strncpy(machine.status_message, "Attesa INDIETRO MANIP.PREF.X", machine.status_message_size);
                    if (machine.actuator[LOAD_UNLOAD_X].position == BACK) {
                        machine.sequence = 440;
                    }
                }



            } else if (machine.sequence == 440) {

                // Comando apertura stampo / abbassamento stazioni blocco
                if (close_mold() > 0) {

                    // Comando abbassamento stazioni blocco
                    down_holding_station();


                    // Comando Avanzamento Catena riscaldamento
                    step_heating_chain();


                    // Esecuzione shift registro di carico preforme
                    do_preform_registry_shift();


                    machine.sequence = 450;
                }


            } else if (machine.sequence == 450) {
                // Attesa movimento stampo e stati blocco bott./pref.
                strncpy(machine.status_message, "Attesa APERTURA STAMPO", machine.status_message_size);
                if (machine.actuator[MOLD].position == CLOSE) {
                    strncpy(machine.status_message, "Attesa GIU BLOCCO PREF.", machine.status_message_size);
                    if (machine.actuator[BOTTLE_PREF_HOLDER].position == DOWN) {
                        machine.sequence = 460;
                    }
                }



            } else if (machine.sequence == 460) {

                // Comando fuori trasferitore stampo
                outside_transferitor();

                // Comando Indietro Step Catena riscaldamento
                back_heating_chain();

                machine.sequence = 470;


            } else if (machine.sequence == 470) {
                // Attesa fuori trasferitore stampo
                strncpy(machine.status_message, "Attesa FUORI TRASF.Y", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) {
                    strncpy(machine.status_message, "Attesa INDIETRO AVANZ. CATENA 1", machine.status_message_size);
                    if (machine.actuator[HEATING_CHAIN1].position == BACK) {
                        machine.sequence = 480;
                    }
                }



            } else if (machine.sequence == 480) {

                // Comando indietro trasferitore stampo
                if (back_transferitor() > 0) {

                    // Comando discesa manipolatore pref.
                    down_preform_manipolator();


                    machine.sequence = 490;
                }


            } else if (machine.sequence == 490) {
                // Attesa fuori trasferitore stampo
                strncpy(machine.status_message, "Attesa INDIETRO. TRASF.X", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_X].position == BACK) {
                    strncpy(machine.status_message, "Attesa GIU MANIP.PREF.Z", machine.status_message_size);
                    if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                        machine.sequence = 500;
                    }
                }


            } else if (machine.sequence == 500) {

                // Comando discesa manipolatore pref.
                unpick_preform_manipolator();

                machine.sequence = 510;


            } else if (machine.sequence == 510) {
                // Attesa pinza aperta
                strncpy(machine.status_message, "Attesa APERTURA PINZA MANIP.", machine.status_message_size);
                if (machine.actuator[LOAD_UNLOAD_PICK].position == OPEN) {
                    machine.sequence = 520;
                }


            } else if (machine.sequence == 520) {

                // Comando salita manipolatore pref.
                up_preform_manipolator();

                machine.sequence = 530;


            } else if (machine.sequence == 530) {
                // Attesa fuori trasferitore stampo
                strncpy(machine.status_message, "Attesa SU MANIP.PREF.Z", machine.status_message_size);
                if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                    machine.sequence = 540;
                }



            } else if (machine.sequence == 540) {

                // Comando indietro manipolatore pref.
                front_preform_manipolator();

                machine.sequence = 550;


            } else if (machine.sequence == 550) {
                // Attesa manip. pref. avanti
                strncpy(machine.status_message, "Attesa AVANTI MANIP.PREF.X", machine.status_message_size);
                if (machine.actuator[LOAD_UNLOAD_X].position == FRONT) {
                    machine.sequence = 560;
                }


            } else if (machine.sequence == 560) {

                // Comando discesa manipolatore pref.
                down_preform_manipolator();

                machine.sequence = 570;


            } else if (machine.sequence == 570) {
                // Attesa giu pinza manip. pref.
                strncpy(machine.status_message, "Attesa GIU PINZA MANIP.PREF.", machine.status_message_size);
                if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                    machine.sequence = 580;
                }




            } else if (machine.sequence == 580) {

                // Comando discesa manipolatore pref.
                pick_preform_manipolator();

                machine.sequence = 590;

            } else if (machine.sequence == 590) {
                // Attesa chiusura pinza
                strncpy(machine.status_message, "Attesa CHIUSURA PINZA MANIP.PREF", machine.status_message_size);
                if (machine.actuator[LOAD_UNLOAD_PICK].position == CLOSE) {
                    machine.sequence = 700;
                }






                ///////////////////////////////////////////////////
                // Azzeramento manipolatore catena
                //

            } else if (machine.sequence == 700) {


                machine.sequence = END_SEQUENCE;






                // ciclo di svuotamento ?
                /*
                if (1==0) {
                        machine.sequence = DISCHARDGING_PREFORMS;
                } else {
                        machine.sequence = ;
                }
                 */




                ////////////////////////////////////////////////////
                // Sequenza svuotamento : da testare
                //

            } else if (machine.sequence == 1000) {
                // Comando inserimento trasferotore
                inside_transferitor();

            } else if (machine.sequence == 1100) {
                // Attesa inserimento trasferotore
                strncpy(machine.status_message, "Attesa DENTRO TRASF.Y", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_Y].position == INSIDE) {
                    machine.sequence = 1200;
                }


            } else if (machine.sequence == 1200) {

                // Comando apertura stampo
                if (open_mold() > 0) {

                    // Comando  alza stazioni blocco
                    up_holding_station();

                    // Comando alza manipolatore pref.
                    up_preform_manipolator();

                    machine.sequence = 1300;
                } else {
                    snprintf(str, sizeof (str), (char*) "Mold open failed");
                    if (generate_alarm((char*) str, 9004, 0, (int) ALARM_ERROR, 0+1) < 0) {
                    }
                }



            } else if (machine.sequence == 1300) {
                // Attesa stampo e stazione blocco
                strncpy(machine.status_message, "Attesa APERTURA STAMPO", machine.status_message_size);
                if (machine.actuator[MOLD].position == OPEN) {
                    strncpy(machine.status_message, "Attesa SU BLOCCO PREF.", machine.status_message_size);
                    if (machine.actuator[BOTTLE_PREF_HOLDER].position == UP) {
                        strncpy(machine.status_message, "Attesa SU MANIP.PREF.Z", machine.status_message_size);
                        if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                            machine.sequence = 1400;
                        }
                    }
                }


            } else if (machine.sequence == 1400) {

                // Comando trasnferitore avanti
                front_transferitor();

                // COmando indietro manipolatore pref.
                back_preform_manipolator();


            } else if (machine.sequence == 1500) {
                // Attesa trasnferitore avanti
                strncpy(machine.status_message, "Attesa AVANTI TRASF.X", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_X].position == FRONT) {
                    strncpy(machine.status_message, "Attesa INDIETRO MANIP.PREF.X", machine.status_message_size);
                    if (machine.actuator[LOAD_UNLOAD_X].position == BACK) {
                        machine.sequence = 1600;
                    }
                }



            } else if (machine.sequence == 1600) {

                // Comando apertura stampo / abbassamento stazioni blocco
                close_mold();

                // Comando abbassamento stazioni blocco
                down_holding_station();

                // Comando discesa manipolatore pref.
                down_preform_manipolator();

                machine.sequence = 1700;


            } else if (machine.sequence == 1700) {
                // Attesa stampo e stazione blocco e manipolatore basso
                strncpy(machine.status_message, "Attesa CHIUSURA STAMPO", machine.status_message_size);
                if (machine.actuator[MOLD].position == CLOSE) {
                    strncpy(machine.status_message, "Attesa GIU BLOCCO.PREF.", machine.status_message_size);
                    if (machine.actuator[BOTTLE_PREF_HOLDER].position == DOWN) {
                        strncpy(machine.status_message, "Attesa GIU MANIP.PREF.Z", machine.status_message_size);
                        if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                            // Comando unpick mandrino
                            unpick_preform_manipolator();
                            machine.sequence = 1800;
                        }
                    }
                }


            } else if (machine.sequence == 1800) {
                // Attesa un pick manipolatore
                strncpy(machine.status_message, "Attesa APERTURA PINZA MANIP.PREF.", machine.status_message_size);
                if (machine.actuator[LOAD_UNLOAD_PICK].position == OPEN) {

                    // Comando trasferitore fuori
                    outside_transferitor();

                    // Comandi salita manipolatore pref.
                    up_preform_manipolator();

                    machine.sequence = 1900;
                }


            } else if (machine.sequence == 1900) {
                // Attesa trasnferitore fuori
                strncpy(machine.status_message, "Attesa FUORI TRASF.Y", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) {
                    strncpy(machine.status_message, "Attesa SU MANIP.PREF.Z", machine.status_message_size);
                    if (machine.actuator[LOAD_UNLOAD_Z].position == UP) {
                        machine.sequence = 2000;
                    }
                }


            } else if (machine.sequence == 2000) {

                // Comando trasnferitore indietro
                back_transferitor();

                // Comandi avanti manipolatore pref.
                front_preform_manipolator();

                // Comando Avanzamento Catena riscaldamento
                step_heating_chain();



            } else if (machine.sequence == 2100) {
                // Attesa trasferitore indietro
                strncpy(machine.status_message, "Attesa INDIETRO TRASF.X", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_X].position == BACK) {
                    // Attesa manipolatore avanti
                    strncpy(machine.status_message, "Attesa AVANTI MANIP.PREF.X", machine.status_message_size);
                    if (machine.actuator[LOAD_UNLOAD_X].position == FRONT) {
                        // Attesa catena riscaldamento avanti
                        strncpy(machine.status_message, "Attesa FUORI AVANZ.CATENA 1", machine.status_message_size);
                        if (machine.actuator[HEATING_CHAIN1].position == OUTSIDE) {
                            machine.sequence = 2200;
                        }
                    }
                }


            } else if (machine.sequence == 2200) {
                // Comando discesa manipolatore
                back_transferitor();

                // Comando abbassa manipolatore pref.
                down_preform_manipolator();

                machine.sequence = 2300;



            } else if (machine.sequence == 2300) {
                // Attesa trasferitore indietro
                strncpy(machine.status_message, "Attesa INDIETRO TRASF.X", machine.status_message_size);
                if (machine.actuator[TRANSFERITOR_X].position == BACK) {
                    // Attesa abbassamento manipolatore pref.
                    strncpy(machine.status_message, "Attesa GIU MANIP.PREF.Z", machine.status_message_size);
                    if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                        machine.sequence = 2400;
                    }
                }





            } else if (machine.sequence == 2400) {

                // Comando pick mandrino
                pick_preform_manipolator();

                machine.sequence = END_SEQUENCE;





            } else if (machine.sequence == END_SEQUENCE) {

                strncpy(machine.status_message, "Attesa CHIUSURA_STAMPO", machine.status_message_size);
                if (machine.actuator[MOLD].position == CLOSE) {
                    // Stampo chiuso
                    strncpy(machine.status_message, "Attesa INDIETRO TRASF.X", machine.status_message_size);
                    if (machine.actuator[TRANSFERITOR_X].position == BACK) {
                        // Trasf indietro
                        strncpy(machine.status_message, "Attesa FUORI TRASF.Y", machine.status_message_size);
                        if (machine.actuator[TRANSFERITOR_Y].position == OUTSIDE) {
                            // Trasf fuori
                            strncpy(machine.status_message, "Attesa GIU MANIP.PREF.Z", machine.status_message_size);
                            if (machine.actuator[LOAD_UNLOAD_Z].position == DOWN) {
                                // Mapipolatore basso
                                strncpy(machine.status_message, "Attesa AVANI MANIP.PREF.X", machine.status_message_size);
                                if (machine.actuator[LOAD_UNLOAD_X].position == FRONT) {
                                    // Mapipolatore avanti
                                    strncpy(machine.status_message, "Attesa CHIUSURA PINZA MANIP.PREF.", machine.status_message_size);
                                    if (machine.actuator[LOAD_UNLOAD_PICK].position == CLOSE) {
                                        // Mapipolatore pinza chiusa : FINE Sequenza riprostino
                                        // machine.sequence = END_SEQUENCE;

                                        // Fine ripristino
                                        machine.status = READY;
                                        machine.sequence = 0;
                                    }
                                }
                            }
                        }
                    }
                }


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
