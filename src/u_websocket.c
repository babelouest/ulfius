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
#ifndef U_DISABLE_WEBSOCKET
#include <openssl/ssl.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "ulfius.h"

struct _websocket_manager_thread_arg {
  struct _websocket_manager_cls * cls;
  struct _u_request * request;
  int (* websocket_manager_callback) (const struct _u_request * request,
                                      const struct _websocket_manager_cls * websocket_manager_cls,
                                      void * websocket_manager_user_data);
  void * websocket_manager_user_data;
};

void * thread_websocket_manager_run(void * args) {
  struct _websocket_manager_thread_arg * ws_arg = (struct _websocket_manager_thread_arg *)args;
  if (ws_arg != NULL) {
    ws_arg->websocket_manager_callback(ws_arg->request, ws_arg->cls, ws_arg->websocket_manager_user_data);
  }
  return NULL;
}

struct _websocket_incoming_thread_arg {
  struct _websocket_manager_cls * cls;
  struct _websocket_message * message;
  struct _u_request * request;
  int (* websocket_incoming_message_callback) (const struct _u_request * request,
                                               const struct _websocket_manager_cls * websocket_manager_cls,
                                               const struct _websocket_message * message,
                                               void * websocket_incoming_user_data);
  void * websocket_incoming_user_data;
};

void * thread_websocket_incoming_run(void * args) {
  struct _websocket_incoming_thread_arg * ws_arg = (struct _websocket_incoming_thread_arg *)args;
  if (ws_arg != NULL) {
    ws_arg->websocket_incoming_message_callback(ws_arg->request, ws_arg->cls, ws_arg->message, ws_arg->websocket_incoming_user_data);
  }
  return NULL;
}

void ulfius_start_websocket_cb (void *cls,
            struct MHD_Connection *connection,
            void * con_cls,
            const char * extra_in,
            size_t extra_in_size,
            MHD_socket sock,
            struct MHD_UpgradeResponseHandle *urh) {
  int len, flags, i, message_complete = 0, message_error = 0, close_websocket = 0;
  size_t msg_len;
  struct _websocket_cls * ws_cls = (struct _websocket_cls*)cls;
  struct _websocket_manager_cls wsm_cls;
  uint8_t header[2], payload_len[7], masking_key[4];
  char * payload_data;
  struct _websocket_message_list message_list;
  struct _websocket_message * message = NULL;
  pthread_t thread_websocket_manager, thread_websocket_incoming;
  int thread_ret_websocket_manager = 0, thread_detach_websocket_manager = 0, thread_ret_websocket_incoming = 0, thread_detach_websocket_incoming = 0;
  
  init_message_list(&message_list);
  y_log_message(Y_LOG_LEVEL_DEBUG, "receiving");
  flags = fcntl (sock, F_GETFL);
  if (-1 == flags)
    return;
  if ((flags & ~O_NONBLOCK) != flags)
    if (-1 == fcntl (sock, F_SETFL, flags & ~O_NONBLOCK & O_RSYNC))
      return;
  wsm_cls.message_list = &message_list;
  wsm_cls.sock = sock;
  // Run websocket manager in a thread if set
  if (ws_cls->websocket_manager_callback != NULL) {
    struct _websocket_manager_thread_arg arg;
    arg.cls = &wsm_cls;
    arg.request = ws_cls->request;
    arg.websocket_manager_callback = ws_cls->websocket_manager_callback;
    arg.websocket_manager_user_data = ws_cls->websocket_manager_user_data;
    thread_ret_websocket_manager = pthread_create(&thread_websocket_manager, NULL, thread_websocket_manager_run, (void *)&arg);
    thread_detach_websocket_manager = pthread_detach(thread_websocket_manager);
    if (thread_ret_websocket_manager || thread_detach_websocket_manager) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error creating or detaching websocket manager thread, return code: %d, detach code: %d",
                  thread_ret_websocket_manager, thread_detach_websocket_manager);
      clear_message_list(&message_list);
      o_free(cls);
      return;
    }
  }
  while (!close_websocket) {
    do {
      message_error = 0;
      y_log_message(Y_LOG_LEVEL_DEBUG, "read for message");
      len = read(sock, header, 2);
      // New frame has arrived
      if (len == 2) {
        if (message == NULL) {
          y_log_message(Y_LOG_LEVEL_DEBUG, "new message");
          message = o_malloc(sizeof(struct _websocket_message));
          message->data_len = 0;
          message->has_mask = 0;
          message->opcode = header[0] & 0x0F;
          message->data = NULL;
          close_websocket = (message->opcode == WEBSOCKET_OPCODE_CLOSE);
        } else {
          y_log_message(Y_LOG_LEVEL_DEBUG, "continue message");
        }
        if ((header[1] & WEBSOCKET_LEN_MASK) <= 125) {
          msg_len = (header[1] & WEBSOCKET_LEN_MASK);
        } else if ((header[1] & WEBSOCKET_LEN_MASK) == 126) {
          len = read(sock, payload_len, 2);
          if (len == 2) {
            msg_len = payload_len[1] | ((uint64_t)payload_len[0] << 8);
          } else {
            message_error = 1;
            y_log_message(Y_LOG_LEVEL_ERROR, "Error reading socket for length");
          }
        } else if ((header[1] & WEBSOCKET_LEN_MASK) == 127) {
          len = read(sock, payload_len, 8);
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
            y_log_message(Y_LOG_LEVEL_ERROR, "Error reading socket for length");
          }
        }
        if (!message_error) {
          if (header[1] & WEBSOCKET_HAS_MASK) {
            message->has_mask = 1;
            len = read(sock, masking_key, 4);
            if (len != 4) {
              message_error = 1;
              y_log_message(Y_LOG_LEVEL_ERROR, "Error reading socket for mask");
            }
          }
          if (!message_error) {
            payload_data = o_malloc(msg_len*sizeof(uint8_t));
            len = read(sock, payload_data, msg_len);
            if (len == msg_len) {
              // If mask, decode message
              if (message->has_mask) {
                message->data = o_realloc(message->data, (msg_len+message->data_len)*sizeof(uint8_t));
                for (i = message->data_len; i < msg_len; i++) {
                  message->data[i] = payload_data[i-message->data_len] ^ masking_key[(i-message->data_len)%4];
                }
              } else {
                memcpy(message->data+message->data_len, payload_data, msg_len);
              }
              message->data_len += msg_len;
            } else {
              message_error = 1;
              y_log_message(Y_LOG_LEVEL_ERROR, "Error reading socket for payload_data");
            }
            o_free(payload_data);
            if (!message_error && (header[0] & WEBSOCKET_BIT_FIN)) {
              push_message(&message_list, message);
              if (ws_cls->websocket_incoming_message_callback != NULL) {
                struct _websocket_incoming_thread_arg arg;
                arg.cls = &wsm_cls;
                arg.message = message;
                arg.request = ws_cls->request;
                arg.websocket_incoming_message_callback = ws_cls->websocket_incoming_message_callback;
                arg.websocket_incoming_user_data = ws_cls->websocket_incoming_user_data;
                thread_ret_websocket_incoming = pthread_create(&thread_websocket_incoming, NULL, thread_websocket_incoming_run, (void *)&arg);
                thread_detach_websocket_incoming = pthread_detach(thread_websocket_incoming);
                if (thread_ret_websocket_incoming || thread_detach_websocket_incoming) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Error creating or detaching websocket manager thread, return code: %d, detach code: %d",
                              thread_ret_websocket_incoming, thread_detach_websocket_incoming);
                  clear_message_list(&message_list);
                  o_free(cls);
                  return;
                }
              }
              message = NULL;
              message_complete = 1;
            }
          }
        }
      } else {
        message_error = 1;
        y_log_message(Y_LOG_LEVEL_ERROR, "Error reading socket for header, len is %d", len);
      }
    } while (!message_complete && !message_error);
  }
  y_log_message(Y_LOG_LEVEL_DEBUG, "closing");
  clear_message_list(&message_list);
  o_free(cls);
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
    intermediate = msprintf("%s%s", key, WEBSOCKET_MAGIC_STRING);
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
    free(intermediate);
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
    message_list->list = realloc(message_list->list, (message_list->len-1));
    message_list->len--;
  }
  return message;
}

