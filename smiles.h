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
	COHO_SMILES_BOND_UNSPECIFIED	= 0,
	COHO_SMILES_BOND_SINGLE		= 1,
	COHO_SMILES_BOND_DOUBLE		= 2,
	COHO_SMILES_BOND_TRIPLE		= 3,
	COHO_SMILES_BOND_QUAD		= 4,
	COHO_SMILES_BOND_AROMATIC	= 5,
};

enum {
	COHO_SMILES_BOND_STEREO_UNSPECIFIED,
	COHO_SMILES_BOND_STEREO_UP,
	COHO_SMILES_BOND_STEREO_DOWN,
};

struct coho_smiles_atom {
	int				 atomic_number;
	char				 symbol[4];
	int				 isotope;
	int				 charge;
	int				 hydrogen_count;
	int				 implicit_hydrogen_count;
	int				 is_bracket;
	int				 is_organic;
	int				 is_aromatic;
	char				 chirality[8];
	int				 atom_class;
	int				 position;
	int				 length;
};

struct coho_smiles_bond {
	int				 atom0;
	int				 atom1;
	int				 order;
	int				 stereo;
	int				 is_implicit;
	int				 is_ring;
	int				 position;
	int				 length;
};

struct coho_smiles_paren {
	int				 position;
	struct coho_smiles_bond		 bond;
};

struct coho_smiles {
	const char			*smi;
	int				 position;
	int				 end;
	char				*err;
	int				 error_position;

	struct coho_smiles_atom		*atoms;
	size_t				 atoms_sz;
	size_t				 atoms_alloc;

	struct coho_smiles_bond		*bonds;
	size_t				 bonds_sz;
	size_t				 bonds_alloc;

	struct coho_smiles_bond		 rbonds[100];
	size_t				 open_ring_closures;

	struct coho_smiles_paren	*paren_stack;
	size_t				 paren_stack_sz;
	size_t				 paren_stack_alloc;
};

void	coho_smiles_free(struct coho_smiles *);
void	coho_smiles_init(struct coho_smiles *);
int	coho_smiles_parse(struct coho_smiles *, const char *, size_t);
