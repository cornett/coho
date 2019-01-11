#include <assert.h>
#include <stdio.h>

#include "coho.h"


static void
check_cnts(struct coho_smiles *x, const char *smi, int acnt, int bcnt)
{
	assert(coho_smiles_parse(x, smi, 0) == COHO_OK);
	assert(x->atom_count == acnt);
	assert(x->bond_count == bcnt);
}


int
main(void)
{
	struct coho_smiles x;

	coho_smiles_init(&x);

	check_cnts(&x, "CC", 2, 1);
	assert(x.bonds[0].is_ring == 0);
	check_cnts(&x, "C1.C1", 2, 1);
	assert(x.bonds[0].is_ring == 1);

	check_cnts(&x, "C.C", 2, 0);

	check_cnts(&x, "[*].C", 2, 0);
	assert(x.atoms[0].atomic_number == 0);
	assert(x.atoms[1].atomic_number == 6);

	assert(coho_smiles_parse(&x, "[*](C)^", 6) == COHO_OK);
	assert(x.atom_count == 2);
	assert(x.bond_count == 1);

	assert(coho_smiles_parse(&x, "[,*](C)^", 0) == COHO_ERROR);
	assert(x.error_position == 1);

	coho_smiles_free(&x);
	return 0;
}
