/*
 * Copyright (c) 2017 Ben Cornett <ben@lantern.is>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


enum {
	SMI_BOND_UNSPECIFIED		= 0,
	SMI_BOND_SINGLE			= 1,
	SMI_BOND_DOUBLE			= 2,
	SMI_BOND_TRIPLE			= 3,
	SMI_BOND_QUAD			= 4,
	SMI_BOND_AROMATIC		= 5,
};


enum {
	SMI_BOND_STEREO_UNSPECIFIED,
	SMI_BOND_STEREO_UP,
	SMI_BOND_STEREO_DOWN,
};


struct smi_atom {
	int			 atomic_number;
	char			 symbol[4];
	int			 isotope;
	int			 charge;
	int			 hcount;
	int			 bracket;
	int			 organic;
	int			 aromatic;
	char			 chirality[8];
	int			 aclass;
	int			 pos;
	int			 len;
};


struct smi_bond {
	int			 a0;
	int			 a1;
	int			 order;
	int			 stereo;
	int			 implicit;
	int			 ring;
	int			 pos;
	int			 len;
};


struct smi_paren {
	int			 pos;
	struct smi_bond		 bond;
};


struct smi {
	const char		*smi;
	int			 pos;
	int			 end;
	char			*err;
	int			 errpos;

	struct smi_atom		*atoms;
	size_t			 atoms_sz;
	size_t			 atoms_alloc;

	struct smi_bond		*bonds;
	size_t			 bonds_sz;
	size_t			 bonds_alloc;

	struct smi_bond		 rbonds[100];
	size_t			 open_ring_closures;

	struct smi_paren	*paren_stack;
	size_t			 paren_stack_sz;
	size_t			 paren_stack_alloc;
};


void	smi_free(struct smi *);
void	smi_init(struct smi *);
int	smi_parse(struct smi *, const char *, size_t);
