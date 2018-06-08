/**
 *
 * Glewlwyd OAuth2 Authorization token check
 *
 * Copyright 2016-2018 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20180607
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
#include <jwt.h>

#define G_OK                       0
#define G_ERROR                    1
#define G_ERROR_INTERNAL           2
#define G_ERROR_INVALID_REQUEST    3
#define G_ERROR_INVALID_TOKEN      4
#define G_ERROR_INSUFFICIENT_SCOPE 5

#define G_METHOD_HEADER 0
#define G_METHOD_BODY   1
#define G_METHOD_URL    2

#define HEADER_PREFIX_BEARER "Bearer "
#define HEADER_RESPONSE      "WWW-Authenticate"
#define HEADER_AUTHORIZATION "Authorization"
#define BODY_URL_PARAMETER   "access_token"

struct _glewlwyd_resource_config {
  int       method;
  char *    oauth_scope;
  char *    jwt_decode_key;
  jwt_alg_t jwt_alg;
  char *    realm;
};

int callback_check_glewlwyd_access_token (const struct _u_request * request, struct _u_response * response, void * user_data);
json_t * access_token_check_signature(struct _glewlwyd_resource_config * config, const char * token_value);
json_t * access_token_get_payload(const char * token_value);
int access_token_check_validity(struct _glewlwyd_resource_config * config, json_t * j_access_token);
json_t * access_token_check_scope(struct _glewlwyd_resource_config * config, json_t * j_access_token);
