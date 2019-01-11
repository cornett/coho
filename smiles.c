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

/*
 * Parses SMILES as specified by the OpenSMILES standard.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

#include "coho.h"

#define ALIPHATIC_ORGANIC	0x00001
#define AROMATIC		0x00002
#define AROMATIC_ORGANIC	0x00004
#define BOND			0x00008
#define BRACKET_CLOSE		0x00010
#define BRACKET_OPEN		0x00020
#define CHIRALITY		0x00040
#define COLON			0x00080
#define DIGIT			0x00100
#define DOT			0x00200
#define ELEMENT			0x00400
#define HYDROGEN		0x00800
#define MINUS			0x01000
#define PAREN_CLOSE		0x02000
#define PAREN_OPEN		0x04000
#define PERCENT			0x08000
#define PLUS			0x10000
#define WILDCARD		0x20000

struct token {
	int		 type;
	int		 position;
	const char	*s;
	size_t		 n;
	int		 intval;
	int		 flags;
};

static int atom_class(struct coho_smiles *, struct coho_smiles_atom *);
static int add_atom(struct coho_smiles *, struct coho_smiles_atom *);
static int add_bond(struct coho_smiles *, struct coho_smiles_bond *);
static int add_ringbond(struct coho_smiles *, int, struct coho_smiles_bond *);
static int aliphatic_organic(struct coho_smiles *, struct coho_smiles_atom *);
static int aromatic_organic(struct coho_smiles *, struct coho_smiles_atom *);
static int assign_implicit_hydrogen_count(struct coho_smiles *);
static int atom(struct coho_smiles *, int *);
static int atom_ringbond(struct coho_smiles *, int *);
static int atom_valence(struct coho_smiles *, size_t);
static int bond(struct coho_smiles *, struct coho_smiles_bond *b);
static int bracket_atom(struct coho_smiles *, struct coho_smiles_atom *);
static int charge(struct coho_smiles *, struct coho_smiles_atom *);
static int check_ring_closures(struct coho_smiles *);
static int chirality(struct coho_smiles *, struct coho_smiles_atom *);
static int close_paren(struct coho_smiles *, struct coho_smiles_bond *);
static int dot(struct coho_smiles *);
static int ensure_array_capacities(struct coho_smiles *, size_t);
static int hydrogen_count(struct coho_smiles *, struct coho_smiles_atom *);
static int integer(struct coho_smiles *, size_t, int *);
static int isotope(struct coho_smiles *, struct coho_smiles_atom *);
static unsigned int lex(struct coho_smiles *, struct token *, int);
static int match(struct coho_smiles *, struct token *, int, unsigned int);
static size_t next_array_capacity(size_t);
static int open_paren(struct coho_smiles *, struct coho_smiles_bond *);
static int pop_paren_stack(struct coho_smiles *, int, struct coho_smiles_bond *);
static void push_paren_stack(struct coho_smiles *, int, struct coho_smiles_bond *);
static int ringbond(struct coho_smiles *, int);
static int round_valence(int, int, int);
static void coho_smiles_atom_init(struct coho_smiles_atom *);
static void coho_smiles_bond_init(struct coho_smiles_bond *);
static void coho_smiles_reinit(struct coho_smiles *, const char *, size_t);
static int symbol(struct coho_smiles *, struct coho_smiles_atom *);
static void tokcpy(char *, struct token *, size_t);
static int wildcard(struct coho_smiles *, struct coho_smiles_atom *);

/*
 * Table of standard atom valences.
 * <atomic number> <valence>...
 */
static int standard_valences[][4] = {
	{5,	3,	-1,	-1},	/* B */
	{6,	4,	-1,	-1},	/* C */
	{7,	3,	5,	-1},	/* N */
	{8,	2,	-1,	-1},	/* O */
	{9,	1,	-1,	-1},	/* F */
	{15,	3,	5,	-1},	/* P */
	{16,	2,	4,	6},	/* S */
	{17,	1,	-1,	-1},	/* Cl */
	{35,	1,	-1,	-1},	/* Br */
	{53,	1,	-1,	-1},	/* I */
	{-1,	-1,	-1,	-1},
};

void
coho_smiles_free(struct coho_smiles *x)
{
	free(x->error);
	free(x->atoms);
	free(x->bonds);
	free(x->paren_stack);
}

void
coho_smiles_init(struct coho_smiles *x)
{
	size_t i;

	x->smiles = NULL;
	x->position = 0;
	x->end = 0;
	x->error = NULL;
	x->error_position = -1;

	x->atom_count = 0;
	x->bond_count = 0;

	x->atoms = NULL;
	x->atoms_capacity = 0;
	x->bonds = NULL;
	x->bonds_capacity = 0;
	x->paren_stack = NULL;
	x->paren_stack_capacity = 0;

	for (i = 0; i < 100; i++)
		coho_smiles_bond_init(&x->ring_bonds[i]);
	x->open_ring_closures = 0;
}

