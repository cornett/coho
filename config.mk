VERSION = 0.4
PREFIX = /usr/local
CFLAGS = -fPIC -Wall -Wextra -std=c99 -pedantic -O2
AR = ar
CC = cc

CYTHON = cython
PYTHON = python3
PYTHON_CONFIG = $(PYTHON)-config
PYTHON_CFLAGS = `$(PYTHON_CONFIG) --cflags` -fPIC
