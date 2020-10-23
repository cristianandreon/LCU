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


#include "./../xBM-Logic/logic_precomp.h"




    
extern "C" void CpyStr(MDB_CHAR **Target, MDB_CHAR *Source, uint32_t *TargetSize) {

    if (Target) {
        if (Source == NULL) {
            if (*Target) {
                **Target = 0;
            } else {
                if (TargetSize) {
                    if (*TargetSize) {
                        *TargetSize = 0;
                    }
                }
            }
            return;
        }

        if (TargetSize) {
            if (strlen(Source) + SAFE_STRING_SIZE >= *TargetSize) {
                *TargetSize = strlen(Source) + DELTA_STRING_DIM;
                *Target = (MDB_CHAR *) realloc(*Target, *TargetSize);
                if (*Target == NULL) {
                    *TargetSize = 0;
                    return;
                }
            }
        } else {
            return;
        }


        if (*Target) {
            strcpy_s(*Target, *TargetSize, Source);
        } else {
            if (TargetSize) {
                if (*TargetSize != 0) {
#ifdef MULTIDB_COMPILE
                    MDB_CHAR str[256];
                    if (App.CurLanguage == 1) {
                        sprintf_s(str, sizeof (str), "CpyStr : pointer disagree with allocated definition!");
                    } else if (App.CurLanguage == 2) {
                        sprintf_s(str, sizeof (str), "CpyStr : puntatore in disaccordo con la dimensione allocata!");
                    }
                    MultiDbMessage(0, GetFocus(), 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#endif
                    *TargetSize = 0;
                }
            }
        }
    }


    return;
}

void AddStr(MDB_CHAR **Target, MDB_CHAR *Source, uint32_t *TargetSize) {
    MDB_CHAR *old_ptr = NULL;

    if (Target) {

        if (Source == NULL) {
            return;
        }



        if (TargetSize) {
            uint32_t LocalSizeTarget = 0, len = 0;


            if (*Target) {
                len = strlen(*Target);
            } else {
                len = 0;
            }

            if (len + strlen(Source) + SAFE_STRING_SIZE >= *TargetSize) {
                *TargetSize = len + strlen(Source) + DELTA_STRING_DIM + len / 5;
                old_ptr = *Target;
                *Target = (MDB_CHAR *) malloc(*TargetSize);
                if (!(*Target)) {
                    *TargetSize = 0;
                    return;
                }
                if (old_ptr) {
                    memcpy((void*) *Target, old_ptr, len);
                    memset((void*) (*Target + len), 0, (*TargetSize - len));
                    free(old_ptr);
                } else {
                    memset((void*) *Target, 0, (*TargetSize));
                }
            }

            if (*Target) {
                strcat_s(*Target, *TargetSize, Source);
            } else {
                if (TargetSize) {
                    if (*TargetSize != 0) {
#ifdef MULTIDB_COMPILE
                        MDB_CHAR str[256];
                        if (App.CurLanguage == 1) {
                            sprintf_s(str, sizeof (str), "AddStr : pointer disagree with allocated definition!");
                        } else if (App.CurLanguage == 2) {
                            sprintf_s(str, sizeof (str), "AddStr : puntatore in disaccordo con la dimensione allocata!");
                        }
                        MultiDbMessage(0, GetFocus(), 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#endif
                        *TargetSize = 0;
                    }
                }
            }
        }
    }


    return;
}






// Ritorna 0 se le stringhe coincidono
int check_str ( MDB_CHAR *_str1, MDB_CHAR *_str2 )
{ uint32_t x;


if (_str1 == NULL && _str2 != NULL) {
        if (_str2[0] != 0) {
                return 1;
                }
        }

if (_str1 != NULL && _str2 == NULL) {
        if (_str1[0] != 0) {
                return 1;
                }
        }

if (_str1 != NULL && _str2 != NULL) {
        x = 0;
        for (;;) {
                if (_str1[x] != _str2[x]) {
			return 1;
			} else if (_str1[x] == 0) {
			if (_str2[x] == 0) {
				return 0;
				} else {
				return 1;
				}
			} else if (_str2[x] == 0) {
			if (_str1[x] == 0) {
				return 0;
				} else {
				return 1;
				}
			}
                x++;
                }
        }

return 0;
}

// Ritorna 0 se le stringhe coincidono, versione NON sensitiva
int check_stri ( MDB_CHAR *_str1, MDB_CHAR *_str2 )
{


if (_str1 == NULL && _str2 != NULL) {
        if (_str2[0]) {
                return 1;
                } else {
		return 0;
		}
        }

if (_str1 != NULL && _str2 == NULL) {
        if (_str1[0]) {
		return 1;
		} else {
		return 0;
		}
	}

if (_str1 != NULL && _str2 != NULL) {
        return strcmpi (_str1, _str2);
        }

return 0;
}










// Mode & 4	->	Controlla se la stringa è gia' contenuta (Insensitive case)
// Mode & 2	->	Controlla se la stringa è gia' contenuta (Sensitive case)
// Mode & 1	->	Array non ridimensionabile

int add_string_to_array(MDB_CHAR ***ArrayString, uint32_t *NumArrayString, uint32_t *NumArrayStringAllocated,
        uint32_t MaxArrayItem, uint32_t MaxArrayItemSize,
        MDB_CHAR *StringToAdd, HWND ptr_hwnd, int Mode) {
    uint32_t i = 0;
    MDB_CHAR FixedSize = 0;
    MDB_CHAR LocalMessageString[512];



    if (!ArrayString) return -1;
    if (!NumArrayString) return -1;
    if (!StringToAdd) return -1;

    if (Mode & 2) {
        // Controlla se la stringa è gia' contenuta
        for (i = 0; i<*NumArrayString; i++) {
            if (!check_str(ArrayString[0][i], StringToAdd)) {
                return 0;
            }
        }
    } else if (Mode & 4) {
        for (i = 0; i<*NumArrayString; i++) {
            if (!check_stri(ArrayString[0][i], StringToAdd)) {
                return 0;
            }
        }
    }

    if (MaxArrayItemSize) {
        FixedSize = 1;
        if (strlen(StringToAdd) >= MaxArrayItemSize) {
            StringToAdd[MaxArrayItemSize - 1] = 0;
        }
    }



    if (Mode & 1) {
        // Array non ridimensionabile
        if (*NumArrayString >= MaxArrayItem) {
            COPY_POINTER(ArrayString[0][*NumArrayString - 1], StringToAdd);
        } else {
            if (FixedSize) {
                if (ArrayString[0]) {
                    if (ArrayString[0][i]) {
                        strcpy_s(ArrayString[0][i], MaxArrayItemSize, StringToAdd);
                    }
                } else {
                    if (ArrayString) {
                        if (ArrayString[0]) {
                            COPY_ARRAY(ArrayString[0][i], StringToAdd);
                        }
                    }
                }
                *NumArrayString = *NumArrayString + 1;
            }
        }
    } else {
        if (check_general_structure_allocated(0, (void**)ArrayString, (uint32_t)sizeof (ArrayString[0][0]), (*NumArrayString + 1), NumArrayStringAllocated, 2, (char*)"add string to array", ptr_hwnd) < 0) {
            sprintf_s(LocalMessageString, sizeof (LocalMessageString), "add_string_to_array : Unable to allocate data!");
#ifdef MULTIDB_COMPILE
            MultiDbMessage(0, GetFocus(), 0 + 0, LocalMessageString, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#endif
            return -1;
        }
        COPY_POINTER(ArrayString[0][*NumArrayString], StringToAdd);
        *NumArrayString = *NumArrayString + 1;
    }


    return 1;
}



int check_general_structure_allocated(int PtrCurWrk, void **StructureData,
        uint32_t StructureSize, uint32_t NumItem, uint32_t *NumItemAllocated,
        uint32_t NumGapItem, MDB_CHAR *FeatureName, HWND ptr_hwnd) {
    uint32_t prev_item = 0;
    MDB_CHAR *old_ptr = NULL, *new_ptr = NULL, str[512];
    int res = 0, res_add = -1;


    if (!StructureData) {
#ifdef MULTIDB_COMPILE
        if (App.CurLanguage == 1) {
            sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - StructureData is NULL pointer!", FeatureName);
        } else if (App.CurLanguage == 2) {
            sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - StructureData è un puntatore non valido!", FeatureName);
        }

        MultiDbMessage(PtrCurWrk, GetFocus(), 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#endif
        return -2;
    }
    if (!NumItemAllocated) {
#ifdef MULTIDB_COMPILE
        if (App.CurLanguage == 1) {
            sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - NumItemAllocated is NULL pointer!", FeatureName);
        } else if (App.CurLanguage == 2) {
            sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - NumItemAllocated è un puntatore non valido!", FeatureName);
        }
        MultiDbMessage(PtrCurWrk, GetFocus(), 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#endif
        return -3;
    }
    if (!StructureSize) {
#ifdef MULTIDB_COMPILE
        if (App.CurLanguage == 1) {
            sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - StructureSize is NULL pointer!", FeatureName);
        } else if (App.CurLanguage == 2) {
            sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - StructureSize è un puntatore non valido!", FeatureName);
        }
        MultiDbMessage(PtrCurWrk, ptr_hwnd, 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#endif
        return -4;
    }


start_execute_function:

    if (NumItem >= *NumItemAllocated) {

        // Riallocazione
        prev_item = *NumItemAllocated;
        old_ptr = (MDB_CHAR *) (StructureData[0]);


start_allocate_memory:

        *NumItemAllocated = NumItem + NumGapItem;

        {

            new_ptr = (MDB_CHAR*)malloc(StructureSize * NumItemAllocated[0] + 1);


            if (!(new_ptr)) {
                MDB_CHAR DummyStr[256];
                *NumItemAllocated = prev_item;
                if (FeatureName) {
                    strcpy_s(str, sizeof (str), "");
                    strcat_s(str, sizeof (str), FeatureName);
#ifdef MULTIDB_COMPILE
                    ViewLastError(ptr_hwnd, 0 + 1);
                    if (App.CurLanguage == 1) {
                        sprintf_s(DummyStr, sizeof (DummyStr), " [ %s ] - Unable to allocate %d Kb! Do you want to rety ?", FeatureName, StructureSize * (NumItem + NumGapItem) / 1024);
                    } else if (App.CurLanguage == 2) {
                        sprintf_s(DummyStr, sizeof (DummyStr), " [ %s ] - Impossibile allocare %d Kb! Vuoi riprovare ?", FeatureName, StructureSize * (NumItem + NumGapItem) / 1024);
                    }
                    strcat_s(str, sizeof (str), DummyStr);
#else
                    sprintf_s(DummyStr, sizeof (DummyStr), " [ %s ] - Unable to allocate %d Kb!\n\n Do you want to rety ?", FeatureName, StructureSize * (NumItem + NumGapItem) / 1024);
                    strcat_s(str, sizeof (str), DummyStr);
#endif

                } else {
#ifdef MULTIDB_COMPILE
                    ViewLastError(ptr_hwnd, 0 + 1);
                    if (App.CurLanguage == 1) {
                        sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - Unable to allocate %d Kb!\n\n Do you want to rety ?", FeatureName, StructureSize * (*NumItemAllocated) / 1024);
                    } else if (App.CurLanguage == 2) {
                        sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - Impossibile allocare %d Kb!\n\n Vuoi riprovare ?", FeatureName, StructureSize * (*NumItemAllocated) / 1024);
                    }
#else
                    sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - Unable to allocate %d Kb!\n\nDo you want to rety ?", FeatureName, StructureSize * (*NumItemAllocated) / 1024);
#endif
                }

#ifdef MULTIDB_COMPILE
                res = MultiDbMessage(PtrCurWrk, ptr_hwnd, 0 + 2 + 128, str, (MDB_CHAR *) MULTIDB_ERROR, MB_YESNO | MB_ICONHAND);
#else
#ifdef _CONSOLE
                res = MessageBox(ptr_hwnd, str, "Memory allocator", MB_YESNO | MB_ICONHAND);
#endif
#endif


                if (res == IDYES) {
                    NumGapItem = 0;
                    goto start_allocate_memory;
                } else {
                    FREE_POINTER(new_ptr);
                    return -1;
                }
            }

            if (prev_item && old_ptr) {
                memcpy(new_ptr, old_ptr, StructureSize * prev_item);
            }

            if (prev_item) {
                memset(((MDB_CHAR*) (new_ptr + prev_item * StructureSize)), 0, StructureSize * (*NumItemAllocated - prev_item));
            } else {
                memset(new_ptr, 0, StructureSize * (*NumItemAllocated));
            }

            StructureData[0] = new_ptr;
            new_ptr = NULL;

            if (old_ptr) {
                free(old_ptr);
            }
            old_ptr = NULL;
        }

    } else {
        if (!(StructureData[0])) {
#ifdef MULTIDB_COMPILE
            if (App.CurLanguage == 1) {
                sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - StructureDataAllocated is greated than 0, but StructureData is NULL pointer!", FeatureName);
            } else if (App.CurLanguage == 2) {
                sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - StructureDataAllocated e' maggiore di 0, ma StructureData è un puntatore non valido!", FeatureName);
            }
#else
            sprintf_s(str, sizeof (str), "check_general_structure_allocated : [ %s ] - StructureDataAllocated is greated than 0, but StructureData is NULL pointer!", FeatureName);
#endif
#ifdef MULTIDB_COMPILE
            MultiDbMessage(PtrCurWrk, ptr_hwnd, 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#else
            // res = MessageBox(ptr_hwnd, str, "Memory allocator", MB_OK | MB_ICONHAND);
#endif
            *NumItemAllocated = 0;
            goto start_execute_function;
        }
        return 0;
    }



    return 1;
}

int check_general_array_allocated(int PtrCurWrk, void **ArrayData,
        uint32_t ArraySize, uint32_t NumItem, uint32_t *NumItemAllocated,
        uint32_t NumGapItem, MDB_CHAR *FeatureName, HWND ptr_hwnd) {
    uint32_t prev_item = 0;
    MDB_CHAR *old_ptr = NULL, *new_ptr = NULL, str[512];
    int res = 0;


    if (!ArrayData) {
#ifdef MULTIDB_COMPILE
        if (App.CurLanguage == 1) {
            sprintf_s(str, sizeof (str), "check_general_array_allocated : ArrayData is NULL pointer!");
        } else if (App.CurLanguage == 2) {
            sprintf_s(str, sizeof (str), "check_general_array_allocated : ArrayData è un puntatore non valido!");
        }
        MultiDbMessage(PtrCurWrk, ptr_hwnd, 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#else
        sprintf_s(str, sizeof (str), "check_general_array_allocated : ArrayData is NULL pointer!");
        // res = MessageBox(ptr_hwnd, str, "Memory allocator", MB_OK | MB_ICONHAND);
#endif
        return -2;
    }
    if (!NumItemAllocated) {
#ifdef MULTIDB_COMPILE
        if (App.CurLanguage == 1) {
            sprintf_s(str, sizeof (str), "check_general_array_allocated : NumItemAllocated is NULL pointer!");
        } else if (App.CurLanguage == 2) {
            sprintf_s(str, sizeof (str), "check_general_array_allocated : NumItemAllocated è un puntatore non valido!");
        }
        MultiDbMessage(PtrCurWrk, GetFocus(), 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#else
        sprintf_s(str, sizeof (str), "check_general_array_allocated : NumItemAllocated is NULL pointer!");
        // res = MessageBox(ptr_hwnd, str, "Memory allocator", MB_OK | MB_ICONHAND);
#endif

        return -3;
    }
    if (!ArraySize) {
#ifdef MULTIDB_COMPILE
        if (App.CurLanguage == 1) {
            sprintf_s(str, sizeof (str), "check_general_array_allocated : ArraySize is NULL pointer!");
        } else if (App.CurLanguage == 2) {
            sprintf_s(str, sizeof (str), "check_general_array_allocated : ArraySize è un puntatore non valido!");
        }
        MultiDbMessage(PtrCurWrk, ptr_hwnd, 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#else
        sprintf_s(str, sizeof (str), "check_general_array_allocated : ArraySize is NULL pointer!");
        // res = MessageBox(ptr_hwnd, str, "Memory allocator", MB_OK | MB_ICONHAND);
#endif
        return -4;
    }


start_execute_function:

    if (NumItem >= *NumItemAllocated) {

        // Riallocazione
        prev_item = *NumItemAllocated;
        old_ptr = (MDB_CHAR *) (ArrayData[0]);


        *NumItemAllocated = NumItem + NumGapItem;

start_allocate_memory:

        new_ptr = (MDB_CHAR*)malloc(ArraySize * (*NumItemAllocated));

        if (!new_ptr) {
            *NumItemAllocated = prev_item;
            if (FeatureName) {
                MDB_CHAR DummyStr[256];
                strcpy_s(str, sizeof (str), "");
                strcat_s(str, sizeof (str), FeatureName);
#ifdef MULTIDB_COMPILE
                if (App.CurLanguage == 1) {
                    sprintf_s(DummyStr, sizeof (DummyStr), " : Unable to allocate %d byte(s)! Do you want to rety ?", ArraySize * (*NumItemAllocated));
                } else if (App.CurLanguage == 2) {
                    sprintf_s(DummyStr, sizeof (DummyStr), " : Impossibile allocare %d byte(s)! Vuoi riprovare ?", ArraySize * (*NumItemAllocated));
                }
                strcat_s(str, sizeof (str), DummyStr);
#else
                sprintf_s(DummyStr, sizeof (DummyStr), " : Unable to allocate %d byte(s)! Do you want to rety ?", ArraySize * (*NumItemAllocated));
                strcat_s(str, sizeof (str), DummyStr);
#endif

            } else {
#ifdef MULTIDB_COMPILE
                if (App.CurLanguage == 1) {
                    sprintf_s(str, sizeof (str), "check_general_structure_allocated : Unable to allocate %d byte(s)! Do you want to rety ?", ArraySize * (*NumItemAllocated));
                } else if (App.CurLanguage == 2) {
                    sprintf_s(str, sizeof (str), "check_general_structure_allocated : Impossibile allocare %d byte(s)! Vuoi riprovare ?", ArraySize * (*NumItemAllocated));
                }
#else
                sprintf_s(str, sizeof (str), "check_general_structure_allocated : Unable to allocate %d byte(s)! Do you want to rety ?", ArraySize * (*NumItemAllocated));
#endif
            }
#ifdef MULTIDB_COMPILE
            res = MultiDbMessage(PtrCurWrk, ptr_hwnd, 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_YESNO | MB_ICONHAND);
#else
            // res = MessageBox(ptr_hwnd, str, "Memory allocator", MB_YESNO | MB_ICONHAND);
#endif

            if (res == IDYES) {
                goto start_allocate_memory;
            } else {
                FREE_POINTER(new_ptr);
                return -1;
            }
        }

        if (prev_item > 0 && old_ptr) {
            memcpy(new_ptr, old_ptr, ArraySize * prev_item);
        }

        if (prev_item > 0) {
            memset(((MDB_CHAR*) new_ptr + prev_item * ArraySize), 0, ArraySize * (*NumItemAllocated - prev_item));
        } else {
            memset(new_ptr, 0, ArraySize * (*NumItemAllocated));
        }

        ArrayData[0] = new_ptr;
        new_ptr = NULL;

        if (old_ptr) {
            free(old_ptr);
        }
        old_ptr = NULL;

    } else {
        if (!ArrayData[0]) {
#ifdef MULTIDB_COMPILE
            if (App.CurLanguage == 1) {
                sprintf_s(str, sizeof (str), "check_general_array_allocated : ArrayDataAllocated is greated than 0, but ArrayData is NULL pointer!");
            } else if (App.CurLanguage == 2) {
                sprintf_s(str, sizeof (str), "check_general_array_allocated : ArrayDataAllocated e' maggiore di 0, ma ArrayData è un puntatore non valido!");
            }
            MultiDbMessage(PtrCurWrk, ptr_hwnd, 0 + 2, str, (MDB_CHAR *) MULTIDB_ERROR, MB_OK | MB_ICONHAND);
#else
            sprintf_s(str, sizeof (str), "check_general_array_allocated : ArrayDataAllocated is greated than 0, but ArrayData is NULL pointer!");
            // MessageBox(ptr_hwnd, str, "Memory allocator", MB_OK | MB_ICONHAND);
#endif

            *NumItemAllocated = 0;
            goto start_execute_function;
        }
        return 0;
    }



    return 1;
}

int general_structure_operation(int PtrCurWrk, void **StructureData,
        uint32_t StructureSize, uint32_t *NumItem, uint32_t *NumItemAllocated,
        uint32_t NumGapItem, MDB_CHAR *FeatureName,
        void *NewStructureData, uint32_t NumItemToModify, uint32_t StartItemToModify,
        MDB_CHAR ModifyType, HWND ptr_hwnd) {
    MDB_CHAR str[512];
    int res = 0;



    if (!StructureData) {
        return -2;
    }

    if (!NumItem) {
        return -3;
    }

    if (!NumItemAllocated) {
        return -3;
    }
    if (!StructureSize) {
        return -4;
    }


    if (ModifyType == GENERAL_STRUCTURE_INSERT) {
        if (!NumItemToModify) {
            return 0;
        }

start_execute_insert:

        res = check_general_structure_allocated(PtrCurWrk, StructureData, StructureSize, *NumItem + NumItemToModify, NumItemAllocated, NumGapItem, FeatureName, ptr_hwnd);
        if (res < 0) {
            return -1;
        }

        // Esecuzione shift
        if (StartItemToModify > *NumItem) {
            StartItemToModify = *NumItem;
        }

        memmove(((MDB_CHAR*) StructureData[0] + (StartItemToModify + NumItemToModify) * StructureSize), ((MDB_CHAR*) StructureData[0] + (StartItemToModify) * StructureSize), (*NumItem - StartItemToModify) * StructureSize);

        // Copia struttura da inserire
        if (NewStructureData) {
            memcpy(((MDB_CHAR*) StructureData[0] + (StartItemToModify) * StructureSize), NewStructureData, NumItemToModify * StructureSize);
        } else {
            memset(((MDB_CHAR*) StructureData[0] + (StartItemToModify) * StructureSize), 0, NumItemToModify * StructureSize);
        }

        *NumItem = *NumItem + NumItemToModify;

    } else if (ModifyType == GENERAL_STRUCTURE_SHIFT) {

        uint32_t NumItemToShift;

        if (!NumItemToModify) {
            return 0;
        }


        if ((int) NumItemToModify > 0) {
            goto start_execute_insert;
        }


        NumItemToShift = abs((int) NumItemToModify);

        // Esecuzione shift
        if (StartItemToModify > *NumItem) {
            NumItemToShift = *NumItem;
        }
        if (StartItemToModify + NumItemToShift > *NumItem) {
            NumItemToShift = *NumItem - StartItemToModify;
        }

        if (StructureData[0]) {
            memmove(((MDB_CHAR*) StructureData[0] + (StartItemToModify) * StructureSize), ((MDB_CHAR*) StructureData[0] + (StartItemToModify + NumItemToShift) * StructureSize), (*NumItem - (StartItemToModify + NumItemToShift)) * StructureSize);

            memset(((MDB_CHAR*) StructureData[0] + (*NumItem - NumItemToShift) * StructureSize), 0, NumItemToShift * StructureSize);
        }

        *NumItem = *NumItem - NumItemToShift;

    } else if (ModifyType == GENERAL_STRUCTURE_MOVE_UP) {
        MDB_CHAR *OldStructureData = (MDB_CHAR *) malloc(StructureSize + 1);

        if (StartItemToModify) {
            if (StartItemToModify + 1 <= *NumItem) {
                if (OldStructureData) {
                    memcpy((MDB_CHAR*) OldStructureData, ((MDB_CHAR*) StructureData[0] + (StartItemToModify - 1) * StructureSize), StructureSize);
                }
                memcpy(((MDB_CHAR*) StructureData[0] + (StartItemToModify - 1) * StructureSize), ((MDB_CHAR*) StructureData[0] + (StartItemToModify) * StructureSize), StructureSize);
                if (OldStructureData) {
                    memcpy(((MDB_CHAR*) StructureData[0] + StartItemToModify * StructureSize), (MDB_CHAR*) OldStructureData, StructureSize);
                } else {
                    memset(((MDB_CHAR*) StructureData[0] + StartItemToModify * StructureSize), 0, StructureSize);
                }
            }
        }
        FREE_POINTER(OldStructureData);


    } else if (ModifyType == GENERAL_STRUCTURE_MOVE_DOWN) {

        MDB_CHAR *OldStructureData = (MDB_CHAR *) malloc(StructureSize + 1);


        if (StartItemToModify + 1 < *NumItem) {

            if (OldStructureData) {
                memcpy((MDB_CHAR*) OldStructureData, ((MDB_CHAR*) StructureData[0] + (StartItemToModify) * StructureSize), StructureSize);
            }
            memcpy(((MDB_CHAR*) StructureData[0] + (StartItemToModify) * StructureSize), ((MDB_CHAR*) StructureData[0] + (StartItemToModify + 1) * StructureSize), StructureSize);
            if (OldStructureData) {
                memcpy(((MDB_CHAR*) StructureData[0] + (StartItemToModify + 1) * StructureSize), (MDB_CHAR*) OldStructureData, StructureSize);
            } else {
                memset(((MDB_CHAR*) StructureData[0] + (StartItemToModify + 1) * StructureSize), 0, StructureSize);
            }
        }

        FREE_POINTER(OldStructureData);
    }


    return 1;
}






////////////////////////////////
// Arrays funcs
//


// Mode & BIT3	->	Aggiungi come 1B (sommando 1)
// Mode & BIT2	->	Aggiungi da tipo dato 'int'
// Mode & BIT1	->	Aggiungi solo se inesistente
int add_array_to_array ( uint32_t *FromArray, uint32_t NumFromArray, uint32_t **ToArray, uint32_t *NumToArray, uint32_t *NumToArrayAllocated, HWND ptr_hwnd, int Mode )
{


        if (ToArray) {

		if (FromArray) {
			int *FromIntegerArray = NULL;

			if (Mode & BIT2) {
				// Aggiungi da tipo dato 'int'
				FromIntegerArray = (int*)FromArray;
				}

			if (NumFromArray) {
				uint32_t i, j, NumFromArrayAdded = 0;


				if (check_general_structure_allocated ( 0, (void**)ToArray, sizeof (ToArray[0][0]), NumToArray[0]+NumFromArray, NumToArrayAllocated, 1, (char*)"Allocating result array", ptr_hwnd ) < 0) {
					MDB_CHAR str[512];
					sprintf_s ( str, sizeof(str), "Unable to allocate result array!");
					#ifdef _UtilityMessageFuncKey
						MultiDbMessage ( 0, NULL, 0+0, str, (MDB_CHAR *)MULTIDB_ERROR, MB_OK | MB_ICONHAND);
						#endif
					return -1;
					}

				for (i=0; i<NumFromArray; i++) {
					BOOL IsFieldFound = FALSE;

					if (Mode & 1) {
						// Aggiungi solo se inesistente
						for (j=0; j<NumToArray[0]; j++) {
							if (ToArray[0][j] == FromArray[i]) {
								IsFieldFound = TRUE;
								break;
								}
							}
						}

					if (!IsFieldFound) {
						if (Mode & BIT2) {
							// Aggiungi da tipo dato 'int'
							ToArray[0][NumToArray[0]+NumFromArrayAdded] = (uint32_t)(FromIntegerArray[i]);
							} else if (Mode & BIT3) {
							// Aggiungi come 1B (sommando 1)
							ToArray[0][NumToArray[0]+NumFromArrayAdded] = (uint32_t)(FromArray[i])+1;
							} else {
							ToArray[0][NumToArray[0]+NumFromArrayAdded] = FromArray[i];
							}
						NumFromArrayAdded++;
						}
					}

				NumToArray[0] = NumToArray[0] + NumFromArrayAdded;
				}
			}
		}



return 1;
}






// Mode & 1	->	Aggiungi solo se inesistente
int add_array_to_string_array ( MDB_CHAR *FromString, MDB_CHAR ***ToArray, uint32_t *NumToArray, uint32_t *NumToArrayAllocated, HWND ptr_hwnd, int Mode )
{


	if (ToArray) {

		if (FromString) {

			uint32_t j, NumFromArrayAdded = 0;


			if (check_general_structure_allocated ( 0, (void**)ToArray, sizeof (ToArray[0][0]), NumToArray[0]+1, NumToArrayAllocated, 7, (char*)"Allocating result array", ptr_hwnd ) < 0) {
				MDB_CHAR str[512];
				sprintf_s ( str, sizeof(str), "Unable to allocate result array!");
				#ifdef _UtilityMessageFuncKey
					MultiDbMessage ( 0, NULL, 0+0, str, (MDB_CHAR *)MULTIDB_ERROR, MB_OK | MB_ICONHAND);
					#endif
				return -1;
				}


			{ 	BOOL IsFieldFound = FALSE;

				if (Mode & 1) {
					// Aggiungi solo se inesistente
					for (j=0; j<NumToArray[0]; j++) {
						if (check_stri(ToArray[0][j], FromString) == 0) {
							IsFieldFound = TRUE;
							break;
							}
						}
					}

				if (!IsFieldFound) {
					COPY_POINTER(ToArray[0][NumToArray[0]], FromString);
					NumToArray[0] = NumToArray[0] + 1;
					}
				}
			}
		}



return 1;
}



// ArrayItemStep = Passo tra un record e il sucessivo
// ArrayItemSize = Dimensione elemento da copiare
int create_array_from_structure ( char *PtrArray, uint32_t NumArray, uint32_t ArrayItemStep, uint32_t ArrayItemSize, uint32_t **OutArray, uint32_t *NumOutArray, uint32_t *NumOutArrayAllocated, HWND ptr_hwnd, int Mode )
{  	uint32_t i;




	if (OutArray) {

		if (NumOutArrayAllocated) {

			if (!ArrayItemSize) return -1;

			if (NumOutArray) {
				NumOutArray[0] = NumArray;
				}

			if (NumArray > NumOutArrayAllocated[0]) {
				uint32_t NeededMem = ArrayItemSize*NumArray;
				ALLOCATE_POINTER (OutArray[0], NeededMem, NumOutArrayAllocated[0], uint32_t*);
				}

			if (!OutArray[0]) {
				if (NumArray) {
					return -1;
					} else {
					return 0;
					}
				}

			if (!ArrayItemStep) {
				ArrayItemStep = ArrayItemSize;
				}


			// Verifica dimensione da copiare
			if (ArrayItemSize > sizeof(OutArray[0][0])) {
				ArrayItemSize = sizeof(OutArray[0][0]);
				}


			for (i=0; i<NumArray; i++) {

				if (ArrayItemSize < sizeof(OutArray[0][0])) {
					// Azzera lo spazio
					memset((void*)&OutArray[0][i], 0, sizeof(OutArray[0][0]));
					}

				memcpy ((void*)&OutArray[0][i], (void*)&PtrArray[i*ArrayItemStep], ArrayItemSize);
				}
			}
		}



return 1;
}



#ifndef ROW_SEP
	#define ROW_SEP	1
	#endif

int create_table_from_array (MDB_CHAR **SourceArrayString, uint32_t NumSourceArrayString, MDB_CHAR **OutString, uint32_t *OutStringAllocated )
{ 	uint32_t i;
	MDB_CHAR str_sep[64];


str_sep[0] = ROW_SEP;
str_sep[1] = ROW_SEP;
str_sep[2] = 0;

if (OutString) {

	if (OutString[0]) {
		OutString[0][0] = 0;
		} else {
		if (OutStringAllocated) {
			if (OutStringAllocated[0]) {
				OutStringAllocated[0] = 0;
				}
			}
		}

	for (i=0; i<NumSourceArrayString; i++) {
		if (i) {
			AddStr (OutString, str_sep, OutStringAllocated);
			}

		AddStr (OutString, SourceArrayString[i], OutStringAllocated);
		}
	}

return 1;
}




int duplicate_fields_array ( MDB_CHAR **FieldsValue, uint32_t NumFieldsValue, MDB_CHAR ***FillingArray, uint32_t *NumFillingArrayAllocated, HWND ptr_hwnd, int Mode )

{ 	uint32_t i, FillingArraySize = 0;


if (FieldsValue) {

	if (FillingArray) {

		if (NumFillingArrayAllocated) {

			NumFillingArrayAllocated[0] = NumFieldsValue;
			FillingArraySize = sizeof (FillingArray[0][0])*NumFillingArrayAllocated[0];
			FillingArray[0] = (MDB_CHAR**)realloc (FillingArray[0], FillingArraySize+1);

			if (FillingArray[0]) {

				memset (FillingArray[0], 0, FillingArraySize);

				for (i=0; i<NumFieldsValue; i++) {
					COPY_POINTER (FillingArray[0][i], FieldsValue[i]);
					}

				return 1;
				}
			}
		}
	}


return 0;
}

int free_fields_array ( MDB_CHAR ***FillingArray, uint32_t NumFillingArrayAllocated )
{

if (FillingArray) {
	if (FillingArray[0]) {
		uint32_t LocalIndex;
		for (LocalIndex=0; LocalIndex<NumFillingArrayAllocated; LocalIndex++) {
			FREE_POINTER (FillingArray[0][LocalIndex]);
			}
		FREE_POINTER (FillingArray[0]);
		}
	}

return 1;
}

int free_only_fields_array ( MDB_CHAR **FillingArray, uint32_t NumFillingArrayAllocated )
{

if (FillingArray) {
	uint32_t LocalIndex;
	for (LocalIndex=0; LocalIndex<NumFillingArrayAllocated; LocalIndex++) {
		FREE_POINTER (FillingArray[LocalIndex]);
		}
	}

return 1;
}


// Mode & 4	->	Pulisce la string da ' ' e "
// Mode & 2	->	Considera SeparatorChar come stringa
// Mode & 1	->	Considera la " come carattere di inizio/fine
int create_array_from_string ( MDB_CHAR *PtrString,
						MDB_CHAR ***OutArray, uint32_t *NumOutArray, uint32_t *NumOutArrayAllocated,
						MDB_CHAR *SeparatorChar, uint32_t NumSeparatorChar,
						HWND ptr_hwnd, int Mode )

{	uint32_t i_brk, j, Len, start_char_position, stripping_char, NumConditionAdded = 0, NumRelativeConditionAdded = 0;
	BOOL IsBreakChar = FALSE, StopOnCommaChar = FALSE;



if (PtrString) {

	start_char_position = 0;

	GET_POINTER_LEN1B (PtrString, Len);

	if (NumOutArray) {
		NumOutArray[0] = 0;
		}

	if (!PtrString[0]) {
		return 1;
		}

	// Salta gli spazi iniziali
	j=0;
	while (PtrString[j] == ' ' && j< Len) j++;


	for (; j<Len; j++) {

		if (Mode & 1) {
			// Considera la " come carattere di inizio/fine
			if (PtrString[j] == '"') {
				StopOnCommaChar = TRUE;
				j++;
				while (PtrString[j] != '"' && j< Len) j++;
				}
			}

		IsBreakChar = FALSE;

		if (Mode & 2) {
			// Considera SeparatorChar come stringa
			IsBreakChar = TRUE;
			stripping_char = j;
			for (i_brk=0; i_brk<NumSeparatorChar; i_brk++) {
				if (PtrString[j+i_brk] != SeparatorChar[i_brk] && PtrString[j] != 0) {
					IsBreakChar = FALSE;
					break;
					}
				}
			} else {
			for (i_brk=0; i_brk<NumSeparatorChar; i_brk++) {
				if (PtrString[j] == SeparatorChar[i_brk] || PtrString[j] == 0) {
					if (!IsBreakChar) {
						IsBreakChar = TRUE;
						stripping_char = j;
						}
					break;
					}
				}
			}


		if (IsBreakChar) {
			if (stripping_char) {
				if (PtrString[stripping_char] == ' ') {
					while (PtrString[stripping_char] == ' ' && stripping_char> 0) stripping_char--;
					stripping_char++;
					}
				}


			if (OutArray) {
				if (NumOutArray) {
					if (NumOutArray[0] >= NumOutArrayAllocated[0]) {
						if (check_general_structure_allocated ( 0, (void**)OutArray, sizeof (OutArray[0][0]), NumOutArray[0]+1, NumOutArrayAllocated, 6, (char*)"Allocation columns array ", ptr_hwnd ) < 0) {
							return -1;
							}
						}
					}
				}


			// Copia della stringa
			{ 	MDB_CHAR OldChar = PtrString[stripping_char];

				// PtrString[stripping_char] = 0;

				if (PtrString) {
					if (OutArray) {
						if (OutArray[0]) {
							if (NumOutArray) {
								uint32_t __local_string_needed_size = (stripping_char-start_char_position)+1;
								uint32_t start_char = start_char_position;
								uint32_t end_char = stripping_char;

								if (Mode & 4) {
									// Pulisce la string da ' ' e "
									while((PtrString[start_char] == ' ' || PtrString[start_char] == '\'' || PtrString[start_char] == '"') && start_char < Len) start_char++;
									while((PtrString[end_char-1] == ' ' || PtrString[end_char-1] == '\'' || PtrString[end_char-1] == '"') && end_char > 0) end_char--;
									}
									
								OutArray[0][NumOutArray[0]] = (MDB_CHAR*)realloc ((void*)OutArray[0][NumOutArray[0]], __local_string_needed_size+1);
								if (OutArray[0][NumOutArray[0]]) {
										
									if (OutArray[0][NumOutArray[0]]) 
                                                                            strncpy ((char*)OutArray[0][NumOutArray[0]], &PtrString[start_char], MIN(__local_string_needed_size, (end_char-start_char)) );
									OutArray[0][NumOutArray[0]][(end_char-start_char)] = 0;

									/*
									PtrString[stripping_char] = 0;
									COPY_POINTER (OutArray[0][NumOutArray[0]], ((MDB_CHAR*)&PtrString[start_char]));
									PtrString[stripping_char] = OldChar;
									*/
									NumOutArray[0]++;
									
									}
								}
							}
						}
					}

				// PtrString[stripping_char] = OldChar;

				if (Mode & 2) {
					// Considera SeparatorChar come stringa
					start_char_position = j+NumSeparatorChar;
					} else {
					start_char_position = j+1;
					}
				}
			}
		}

	return 1;
	}

return 0;
}