int ulfius_websocket_send_message(const struct _websocket_manager_cls * websocket_manager_cls, struct _websocket_message * message) {
  size_t data_len;
  uint8_t * sent_data;
  int off;
  
  if (message != NULL) {
    data_len = 2 + message->data_len;
    if (message->has_mask) {
      data_len += 4;
    }
    if (message->data_len > 65536) {
      data_len += 8;
    } else if (message->data_len > 128) {
      data_len += 2;
    }
    sent_data = o_malloc(data_len + 1);
    if (sent_data != NULL) {
      sent_data[0] = message->opcode|WEBSOCKET_BIT_FIN;
      if (message->data_len > 65536) {
        sent_data[1] = (message->has_mask?WEBSOCKET_HAS_MASK:0x00)|127;
        sent_data[2] = (uint8_t)(message->data_len >> 54);
        sent_data[3] = (uint8_t)(message->data_len >> 48);
        sent_data[4] = (uint8_t)(message->data_len >> 40);
        sent_data[5] = (uint8_t)(message->data_len >> 32);
        sent_data[6] = (uint8_t)(message->data_len >> 24);
        sent_data[7] = (uint8_t)(message->data_len >> 16);
        sent_data[8] = (uint8_t)(message->data_len >> 8);
        sent_data[9] = (uint8_t)(message->data_len);
        off = 10;
      } else if (message->data_len > 125) {
        sent_data[1] = (message->has_mask?WEBSOCKET_HAS_MASK:0x00)|126;
        sent_data[2] = (uint8_t)(message->data_len >> 8);
        sent_data[3] = (uint8_t)(message->data_len);
        off = 4;
      } else {
        sent_data[1] = (message->has_mask?WEBSOCKET_HAS_MASK:0x00)|(uint8_t)message->data_len;
        off = 2;
      }
      if (message->has_mask) {
        sent_data[off] = message->mask[0];
        sent_data[off+1] = message->mask[1];
        sent_data[off+2] = message->mask[2];
        sent_data[off+3] = message->mask[3];
        off += 4;
      }
      if (message->data_len > 0) {
        memcpy(sent_data + off, message->data, message->data_len);
      }
      send_all(websocket_manager_cls->sock, sent_data, data_len);
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
  ssize_t ret = 0;
  for (size_t off = 0; off < len; off += ret) {
    ret = write (sock, &data[off], len - off);
  }
}

#endif
