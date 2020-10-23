/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifdef WATCOM
#include "FreeRTOS.h"
#else
#include "./../../RTLinux/RTLinux.h"
#endif


#define EXTERN
#include "./../../xBM-Logic/logic_precomp.h"


#include "../../libconfig-1.5/lib/libconfig.h"



#define READ_SETTINGS_FIELD_INT(__param,__baseStruct,__fieldName) \
        {   int32_t __bValue = 0;  \
        if (!(config_setting_lookup_int(__param, #__fieldName, &__bValue))) {\
                snprintf(App.Msg, App.MsgSize, "READ_SETTINGS_FIELD_INT error : %s%s:%d%s\n", (char*)ANSI_COLOR_RED, (char*)config_error_text(&cfg), config_error_line(&cfg), (char*)ANSI_COLOR_RED );\
                vDisplayMessage(App.Msg);\
                retVal = -1;\
            } else {    \
                __baseStruct.__fieldName = __bValue; \
            }   \
        }

#define READ_SETTINGS_FIELD_FLOAT(__param,__baseStruct,__fieldName) \
        {   double __bValue = 0.0;  \
            if (!(config_setting_lookup_float(__param, #__fieldName, &__bValue))) {   \
                snprintf(App.Msg, App.MsgSize, "READ_SETTINGS_FIELD_FLOAT error : %s%s:%d%s\n", (char*)ANSI_COLOR_RED, (char*)config_error_text(&cfg), config_error_line(&cfg), (char*)ANSI_COLOR_RED );\
                vDisplayMessage(App.Msg);\
                retVal = -1;\
            } else {    \
            __baseStruct.__fieldName = (float)__bValue; \
            }   \
        }

#define READ_SETTINGS_FIELD_STRING(__param,__baseStruct,__fieldName) \
        {   const char * __sValue = NULL;  \
            if (!(config_setting_lookup_string(__param, #__fieldName, &__sValue))) {   \
                snprintf(App.Msg, App.MsgSize, "READ_SETTINGS_FIELD_STRING error : %s%s:%d%s\n", (char*)ANSI_COLOR_RED, (char*)config_error_text(&cfg), config_error_line(&cfg), (char*)ANSI_COLOR_RED ); \
                vDisplayMessage(App.Msg); \
                retVal = -1; \
            } else {    \
            /* if(__baseStruct.__fieldName) free(__baseStruct.__fieldName); */\
            /* __baseStruct.__fieldName = (char*)__sValue; */\
            COPY_POINTER(__baseStruct.__fieldName, __sValue);\
            }   \
        }

#define READ_SETTINGS_FIELD_TEXT(__param,__baseStruct,__fieldName) \
        {   char * __sValue = NULL  \
            if (!(config_setting_lookup_string(__param, #__fieldName, &__sValue))) {   \
                snprintf(App.Msg, App.MsgSize, "READ_SETTINGS_FIELD_TEXT error : %s%s:%d%s\n", (char*)ANSI_COLOR_RED, (char*)config_error_text(&cfg), config_error_line(&cfg), (char*)ANSI_COLOR_RED );\
                vDisplayMessage(App.Msg);\
                retVal = -1;\
            } else {    \
                strncpy(__baseStruct.__fieldName, (char*)__sValue, sizeof(__baseStruct.__fieldName)) \
            }   \
        }


#define WRITE_SETTINGS_FIELD_INT(__param,__baseStruct, __fieldName) \
    setting = config_setting_add(__param, #__fieldName, CONFIG_TYPE_INT); \
    if(setting) \
        config_setting_set_int(setting, __baseStruct.__fieldName); \
    else {\
        snprintf(App.Msg, App.MsgSize, "WRITE_SETTINGS_FIELD_INT error : %s%s:%d%s\n", (char*)ANSI_COLOR_RED, (char*)config_error_text(&cfg), config_error_line(&cfg), (char*)ANSI_COLOR_RED );\
        vDisplayMessage(App.Msg);\
        retVal = -1;\
        }

#define WRITE_SETTINGS_FIELD_FLOAT(__param,__baseStruct,__fieldName) \
    setting = config_setting_add(__param, #__fieldName, CONFIG_TYPE_FLOAT); \
    if(setting) \
        config_setting_set_float(setting, __baseStruct.__fieldName); \
    else {\
        snprintf(App.Msg, App.MsgSize, "WRITE_SETTINGS_FIELD_FLOAT error : %s%s:%d%s\n", (char*)ANSI_COLOR_RED, (char*)config_error_text(&cfg), config_error_line(&cfg), (char*)ANSI_COLOR_RED );\
        vDisplayMessage(App.Msg);\
        retVal = -1;\
        }
#define WRITE_SETTINGS_FIELD_STRING(__param,__baseStruct,__fieldName) \
    setting = config_setting_add(__param, #__fieldName, CONFIG_TYPE_STRING); \
    if(setting) \
        config_setting_set_string(setting, (char*)__baseStruct.__fieldName); \
    else {\
        snprintf(App.Msg, App.MsgSize, "WRITE_SETTINGS_FIELD_STRING error : %s%s:%d%s\n", (char*)ANSI_COLOR_RED, (char*)config_error_text(&cfg), config_error_line(&cfg), (char*)ANSI_COLOR_RED );\
        vDisplayMessage(App.Msg);\
        retVal = -1;\
        }



#ifdef xBM_COMPILE
    #define CONFIG_FILE     (char*)"./XPSETTINGS.cfg"
#elif xCNC_COMPILE
    #define CONFIG_FILE     (char*)"./xCNC_SETTINGS.cfg"
#endif


int read_settings(void) {
    int ok, retVal = 1;
    const char *str = NULL, key[256], *localString = NULL;

    config_t cfg;
    config_setting_t *param = NULL, *act = NULL, *acts = NULL;


    config_init(&cfg);

    config_set_include_dir(&cfg, "/share");

    ok = config_read_file(&cfg, CONFIG_FILE);
    if (!ok) {
        snprintf(App.Msg, App.MsgSize, "error : %s%s:%d%s\n", (char*) ANSI_COLOR_RED, config_error_text(&cfg), config_error_line(&cfg), (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);
        return -1;
    }



    /* Output a list of all books in the inventory. */
    param = config_lookup(&cfg, "settings");
    if (param != NULL) {
        int count = config_setting_length(param);

#ifdef xBM_COMPILE
        READ_SETTINGS_FIELD_INT(param, machine.settings, startup_owens_cycles)
        READ_SETTINGS_FIELD_INT(param, machine.settings, startup_owens_delay_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, turnoff_owens_cycles)
        READ_SETTINGS_FIELD_INT(param, machine.settings, turnoff_owens_delay_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, owens_standby_cycles)
        READ_SETTINGS_FIELD_INT(param, machine.settings, owens_standby_delay_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, owens_towork_cycles)
        READ_SETTINGS_FIELD_INT(param, machine.settings, owens_towork_delay_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, initial_owens_cycles)
        READ_SETTINGS_FIELD_INT(param, machine.settings, chain_stepper1_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, chain_stepper2_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, chain_stepper3_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, trasf_x_forward_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, chain_trasf_z_down_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, chain_picker_open_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, chain_picker_close_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, pref_load_inside_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, pref_load_outside_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, pit_stopper_inside_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, pit_stopper_outside_pause_ms)
        READ_SETTINGS_FIELD_INT(param, machine.settings, aspirator_delay_ms)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, initial_owens_ratio)

        READ_SETTINGS_FIELD_INT(param, machine.statistic, bottles)
        READ_SETTINGS_FIELD_INT(param, machine.statistic, discharged)
        READ_SETTINGS_FIELD_INT(param, machine.statistic, errors)
        READ_SETTINGS_FIELD_INT(param, machine.statistic, mold_cycles)
        READ_SETTINGS_FIELD_INT(param, machine.statistic, preforms_blowed)
        READ_SETTINGS_FIELD_INT(param, machine.statistic, preforms_loaded)
        READ_SETTINGS_FIELD_INT(param, machine.statistic, warnings)
                
        READ_SETTINGS_FIELD_INT(param, machine.statistic, fatal_errors)
        READ_SETTINGS_FIELD_INT(param, machine.statistic, machine_stops)
                
        READ_SETTINGS_FIELD_INT(param, machine.statistic, machine_elapsed)
        READ_SETTINGS_FIELD_INT(param, machine.statistic, machine_running)
                
#elif xCNC_COMPILE
            
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, rapid_feed_X)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, rapid_feed_Y)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, rapid_feed_Z)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, rapid_feed_W)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, rapid_feed_T)

        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, mill_feed_mm_min_X)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, mill_feed_mm_min_Y)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, mill_feed_mm_min_Z)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, mill_feed_mm_min_W)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, mill_feed_mm_min_T)

        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, spindle_speed)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, spindle_power)
                
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, cam_ratio_X)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, cam_ratio_Y)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, cam_ratio_Z)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, cam_ratio_W)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, cam_ratio_T)

        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, max_weight)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, max_X)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, max_Y)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, max_Z)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, max_W)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, max_T)

        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, spindle_speed_toll)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, feed_toll)

    
        // tabella materiali
        READ_SETTINGS_FIELD_INT(param, machine.settings, num_materials)


        if (check_general_structure_allocated ( 0, (void**)&machine.settings.materials, sizeof (machine.settings.materials[0]), machine.settings.num_materials, &machine.settings.num_materials_allocated, 3, "Allocating materials table", NULL ) < 0) {
            return -1;
        }


        for (uint32_t i_mat=0; i_mat<machine.settings.num_materials; i_mat++) {

            READ_SETTINGS_FIELD_STRING(param, machine.settings.materials[i_mat], name)
            READ_SETTINGS_FIELD_STRING(param, machine.settings.materials[i_mat], desc)      
            READ_SETTINGS_FIELD_STRING(param, machine.settings.materials[i_mat], alias);

            READ_SETTINGS_FIELD_FLOAT(param, machine.settings.materials[i_mat], cut_speet)
            READ_SETTINGS_FIELD_FLOAT(param, machine.settings.materials[i_mat], sigma)
            READ_SETTINGS_FIELD_FLOAT(param, machine.settings.materials[i_mat], density)

            READ_SETTINGS_FIELD_INT(param, machine.settings.materials[i_mat], options)
        }

        // Testo del programma
        READ_SETTINGS_FIELD_STRING(param, machine.App.GCode, Content)
        
        gcode_update_content(machine.App.GCode.Content);

        
        READ_SETTINGS_FIELD_INT(param, machine.App.GCode, startRow)
                
        READ_SETTINGS_FIELD_INT(param, machine.App.GCodeCmd, curRow)
        READ_SETTINGS_FIELD_INT(param, machine.App.GCodeCmd, curRowChar)
                
        READ_SETTINGS_FIELD_INT(param, machine.App.GCode, numDisplayRows)

                
                
        // relink del contenuto
        gcode_update_start_row ( machine.App.GCode.startRow );
        
        
        // Calcolo parametri taglio/vita utensile
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, TaylorC)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, TaylorN)
    
        // Precisione interpolazioni
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, InterpolationPrecisionMM)
        READ_SETTINGS_FIELD_FLOAT(param, machine.settings, DefaultGap)

        READ_SETTINGS_FIELD_INT(param, machine.settings, DefaultGapCorrection)
        
        READ_SETTINGS_FIELD_INT(param, machine.settings, InterpolationPeriodMS)

