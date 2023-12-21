/* Public domain, no copyright. Use at your own risk. */

#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ulfius.h>

START_TEST(test_u_map_init)
{
  struct _u_map map;
  ck_assert_int_eq(u_map_init(&map), U_OK);
  ck_assert_int_eq(u_map_init(NULL), U_ERROR_PARAMS);
  u_map_clean(&map);
}
END_TEST

START_TEST(test_u_map_put)
{
  struct _u_map map;
  u_map_init(&map);
  ck_assert_int_eq(u_map_put(&map, "key", "value"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key", "value2"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key", NULL), U_OK);
  ck_assert_int_eq(u_map_put(&map, NULL, "value"), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_put(&map, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_put(NULL, "key", "value"), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_put(&map, "key", "new_value"), U_OK);
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "value", 0, o_strlen("value")), U_OK);
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", NULL, 0, o_strlen("value")), U_OK);
  ck_assert_int_eq(u_map_put_binary(&map, NULL, "value", 0, o_strlen("value")), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_put_binary(&map, NULL, NULL, 0, o_strlen("value")), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_put_binary(NULL, "bkey", "value", 0, o_strlen("value")), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "new_value", 0, o_strlen("new_value")), U_OK);
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "append_value", u_map_get_length(&map, "bkey"), o_strlen("append_value")), U_OK);
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "far_value", 1024, o_strlen("far_value")), U_OK);
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "inserted_value", 3, o_strlen("inserted_value")), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key", "value"), U_OK);
  u_map_clean(&map);
}
END_TEST

START_TEST(test_u_map_get)
{
  struct _u_map map;
  u_map_init(&map);
  ck_assert_int_eq(u_map_put(&map, "key", "value"), U_OK);
  ck_assert_str_eq(u_map_get(&map, "key"), "value");
  ck_assert_int_eq(u_map_get_length(&map, "key"), 6);
  ck_assert_ptr_eq((void *)u_map_get(&map, "nope"), NULL);
  ck_assert_ptr_eq((void *)u_map_get(NULL, "key"), NULL);
  ck_assert_int_eq(u_map_get_length(&map, "nope"), -1);
  ck_assert_int_eq(u_map_get_length(NULL, "key"), -1);
  ck_assert_str_eq(u_map_get_case(&map, "Key"), "value");
  ck_assert_int_eq(u_map_get_case_length(&map, "kEy"), 6);
  ck_assert_ptr_eq((void *)u_map_get_case(&map, "noPE"), NULL);
  ck_assert_ptr_eq((void *)u_map_get_case(NULL, "keY"), NULL);
  ck_assert_int_eq(u_map_get_case_length(&map, "noPe"), -1);
  ck_assert_int_eq(u_map_get_case_length(NULL, "KEy"), -1);
  ck_assert_int_eq(u_map_put(&map, "key", NULL), U_OK);
  ck_assert_ptr_eq(u_map_get(&map, "key"), NULL);
  ck_assert_int_eq(u_map_put(&map, "key", "value"), U_OK);
  ck_assert_str_eq(u_map_get(&map, "key"), "value");
  ck_assert_int_eq(u_map_put(&map, "key", NULL), U_OK);
  ck_assert_ptr_eq(u_map_get(&map, "key"), NULL);
  ck_assert_int_eq(u_map_put(&map, "key", "value"), U_OK);
  ck_assert_str_eq(u_map_get(&map, "key"), "value");
  ck_assert_int_eq(u_map_put(&map, "key", NULL), U_OK);
  ck_assert_ptr_eq(u_map_get(&map, "key"), NULL);
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "new_value", 0, o_strlen("new_value")), U_OK);
  ck_assert_str_eq(u_map_get(&map, "bkey"), "new_value");
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "append_value", u_map_get_length(&map, "bkey"), o_strlen("append_value")), U_OK);
  ck_assert_str_eq(u_map_get(&map, "bkey"), "new_valueappend_value");
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "inserted_value", 3, o_strlen("inserted_value")), U_OK);
  ck_assert_str_eq(u_map_get(&map, "bkey"), "newinserted_valuealue");
  ck_assert_int_eq(u_map_put_binary(&map, "bkey", "far_value", 1024, o_strlen("far_value")), U_OK);
  ck_assert_str_eq(u_map_get(&map, "bkey"), "newinserted_valuealue");
  ck_assert_int_eq(1024+o_strlen("far_value"), u_map_get_length(&map, "bkey"));
  u_map_clean(&map);
}
END_TEST