int
coho_smiles_parse(struct coho_smiles *x, const char *smiles, size_t sz)
{
	struct coho_smiles_bond b;
	int anum;			/* index of last atom read */
	int eos;			/* end-of-string flag */
	int rc;
	size_t end;

	enum {
		INIT,
		ATOM_READ,
		BOND_READ,
		DOT_READ,
		OPEN_PAREN_READ,
		CLOSE_PAREN_READ,
	} state;

	end = sz ? sz : strlen(smiles);
	if (sz > INT_MAX) {
		x->error = strdup("SMILES too long");
		return COHO_NOMEM;
	}
	coho_smiles_reinit(x, smiles, end);

	if (ensure_array_capacities(x, end)) {
		return COHO_NOMEM;
	}

	b.atom0 = -1;		/* no previous atom to bond to */
	anum = -1;
	state = INIT;

	for (;;) {
		eos = x->position == x->end;

		switch (state) {

		/*
		 * Parsing has just begun.
		 */
		case INIT:
			if (eos) {
				x->error = strdup("empty SMILES");
				goto err;
			}

			else if ((rc = atom_ringbond(x, &anum))) {
				if (rc == -1)
					goto err;
			}

			else {
				x->error = strdup("atom expected");
				goto err;
			}
			state = ATOM_READ;
			break;

		/*
		 * An atom has just been read.
		 */
		case ATOM_READ:
			/*
			 * If there is an open bond to the previous
			 * atom, complete it.
			 */
			if (b.atom0 != -1) {
				b.atom1 = anum;
				/*
				 * Finalize order of implicit bonds, which
				 * depends on atom aromaticity.
				 */
				if (b.is_implicit) {
					if (x->atoms[b.atom0].is_aromatic &&
					    x->atoms[b.atom1].is_aromatic)
						b.order = COHO_SMILES_BOND_AROMATIC;
					else
						b.order = COHO_SMILES_BOND_SINGLE;
				}
				if (add_bond(x, &b) == -1)
					goto err;
			}

			/*
			 * The atom just read may be bonded to
			 * subsequent atoms.
			 * Store this state in an incomplete bond.
			 */
			coho_smiles_bond_init(&b);
			b.atom0 = anum;
			b.is_implicit = 1;

			if (eos) {
				goto done;
			}

			else if ((rc = atom_ringbond(x, &anum))) {
				if (rc == -1)
					goto err;
			}

			else if ((rc = bond(x, &b))) {
				if (rc == -1)
					goto err;
				state = BOND_READ;
			}

			else if (dot(x)) {
				state = DOT_READ;
			}

			else if (open_paren(x, &b)) {
				state = OPEN_PAREN_READ;
			}

			else if ((rc = close_paren(x, &b))) {
				if (rc == -1)
					goto err;
				state = CLOSE_PAREN_READ;
			}

			else {
				goto unexpected;
			}

			break;

		/*
		 * A dot (.) has just been read.
		 * An atom is expected.
		 * If there is a bond to a previous atom awaiting
		 * completion, it must be cancelled.
		 */
		case DOT_READ:
			/* Invalidate open bond to previous atom. */
			b.atom0 = -1;

			if ((rc = atom_ringbond(x, &anum))) {
				if (rc == -1)
					goto err;
			} else {
				x->error = strdup("atom must follow dot");
				goto err;
			}
			state = ATOM_READ;
			break;

		/*
		 * A bond (-, =, #, etc) has just been read.
		 * An atom is expected.
		 */
		case BOND_READ:

			if ((rc = atom_ringbond(x, &anum))) {
				if (rc == -1)
					goto err;
			} else {
				x->error = strdup("atom must follow bond");
				goto err;
			}
			state = ATOM_READ;
			break;

		/*
		 * An opening parenthesis has just been read
		 * and the parenthesis stack pushed.
		 */
		case OPEN_PAREN_READ:

			if (eos) {
				x->error = strdup("unbalanced parenthesis");
				x->error_position = x->position - 1;
				goto err;
			}

			else if ((rc = atom_ringbond(x, &anum))) {
				if (rc == -1)
					goto err;
				state = ATOM_READ;
			}

			else if ((rc = bond(x, &b))) {
				if (rc == -1)
					goto err;
				state = BOND_READ;
			}

			else if (dot(x)) {
				state = DOT_READ;
			}

			else {
				x->error = strdup("atom, bond, or dot "
						"expected");
				goto err;
			}
			break;

		/*
		 * A closing parenthesis has just been read
		 * and the parenthesis stack popped.
		 */
		case CLOSE_PAREN_READ:

			if (eos) {
				goto done;
			}

			else if ((rc = atom_ringbond(x, &anum))) {
				if (rc == -1)
					goto err;
				state = ATOM_READ;
			}

			else if ((rc = bond(x, &b))) {
				if (rc == -1)
					goto err;
				state = BOND_READ;
			}

			else if (dot(x)) {
				state = DOT_READ;
			}

			else if (open_paren(x, &b)) {
				state = OPEN_PAREN_READ;
			}

			else if ((rc = close_paren(x, &b))) {
				if (rc == -1)
					goto err;
				state = CLOSE_PAREN_READ;
			}

			else {
				goto unexpected;
			}
			break;

		}

	}

done:
	assert(x->position == x->end);

	if (check_ring_closures(x))
		goto err;

	if (x->paren_stack_count > 0) {
		x->error = strdup("unbalanced parenthesis");
		x->error_position = x->paren_stack[0].position;
		goto err;
	}

	if (assign_implicit_hydrogen_count(x))
		goto err;

	return COHO_OK;

unexpected:
	x->error = strdup("unexpected character");
err:
	if (x->error_position == -1)
		x->error_position = x->position;
	return COHO_ERROR;
}

/*
 * Parses optional atom class inside a bracket atom (ex: [C:23]).
 * If successful, sets a->atom_class and increments a->length.
 * Returns 1 if atom class was read, else 0.
 * On error, sets x->error and returns -1.
 *
 * class ::= ':' NUMBER
 */
static int
atom_class(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;
	int n;

	if (!match(x, &t, 1, COLON))
		return 0;

	a->length += t.n;

	if ((n = integer(x, 8, &a->atom_class)) == -1) {
		x->error = strdup("atom class too large");
		return -1;
	} else if (n == 0) {
		x->error = strdup("atom class expected");
		return -1;
	}

	a->length += n;
	return 1;
}

/*
 * Saves a completed atom and returns its index.
 */
static int
add_atom(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	x->atoms[x->atom_count] = *a;
	return x->atom_count++;
}

