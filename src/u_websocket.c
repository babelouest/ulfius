/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_websocket.c: websocket implementation
 * 
 * Copyright 2017 Nicolas Mora <mail@babelouest.org>
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
#if !defined(U_DISABLE_WEBSOCKET)
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <gnutls/gnutls.h>

#include "ulfius.h"

/**
 * Set a websocket in the response
 * You must set at least websocket_manager_callback or websocket_incoming_message_callback
 * @Parameters
 * response: struct _u_response to send back the websocket initialization, mandatory
 * websocket_protocol: list of protocols, separated by a comma, or NULL if all protocols are accepted
 * websocket_extensions: list of extensions, separated by a comma, or NULL if all extensions are accepted
 * websocket_manager_callback: callback function called right after the handshake acceptance, optional
 * websocket_manager_user_data: any data that will be given to the websocket_manager_callback, optional
 * websocket_incoming_message_callback: callback function called on each incoming complete message, optional
 * websocket_incoming_user_data: any data that will be given to the websocket_incoming_message_callback, optional
 * websocket_onclose_callback: callback function called right before closing the websocket, must be complete for the websocket to close
 * websocket_onclose_user_data: any data that will be given to the websocket_onclose_callback, optional
 * @Return value: U_OK on success
 */
int ulfius_set_websocket_response(struct _u_response * response,
                                   const char * websocket_protocol,
                                   const char * websocket_extensions, 
                                   void (* websocket_manager_callback) (const struct _u_request * request,
                                                                       struct _websocket_manager * websocket_manager,
                                                                       void * websocket_manager_user_data),
                                   void * websocket_manager_user_data,
                                   void (* websocket_incoming_message_callback) (const struct _u_request * request,
                                                                                struct _websocket_manager * websocket_manager,
                                                                                const struct _websocket_message * message,
                                                                                void * websocket_incoming_user_data),
                                   void * websocket_incoming_user_data,
                                   void (* websocket_onclose_callback) (const struct _u_request * request,
                                                                       struct _websocket_manager * websocket_manager,
                                                                       void * websocket_onclose_user_data),
                                   void * websocket_onclose_user_data) {
  if (response != NULL && (websocket_manager_callback != NULL || websocket_incoming_message_callback)) {
    if (response->websocket_protocol != NULL) {
      o_free(response->websocket_protocol);
    }
    response->websocket_protocol = o_strdup(websocket_protocol);
    if (response->websocket_protocol == NULL && websocket_protocol != NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for response->websocket_protocol");
      return U_ERROR_MEMORY;
    }
    if (response->websocket_extensions != NULL) {
      o_free(response->websocket_extensions);
    }
    response->websocket_extensions = o_strdup(websocket_extensions);
    if (response->websocket_extensions == NULL && websocket_extensions != NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for response->websocket_extensions");
      o_free(response->websocket_protocol);
      return U_ERROR_MEMORY;
    }
    response->websocket_manager_callback = websocket_manager_callback;
    response->websocket_manager_user_data = websocket_manager_user_data;
    response->websocket_incoming_message_callback = websocket_incoming_message_callback;
    response->websocket_incoming_user_data = websocket_incoming_user_data;
    response->websocket_onclose_callback = websocket_onclose_callback;
    response->websocket_onclose_user_data = websocket_onclose_user_data;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Run websocket in a separate thread
 * then sets a listening message loop
 * Complete the callback when the websocket is closed
 * The websocket can be closed by the client, the manager, the program, or on network disconnect
 */
void * ulfius_thread_websocket(void * data) {
  int opcode;
  struct _websocket * websocket = (struct _websocket*)data;
  struct _websocket_message * message = NULL;
  pthread_t thread_websocket_manager;
  pthread_mutexattr_t mutexattr;
  int thread_ret_websocket_manager = 0, thread_detach_websocket_manager = 0;
  
  if (websocket != NULL && websocket->websocket_manager != NULL) {
    pthread_mutexattr_init ( &mutexattr );
    pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE_NP );
    if (pthread_mutex_init(&(websocket->websocket_manager->read_lock), &mutexattr) != 0 || pthread_mutex_init(&(websocket->websocket_manager->write_lock), &mutexattr) != 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Impossible to initialize Mutex Lock for websocket");
      websocket->websocket_manager->connected = 0;
    }
    pthread_mutexattr_destroy( &mutexattr );
    if (websocket->websocket_manager_callback != NULL && websocket->websocket_manager->connected) {
      websocket->websocket_manager->manager_closed = 0;
      thread_ret_websocket_manager = pthread_create(&thread_websocket_manager, NULL, ulfius_thread_websocket_manager_run, (void *)websocket);
      thread_detach_websocket_manager = pthread_detach(thread_websocket_manager);
      if (thread_ret_websocket_manager || thread_detach_websocket_manager) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error creating or detaching websocket manager thread, return code: %d, detach code: %d",
                      thread_ret_websocket_manager, thread_detach_websocket_manager);
        websocket->websocket_manager->connected = 0;
      }
    } else {
      websocket->websocket_manager->manager_closed = 1;
    }
    while (websocket->websocket_manager->connected && !websocket->websocket_manager->closing) {
      message = NULL;
      if (pthread_mutex_lock(&websocket->websocket_manager->read_lock)) {
        websocket->websocket_manager->connected = 0;
      }
      opcode = ulfius_read_incoming_message(websocket->websocket_manager, &message);
      if (opcode == U_WEBSOCKET_OPCODE_CLOSE) {
        // Send close command back, then close the socket
        if (ulfius_websocket_send_message(websocket->websocket_manager, U_WEBSOCKET_OPCODE_CLOSE, 0, NULL) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error sending close command");
        }
        websocket->websocket_manager->closing = 1;
      } else if (opcode == U_WEBSOCKET_OPCODE_PING) {
        // Send pong command
        if (ulfius_websocket_send_message(websocket->websocket_manager, U_WEBSOCKET_OPCODE_PONG, 0, NULL) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error sending pong command");
        }
      } else if (opcode != U_WEBSOCKET_OPCODE_NONE && message != NULL) {
        if (websocket->websocket_incoming_message_callback != NULL) {
          // TODO: run in thread
          websocket->websocket_incoming_message_callback(websocket->request, websocket->websocket_manager, message, websocket->websocket_incoming_user_data);
        }
      }
      if (message != NULL) {
        if (ulfius_push_websocket_message(websocket->websocket_manager->message_list_incoming, message) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error pushing new websocket message in list");
        }
      }
      pthread_mutex_unlock(&websocket->websocket_manager->read_lock);
      usleep(U_WEBSOCKET_USEC_WAIT);
    }
    if (ulfius_close_websocket(websocket) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error closing websocket");
    }
    // Wait for thread manager to close
    while (!websocket->websocket_manager->manager_closed) {
      usleep(U_WEBSOCKET_USEC_WAIT);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error websocket parameters");
  }
  ulfius_clear_websocket(websocket);
  return NULL;
}

