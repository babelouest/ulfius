/* Public domain, no copyright. Use at your own risk. */

#include <check.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ulfius.h>

#define UNUSED(x) (void)(x)

#include "static_compressed_inmemory_website_callback.h"
#include "http_compression_callback.h"

// Source: https://commons.wikimedia.org/wiki/File:Tux.svg
// download as png, 32px
unsigned char binary_data[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x26, 0x08,
                               0x06, 0x00, 0x00, 0x00, 0xa5, 0x23, 0x99, 0xe9, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41, 0x4d, 0x41, 0x00, 0x00, 0xb1, 0x8f, 0x0b, 0xfc, 0x61, 0x05, 0x00,
                               0x00, 0x00, 0x20, 0x63, 0x48, 0x52, 0x4d, 0x00, 0x00, 0x7a, 0x26, 0x00, 0x00, 0x80, 0x84, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x80, 0xe8, 0x00, 0x00,
                               0x75, 0x30, 0x00, 0x00, 0xea, 0x60, 0x00, 0x00, 0x3a, 0x98, 0x00, 0x00, 0x17, 0x70, 0x9c, 0xba, 0x51, 0x3c, 0x00, 0x00, 0x00, 0x06, 0x62, 0x4b, 0x47,
                               0x44, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xa0, 0xbd, 0xa7, 0x93, 0x00, 0x00, 0x07, 0xba, 0x49, 0x44, 0x41, 0x54, 0x58, 0xc3, 0xb5, 0x57, 0x6b, 0x6c,
                               0x54, 0xc7, 0x15, 0xfe, 0xe6, 0x71, 0x1f, 0xbb, 0x7b, 0x77, 0x6d, 0xef, 0xb2, 0xb6, 0x81, 0x60, 0x8b, 0x86, 0x18, 0x83, 0xe4, 0x88, 0x40, 0x28, 0xc4,
                               0x35, 0x29, 0x4d, 0x30, 0x24, 0x41, 0xa0, 0x52, 0x29, 0xa2, 0x4d, 0xda, 0x82, 0x42, 0x8a, 0x42, 0x28, 0x6a, 0xa5, 0x0a, 0x21, 0xb5, 0x4d, 0x55, 0x35,
                               0x51, 0x11, 0xf9, 0xd1, 0x48, 0x7d, 0xa0, 0x54, 0x25, 0x69, 0x4b, 0x88, 0x50, 0x12, 0x29, 0x0a, 0x44, 0x2a, 0x69, 0x54, 0x42, 0x54, 0x85, 0x14, 0xd9,
                               0x58, 0x2e, 0xcf, 0x18, 0x88, 0x84, 0x5f, 0x18, 0x9b, 0xb5, 0xbd, 0xeb, 0xdd, 0xf5, 0xee, 0xde, 0xd7, 0x9c, 0xfe, 0x58, 0xdb, 0xd8, 0x25, 0x60, 0x9b,
                               0x38, 0x23, 0x1d, 0x69, 0x67, 0xee, 0xdc, 0x39, 0xdf, 0x9e, 0xf3, 0x9d, 0x6f, 0xce, 0x05, 0xa6, 0x3f, 0x74, 0xce, 0xe5, 0x1e, 0xce, 0xe5, 0x19, 0xce,
                               0x65, 0x8e, 0x73, 0xd9, 0xc3, 0x98, 0xfc, 0x97, 0x10, 0xda, 0x36, 0x00, 0x02, 0x5f, 0xf1, 0xb0, 0x84, 0x90, 0xa7, 0x38, 0x97, 0x34, 0xde, 0x22, 0x91,
                               0x52, 0x8a, 0x46, 0x63, 0x14, 0x0e, 0x97, 0x7e, 0x02, 0xa0, 0x74, 0x3a, 0x07, 0xf2, 0x69, 0x6d, 0xe6, 0x62, 0x1f, 0x11, 0x56, 0xdc, 0xba, 0xce, 0x21,
                               0x84, 0x80, 0x10, 0xfc, 0x1b, 0x8c, 0xc9, 0xc3, 0x00, 0xd8, 0x54, 0xcf, 0x9c, 0x4e, 0xc8, 0xc2, 0x8c, 0x89, 0x37, 0x01, 0x68, 0xff, 0xff, 0x40, 0x4a,
                               0x09, 0x80, 0xc1, 0xf7, 0x7d, 0xb8, 0xae, 0xbb, 0x80, 0x73, 0x76, 0x9e, 0x88, 0x2e, 0xce, 0x70, 0xf4, 0xe5, 0x6a, 0xce, 0x25, 0x09, 0xa1, 0x91, 0x94,
                               0xfa, 0x84, 0x14, 0x48, 0xa9, 0x93, 0xa6, 0x19, 0x63, 0x73, 0xc6, 0xe4, 0x3f, 0x66, 0x3c, 0x05, 0x42, 0x50, 0xa5, 0x10, 0x02, 0x6b, 0xd6, 0x3c, 0x02,
                               0xcb, 0xb2, 0xc6, 0xd6, 0xd7, 0xae, 0x6d, 0xc4, 0xbe, 0x7d, 0x7b, 0x51, 0x5f, 0xff, 0xd0, 0xd8, 0x1a, 0x63, 0xb8, 0x7f, 0xc6, 0xd9, 0x27, 0x84, 0x78,
                               0x6a, 0xeb, 0xd6, 0xad, 0x74, 0xe5, 0xca, 0x65, 0x3a, 0x70, 0xe0, 0x2f, 0xc4, 0xb9, 0xa4, 0x9a, 0x9a, 0x1a, 0x6a, 0x69, 0x39, 0x4d, 0xcd, 0xcd, 0xcd,
                               0x74, 0xf6, 0xec, 0x59, 0x0a, 0x04, 0x42, 0xa3, 0x51, 0xf0, 0xbe, 0x0a, 0x12, 0xaa, 0xea, 0xea, 0x6a, 0x2c, 0x5f, 0xbe, 0x12, 0x55, 0x55, 0xf3, 0x50,
                               0x59, 0x59, 0x81, 0x85, 0x0b, 0x6b, 0x71, 0xfa, 0x74, 0x0b, 0x56, 0xac, 0x78, 0x08, 0x97, 0x2f, 0x5f, 0xc2, 0xec, 0xd9, 0x95, 0xe3, 0xb9, 0x35, 0x25, 
                               0x7e, 0xc9, 0x69, 0x00, 0xf0, 0xdb, 0xdb, 0x3b, 0x10, 0x89, 0x44, 0xd0, 0x76, 0xea, 0xf7, 0x78, 0x71, 0x57, 0x06, 0x2e, 0x5d, 0x80, 0xa5, 0xd5, 0x02, 
                               0x00, 0x72, 0xb9, 0x1c, 0x0a, 0x05, 0x7b, 0xfc, 0x7e, 0x03, 0x40, 0x6e, 0xc6, 0x00, 0x30, 0xc6, 0x32, 0xc7, 0x8f, 0x1f, 0xc7, 0x3b, 0x2f, 0x3f, 0x88, 
                               0x9a, 0x59, 0x1f, 0x02, 0x51, 0x0b, 0x86, 0x15, 0x07, 0xc7, 0x5b, 0xe8, 0x38, 0x59, 0x87, 0xb3, 0x17, 0x0f, 0x23, 0x71, 0xa3, 0x6f, 0xfc, 0x2b, 0x25, 
                               0x53, 0x01, 0x30, 0xe5, 0x14, 0x78, 0x1e, 0xd2, 0x5f, 0xbf, 0xa7, 0x0f, 0x51, 0xe7, 0xd8, 0x08, 0x22, 0x01, 0xc6, 0x8a, 0xe5, 0x2e, 0xbd, 0x6e, 0xcc, 
                               0x13, 0xcd, 0xa8, 0xae, 0x00, 0x4c, 0xd3, 0x84, 0x69, 0x9a, 0x90, 0x52, 0xd6, 0xcd, 0x30, 0x07, 0x78, 0x66, 0x30, 0x03, 0x5c, 0x4f, 0x14, 0x9d, 0x92, 
                               0x5f, 0x40, 0x7e, 0xd8, 0xc6, 0xb9, 0xe6, 0x2c, 0xfe, 0xfb, 0xef, 0x2c, 0x92, 0x99, 0x08, 0x7a, 0x93, 0x12, 0xa6, 0x69, 0x20, 0x18, 0x0c, 0x40, 0x29, 
                               0xac, 0x9d, 0x61, 0x00, 0xcc, 0x69, 0xed, 0x0e, 0x62, 0x30, 0xfe, 0x2b, 0xb4, 0x34, 0x29, 0x0c, 0x5e, 0xcc, 0x23, 0xd1, 0x72, 0x11, 0x96, 0xad, 0x40, 
                               0xf7, 0x6c, 0x87, 0xbe, 0xec, 0x10, 0x1e, 0x5f, 0xbf, 0x11, 0x8c, 0x31, 0x08, 0x21, 0x00, 0x60, 0x0b, 0x80, 0xf0, 0xa4, 0xa7, 0x4e, 0x5d, 0x86, 0xb5, 
                               0x9d, 0x75, 0x75, 0x8b, 0xff, 0xf8, 0xf6, 0xdb, 0x6f, 0xa1, 0xfb, 0xb3, 0x8f, 0xd1, 0xff, 0xf9, 0x47, 0x30, 0xac, 0x0a, 0xcc, 0xa9, 0xfb, 0x36, 0xca, 
                               0xca, 0xe7, 0xc1, 0xb2, 0x42, 0x68, 0x6f, 0xef, 0xc4, 0xaa, 0x55, 0xdf, 0x84, 0x52, 0x6a, 0x54, 0x0f, 0xde, 0xf7, 0x7d, 0x6f, 0xe3, 0x4c, 0xa8, 0x60, 
                               0x3d, 0xe7, 0x32, 0x79, 0xf8, 0xf0, 0x9b, 0xd4, 0xd3, 0xd3, 0x4d, 0x97, 0x2e, 0xb5, 0xd1, 0xf9, 0xf3, 0xe7, 0xa8, 0xad, 0xed, 0x33, 0xea, 0xec, 0x6c, 
                               0xa7, 0xfe, 0xfe, 0x1b, 0x94, 0xcd, 0xa6, 0xc9, 0xb6, 0xf3, 0xb4, 0x7e, 0xfd, 0x86, 0x09, 0x2a, 0x09, 0xc8, 0x35, 0x5f, 0xd6, 0x7b, 0x25, 0xe7, 0x22, 
                               0xbb, 0x6e, 0xdd, 0x13, 0x54, 0x28, 0xe4, 0x28, 0x9d, 0x4e, 0x51, 0x22, 0xd1, 0x47, 0xd7, 0xaf, 0x5f, 0xa3, 0x44, 0xa2, 0x8f, 0xd2, 0xe9, 0x14, 0xe5, 
                               0xf3, 0xc3, 0x64, 0xdb, 0x79, 0x72, 0x9c, 0x02, 0xbd, 0xf7, 0xde, 0xbb, 0x34, 0xf1, 0xb6, 0x14, 0x43, 0x00, 0xca, 0xef, 0xd6, 0xb9, 0x26, 0xa5, 0x76, 
                               0x91, 0x73, 0x49, 0x4d, 0x4d, 0x4d, 0xe4, 0x38, 0x05, 0x72, 0x5d, 0x9b, 0x3c, 0xcf, 0x21, 0xd7, 0xb5, 0xc9, 0xf7, 0x5d, 0x72, 0x5d, 0x9b, 0x6c, 0x3b, 
                               0x4f, 0xb9, 0x5c, 0x96, 0x32, 0x99, 0x21, 0xea, 0xef, 0xbf, 0x41, 0xba, 0x6e, 0x92, 0x94, 0x3a, 0x49, 0xa9, 0x8f, 0xdc, 0x1d, 0x5a, 0x33, 0x80, 0xc0, 
                               0xb4, 0x49, 0x68, 0x9a, 0xc1, 0x3d, 0xba, 0x6e, 0x2c, 0x92, 0x52, 0xa2, 0xb6, 0xb6, 0x06, 0x4a, 0x29, 0x10, 0x11, 0x88, 0xa8, 0x58, 0x09, 0x23, 0xbf, 
                               0x95, 0x52, 0xf0, 0x3c, 0x0f, 0x8e, 0xe3, 0x80, 0x31, 0x86, 0xc6, 0xc6, 0x46, 0x70, 0xce, 0xa1, 0xeb, 0x3a, 0x74, 0x5d, 0x07, 0xe7, 0xe2, 0x41, 0xce, 
                               0xe5, 0x6f, 0xa6, 0x4b, 0x42, 0x33, 0x16, 0x9b, 0xd5, 0x4f, 0x44, 0xe6, 0x92, 0x25, 0x0f, 0x88, 0xa3, 0x47, 0xdf, 0x85, 0x94, 0x12, 0x52, 0xca, 0xb1, 
                               0xfa, 0x07, 0x00, 0xa5, 0x14, 0x7c, 0xdf, 0x87, 0xe7, 0x79, 0x70, 0x5d, 0x17, 0x4a, 0x29, 0xa4, 0xd3, 0x69, 0x34, 0x34, 0xac, 0x46, 0x2a, 0x95, 0x82, 
                               0x61, 0x18, 0x70, 0x1c, 0x07, 0xb9, 0x5c, 0x2e, 0xaf, 0x94, 0x57, 0x03, 0xa0, 0x7b, 0x4a, 0xfd, 0x40, 0x2c, 0x16, 0xdb, 0x50, 0x56, 0x16, 0xdb, 0x3c, 
                               0x30, 0x30, 0xa0, 0xbd, 0xf1, 0xc6, 0x41, 0xcc, 0x99, 0x33, 0x1b, 0x42, 0x14, 0xc5, 0x67, 0x3c, 0x80, 0xd1, 0x39, 0xe7, 0x7c, 0xcc, 0x2c, 0xcb, 0x82, 
                               0x65, 0x59, 0xf8, 0xe0, 0x83, 0x7f, 0x82, 0x73, 0x06, 0xc3, 0x30, 0x90, 0xcf, 0x17, 0x34, 0xc6, 0x98, 0x4e, 0x44, 0xc7, 0xa6, 0x94, 0xfc, 0xb9, 0x73, 
                               0xab, 0xde, 0xaf, 0xae, 0x9e, 0xdf, 0xf5, 0xf4, 0xd3, 0x3f, 0x20, 0xdf, 0x77, 0xc7, 0xcc, 0xf3, 0x9c, 0x09, 0xf3, 0xdb, 0x59, 0x6f, 0x6f, 0x0f, 0x95, 
                               0x94, 0x94, 0x52, 0x20, 0x10, 0xa2, 0x68, 0x34, 0x36, 0x4a, 0xca, 0x41, 0x00, 0xfa, 0xa4, 0x1c, 0x08, 0x87, 0xc3, 0xb3, 0xc2, 0xe1, 0xf0, 0xa3, 0xbd, 
                               0xbd, 0xbd, 0x73, 0x9e, 0x79, 0x66, 0xeb, 0x5d, 0xb1, 0x37, 0x1e, 0x8f, 0x63, 0xd1, 0xa2, 0x5a, 0xd8, 0xb6, 0x0d, 0xc7, 0x71, 0x47, 0x97, 0xcb, 0x00, 
                               0x59, 0x3f, 0x29, 0x00, 0xc3, 0x08, 0xec, 0xe1, 0x1c, 0xb9, 0x58, 0x2c, 0xc6, 0x1f, 0x7e, 0xb8, 0x61, 0xc2, 0xb3, 0x51, 0x02, 0x4e, 0x65, 0xdc, 0x77, 
                               0xdf, 0xc2, 0x11, 0x9e, 0xd0, 0x38, 0x41, 0xc3, 0xaa, 0xc9, 0x6e, 0xc3, 0xa8, 0xa6, 0xc9, 0x1d, 0x9e, 0xe7, 0x1b, 0x4b, 0x97, 0x2e, 0x05, 0xe7, 0xfc, 
                               0xae, 0x05, 0xc4, 0xb2, 0x82, 0x63, 0x4d, 0xeb, 0x38, 0xde, 0xd7, 0xdd, 0x11, 0x00, 0xe7, 0x72, 0x1b, 0x63, 0x2c, 0xc4, 0x18, 0xc3, 0xe2, 0xc5, 0xb5, 
                               0xb7, 0x1c, 0x3a, 0x9d, 0x08, 0xdc, 0x6c, 0x5a, 0xc5, 0xf8, 0xf7, 0x63, 0x93, 0xa5, 0xa0, 0xdc, 0xb6, 0x6d, 0x70, 0xce, 0xe1, 0xfb, 0xfe, 0x97, 0x92, 
                               0xd0, 0x81, 0x81, 0x24, 0x34, 0xad, 0x58, 0xb6, 0x9a, 0xa6, 0x8d, 0x56, 0x4f, 0xf2, 0x8e, 0x11, 0xf8, 0xc3, 0x0e, 0x39, 0xeb, 0x5b, 0xcb, 0x86, 0x71, 
                               0xe9, 0x5a, 0x07, 0xf6, 0x1e, 0xfd, 0x08, 0xa9, 0x54, 0x0a, 0xa5, 0xa5, 0xa5, 0x13, 0xca, 0xee, 0x76, 0x63, 0x34, 0x3a, 0x76, 0x36, 0x81, 0xee, 0xd6, 
                               0xfd, 0xd0, 0x73, 0xcd, 0x90, 0x52, 0x83, 0x52, 0x3e, 0xc2, 0xe1, 0x30, 0x94, 0x52, 0x48, 0xa5, 0x52, 0xad, 0xe3, 0x83, 0x38, 0x21, 0x02, 0x2d, 0xfb, 
                               0x83, 0xcb, 0x1e, 0xab, 0x67, 0x4f, 0x95, 0x96, 0x30, 0x2c, 0xaf, 0xf5, 0xf1, 0xec, 0xaa, 0xcb, 0x68, 0x6c, 0x5c, 0x87, 0xf6, 0xf6, 0xf6, 0x29, 0x01, 
                               0x50, 0x4a, 0xc1, 0x75, 0x5d, 0xb4, 0x7d, 0xf2, 0x27, 0x0c, 0x5d, 0x78, 0x15, 0x15, 0xc1, 0x04, 0xf2, 0xf9, 0x3c, 0x32, 0x99, 0x2c, 0x06, 0x07, 0x07, 
                               0x11, 0xe0, 0x69, 0x9c, 0x7c, 0x25, 0xbc, 0xe3, 0xf4, 0xf6, 0x9b, 0xdf, 0x16, 0xfc, 0xc2, 0x01, 0x63, 0xc1, 0x99, 0x57, 0xcd, 0xcf, 0xdb, 0xfe, 0x66, 
                               0x0e, 0xc5, 0xa3, 0x68, 0x0e, 0x87, 0x98, 0x6e, 0x9a, 0x0c, 0x81, 0x00, 0xc7, 0x63, 0x2b, 0x09, 0x65, 0xec, 0x3c, 0x1a, 0x1a, 0x56, 0xe3, 0xd4, 0xa9, 
                               0xa6, 0xc9, 0x9b, 0x46, 0xdf, 0x87, 0x6d, 0xdb, 0xb0, 0xed, 0x0c, 0xa4, 0xe4, 0x58, 0x5c, 0x25, 0x20, 0x46, 0xfe, 0xe2, 0xbc, 0x38, 0xc7, 0x81, 0xdd, 
                               0x25, 0xa8, 0x2c, 0x37, 0xe6, 0xe6, 0xbe, 0x16, 0x7d, 0x64, 0x0c, 0x80, 0xe7, 0xf0, 0x9d, 0xe1, 0x08, 0xbb, 0xb7, 0x24, 0xc2, 0x23, 0x21, 0x8b, 0x31, 
                               0xdd, 0x64, 0x90, 0x1a, 0xa0, 0x69, 0x80, 0x19, 0x60, 0xd8, 0xbb, 0x5d, 0x62, 0x28, 0xd9, 0x87, 0xc6, 0xc6, 0x75, 0x38, 0x72, 0xe4, 0xc8, 0x1d, 0x23, 
                               0xe0, 0xfb, 0x3e, 0x0a, 0x85, 0x02, 0x5c, 0x97, 0x20, 0x39, 0x43, 0x7d, 0x9d, 0x8e, 0xa3, 0x2f, 0x95, 0xe1, 0xb5, 0xdd, 0x25, 0x38, 0xb6, 0x2f, 0x86, 
                               0xba, 0x05, 0x06, 0x02, 0x41, 0x81, 0x80, 0x45, 0xcf, 0x8f, 0x2b, 0x4b, 0x78, 0x86, 0x06, 0xe8, 0x3a, 0x83, 0xae, 0x03, 0x52, 0x32, 0x70, 0xce, 0xc0, 
                               0x05, 0x83, 0xae, 0x73, 0xcc, 0x8d, 0x03, 0x2f, 0x7c, 0x5f, 0x43, 0xa1, 0x50, 0xc0, 0x96, 0x2d, 0xdb, 0x90, 0xcd, 0x66, 0xef, 0x98, 0x02, 0xdf, 0xf7, 
                               0x71, 0xa5, 0xad, 0x0d, 0x52, 0x63, 0xb0, 0x42, 0x02, 0x75, 0x35, 0x06, 0x1a, 0x57, 0x06, 0x11, 0x8b, 0x69, 0x08, 0x85, 0x38, 0x34, 0x8d, 0xa3, 0xbc, 
                               0xdc, 0xdc, 0x70, 0xf2, 0x95, 0x92, 0x97, 0x00, 0x40, 0xba, 0x90, 0xfd, 0x9c, 0xfb, 0xe0, 0x1c, 0x60, 0x9c, 0x81, 0x71, 0x00, 0x23, 0x24, 0x91, 0x5c, 
                               0x41, 0xd7, 0x19, 0xbe, 0xb3, 0x1a, 0xf8, 0xf0, 0x7c, 0x25, 0x7a, 0x87, 0xc2, 0xd0, 0x34, 0xed, 0x4e, 0x9d, 0x33, 0x4e, 0x9c, 0xf8, 0x18, 0xe7, 0xce, 
                               0x9c, 0x42, 0xc3, 0xfc, 0x10, 0x02, 0x41, 0x0e, 0xc3, 0x94, 0xe0, 0x8c, 0x81, 0x40, 0x20, 0x45, 0xf0, 0x5c, 0x40, 0x4a, 0xc6, 0xe2, 0x71, 0xe3, 0x17, 
                               0x9f, 0xfe, 0x2e, 0x76, 0x3f, 0x57, 0xca, 0x8b, 0x13, 0x8a, 0x2d, 0x14, 0xe1, 0xa6, 0x73, 0xb0, 0x22, 0x20, 0x4d, 0x02, 0xa1, 0x00, 0xe1, 0x85, 0xef, 
                               0xe5, 0xd1, 0xda, 0xf2, 0x1f, 0x18, 0x86, 0x71, 0x5b, 0x00, 0x07, 0x0f, 0x1e, 0xc2, 0xb6, 0x6d, 0x3f, 0x42, 0xd4, 0x02, 0x88, 0x8a, 0x06, 0x55, 0xbc, 
                               0xae, 0x7d, 0x8f, 0xe0, 0x38, 0x0a, 0x76, 0xc1, 0x87, 0x6d, 0x2b, 0x38, 0x36, 0x01, 0x9c, 0x1e, 0xe5, 0x0a, 0x6a, 0xbd, 0xe3, 0x32, 0x78, 0x2e, 0xc1, 
                               0x77, 0x01, 0x5f, 0x15, 0x91, 0x92, 0x3f, 0xfa, 0x49, 0x56, 0x4c, 0x4f, 0x2c, 0x38, 0x8c, 0x5f, 0x3e, 0xbb, 0x04, 0xad, 0xad, 0x67, 0x6e, 0x71, 0xdc, 
                               0xd4, 0x74, 0x1a, 0x9b, 0x36, 0x3d, 0x89, 0x5d, 0xbb, 0x7e, 0x02, 0xd7, 0xf5, 0x50, 0x6a, 0x31, 0x78, 0x9e, 0x82, 0xef, 0x12, 0x5c, 0x57, 0xc1, 0xb5, 
                               0x7d, 0xe4, 0x73, 0x1e, 0xb2, 0x69, 0x0f, 0xc9, 0xa4, 0x8b, 0x81, 0x01, 0x07, 0x37, 0x12, 0x5e, 0xba, 0xbb, 0xc7, 0xd9, 0x22, 0x3d, 0x17, 0xf7, 0xa6, 
                               0x33, 0x0a, 0xbe, 0xc7, 0x60, 0x06, 0x08, 0x52, 0x30, 0x08, 0x51, 0x74, 0xcc, 0x39, 0x81, 0xc0, 0xa0, 0xfc, 0x62, 0x60, 0x2a, 0xcc, 0x6b, 0x68, 0xa8, 
                               0x5f, 0x89, 0xca, 0x39, 0x55, 0x98, 0x3f, 0xbf, 0x1a, 0x44, 0x0c, 0x57, 0xaf, 0x5e, 0x45, 0x57, 0x57, 0xd7, 0xc4, 0x36, 0x8a, 0x13, 0xb2, 0x39, 0x1f, 
                               0xc1, 0x61, 0x0f, 0xae, 0xc3, 0xe0, 0x2b, 0x20, 0x37, 0xec, 0x61, 0x28, 0xab, 0x90, 0x4a, 0x7b, 0x85, 0xfe, 0x94, 0x3a, 0x98, 0xce, 0x67, 0x7f, 0xb6, 
                               0x73, 0x3f, 0xb2, 0xb2, 0x2b, 0xa1, 0x9e, 0x0b, 0xa6, 0xf1, 0xd3, 0x48, 0x90, 0x2f, 0x0e, 0x06, 0x49, 0x18, 0x3a, 0x60, 0x18, 0x0c, 0xfa, 0x08, 0x31, 
                               0x19, 0x08, 0xf9, 0x02, 0x90, 0x1d, 0x26, 0x78, 0x3e, 0x60, 0x48, 0xe6, 0x75, 0x76, 0x76, 0x74, 0x74, 0x75, 0x75, 0xa6, 0x88, 0xc0, 0x00, 0x16, 0x07, 
                               0x10, 0x07, 0x60, 0x8e, 0x74, 0xc2, 0xc8, 0x3b, 0x0a, 0x03, 0x49, 0x1f, 0x02, 0x04, 0xa9, 0x71, 0xd8, 0xb6, 0xc2, 0x50, 0x56, 0xa1, 0x27, 0xa1, 0x3e, 
                               0xcd, 0x25, 0x33, 0x1b, 0x7f, 0x7c, 0x10, 0x03, 0x63, 0x4a, 0xb8, 0xf9, 0xb7, 0xee, 0xeb, 0x00, 0x5e, 0x3f, 0xb4, 0x0b, 0x91, 0x61, 0x69, 0x6e, 0x0a, 
                               0xea, 0x78, 0x22, 0x12, 0xa4, 0x55, 0xd1, 0x08, 0x66, 0x07, 0x4d, 0x05, 0xc1, 0x19, 0x0a, 0x0e, 0xd0, 0x37, 0x40, 0x48, 0x0c, 0xd1, 0x8d, 0xa1, 0x9c, 
                               0x37, 0x0f, 0x80, 0xf3, 0x05, 0x14, 0xa8, 0x90, 0x52, 0x3e, 0xa0, 0x14, 0x56, 0x74, 0xf6, 0xd2, 0xcf, 0x2d, 0xcd, 0xd5, 0x6d, 0x9b, 0x43, 0x0a, 0x86, 
                               0x9c, 0x0d, 0x74, 0x27, 0xdc, 0x3f, 0xef, 0xfe, 0x6b, 0xe1, 0xb9, 0x29, 0xb7, 0x64, 0xbf, 0xfe, 0x21, 0xaa, 0x0c, 0x2e, 0x9f, 0x2f, 0x09, 0xf0, 0xcd, 
                               0x82, 0xab, 0xb9, 0x3d, 0x49, 0xd6, 0xd1, 0x7e, 0x1d, 0xdf, 0xfd, 0xfb, 0x09, 0xb7, 0x65, 0x32, 0x41, 0x7a, 0xb2, 0x41, 0x36, 0x54, 0x46, 0xc4, 0xfe, 
                               0x59, 0x16, 0x2d, 0x02, 0x63, 0xfd, 0xc3, 0x05, 0xbc, 0xf8, 0xf2, 0x11, 0x7b, 0xff, 0x17, 0xed, 0xfd, 0x1f, 0x43, 0xda, 0x93, 0x3f, 0xaa, 0x87, 0x44, 
                               0xe4, 0x00, 0x00, 0x00, 0x25, 0x74, 0x45, 0x58, 0x74, 0x64, 0x61, 0x74, 0x65, 0x3a, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x00, 0x32, 0x30, 0x32, 0x32, 
                               0x2d, 0x30, 0x33, 0x2d, 0x32, 0x30, 0x54, 0x32, 0x30, 0x3a, 0x32, 0x39, 0x3a, 0x33, 0x36, 0x2b, 0x30, 0x30, 0x3a, 0x30, 0x30, 0xfd, 0xee, 0xdd, 0x63, 
                               0x00, 0x00, 0x00, 0x25, 0x74, 0x45, 0x58, 0x74, 0x64, 0x61, 0x74, 0x65, 0x3a, 0x6d, 0x6f, 0x64, 0x69, 0x66, 0x79, 0x00, 0x32, 0x30, 0x32, 0x32, 0x2d, 
                               0x30, 0x33, 0x2d, 0x32, 0x30, 0x54, 0x32, 0x30, 0x3a, 0x32, 0x39, 0x3a, 0x33, 0x36, 0x2b, 0x30, 0x30, 0x3a, 0x30, 0x30, 0x8c, 0xb3, 0x65, 0xdf, 0x00, 
                               0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

#define VALUE_JSON_NEWLINES "[\n\
	{\n\
		\"random\": \"100\",\n\
		\"random float\": \"19.768\",\n\
		\"bool\": \"true\",\n\
		\"date\": \"1987-01-31\",\n\
		\"regEx\": \"hellooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo to you\",\n\
		\"enum\": \"json\",\n\
		\"firstname\": \"Juliane\",\n\
		\"lastname\": \"Sibyls\",\n\
		\"city\": \"Miri\",\n\
		\"country\": \"Greece\",\n\
		\"countryCode\": \"AR\",\n\
		\"email uses current data\": \"Juliane.Sibyls@mail.tld\",\n\
		\"email from expression\": \"Juliane.Sibyls@expmail.tld\",\n\
		\"array\": [\n\
			\"Ellette\",\n\
			\"Dorthy\",\n\
			\"Ivett\",\n\
			\"Deane\",\n\
			\"Berget\"\n\
		],\n\
		\"array of objects\": [\n\
			{\n\
				\"index\": \"0\",\n\
				\"index start at 5\": \"5\"\n\
			},\n\
			{\n\
				\"index\": \"1\",\n\
				\"index start at 5\": \"6\"\n\
			},\n\
			{\n\
				\"index\": \"2\",\n\
				\"index start at 5\": \"7\"\n\
			}\n\
		],\n\
		\"Adele\": {\n\
			\"age\": \"89\"\n\
		}\n\
	},\n\
	{\n\
		\"random\": \"93\",\n\
		\"random float\": \"25.543\",\n\
		\"bool\": \"true\",\n\
		\"date\": \"1993-06-29\",\n\
		\"regEx\": \"hellooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo to you\",\n\
		\"enum\": \"generator\",\n\
		\"firstname\": \"Fidelia\",\n\
		\"lastname\": \"Rolf\",\n\
		\"city\": \"Alexandria\",\n\
		\"country\": \"Venezuela\",\n\
		\"countryCode\": \"WF\",\n\
		\"email uses current data\": \"Fidelia.Rolf@mail.tld\",\n\
		\"email from expression\": \"Fidelia.Rolf@expmail.tld\",\n\
		\"array\": [\n\
			\"Chrystel\",\n\
			\"Iseabal\",\n\
			\"Belinda\",\n\
			\"Lelah\",\n\
			\"Alleen\"\n\
		],\n\
		\"array of objects\": [\n\
			{\n\
				\"index\": \"0\",\n\
				\"index start at 5\": \"5\"\n\
			},\n\
			{\n\
				\"index\": \"1\",\n\
				\"index start at 5\": \"6\"\n\
			},\n\
			{\n\
				\"index\": \"2\",\n\
				\"index start at 5\": \"7\"\n\
			}\n\
		],\n\
		\"Kara-Lynn\": {\n\
			\"age\": \"49\"\n\
		}\n\
	}\n\
]"

static int callback_function_empty(const struct _u_request * request, struct _u_response * response, void * user_data) {
  UNUSED(request);
  UNUSED(response);
  UNUSED(user_data);
  return U_CALLBACK_CONTINUE;
}

int callback_function_text_data(const struct _u_request * request, struct _u_response * response, void * user_data) {
  UNUSED(request);
  UNUSED(user_data);
  ulfius_set_string_body_response(response, 200, "Hello World!");
  return U_CALLBACK_CONTINUE;
}

int callback_function_large_text_data(const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t * j_res;
  UNUSED(request);
  UNUSED(user_data);
  ck_assert_ptr_ne(NULL, j_res = json_loads(VALUE_JSON_NEWLINES, JSON_DECODE_ANY, NULL));
  ulfius_set_json_body_response(response, 200, j_res);
  json_decref(j_res);
  return U_CALLBACK_CONTINUE;
}

int callback_function_binary_data(const struct _u_request * request, struct _u_response * response, void * user_data) {
  UNUSED(request);
  UNUSED(user_data);
  ulfius_set_binary_body_response(response, 200, (const char *)binary_data, sizeof(binary_data));
  return U_CALLBACK_CONTINUE;
}

START_TEST(test_ulfius_compress_allow_all_accept_all)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_all_accept_gzip)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_all_accept_deflate)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_all_accept_none)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_all_accept_unknown)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_gzip_accept_all)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_gzip_accept_gzip)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("gzip", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_gzip_accept_deflate)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_gzip_accept_none)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_gzip_accept_unknown)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 1;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_deflate_accept_all)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_deflate_accept_gzip)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_deflate_accept_deflate)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_str_eq("deflate", u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_deflate_accept_none)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_deflate_accept_unknown)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 1;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_none_accept_all)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_none_accept_gzip)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_none_accept_deflate)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_none_accept_none)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_compress_allow_none_accept_unknown)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _http_compression_config http_compression_config;

  http_compression_config.allow_gzip = 0;
  http_compression_config.allow_deflate = 0;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "text", NULL, 0, &callback_function_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "large_text", NULL, 0, &callback_function_large_text_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "binary", NULL, 0, &callback_function_binary_data, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "*", NULL, 1, &callback_http_compression, &http_compression_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/empty",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/large_text",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/binary",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "POST",
                                                           U_OPT_HTTP_URL, "http://localhost:8080/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "unknown",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_gt(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_static_compressed_files_compress_accept_all)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _u_compressed_inmemory_website_config u_compressed_inmemory_website_config;

  ck_assert_int_eq(u_init_compressed_inmemory_website_config(&u_compressed_inmemory_website_config), U_OK);
  u_compressed_inmemory_website_config.files_path = realpath("./static", NULL);
  u_compressed_inmemory_website_config.redirect_on_404 = "/404.html";
  u_map_put(&u_compressed_inmemory_website_config.mime_types, "*", "application/octet-stream");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".html", "text/html");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".css", "text/css");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".png", "text/png");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/html");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/css");

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8081, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", NULL, "*", 1, &callback_static_compressed_inmemory_website, &u_compressed_inmemory_website_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html?param=value",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html#param",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html?param=value#param",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/sheep.png",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/styles.css",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081//../example_callbacks.c",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/../example_callbacks.c",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/\\\\error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  free(u_compressed_inmemory_website_config.files_path);
  u_clean_compressed_inmemory_website_config(&u_compressed_inmemory_website_config);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_static_compressed_files_compress_accept_gzip)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _u_compressed_inmemory_website_config u_compressed_inmemory_website_config;

  ck_assert_int_eq(u_init_compressed_inmemory_website_config(&u_compressed_inmemory_website_config), U_OK);
  u_compressed_inmemory_website_config.files_path = realpath("./static", NULL);
  u_compressed_inmemory_website_config.redirect_on_404 = "/404.html";
  u_compressed_inmemory_website_config.allow_deflate = 0;
  u_map_put(&u_compressed_inmemory_website_config.mime_types, "*", "application/octet-stream");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".html", "text/html");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".css", "text/css");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".png", "text/png");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/html");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/css");

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8081, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", NULL, "*", 1, &callback_static_compressed_inmemory_website, &u_compressed_inmemory_website_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/sheep.png",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/styles.css",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  free(u_compressed_inmemory_website_config.files_path);
  u_clean_compressed_inmemory_website_config(&u_compressed_inmemory_website_config);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_static_compressed_files_compress_accept_deflate)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _u_compressed_inmemory_website_config u_compressed_inmemory_website_config;

  ck_assert_int_eq(u_init_compressed_inmemory_website_config(&u_compressed_inmemory_website_config), U_OK);
  u_compressed_inmemory_website_config.files_path = realpath("./static", NULL);
  u_compressed_inmemory_website_config.redirect_on_404 = "/404.html";
  u_compressed_inmemory_website_config.allow_gzip = 0;
  u_map_put(&u_compressed_inmemory_website_config.mime_types, "*", "application/octet-stream");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".html", "text/html");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".css", "text/css");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".png", "text/png");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/html");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/css");

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8081, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", NULL, "*", 1, &callback_static_compressed_inmemory_website, &u_compressed_inmemory_website_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/sheep.png",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/styles.css",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  free(u_compressed_inmemory_website_config.files_path);
  u_clean_compressed_inmemory_website_config(&u_compressed_inmemory_website_config);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_static_compressed_files_compress_accept_none)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _u_compressed_inmemory_website_config u_compressed_inmemory_website_config;

  ck_assert_int_eq(u_init_compressed_inmemory_website_config(&u_compressed_inmemory_website_config), U_OK);
  u_compressed_inmemory_website_config.files_path = realpath("./static", NULL);
  u_compressed_inmemory_website_config.redirect_on_404 = "/404.html";
  u_compressed_inmemory_website_config.allow_gzip = 0;
  u_compressed_inmemory_website_config.allow_deflate = 0;
  u_map_put(&u_compressed_inmemory_website_config.mime_types, "*", "application/octet-stream");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".html", "text/html");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".css", "text/css");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".png", "text/png");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/html");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/css");

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8081, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", NULL, "*", 1, &callback_static_compressed_inmemory_website, &u_compressed_inmemory_website_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/sheep.png",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/styles.css",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  free(u_compressed_inmemory_website_config.files_path);
  u_clean_compressed_inmemory_website_config(&u_compressed_inmemory_website_config);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_static_compressed_files_compress_no_cache)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct _u_compressed_inmemory_website_config u_compressed_inmemory_website_config;

  ck_assert_int_eq(u_init_compressed_inmemory_website_config(&u_compressed_inmemory_website_config), U_OK);
  u_compressed_inmemory_website_config.files_path = realpath("./static", NULL);
  u_compressed_inmemory_website_config.redirect_on_404 = "/404.html";
  u_compressed_inmemory_website_config.allow_cache_compressed = 0;
  u_map_put(&u_compressed_inmemory_website_config.mime_types, "*", "application/octet-stream");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".html", "text/html");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".css", "text/css");
  u_map_put(&u_compressed_inmemory_website_config.mime_types, ".png", "text/png");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/html");
  u_add_mime_types_compressed(&u_compressed_inmemory_website_config, "text/css");

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8081, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", NULL, "*", 1, &callback_static_compressed_inmemory_website, &u_compressed_inmemory_website_config), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/sheep.png",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/styles.css",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/error",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_ptr_eq(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ck_assert_int_eq(response.binary_body_length, 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  ulfius_init_request(&request);
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HTTP_VERB, "GET",
                                                           U_OPT_HTTP_URL, "http://localhost:8081/index.html",
                                                           U_OPT_HEADER_PARAMETER, "Accept-Encoding", "gzip,deflate",
                                                           U_OPT_NONE), U_OK);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_gt(response.binary_body_length, 0);
  ck_assert_int_gt(response.binary_body_length, strtol(u_map_get_case(response.map_header, "Content-Length"), NULL, 10));
  ck_assert_ptr_ne(NULL, u_map_get_case(response.map_header, "Content-Encoding"));
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  free(u_compressed_inmemory_website_config.files_path);
  u_clean_compressed_inmemory_website_config(&u_compressed_inmemory_website_config);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

