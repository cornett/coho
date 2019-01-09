.. default-domain:: c
.. highlight:: c

C API
=====

SMILES
------

The :func:`smiles_parse()` function parses
`SMILES <https://en.wikipedia.org/wiki/Simplified_molecular-input_line-entry_system>`_
as specified by the
`OpenSMILES <http://opensmiles.org/>`_ standard.

Parsing requires a context, which has type
:type:`struct smiles <smiles>` and
is initialized using :func:`smiles_init()`.
Once initialized, a context can be used with :func:`smiles_parse()`
to parse one or more SMILES strings.
:func:`smiles_free()` releases any resources acquired during parsing.

Note that the data structures contained in the context
are intended to represent parsed SMILES strings, not molecules.
Conformance of a particular string to the SMILES grammar does
not imply description of a chemically-meaningful structure.


.. type:: struct smiles

    ::

        struct smiles {
                char                        *error;
                int                          error_position;
                struct smiles_atom             *atoms;
                size_t                       atoms_sz;
                struct smiles_bond             *bonds;
                size_t                       bonds_sz;
        }

    The following fields of the context are public and can
    be used to examine the results of a
    call to :func:`smiles_parse()`:

    .. member:: char \*error

        If :func:`smiles_parse()` fails, ``error``
        will point to an error message, otherwise it will be ``NULL``.

    .. member:: int error_position

        If :func:`smiles_parse()` fails, ``error_position`` will contain the offset
        into the SMILES string where the
        error was detected, otherwise it will be -1.

    .. member:: struct smiles_atom \*atoms

        Each parsed atom is represented by an instance of
        :type:`struct smiles_atom <smiles_atom>`
        described below.

    .. member:: size_t atoms_sz

        Length of :member:`atoms <smiles.atoms>`.

    .. member:: struct smiles_bond \*bonds

        Each parsed bond is represented by an instance of
        :type:`struct smiles_bond <smiles_bond>`
        described below.

    .. member:: size_t bonds_sz

        Length of :member:`bonds <smiles.bonds>`.

If :func:`smiles_parse()` fails, the only valid access is to the
:member:`error <smiles.error>` and :member:`error_position <smiles.error_position>`
fields.


.. type:: struct smiles_atom

    ::

        struct smiles_atom {
                int                      atomic_number;
                char                     symbol[4];
                int                      isotope;
                int                      charge;
                int                      hydrogen_count;
                int                      implicit_hydrogen_count;
                int                      is_bracket;
                int                      is_organic;
                int                      is_aromatic;
                char                     chirality[8];
                int                      atom_class;
                int                      position;
                int                      length;
        };

    Each atom parsed from the input is represented
    by an instance of :type:`struct smiles_atom <smiles_atom>`.
    Its fields are described below:

    .. member:: int atomic_number

        The atom's atomic number, deduced from the symbol.
        The wildcard atom is assigned an atomic number of zero.

    .. member:: char symbol[4]

        Element symbol as it appears in the SMILES string.
        Atoms designated as aromatic will have lowercase symbols.

    .. member:: int isotope

        Isotope, or -1 if unspecified.
        Note that the `OpenSMILES <http://opensmiles.org/>`_ specification
        states that zero is a valid isotope and that
        ``[0S]`` is not the same as ``[S]``.

    .. member:: int charge

        Formal charge, or 0 if none was specified.

    .. member:: int hydrogen_count

        Number of explicit hydrogens, or -1 if none were specified.

    .. member:: int implicit_hydrogen_count

        Number of implicit hydrogens required to bring atom to its
        next standard valence state.
        Set to -1 for atoms not specified using the organic
        subset nomenclature.

    .. member:: int is_bracket

        1 if the atom was specified using bracket(``[]``) notation, else 0.

    .. member:: int organic

        1 if the atom was specified using the
        organic subset nomenclature, else 0.
        Wildcard atoms are not considered part of the organic subset.
        If they occur outside of a bracket, their ``is_bracket`` and
        ``organic`` fields will both be 0.

    .. member:: int aromatic

        1 if the atom's symbol is lowercase, indicating that it is
        aromatic, else 0.

    .. member:: char chirality[8]

        The chirality label, if provided, else the empty string.
        Currently, parsing is limited to ``@`` and ``@@``.
        Use of other chirality designators will result in a parsing error.

    .. member:: int atom_class

        Positive integer atom class if specified, else -1.

    .. member:: int position

        Offset of the atom's token in the SMILES string.

    .. member:: int length

        Length of the atom's token.


