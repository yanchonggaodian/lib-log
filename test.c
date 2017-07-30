#include <stdio.h>
#include "log.h"

int main(int argc, char**argv)
{

	(void)argc;
	(void)argv;
	log_t hnd = module_log_init("TEST", LOG_INFO, 8, NULL, NULL);
	char*p=NULL;
	LOG(&p, hnd, LOG_WARN, "ajksj\n");
	printf("%s",p);
	LOG(hnd, LOG_WARN, "ipgpqwoir\n");

	return 0;
}

