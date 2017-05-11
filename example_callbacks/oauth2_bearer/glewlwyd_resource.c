/**
 *
 * Glewlwyd OAuth2 Authorization token check
 *
 * Copyright 2016-2017 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>
#include <time.h>
#include <orcania.h>
#include <ulfius.h>
#include <jansson.h>

#include "glewlwyd_resource.h"

/**
 * check if bearer token has the specified scope
 */
int callback_check_glewlwyd_access_token (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct _glewlwyd_resource_config * config = (struct _glewlwyd_resource_config *)user_data;
  json_t * j_access_token = NULL;
  int res = U_ERROR_UNAUTHORIZED, res_scope, res_validity;
  const char * token_value = NULL;
  char * response_value = NULL;
  
  if (config != NULL) {
    switch (config->method) {
      case G_METHOD_HEADER:
        if (u_map_get(request->map_header, HEADER_AUTHORIZATION) != NULL) {
          if (o_strstr(u_map_get(request->map_header, HEADER_AUTHORIZATION), HEADER_PREFIX_BEARER) == u_map_get(request->map_header, HEADER_AUTHORIZATION)) {
            token_value = u_map_get(request->map_header, HEADER_AUTHORIZATION) + strlen(HEADER_PREFIX_BEARER);
          }
        }
        break;
      case G_METHOD_BODY:
        if (o_strstr(u_map_get(request->map_header, ULFIUS_HTTP_HEADER_CONTENT), MHD_HTTP_POST_ENCODING_FORM_URLENCODED) != NULL && u_map_get(request->map_post_body, BODY_URL_PARAMETER) != NULL) {
          token_value = u_map_get(request->map_post_body, BODY_URL_PARAMETER);
        }
        break;
      case G_METHOD_URL:
        token_value = u_map_get(request->map_url, BODY_URL_PARAMETER);
        break;
    }
    if (token_value != NULL) {
      j_access_token = access_token_check_signature(config, token_value);
      if (check_result_value(j_access_token, G_OK)) {
        res_validity = access_token_check_validity(config, json_object_get(j_access_token, "grants"));
        if (res_validity == G_OK) {
          res_scope = access_token_check_scope(config, json_object_get(j_access_token, "grants"));
          if (res_scope == G_ERROR_INSUFFICIENT_SCOPE) {
            response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"insufficient_scope\",error_description=\"The scope is invalid\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
            u_map_put(response->map_header, HEADER_RESPONSE, response_value);
            o_free(response_value);
          } else if (res_scope != G_OK) {
            response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"Internal server error\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
            u_map_put(response->map_header, HEADER_RESPONSE, response_value);
            o_free(response_value);
          } else {
            res = U_OK;
          }
        } else if (res_validity == G_ERROR_INVALID_TOKEN) {
          response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"The access token is invalid\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
          u_map_put(response->map_header, HEADER_RESPONSE, response_value);
          o_free(response_value);
        } else {
          response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"Internal server error\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
          u_map_put(response->map_header, HEADER_RESPONSE, response_value);
          o_free(response_value);
        }
      } else {
        response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"The access token is invalid\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
        u_map_put(response->map_header, HEADER_RESPONSE, response_value);
        o_free(response_value);
      }
      json_decref(j_access_token);
    } else {
      response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_token\",error_description=\"The access token is missing\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
      u_map_put(response->map_header, HEADER_RESPONSE, response_value);
      o_free(response_value);
    }
  }
  return res;
}

/**
 * Validates if an access_token grants has a valid scope
 */
