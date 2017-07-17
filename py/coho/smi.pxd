# Copyright (c) 2017 Ben Cornett <ben@lantern.is>
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

cdef extern from "smi.h":

    enum:
        SMI_BOND_SINGLE
        SMI_BOND_DOUBLE
        SMI_BOND_TRIPLE
        SMI_BOND_QUAD
        SMI_BOND_AROMATIC

    enum:
        SMI_BOND_STEREO_UP
        SMI_BOND_STEREO_DOWN

    struct smi_atom:
        int              atomic_number
        char             symbol[4]
        int              charge
        int              hcount
        int              isotope
        char             chirality[8]
        int              bracket
        int              organic
        int              aromatic
        int              aclass
        int              pos
        int              len

    struct smi_bond:
        int              a0
        int              a1
        int              order
        int              implicit
        int              ring
        int              stereo
        int              pos
        int              len

    struct smi:
        smi_atom        *atoms
        size_t           atoms_sz
        smi_bond        *bonds
        size_t           bonds_sz
        char            *err
        int              errpos

    void smi_free(smi *)
    int smi_init(smi *)
    int smi_parse(smi *, const char *, size_t)
