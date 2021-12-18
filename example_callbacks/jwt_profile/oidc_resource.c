/**
 *
 * Glewlwyd SSO Access Token token check
 *
 * Copyright 2016-2020 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20201013
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

#include <jansson.h>
#include <orcania.h>
#include <string.h>
#include <time.h>
#include <ulfius.h>

#include "oidc_resource.h"

/**
 * Check if the result json object has a "result" element that is equal to value
 */
static int check_result_value(json_t * result, const int value) {
  return (result != NULL && 
          json_is_object(result) && 
          json_object_get(result, "result") != NULL && 
          json_is_integer(json_object_get(result, "result")) && 
          json_integer_value(json_object_get(result, "result")) == value);
}

/**
 * Validates if an access_token grants has a valid scope
 * return the final scope list on success
 */
static json_t * access_token_check_scope(struct _oidc_resource_config * config, json_t * j_access_token) {
  int i, scope_count_token, scope_count_expected;
  char ** scope_list_token, ** scope_list_expected;
  json_t * j_res = NULL, * j_scope_final_list = json_array();
  
  if (j_scope_final_list != NULL) {
    if (j_access_token != NULL) {
      scope_count_token = split_string(json_string_value(json_object_get(j_access_token, "scope")), " ", &scope_list_token);
      if (o_strlen(config->oauth_scope)) {
        scope_count_expected = split_string(config->oauth_scope, " ", &scope_list_expected);
        if (scope_count_token > 0 && scope_count_expected > 0) {
          for (i=0; scope_count_expected > 0 && scope_list_expected[i] != NULL; i++) {
            if (string_array_has_value((const char **)scope_list_token, scope_list_expected[i])) {
              json_array_append_new(j_scope_final_list, json_string(scope_list_expected[i]));
            }
          }
          if (json_array_size(j_scope_final_list) > 0) {
            j_res = json_pack("{sisO}", "result", G_TOKEN_OK, "scope", j_scope_final_list);
          } else {
            j_res = json_pack("{si}", "result", G_TOKEN_ERROR_INSUFFICIENT_SCOPE);
          }
        } else {
          j_res = json_pack("{si}", "result", G_TOKEN_ERROR_INTERNAL);
        }
        free_string_array(scope_list_expected);
      } else {
        j_res = json_pack("{sisO}", "result", G_TOKEN_OK, "scope", json_object_get(j_access_token, "scope"));
      }
      free_string_array(scope_list_token);
    } else {
      j_res = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
    }
  } else {
    j_res = json_pack("{si}", "result", G_TOKEN_ERROR_INTERNAL);
  }
  json_decref(j_scope_final_list);
  return j_res;
}

/**
 * Validates if an access_token grants has valid parameters:
 * - sub: non empty string
 * - aud: non empty string
 * - type: match "access_token" or "client_token"
 * - exp < now
 */
static int access_token_check_validity(struct _oidc_resource_config * config, json_t * j_access_token) {
  time_t now;
  json_int_t expiration;
  int res;
  
  if (j_access_token != NULL) {
    // Token is valid, check type and expiration date
    time(&now);
    expiration = json_integer_value(json_object_get(j_access_token, "exp"));
    if (now < expiration &&
        json_object_get(j_access_token, "type") != NULL &&
        json_is_string(json_object_get(j_access_token, "type"))) {
      if (config->accept_access_token &&
        0 == o_strcmp("access_token", json_string_value(json_object_get(j_access_token, "type"))) &&
        json_string_length(json_object_get(j_access_token, "sub")) > 0) {
        res = G_TOKEN_OK;
      } else if (config->accept_client_token &&
        0 == o_strcmp("client_token", json_string_value(json_object_get(j_access_token, "type"))) &&
        json_string_length(json_object_get(j_access_token, "aud")) > 0) {
        res = G_TOKEN_OK;
      } else {
        res = G_TOKEN_ERROR_INVALID_REQUEST;
      }
    } else {
      res = G_TOKEN_ERROR_INVALID_REQUEST;
    }
  } else {
    res = G_TOKEN_ERROR_INVALID_TOKEN;
  }
  return res;
}

/**
 * validates if the token value is a valid jwt and has a valid signature
 */
