.. highlight:: none

Installation
============

A self-contained Python package is available on `PyPI`_.
It can be installed using pip::

    pip install coho

Installing this package is sufficient to use Coho via Python.
The remainder of this document describes how to build and install
the Coho C library.


Requirements
------------

Building requires GNU make and a C compiler.
Compiler requirements are modest: C89 plus a
few C99 features such as ``<stdint.h>``.

`Cython`_ is required to build the Python bindings from source.

Coho has been tested on the following platforms, but will likely work
on any modern unix-like system.

* OpenBSD 6.4
* CentOS 7
* OSX 10.12

Configure
---------

After unpacking the source distribution,
customize ``Makefile`` to fit your system.
You can either edit ``Makefile`` directly, or override the
defaults by creating a file called ``config.mk``.
In particular, you may wish to adjust the following
variable settings.

``PREFIX``
    The top-level directory under which Coho will be installed.

``CPPFLAGS``
    Extra flags to pass to the C preprocessor.
    This is not needed for most systems.

``CFLAGS``
    Extra flags to pass to the C compiler.
    The default is fine for most systems.

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

``PYTHON_CONFIG``
    Python ships with a ``python-config`` script that can be used to
    determine the compiler and linker flags needed to build
    extension modules.
    Note that you may need to change the default
    if you set your ``PYTHON`` variable to an executable
    within a virtual env.
    This variable is only used to set the ``PYTHON_CFLAGS``,
    ``PYTHON_LDFLAGS``, and ``PYTHON_LIBS`` variables, so it can
    be ignored if you choose to set those manually.

``PYTHON_CFLAGS``
    Flags to pass to the C compiler when compiling Python
    extension modules.
    The default will likely work if ``PYTHON_CONFIG`` is set correctly,
    but if relocation errors occur, check that ``-fPIC`` is being
    included in the flags.

``PYTHON_LDFLAGS``
    Flags to pass to the C compiler when linking Python extension modules.
    The default should work if ``PYTHON_CONFIG`` is set correctly.

``PYTHON_LIBS``
    Library flags (ex. -lpython3.7m) to pass to the C compiler when linking
    Python extension modules.
    The default should work if ``PYTHON_CONFIG`` is set correctly.


Build
-----

To build Coho, type ``make`` (or ``gmake`` on some BSD systems).
This will build ``libcoho.a`` and its Python bindings.
Type ``make libcoho.a`` to only build the C library.


Install
-------

To install Coho, type ``make install``.

.. _Cython: http://cython.org/
.. _PyPI: https://pypi.org/project/coho/
.. _venv: https://docs.python.org/3/library/venv.html
