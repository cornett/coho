#include <stdio.h>

#include "smiles.h"


int
main(void)
{
	struct smiles smi;
	char *line = NULL;
	size_t linesz = 0;
	ssize_t n;

	smiles_init(&smi);

	while ((n = getline(&line, &linesz, stdin)) != -1) {
		smiles_parse(&smi, line, n - 1);
	}

	smiles_free(&smi);
	return 0;
}
