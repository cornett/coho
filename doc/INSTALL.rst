.. highlight:: none

Installation
============

A self-contained Python package is available on `PyPI`_.
It can be installed using pip::

    pip3 install coho

Installing this package is sufficient to use Coho via Python.
The remainder of this document describes how to build and install
the Coho C library.


Requirements
------------

Building requires ``make`` and a C compiler.
Compiler requirements are modest: C89 plus a
few C99 features such as ``<stdint.h>``.

`Cython`_ is required to build the Python bindings.

Coho has been tested on the following platforms, but will likely work
on any modern unix-like system.

* OpenBSD 6.2
* CentOS 7
* OSX 10.12

Configure
---------

After unpacking the source distribution,
customize ``Makefile`` to fit your system.
In particular, you may wish to adjust the following
variable settings.

``PREFIX``
    This is the top-level directory under which Coho is to be installed.
    For example, you could change this to ``~/pkg/coho`` if you wanted
    to install it just for yourself.

``CPPFLAGS``
    Extra flags to pass to the C preprocessor.
    This is not needed on most systems.

``CFLAGS``
    Extra flags to pass to the C compiler.
    The default is fine on most systems.

``PYTHON``
    Path to the Python executable.
    This can be ignored if you are only building the C library.
    Otherwise, make sure it points to an installation of
    (C)Python, version 3.4 or higher.
    You may need to change this setting if multiple Python
    installations are available on your system.

``CYTHON``
    Path to the `Cython`_ compiler, which is used to create the
    Python bindings.
    This can be ignored if you are only building the C library.
    The following procedure may be useful if Cython is
    not already installed on your system::

        python3 -m venv pyenv
        ./pyenv/bin/pip install cython

    The above installs Cython into a Python
    `virtual environment <venv>`_.
    You should then set ``CYTHON`` to ``./pyenv/bin/cython``.
    Cython is required only at build time.

``PY.CONFIG``
    Python ships with a ``python-config`` script that can be used to
    determine the compiler and linker flags needed to build
    extension modules.
    Note that you may need to change the default
    if you set your ``PYTHON`` variable to an executable
    within a virtual env.
    This variable is only used to set the ``PY.CFLAGS``, ``PY.LDFLAGS``,
    and ``PY.LDLIBS`` variables, so it can be ignored if you choose
    to set those manually.

``PY.CFLAGS``
    Flags to pass to the C compiler when compiling Python
    extension modules.
    The default will likely work if ``PY.CONFIG`` is set correctly,
    but check that ``-fPIC`` is being included in the flags
    if relocation errors occur.

``PY.LDFLAGS``
    Flags to pass to the C compiler when linking
    Python extension modules.
    The default should work if ``PY.CONFIG`` is set correctly.

``PY.LDLIBS``
    Library flags (-lfoo) to pass to the C compiler when linking
    Python extension modules.
    The default should work if ``PY.CONFIG`` is set correctly.


Build
-----

To build Coho, type ``make``.
This will build ``libcoho.a`` and its Python bindings.
Type ``make libcoho.a`` to only build the C library.


Install
-------

To install Coho, type ``make install``.

.. _Cython: http://cython.org/
.. _PyPI: https://pypi.python.org/
.. _venv: https://docs.python.org/3/library/venv.html
