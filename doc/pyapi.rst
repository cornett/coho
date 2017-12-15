Python API
==========

The Coho Python API is produced using `Cython <http://cython.org/>`_ and
works with recent releases of Python 3.

The API is contained under a top-level package called ``coho``.


coho.smi
--------

.. :py:module:: coho.smi

The ``coho.smi`` module contains a parser for the
`OpenSMILES <http://opensmiles.org/>`_ language.

.. class:: Parser

    Create a SMILES parser.

    The :meth:`parse` method can be called repeatedly to process
    multiple SMILES strings.
    The results of each call are stored in the :attr:`atoms` and
    :attr:`bonds` attributes.

    Note that the data structures described below are intended to
    represent parsed SMILES strings, not molecules.
    Conformance of a particular string to the SMILES grammar does
    not imply description of a chemically-meaningful structure.

    .. method:: parse(smiles)

        Parse a SMILES string.

        :param str smiles: SMILES string

        If parsing SMILES fails, a :class:`ValueError` is raised.

    .. attribute:: err

        If :meth:`parse()` fails, ``err``
        will be an error message, otherwise it will be ``None``.

    .. attribute:: errpos

        If :meth:`parse()` fails, ``errpos`` will contain the offset
        into the SMILES string where the
        error was detected, otherwise it will be ``None``.

    .. attribute:: atoms

        Return a list of parsed atoms.
        Each atom is represented as a dictionary with the following keys.

        ``atomic_number``
            The atom's atomic number, deduced from the symbol.
            The wildcard atom is assigned an atomic number of zero.

        ``symbol``
            Element symbol as it appears in the SMILES string.
            Atoms designated as aromatic will have lowercase symbols.

        ``isotope``
            Isotope, or ``None`` if unspecified.
            Note that the `OpenSMILES <http://opensmiles.org/>`_ specification
            states that zero is a valid isotope and that
            ``[0S]`` is not the same as ``[S]``.

        ``charge``
            Formal charge, or 0 if none was specified.

        ``hcount``
            Number of explicit hydrogens, or -1 if none were specified.

        ``bracket``
            True if the atom was specified using bracket(``[]``) notation,
            else False.

        ``organic``
            True if the atom was specified using the
            organic subset nomenclature, else False.
            Wildcard atoms are not considered part of the organic subset.
            If they occur outside of a bracket, their ``bracket``
            and ``organic`` fields will both be False.

        ``aromatic``
            True if the atom's symbol is lowercase, indicating that it is
            aromatic, else False.

        ``chirality``
            The chirality label, if provided, else ``None``.
            Currently, parsing is limited to ``@`` and ``@@``.
            Use of other chirality designators will result in a parsing error.

        ``aclass``
            Integer atom class if specified, else ``None``.

        ``pos``
            Offset of the atom's token in the SMILES string.

        ``len``
            Length of the atom's token.


    .. attribute:: bonds

        Return a list of parsed bonds.
        Each bond is represented as a dictionary with the following keys.

        ``a0``
            The atom number (position in :attr:`atoms` list)
            of the first member of the bond pair.

        ``a1``
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
                lies "up" from ``a0``
            ``BOND_STEREO_DOWN``
                lies "down" from ``a1``

        ``implicit``
            True if bond was produced implicitly by the presence of two
            adjacent atoms without an intervening bond symbol, else False.
            Implicit bonds do not have a token position or length.
            An aromatic bond is implied by two adjacent aromatic atoms,
            otherwise implicit bonds are single.

        ``ring``
            True if the bond was produced using the ring bond nomenclature,
            else False.
            This does not imply anything about the number of rings
            in the molecule described by the SMILES string.

        ``pos``
            Offset of the bond's token in the SMILES string, or ``None``
            if the bond is implicit.

        ``len``
            Length of the bond's token, or zero if implicit.

Example
^^^^^^^

The following example shows how to parse a SMILES string::

    import coho.smi
    from pprint import pprint

    p = coho.smi.Parser()
    try:
        p.parse('CNCC')
    except ValueError as e:
        print(e)
    else:
        pprint(p.atoms)
        print()
        pprint(p.bonds)