#endif
                

                
    } else {

        snprintf(App.Msg, App.MsgSize, "%s settings not found %s\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RED);
        vDisplayMessage(App.Msg);

        retVal = -2;
    }





    acts = config_lookup(&cfg, "acts");
    if (acts != NULL) {
        int i_act, j, Id, count = config_setting_length(acts);

        for (i_act = 0; i_act < count; i_act++) {
            
            snprintf((char*) key, sizeof (key), "act-%d", i_act + 1);
            
            act = config_setting_lookup(acts, key);

            if (act) {
                bool bActFound = false;
                
                config_setting_lookup_int(act, "Id", &Id);

                for (j = 0; j < machine.num_actuator; j++) {

                    if (machine.actuator[j].Id == Id) {

                        bActFound = true;
                        
                        config_setting_lookup_string(act, "name", &localString); // if (localString) free((void*)localString);

                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], acc_auto1)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], speed_auto1)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], dec_auto1)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], force_auto1)

                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], acc_man1)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], speed_man1)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], dec_man1)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], force_man1)

                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], acc_auto2)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], speed_auto2)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], dec_auto2)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], force_auto2)

                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], acc_man2)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], speed_man2)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], dec_man2)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], force_man2)

                        READ_SETTINGS_FIELD_INT(act, machine.actuator[j], timeout1_ms)
                        READ_SETTINGS_FIELD_INT(act, machine.actuator[j], timeout2_ms)
                        READ_SETTINGS_FIELD_INT(act, machine.actuator[j], timewarn1_ms)
                        READ_SETTINGS_FIELD_INT(act, machine.actuator[j], timewarn2_ms)

                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], follow_error1)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], follow_error2)

                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], start_rpos_toll)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], end_rpos_toll)

                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], near_start_rposition)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], near_end_rposition)
                                
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], homing_offset_mm)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], homing_speed_rpm)
                        READ_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], homing_rated_torque)
                        READ_SETTINGS_FIELD_INT(act, machine.actuator[j], homing_timeout_ms)

                                
                        // Imposta l'equivalente in mm/s
                        actuator_speed_to_linear(&machine.actuator[j], machine.actuator[j].speed_auto1, &machine.actuator[j].speed_lin_auto1);
                        actuator_speed_to_linear(&machine.actuator[j], machine.actuator[j].speed_auto2, &machine.actuator[j].speed_lin_auto2);

                        break;
                    }
                }
                
                if (!bActFound) {
                    snprintf(App.Msg, App.MsgSize, "%s act.Id=%d not found %s\n", (char*) ANSI_COLOR_RED, Id, (char*) ANSI_COLOR_RED);
                    vDisplayMessage(App.Msg);
                    retVal = -4;
                }
                        
            } else {
                snprintf(App.Msg, App.MsgSize, "%s act %s not found %s\n", (char*) ANSI_COLOR_RED, key, (char*) ANSI_COLOR_RED);
                vDisplayMessage(App.Msg);
                retVal = -3;
            }
        }

        // crea lo specchio degli attuatori (per identificare i cambiamenti)
        do_acctuator_mirror();

    } else {

        snprintf(App.Msg, App.MsgSize, "%s acts not found %s\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RED);
        vDisplayMessage(App.Msg);
        retVal = -2;
    }
 

    config_destroy(&cfg);

    return retVal;
}




