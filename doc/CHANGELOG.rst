ChangeLog
=========

`Unreleased`_
-------------


Changed
^^^^^^^
* Enable building with both BSD and GNU Make.

`v0.4`_ - 2019-01-17
--------------------

Added
^^^^^
* `mypy <http://mypy-lang.org/>`_ type annotations
* Reliable reporting of allocation failures. 
* PYPI `manylinux1`_ wheels.

Changed
^^^^^^^
* Build with GNU Make.
* Rename various identifiers (API change).
* Consolidate all declarations into ``coho.h``.

`v0.3`_ - 2017-12-16
--------------------

Changed
^^^^^^^
* Revise Makefile.
* Improve documentation.
* Improve positioning info for SMILES integer-too-large errors.
* Allow parsing of deeply-nested SMILES.

Fixed
^^^^^
* SMILES: Fix implicit bond order between aromatic atoms.

`v0.2`_ - 2017-07-21
--------------------

Added
^^^^^
* Self-contained `PyPI package <https://pypi.python.org/pypi/coho>`_.
* SMILES tests.
* SMILES AFL harness.

Changed
^^^^^^^
* Revise build system.
* Improve python packaging.
* Improve documentation.

v0.1 - 2017-07-12
-----------------

Added
^^^^^
* SMILES parser.

.. _Unreleased: https://github.com/cornett/coho/compare/v0.4...HEAD
.. _v0.4: https://github.com/cornett/coho/compare/v0.3...v0.4
.. _v0.3: https://github.com/cornett/coho/compare/v0.2...v0.3
.. _v0.2: https://github.com/cornett/coho/compare/v0.1...v0.2

.. _manylinux1: https://www.python.org/dev/peps/pep-0513/
