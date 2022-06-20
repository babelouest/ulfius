/**
 *
 * Iddawc OIDC Access Token token check
 *
 * Copyright 2021-2022 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20220620
 *
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <orcania.h>
#include <ulfius.h>
#include <iddawc.h>

#include "iddawc_resource.h"

static const char * get_ip_source(const struct _u_request * request) {
  const char * ip_source = u_map_get_case(request->map_header, "X-Forwarded-For");
  
  if (ip_source == NULL) {
    struct sockaddr_in * in_source = (struct sockaddr_in *)request->client_address;
    if (in_source != NULL) {
      ip_source = inet_ntoa(in_source->sin_addr);
    } else {
      ip_source = "NOT_FOUND";
    }
  }
  
  return ip_source;
};

static const char * get_auth_header_token(const char * auth_header, int * is_header_dpop) {
  if (0 == o_strncmp(HEADER_PREFIX_BEARER, auth_header, HEADER_PREFIX_BEARER_LEN)) {
    *is_header_dpop = 0;
    return auth_header + HEADER_PREFIX_BEARER_LEN;
  } else if (0 == o_strncmp(HEADER_PREFIX_DPOP, auth_header, HEADER_PREFIX_DPOP_LEN)) {
    *is_header_dpop = 1;
    return auth_header + HEADER_PREFIX_DPOP_LEN;
  } else {
    return NULL;
  }
}

/**
 * Validates if an access_token grants has a valid scope
 * return the final scope list on success
 */
int jwt_profile_access_token_check_scope(struct _iddawc_resource_config * config, json_t * j_access_token) {
  int i, scope_count_token, scope_count_expected, ret;
  char ** scope_list_token = NULL, ** scope_list_expected = NULL;
  json_t * j_scope_final_list = json_array();
  
  if (j_scope_final_list != NULL) {
    if (j_access_token != NULL) {
      scope_count_token = split_string(json_string_value(json_object_get(j_access_token, "scope")), " ", &scope_list_token);
      if (!o_strnullempty(config->oauth_scope)) {
        scope_count_expected = split_string(config->oauth_scope, " ", &scope_list_expected);
        if (scope_count_token > 0 && scope_count_expected > 0) {
          for (i=0; scope_count_expected > 0 && scope_list_expected[i] != NULL; i++) {
            if (string_array_has_value((const char **)scope_list_token, scope_list_expected[i])) {
              json_array_append_new(j_scope_final_list, json_string(scope_list_expected[i]));
            }
          }
          if (json_array_size(j_scope_final_list) > 0) {
            ret = I_TOKEN_OK;
          } else {
            ret = I_TOKEN_ERROR_INSUFFICIENT_SCOPE;
          }
        } else {
          ret = I_TOKEN_ERROR_INTERNAL;
        }
        free_string_array(scope_list_expected);
      } else {
        ret = I_TOKEN_OK;
      }
      free_string_array(scope_list_token);
    } else {
      ret = I_TOKEN_ERROR_INVALID_TOKEN;
    }
  } else {
    ret = I_TOKEN_ERROR_INTERNAL;
  }
  json_decref(j_scope_final_list);
  return ret;
}

/**
 * check if bearer token has some of the specified scope
 */
int callback_check_jwt_profile_access_token (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct _iddawc_resource_config * config = (struct _iddawc_resource_config *)user_data;
  json_t * j_access_token = NULL;
  int res = U_CALLBACK_UNAUTHORIZED, res_validity, is_header_dpop = 0;
  const char * token_value = NULL, * dpop = u_map_get_case(request->map_header, HEADER_DPOP);
  char * response_value = NULL, * htu;
  
  if (config != NULL) {
    switch (config->method) {
      case I_METHOD_HEADER:
        if (u_map_get_case(request->map_header, HEADER_AUTHORIZATION) != NULL) {
          token_value = get_auth_header_token(u_map_get_case(request->map_header, HEADER_AUTHORIZATION), &is_header_dpop);
        }
        break;
      case I_METHOD_BODY:
        if (o_strstr(u_map_get(request->map_header, ULFIUS_HTTP_HEADER_CONTENT), MHD_HTTP_POST_ENCODING_FORM_URLENCODED) != NULL && u_map_get(request->map_post_body, BODY_URL_PARAMETER) != NULL) {
          token_value = u_map_get(request->map_post_body, BODY_URL_PARAMETER);
        }
        break;
      case I_METHOD_URL:
        token_value = u_map_get(request->map_url, BODY_URL_PARAMETER);
        break;
    }
    if (token_value != NULL) {
      if (!pthread_mutex_lock(&config->session_lock)) {
        i_set_str_parameter(config->session, I_OPT_ACCESS_TOKEN, token_value);
        if (i_verify_jwt_access_token(config->session, config->aud) == I_OK) {
          j_access_token = json_deep_copy(config->session->access_token_payload);
          if ((res_validity = jwt_profile_access_token_check_scope(config, j_access_token)) == I_TOKEN_ERROR_INSUFFICIENT_SCOPE) {
            response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"insufficient_scope\",error_description=\"The scope is invalid\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
            u_map_put(response->map_header, HEADER_RESPONSE, response_value);
            o_free(response_value);
          } else if (res_validity != I_TOKEN_OK) {
            response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"Internal server error\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
            u_map_put(response->map_header, HEADER_RESPONSE, response_value);
            o_free(response_value);
          } else {
            if (is_header_dpop && json_object_get(json_object_get(j_access_token, "cnf"), "jkt") != NULL && dpop != NULL) {
              htu = msprintf("%s%s", config->resource_url_root, request->url_path+1);
              if (i_verify_dpop_proof(u_map_get(request->map_header, I_HEADER_DPOP), request->http_verb, htu, config->dpop_max_iat, json_string_value(json_object_get(json_object_get(j_access_token, "cnf"), "jkt")), token_value) == I_OK) {
                res = U_CALLBACK_CONTINUE;
                if (ulfius_set_response_shared_data(response, json_deep_copy(j_access_token), (void (*)(void *))&json_decref) != U_OK) {
                  res = U_CALLBACK_ERROR;
                }
              } else {
                response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"The DPoP token is invalid\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
                u_map_put(response->map_header, HEADER_RESPONSE, response_value);
                o_free(response_value);
              }
              o_free(htu);
            } else if (!is_header_dpop && json_object_get(json_object_get(j_access_token, "cnf"), "jkt") == NULL && dpop == NULL) {
              res = U_CALLBACK_CONTINUE;
              if (ulfius_set_response_shared_data(response, json_deep_copy(j_access_token), (void (*)(void *))&json_decref) != U_OK) {
                res = U_CALLBACK_ERROR;
              }
            }
          }
        } else {
          y_log_message(Y_LOG_LEVEL_WARNING, "Security - Invalid access token from address %s", get_ip_source(request));
          response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"The access token is invalid\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
          u_map_put(response->map_header, HEADER_RESPONSE, response_value);
          o_free(response_value);
        }
        json_decref(j_access_token);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_check_jwt_profile_access_token - Error pthread_mutex_lock");
        response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"Internal server error\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
        u_map_put(response->map_header, HEADER_RESPONSE, response_value);
        o_free(response_value);
      }
      pthread_mutex_unlock(&config->session_lock);
    } else {
      y_log_message(Y_LOG_LEVEL_WARNING, "Security - Missing access token from address %s", get_ip_source(request));
      response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_token\",error_description=\"The access token is missing\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
      u_map_put(response->map_header, HEADER_RESPONSE, response_value);
      o_free(response_value);
    }
  }
  return res;
}

