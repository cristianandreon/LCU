#ifndef MEMORY_MANAGER_H


    #ifdef EXTERN
    	#ifdef __cplusplus
        	#define MEMORY_MANAGER_H 
    	#else
        	#define MEMORY_MANAGER_H extern
    	#endif
    #else
        #define MEMORY_MANAGER_H
    #endif


    #ifndef UINT
        #define MDB_CHAR    char
    #endif
    #ifndef UINT
        #define UINT    uint32_t
    #endif
    #ifndef UINT
        #define USHORT  uint16_t
    #endif
    #ifndef UINT
        #define HWND    void *
    #endif

#ifdef __cplusplus
    extern "C" {
#endif
        
        MEMORY_MANAGER_H void CpyStr(MDB_CHAR **Target, MDB_CHAR *Source, UINT *TargetSize);
        MEMORY_MANAGER_H void AddStr(MDB_CHAR **Target, MDB_CHAR *Source, UINT *TargetSize);
        MEMORY_MANAGER_H int check_str ( MDB_CHAR *_str1, MDB_CHAR *_str2 );
        MEMORY_MANAGER_H int check_stri ( MDB_CHAR *_str1, MDB_CHAR *_str2 );
        MEMORY_MANAGER_H int add_string_to_array(MDB_CHAR ***ArrayString, UINT *NumArrayString, UINT *NumArrayStringAllocated, UINT MaxArrayItem, UINT MaxArrayItemSize, MDB_CHAR *StringToAdd, HWND ptr_hwnd, int Mode);
        MEMORY_MANAGER_H int check_general_short_structure_allocated(int PtrCurWrk, void **StructureData, UINT StructureSize, USHORT NumSItem, USHORT *NumSItemAllocated, UINT NumGapItem, MDB_CHAR *FeatureName, HWND ptr_hwnd);
        MEMORY_MANAGER_H int check_general_structure_allocated(int PtrCurWrk, void **StructureData, UINT StructureSize, UINT NumItem, UINT *NumItemAllocated, UINT NumGapItem, MDB_CHAR *FeatureName, HWND ptr_hwnd);
        MEMORY_MANAGER_H int check_general_array_allocated(int PtrCurWrk, void **ArrayData, UINT ArraySize, UINT NumItem, UINT *NumItemAllocated, UINT NumGapItem, MDB_CHAR *FeatureName, HWND ptr_hwnd);
        MEMORY_MANAGER_H int general_structure_operation(int PtrCurWrk, void **StructureData, UINT StructureSize, UINT *NumItem, UINT *NumItemAllocated, UINT NumGapItem, MDB_CHAR *FeatureName, void *NewStructureData, UINT NumItemToModify, UINT StartItemToModify, MDB_CHAR ModifyType, HWND ptr_hwnd);
        
        MEMORY_MANAGER_H int add_array_to_array ( UINT *FromArray, UINT NumFromArray, UINT **ToArray, UINT *NumToArray, UINT *NumToArrayAllocated, HWND ptr_hwnd, int Mode );
        MEMORY_MANAGER_H int add_array_to_string_array ( MDB_CHAR *FromString, MDB_CHAR ***ToArray, UINT *NumToArray, UINT *NumToArrayAllocated, HWND ptr_hwnd, int Mode );
        MEMORY_MANAGER_H int add_string_to_array(MDB_CHAR ***ArrayString, UINT *NumArrayString, UINT *NumArrayStringAllocated, UINT MaxArrayItem, UINT MaxArrayItemSize, MDB_CHAR *StringToAdd, HWND ptr_hwnd, int Mode);
        MEMORY_MANAGER_H int create_array_from_structure ( char *PtrArray, UINT NumArray, UINT ArrayItemStep, UINT ArrayItemSize, UINT **OutArray, UINT *NumOutArray, UINT *NumOutArrayAllocated, HWND ptr_hwnd, int Mode );
        MEMORY_MANAGER_H int create_table_from_array (MDB_CHAR **SourceArrayString, UINT NumSourceArrayString, MDB_CHAR **OutString, UINT *OutStringAllocated );
        MEMORY_MANAGER_H int duplicate_fields_array ( MDB_CHAR **FieldsValue, UINT NumFieldsValue, MDB_CHAR ***FillingArray, UINT *NumFillingArrayAllocated, HWND ptr_hwnd, int Mode );

        MEMORY_MANAGER_H int free_fields_array ( MDB_CHAR ***FillingArray, UINT NumFillingArrayAllocated );
        MEMORY_MANAGER_H int free_only_fields_array ( MDB_CHAR **FillingArray, UINT NumFillingArrayAllocated );
        MEMORY_MANAGER_H int create_array_from_string ( MDB_CHAR *PtrString, MDB_CHAR ***OutArray, UINT *NumOutArray, UINT *NumOutArrayAllocated, MDB_CHAR *SeparatorChar, UINT NumSeparatorChar, HWND ptr_hwnd, int Mode );



#ifdef __cplusplus
    }
#endif
    
#endif