/*
 * Saves a new bond to the bond list and returns its index.
 * Returns new length of bond list on success.
 * If the bond is already in the list, sets x->error and returns -1.
 * Bonds are added so that bond->atom0 < bond->atom1 and the entire bond list
 * remains sorted.
 */
static int
add_bond(struct coho_smiles *x, struct coho_smiles_bond *bond)
{
	size_t i, move;
	struct coho_smiles_bond nb, *b;

	nb = *bond;

	/* Flip so atom0 < atom1
	 */
	if (bond->atom0 > bond->atom1) {
		nb.atom0 = bond->atom1;
		nb.atom1 = bond->atom0;

		if (bond->stereo == COHO_SMILES_BOND_STEREO_UP)
			nb.stereo = COHO_SMILES_BOND_STEREO_DOWN;
		else if (bond->stereo == COHO_SMILES_BOND_STEREO_DOWN)
			nb.stereo = COHO_SMILES_BOND_STEREO_UP;
	}

	/* Find position to insert and check for duplicates.
	 * Start search from end, since bonds are
	 * mostly generated in the correct order.
	 */
	for (i = x->bond_count; i > 0; i--) {
		b = &x->bonds[i-1];

		if (nb.atom0 > b->atom0)
			break;
		else if (nb.atom0 < b->atom0)
			continue;
		else if (nb.atom1 > b->atom1)
			break;
		else if (nb.atom1 < b->atom1)
			continue;
		else {
			x->error = strdup("duplicate bond");
			x->error_position = nb.position;
			return -1;
		}
	}

	move = x->bond_count - i;			/* # elements to shift */
	if (move) {
		memmove(x->bonds + i + 1,
			x->bonds + i,
			move * sizeof(x->bonds[0]));
	}

	x->bonds[i] = nb;
	return x->bond_count++;
}

/*
 * Adds a ring bond closure.
 * If there is already an open ring bond using rnum,
 * it is closed and the new bond is added to the bond list.
 * Otherwise, a new bond is opened.
 * Returns 0 on success.
 * On failure, sets x->error and returns -1.
 */
static int
add_ringbond(struct coho_smiles *x, int rnum, struct coho_smiles_bond *b)
{
	struct coho_smiles_bond *rb;

	assert(rnum < 100);

	if (b->order == COHO_SMILES_BOND_UNSPECIFIED)
		assert(b->stereo == COHO_SMILES_BOND_STEREO_UNSPECIFIED);

	rb = &x->ring_bonds[rnum];

	if (rb->atom0 == -1) {
		rb->atom0	= b->atom0;
		rb->order	= b->order;
		rb->stereo	= b->stereo;
		rb->is_implicit	= 0;
		rb->is_ring	= 1;
		rb->position	= b->position;
		rb->length	= b->length;
		x->open_ring_closures++;
		return 0;
	}

	/* Close the open bond */

	if (rb->atom0 == b->atom0) {
		x->error = strdup("Atom ring-bonded to itself");
		x->error_position = x->atoms[b->atom0].position;
		return -1;
	}

	if (rb->order == COHO_SMILES_BOND_UNSPECIFIED)
		rb->order = b->order;
	else if (b->order == COHO_SMILES_BOND_UNSPECIFIED)
		; /* pass */
	else if (rb->order != b->order) {
		x->error = strdup("conflicting ring bond orders");
		x->error_position = x->atoms[b->atom0].position;
		return -1;
	}
	if (rb->order == COHO_SMILES_BOND_UNSPECIFIED)
		rb->order = COHO_SMILES_BOND_SINGLE;

	rb->atom1 = b->atom0;

	if (add_bond(x, rb) == -1)
		return -1;

	coho_smiles_bond_init(rb);
	rb->atom0 = -1; ;		/* mark slot open again */
	x->open_ring_closures--;

	return 0;
}

/*
 * Matches an aliphatic organic atom (C, N, O, etc.).
 * Returns 1 on match, 0 if no match, or -1 on error.
 */
static int
aliphatic_organic(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;

	if (!match(x, &t, 0, ALIPHATIC_ORGANIC))
		return 0;
	coho_smiles_atom_init(a);
	a->position = t.position;
	a->atomic_number = t.intval;
	a->is_organic = 1;
	a->length = t.n;
	tokcpy(a->symbol, &t, sizeof(a->symbol));
	return 1;
}

/*
 * Matches an aromatic organic atom (c, n, o, etc.).
 * Returns 1 on match, 0 if no match, or -1 on error.
 */
static int
aromatic_organic(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;

	if (!match(x, &t, 0, AROMATIC_ORGANIC))
		return 0;
	coho_smiles_atom_init(a);
	a->position = t.position;
	a->atomic_number = t.intval;
	a->is_organic = 1;
	a->is_aromatic = 1;
	a->length = t.n;
	tokcpy(a->symbol, &t, sizeof(a->symbol));
	return 1;
}

/*
 * Assigns implicit hydrogen counts for all atoms that were
 * specified using the organic-subset shorthand.
 */
static int
assign_implicit_hydrogen_count(struct coho_smiles *x)
{
	int i, valence, std;
	struct coho_smiles_atom *a;

	for (i = 0; i < x->atom_count; i++) {
		a = &x->atoms[i];

		if (!a->is_organic)
			continue;

		valence = atom_valence(x, i);
		std = round_valence(a->atomic_number, valence, a->is_aromatic);

		if (std == -1)
			a->implicit_hydrogen_count = 0;
		else
			a->implicit_hydrogen_count = std - valence;
	}

	return 0;
}

/*
 * Matches an atom or returns 0 if not found.
 * If successful, stores the index of the new atom in *anum and returns 1.
 * On error, sets x->error and returns -1.
 *
 * atom ::= bracket_atom | aliphatic_organic | aromatic_organic | '*'
 */