static Suite *ulfius_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Ulfius example callbacks function tests");
  tc_core = tcase_create("test_ulfius_callbacks");
  tcase_add_test(tc_core, test_ulfius_compress_allow_all_accept_all);
  tcase_add_test(tc_core, test_ulfius_compress_allow_all_accept_gzip);
  tcase_add_test(tc_core, test_ulfius_compress_allow_all_accept_deflate);
  tcase_add_test(tc_core, test_ulfius_compress_allow_all_accept_none);
  tcase_add_test(tc_core, test_ulfius_compress_allow_all_accept_unknown);
  tcase_add_test(tc_core, test_ulfius_compress_allow_gzip_accept_all);
  tcase_add_test(tc_core, test_ulfius_compress_allow_gzip_accept_gzip);
  tcase_add_test(tc_core, test_ulfius_compress_allow_gzip_accept_deflate);
  tcase_add_test(tc_core, test_ulfius_compress_allow_gzip_accept_none);
  tcase_add_test(tc_core, test_ulfius_compress_allow_gzip_accept_unknown);
  tcase_add_test(tc_core, test_ulfius_compress_allow_deflate_accept_all);
  tcase_add_test(tc_core, test_ulfius_compress_allow_deflate_accept_gzip);
  tcase_add_test(tc_core, test_ulfius_compress_allow_deflate_accept_deflate);
  tcase_add_test(tc_core, test_ulfius_compress_allow_deflate_accept_none);
  tcase_add_test(tc_core, test_ulfius_compress_allow_deflate_accept_unknown);
  tcase_add_test(tc_core, test_ulfius_compress_allow_none_accept_all);
  tcase_add_test(tc_core, test_ulfius_compress_allow_none_accept_gzip);
  tcase_add_test(tc_core, test_ulfius_compress_allow_none_accept_deflate);
  tcase_add_test(tc_core, test_ulfius_compress_allow_none_accept_none);
  tcase_add_test(tc_core, test_ulfius_compress_allow_none_accept_unknown);
  tcase_add_test(tc_core, test_ulfius_static_compressed_files_compress_accept_all);
  tcase_add_test(tc_core, test_ulfius_static_compressed_files_compress_accept_gzip);
  tcase_add_test(tc_core, test_ulfius_static_compressed_files_compress_accept_deflate);
  tcase_add_test(tc_core, test_ulfius_static_compressed_files_compress_accept_none);
  tcase_add_test(tc_core, test_ulfius_static_compressed_files_compress_no_cache);
  tcase_set_timeout(tc_core, 30);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void)
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  y_init_logs("Ulfius", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius example callbacks tests");
  ulfius_global_init();
  s = ulfius_suite();
  sr = srunner_create(s);
  
  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  ulfius_global_close();

  y_close_logs();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
