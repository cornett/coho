BOND_SINGLE         : int
BOND_DOUBLE         : int
BOND_TRIPLE         : int
BOND_QUAD           : int
BOND_AROMATIC       : int

BOND_STEREO_UP      : int
BOND_STEREO_DOWN    : int


class Parser:
    def parse(self, smiles: str): ...
