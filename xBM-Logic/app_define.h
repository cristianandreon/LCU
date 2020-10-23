
/*
________________________________________________________________________________________

TITOLO SEZIONE :        Definizione strutture dati
AUTORE :                        Cristian Andreon
DATA :                  6-5-2007
________________________________________________________________________________________                                                                                                                                                                                                                                                                                                            */


#ifndef __APP_DEFINE_KEY

#define __APP_DEFINE_KEY

// #pragma message ("__APP_DEFINE_KEY DEFINED")


#ifdef WATCOM
#ifdef __cplusplus
#define _far
#else
#define _far    __far
#endif
#else
#define _far
#endif





enum ACTUATOR_NAME_ENUM {
    MOLD = 0, // stampo
    STRETCH, // asta
    TRANSFERITOR_X, // trasferitore avanti/indietro
    TRANSFERITOR_Y, // inserimento trasferitore
    BOTTLE_PREF_HOLDER, // espulsore bottiglia
    PRIMARY_AIR, // aria primaria
    SECONDARY_AIR, // aria secondaria
    DISCHARGE_AIR, // scarico aria
    RECOVERY_AIR, // Recupero aria
    HEATING_CHAIN1, // avanzamento mandrini I
    HEATING_CHAIN2, // avanzamento mandrini II
    HEATING_CHAIN3, // avanzamento mandrini III
    LOAD_UNLOAD_X, // manipolatore carico/scarico asse X
    LOAD_UNLOAD_Z, // manipolatore carico/scarico asse Z
    LOAD_UNLOAD_PICK, // manipolatore carico/scarico pinza
    BLOW_PRESSURE, // pressione aria bottiglia
    LINE_PRESSURE, // pressione aria linea
    PIT_LOCK, // blocco pista preforme
    PREF_LOAD_SUPP, // supporto carico preforme
    OWENS, // forni
    OWENS_TEMPERATURE1, // lettore temperatura forni
    OWENS_TEMPERATURE2, // lettore temperatura forni
    PREFORM_TEMPERATURE, // lettore temperatura preforma
    VENTILATOR, // ventilatore
    ASPIRATOR, // aspiratore
    COOLING_WATER1, // acqua forni
    COOLING_WATER2, // acqua stampo
    OWEN_AIR, // aria rafreddamento
    ELEV_UNJAM,
    ELEV_MOTOR,
    ORIENT_FAN,
    ORIENT_ROLL,
    MAX_ACTUATOR_NAME
};

enum PREFORM_STATUS_ENUM {
    EMPTY = 0, // vuoto
    PREFORM_LOADED, // preforma presente
    PREFORM_OK, // preforma a temperatura ok
    PREFORM_UNDER_LIMIT, // preforma a temperatura bassa
    PREFORM_OVER_LIMIT, // preforma a temperatura alta
    PREFORM_DAMAGED, // preforma danneggiata o parzialmente riscaldata
    MAX_PREFORM_STATUS
};






// Struttura parametri lavoro

typedef struct tag_work_set {
    char name[256];
    char description[512];

    uint32_t loadCount;

    float capacity;
    float preform_weight;
    float preform_thickness;
    char preform_color[256];

    // Produzione richiesta
    uint32_t production;
    uint32_t productionRecomputed;

    // Gestione aria
    float primary_air_gap_mm;          // mm relatico all'asta
    int32_t secondary_air_gap_ms;      // msec, relativo al raggiungimento quota asta
    int32_t min_secondary_air_time_ms;

    uint32_t discharge_air_time_ms;
    float recovery_air_factor;

    float max_pressure_in_mold;
    float pressure_min, pressure_max;

    float pressure_check;
    float pressure_check_gap;
    float pressure_check_time;


    // Spessore fondo bottiglia
    float stretch_mantein_force;
    float stretch_bottom_gap_mm;


    float ventilation_ratio;

    // Temperatura preforme
    float preform_temp1, preform_temp2, preform_temp3;
    float preform_temp_gap1, preform_temp_gap2, preform_temp_gap3;


    // Riscaldamento forni

    // Globali riscaldamento
    float init_heat_ratio1, init_heat_ratio2;
    float global_heat_ratio1, global_heat_ratio2;
    float standby_heat_ratio1, standby_heat_ratio2;



    // Temperature forni
    float owen1_min_temp, owen2_min_temp, owen1_max_temp, owen2_max_temp;

    // Percentuale irraggiatori
     #define MAX_OWENS_ROWS 16
    float owen1_row[MAX_OWENS_ROWS], owen2_row[MAX_OWENS_ROWS];
    float owen1_row_bk[MAX_OWENS_ROWS], owen2_row_bk[MAX_OWENS_ROWS];
    float owen1_power[MAX_OWENS_ROWS], owen2_power[MAX_OWENS_ROWS];



    // Tempi (msec)
    uint32_t pit_unlock_time_ms;
    uint32_t preform_loader_up_time_ms;
    uint32_t bottle_eject_down_time_ms;

    // Timeout generali (msec)
    uint32_t empy_preform_elevator_timeout_ms;
    uint32_t empy_preform_orientator_roller_timeout_ms;
    uint32_t empy_preform_orientator_pit_timeout_ms;

    // Tempi (sec)
    uint32_t fan_motor_time_msec;
    uint32_t roll_motor_time_msec;
    uint32_t aspiration_persistence_time_msec;
    uint32_t preform_elevator_time_msec;
    uint32_t unjammer_time_msec;


    // Arresto automatico su produzione raggiunta
    uint32_t num_bottles_to_product;

    // Presenza linea evacuazione (ingora la fotocellula scarico intasato)
    bool online_bottles_transfer_present;
    bool force_bottles_discharge;


    int32_t options;

} WORK_SET, *LP_WORK_SET, **LPP_WORK_SET;






