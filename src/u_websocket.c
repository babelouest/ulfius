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
#include <openssl/ssl.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "ulfius.h"

void * thread_websocket_manager_run(void * args) {
  struct _websocket_manager_thread_arg * ws_arg = (struct _websocket_manager_thread_arg *)args;
  if (ws_arg != NULL && ws_arg->websocket_manager_callback != NULL && ws_arg->websocket->websocket_manager != NULL) {
    ws_arg->websocket_manager_callback(ws_arg->request, ws_arg->websocket->websocket_manager, ws_arg->websocket_manager_user_data);
    ws_arg->websocket->websocket_manager->closed = 1;
  }
  return NULL;
}

int read_incoming_message(struct _websocket * websocket, struct _websocket_message ** message) {
  int len, opcode, i;
  int message_complete = 0, message_error;
  uint8_t header[2], payload_len[8], masking_key[4];
  char * payload_data;
  size_t msg_len;
  
  (*message) = NULL;
  do {
    message_error = 0;
    len = read(websocket->websocket_manager->sock, header, 2);
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
        len = read(websocket->websocket_manager->sock, payload_len, 2);
        if (len == 2) {
          msg_len = payload_len[1] | ((uint64_t)payload_len[0] << 8);
        } else {
          message_error = 1;
          opcode = U_WEBSOCKET_OPCODE_ERROR;
          y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket message length");
        }
      } else if ((header[1] & U_WEBSOCKET_LEN_MASK) == 127) {
        len = read(websocket->websocket_manager->sock, payload_len, 8);
        if (len == 8) {
          msg_len = payload_len[7] |
                    ((uint64_t)payload_len[6] << 8) |
                    ((uint64_t)payload_len[5] << 16) |
                    ((uint64_t)payload_len[4] << 24) |
                    ((uint64_t)payload_len[3] << 32) |
                    ((uint64_t)payload_len[2] << 40) |
                    ((uint64_t)payload_len[1] << 48) |
                    ((uint64_t)payload_len[0] << 54);
        } else {
          message_error = 1;
          opcode = U_WEBSOCKET_OPCODE_ERROR;
          y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket message length");
        }
      }
      if (!message_error) {
        if (header[1] & U_WEBSOCKET_HAS_MASK) {
          (*message)->has_mask = 1;
          len = read(websocket->websocket_manager->sock, masking_key, 4);
          if (len != 4) {
            message_error = 1;
            opcode = U_WEBSOCKET_OPCODE_ERROR;
            y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket for mask");
          }
        }
        if (!message_error) {
          if (msg_len > 0) {
            payload_data = o_malloc(msg_len*sizeof(uint8_t));
            len = read(websocket->websocket_manager->sock, payload_data, msg_len);
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
            } else {
              message_error = 1;
              opcode = U_WEBSOCKET_OPCODE_ERROR;
              y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket for payload_data");
            }
            o_free(payload_data);
          }
          if (!message_error && (header[0] & U_WEBSOCKET_BIT_FIN)) {
            opcode = (*message)->opcode;
            message_complete = 1;
          }
        }
      }
    } else if (len == 0) {
      // Socket closed
      websocket->websocket_manager->connected = 0;
      opcode = U_WEBSOCKET_OPCODE_ERROR;
      message_error = 1;
    } else {
      opcode = U_WEBSOCKET_OPCODE_ERROR;
      message_error = 1;
      y_log_message(Y_LOG_LEVEL_ERROR, "Error reading websocket for header, len is %d", len);
    }
  } while (!message_complete && !message_error);
  return opcode;
}