int write_settings(void) {
    config_t cfg;
    config_setting_t *root, *setting, *group, *act;
    int ok, retVal = 1;
    char key[256];
    

    config_init(&cfg);
    config_set_include_dir(&cfg, "/share");


    root = config_root_setting(&cfg);

    /* Add some settings to the configuration. */
    group = config_setting_add(root, "settings", CONFIG_TYPE_GROUP);

#ifdef xBM_COMPILE    

    WRITE_SETTINGS_FIELD_INT(group, machine.settings, startup_owens_cycles)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, startup_owens_delay_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, turnoff_owens_cycles)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, turnoff_owens_delay_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, owens_standby_cycles)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, owens_standby_delay_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, owens_towork_cycles)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, owens_towork_delay_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, initial_owens_cycles)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, chain_stepper1_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, chain_stepper2_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, chain_stepper3_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, trasf_x_forward_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, chain_trasf_z_down_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, chain_picker_open_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, chain_picker_close_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, pref_load_inside_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, pref_load_outside_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, pit_stopper_inside_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, pit_stopper_outside_pause_ms)
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, aspirator_delay_ms)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, initial_owens_ratio)

    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, bottles)
    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, discharged)
    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, errors)
    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, mold_cycles)
    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, preforms_blowed)
    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, preforms_loaded)
    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, warnings)

    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, fatal_errors)
    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, machine_stops)

    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, machine_elapsed)
    WRITE_SETTINGS_FIELD_INT(group, machine.statistic, machine_running)
                
