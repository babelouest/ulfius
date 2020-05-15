/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * ulfius-cfg.h.in: configuration file
 * 
 * Copyright 2018-2020 Nicolas Mora <mail@babelouest.org>
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
#ifndef _ULFIUS_CFG_H_
#define _ULFIUS_CFG_H_

#define ULFIUS_VERSION 2.6.6
#define ULFIUS_VERSION_STR "2.6.6"

#define ULFIUS_VERSION_MAJOR 2
#define ULFIUS_VERSION_MINOR 6
#define ULFIUS_VERSION_PATCH 6

#define ULFIUS_VERSION_NUMBER ((ULFIUS_VERSION_MAJOR << 16) | (ULFIUS_VERSION_MINOR << 8) | (ULFIUS_VERSION_PATCH << 0))

/* #undef U_DISABLE_JANSSON */
/* #undef U_DISABLE_CURL */
/* #undef U_DISABLE_GNUTLS */
/* #undef U_DISABLE_WEBSOCKET */
/* #undef U_DISABLE_YDER */
/* #undef U_WITH_FREERTOS */
/* #undef U_WITH_LWIP */

#endif /* _ULFIUS_CFG_H_ */

