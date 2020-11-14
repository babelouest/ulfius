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

/**
 * For using Ulfius in embedded systems
 * Thanks to Dirk Uhlemann
 */
#ifdef U_WITH_FREERTOS
  #include <FreeRTOS_Sockets.h>
  #define  sockaddr  freertos_sockaddr
  typedef  unsigned long int  socklen_t;
#else
  #ifdef U_WITH_LWIP
    #include <lwip/sockets.h>
  #endif // U_WITH_LWIP
#endif // U_WITH_FREERTOS


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
int ulfius_parse_url(const char * url, const struct _u_endpoint * endpoint, struct _u_map * map, int check_utf8);

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

/**
 * The utf8_check() function scans the '\0'-terminated string starting
 * at s. It returns a pointer to the first byte of the first malformed
 * or overlong UTF-8 sequence found, or NULL if the string contains
 * only correct UTF-8. It also spots UTF-8 sequences that could cause
 * trouble if converted to UTF-16, namely surrogate characters
 * (U+D800..U+DFFF) and non-Unicode positions (U+FFFE..U+FFFF). This
 * routine is very likely to find a malformed sequence if the input
 * uses any other encoding than UTF-8. It therefore can be used as a
 * very effective heuristic for distinguishing between UTF-8 and other
 * encodings.
 *
 * I wrote this code mainly as a specification of functionality; there
 * are no doubt performance optimizations possible for certain CPUs.
 *
 * Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> -- 2005-03-30
 * License: http://www.cl.cam.ac.uk/~mgk25/short-license.html
 */
const unsigned char * utf8_check(const char * s_orig);

#ifndef U_DISABLE_WEBSOCKET

#define _U_W_BUFF_LEN 256
#define _U_W_EXT_DEFLATE "permessage-deflate"
#define _U_W_EXT_DEFLATE_S_CTX "server_no_context_takeover"
#define _U_W_EXT_DEFLATE_C_CTX "client_no_context_takeover"

void ulfius_free_websocket_extension_pointer_list(void * extension);

void ulfius_free_websocket_extension(struct _websocket_extension * websocket_extension);

int ulfius_init_websocket_extension(struct _websocket_extension * websocket_extension);

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
 * Generates a handhshake answer from the key given in parameter
 */
int ulfius_generate_handshake_answer(const char * key, char * out_digest);

/**
 * Return a match list between two list of items
 * If match is NULL, then return source duplicate
 * result must be u_free'd after use
 */
int ulfius_check_list_match(const char * source, const char * match, const char * separator, char ** result);

/**
 * Return the first match between two list of items
 * If match is NULL, then return the first element of source
 * result must be u_free'd after use
 */
int ulfius_check_first_match(const char * source, const char * match, const char * separator, char ** result);

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
 * Add a websocket in the list of active websockets of the instance
 */
int ulfius_instance_add_websocket_active(struct _u_instance * instance, struct _websocket * websocket);

/**
 * Remove a websocket from the list of active websockets of the instance
 */
int ulfius_instance_remove_websocket_active(struct _u_instance * instance, struct _websocket * websocket); 

/**
 * Initialize a struct _websocket
 * return U_OK on success
 */
int ulfius_init_websocket(struct _websocket * websocket);

/**
 * Initialize a struct _websocket_manager
 * return U_OK on success
 */
int ulfius_init_websocket_manager(struct _websocket_manager * websocket_manager);

/**
 * Check if the response corresponds to the transformation of the key with the magic string
 */
int ulfius_check_handshake_response(const char * key, const char * response);

#endif // U_DISABLE_WEBSOCKET

#endif // __U_PRIVATE_H__
