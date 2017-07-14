#!/usr/bin/env python3

# Copyright (c) 2017 Ben Cornett <ben@lantern.is>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

import base64
import csv
import distutils.util
import hashlib
import io
import os
import platform
import re
import sys
import sysconfig
import zipfile

# Wheel compatiblity tags.  See PEP 425, 427.

def py_tag():
    return 'cp' + sysconfig.get_config_var('py_version_nodot')


def abi_tag():
    soabi = sysconfig.get_config_var('SOABI')
    assert soabi.startswith('cpython')
    return 'cp' + soabi.split('-')[1]


def plat_tag():
    # distutils.util.get_platform() is not accurate on osx
    # Workaround taken from pip code.
    if sys.platform == 'darwin':
        release, _, machine = platform.mac_ver()
        ver = release.split('.')
        return 'macosx_{}_{}_{}'.format(ver[0], ver[1], machine)

    return re.sub('[-.]', '_', distutils.util.get_platform())


def wheel_compat_tag():
    return '{}-{}-{}'.format(py_tag(), abi_tag(), plat_tag())


WHEEL = '''\
Wheel-Version: 1.0
Generator: coho ({})
Root-Is-Purelib: false
Tag: {}
'''

METADATA = '''\
Metadata-Version: 2.0
Name: coho
Version: {}
Summary: UNKNOWN
Home-page: UNKNOWN
Author: UNKNOWN
Author-email: UNKNOWN
License: ISC
Platform: UNKNOWN

UNKNOWN
'''

def escape_hash(hash):
    return base64.urlsafe_b64encode(hash).decode().rstrip('=')


def content_hash(data):
    m = hashlib.sha256()
    m.update(data)
    return 'sha256=' + escape_hash(m.digest())


def main():
    version, *objects = sys.argv[1:]
    tag = wheel_compat_tag()
    whlpath = 'py/dist/coho-{}-{}.whl'.format(version, tag)
    distdir = 'coho-{}.dist-info/'.format(version)

    # RECORD writer
    rf = io.StringIO(newline='')
    rw = csv.writer(rf)

    with zipfile.ZipFile(whlpath, 'w') as zf:
        try:
            for path in objects:
                assert path.startswith('py')
                zfpath = path[3:]
                with open(path, 'rb') as f:
                    data = f.read()
                    zf.writestr(zfpath, data)
                    rw.writerow((zfpath, content_hash(data), len(data)))

            path = distdir + 'WHEEL'
            data = WHEEL.format(version, wheel_compat_tag())
            data = data.encode()
            zf.writestr(path, data)
            rw.writerow((path, content_hash(data), len(data)))

            path = distdir + 'METADATA'
            data = METADATA.format(version)
            data = data.encode()
            zf.writestr(path, data)
            rw.writerow((path, content_hash(data), len(data)))

            path = distdir + 'RECORD'
            rw.writerow((path, '', ''))
            zf.writestr(path, rf.getvalue())

        except:
            zf.close()
            os.remove(whlpath)
            raise


if __name__ == '__main__':
    main()
