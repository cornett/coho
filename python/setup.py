import glob
import os
import re
import shutil
from setuptools import setup, Extension

# Work from directory containing setup.py
os.chdir(os.path.abspath(os.path.dirname(__file__)))

# Get version number
with open("version.txt") as f:
    version = f.read().strip()

ext = [
    Extension(
        "coho.__init__",
        define_macros=[("VERSION", '"{}"'.format(version))],
        include_dirs=["src"],
        sources=["coho/__init__.c"],
    ),
    Extension(
        "coho.smiles",
        include_dirs=["src"],
        sources=["coho/smiles.c", "src/smiles.c", "src/compat.c"],
    ),
]

setup(
    name="coho",
    version=version,
    packages=["coho"],
    ext_modules=ext,
    author="Ben Cornett",
    author_email="ben@lantern.is",
    description="SMILES parser",
    license="ISC",
    package_data={"coho": ["py.typed", "*.pyi"]},
    keywords="smiles opensmiles cheminformatics",
    python_requires=">= 3.4",
    url="https://github.com/cornett/coho",
    include_package_data=True,
)