/**
 * Websocket callback function for MHD
 * Starts the websocket manager if set,
 */
void ulfius_start_websocket_cb (void *cls,
            struct MHD_Connection *connection,
            void * con_cls,
            const char * extra_in,
            size_t extra_in_size,
            MHD_socket sock,
            struct MHD_UpgradeResponseHandle * urh) {
  struct _websocket * websocket = (struct _websocket*)cls;
  pthread_t thread_websocket;
  int thread_ret_websocket = 0, thread_detach_websocket = 0;
  
  if (websocket != NULL) {
    websocket->urh = urh;
    websocket->websocket_manager = o_malloc(sizeof(struct _websocket_manager));
    // Run websocket manager in a thread if set
    if (websocket->websocket_manager != NULL) {
      websocket->websocket_manager->message_list_incoming = o_malloc(sizeof(struct _websocket_message_list));
      websocket->websocket_manager->message_list_outcoming = o_malloc(sizeof(struct _websocket_message_list));
      ulfius_init_websocket_message_list(websocket->websocket_manager->message_list_incoming);
      ulfius_init_websocket_message_list(websocket->websocket_manager->message_list_outcoming);
      websocket->websocket_manager->sock = sock;
      websocket->websocket_manager->connected = 1;
      websocket->websocket_manager->closing = 0;
      thread_ret_websocket = pthread_create(&thread_websocket, NULL, ulfius_thread_websocket, (void *)websocket);
      thread_detach_websocket = pthread_detach(thread_websocket);
      if (thread_ret_websocket || thread_detach_websocket) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error creating or detaching websocket manager thread, return code: %d, detach code: %d",
                      thread_ret_websocket, thread_detach_websocket);
        ulfius_clear_websocket(websocket);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for websocket_manager");
      ulfius_clear_websocket(websocket);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error websocket is NULL");
    ulfius_clear_websocket(websocket);
  }
  return;
}

