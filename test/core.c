/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#ifndef _WIN32
#include <netinet/in.h>
#endif
#include <jansson.h>

#include <check.h>
#include "../include/ulfius.h"

int callback_function_empty(const struct _u_request * request, struct _u_response * response, void * user_data) {
  return U_CALLBACK_CONTINUE;
}

#ifndef _WIN32
START_TEST(test_ulfius_init_instance)
{
  struct _u_instance u_instance;
  struct sockaddr_in listen;
  listen.sin_family = AF_INET;
  listen.sin_addr.s_addr = htonl (INADDR_ANY);

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, NULL), U_OK);
  ulfius_clean_instance(&u_instance);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 99999, NULL, NULL), U_ERROR_PARAMS);

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, &listen, NULL), U_OK);
  ulfius_clean_instance(&u_instance);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, "test_ulfius"), U_OK);
  ulfius_clean_instance(&u_instance);
}
END_TEST
#else
START_TEST(test_ulfius_init_instance)
{
  struct _u_instance u_instance;

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, NULL), U_OK);
  ulfius_clean_instance(&u_instance);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 99999, NULL, NULL), U_ERROR_PARAMS);

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, "test_ulfius"), U_OK);
  ulfius_clean_instance(&u_instance);
}
END_TEST
#endif

START_TEST(test_endpoint)
{
  struct _u_instance u_instance;
  struct _u_endpoint endpoint;
  endpoint.http_method = "nope";
  endpoint.url_prefix = NULL;
  endpoint.url_format = NULL;
  endpoint.priority = 0;
  endpoint.callback_function = NULL;

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.http_method = NULL;
  endpoint.url_prefix = "nope";
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.url_prefix = NULL;
  endpoint.url_format = "nope";
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  
  endpoint.callback_function = &callback_function_empty;
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.url_format = NULL;
  endpoint.url_prefix = "nope";
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.url_prefix = NULL;
  endpoint.http_method = "nope";
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.http_method = o_strdup("test0");
  endpoint.url_prefix = o_strdup("test0");
  endpoint.url_format = o_strdup("test0");
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_OK);
  
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "nope", NULL, NULL, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, "nope", NULL, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, NULL, "nope", 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, "nope", "nope", 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "nope", NULL, "nope", 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "nope", "nope", NULL, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "nope", NULL, NULL, 0, &callback_function_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, "nope", NULL, 0, &callback_function_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, NULL, "nope", 0, &callback_function_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, "nope", "nope", 0, &callback_function_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1", "test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test2", NULL, "test2", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test3", "test3", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_remove_endpoint(&u_instance, &endpoint), U_OK);
  ck_assert_int_eq(ulfius_remove_endpoint(&u_instance, &endpoint), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "nope", "nope", NULL), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "nope", NULL, "nope"), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "test1", "test1"), U_OK);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test2", NULL, "test2"), U_OK);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test3", "test3", NULL), U_OK);
  ck_assert_int_eq(ulfius_set_default_endpoint(&u_instance, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_set_default_endpoint(&u_instance, &callback_function_empty, NULL), U_OK);
  
  o_free(endpoint.http_method);
  o_free(endpoint.url_prefix);
  o_free(endpoint.url_format);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_start_instance)
{
  struct _u_instance u_instance;

  ck_assert_int_eq(ulfius_start_framework(NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_start_secure_framework(NULL, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", NULL, "test", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  ck_assert_int_eq(ulfius_stop_framework(&u_instance), U_OK);
  ck_assert_int_eq(ulfius_start_secure_framework(&u_instance, "error", "error"), U_ERROR_LIBMHD);
  ck_assert_int_eq(ulfius_start_secure_framework(&u_instance, "-----BEGIN PRIVATE KEY-----\
MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDr90HrswgEmln/\
rXeNqYq0boIvas5wu27hmeHDdGGKtkCWIWGAo9GUy45xqsI4mDl3bOWS+pmb/3yi\
+nhe+BmYHvEqUFo1JfUcVMxaNEbdd9REytMjKdOS+kkLf++BBRoZI/g8DggIu+Ri\
dOSypk+pUECyQxROsyCrB/FgXuKbyC4QNl7fqZxMSpzw7jsWCZiwFv4pu8kMqzDG\
2wTl/r/4STyK4Pj2TVa/JVzbZbH7VfcjT8MdMsXvKhlmPywjbqo70Hnmt3cnakYF\
X+07ncx/5mjYYd3eSFgiNXr7WNw2rhFKtfTUcjrqSw9FDxmHFWUU76mwJyUo02N9\
ViakSoQpAgMBAAECggEBAJp/VBwdJpzM6yxqyaJpZbXpvTeKuQw6zMjN1nIBG3SV\
DAjAZnSxziGcffGSmoQvt0CoflAT4MuxJkwXrwSPcUKWz9Sis82kwq4AH6TYIaYU\
NVmtazzUwAC1+2maJJjXXFUlpfy8Oypsy4ZjfvIxzmrPbuzI2t0Ej9kr5DDzL3BL\
CWQ/U7w7y4KC0Pnq1ueIzM+UJIfvI0ldUcXHWsAnjyQzwgFBC35qDOfDTw0YUJv+\
ElfFFcGYCA+9wlQyhM/zhAWqKgZ2mwAS6WykgbSc7j4NDjlmZwf4ZuTxbDUV1kBX\
pPH21snqO42CFpw9hRUAA0W0XydCIfUhH8/6tH9enQECgYEA+rM9f6cUk3c7aLWs\
hnauVqJuyGhgCkMyF9sSxgfcs87OVLNuGgaTIfwcT/7oxAY8G7sY44cbk1ZRhh7y\
6kf01xqiJeXxBQei1qiJxMb2gukvpeY81s2Mg9og5d9qbEhLzp8TdiRJHxLIiGwF\
xOM69CpugKN4T0Zum7EBGeSvmBECgYEA8PRG5SRTE4JwzGtLuTbMbjYTqyEjXAkB\
zo33a92znA0EXEeLCl845EUgzUkSkeN/T/uyWRjj0hrPU99UaaXHt3bc+lrDHrc7\
WFAR3QoAfFFJPIqqwiHcBDdTeAozQ8IOqFIxspl72RukuRdeQR3EdfcF9TUZyUbU\
k8SuRioggpkCgYA2scgnA3KvwYGKlKgxJc9fQ0zcGDlrw8E4BymPXsO9zs6hGAxb\
TTfoYDJlGX361kli22zQpvdTK6/ZjQL+LfiyvTLHBeWRbVsPbfGwpp+9a9ZjYVnA\
m1OeqIYo4Jc9TICNcZMzYTM6vkRVzwtrKw//mQpGsmNbGEilWvaciZHtoQKBgQDo\
FDBQtir6SJIConm9/ETtBlLtai6Xj+lYnK6qC1DaxkLj6tjF9a9jVh3g/DfRopBW\
ZnSCkpGkJcR54Up5s35ofCkdTdxPsmaLihuaje6nztc+Y8VS1LAIs41GunRkF/5s\
KzbI8kIyfAitag+Toms+v93SLwIWNo27gh3lYOANSQKBgQDIidSO3fzB+jzJh7R0\
Yy9ADWbBsLxc8u+sBdxmZBGl+l4YZWNPlQsnsafwcpJWT3le6N7Ri3iuOZw9KiGe\
QDkc7olxUZZ3pshg+cOORK6jVE8v6FeUlLnxpeAWa4C4JDawGPTOBct6bVBl5sxi\
7GaqDcEK1TSxc4cUaiiPDNNXQA==\
-----END PRIVATE KEY-----", "-----BEGIN CERTIFICATE-----\
MIIDhTCCAm2gAwIBAgIJANrO2RnCbURLMA0GCSqGSIb3DQEBCwUAMFkxCzAJBgNV\
BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX\
aWRnaXRzIFB0eSBMdGQxEjAQBgNVBAMMCWxvY2FsaG9zdDAeFw0xNzA0MjgxNTA0\
NDVaFw0xODA0MjgxNTA0NDVaMFkxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21l\
LVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQxEjAQBgNV\
BAMMCWxvY2FsaG9zdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOv3\
QeuzCASaWf+td42pirRugi9qznC7buGZ4cN0YYq2QJYhYYCj0ZTLjnGqwjiYOXds\
5ZL6mZv/fKL6eF74GZge8SpQWjUl9RxUzFo0Rt131ETK0yMp05L6SQt/74EFGhkj\
+DwOCAi75GJ05LKmT6lQQLJDFE6zIKsH8WBe4pvILhA2Xt+pnExKnPDuOxYJmLAW\
/im7yQyrMMbbBOX+v/hJPIrg+PZNVr8lXNtlsftV9yNPwx0yxe8qGWY/LCNuqjvQ\
eea3dydqRgVf7TudzH/maNhh3d5IWCI1evtY3DauEUq19NRyOupLD0UPGYcVZRTv\
qbAnJSjTY31WJqRKhCkCAwEAAaNQME4wHQYDVR0OBBYEFPFfmGA3jO9koBZNGNZC\
T/dZHZyHMB8GA1UdIwQYMBaAFPFfmGA3jO9koBZNGNZCT/dZHZyHMAwGA1UdEwQF\
MAMBAf8wDQYJKoZIhvcNAQELBQADggEBAIc8Yuom4vz82izNEV+9bcCvuabcVwLH\
Qgpv5Nzy/W+1hDoqfMfKNwOSdUB7jZoDaNDG1WhjKGGCLTAx4Hx+q1LwUXvu4Bs1\
woocge65bl85h10l2TxxnlT5BIJezm5r3NiZSwOK2zxxIEyL4vh+b/xqQblBEkR3\
e4/A4Ugn9Egh8GdpF4klGp4MjjpRyAVI7BDaleAhvDSfPmm7ylHJ2y7CLI9ApOQY\
glwRuTmowAZQtaSiE1Ox7QtWj858HDzzTZyFWRG/MNqQptn7AMTPJv3DivNfDNPj\
fYxFAheH3CjryHqqR9DD+d9396W8mqEaUp+plMwSjpcTDSR4rEQkUJg=\
-----END CERTIFICATE-----"), U_OK);
  ck_assert_int_eq(ulfius_stop_framework(&u_instance), U_OK);
  ulfius_clean_instance(&u_instance);
}
END_TEST

static Suite *ulfius_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Ulfius core function tests");
	tc_core = tcase_create("test_ulfius_core");
	tcase_add_test(tc_core, test_ulfius_init_instance);
	tcase_add_test(tc_core, test_endpoint);
	tcase_add_test(tc_core, test_ulfius_start_instance);
	tcase_set_timeout(tc_core, 30);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  //y_init_logs("Ulfius", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius core tests");
  s = ulfius_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  //y_close_logs();
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
