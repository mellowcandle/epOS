#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "printk.h"

void VIDEO_print_string(const char * str)
{
	printf("Recieved string: %s\r\n", str);
	ck_assert_str_eq(str, "string: Hello world, char: H, Hex: 0x1A, Integer: 15389, Negative Integer: -15389\r\n");
}

START_TEST(test_printk)
{
	char * string = "Hello world";
	char c = 'H';
	int hex = 0x1A;
	int pos = 15389;
	int neg = -15389;
	printk("string: %s, char: %c, Hex: %x, Integer: %u, Negative Integer: %d\r\n", string, c, hex, pos, neg);
}
END_TEST



Suite * lib_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("lib");

	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_printk);
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
	srunner_set_fork_status ( sr, CK_NOFORK );
	srunner_run_all(sr, CK_NORMAL);

	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