/**
 * Read and parse a new message from the websocket
 * Return the opcode of the new websocket, U_WEBSOCKET_OPCODE_NONE if no message arrived, or U_WEBSOCKET_OPCODE_ERROR on error
 * Sets the new message in the message variable
 */
int ulfius_read_incoming_message(struct _websocket_manager * websocket_manager, struct _websocket_message ** message) {
  int len, opcode, i;
  int message_complete = 0, message_error;
  uint8_t header[2], payload_len[8], masking_key[4];
  uint8_t * payload_data;
  size_t msg_len = 0;
  
  (*message) = NULL;
  do {
    message_error = 0;
    len = ulfius_websocket_recv_all(websocket_manager->sock, header, 2);
    // New frame has arrived
    if (len == 2) {
      if ((*message) == NULL) {
        *message = o_malloc(sizeof(struct _websocket_message));
        (*message)->data_len = 0;
        (*message)->has_mask = 0;
        (*message)->opcode = header[0] & 0x0F;
        (*message)->data = NULL;
      }
      if ((header[1] & U_WEBSOCKET_LEN_MASK) <= 125) {
        msg_len = (header[1] & U_WEBSOCKET_LEN_MASK);
      } else if ((header[1] & U_WEBSOCKET_LEN_MASK) == 126) {
        len = ulfius_websocket_recv_all(websocket_manager->sock, payload_len, 2);
        if (len == 2) {
          msg_len = payload_len[1] | ((uint64_t)payload_len[0] << 8);
        } else if (len != -1) {
          message_error = 1;
          opcode = U_WEBSOCKET_OPCODE_ERROR;
          y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket message length");
        }
      } else if ((header[1] & U_WEBSOCKET_LEN_MASK) == 127) {
        len = ulfius_websocket_recv_all(websocket_manager->sock, payload_len, 8);
        if (len == 8) {
          msg_len = payload_len[7] |
                    ((uint64_t)payload_len[6] << 8) |
                    ((uint64_t)payload_len[5] << 16) |
                    ((uint64_t)payload_len[4] << 24) |
                    ((uint64_t)payload_len[3] << 32) |
                    ((uint64_t)payload_len[2] << 40) |
                    ((uint64_t)payload_len[1] << 48) |
                    ((uint64_t)payload_len[0] << 54);
        } else if (len != -1) {
          message_error = 1;
          opcode = U_WEBSOCKET_OPCODE_ERROR;
          y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket message length");
        }
      }
      if (!message_error) {
        if (header[1] & U_WEBSOCKET_HAS_MASK) {
          (*message)->has_mask = 1;
          len = ulfius_websocket_recv_all(websocket_manager->sock, masking_key, 4);
          if (len != 4 && len != -1) {
            message_error = 1;
            opcode = U_WEBSOCKET_OPCODE_ERROR;
            y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket for mask");
          }
          if (!message_error) {
            if (msg_len > 0) {
              payload_data = o_malloc(msg_len*sizeof(uint8_t));
              len = ulfius_websocket_recv_all(websocket_manager->sock, payload_data, msg_len);
              if (len == msg_len) {
                // If mask, decode message
                if ((*message)->has_mask) {
                  (*message)->data = o_realloc((*message)->data, (msg_len+(*message)->data_len)*sizeof(uint8_t));
                  for (i = (*message)->data_len; i < msg_len; i++) {
                    (*message)->data[i] = payload_data[i-(*message)->data_len] ^ masking_key[(i-(*message)->data_len)%4];
                  }
                } else {
                  memcpy((*message)->data+(*message)->data_len, payload_data, msg_len);
                }
                (*message)->data_len += msg_len;
              } else if (len != -1) {
                message_error = 1;
                opcode = U_WEBSOCKET_OPCODE_ERROR;
                y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket for payload_data");
              }
              o_free(payload_data);
            }
            if (!message_error && (header[0] & U_WEBSOCKET_BIT_FIN)) {
              opcode = (*message)->opcode;
              time(&(*message)->datestamp);
              message_complete = 1;
            }
          }
        } else {
          message_error = 1;
          opcode = U_WEBSOCKET_OPCODE_ERROR;
          y_log_message(Y_LOG_LEVEL_ERROR, "Incoming message has no MASK flag, exiting");
        }
      }
    } else if (len == 0) {
      // Socket closed
      opcode = U_WEBSOCKET_OPCODE_CLOSED;
      message_complete = 1;
    } else if (len != -1) {
      opcode = U_WEBSOCKET_OPCODE_ERROR;
      message_error = 1;
      y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket for header, len is %d", len);
    } else {
      opcode = U_WEBSOCKET_OPCODE_NONE;
      message_complete = 1;
    }
  } while (!message_complete && !message_error);
  return opcode;
}

