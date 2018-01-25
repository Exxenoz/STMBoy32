#include "error.h"

void Error_BaseHandler(Error_Info_t err_info)
{
    switch (err_info.Error.Base)
    {
        case OS_ERROR_BASE_NUM:
            Error_OSHandler(err_info);
            break;

        case SDC_ERROR_BASE_NUM:
            Error_SDCHandler(err_info);
            break;

        default:
            break;
    }
}

void Error_SDCHandler(Error_Info_t err_info)
{
    
}

void Error_OSHandler(Error_Info_t err_info)
{
    
}
