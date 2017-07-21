import os
import glob
import re
import shutil
from setuptools import setup, Extension

# Work from directory containing setup.py
os.chdir(os.path.abspath(os.path.dirname(__file__)))

# Check if building package or installing package
isbuild = not os.path.exists('PKG-INFO')

if isbuild:
    # Copy C sources
    try:
        os.mkdir('src', 0o755)
    except OSError:
        pass
    for src in glob.glob('../*.[ch]'):
        shutil.copy(src, 'src')


# Get version number
with open('version.txt') as f:
    version = f.read().strip()


ext = [
    Extension(
        'coho.__init__',
        extra_compile_args=['-DVERSION="{}"'.format(version)],
        sources = ['coho/__init__.c']
    ),

    Extension(
        'coho.smi',
        extra_compile_args=['-I', 'src'],
        sources = [
            'coho/smi.c',
            'src/smi.c',
            'src/vec.c',
            'src/compat.c',
        ]
    ),

]


setup(
    name = 'coho',
    version = version,
    ext_modules = ext,
    author = 'Ben Cornett',
    author_email = 'ben@lantern.is',
    description = 'SMILES parser',
    license = 'ISC',
    keywords = 'smiles opensmiles cheminformatics',
    url = 'https://github.com/cornett/coho',
)