static int
atom(struct coho_smiles *x, int *anum)
{
	struct coho_smiles_atom a;
	int rc;

	if ((rc = bracket_atom(x, &a)) ||
	    (rc = aliphatic_organic(x, &a)) ||
	    (rc = aromatic_organic(x, &a)) ||
	    (rc = wildcard(x, &a))) {
		if (rc == -1)
			return -1;
	} else
		return 0;

	*anum = add_atom(x, &a);
	return 1;
}

/*
 * Matches an atom followed by zero or more ringbonds.
 * On success, stores the index of the new atom in *anum and returns 1.
 * Returns 0 if there is no match.
 * On error, sets x->error and returns -1.
 */
static int
atom_ringbond(struct coho_smiles *x, int *anum)
{
	int rc;

	if ((rc = atom(x, anum))) {
		if (rc == -1 )
			return -1;
	} else {
		return 0;
	}

	while ((rc = ringbond(x, *anum)))
		if (rc == -1 )
			return -1;

	return 1;
}

/*
 * Computes the valence of an atom by summing the orders
 * of its bonds.
 * Treats aromatic atoms as a special case in an attempt to
 * properly derive implicit hydrogen count.
 */
static int
atom_valence(struct coho_smiles *x, size_t idx)
{
	int i;
	int valence, neighbors;
	struct coho_smiles_bond *b;

	valence = 0;
	neighbors = 0;

	for (i = 0; i < x->bond_count; i++) {
		b = &x->bonds[i];
		if (b->atom0 > (int)idx)
			break;
		else if (b->atom0 != (int)idx && b->atom1 != (int)idx)
			continue;

		if (b->order == COHO_SMILES_BOND_SINGLE)
			valence += 1;
		else if (b->order == COHO_SMILES_BOND_AROMATIC)
			valence += 1;
		else if (b->order == COHO_SMILES_BOND_DOUBLE)
			valence += 2;
		else if (b->order == COHO_SMILES_BOND_TRIPLE)
			valence += 3;
		else if (b->order == COHO_SMILES_BOND_QUAD)
			valence += 4;

		neighbors += 1;
	}

	if (x->atoms[idx].is_aromatic && valence == neighbors) {
		valence += 1;
	}

	return valence;
}

/*
 * Matches a bond or returns 0 if not found.
 * If found, sets fields of *b and returns 1.
 * Only sets fields that can be determined by the matching bond
 * token (order, stereo, position, and length).
 * Clears implicit flag.
 * Doesn't set bond atoms.
 *
 * bond ::= '-' | '=' | '#' | '$' | ':' | '/' | '\'
 */
static int
bond(struct coho_smiles *x, struct coho_smiles_bond *b)
{
	struct token t;

	if (!match(x, &t, 0, BOND))
		return 0;

	b->order = t.intval;
	b->stereo = t.flags;
	b->is_implicit = 0;
	b->position = t.position;
	b->length = t.n;
	return 1;
}

/*
 * Matches a bracket atom or returns 0 if not found.
 * If found, initializes the atom, sets its fields, and returns 1.
 * On error, sets x->error and returns -1.
 *
 * bracket_atom ::= '[' isotope? symbol chiral? hydrogen_count? charge? class? ']'
 */
static int
bracket_atom(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;

	if (!match(x, &t, 0, BRACKET_OPEN))
		return 0;

	coho_smiles_atom_init(a);
	a->is_bracket = 1;
	a->position = t.position;
	a->length = t.n;

	if (isotope(x, a) == -1)
		return -1;

	if (symbol(x, a) == 0) {
		x->error = strdup("atom symbol expected");
		return -1;
	}

	if (chirality(x, a) == -1)
		return -1;

	if (hydrogen_count(x, a) == -1)
		return -1;

	if (charge(x, a) == -1)
		return -1;

	if (atom_class(x, a) == -1)
		return -1;

	if (!match(x, &t, 0, BRACKET_CLOSE)) {
		x->error = strdup("bracket atom syntax error");
		return -1;
	}
	a->length += t.n;
	return 1;
}

/*
 * Returns 0 if all rings have been closed.
 * Otherwise, sets x->error and returns -1.
 */
static int
check_ring_closures(struct coho_smiles *x)
{
	size_t i;

	if (x->open_ring_closures == 0)
		return 0;

	x->error = strdup("unclosed ring bond");

	for (i = 0; i < 100; i++) {
		if (x->ring_bonds[i].atom0 != -1) {
			x->error_position = x->ring_bonds[i].position;
			break;
		}
	}

	return -1;
}

/*
 * Parses optional charge inside a bracket atom.
 * If successful, sets a->charge and increments a->length.
 * Returns 1 if charge was read, else 0.
 * On error, sets x->error and returns -1.
 *
 * charge ::=   '-'
 *            | '-' DIGIT? DIGIT
 *            | '+'
 *            | '+' DIGIT? DIGIT
 *            | '--' deprecated
 *            | '++' deprecated
 */
static int
charge(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;
	int sign;
	int n;
	int length;

	if (!match(x, &t, 1, PLUS|MINUS))
		return 0;
	sign = t.intval;
	length = t.n;

	if ((n = integer(x, 2, &a->charge)) == -1) {
		x->error = strdup("charge too large");
		return -1;
	} else if (n) {
		a->charge *= sign;
		length += n;
	} else {
		a->charge = sign;

		if (lex(x, &t, 1) & (PLUS|MINUS)) {
			if (t.intval == sign) {
				x->position += t.n;
				a->charge *= 2;
				length += t.n;
			}
		}
	}

	a->length += length;
	return 1;
}

/*
 * Parses chirality inside a bracket atom.
 * If successful, sets a->chirality and increments a->length.
 * Returns 1 if chirality was read, else 0.
 * TODO: Currently, this only understands @ and @@.
 */
static int
chirality(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;

	if (!match(x, &t, 1, CHIRALITY))
		return 0;
	tokcpy(a->chirality, &t, sizeof(a->chirality));
	a->length += t.n;
	return 1;
}