START_TEST(test_u_map_get_case)
{
  struct _u_map map;
  u_map_init(&map);
  ck_assert_int_eq(u_map_put(&map, "key", "value"), U_OK);
  ck_assert_str_eq(u_map_get(&map, "key"), "value");
  ck_assert_int_eq(u_map_get_case_length(&map, "key"), 6);
  ck_assert_ptr_eq((void *)u_map_get_case(&map, "nope"), NULL);
  ck_assert_ptr_eq((void *)u_map_get_case(NULL, "key"), NULL);
  ck_assert_int_eq(u_map_get_case_length(&map, "nope"), -1);
  ck_assert_int_eq(u_map_get_case_length(NULL, "key"), -1);
  u_map_clean(&map);
}
END_TEST

START_TEST(test_u_map_enum)
{
  struct _u_map map;
  const char ** enum_keys, ** enum_values;
  u_map_init(&map);
  ck_assert_int_eq(u_map_put(&map, "key1", "value1"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key2", "value2"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key3", "value3"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key4", "value4"), U_OK);
  enum_keys = u_map_enum_keys(&map);
  enum_values = u_map_enum_values(&map);
  ck_assert_ptr_ne(enum_keys, NULL);
  ck_assert_ptr_ne(enum_values, NULL);
  ck_assert_int_eq(u_map_count(&map), 4);
  u_map_clean(&map);
}
END_TEST

START_TEST(test_u_map_has)
{
  struct _u_map map;
  u_map_init(&map);
  ck_assert_int_eq(u_map_put(&map, "key1", "value1"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key2", "value2"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key3", "value3"), U_OK);
  ck_assert_int_eq(u_map_put_binary(&map, "key4", "value4", 0, o_strlen("value4")), U_OK);
  ck_assert_int_eq(u_map_has_key(&map, "key3"), 1);
  ck_assert_int_eq(u_map_has_key(&map, "nope"), 0);
  ck_assert_int_eq(u_map_has_value(&map, "value1"), 1);
  ck_assert_int_eq(u_map_has_value(&map, "nope"), 0);
  ck_assert_int_eq(u_map_has_value_binary(&map, "value4", o_strlen("value4")), 1);
  ck_assert_int_eq(u_map_has_value_binary(&map, "nope", o_strlen("nope")), 0);
  ck_assert_int_eq(u_map_has_key_case(&map, "Key3"), 1);
  ck_assert_int_eq(u_map_has_key_case(&map, "Nope"), 0);
  ck_assert_int_eq(u_map_has_value_case(&map, "ValuE1"), 1);
  ck_assert_int_eq(u_map_has_value_case(&map, "nOpe"), 0);
  u_map_clean(&map);
}
END_TEST

START_TEST(test_u_map_remove)
{
  struct _u_map map;
  u_map_init(&map);
  ck_assert_int_eq(u_map_put(&map, "key1", "value1"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key2", "value2"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key3", "value3"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key4", "value4"), U_OK);
  ck_assert_int_eq(u_map_put_binary(&map, "key5", "value5", 0, o_strlen("value5")), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key6", "value6"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key7", "value7"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key8", "value8"), U_OK);
  ck_assert_int_eq(u_map_remove_from_key(&map, "key1"), U_OK);
  ck_assert_int_eq(u_map_remove_from_key(&map, "nope"), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(u_map_remove_from_key_case(&map, "Key2"), U_OK);
  ck_assert_int_eq(u_map_remove_from_key_case(&map, "nOPe"), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(u_map_remove_from_value(&map, "value3"), U_OK);
  ck_assert_int_eq(u_map_remove_from_value(&map, "nope"), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(u_map_remove_from_value_case(&map, "value4"), U_OK);
  ck_assert_int_eq(u_map_remove_from_value_case(&map, "nOPe"), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(u_map_remove_from_value_binary(&map, "value5", o_strlen("value5")), U_OK);
  ck_assert_int_eq(u_map_remove_from_value_binary(&map, "nope", o_strlen("nope")), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(u_map_remove_at(&map, 1), U_OK);
  ck_assert_int_eq(u_map_remove_at(&map, 10), U_ERROR_NOT_FOUND);
  u_map_clean(&map);
}
END_TEST

START_TEST(test_u_map_copy_empty)
{
  struct _u_map map, copy_map;
  u_map_init(&map);
  u_map_init(&copy_map);
  ck_assert_int_eq(u_map_put(&map, "key1", "value1"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key2", "value2"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key3", "value3"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key4", NULL), U_OK);
  ck_assert_int_eq(u_map_copy_into(&copy_map, &map), U_OK);
  ck_assert_int_eq(u_map_copy_into(NULL, &map), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_copy_into(&copy_map, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_copy_into(NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_count(&map), 4);
  ck_assert_int_eq(u_map_count(&copy_map), 4);
  ck_assert_int_eq(u_map_empty(&copy_map), U_OK);
  ck_assert_int_eq(u_map_empty(NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(u_map_count(&copy_map), 0);
  u_map_clean(&map);
  u_map_clean(&copy_map);
}
END_TEST

START_TEST(test_u_map_copy)
{
  struct _u_map map, * copy_map;
  u_map_init(&map);
  ck_assert_int_eq(u_map_put(&map, "key1", "value1"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key2", "value2"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key3", "value3"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key4", NULL), U_OK);
  ck_assert_ptr_eq(NULL, u_map_copy(NULL));
  ck_assert_ptr_ne(NULL, copy_map = u_map_copy(&map));
  ck_assert_int_eq(u_map_count(&map), 4);
  ck_assert_int_eq(u_map_count(copy_map), 4);
  u_map_clean(&map);
  u_map_clean_full(copy_map);
}
END_TEST

START_TEST(test_u_map_count)
{
  struct _u_map map;
  u_map_init(&map);
  ck_assert_int_eq(u_map_count_keys_case(&map, "key3"), 0);
  ck_assert_int_eq(u_map_put(&map, "key1", "value1"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key2", "value2"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "key3", "value3"), U_OK);
  ck_assert_int_eq(u_map_count(&map), 3);
  ck_assert_int_eq(u_map_count_keys_case(&map, "key3"), 1);
  ck_assert_int_eq(u_map_put(&map, "Key3", "value4"), U_OK);
  ck_assert_int_eq(u_map_count(&map), 4);
  ck_assert_int_eq(u_map_count_keys_case(&map, "key3"), 2);
  ck_assert_int_eq(u_map_put(&map, "KEY3", "value5"), U_OK);
  ck_assert_int_eq(u_map_put(&map, "KEy3", "value6"), U_OK);
  ck_assert_int_eq(u_map_count_keys_case(&map, "key3"), 4);
  ck_assert_int_eq(u_map_count_keys_case(&map, "KEY3"), 4);
  ck_assert_int_eq(u_map_remove_from_key(&map, "KEY3"), U_OK);
  ck_assert_int_eq(u_map_count_keys_case(&map, "key3"), 3);
  ck_assert_int_eq(u_map_count_keys_case(&map, "KEY3"), 3);
  ck_assert_int_eq(u_map_count_keys_case(&map, "key3e"), 0);
  ck_assert_int_eq(u_map_count_keys_case(&map, "key3 "), 0);
  ck_assert_int_eq(u_map_count_keys_case(&map, " key3"), 0);
  u_map_clean(&map);
}
END_TEST

static Suite *ulfius_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Ulfius struct _u_map function tests");
	tc_core = tcase_create("test_ulfius_u_map");
	tcase_add_test(tc_core, test_u_map_init);
	tcase_add_test(tc_core, test_u_map_put);
	tcase_add_test(tc_core, test_u_map_get);
	tcase_add_test(tc_core, test_u_map_get_case);
	tcase_add_test(tc_core, test_u_map_enum);
	tcase_add_test(tc_core, test_u_map_has);
	tcase_add_test(tc_core, test_u_map_remove);
	tcase_add_test(tc_core, test_u_map_copy_empty);
	tcase_add_test(tc_core, test_u_map_copy);
	tcase_add_test(tc_core, test_u_map_count);
	tcase_set_timeout(tc_core, 30);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  
  //y_init_logs("Ulfius", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius u_map tests");
  ulfius_global_init();
  
  s = ulfius_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  ulfius_global_close();
  //y_close_logs();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
