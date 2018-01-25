#ifndef ERROR_H
#define ERROR_H

#include "common.h"

#define OS_ERROR_BASE_NUM                        (0x0000)
#define SDC_ERROR_BASE_NUM                       (0x1000)

#define OS_SUCCESS                               (OS_ERROR_BASE_NUM + 0)
#define OS_ERROR_GAME_NOT_FOUND                  (OS_ERROR_BASE_NUM + 1)
#define OS_ERROR_INVALID_INDEX                   (OS_ERROR_BASE_NUM + 2)
#define OS_ERROR_INVALID_PATH                    (OS_ERROR_BASE_NUM + 3)
#define OS_ERROR_INVALID_PATH_LENGTH             (OS_ERROR_BASE_NUM + 4)

#define SDC_SUCCESS                              (SDC_ERROR_BASE_NUM + 0)
#define SDC_ERROR_NO_SDC                         (SDC_ERROR_BASE_NUM + 1)
#define SDC_ERROR_FILE_ACCESS_FAILED             (SDC_ERROR_BASE_NUM + 2)
#define SDC_ERROR_FAV_DIR_ACCESS_FAILED          (SDC_ERROR_BASE_NUM + 3)
#define SDC_ERROR_GAME_DIR_ACCESS_FAILED         (SDC_ERROR_BASE_NUM + 4)

#define ERROR_CHECK(ERR_CODE)                                  \
do                                                             \
{                                                              \
    if ((ERR_CODE & 0x0FFF) != 0x0000)                         \
    {                                                          \
        Error_Info_t err_info;                                 \
        err_info.Error.Base       = (ERR_CODE & 0xF000) >> 24; \
        err_info.Error.Descriptor = ERR_CODE & 0x0FFF;         \
        err_info.FileName         = __FILE__;                  \
        err_info.LineNum          = __LINE__;                  \
                                                               \
        Error_BaseHandler(err_info);                           \
    }                                                          \
} while (0)

typedef union
{
    uint32_t ErrCode;

    #pragma pack(1)
    struct
    {
        unsigned int Base        : 8;
        unsigned int Descriptor  : 24;
    };
}
Error_Def_t;

typedef struct
{
    Error_Def_t  Error;
    uint32_t     LineNum;
    const char  *FileName;
}
Error_Info_t;


void Error_BaseHandler(Error_Info_t err_info);
void Error_SDCHandler(Error_Info_t err_info);
void Error_OSHandler(Error_Info_t err_info);

#endif //ERROR_H
