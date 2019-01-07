#include <stdio.h>

#include "smiles.h"


int
main(void)
{
	struct coho_smiles smi;
	char *line = NULL;
	size_t linesz = 0;
	ssize_t n;

	coho_smiles_init(&smi);

	while ((n = getline(&line, &linesz, stdin)) != -1) {
		coho_smiles_parse(&smi, line, n - 1);
	}

	coho_smiles_free(&smi);
	return 0;
}
