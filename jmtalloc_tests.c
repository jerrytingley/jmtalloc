#include "jmtalloc.h"

int main() {
	char *test_data = (char *) jmtalloc(15);
	strcpy(test_data, "this is a test");
	printf("%s\n", test_data);
	jmtfree(test_data);
	//__print_diag_info(15);
	return 0;
}
