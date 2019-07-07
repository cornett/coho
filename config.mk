VERSION = 0.4
B = build
PREFIX = /usr/local
CFLAGS = -fPIC -Wall -Wextra -std=c99 -pedantic -O2
AR = ar
CC = cc

CYTHON = cython
CYTHON_FLAGS = -3 -X embedsignature=True
PYTHON = python3
PYTHON_CONFIG = $(PYTHON)-config
PYTHON_CFLAGS = `$(PYTHON_CONFIG) --cflags` -fPIC