#elif xCNC_COMPILE
            
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, rapid_feed_X)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, rapid_feed_Y)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, rapid_feed_Z)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, rapid_feed_W)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, rapid_feed_T)

    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, mill_feed_mm_min_X)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, mill_feed_mm_min_Y)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, mill_feed_mm_min_Z)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, mill_feed_mm_min_W)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, mill_feed_mm_min_T)

    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, spindle_speed)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, spindle_power)
            
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, cam_ratio_X)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, cam_ratio_Y)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, cam_ratio_Z)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, cam_ratio_W)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, cam_ratio_T)

    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, max_weight)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, max_X)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, max_Y)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, max_Z)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, max_W)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, max_T)
            
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, spindle_speed_toll)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, feed_toll)
            

        // tabella materiali
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, num_materials)
            
    for (uint32_t i_mat=0; i_mat<machine.settings.num_materials; i_mat++) {
        
        WRITE_SETTINGS_FIELD_STRING(group, machine.settings.materials[i_mat], name);
        WRITE_SETTINGS_FIELD_STRING(group, machine.settings.materials[i_mat], desc);        
        WRITE_SETTINGS_FIELD_STRING(group, machine.settings.materials[i_mat], alias);
        
        WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings.materials[i_mat], cut_speet)
        WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings.materials[i_mat], sigma)
        WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings.materials[i_mat], density)
                
    }

    
    // Ricostruisce il contenuto
    if (machine.App.GCode.ContentChanged) {
        gcode_rebuld_content_from_rows ();
    }

    // Testo del programma
    WRITE_SETTINGS_FIELD_STRING(group, machine.App.GCode, Content)
    

    WRITE_SETTINGS_FIELD_INT(group, machine.App.GCode, startRow)
            
    WRITE_SETTINGS_FIELD_INT(group, machine.App.GCodeCmd, curRow)
    WRITE_SETTINGS_FIELD_INT(group, machine.App.GCodeCmd, curRowChar)

    WRITE_SETTINGS_FIELD_INT(group, machine.App.GCode, numDisplayRows)
            
            
    // Calcolo parametri taglio/vita utensile
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, TaylorC)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, TaylorN)

    // Precisione interpolazioni
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, InterpolationPrecisionMM)
    WRITE_SETTINGS_FIELD_FLOAT(group, machine.settings, DefaultGap)

    WRITE_SETTINGS_FIELD_INT(group, machine.settings, DefaultGapCorrection)
            
    WRITE_SETTINGS_FIELD_INT(group, machine.settings, InterpolationPeriodMS)

            
