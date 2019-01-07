/*
 * Copyright (c) 2017-2018 Ben Cornett <ben@lantern.is>
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
	SMILES_BOND_UNSPECIFIED		= 0,
	SMILES_BOND_SINGLE		= 1,
	SMILES_BOND_DOUBLE		= 2,
	SMILES_BOND_TRIPLE		= 3,
	SMILES_BOND_QUAD		= 4,
	SMILES_BOND_AROMATIC		= 5,
};

enum {
	SMILES_BOND_STEREO_UNSPECIFIED,
	SMILES_BOND_STEREO_UP,
	SMILES_BOND_STEREO_DOWN,
};

struct smiles_atom {
	int				 atomic_number;
	char				 symbol[4];
	int				 isotope;
	int				 charge;
	int				 hcount;
	int				 implicit_hcount;
	int				 bracket;
	int				 organic;
	int				 aromatic;
	char				 chirality[8];
	int				 aclass;
	int				 pos;
	int				 len;
};

struct smiles_bond {
	int				 a0;
	int				 a1;
	int				 order;
	int				 stereo;
	int				 implicit;
	int				 ring;
	int				 pos;
	int				 len;
};

struct smiles_paren {
	int				 pos;
	struct smiles_bond		 bond;
};

struct smiles {
	const char			*smi;
	int				 pos;
	int				 end;
	char				*err;
	int				 errpos;

	struct smiles_atom		*atoms;
	size_t				 atoms_sz;
	size_t				 atoms_alloc;

	struct smiles_bond		*bonds;
	size_t				 bonds_sz;
	size_t				 bonds_alloc;

	struct smiles_bond		 rbonds[100];
	size_t				 open_ring_closures;

	struct smiles_paren		*paren_stack;
	size_t				 paren_stack_sz;
	size_t				 paren_stack_alloc;
};

void	smiles_free(struct smiles *);
void	smiles_init(struct smiles *);
int	smiles_parse(struct smiles *, const char *, size_t);
