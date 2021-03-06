Python API
==========

The Coho Python API is produced using `Cython <http://cython.org/>`_
for Python 3.5+.

The API is contained under a top-level package called ``coho``.


coho.smiles
-----------

.. :py:module:: coho.smiles

The ``coho.smiles`` module contains a parser for the
`OpenSMILES <http://opensmiles.org/>`_ language.

.. class:: Parser

    Create a SMILES parser.

    The :meth:`parse` method can be called repeatedly to process
    multiple SMILES strings.
    The results of each call are stored in the :attr:`atoms` and
    :attr:`bonds` attributes.
    Each call to :meth:`parse` overwrites the previous
    values of these attributes.

    Note that the data structures described below are intended to
    represent parsed SMILES strings, not molecules.
    Conformance of a particular string to the SMILES grammar does
    not imply description of a chemically-meaningful structure.

    .. method:: parse(smiles: str)

        Parses a SMILES string.

        :param str smiles: SMILES string

        If parsing SMILES fails, a :class:`ValueError` is raised.

    .. attribute:: error

        If :meth:`parse()` fails, ``error``
        will contain an error message, otherwise it will be ``None``.

    .. attribute:: error_position

        If :meth:`parse()` fails, ``error_position`` will contain the offset
        into the SMILES string where the
        error was detected, otherwise it will be ``None``.

    .. attribute:: atoms

        Returns a list of parsed atoms.
        Each atom is represented as a dictionary with the following keys.

        ``atomic_number``
            The atom's atomic number, deduced from the symbol.
            The wildcard atom is assigned an atomic number of zero.

        ``symbol``
            Element symbol as it appears in the SMILES string.
            Atoms designated as aromatic will have lowercase symbols.

        ``isotope``
            An integer isotope value, or ``None`` if none was specified.
            Note that the `OpenSMILES <http://opensmiles.org/>`_ specification
            states that zero is a valid isotope and that
            ``[0S]`` is not the same as ``[S]``.

        ``charge``
            Formal charge, or 0 if none was specified.

        ``hydrogen_count``
            Number of explicit hydrogens, or ``None`` if the hydrogen
            count was not specified.

        ``implicit_hydrogen_count``
            Number of implicit hydrogens required to bring the atom to its
            next standard valence state.
            Set to ``None`` for atoms that were not specified using the organic
            subset nomenclature.

        ``is_bracket``
            True if the atom was specified using bracket(``[]``) notation,
            else False.

        ``is_organic``
            True if the atom was specified using the
            organic subset nomenclature, else False.

            Wildcard atoms are not considered part of the organic subset.
            If they occur outside of a bracket, their ``is_bracket``
            and ``is_organic`` fields will both be False.

        ``is_aromatic``
            True if the atom's symbol is lowercase, indicating that it is
            aromatic, else False.

        ``chirality``
            The chirality label, if provided, else ``None``.
            Currently, parsing is limited to ``@`` and ``@@``.
            Use of other chirality designators will result in a parsing error.

        ``atom_class``
            Integer atom class if specified, else ``None``.

        ``position``
            Offset of the atom's token in the SMILES string.

        ``length``
            Length of the atom's token.


    .. attribute:: bonds

        Returns a list of parsed bonds.
        Each bond is represented as a dictionary with the following keys.

        ``atom0``
            The atom number (position in :attr:`atoms` list)
            of the first member of the bond pair.

        ``atom1``
            The atom number (position in :attr:`atoms` list)
            of the second member of the bond pair.

        ``order``
            Bond order, with values from the following list:

            * BOND_SINGLE
            * BOND_DOUBLE
            * BOND_TRIPLE
            * BOND_QUAD
            * BOND_AROMATIC

        ``stereo``
            Used to indicate the cis/trans configuration of atoms
            around double bonds.
            Takes values from the following enumeration:

            ``BOND_STEREO_UNSPECIFIED``
                Bond has no stereochemistry
            ``BOND_STEREO_UP``
                lies "up" from ``atom0``
            ``BOND_STEREO_DOWN``
                lies "down" from ``atom1``

        ``is_implicit``
            True if bond was produced implicitly by the presence of two
            adjacent atoms without an intervening bond symbol, else False.
            Implicit bonds do not have a token position or length.
            An aromatic bond is implied by two adjacent aromatic atoms,
            otherwise implicit bonds are single.

        ``is_ring``
            True if the bond was produced using the ring bond nomenclature,
            else False.
            This does not imply anything about the number of rings
            in the molecule described by the SMILES string.

        ``position``
            Offset of the bond's token in the SMILES string, or ``None``
            if the bond is implicit.

        ``length``
            Length of the bond's token, or zero if implicit.

Example
^^^^^^^

The following example shows how to parse a SMILES string::

    import coho.smiles
    from pprint import pprint

    p = coho.smiles.Parser()
    try:
        p.parse('CNCC')
    except ValueError as e:
        print(e)
    else:
        pprint(p.atoms)
        print()
        pprint(p.bonds)
