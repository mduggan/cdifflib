cdifflib
========

Python difflib core parts reimplemented in C.

Actually only contains the reimplemented parts.  Creates a "CSequenceMatcher"
type which inherets most functions from difflib's SequenceMatcher.

Limitations
-----------
Can only work on 2 lists rather than generic sequences.


Making
------

To install:
 make install

To test:
 make test

TODO
----

* Structure more like a proper python module
* PyPi packaging
