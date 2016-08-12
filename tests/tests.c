#include <stdlib.h>
#include <check.h>

START_TEST(vsprintf)
{
}
END_TEST



Suite * lib_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("lib");

	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, vsprintf);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;
	
	s = lib_suite();

	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);

	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

