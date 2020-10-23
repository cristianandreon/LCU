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



int mill_set_tool_param ( char *tecnlogy ) {   
    if (strcmpi(tecnlogy, "WC") == 0) {
        machine.settings.TaylorN = 0.28;    // vidia
    } else if (strcmpi(tecnlogy, "HSS") == 0) {
        machine.settings.TaylorN = 0.12f;    // superrapidi
    } else if (strcmpi(tecnlogy, "CEAMIC") == 0) {
        machine.settings.TaylorN = 0.70f;    // Ceramici
    }    
    return 1;
}


int mill_cal_tool_life ( float cut_speed_mt_min, float *tool_life_min ) {
    *tool_life_min = (float)pow( (double)machine.settings.TaylorC / cut_speed_mt_min, (double)(1.0 / (double)machine.settings.TaylorN)); 
    return 1;
}


// Profondita passata
int mill_cal_depth_step ( float tool_dia, float tool_corner_rad, float rugosity, float cut_speed_mt_min, float *depth_step_mm, float *feed_mm_turn, float *spingle_speed_rpm  ) {
    
    float KC04 = 2100.0f; // MPA
    float x = 0.16f;
    float k = 60.0f;    // 60°
    
    float power = machine.settings.spindle_power * 0.8f;
    float cut_force_n = power * 60.0f / cut_speed_mt_min;
    float feed_ratio = 0.0f;
            
    *spingle_speed_rpm = cut_speed_mt_min * 1000.0f / (PIGRECO * tool_dia);
            
    *feed_mm_turn = sqrt(32.0 * rugosity * tool_corner_rad / 1000.0f);
    
    feed_ratio = tool_corner_rad / *feed_mm_turn;
    if (feed_ratio < 0.5 && feed_ratio >= 0.2) {
        // ok
    } else {
    }
    
    *depth_step_mm = cut_force_n / KC04 / pow (0.4, x) / pow (*feed_mm_turn, 1.0-x) * (pow (sinf(k), x));
    
    return 1;
}