#endif
            
            
            
            
    /* Add some settings to the configuration. */
    group = config_setting_add(root, "acts", CONFIG_TYPE_GROUP);

    if (group != NULL) {
        int j, Id;

        for (j = 0; j <  machine.num_actuator; j++) {

            snprintf(key, sizeof (key), "act-%d", j + 1);

            act = config_setting_add(group, key, CONFIG_TYPE_GROUP);

            if (act) {
                
                WRITE_SETTINGS_FIELD_INT(act, machine.actuator[j], Id)

                WRITE_SETTINGS_FIELD_STRING(act, machine.actuator[j], name)

                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], acc_auto1)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], speed_auto1)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], dec_auto1)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], force_auto1)

                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], acc_man1)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], speed_man1)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], dec_man1)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], force_man1)

                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], acc_auto2)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], speed_auto2)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], dec_auto2)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], force_auto2)

                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], acc_man2)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], speed_man2)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], dec_man2)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], force_man2)

                WRITE_SETTINGS_FIELD_INT(act, machine.actuator[j], timeout1_ms)
                WRITE_SETTINGS_FIELD_INT(act, machine.actuator[j], timeout2_ms)
                WRITE_SETTINGS_FIELD_INT(act, machine.actuator[j], timewarn1_ms)
                WRITE_SETTINGS_FIELD_INT(act, machine.actuator[j], timewarn2_ms)

                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], follow_error1)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], follow_error2)

                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], start_rpos_toll)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], end_rpos_toll)

                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], near_start_rposition)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], near_end_rposition)
                        
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], homing_offset_mm)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], homing_speed_rpm)
                WRITE_SETTINGS_FIELD_FLOAT(act, machine.actuator[j], homing_rated_torque)
                        
                WRITE_SETTINGS_FIELD_INT(act, machine.actuator[j], homing_timeout_ms)

            } else {

                snprintf(App.Msg, App.MsgSize, "error : %s%s:%d%s\n", (char*) ANSI_COLOR_RED, config_error_text(&cfg), config_error_line(&cfg), (char*) ANSI_COLOR_RED);
                vDisplayMessage(App.Msg);
                retVal = -3;
                
            }
        }

        
        

    } else {
        snprintf(App.Msg, App.MsgSize, "error : %s%s:%d%s\n", (char*) ANSI_COLOR_RED, config_error_text(&cfg), config_error_line(&cfg), (char*) ANSI_COLOR_RED);
        vDisplayMessage(App.Msg);
        retVal = -2;
    }

    

            
    config_write_file(&cfg, CONFIG_FILE);

    config_destroy(&cfg);

    if (retVal < 0) {
        snprintf(App.Msg, App.MsgSize, "error : %s%s:%d%s\n", (char*) ANSI_COLOR_RED, config_error_text(&cfg), config_error_line(&cfg), (char*) ANSI_COLOR_RED);
        vDisplayMessage(App.Msg);
    }

    return retVal;
}