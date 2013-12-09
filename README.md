cdifflib
========

Python [difflib](http://docs.python.org/2/library/difflib.html) sequence
matcher reimplemented in C.

Actually only contains reimplemented parts.  Creates a `CSequenceMatcher` type
which inherets most functions from `difflib.SequenceMatcher`.

`cdifflib` is about 4x the speed of the pure python `difflib` when diffing
large streams.

Limitations
-----------
Can only work on 2 lists rather than generic sequences.

Usage
-----
Can be used just like the `difflib.SequenceMatcher` as long as you pass lists.  These examples are right out of the [difflib docs](http://docs.python.org/2/library/difflib.html):
```Python
>>> from cdifflib import CSequenceMatcher
>>> s = CSequenceMatcher(None, list(' abcd'), list('abcd abcd')
>>> s.find_longest_match(0, 5, 0, 9)
Match(a=1, b=0, size=4)
>>> s = CSequenceMatcher(lambda x: x == " ",
...                      list("private Thread currentThread;"),
...                      list("private volatile Thread currentThread;"))
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
