cdifflib
========
  <img src="https://travis-ci.org/mduggan/cdifflib.svg?branch=master">
  
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

The C extension only works with Python 2.7 at the moment.

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