void ulfius_start_websocket_cb (void *cls,
            struct MHD_Connection *connection,
            void * con_cls,
            const char * extra_in,
            size_t extra_in_size,
            MHD_socket sock,
            struct MHD_UpgradeResponseHandle * urh) {
  int flags, opcode;
  struct _websocket * websocket = (struct _websocket*)cls;
  struct _websocket_message * message = NULL;
  pthread_t thread_websocket_manager;
  int thread_ret_websocket_manager = 0, thread_detach_websocket_manager = 0;
  
  if (websocket != NULL) {
    websocket->urh = urh;
    // Set socket blocking mode
    flags = fcntl (sock, F_GETFL);
    if (-1 == flags) {
      return;
    }
    if ((flags & ~O_NONBLOCK) != flags && -1 == fcntl (sock, F_SETFL, flags & ~O_NONBLOCK)) {
      return;
    }
    websocket->websocket_manager = o_malloc(sizeof(struct _websocket_manager));
    // Run websocket manager in a thread if set
    if (websocket->websocket_manager_callback != NULL) {
      websocket->websocket_manager->closed = 0;
      struct _websocket_manager_thread_arg arg;
      websocket->websocket_manager->message_list_incoming = o_malloc(sizeof(struct _websocket_message_list));
      websocket->websocket_manager->message_list_outcoming = o_malloc(sizeof(struct _websocket_message_list));
      init_message_list(websocket->websocket_manager->message_list_incoming);
      init_message_list(websocket->websocket_manager->message_list_outcoming);
      websocket->websocket_manager->sock = sock;
      websocket->websocket_manager->connected = 1;
      arg.websocket = websocket;
      arg.request = websocket->request;
      arg.websocket_manager_callback = websocket->websocket_manager_callback;
      arg.websocket_manager_user_data = websocket->websocket_manager_user_data;
      thread_ret_websocket_manager = pthread_create(&thread_websocket_manager, NULL, thread_websocket_manager_run, (void *)&arg);
      thread_detach_websocket_manager = pthread_detach(thread_websocket_manager);
      if (thread_ret_websocket_manager || thread_detach_websocket_manager) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error creating or detaching websocket manager thread, return code: %d, detach code: %d",
                    thread_ret_websocket_manager, thread_detach_websocket_manager);
        shutdown_websocket(websocket);
        return;
      }
    } else {
      websocket->websocket_manager->closed = 1;
    }
    while (websocket->websocket_manager->connected) {
      message = NULL;
      opcode = read_incoming_message(websocket, &message);
      if (opcode != U_WEBSOCKET_OPCODE_ERROR) {
        if (opcode == U_WEBSOCKET_OPCODE_CLOSE) {
          // Send close command back, then close the socket
          if (websocket->websocket_manager->connected && ulfius_websocket_send_message(websocket->websocket_manager, U_WEBSOCKET_OPCODE_CLOSE, 0, NULL) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Error sending close command");
          }
          websocket->websocket_manager->connected = 0;
        } else {
          if (websocket->websocket_incoming_message_callback != NULL) {
            websocket->websocket_incoming_message_callback(websocket->request, websocket->websocket_manager, message, websocket->websocket_incoming_user_data);
          }
        }
        push_message(websocket->websocket_manager->message_list_incoming, message);
      }
    }
  }
  while (!websocket->websocket_manager->closed) {
    usleep(50);
  }
  shutdown_websocket(websocket);
}

