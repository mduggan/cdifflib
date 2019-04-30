cdifflib
========
  [<img src="https://travis-ci.org/mduggan/cdifflib.svg?branch=master">](https://travis-ci.org/mduggan/cdifflib/)

Python [difflib](http://docs.python.org/2/library/difflib.html) sequence
matcher reimplemented in C.

Actually only contains reimplemented parts.  Creates a `CSequenceMatcher` type
which inherets most functions from `difflib.SequenceMatcher`.

`cdifflib` is about 4x the speed of the pure python `difflib` when diffing
large streams.

Limitations
-----------
The C part of the code can only work on `list` rather than generic iterables,
so anything that isn't a `list` will be converted to `list` in the
`CSequenceMatcher` constructor.  This may cause undesirable behavior if you're
not expecting it.

Works with Python 2.7 and 3.6 (Should work on all 3.3+)

Usage
-----
Can be used just like the `difflib.SequenceMatcher` as long as you pass lists.  These examples are right out of the [difflib docs](http://docs.python.org/2/library/difflib.html):
```Python
>>> from cdifflib import CSequenceMatcher
>>> s = CSequenceMatcher(None, ' abcd', 'abcd abcd')
>>> s.find_longest_match(0, 5, 0, 9)
Match(a=1, b=0, size=4)
>>> s = CSequenceMatcher(lambda x: x == " ",
...                      "private Thread currentThread;",
...                      "private volatile Thread currentThread;")
>>> print round(s.ratio(), 3)
0.866
```

It's completely compatible, so you can replace the difflib version on startup
and then other libraries will use CSequenceMatcher too, eg:
```Python
from cdifflib import CSequenceMatcher
import difflib
difflib.SequenceMatcher = CSequenceMatcher
import library_that_uses_difflib

# Now the library will transparantely be using the C SequenceMatcher - other
# things remain the same
library_that_uses_difflib.do_some_diffing()
```


Making
------
To install:
```
python setup.py install
```

To test:
```
python setup.py test
```

License etc
-----------
This code lives at https://github.com/mduggan.  See LICENSE for the license.


Changelog
---------
* 1.2.4 - Repackage yet again using twine for pypi upload (no binary changes)
* 1.2.3 - Repackage again with changelog update and corrected src package (no binary changes)
* 1.2.2 - Repackage to add README.md in a way pypi supports (no binary changes)
* 1.2.1 - Fix bug for longer sequences with "autojunk"
* 1.2.0 - Python 3 support for other versions
* 1.1.0 - Added Python 3.6 support (thanks Bclavie)
* 1.0.4 - Changes to make it compile on MSVC++ compiler, no change for other platforms
* 1.0.2 - Bugfix - also replace set_seq1 implementation so `difflib.compare` works with a `CSequenceMatcher`
* 1.0.1 - Implement more bits in c to squeeze a bit more speed out
* 1.0.0 - First release
