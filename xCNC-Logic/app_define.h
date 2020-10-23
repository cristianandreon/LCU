
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
    X = 0,  // Asse X
    Y,      // Asse Y
    Z,      // Asse Z
    W,      // Asse W Tavola
    T,      // Asse Canotto
    SPINDLE, // Mandrino
    COOLER_I,   // Rafreddamento
    COOLER_II,  
    AIR_PRESS,
    MAX_ACTUATOR_NAME
};




// Struttura parametri lavoro

typedef struct tag_gcode {

    // Righe da visualizzare
    char **displayRows;
    int32_t **displayRowsOptions;
    uint32_t numDisplayRows;
    uint32_t numDisplayRowsAlloc;
    
    // Righe GCode
    char *Content;    
    char **Rows;
    int32_t *RowsOptions;
        // BIT1 ->  Riga in processo
        // BIT2 ->  Riga con errore
        // BIT3 ->  Riga processata
        // BIT4 ->  Riga commento
        // -------------------------
        // BIT5 ->  X setted
        // BIT6 ->  Y setted
        // BIT7 ->  Z setted
        // BIT8 ->  W setted
        // BIT9 ->  T setted
        // -------------------------
        // BIT10 ->  P setted
        // BIT11 ->  R setted
        // BIT12 ->  I setted
        // BIT13 ->  J setted
        // BIT14 ->  K setted
        // BIT15 ->  Q setted
        // -------------------------
        // BIT20 ->  S speed setted    
        // BIT21 ->  F feed setted
        // -------------------------
        // BIT25 ->  M spindle code setted    
        // BIT26 ->  M cooler code setted    
        // -------------------------    
        // BIT28 ->  Riga selezionata
        // BIT29 ->  Break point
        // -------------------------    
        // BIT30 ->  Riga commento con chiave
        // BIT31 ->  Riga commento con chiave XP
    
    uint32_t startRow;
    uint32_t numRows;
    uint32_t numRowsAlloc;
       
    // Rica selezionata
    uint32_t cRow;
    
    uint32_t ContentChanged;
    
    
    char *SearchStr, *ReplaceStr;
    
} GCODE, *LP_GCODE, **LPP_GCODE;





// Comando dal gcode parser

typedef struct tag_gcode_cmd {

    // Rica corrente in esecuzione
    uint32_t curRow, curRowChar;

    int32_t Result;
    
    float targetX, targetY, targetZ, targetW, targetT;
    float centerX, centerY, centerZ, radius, startAng, endAng;
    uint8_t moveType;   
        // 0 = interpolazione lineare rapido
        // 1 = interpolazione lineare
        // 2 = interpolazione circolare CW (orario)
        // 3 = interpolazione circolare CCW (anti-orario)
    
    float P, R, I, J, K, Q;
    float drillDepth, drillTarget;
    float speed_mm_min, acc_ms2, dec_ms2;
    float spindle_speed_rpm;
    uint8_t curPlane;   // G17/G18/G19
    
    // Risultato del parser
    int32_t parserResult;
    
    // sequesnza da impostare nello stato macchina
    int32_t nextSequence;
    
    // Opzioni di riga impostate dal parser
    int32_t RowOptions;

    // Coordinate assolute/relative
    int32_t abs_coord;
    int32_t abs_arc_coord;
  
    // Comandi Codici M
    int32_t spindleCMD;
    int32_t CoolerCMD;
    uint32_t gotoRow1B;

    uint32_t WaitTimeMS;
                            
    
    // Profondita di passata
    float DepthStep;

    // Velocità taglio utensile
    float ToolCutSpeed;
    
    // Raggio utensile
    float ToolRad;

    // Numero taglienti
    int32_t ToolNumCuttingEdge;
    
    // Posizione / numero utensile (mnemonico)
    int32_t ToolPos;
    
    // Precisione lavorazione (interpolazione)
    float Precision;
    
    
} GCODE_CMD, *LP_GCODE_CMD, **LPP_GCODE_CMD;




typedef struct tag_gcode_setup {

    // Modalita simulazione
    int32_t simulateMode;

    int32_t Options;

    // Dati materiale
    char *Material;
    float MaterialCurSpeed;
    float MaterialSigma;


    
    // Mappatura assi
    uint8_t XMap;
    uint8_t YMap;
    uint8_t ZMap;
    uint8_t WMap;
    uint8_t TMap;
    
} GCODE_SETUP, *LP_GCODE_SETUP, **LPP_GCODE_SETUP;



// Struttura parametri lavoro

typedef struct tag_material {
    
    int8_t *name;
    int8_t *desc;
    int8_t *alias;
     
    float sigma;        // N/mm2
    float cut_speet;    // mm/min
    float density;      // kg/dcm3
    
    int32_t options;
    
} MATERIAL, *LP_MATERIAL, **LPP_MATERIAL;






// Struttura parametri lavoro

typedef struct tag_work_set {
    
    char name[256];
    char description[512];

    uint32_t loadCount;

    int32_t options;
    
    float rapid_feed, mill_feed;
    float spindle_speed;
    
    uint32_t current_tool;
    char *tool_name;
    float tool_diam;
    float tool_height;
    uint32_t tool_time;

} WORK_SET, *LP_WORK_SET, **LPP_WORK_SET;






// Struttura parametri macchina

typedef struct tag_settings {

    float rapid_feed_X, rapid_feed_Y, rapid_feed_Z, rapid_feed_W, rapid_feed_T;
    float mill_feed_mm_min_X, mill_feed_mm_min_Y, mill_feed_mm_min_Z, mill_feed_mm_min_W, mill_feed_mm_min_T;
    float spindle_speed;

    float max_weight;
    float max_X, max_Y, max_Z, max_W, max_T;

    float spindle_power;
    float cam_ratio_X;
    float cam_ratio_Y;
    float cam_ratio_Z;
    float cam_ratio_W;
    float cam_ratio_T;
    
    
    LP_MATERIAL materials;
    uint32_t num_materials, num_materials_allocated;
    
    float spindle_speed_toll;
    float feed_toll;
    
    // Calcolo parametri taglio/vita utensile
    float TaylorC;
    float TaylorN;
    
    // Precisione interpolazioni
    float InterpolationPrecisionMM;
    float DefaultGap;
    uint32_t DefaultGapCorrection;
    
    uint32_t InterpolationPeriodMS;

} SETTINGS, *LP_SETTINGS, **LPP_SETTINGS;







// Struttura statistiche

typedef struct tag_statistic {
    uint32_t pgr_cycles;
    uint32_t pgr_rows;
    uint32_t elapsed_time;
    uint32_t milling_time;
    uint32_t rapid_time;
    uint32_t collisions;
    
    uint32_t machine_running, machine_elapsed, machine_stops;
    uint32_t fatal_errors, errors, warnings;
    
    int8_t machine_running_string[64], machine_elapsed_string[64];

} STATISTIC, *LP_STATISTIC, **LPP_STATISTIC;




// Struttura dipendente dall' applicazione
typedef struct tag_app_specific {

    GCODE GCode;
    GCODE_CMD GCodeCmd;
    GCODE_SETUP GCodeSetup;
    
    int32_t options;
    
    float toll;
    
    float Epsilon;
    double dEpsilon;
    
    uint32_t SectionTime, AfterMillWaitTimeMS;
    

            
} APP_SPECIFIC, *LP_APP_SPECIFIC;

#endif