int access_token_check_scope(struct _glewlwyd_resource_config * config, json_t * j_access_token) {
  int res, i, scope_count_token, scope_count_expected, count;
  char ** scope_list_token, ** scope_list_expected;
  
  if (j_access_token != NULL) {
    scope_count_token = split_string(json_string_value(json_object_get(j_access_token, "scope")), " ", &scope_list_token);
    scope_count_expected = split_string(config->oauth_scope, " ", &scope_list_expected);
    count = 0;
    if (scope_count_token > 0 && scope_count_expected > 0) {
      for (i=0; scope_count_expected > 0 && scope_list_expected[i] != NULL; i++) {
        if (string_array_has_value((const char **)scope_list_token, scope_list_expected[i])) {
          count++;
        }
      }
      res = (count==scope_count_expected?G_OK:G_ERROR_INSUFFICIENT_SCOPE);
    } else {
      res = G_ERROR_INVALID_TOKEN;
    }
    free_string_array(scope_list_token);
    free_string_array(scope_list_expected);
  } else {
    res = G_ERROR_INVALID_TOKEN;
  }
  return res;
}

/**
 * Validates if an access_token grants has valid parameters:
 * - username: non empty string
 * - type: match "access_token"
 * - iat + expires_in < now
 */
int access_token_check_validity(struct _glewlwyd_resource_config * config, json_t * j_access_token) {
  time_t now;
  json_int_t expiration;
  int res;
  
  if (j_access_token != NULL) {
    // Token is valid, check type and expiration date
    time(&now);
    expiration = json_integer_value(json_object_get(j_access_token, "iat")) + json_integer_value(json_object_get(j_access_token, "expires_in"));
    if (now < expiration && 
        json_object_get(j_access_token, "type") != NULL &&
        json_is_string(json_object_get(j_access_token, "type")) &&
        0 == o_strcmp("access_token", json_string_value(json_object_get(j_access_token, "type"))) &&
        json_object_get(j_access_token, "username") != NULL &&
        json_is_string(json_object_get(j_access_token, "username")) &&
        json_string_length(json_object_get(j_access_token, "username")) > 0) {
      res = G_OK;
    } else {
      res = G_ERROR_INVALID_REQUEST;
    }
  } else {
    res = G_ERROR_INVALID_TOKEN;
  }
  return res;
}

/**
 * validates if the token value is a valid jwt and has a valid signature
 */
json_t * access_token_check_signature(struct _glewlwyd_resource_config * config, const char * token_value) {
  json_t * j_return, * j_grants;
  jwt_t * jwt = NULL;
  char  * grants;
  
  if (token_value != NULL) {
    if (!jwt_decode(&jwt, token_value, (const unsigned char *)config->jwt_decode_key, strlen(config->jwt_decode_key)) && jwt_get_alg(jwt) == config->jwt_alg) {
      grants = jwt_get_grants_json(jwt, NULL);
      j_grants = json_loads(grants, JSON_DECODE_ANY, NULL);
      if (j_grants != NULL) {
        j_return = json_pack("{siso}", "result", G_OK, "grants", j_grants);
      } else {
        j_return = json_pack("{si}", "result", G_ERROR);
      }
      o_free(grants);
    } else {
      j_return = json_pack("{si}", "result", G_ERROR_INVALID_TOKEN);
    }
    jwt_free(jwt);
  } else {
    j_return = json_pack("{si}", "result", G_ERROR_INVALID_TOKEN);
  }
  return j_return;
}

/**
 * Return the ayload of an access token
 */
json_t * access_token_get_payload(const char * token_value) {
  json_t * j_return, * j_grants;
  jwt_t * jwt = NULL;
  char  * grants;
  
  if (token_value != NULL) {
    if (!jwt_decode(&jwt, token_value, NULL, 0)) {
      grants = jwt_get_grants_json(jwt, NULL);
      j_grants = json_loads(grants, JSON_DECODE_ANY, NULL);
      if (j_grants != NULL) {
        j_return = json_pack("{siso}", "result", G_OK, "grants", j_grants);
      } else {
        j_return = json_pack("{si}", "result", G_ERROR);
      }
      o_free(grants);
    } else {
      j_return = json_pack("{si}", "result", G_ERROR_INVALID_TOKEN);
    }
    jwt_free(jwt);
  } else {
    j_return = json_pack("{si}", "result", G_ERROR_INVALID_TOKEN);
  }
  return j_return;
}
