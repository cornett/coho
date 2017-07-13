# Wheel compatiblity tags.  See PEP 425, 427.

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

import distutils.util
import re
import sysconfig

def py_tag():
    return 'cp' + sysconfig.get_config_var('py_version_nodot')

def abi_tag():
    soabi = sysconfig.get_config_var('SOABI')
    assert soabi.startswith('cpython')
    return 'cp' + soabi.split('-')[1]

def plat_tag():
    return re.sub('[-.]', '_', distutils.util.get_platform())

def wheel_compat_tag():
    return '{}-{}-{}'.format(py_tag(), abi_tag(), plat_tag())

if __name__ == '__main__':
    print(wheel_compat_tag())
