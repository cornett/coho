Installation
============

Building the Coho C library requires ``make`` and a C compiler.
The Python bindings require Python 3 and `Cython`_.

Coho has been tested on the following platforms, but will likely work
on any modern unix-like system.

* OpenBSD 6.1
* CentOS 7
* OSX 10.12

Configure
---------

After unpacking the source distribution,
customize the ``config.mk`` file to fit your system.
The default file looks like this::

    PREFIX      = /usr/local
    MANPREFIX   = $(PREFIX)/share/man

    CPPFLAGS    =
    CFLAGS      = -Wall -Wextra -O2 -fPIC

    PYTHON      = python3
    CYTHON      = cython

    PY.CONFIG   = $(PYTHON)-config
    PY.CFLAGS   = `$(PY.CONFIG) --cflags`
    PY.LDFLAGS  = `$(PY.CONFIG) --ldflags`
    PY.LDLIBS   = `$(PY.CONFIG) --libs`


These variables are explained below.

``PREFIX``
    This is the top-level directory under which Coho is to be installed.
    For example, you could change this to ``~/pkg/coho`` if you wanted
    to install it just for yourself.

``MANPREFIX``
    Directory under which man pages are to be installed.
    The default is fine on most systems.

``CPPFLAGS``
    Extra flags to pass to the C preprocessor.
    This is not needed on most systems.

``CFLAGS``
    Extra flags to pass to the C compiler.
    The default is fine on most systems.

``PYTHON``
    Path to the Python executable.
    This can be ignored if you are only building the C library.
    Otherwise, make sure it points to the version of CPython 3
    that you wish to create bindings for.
    You may need to change this setting if multiple Python
    installations are available on your system.

``CYTHON``
    Path to the `Cython`_ compiler, which is used to create the
    Python bindings.
    This can be ignored if you are only building the C library.
    The following procedure may be useful if Cython is
    not installed on your system::

        python3 -m venv pyenv
        ./pyenv/bin/pip install cython

    The above installs Cython into a temporary Python
    `virtual environment <venv>`_.
    You should then set ``CYTHON`` to ``./pyenv/bin/cython``.
    This works because Cython is required only at build time.

``PY.CONFIG``
    Python ships with a ``python-config`` script that can be used to
    determine the compiler and linker flags needed to build
    extension modules.
    The default setting is usually correct, but may need to be adjusted
    if you set your ``PYTHON`` variable to an executable
    within a virtual env.
    This variable is only used to set the ``PY.CFLAGS``, ``PY.LDFLAGS``,
    and ``PY.LDLIBS`` variables, so it can be ignored if you set
    those manually.

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

.. _Cython: http://cython.org/
.. _venv: https://docs.python.org/3/library/venv.html


Build
-----

To build Coho, type ``make``.


Install
-------

To install Coho, type ``make install``.
