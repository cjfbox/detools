|buildstatus|_
|appveyor|_
|coverage|_
|codecov|_

About
=====

Binary `delta encoding`_ in Python 3 and C.

Based on http://www.daemonology.net/bsdiff/, with the following
differences:

- BZ2, LZ4, LZMA, `Zstandard`_, `heatshrink`_ or CRLE compression.

- Linear patch file access pattern to allow streaming and less RAM
  usage.

- `SA-IS`_ or divsufsort instead of qsufsort.

- Variable length size fields.

- `Incremental apply patch`_ implemented in C, suitable for memory
  constrained embedded devices.

- `Normal`_ or `in-place`_ (resumable) updates.

- Optional experimental data format aware algorithm for potentially
  smaller patches. I don't recommend anyone to use this functionality
  as the gain is small in relation to memory usage and code
  complexity!

  There is a risk this functionality uses patent
  https://patents.google.com/patent/EP1988455B1/en. Anyway, this
  patent expires in August 2019 as I understand it.

  Supported data formats:

  - ARM Cortex-M4

  - AArch64

There is also a wrapper of `HDiffPatch`_, implementing basic
operations. Adds a custom header, so it is not compatible with
original `HDiffPatch`_ implementation as of today.

- See `HDiffPatch`_ for characteristics.

- Suitable for large files.

- Often slightly smaller patches than bsdiff.

- Often faster than bsdiff.

Project homepage: https://github.com/eerimoq/detools

Documentation: http://detools.readthedocs.org/en/latest

Installation
============

.. code-block:: python

    pip install detools

Statistics
==========

The percentages are calculated as "patch size" / "to size". Lower is
better.

+-----------------------+----------+---------+------------+
| Update                |  To size | normal  | hdiffpatch |
+=======================+==========+=========+============+
| upy v1.9.4 -> v1.10   |   615388 |  11.7 % |     10.7 % |
+-----------------------+----------+---------+------------+
| python.tar 3.7 -> 3.8 | 87091200 |   4.1 % |      2.8 % |
+-----------------------+----------+---------+------------+
| foo old -> new        |     2780 |   4.6 % |      5.3 % |
+-----------------------+----------+---------+------------+

Example usage
=============

Examples in C are found in `src/c`_.

Command line tool
-----------------

The create patch subcommand
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Create a patch ``foo.patch`` from ``tests/files/foo/old`` to
``tests/files/foo/new``.

.. code-block:: text

   $ detools create_patch tests/files/foo/old tests/files/foo/new foo.patch
   Successfully created patch 'foo.patch'!
   $ ls -l foo.patch
   -rw-rw-r-- 1 erik erik 127 feb  2 10:35 foo.patch

Create the same patch as above, but without compression.

.. code-block:: text

   $ detools create_patch --compression none \
         tests/files/foo/old tests/files/foo/new foo-no-compression.patch
   Successfully created patch 'foo-no-compression.patch'!
   $ ls -l foo-no-compression.patch
   -rw-rw-r-- 1 erik erik 2792 feb  2 10:35 foo-no-compression.patch

The create in-place patch subcommand
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Create an in-place patch ``foo-in-place.patch``.

.. code-block:: text

   $ detools create_patch_in_place --memory-size 3000 --segment-size 500 \
         tests/files/foo/old tests/files/foo/new foo-in-place.patch
   Successfully created patch 'foo-in-place.patch'!
   $ ls -l foo-in-place.patch
   -rw-rw-r-- 1 erik erik 672 feb  2 10:36 foo-in-place.patch

The create bsdiff patch subcommand
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Create a bsdiff patch ``foo-bsdiff.patch``, compatible with the
original bsdiff program.

.. code-block:: text

   $ detools create_patch_bsdiff \
         tests/files/foo/old tests/files/foo/new foo-bsdiff.patch
   Successfully created patch 'foo-bsdiff.patch'!
   $ ls -l foo-bsdiff.patch
   -rw-rw-r-- 1 erik erik 261 feb  2 10:36 foo-bsdiff.patch

The create hdiffpatch patch subcommand
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Create a hdiffpatch patch ``foo-hdiffpatch.patch``.

.. code-block:: text

   $ detools create_patch_hdiffpatch \
         tests/files/foo/old tests/files/foo/new foo-hdiffpatch.patch
   Successfully created patch 'foo-hdiffpatch.patch'!
   $ ls -l foo-hdiffpatch.patch
   -rw-rw-r-- 1 erik erik 146 feb  2 10:37 foo-hdiffpatch.patch

