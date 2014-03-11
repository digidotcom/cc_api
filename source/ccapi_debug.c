#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"

#if (defined CCAPI_DEBUG)
/* TODO: Can we do this? It's done for RCI*/
#include <stdarg.h>

void debug_printf(ccapi_debug_zones_t zone, ccapi_debug_level_t level, va_list args, char const * const format)
{
    char formatted_string[250];


    if (ccapi_data == NULL) 
    {
        /* We can't reach the zones/level info:
         *     - For production code, just print it 
         *     - For unit testing, skip
         */    
#if (defined UNIT_TEST)
        return;
#endif
    }
    else
    {
        /* Evaluate level */
        if (level < ccapi_data->dbg_level)
            return;

        /* zone */
        if (!(zone & ccapi_data->dbg_zones))
            return;
    }

    /* TODO: Call connector_snprintf so we check buffer size */
    vsprintf(formatted_string, format, args);
    va_end(args);

    ccimp_debug_printf(formatted_string);
}

void connector_debug_printf(char const * const format, ...)
{
    va_list args;
    
    va_start(args, format);
    debug_printf(ZONE_LAYER1, LEVEL_ERROR, args, format);
    va_end(args);
}


void ccapi_debug_printf(ccapi_debug_zones_t zone, ccapi_debug_level_t level, char const * const format, ...)
{
    va_list args;
    
    va_start(args, format);
    debug_printf(zone, level, args, format);
    va_end(args);
}

ccapi_config_debug_error_t ccapi_config_debug(ccapi_debug_zones_t zones, ccapi_debug_level_t level)
{
    if (ccapi_data == NULL)
        return CCAPI_CONFIG_DEBUG_ERROR_NOT_STARTED;

    if (zones > ZONE_ALL)
        return CCAPI_CONFIG_DEBUG_ERROR_INVALID_ZONES;

    if (level >= LEVEL_COUNT)
        return CCAPI_CONFIG_DEBUG_ERROR_INVALID_LEVEL;

    ccapi_data->dbg_zones = zones;
    ccapi_data->dbg_level = level;

    return CCAPI_CONFIG_DEBUG_ERROR_NONE;
}
#else
void ccapi_debug_printf(ccapi_debug_zones_t zone, ccapi_debug_level_t level, char const * const format, ...)
{  
    (void)zone;
    (void)level;
    (void)format;
}
#endif /* (defined CCAPI_DEBUG) */