static json_t * access_token_check_signature(struct _oidc_resource_config * config, const char * token_value) {
  json_t * j_return = NULL, * j_grants;
  jwt_t * jwt = r_jwt_copy(config->jwt);
  jwk_t * jwk = NULL;
  const char * kid;
  
  if (token_value != NULL) {
    if (r_jwt_parse(jwt, token_value, 0) == RHN_OK) {
      if ((kid = r_jwt_get_header_str_value(jwt, "kid")) != NULL) {
        if ((jwk = r_jwks_get_by_kid(jwt->jwks_pubkey_sign, kid)) == NULL) {
          j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
        }
      } else {
        jwk = r_jwk_copy(config->jwk_verify_default);
      }
      if (j_return == NULL) {
        if (r_jwt_verify_signature(jwt, jwk, 0) == RHN_OK && r_jwt_get_sign_alg(jwt) == config->alg) {
          j_grants = r_jwt_get_full_claims_json_t(jwt);
          if (j_grants != NULL) {
            j_return = json_pack("{siso}", "result", G_TOKEN_OK, "grants", j_grants);
          } else {
            j_return = json_pack("{si}", "result", G_TOKEN_ERROR);
          }
        } else {
          j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
        }
      }
      r_jwk_free(jwk);
    } else {
      j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
    }
  } else {
    j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
  }
  r_jwt_free(jwt);
  return j_return;
}

/**
 * check if bearer token has some of the specified scope
 */
