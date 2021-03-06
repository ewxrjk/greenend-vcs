# VCS

VCS is a wrapper for version control systems.  It presents an
essentially uniform interface to the user, allowing 'muscle memory' to
use vcs commands rather than adapt to the version control system
currently in use.

Bazaar, Git, CVS, Subversion, Mercurial, Darcs, Perforce, RCS and SCCS
are supported, with a small set of commands.  See the man page for
more details.

⚠ I don't use this program any more, do not expect updates. ⚠

# Installation

To build VCS, you will need:

* [GNU C++](http://gcc.gnu.org/)
* [cURL](http://curl.haxx.se/)
* [Expat](http://expat.sourceforge.net/)

Use the following commands:

    ./configure
    make
    sudo make install

Read `man vcs` for documentation.

## Using configure

See [INSTALL](INSTALL) for generic instructions.  Options specific to this
package are:

`./configure --without-werror`: Suppresses use of the -Werror option.
This is useful (for instance) if your platform generates warnings for
its own header files.

`./configure --with-gcov`: Enable coverage testing.  Only useful for
developers.

# Feedback

## Bugs and Feature Requests

[Please raise issues on GitHub](https://github.com/ewxrjk/greenend-vcs/issues).

## Contributions

Please see [DEVELOPERS.md](DEVELOPERS.md).

# Copyright

Copyright (C) 2009-2018 Richard Kettlewell

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see [http://www.gnu.org/licenses/]([http://www.gnu.org/licenses/).
