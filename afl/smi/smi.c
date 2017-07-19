#include <stdio.h>

#include "smi.h"


int
main(void)
{
	struct smi smi;
	char *line = NULL;
	size_t linesz = 0;
	ssize_t n;

	smi_init(&smi);

	while ((n = getline(&line, &linesz, stdin)) != -1) {
		smi_parse(&smi, line, n - 1);
	}

	smi_free(&smi);
	return 0;
}
