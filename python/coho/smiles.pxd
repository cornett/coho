# Copyright (c) 2017-2018 Ben Cornett <ben@lantern.is>
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

cdef extern from "smiles.h":

    enum:
        SMILES_BOND_SINGLE
        SMILES_BOND_DOUBLE
        SMILES_BOND_TRIPLE
        SMILES_BOND_QUAD
        SMILES_BOND_AROMATIC

    enum:
        SMILES_BOND_STEREO_UP
        SMILES_BOND_STEREO_DOWN

    struct smiles_atom:
        int              atomic_number
        char             symbol[4]
        int              isotope
        int              charge
        int              hcount
        int              implicit_hcount
        int              bracket
        int              organic
        int              aromatic
        char             chirality[8]
        int              aclass
        int              pos
        int              len

    struct smiles_bond:
        int              a0
        int              a1
        int              order
        int              implicit
        int              ring
        int              stereo
        int              pos
        int              len

    struct smiles:
        smiles_atom     *atoms
        size_t           atoms_sz
        smiles_bond     *bonds
        size_t           bonds_sz
        char            *err
        int              errpos

    void smiles_free(smiles *)
    int smiles_init(smiles *)
    int smiles_parse(smiles *, const char *, size_t)
