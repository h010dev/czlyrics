/**
 * @file   api.h 
 * @author Mohamed Al-Hussein
 * @date   08/21/2021
 * @brief  Public header file for API.
 */
#ifndef API_H
#define API_H

#include "../external/mongoose/mongoose.h"

/**
 * @brief Event handler for API.
 *
 * Calls handle_request function if event number is an HTTP message event. Ignores everything else.
 * @see https://cesanta.com/docs/#event-handler-function
 *
 * @param[in] c       a connection that received an event 
 * @param[in] ev      an event number
 * @param[in] ev_data points to event-specific data
 * @param[in] fn_data points to user-defined data (NULL in this case)
 */
void fn_api (struct mg_connection *c, int ev, void *ev_data, void *fn_data);

#endif /* API_H */