int callback_check_glewlwyd_oidc_access_token (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct _oidc_resource_config * config = (struct _oidc_resource_config *)user_data;
  json_t * j_access_token = NULL, * j_res_scope;
  int res = U_CALLBACK_UNAUTHORIZED, res_validity;
  const char * token_value = NULL;
  char * response_value = NULL;
  
  if (config != NULL) {
    switch (config->method) {
      case G_METHOD_HEADER:
        if (u_map_get_case(request->map_header, HEADER_AUTHORIZATION) != NULL) {
          if (o_strstr(u_map_get_case(request->map_header, HEADER_AUTHORIZATION), HEADER_PREFIX_BEARER) == u_map_get_case(request->map_header, HEADER_AUTHORIZATION)) {
            token_value = u_map_get_case(request->map_header, HEADER_AUTHORIZATION) + o_strlen(HEADER_PREFIX_BEARER);
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
      if (check_result_value(j_access_token, G_TOKEN_OK)) {
        res_validity = access_token_check_validity(config, json_object_get(j_access_token, "grants"));
        if (res_validity == G_TOKEN_OK) {
          j_res_scope = access_token_check_scope(config, json_object_get(j_access_token, "grants"));
          if (check_result_value(j_res_scope, G_TOKEN_ERROR_INSUFFICIENT_SCOPE)) {
            response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"insufficient_scope\",error_description=\"The scope is invalid\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
            u_map_put(response->map_header, HEADER_RESPONSE, response_value);
            o_free(response_value);
          } else if (!check_result_value(j_res_scope, G_TOKEN_OK)) {
            response_value = msprintf(HEADER_PREFIX_BEARER "%s%s%serror=\"invalid_request\",error_description=\"Internal server error\"", (config->realm!=NULL?"realm=":""), (config->realm!=NULL?config->realm:""), (config->realm!=NULL?",":""));
            u_map_put(response->map_header, HEADER_RESPONSE, response_value);
            o_free(response_value);
          } else {
            res = U_CALLBACK_CONTINUE;
            response->shared_data = (void*)json_pack("{sssOsO*}", "sub", json_string_value(json_object_get(json_object_get(j_access_token, "grants"), "sub")), "scope", json_object_get(j_res_scope, "scope"), "jkt", json_object_get(json_object_get(json_object_get(j_access_token, "grants"), "cnf"), "jkt"));
            if (json_object_get(json_object_get(j_access_token, "grants"), "aud") != NULL) {
              json_object_set((void*)response->shared_data, "aud", json_object_get(json_object_get(j_access_token, "grants"), "aud"));
            }
            if (json_object_get(json_object_get(j_access_token, "grants"), "client_id") != NULL) {
              json_object_set((void*)response->shared_data, "client_id", json_object_get(json_object_get(j_access_token, "grants"), "client_id"));
            }
            if (json_object_get(json_object_get(j_access_token, "grants"), "claims") != NULL) {
              json_object_set((void*)response->shared_data, "claims", json_object_get(json_object_get(j_access_token, "grants"), "claims"));
            }
            if (response->shared_data == NULL) {
              res = U_CALLBACK_ERROR;
            }
          }
          json_decref(j_res_scope);
        } else if (res_validity == G_TOKEN_ERROR_INVALID_TOKEN) {
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
 * Parse the DPoP header and extract its jkt value if the DPoP is valid
 */
json_t * verify_dpop_proof(const struct _u_request * request, const char * htm, const char * htu, time_t max_iat, const char * jkt) {
  json_t * j_return = NULL, * j_header = NULL, * j_claims = NULL;
  const char * dpop_header;
  jwt_t * dpop_jwt = NULL;
  jwa_alg alg;
  jwk_t * jwk_header = NULL;
  char * jkt_from_token = NULL;
  time_t now;
  
  if ((dpop_header = u_map_get_case(request->map_header, HEADER_DPOP)) != NULL) {
    if (r_jwt_init(&dpop_jwt) == RHN_OK) {
      if (r_jwt_parse(dpop_jwt, dpop_header, R_FLAG_IGNORE_REMOTE) == RHN_OK) {
        if (r_jwt_verify_signature(dpop_jwt, NULL, R_FLAG_IGNORE_REMOTE) == RHN_OK) {
          do {
            if (0 != o_strcmp("dpop+jwt", r_jwt_get_header_str_value(dpop_jwt, "typ"))) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid typ");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
            if ((alg = r_jwt_get_sign_alg(dpop_jwt)) != R_JWA_ALG_RS256 && alg != R_JWA_ALG_RS384 && alg != R_JWA_ALG_RS512 &&
                alg != R_JWA_ALG_ES256 && alg != R_JWA_ALG_ES384 && alg != R_JWA_ALG_ES512 && alg != R_JWA_ALG_EDDSA && alg != R_JWA_ALG_ES256K) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid sign_alg");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
            if ((j_header = r_jwt_get_full_header_json_t(dpop_jwt)) == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "verify_dpop_proof - Error r_jwt_get_full_header_json_t");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR);
              break;
            }
            if ((j_claims = r_jwt_get_full_claims_json_t(dpop_jwt)) == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "verify_dpop_proof - Error r_jwt_get_full_claims_json_t");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR);
              break;
            }
            if (json_object_get(j_header, "x5c") != NULL || json_object_get(j_header, "x5u") != NULL) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid header, x5c or x5u present");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
            if (r_jwk_init(&jwk_header) != RHN_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "verify_dpop_proof - Error r_jwk_init");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR);
              break;
            }
            if (r_jwk_import_from_json_t(jwk_header, json_object_get(j_header, "jwk")) != RHN_OK) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid jwk property in header");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
            if (!o_strlen(r_jwt_get_claim_str_value(dpop_jwt, "jti"))) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid jti");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
            if (0 != o_strcmp(htm, r_jwt_get_claim_str_value(dpop_jwt, "htm"))) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid htm");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
            if (0 != o_strcmp(htu, r_jwt_get_claim_str_value(dpop_jwt, "htu"))) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid htu");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
            time(&now);
            if ((time_t)r_jwt_get_claim_int_value(dpop_jwt, "iat") > now || ((time_t)r_jwt_get_claim_int_value(dpop_jwt, "iat"))+max_iat < now) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid iat");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
            if ((jkt_from_token = r_jwk_thumbprint(jwk_header, R_JWK_THUMB_SHA256, R_FLAG_IGNORE_REMOTE)) == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "verify_dpop_proof - Error r_jwk_thumbprint");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR);
              break;
            }
            if (0 != o_strcmp(jkt, jkt_from_token)) {
              y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - jkt value doesn't match");
              j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
              break;
            }
          } while (0);
          if (j_return == NULL) {
            j_return = json_pack("{sisO*sO*}", "result", G_TOKEN_OK, "header", j_header, "claims", j_claims);
          }
          json_decref(j_header);
          json_decref(j_claims);
          r_jwk_free(jwk_header);
          o_free(jkt_from_token);
        } else {
          y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid signature");
          j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_REQUEST);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_DEBUG, "verify_dpop_proof - Invalid DPoP token");
        j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "verify_dpop_proof - Error r_jwt_init");
      j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INTERNAL);
    }
    r_jwt_free(dpop_jwt);
  } else {
    j_return = json_pack("{si}", "result", G_TOKEN_ERROR_INVALID_TOKEN);
  }
  return j_return;
}