int shutdown_websocket(struct _websocket * websocket) {
  if (websocket != NULL) {
    ulfius_close_websocket(websocket);
    if (MHD_upgrade_action (websocket->urh, MHD_UPGRADE_ACTION_CLOSE) != MHD_YES) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error sending MHD_UPGRADE_ACTION_CLOSE frame to urh");
    }
    ulfius_instance_remove_websocket_active(websocket->instance, websocket);
    clear_websocket(websocket);
    o_free(websocket);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int generate_handshake_answer(const char * key, char * out_digest) {
  EVP_MD_CTX mdctx;
  const EVP_MD *md;
  unsigned char md_value[32] = {0};
  unsigned int md_len, res = 0;
  
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;
  char * intermediate, buffer[32] = {0};
  
  if (key != NULL && out_digest != NULL) {
    md = EVP_sha1();
    intermediate = msprintf("%s%s", key, U_WEBSOCKET_MAGIC_STRING);
    EVP_MD_CTX_init(&mdctx);
    if (EVP_DigestInit_ex(&mdctx, md, NULL) && 
        EVP_DigestUpdate(&mdctx,
                         intermediate,
                         (unsigned int) strlen(intermediate)) &&
        EVP_DigestFinal_ex(&mdctx,
                           md_value,
                           &md_len)) {
      memcpy(buffer, md_value, md_len);
      b64 = BIO_new(BIO_f_base64());
      bio = BIO_new(BIO_s_mem());
      bio = BIO_push(b64, bio);

      BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
      if (BIO_write(bio, buffer, md_len) > 0) {
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        memcpy(out_digest, (*bufferPtr).data, (*bufferPtr).length);
        
        BIO_set_close(bio, BIO_CLOSE);
        BIO_free_all(bio);
        EVP_MD_CTX_cleanup(&mdctx);
        res = 1;
      }
    }
    o_free(intermediate);
  }
  return res;
}

int init_message_list(struct _websocket_message_list * message_list) {
  if (message_list != NULL) {
    message_list->len = 0;
    message_list->list = NULL;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

void clear_message_list(struct _websocket_message_list * message_list) {
  size_t i;
  if (message_list != NULL) {
    for (i=0; i < message_list->len; i++) {
      clear_message(message_list->list[i]);
      o_free(message_list->list[i]);
    }
    o_free(message_list->list);
  }
}

void clear_message(struct _websocket_message * message) {
  if (message != NULL) {
    o_free(message->data);
  }
}

int push_message(struct _websocket_message_list * message_list, struct _websocket_message * message) {
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

struct _websocket_message * pop_first_message(struct _websocket_message_list * message_list) {
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

int ulfius_websocket_send_message(const struct _websocket_manager * websocket_manager,
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
      send_all(websocket_manager->sock, sent_data, frame_data_len);
      push_message(websocket_manager->message_list_outcoming, my_message);
      o_free(sent_data);
      return U_OK;
    } else {
      return U_ERROR_MEMORY;
    }
  } else {
    return U_ERROR_PARAMS;
  }
}

void send_all(MHD_socket sock, const uint8_t * data, size_t len) {
  ssize_t ret = 0, off;
  if (data != NULL && len > 0) {
    for (off = 0; off < len; off += ret) {
      ret = write(sock, &data[off], len - off);
      if (ret < 0) {
        break;
      }
    }
  }
}

/**
 * Initialize websocket in the response
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
int ulfius_init_websocket_response(struct _u_response * response,
                                   const char * websocket_protocol,
                                   const char * websocket_extensions, 
                                   void (* websocket_manager_callback) (const struct _u_request * request,
                                                                       const struct _websocket_manager * websocket_manager,
                                                                       void * websocket_manager_user_data),
                                   void * websocket_manager_user_data,
                                   void (* websocket_incoming_message_callback) (const struct _u_request * request,
                                                                                const struct _websocket_manager * websocket_manager,
                                                                                const struct _websocket_message * message,
                                                                                void * websocket_incoming_user_data),
                                   void * websocket_incoming_user_data,
                                   void (* websocket_onclose_callback) (const struct _u_request * request,
                                                                       const struct _websocket_manager * websocket_manager,
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

char * check_list_match(const char * source, const char * match) {
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

int ulfius_close_websocket(struct _websocket * websocket) {
  if (websocket != NULL && websocket->websocket_manager != NULL && websocket->urh != NULL) {
    if (websocket->websocket_onclose_callback != NULL) {
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

int ulfius_instance_add_websocket_active(struct _u_instance * instance, struct _websocket * websocket) {
  if (instance != NULL && websocket != NULL) {
    instance->websocket_active = o_realloc(instance->websocket_active, (instance->nb_websocket_active+1)*sizeof(struct _websocket *));
    if (instance->websocket_active != NULL) {
      instance->websocket_active[instance->nb_websocket_active] = websocket;
      instance->nb_websocket_active++;
      return U_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for instance->websocket_active");
      return U_ERROR_MEMORY;
    }
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_instance_remove_websocket_active(struct _u_instance * instance, struct _websocket * websocket) {
  size_t i, j;
  if (instance != NULL && instance->websocket_active != NULL && websocket != NULL) {
    for (i=0; i<instance->nb_websocket_active; i++) {
      if (instance->websocket_active[i] == websocket) {
        if (instance->nb_websocket_active > 1) {
          for (j=i; j<instance->nb_websocket_active-1; j++) {
            instance->websocket_active[j] = instance->websocket_active[j+1];
          }
          instance->websocket_active = o_realloc(instance->websocket_active, (instance->nb_websocket_active-1)*sizeof(struct _websocket *));
          if (instance->websocket_active == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for instance->websocket_active");
            return U_ERROR_MEMORY;
          }
        } else {
          o_free(instance->websocket_active);
          instance->websocket_active = NULL;
        }
        instance->nb_websocket_active--;
        return U_OK;
      }
    }
    return U_ERROR_NOT_FOUND;
  } else {
    return U_ERROR_PARAMS;
  }
}

void clear_websocket(struct _websocket * websocket) {
  if (websocket != NULL) {
    clear_websocket_manager(websocket->websocket_manager);
    o_free(websocket->websocket_manager);
    websocket->websocket_manager = NULL;
  }
}

void clear_websocket_manager(struct _websocket_manager * websocket_manager) {
  if (websocket_manager != NULL) {
    clear_message_list(websocket_manager->message_list_incoming);
    o_free(websocket_manager->message_list_incoming);
    websocket_manager->message_list_incoming = NULL;
    clear_message_list(websocket_manager->message_list_outcoming);
    o_free(websocket_manager->message_list_outcoming);
    websocket_manager->message_list_outcoming = NULL;
  }
}

#endif