int i_jwt_profile_access_token_init_config(struct _iddawc_resource_config * config, unsigned short method, const char * realm, const char * aud, const char * oauth_scope, const char * resource_url_root, time_t dpop_max_iat) {
  int ret;
  pthread_mutexattr_t mutexattr;
  
  if (config != NULL) {
    config->method = method;
    config->realm = o_strdup(realm);
    config->aud = o_strdup(aud);
    config->oauth_scope = o_strdup(oauth_scope);
    config->resource_url_root = o_strdup(resource_url_root);
    config->dpop_max_iat = dpop_max_iat;
    
    if ((config->session = o_malloc(sizeof(struct _i_session))) != NULL) {
      if (i_init_session(config->session) == I_OK) {
        pthread_mutexattr_init ( &mutexattr );
        pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
        if (pthread_mutex_init(&config->session_lock, &mutexattr) == 0) {
          ret = I_TOKEN_OK;
        } else {
          ret = I_TOKEN_ERROR_INTERNAL;
        }
        pthread_mutexattr_destroy(&mutexattr);
      } else {
        ret = I_TOKEN_ERROR_INTERNAL;
      }
    } else {
      ret = I_TOKEN_ERROR_INTERNAL;
    }
  } else {
    ret = I_TOKEN_ERROR_INVALID_REQUEST;
  }
  return ret;
}

int i_jwt_profile_access_token_load_config(struct _iddawc_resource_config * config, const char * config_url, int verify_cert) {
  int ret = 1;
  
  do {
    if (i_set_parameter_list(config->session, I_OPT_OPENID_CONFIG_ENDPOINT, config_url,
                                              I_OPT_REMOTE_CERT_FLAG, verify_cert?(I_REMOTE_HOST_VERIFY_PEER|I_REMOTE_HOST_VERIFY_HOSTNAME|I_REMOTE_PROXY_VERIFY_PEER|I_REMOTE_PROXY_VERIFY_HOSTNAME):I_REMOTE_VERIFY_NONE,
                                              I_OPT_NONE) != I_OK) {
      ret = 0;
      y_log_message(Y_LOG_LEVEL_ERROR, "oidc_resource_init_config - Error i_set_parameter_list");
    }
    
    if (i_get_openid_config(config->session) != I_OK) {
      ret = 0;
      y_log_message(Y_LOG_LEVEL_ERROR, "oidc_resource_init_config - Error i_get_openid_config");
    }
  } while (0);
  return ret;
}

int i_jwt_profile_access_token_load_jwks(struct _iddawc_resource_config * config, json_t * j_jwks, const char * iss) {
  int ret;
  
  if (i_set_str_parameter(config->session, I_OPT_ISSUER, iss) == I_OK && i_set_server_jwks(config->session, j_jwks) == I_OK) {
    ret = 1;
  } else {
    ret = 0;
  }
  
  return ret;
}

void i_jwt_profile_access_token_close_config(struct _iddawc_resource_config * config) {
  if (config != NULL) {
    i_clean_session(config->session);
    o_free(config->realm);
    o_free(config->aud);
    o_free(config->oauth_scope);
    o_free(config->resource_url_root);
    o_free(config->session);
    pthread_mutex_destroy(&config->session_lock);
  }
}
