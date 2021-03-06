# Copyright (c) 2017-2019 Ben Cornett <ben@lantern.is>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

cdef extern from "coho.h":

    enum:
        COHO_OK
        COHO_ERROR
        COHO_NOMEM

    enum:
        COHO_SMILES_BOND_SINGLE
        COHO_SMILES_BOND_DOUBLE
        COHO_SMILES_BOND_TRIPLE
        COHO_SMILES_BOND_QUAD
        COHO_SMILES_BOND_AROMATIC

    enum:
        COHO_SMILES_BOND_STEREO_UP
        COHO_SMILES_BOND_STEREO_DOWN

    struct coho_smiles_atom:
        int atomic_number
        char symbol[4]
        int isotope
        int charge
        int hydrogen_count
        int implicit_hydrogen_count
        int is_bracket
        int is_organic
        int is_aromatic
        char chirality[8]
        int atom_class
        int position
        int length

    struct coho_smiles_bond:
        int atom0
        int atom1
        int order
        int is_implicit
        int is_ring
        int stereo
        int position
        int length

    struct coho_smiles:
        int atom_count
        int bond_count
        coho_smiles_atom *atoms
        coho_smiles_bond *bonds
        char error[32]
        int error_position

    void coho_smiles_free(coho_smiles *)
    int coho_smiles_init(coho_smiles *)
    int coho_smiles_read(coho_smiles *, const char *, size_t)