// Struttura parametri macchina

typedef struct tag_settings {
    // Numero cicli attesa riscaldamento forni
    uint32_t startup_owens_cycles;
    uint32_t startup_owens_delay_ms;

    // Numero cicli attesa speginmento forni
    uint32_t turnoff_owens_cycles;
    uint32_t turnoff_owens_delay_ms;

    // Numero di cicli senza carico innesco standby forni
    uint32_t owens_standby_cycles;
    uint32_t owens_standby_delay_ms;
    uint32_t owens_towork_cycles;
    uint32_t owens_towork_delay_ms;

    uint32_t initial_owens_cycles;
    float initial_owens_ratio;

    uint32_t chain_stepper1_pause_ms;
    uint32_t chain_stepper2_pause_ms;
    uint32_t chain_stepper3_pause_ms;

    uint32_t trasf_x_forward_pause_ms;
    uint32_t chain_trasf_z_down_pause_ms;

    uint32_t chain_picker_open_pause_ms;
    uint32_t chain_picker_close_pause_ms;

    uint32_t pref_load_inside_pause_ms;
    uint32_t pref_load_outside_pause_ms;

    uint32_t pit_stopper_inside_pause_ms;
    uint32_t pit_stopper_outside_pause_ms;


    // Attesa spegnimento aspiratori
    uint32_t aspirator_delay_ms;

} SETTINGS, *LP_SETTINGS, **LPP_SETTINGS;







// Struttura statistiche

typedef struct tag_statistic {
    
    uint32_t preforms_loaded;
    uint32_t preforms_blowed;
    uint32_t bottles;
    uint32_t discharged;
    uint32_t mold_cycles;
    uint32_t fatal_errors;
    uint32_t errors;
    uint32_t warnings;
    uint32_t machine_stops;
    uint32_t machine_elapsed;
    char machine_elapsed_string[32];
    uint32_t machine_running;
    char machine_running_string[32];

} STATISTIC, *LP_STATISTIC, **LPP_STATISTIC;





#define MAX_PREFORM_REGISTRY    256

typedef struct tag_app_specific {
    
    // Posizione sensore temp e stampo
    uint32_t TEMPERATURE_READER_POS;
    uint32_t BLOW_MOLD_POS;
    uint32_t NUM_PREFORM_REGISTRY;
    

    // Registri di carico preforme
    uint8_t preform_registry[MAX_PREFORM_REGISTRY];

    // Numero cicli stampo eseguiti
    uint32_t mold_cycles;

    // Contatore cicli a forni attivati
    uint32_t owens_cycles;


    // Variabile lettura temperatura preforma corrente
    float preform_temp;

    // Variabile Produzione richiesta corrente
    uint32_t production;

    // Gestione carico
    int32_t load_on;

    // Gestione carico
    int32_t single_load_on;

    // Gestione riscaldamento
    int32_t heat_on;

    // Gestione soffiaggio
    int32_t blow_on;
    
    // Gestione scarico in-linea
    int32_t inline_output_on;
    
    
    

    // Sequenza corrente stampo
    uint32_t mold_sequence;
    // Conteggio temporale (msec)
    uint32_t mold_cycle_time_ms;
    uint32_t reference_mold_cycle_time_ms;
            
    // Sezione temporale apertura aria secondaria
    uint32_t secondary_air_on_time_ms;

    uint32_t secondary_air_persistence_start;
    
    // Durata temporale aria secondaria
    uint32_t secondary_air_persistence_time_ms;

    // Tempo scarico aria necessario
    uint32_t discharge_needed_time_ms;
    uint32_t discharge_needed_start_time_ms;

												
    float primmary_air_press;
    float secondary_air_press;
    

    // Sequenza del carico preforma
    uint32_t transferitor_sequence;
    // Conteggio temporale (msec)
    uint32_t transferitor_cycle_time_ms;
    uint32_t transferitor_sequence_start_time;



    // Sequenza del carico preforma
    uint32_t load_sequence;
    // Conteggio temporale (msec)
    uint32_t load_cycle_time;



    // Sequenza del pettine
    uint32_t manip_pref_sequence;
    // Conteggio temporale (msec)
    uint32_t manip_pref_cycle_time_ms;



    // Sequenza dell' espulsore bottiglia
    uint32_t bottle_eject_sequence;
    // Conteggio temporale (msec)
    uint32_t bottle_eject_cycle_time;





    // Sequenza del blocco/sblocco pista
    uint32_t pit_lock_sequence;
    // Conteggio temporale (msec)
    uint32_t pit_lock_cycle_time_ms;


    // Sequenza dei forni
    uint32_t owens_sequence;
    // Conteggio temporale (msec)
    uint32_t owens_cycle_time_ms;


    // Durata ciclo principale (calcolato dalla produzione) (msec)
    uint32_t production_cycle_time_ms;
    
    uint32_t mold_sequence_start_time;
    
    float Epsilon;
    
} APP_SPECIFIC, *LP_APP_SPECIFIC;
    
    

#endif

