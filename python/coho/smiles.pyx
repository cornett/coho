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

from coho cimport COHO_OK

BOND_SINGLE         = COHO_SMILES_BOND_SINGLE
BOND_DOUBLE         = COHO_SMILES_BOND_DOUBLE
BOND_TRIPLE         = COHO_SMILES_BOND_TRIPLE
BOND_QUAD           = COHO_SMILES_BOND_QUAD
BOND_AROMATIC       = COHO_SMILES_BOND_AROMATIC

BOND_STEREO_UP      = COHO_SMILES_BOND_STEREO_UP
BOND_STEREO_DOWN    = COHO_SMILES_BOND_STEREO_DOWN


cdef class Parser:
    """Parses SMILES"""
    cdef coho_smiles _x

    def __cinit__(self):
        coho_smiles_init(&self._x)

    def __dealloc__(self):
        coho_smiles_free(&self._x)

    def error(self):
        """Error message if last parse failed"""
        if self._x.error != NULL:
            return self._x.error.decode()

    error = property(error, doc=error.__doc__)

    def error_position(self):
        """Error position if last parse failed"""
        if self._x.error != NULL:
            return self._x.error_position

    error_position = property(error_position, doc=error_position.__doc__)

    def parse(self, str smi):
        """Parse SMILES string."""
        cdef bytes s = smi.encode()
        if coho_smiles_parse(&self._x, s, len(s)) != COHO_OK:
            lead = "-" * self.error_position
            msg = f"{self.error}\n{smi}\n{lead}^\n"
            x = ValueError(msg)
            raise x

    def atoms(self):
        """Atom information produced by the last parse."""
        if self._x.error != NULL:
            return None
        x = []
        def noneif(x, mark):
            return None if x == mark else x
        for i in range(self._x.atoms_sz):
            a = self._x.atoms[i]
            x.append({
                "atomic_number": a.atomic_number,
                "symbol": a.symbol.decode(),
                "charge": a.charge,
                "hydrogen_count": noneif(a.hydrogen_count, -1),
                "implicit_hydrogen_count": noneif(a.implicit_hydrogen_count, -1),
                "isotope": noneif(a.isotope, -1),
                "chirality": a.chirality.decode() or None,
                "is_bracket": bool(a.is_bracket),
                "is_organic": bool(a.is_organic),
                "is_aromatic": bool(a.is_aromatic),
                "atom_class": noneif(a.atom_class, -1),
                "position": a.position,
                "length": a.length,
            })
        return x

    atoms = property(atoms, doc=atoms.__doc__)

    def bonds(self):
        """Bond information produced by the last parse."""
        if self._x.error != NULL:
            return None
        x = []
        for i in range(self._x.bonds_sz):
            b = self._x.bonds[i]
            x.append({
                "atom0": b.atom0,
                "atom1": b.atom1,
                "order": b.order,
                "stereo": b.stereo,
                "is_implicit": bool(b.is_implicit),
                "is_ring": bool(b.is_ring),
                "position": b.position if b.position >= 0 else None,
                "length": b.length,
            })
        return x

    bonds = property(bonds, doc=bonds.__doc__)