Lower memory usage with ``--match-block-size``. Mainly useful for big
files. Creates slightly bigger patches.

.. code-block:: text

   $ detools create_patch_hdiffpatch --match-block-size 64 \
         tests/files/foo/old tests/files/foo/new foo-hdiffpatch-64.patch
   Successfully created patch 'foo-hdiffpatch.patch'!
   $ ls -l foo-hdiffpatch-64.patch
   -rw-rw-r-- 1 erik erik 389 feb  2 10:38 foo-hdiffpatch-64.patch

The apply patch subcommand
^^^^^^^^^^^^^^^^^^^^^^^^^^

Apply the patch ``foo.patch`` to ``tests/files/foo/old`` to create
``foo.new``.

.. code-block:: text

   $ detools apply_patch tests/files/foo/old foo.patch foo.new
   $ ls -l foo.new
   -rw-rw-r-- 1 erik erik 2780 feb  2 10:38 foo.new

The in-place apply patch subcommand
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Apply the in-place patch ``foo-in-place.patch`` to ``foo.mem``.

.. code-block:: text

   $ cp tests/files/foo/in-place-3000-500.mem foo.mem
   $ detools apply_patch_in_place foo.mem foo-in-place.patch
   $ ls -l foo.mem
   -rw-rw-r-- 1 erik erik 3000 feb  2 10:40 foo.mem

The bsdiff apply patch subcommand
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Apply the patch ``foo-bsdiff.patch`` to ``tests/files/foo/old`` to
create ``foo.new``.

.. code-block:: text

   $ detools apply_patch_bsdiff tests/files/foo/old foo-bsdiff.patch foo.new
   $ ls -l foo.new
   -rw-rw-r-- 1 erik erik 2780 feb  2 10:41 foo.new

The patch info subcommand
^^^^^^^^^^^^^^^^^^^^^^^^^

Print information about the patch ``foo.patch``.

.. code-block:: text

   $ detools patch_info foo.patch
   Type:               normal
   Patch size:         127 bytes
   To size:            2.71 KiB
   Patch/to ratio:     4.6 % (lower is better)
   Diff/extra ratio:   9828.6 % (higher is better)
   Size/data ratio:    0.3 % (lower is better)
   Compression:        lzma

   Number of diffs:    2
   Total diff size:    2.69 KiB
   Average diff size:  1.34 KiB
   Median diff size:   1.34 KiB

   Number of extras:   2
   Total extra size:   28 bytes
   Average extra size: 14 bytes
   Median extra size:  14 bytes

Contributing
============

#. Fork the repository.

#. Install prerequisites.

   .. code-block:: text

      pip install -r requirements.txt

#. Implement the new feature or bug fix.

#. Implement test case(s) to ensure that future changes do not break
   legacy.

#. Run the tests.

   .. code-block:: text

      make test

#. Create a pull request.

.. |buildstatus| image:: https://travis-ci.org/eerimoq/detools.svg?branch=master
.. _buildstatus: https://travis-ci.org/eerimoq/detools

.. |appveyor| image:: https://ci.appveyor.com/api/projects/status/github/eerimoq/detools?svg=true
.. _appveyor: https://ci.appveyor.com/project/eerimoq/detools/branch/master

.. |coverage| image:: https://coveralls.io/repos/github/eerimoq/detools/badge.svg?branch=master
.. _coverage: https://coveralls.io/github/eerimoq/detools

.. |codecov| image:: https://codecov.io/gh/eerimoq/detools/branch/master/graph/badge.svg
.. _codecov: https://codecov.io/gh/eerimoq/detools

.. _SA-IS: https://sites.google.com/site/yuta256/sais

.. _HDiffPatch: https://github.com/sisong/HDiffPatch

.. _Incremental apply patch: https://github.com/eerimoq/detools/tree/master/src/c

.. _delta encoding: https://en.wikipedia.org/wiki/Delta_encoding

.. _heatshrink: https://github.com/atomicobject/heatshrink

.. _Zstandard: https://facebook.github.io/zstd

.. _Normal: https://detools.readthedocs.io/en/latest/#id1

.. _in-place: https://detools.readthedocs.io/en/latest/#id2

.. _src/c: https://github.com/eerimoq/detools/tree/master/src/c
