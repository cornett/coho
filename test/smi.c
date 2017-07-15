#include <assert.h>
#include <stdio.h>

#include "smi.h"


static void
check_cnts(struct smi *x, const char *smi, size_t acnt, size_t bcnt)
{
	assert(smi_parse(x, smi, 0) == 0);
	assert(x->atoms_sz == acnt);
	assert(x->bonds_sz == bcnt);
}


int
main(void)
{
	struct smi x;

	smi_init(&x);

	check_cnts(&x, "CC", 2, 1);
	assert(x.bonds[0].ring == 0);
	check_cnts(&x, "C1.C1", 2, 1);
	assert(x.bonds[0].ring == 1);

	check_cnts(&x, "C.C", 2, 0);

	check_cnts(&x, "[*].C", 2, 0);
	assert(x.atoms[0].atomic_number == 0);
	assert(x.atoms[1].atomic_number == 6);

	assert(smi_parse(&x, "[*](C)^", 6) == 0);
	assert(x.atoms_sz == 2);
	assert(x.bonds_sz == 1);

	assert(smi_parse(&x, "[,*](C)^", 0) == -1);
	assert(x.errpos == 1);

	smi_free(&x);
	return 0;
}
