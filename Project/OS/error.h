#ifndef ERROR_H
#define ERROR_H

#include "common.h"


#define OS_ERROR_BASE_NUM                           (0x0000)
#define SDC_ERROR_BASE_NUM                          (0x1000)

#define OS_SUCCESS                                  (OS_ERROR_BASE_NUM + 0)
#define OS_ERROR_GAME_NOT_FOUND                     (OS_ERROR_BASE_NUM + 1)
#define OS_ERROR_INVALID_INDEX                      (OS_ERROR_BASE_NUM + 2)
#define OS_ERROR_INVALID_PATH                       (OS_ERROR_BASE_NUM + 3)
#define OS_ERROR_INVALID_PATH_LENGTH                (OS_ERROR_BASE_NUM + 4)

#define SDC_SUCCESS                                 (SDC_ERROR_BASE_NUM + 0)
#define SDC_ERROR_NO_SDC                            (SDC_ERROR_BASE_NUM + 1)
#define SDC_ERROR_FILE_ACCESS_FAILED                (SDC_ERROR_BASE_NUM + 2)
#define SDC_ERROR_READING_FILE_FAILED               (SDC_ERROR_BASE_NUM + 3)
#define SDC_ERROR_WRITING_FILE_FAILED               (SDC_ERROR_BASE_NUM + 4)
#define SDC_ERROR_DIR_ACCESS_FAILED                 (SDC_ERROR_BASE_NUM + 5)
#define SDC_ERROR_READING_DIR_FAILED                (SDC_ERROR_BASE_NUM + 6)

#define ERROR_CHECK(ERR_DEF)                        \
do                                                  \
{                                                   \
    if (ERR_DEF.Type != 0x000)                      \
    {                                               \
        Error_Info_t err_info;                      \
        err_info.Error     = ERR_DEF;               \
        err_info.FileName  = __FILE__;              \
        err_info.LineNum   = __LINE__;              \
                                                    \
        Error_BaseHandler(err_info);                \
    }                                               \
} while (0)

#define ERR_DEF_INIT_NO_ARGUMENT(ERR_DEF, ERR_CODE) \
do                                                  \
{                                                   \
    ERR_DEF.ErrCode  = ERR_CODE;                    \
    ERR_DEF.Argument = NULL;                        \
} while (0)

#define ERR_DEF_INIT(ERR_DEF, ERR_CODE, ARGUMENT)   \
do                                                  \
{                                                   \
    ERR_DEF.ErrCode  = ERR_CODE;                    \
    ERR_DEF.Argument = ARGUMENT;                    \
} while (0)


typedef struct
{
    union
    {
       uint32_t ErrCode;

        #pragma pack(1)
        struct
        {
            unsigned int Base  : 8;
            unsigned int Type  : 24;
        }; 
    };

    const char *Argument;
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
void Error_SystemHandler(void);

#endif //ERROR_H