.. type:: struct smiles_bond

    ::

        struct smiles_bond {
                int                      atom0;
                int                      atom1;
                int                      order;
                int                      stereo;
                int                      is_implicit;
                int                      is_ring;
                int                      position;
                int                      length;
        };

    Each bond parsed from the input produces an
    instance of :type:`struct smiles_bond <smiles_bond>`.
    Its fields are described below:

    .. member:: int atom0

        The atom number (offset into :member:`atoms <smiles.atoms>`)
        of the first member of the bond pair.

    .. member:: int atom1

        The atom number (offset into :member:`atoms <smiles.atoms>`)
        of the second member of the bond pair.

    .. member:: int order

        Bond order, with values from the following enumeration:

        * SMILES_BOND_SINGLE
        * SMILES_BOND_DOUBLE
        * SMILES_BOND_TRIPLE
        * SMILES_BOND_QUAD
        * SMILES_BOND_AROMATIC

    .. member:: int stereo

        Used to indicate the cis/trans configuration of atoms
        around double bonds.
        Takes values from the following enumeration:

        ``SMILES_BOND_STEREO_UNSPECIFIED``
            Bond has no stereochemistry
        ``SMILES_BOND_STEREO_UP``
            lies "up" from :member:`atom0 <smiles_bond.atom0>`
        ``SMILES_BOND_STEREO_DOWN``
            lies "down" from :member:`atom0 <smiles_bond.atom0>`

    .. member:: int implicit

        1 if bond was produced implicitly by the presence of two
        adjacent atoms without an intervening bond symbol, else 0.
        Implicit bonds do not have a token position or length.
        An aromatic bond is implied by two adjacent aromatic atoms,
        otherwise implicit bonds are single.

    .. member:: int ring

        1 if the bond was produced using the ring bond nomenclature,
        else 0.
        This does not imply anything about the number of rings
        in the molecule described by the SMILES string.

    .. member:: int position

        Offset of the bond's token in the SMILES string, or -1 if the
        bond is implicit.

    .. member:: int length

        Length of the bond's token, or zero if implicit.


.. function:: void smiles_init(struct smiles \*)

    Initializes a SMILES parsing context.

.. function:: void smiles_free(struct smiles \*)

    Releases resources held by the context.
    This only needs to be called once, after all parsing is complete.

.. function:: int smiles_parse(struct smiles \*smiles, const char \*str, size_t sz)

    Parses a SMILES string.
    If successful, the fields of :type:`smiles <smiles>` will contain
    the results.

    :param smiles: Parsing context, initialized by :func:`smiles_init()`
    :param str: SMILES string
    :param sz: Amount of string to read.  If zero, the entire string is parsed.
    :return: Returns 0 on success, -1 on failure

Example
^^^^^^^

The following example shows how to parse a SMILES string::

    #include <stdio.h>
    #include <coho/smiles.h>

    int
    main(void)
    {
            size_t i;
            struct smiles smiles;

            smiles_init(&smiles);

            if (smiles_parse(&smiles, "CNCC", 0)) {
                    fprintf(stderr, "failed: %s\n", smiles.error);
                    smiles_free(&smiles);
                    return 1;
            }

            printf("# atoms: %zi\n", smiles.atoms_sz);
            printf("# bonds: %zi\n", smiles.bonds_sz);
            printf("\n");

            for (i = 0; i < smiles.atoms_sz; i++) {
                    printf("%zi: %s\n", i, smiles.atoms[i].symbol);
            }
            printf("\n");

            for (i = 0; i < smiles.bonds_sz; i++) {
                    printf("%zi-%zi %i\n",
                           smiles.bonds[i].atom0,
                           smiles.bonds[i].atom1,
                           smiles.bonds[i].order);
            }

            smiles_free(&smiles);

            return 0;
    }
