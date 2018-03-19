/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_private.h: private structures and functions declarations
 * 
 * Copyright 2015-2017 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation;
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef __U_PRIVATE_H__
#define __U_PRIVATE_H__

#include "ulfius.h"

/** Macro to avoid compiler warning when some parameters are unused and that's ok **/
#define UNUSED(x) (void)(x)

/**********************************
 * Internal functions declarations
 **********************************/

/**
 * ulfius_endpoint_match
 * return the endpoint array matching the url called with the proper http method
 * the returned array always has its last value to NULL
 * return NULL on memory error
 */
struct _u_endpoint ** ulfius_endpoint_match(const char * method, const char * url, struct _u_endpoint * endpoint_list);

/**
 * ulfius_parse_url
 * fills map with the keys/values defined in the url that are described in the endpoint format url
 * return U_OK on success
 */
int ulfius_parse_url(const char * url, const struct _u_endpoint * endpoint, struct _u_map * map);

/**
 * ulfius_set_response_header
 * adds headers defined in the response_map_header to the response
 * return the number of added headers, -1 on error
 */
int ulfius_set_response_header(struct MHD_Response * response, const struct _u_map * response_map_header);

/**
 * ulfius_set_response_cookie
 * adds cookies defined in the response_map_cookie
 * return the number of added headers, -1 on error
 */
int ulfius_set_response_cookie(struct MHD_Response * mhd_response, const struct _u_response * response);

#ifndef U_DISABLE_WEBSOCKET

/**
 * Websocket callback function for MHD
 * Starts the websocket manager if set,
 * then sets a listening message loop
 * Complete the callback when the websocket is closed
 * The websocket can be closed by the client, the manager, the program, or on network disconnect
 */
void ulfius_start_websocket_cb (void *cls,
            struct MHD_Connection *connection,
            void *con_cls,
            const char *extra_in,
            size_t extra_in_size,
            MHD_socket sock,
            struct MHD_UpgradeResponseHandle *urh);

/**
 * Workaround to make sure a message, as long as it can be is complete sent
 */
void ulfius_websocket_send_all(MHD_socket sock, const uint8_t * data, size_t len);

/**
 * Centralise socket reading in this function
 * so if options or check must be done, it's done here instead of each read call
 */
size_t ulfius_websocket_recv_all(MHD_socket sock, uint8_t * data, size_t len);

/**
 * Generates a handhshake answer from the key given in parameter
 */
int ulfius_generate_handshake_answer(const char * key, char * out_digest);

/**
 * Return a match list between two list of items
 * If match is NULL, then return source duplicate
 * Returned value must be u_free'd after use
 */
char * ulfius_check_list_match(const char * source, const char * match, const char * separator);

/**
 * Return the first match between two list of items
 * If match is NULL, then return the first element of source
 * Returned value must be u_free'd after use
 */
char * ulfius_check_first_match(const char * source, const char * match, const char * separator);

/**
 * Initialize a websocket message list
 * Return U_OK on success
 */
int ulfius_init_websocket_message_list(struct _websocket_message_list * message_list);

/**
 * Clear data of a websocket message list
 */
void ulfius_clear_websocket_message_list(struct _websocket_message_list * message_list);

/**
 * Append a message in a message list
 * Return U_OK on success
 */
int ulfius_push_websocket_message(struct _websocket_message_list * message_list, struct _websocket_message * message);

/**
 * Clear data of a websocket
 */
int ulfius_clear_websocket(struct _websocket * websocket);

/**
 * Clear data of a websocket_manager
 */
void ulfius_clear_websocket_manager(struct _websocket_manager * websocket_manager);

/**
 * Close the websocket
 */
int ulfius_close_websocket(struct _websocket * websocket);

/**
 * Read and parse a new message from the websocket
 * Return the opcode of the new websocket, U_WEBSOCKET_OPCODE_NONE if no message arrived, or U_WEBSOCKET_OPCODE_ERROR on error
 * Sets the new message in the message variable if available
 */
int ulfius_read_incoming_message(struct _websocket_manager * websocket_manager, struct _websocket_message ** message);

/**
 * Run the websocket manager in a separated detached thread
 */
void * ulfius_thread_websocket_manager_run(void * args);

/**
 * Send a message in the websocket without lock
 * Return U_OK on success
 */
int ulfius_websocket_send_message_nolock(struct _websocket_manager * websocket_manager,
                                  const uint8_t opcode,
                                  const uint64_t data_len,
                                  const char * data);

/**
 * Add a websocket in the list of active websockets of the instance
 */
int ulfius_instance_add_websocket_active(struct _u_instance * instance, struct _websocket * websocket);

/**
 * Remove a websocket from the list of active websockets of the instance
 */
int ulfius_instance_remove_websocket_active(struct _u_instance * instance, struct _websocket * websocket); 

#endif // U_DISABLE_WEBSOCKET

#endif // __U_PRIVATE_H__