/*
 * Matches a closing parenthesis that ends a branch.
 * On success, pops the parenthesis stack and returns 1.
 * Returns 0 if there was no match.
 * On error, sets x->error and returns -1.
 */
static int
close_paren(struct coho_smiles *x, struct coho_smiles_bond *b)
{
	struct token t;

	if (!match(x, &t, 0, PAREN_CLOSE))
		return 0;

	if (pop_paren_stack(x, t.position, b))
		return -1;
	return 1;
}

/*
 * Matches dot, the no-bond specifier.
 * Returns 1 on success, 0 if there was no match.
 */
static int
dot(struct coho_smiles *x)
{
	struct token t;

	return match(x, &t, 0, DOT);
}

static int
ensure_array_capacities(struct coho_smiles *x, size_t smiles_length)
{
	size_t new_capacity;
	void *p;

	/*
	 * Maximum required storage is bounded by length of SMILES string.
	 */
	if (x->atoms_capacity >= smiles_length)
		return 0;

	new_capacity = next_array_capacity(smiles_length);

#define GROW(name)						\
	do {							\
		p = reallocarray(x->name,			\
				 new_capacity,			\
				 sizeof(x->name[0]));		\
		if (p == NULL)					\
			return -1;				\
		x->name = p;					\
		x->name##_capacity = new_capacity;			\
	} while (0)

	GROW(atoms);
	GROW(bonds);
	GROW(paren_stack);

#undef GROW
	return 0;
}
/*
 * Parses hydrogen count inside a bracket atom.
 * If successful, sets a->hydrogen_count and increments a->length.
 * Returns 1 if hydrogen_count was read, else 0.
 *
 * hydrogen_count ::= 'H' | 'H' DIGIT
 */
static int
hydrogen_count(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;

	if (!match(x, &t, 1, HYDROGEN))
		return 0;

	a->length += t.n;

	if (match(x, &t, 1, DIGIT)) {
		a->hydrogen_count = t.intval;
		a->length += t.n;
	} else {
		a->hydrogen_count = 1;
	}

	return 1;
}

/*
 * Matches an integer up to maxdigit long.
 * On success, stores the integer in *dst and returns number of digits.
 * Returns 0 if no digits are available.
 * Returns -1 if maxdigit is exceeded.
 */
static int
integer(struct coho_smiles *x, size_t maxdigit, int *dst)
{
	size_t i;
	int n = 0;
	int saved = x->position;
	struct token t;

	for (i = 0; lex(x, &t, 0) & DIGIT; i++) {
		if (maxdigit && i == maxdigit) {
			x->position = saved;
			return -1;
		}
		x->position += t.n;
		n = n * 10 + t.intval;
	}
	if (i == 0)
		return 0;
	*dst = n;
	return i;
}

/*
 * Parses isotope inside a bracket atom.
 * If successful, sets a->isotope and increments a->length.
 * Returns 1 if isotope was read, else 0.
 * On error, returns -1 and sets x->error.
 */
static int
isotope(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	int n;

	if ((n = integer(x, 5, &a->isotope)) == -1) {
		x->error = strdup("isotope too large");
		return -1;
	}
	a->length += n;
	return 0;
}

/*
 * Reads next token and checks if its type is among those requested.
 * If so, consumes the token and returns 1.
 * If not, returns 0 and the parsing position remains unchanged.
 */
static int
match(struct coho_smiles *x, struct token *t, int inbracket, unsigned int ttype)
{
	if (lex(x, t, inbracket) & ttype) {
		x->position += t->n;
		return 1;
	}
	return 0;
}

/*
 * Returns a new array capacity that is larger than
 * its previous capacity.
 */
static size_t
next_array_capacity(size_t previous_capacity)
{
	size_t cap = 2 * previous_capacity - 1;

	while (cap & (cap - 1))
		cap = cap & (cap - 1);
	return cap;
}

/*
 * Matches an opening parenthesis that begins a branch.
 * On success, pushes the parenthesis stack and returns 1.
 * Returns 0 if there was no match.
 */
static int
open_paren(struct coho_smiles *x, struct coho_smiles_bond *b)
{
	struct token t;

	if (!match(x, &t, 0, PAREN_OPEN))
		return 0;

	push_paren_stack(x, t.position, b);
	return 1;
}

/*
 * Pops the parenthesis stack that holds the open bonds to
 * "previous" atoms.
 * Ex: In C(N)=O the closing parenthesis will trigger the popping of
 * the stack, ensuring that the oxygen is bonded to the carbon instead
 * of the nitrogen.
 * The position of the parenthesis triggering the pop is used for
 * error messages.
 * Returns 0 on success.
 * On failure, sets x->error and returns -1.
 */
static int
pop_paren_stack(struct coho_smiles *x, int position, struct coho_smiles_bond *b)
{
	if (!x->paren_stack_count) {
		x->error = strdup("unbalanced parenthesis");
		x->error_position = position;
		return -1;
	}

	*b = x->paren_stack[--x->paren_stack_count].bond;
	return 0;
}

/*
 * Pushes the parenthesis stack that holds open bonds to
 * "previous" atoms.
 * Ex: In C(N)=O the first parenthesis will trigger the pushing of
 * an open bond to the carbon onto the stack.
 * The closing parenthesis will pop the stack, ensuring that the carbon
 * is correctly bonded to the oxygen.
 * The position of the parenthesis triggering the push is stored
 * to support error messages.
 */
static void
push_paren_stack(struct coho_smiles *x, int position, struct coho_smiles_bond *b)
{
	struct coho_smiles_paren *p;

	assert(b->atom0 != -1);

	p = &x->paren_stack[x->paren_stack_count++];
	p->position = position;
	p->bond = *b;
}