/**
 * Clear all data related to the websocket
 */
int ulfius_clear_websocket(struct _websocket * websocket) {
  if (websocket != NULL) {
    if (MHD_upgrade_action (websocket->urh, MHD_UPGRADE_ACTION_CLOSE) != MHD_YES) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error sending MHD_UPGRADE_ACTION_CLOSE frame to urh");
    }
    ulfius_clear_websocket_manager(websocket->websocket_manager);
    o_free(websocket->websocket_manager);
    websocket->websocket_manager = NULL;
    o_free(websocket);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Run the websocket manager in a separated detached thread
 */
void * ulfius_thread_websocket_manager_run(void * args) {
  struct _websocket * websocket = (struct _websocket *)args;
  if (websocket != NULL && websocket->websocket_manager_callback != NULL && websocket->websocket_manager != NULL) {
    websocket->websocket_manager_callback(websocket->request, websocket->websocket_manager, websocket->websocket_manager_user_data);
    // Websocket manager callback complete, set close signal
    websocket->websocket_manager->manager_closed = 1;
    websocket->websocket_manager->closing = 1;
  }
  return NULL;
}

/**
 * Generates a handhshake answer from the key given in parameter
 */
int ulfius_generate_handshake_answer(const char * key, char * out_digest) {
  gnutls_datum_t key_data;
  unsigned char encoded_key[32] = {0};
  size_t encoded_key_size = 32, encoded_key_size_base64;
  int res, to_return = 0;
  
  key_data.data = (unsigned char*)msprintf("%s%s", key, U_WEBSOCKET_MAGIC_STRING);
  key_data.size = strlen((const char *)key_data.data);
  
  if (key_data.data != NULL && out_digest != NULL && (res = gnutls_fingerprint(GNUTLS_DIG_SHA1, &key_data, encoded_key, &encoded_key_size)) == GNUTLS_E_SUCCESS) {
    if (o_base64_encode(encoded_key, encoded_key_size, (unsigned char *)out_digest, &encoded_key_size_base64)) {
      to_return = 1;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error base64 encoding hashed key");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error getting sha1 signature for key");
  }
  o_free(key_data.data);
  return to_return;
}

/**
 * Initialize a websocket message list
 * Return U_OK on success
 */
int ulfius_init_websocket_message_list(struct _websocket_message_list * message_list) {
  if (message_list != NULL) {
    message_list->len = 0;
    message_list->list = NULL;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Clear data of a websocket message list
 */
void ulfius_clear_websocket_message_list(struct _websocket_message_list * message_list) {
  size_t i;
  if (message_list != NULL) {
    for (i=0; i < message_list->len; i++) {
      ulfius_clear_websocket_message(message_list->list[i]);
    }
    o_free(message_list->list);
  }
}

/**
 * Clear data of a websocket message
 */
void ulfius_clear_websocket_message(struct _websocket_message * message) {
  if (message != NULL) {
    o_free(message->data);
    o_free(message);
  }
}

/**
 * Append a message in a message list
 * Return U_OK on success
 */
int ulfius_push_websocket_message(struct _websocket_message_list * message_list, struct _websocket_message * message) {
  if (message_list != NULL && message != NULL) {
    message_list->list = o_realloc(message_list->list, (message_list->len+1)*sizeof(struct _websocket_message *));
    if (message_list->list != NULL) {
      message_list->list[message_list->len] = message;
      message_list->len++;
      return U_OK;
    } else {
      return U_ERROR_MEMORY;
    }
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Return the first message of the message list
 * Return NULL if message_list has no message
 * Returned value must be cleared after use
 */
struct _websocket_message * ulfius_websocket_pop_first_message(struct _websocket_message_list * message_list) {
  size_t len;
  struct _websocket_message * message = NULL;
  if (message_list != NULL && message_list->len > 0) {
    message = message_list->list[0];
    for (len=0; len < message_list->len-1; len++) {
      message_list->list[len] = message_list->list[len+1];
    }
    message_list->list = o_realloc(message_list->list, (message_list->len-1));
    message_list->len--;
  }
  return message;
}

/**
 * Send a message in the websocket
 * Return U_OK on success
 */
int ulfius_websocket_send_message(struct _websocket_manager * websocket_manager,
                                  const uint8_t opcode,
                                  const uint64_t data_len,
                                  const char * data) {
  int ret, i, ret_message;
  struct _websocket_message * message;
  if (websocket_manager != NULL && websocket_manager->connected) {
    if (pthread_mutex_lock(&websocket_manager->write_lock)) {
      return U_ERROR;
    }
    if (opcode == U_WEBSOCKET_OPCODE_CLOSE) {
      // If message sent is U_WEBSOCKET_OPCODE_CLOSE, wait for the response for 2 s max, then close the connection
      if (pthread_mutex_lock(&websocket_manager->read_lock)) {
        pthread_mutex_unlock(&websocket_manager->write_lock);
        return U_ERROR;
      }
      ret = ulfius_websocket_send_message_nolock(websocket_manager, opcode, data_len, data);
      for (i=0; i<40; i++) {
        message = NULL;
        ret_message = ulfius_read_incoming_message(websocket_manager, &message);
        if (message != NULL) {
          if (ulfius_push_websocket_message(websocket_manager->message_list_incoming, message) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Error pushing new websocket message in list");
          }
        }
        if (ret_message == U_WEBSOCKET_OPCODE_CLOSE) {
          break;
        }
        usleep(U_WEBSOCKET_USEC_WAIT);
      }
      websocket_manager->closing = 1;
      pthread_mutex_unlock(&websocket_manager->read_lock);
    } else {
      ret = ulfius_websocket_send_message_nolock(websocket_manager, opcode, data_len, data);
    }
    pthread_mutex_unlock(&websocket_manager->write_lock);
    return ret;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Send a message in the websocket without lock
 * Return U_OK on success
 */
int ulfius_websocket_send_message_nolock(struct _websocket_manager * websocket_manager,
                                         const uint8_t opcode,
                                         const uint64_t data_len,
                                         const char * data) {
  size_t frame_data_len;
  uint8_t * sent_data;
  int off;
  struct _websocket_message * my_message;
  if (websocket_manager != NULL &&
      websocket_manager->connected &&
     (
       opcode == U_WEBSOCKET_OPCODE_TEXT ||
       opcode == U_WEBSOCKET_OPCODE_BINARY ||
       opcode == U_WEBSOCKET_OPCODE_CLOSE ||
       opcode == U_WEBSOCKET_OPCODE_PING ||
       opcode == U_WEBSOCKET_OPCODE_PONG
     ) &&
     (data_len == 0 || data != NULL)) {
    frame_data_len = 2 + data_len;
    if (data_len > 65536) {
      frame_data_len += 8;
    } else if (data_len > 128) {
      frame_data_len += 2;
    }
    sent_data = o_malloc(frame_data_len + 1);
    my_message = o_malloc(sizeof(struct _websocket_message));
    if (sent_data != NULL && my_message != NULL) {
      if (data_len > 0) {
        my_message->data = o_malloc(data_len*sizeof(char));
        if (my_message->data == NULL) {
          o_free(sent_data);
          o_free(my_message);
          return U_ERROR_MEMORY;
        }
      }
      sent_data[0] = opcode|U_WEBSOCKET_BIT_FIN;
      my_message->opcode = opcode;
      my_message->has_mask = 0;
      memset(my_message->mask, 0, 4);
      my_message->data_len = data_len;
      if (data_len > 65536) {
        sent_data[1] = 127;
        sent_data[2] = (uint8_t)(data_len >> 54);
        sent_data[3] = (uint8_t)(data_len >> 48);
        sent_data[4] = (uint8_t)(data_len >> 40);
        sent_data[5] = (uint8_t)(data_len >> 32);
        sent_data[6] = (uint8_t)(data_len >> 24);
        sent_data[7] = (uint8_t)(data_len >> 16);
        sent_data[8] = (uint8_t)(data_len >> 8);
        sent_data[9] = (uint8_t)(data_len);
        off = 10;
      } else if (data_len > 125) {
        sent_data[1] = 126;
        sent_data[2] = (uint8_t)(data_len >> 8);
        sent_data[3] = (uint8_t)(data_len);
        off = 4;
      } else {
        sent_data[1] = (uint8_t)data_len;
        off = 2;
      }
      if (data_len > 0) {
        memcpy(sent_data + off, data, data_len);
        memcpy(my_message->data, data, data_len);
      } else {
        my_message->data = NULL;
      }
      time(&my_message->datestamp);
      ulfius_websocket_send_all(websocket_manager->sock, sent_data, frame_data_len);
      if (ulfius_push_websocket_message(websocket_manager->message_list_outcoming, my_message) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error pushing new websocket message in list");
      }
      o_free(sent_data);
      return U_OK;
    } else {
      return U_ERROR_MEMORY;
    }
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Workaround to make sure a message, as long as it can be is complete sent
 */
void ulfius_websocket_send_all(MHD_socket sock, const uint8_t * data, size_t len) {
  ssize_t ret = 0, off;
  if (data != NULL && len > 0) {
    for (off = 0; off < len; off += ret) {
      ret = send(sock, &data[off], len - off, MSG_NOSIGNAL);
      if (ret < 0) {
        break;
      }
    }
  }
}

/**
 * Centralise socket reading in this function
 * so if options or check must be done, it's done here instead of each read call
 */
size_t ulfius_websocket_recv_all(MHD_socket sock, uint8_t * data, size_t len) {
  return read(sock, data, len);
}

/**
 * Return a match list between two list of items
 * If match is NULL, then return source duplicate
 * Returned value must be u_free'd after use
 */
char * ulfius_check_list_match(const char * source, const char * match) {
  char ** source_list = NULL, ** match_list = NULL;
  char * to_return = NULL;
  int i;
  if (match == NULL) {
    to_return = o_strdup(source);
  } else {
    if (source != NULL) {
      if (split_string(source, ",", &source_list) > 0 && split_string(match, ",", &match_list) > 0) {
        for (i=0; source_list[i] != NULL; i++) {
          if (string_array_has_trimmed_value((const char **)match_list, source_list[i])) {
            if (to_return == NULL) {
              to_return = o_strdup(trimwhitespace(source_list[i]));
            } else {
              char * tmp = msprintf("%s, %s", to_return, trimwhitespace(source_list[i]));
              o_free(to_return);
              to_return = tmp;
            }
          }
        }
        free_string_array(source_list);
        free_string_array(match_list);
      }
    }
  }
  return to_return;
}

/**
 * Close the websocket
 */
int ulfius_close_websocket(struct _websocket * websocket) {
  if (websocket != NULL && websocket->websocket_manager != NULL) {
    if (websocket->websocket_onclose_callback != NULL && websocket->websocket_manager->connected) {
      // Call websocket_onclose_callback if set
      websocket->websocket_onclose_callback(websocket->request, websocket->websocket_manager, websocket->websocket_onclose_user_data);
    }
    // If websocket is still open, send opcode 0x08 (close)
    if (websocket->websocket_manager->connected) {
      if (ulfius_websocket_send_message(websocket->websocket_manager, U_WEBSOCKET_OPCODE_CLOSE, 0, NULL) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error sending close frame to websocket");
      }
    }
    websocket->websocket_manager->connected = 0;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Clear data of a websocket_manager
 */
void ulfius_clear_websocket_manager(struct _websocket_manager * websocket_manager) {
  if (websocket_manager != NULL) {
    pthread_mutex_destroy(&websocket_manager->read_lock);
    pthread_mutex_destroy(&websocket_manager->write_lock);
    ulfius_clear_websocket_message_list(websocket_manager->message_list_incoming);
    o_free(websocket_manager->message_list_incoming);
    websocket_manager->message_list_incoming = NULL;
    ulfius_clear_websocket_message_list(websocket_manager->message_list_outcoming);
    o_free(websocket_manager->message_list_outcoming);
    websocket_manager->message_list_outcoming = NULL;
  }
}

#endif