/*
 * Matches a ring bond or returns 0 if not found.
 * On error, sets x->error and returns -1.
 * On success, uses atom anum to open or close a ring
 * bond and then returns 1.
 * If the parsed ring bond ID is in use, closes it and adds a new bond
 * to the bond list.
 * Otherwise, marks the ring ID as open.
 *
 * ringbond ::= bond? DIGIT | bond? '%' DIGIT DIGIT
 */
static int
ringbond(struct coho_smiles *x, int anum)
{
	struct token t;
	struct coho_smiles_bond b;
	int rc;
	int rnum;
	int saved = x->position;

	coho_smiles_bond_init(&b);
	b.atom0 = anum;

	if ((rc = bond(x, &b))) {
		if (rc == -1)
			return -1;
	} else {
		b.order = COHO_SMILES_BOND_UNSPECIFIED;
		b.position = x->position;
	}

	if (!match(x, &t, 0, PERCENT|DIGIT)) {
		x->position = saved;
		return 0;
	}

	if (t.type == PERCENT) {
		if (!match(x, &t, 0, DIGIT)) {
			x->error = strdup("ring bond expected");
			return -1;
		}
		rnum = t.intval * 10;

		if (!match(x, &t, 0, DIGIT)) {
			x->error = strdup("2 digit ring bond expected");
			return -1;
		}
		rnum += t.intval;
	} else {
		rnum = t.intval;
	}

	if (add_ringbond(x, rnum, &b))
		return -1;
	return 1;
}

/*
 * Rounds an atom's current valence to its next standard one.
 * Returns its current valence if it among the standard ones.
 * Otherwise, returns the next higher standard one or -1 if
 * none are found.
 * Setting lowest_only to true causes the search to stop after
 * the first standard valence, disregarding higher valences.
 */
static int
round_valence(int atomic_number, int valence, int lowest_only)
{
	int i, j, anum;

	for (i = 0; (anum = standard_valences[i][0]) != -1; i++) {
		if (anum > atomic_number)
			break;
		else if (anum == atomic_number) {
			for (j = 1; j < 4; j++) {
				if (valence <= standard_valences[i][j])
					return standard_valences[i][j];
				if (lowest_only)
					break;
			}
		}
	}
	return -1;
}

/*
 * Initializes struct coho_smiles_atom.
 */
static void
coho_smiles_atom_init(struct coho_smiles_atom *x)
{
	x->atomic_number = 0;
	x->symbol[0] = '\0';
	x->isotope = -1;
	x->charge = 0;
	x->hydrogen_count = -1;
	x->implicit_hydrogen_count = -1;
	x->is_bracket = 0;
	x->is_organic = 0;
	x->is_aromatic = 0;
	x->chirality[0] = '\0';
	x->atom_class = -1;
	x->position = -1;
	x->length = 0;
}

/*
 * Initializes struct coho_smiles_bond.
 */
static void
coho_smiles_bond_init(struct coho_smiles_bond *x)
{
	x->atom0 = -1;
	x->atom1 = -1;
	x->order = -1;
	x->stereo = COHO_SMILES_BOND_STEREO_UNSPECIFIED;
	x->is_implicit = 0;
	x->is_ring = 0;
	x->position = -1;
	x->length = 0;
}

/*
 * Reinitializes struct coho_smiles prior to parsing a new SMILES.
 * The given number of bytes of smiles will be parsed.
 */
static void
coho_smiles_reinit(struct coho_smiles *x, const char *smiles, size_t end)
{
	size_t i;

	x->smiles = smiles;
	x->position = 0;
	x->end = end;
	free(x->error);
	x->error = NULL;
	x->error_position = -1;
	x->atom_count = 0;
	x->bond_count = 0;
	x->paren_stack_count = 0;

	for (i = 0; i < 100; i++)
		coho_smiles_bond_init(&x->ring_bonds[i]);
	x->open_ring_closures = 0;
}

/*
 * Parses atom symbol inside a bracket atom.
 * If successful, sets a->symbol, a->is_aromatic, and increments a->length.
 * Returns 1 if symbol was read, else 0.
 *
 * symbol ::= element_symbols | aromatic_symbols | '*'
 */
static int
symbol(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;

	if (!match(x, &t, 1, ELEMENT|AROMATIC|WILDCARD))
		return 0;
	a->atomic_number = t.intval;
	a->is_aromatic = t.type & AROMATIC ? 1 : 0;
	a->length += t.n;
	tokcpy(a->symbol, &t, sizeof(a->symbol));
	return 1;
}

/*
 * Copies up to dstsz - 1 bytes from the token to dst, NUL-terminating
 * dst if dstsz is not 0.
 */
static void
tokcpy(char *dst, struct token *t, size_t dstsz)
{
	size_t i;

	if (dstsz == 0)
		return;

	for (i = 0; i < t->n; i++) {
		if (i == dstsz - 1)
			break;
		dst[i] = t->s[i];
	}

	dst[i] = 0;
}

/*
 * Matches a wildcard atom (*) or returns 0 if not found.
 * If found, initializes the atom, sets its fields, and returns 1.
 * On error, sets x->error and returns -1.
 */
static int
wildcard(struct coho_smiles *x, struct coho_smiles_atom *a)
{
	struct token t;

	if (!match(x, &t, 0, WILDCARD))
		return 0;
	coho_smiles_atom_init(a);
	a->position = t.position;
	a->atomic_number = 0;
	a->length = t.n;
	tokcpy(a->symbol, &t, sizeof(a->symbol));
	return 1;
}

/*
 * Reads next token from SMILES string.
 * The inbracket parameter should be set to true when parsing is
 * inside a bracket atom.
 * Returns the token type or zero if no token could be read.
 * The token type is a bitmask since a particular token can belong
 * to multiple categories.  For example, the symbol for
 * hydrogen will have type ELEMENT|HYDROGEN.
 */
static unsigned int
lex(struct coho_smiles *x, struct token *t, int inbracket)
{
	int c0, c1;
	const char *s;

	if (x->position == x->end)
		return 0;

	s = x->smiles + x->position;
	c0 = s[0];
	c1 = 0;

	if (x->position < x->end)
		c1 = s[1];

	t->s = s;
	t->position = x->position;
	t->n = 1;
	t->type = 0;
	t->intval = -1;
	t->flags = 0;

	switch (c0) {
	case 'a':
		if (inbracket && c1 == 's') {
			t->n = 2;
			t->type = AROMATIC;
			t->intval = 33;
			goto out;
		}
		return 0;
	case 'b':
		t->type = inbracket ? AROMATIC : AROMATIC_ORGANIC;
		t->intval = 5;
		goto out;
	case 'c':
		t->type = inbracket ? AROMATIC : AROMATIC_ORGANIC;
		t->intval = 6;
		goto out;
	case 'n':
		t->type = inbracket ? AROMATIC : AROMATIC_ORGANIC;
		t->intval = 7;
		goto out;
	case 'o':
		t->type = inbracket ? AROMATIC : AROMATIC_ORGANIC;
		t->intval = 8;
		goto out;
	case 'p':
		t->type = inbracket ? AROMATIC : AROMATIC_ORGANIC;
		t->intval = 15;
		goto out;
	case 's':
		if (!inbracket) {
			t->type = AROMATIC_ORGANIC;
			t->intval = 16;
			goto out;
		}
		switch (c1) {
		case 'e':
			t->type = AROMATIC;
			t->n = 2;
			t->intval = 34;
			goto out;
		default:
			t->type = AROMATIC;
			t->intval = 16;
			goto out;
		}
	case 'A':
		switch (c1) {
		case 'c':
			t->type = ELEMENT;
			t->intval = 89;
			t->n = 2;
			goto out;
		case 'g':
			t->type = ELEMENT;
			t->intval = 47;
			t->n = 2;
			goto out;
		case 'l':
			t->type = ELEMENT;
			t->intval = 13;
			t->n = 2;
			goto out;
		case 'm':
			t->type = ELEMENT;
			t->intval = 95;
			t->n = 2;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->intval = 18;
			t->n = 2;
			goto out;
		case 's':
			t->type = ELEMENT;
			t->intval = 33;
			t->n = 2;
			goto out;
		case 't':
			t->type = ELEMENT;
			t->intval = 85;
			t->n = 2;
			goto out;
		case 'u':
			t->type = ELEMENT;
			t->intval = 79;
			t->n = 2;
			goto out;
		default:
			return 0;
		}
	case 'B':
		if (!inbracket) {
			if (c1 == 'r') {
				t->intval = 35;
				t->n = 2;
			} else {
				t->intval = 5;
			}
			t->type = ALIPHATIC_ORGANIC;
			goto out;
		}
		switch (c1) {
		case 'a':
			t->type = ELEMENT;
			t->intval = 56;
			t->n = 2;
			goto out;
		case 'e':
			t->type = ELEMENT;
			t->intval = 4;
			t->n = 2;
			goto out;
		case 'h':
			t->type = ELEMENT;
			t->intval = 107;
			t->n = 2;
			goto out;
		case 'i':
			t->type = ELEMENT;
			t->intval = 83;
			t->n = 2;
			goto out;
		case 'k':
			t->type = ELEMENT;
			t->intval = 97;
			t->n = 2;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->intval = 35;
			t->n = 2;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 5;
			goto out;
		}
	case 'C':
		if (!inbracket) {
			if (c1 == 'l') {
				t->intval = 17;
				t->n = 2;
			} else {
				t->intval = 6;
			}
			t->type = ALIPHATIC_ORGANIC;
			goto out;
		}
		switch (c1) {
		case 'a':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 20;
			goto out;
		case 'd':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 20;
			goto out;
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 58;
			goto out;
		case 'f':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 98;
			goto out;
		case 'l':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 17;
			goto out;
		case 'm':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 96;
			goto out;
		case 'n':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 112;
			goto out;
		case 'o':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 27;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 24;
			goto out;
		case 's':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 55;
			goto out;
		case 'u':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 29;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 6;
			goto out;
		}
	case 'D':
		switch (c1) {
		case 'b':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 105;
			goto out;
		case 's':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 110;
			goto out;
		case 'y':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 66;
			goto out;
		default:
			return 0;
		}
	case 'E':
		switch (c1) {
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 68;
			goto out;
		case 's':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 99;
			goto out;
		case 'u':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 63;
			goto out;
		default:
			return 0;
		}
	case 'F':
		if (!inbracket) {
			t->intval = 9;
			t->type = ALIPHATIC_ORGANIC;
			goto out;
		}
		switch (c1) {
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 26;
			goto out;
		case 'l':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 114;
			goto out;
		case 'm':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 100;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 87;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 9;
			goto out;
		}
	case 'G':
		switch (c1) {
		case 'a':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 31;
			goto out;
		case 'd':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 64;
			goto out;
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 32;
			goto out;
		default:
			return 0;
		}
	case 'H':
		switch (c1) {
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 2;
			goto out;
		case 'f':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 72;
			goto out;
		case 'g':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 80;
			goto out;
		case 'o':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 67;
			goto out;
		case 's':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 108;
			goto out;
		default:
			t->type = ELEMENT | HYDROGEN;
			t->intval = 1;
			goto out;
		}
	case 'I':
		if (!inbracket) {
			t->intval = 53;
			t->type = ALIPHATIC_ORGANIC;
			goto out;
		}
		switch (c1) {
		case 'n':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 49;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 77;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 53;
			goto out;
		}
	case 'K':
		switch (c1) {
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 36;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 19;
			goto out;
		}
	case 'L':
		switch (c1) {
		case 'a':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 57;
			goto out;
		case 'i':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 3;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 103;
			goto out;
		case 'u':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 71;
			goto out;
		case 'v':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 116;
			goto out;
		default:
			return 0;
		}
	case 'M':
		switch (c1) {
		case 'd':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 101;
			goto out;
		case 'g':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 12;
			goto out;
		case 'n':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 25;
			goto out;
		case 'o':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 42;
			goto out;
		case 't':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 109;
			goto out;
		default:
			return 0;
		}
	case 'N':
		if (!inbracket) {
			t->intval = 7;
			t->type = ALIPHATIC_ORGANIC;
			goto out;
		}
		switch (c1) {
		case 'a':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 11;
			goto out;
		case 'b':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 41;
			goto out;
		case 'd':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 101;
			goto out;
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 10;
			goto out;
		case 'i':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 28;
			goto out;
		case 'o':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 102;
			goto out;
		case 'p':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 93;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 7;
			goto out;
		}
	case 'O':
		if (!inbracket) {
			t->intval = 8;
			t->type = ALIPHATIC_ORGANIC;
			goto out;
		}
		switch (c1) {
		case 's':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 76;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 8;
			goto out;
		}
	case 'P':
		if (!inbracket) {
			t->intval = 15;
			t->type = ALIPHATIC_ORGANIC;
			goto out;
		}
		switch (c1) {
		case 'a':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 91;
			goto out;
		case 'b':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 82;
			goto out;
		case 'd':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 46;
			goto out;
		case 'm':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 61;
			goto out;
		case 'o':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 84;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 59;
			goto out;
		case 't':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 78;
			goto out;
		case 'u':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 94;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 15;
			goto out;
		}
	case 'R':
		switch (c1) {
		case 'a':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 88;
			goto out;
		case 'b':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 37;
			goto out;
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 75;
			goto out;
		case 'f':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 104;
			goto out;
		case 'g':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 111;
			goto out;
		case 'h':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 45;
			goto out;
		case 'n':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 86;
			goto out;
		case 'u':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 44;
			goto out;
		default:
			return 0;
		}
	case 'S':
		if (!inbracket) {
			t->intval = 16;
			t->type = ALIPHATIC_ORGANIC;
			goto out;
		}
		switch (c1) {
		case 'b':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 51;
			goto out;
		case 'c':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 21;
			goto out;
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 34;
			goto out;
		case 'g':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 106;
			goto out;
		case 'i':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 14;
			goto out;
		case 'm':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 62;
			goto out;
		case 'n':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 50;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 38;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 16;
			goto out;
		}
	case 'T':
		switch (c1) {
		case 'a':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 73;
			goto out;
		case 'b':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 65;
			goto out;
		case 'c':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 43;
			goto out;
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 52;
			goto out;
		case 'h':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 90;
			goto out;
		case 'i':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 22;
			goto out;
		case 'l':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 81;
			goto out;
		case 'm':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 69;
			goto out;
		default:
			return 0;
		}
	case 'U':
		t->type = ELEMENT;
		t->intval = 92;
		goto out;
	case 'V':
		t->type = ELEMENT;
		t->intval = 23;
		goto out;
	case 'W':
		t->type = ELEMENT;
		t->intval = 74;
		goto out;
	case 'X':
		switch (c1) {
		case 'e':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 54;
			goto out;
		default:
			return 0;
		}
	case 'Y':
		switch (c1) {
		case 'b':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 70;
			goto out;
		default:
			t->type = ELEMENT;
			t->intval = 39;
			goto out;
		}
	case 'Z':
		switch (c1) {
		case 'n':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 30;
			goto out;
		case 'r':
			t->type = ELEMENT;
			t->n = 2;
			t->intval = 40;
			goto out;
		default:
			return 0;
		}
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		t->type = DIGIT;
		t->intval = c0 - '0';
		goto out;
	case '*':
		t->type = WILDCARD;
		t->intval = 0;
		goto out;
	case '[':
		t->type = BRACKET_OPEN;
		goto out;
	case ']':
		t->type = BRACKET_CLOSE;
		goto out;
	case '(':
		t->type = PAREN_OPEN;
		goto out;
	case ')':
		t->type = PAREN_CLOSE;
		goto out;
	case '+':
		t->type = PLUS;
		t->intval = 1;
		goto out;
	case '-':
		t->type   = inbracket ? MINUS : BOND;
		t->intval = inbracket ? -1    : COHO_SMILES_BOND_SINGLE;
		goto out;
	case '%':
		t->type = PERCENT;
		goto out;
	case '=':
		t->type = BOND;
		t->intval = COHO_SMILES_BOND_DOUBLE;
		goto out;
	case '#':
		t->type = BOND;
		t->intval = COHO_SMILES_BOND_TRIPLE;
		goto out;
	case '$':
		t->type = BOND;
		t->intval = COHO_SMILES_BOND_QUAD;
		goto out;
	case ':':
		if (inbracket) {
			t->type = COLON;
		} else {
			t->type = BOND;
			t->intval = COHO_SMILES_BOND_AROMATIC;
		}
		goto out;
	case '/':
		t->type = BOND;
		t->intval = COHO_SMILES_BOND_SINGLE;
		t->flags = COHO_SMILES_BOND_STEREO_UP;
		goto out;
	case '\\':
		t->type = BOND;
		t->intval = COHO_SMILES_BOND_SINGLE;
		t->flags = COHO_SMILES_BOND_STEREO_DOWN;
		goto out;
	case '.':
		t->type = DOT;
		goto out;
	case '@':
		t->type = CHIRALITY;
		if (c1 == '@')
			t->n = 2;
		goto out;
	default:
		return 0;
	}

out:
	return t->type;
}
