#include <Python.h>

#if PY_MAJOR_VERSION == 2

//
// A simple wrapper to see if two Python list entries are "Python equal".
//
static __inline int
list_items_eq(PyObject *a, int ai, PyObject *b, int bi)
{
    PyObject *o1 = PyList_GET_ITEM(a, ai);
    PyObject *o2 = PyList_GET_ITEM(b, bi);
    int result = PyObject_RichCompareBool(o1, o2, Py_EQ);
    return result;
}

//
// A simple wrapper to call a callable python object with an argument and
// return the result as a boolean.
//
static __inline int
call_obj(PyObject *callable, PyObject *arg)
{
    PyObject *result;
    int retval;
    if (!callable)
        return 0;
    assert(PyCallable_Check(callable));
    result = PyObject_CallFunctionObjArgs(callable, arg, NULL);
    retval = PyObject_IsTrue(result);
    Py_DECREF(result);
    return retval;
}

static void
_find_longest_match_worker(
    PyObject *self,
    PyObject *a,
    PyObject *b,
    PyObject *isbjunk,
    const int alo,
    const int ahi,
    const int blo,
    const int bhi,
    long *_besti,
    long *_bestj,
    long *_bestsize
)
{
    int besti = alo;
    int bestj = blo;
    int bestsize = 0;
    int i;

    assert(self && a && b);

    // Degenerate case: evaluate an empty range: there is no match.
    if (alo == ahi || blo == bhi) {
        *_besti = alo;
        *_bestj = blo;
        *_bestsize = 0;
        return;
    }

    //printf("longest match helper\n");
    {
        PyObject *b2j = PyObject_GetAttrString(self, "b2j");
        PyObject *j2len = PyDict_New();
        PyObject *newj2len = PyDict_New();

        assert(PyDict_CheckExact(b2j));

        //
        // This loop creates a lot of python objects only to read back
        // their values inside the same loop.  It should be faster using a
        // simpler data structure to do the same thing, but I rewrote it
        // with a c++ unordered_map<int,int> and it was half the speed :(
        //
        for (i = alo; i < ahi; i++)
        {
            PyObject *tmp;
            PyObject *oj = PyDict_GetItem(b2j, PyList_GET_ITEM(a, i));

            /* oj is a list of indexes in b at which the line a[i] appears, or
               NULL if it does not appear */
            if (oj != NULL)
            {
                int ojlen, oji;
                assert(PyList_Check(oj));
                ojlen = (int)PyList_GET_SIZE(oj);
                for (oji = 0; oji < ojlen; oji++)
                {
                    PyObject *j2len_j, *jint, *kint, *jminus1;
                    int j;
                    int k = 1;

                    jint = PyList_GET_ITEM(oj, oji);
                    assert(PyInt_CheckExact(jint));
                    j = (int)PyInt_AS_LONG(jint);

                    if (j < blo)
                        continue;
                    if (j >= bhi)
                        break;

                    jminus1 = PyInt_FromLong(j-1);
                    j2len_j = PyDict_GetItem(j2len, jminus1);
                    Py_DECREF(jminus1);
                    if (j2len_j)
                        k += (int)PyInt_AS_LONG(j2len_j);

                    // this looks like an allocation, but k is usually low
                    kint = PyInt_FromLong(k);
                    PyDict_SetItem(newj2len, jint, kint);
                    Py_DECREF(kint);

                    if (k > bestsize) {
                        besti = i-k;
                        bestj = j-k;
                        bestsize = k;
                    }
                }
            }

            // Cycle j2len and newj2len
            tmp = j2len;
            j2len = newj2len;
            newj2len = tmp;
            PyDict_Clear(newj2len);
        }

        /* besti and bestj are offset by 1 if set in the loop above */
        if (bestsize)
        {
            besti++;
            bestj++;
        }

        /* Done with these now. */
        Py_DECREF(j2len);
        Py_DECREF(newj2len);
        Py_DECREF(b2j);
    }

    //printf("twiddle values %d %d %d %d %d %d\n", besti, alo, ahi, bestj, blo, bhi);
    while (besti > alo && bestj > blo &&
            !call_obj(isbjunk, PyList_GET_ITEM(b, bestj-1)) &&
            list_items_eq(a, besti-1, b, bestj-1))
    {
        besti--;
        bestj--;
        bestsize++;
    }

    //printf("twiddle values 2\n");
    while (besti+bestsize < ahi && bestj+bestsize < bhi &&
            !call_obj(isbjunk, PyList_GET_ITEM(b, bestj+bestsize)) &&
            list_items_eq(a, besti+bestsize, b, bestj+bestsize))
    {
        bestsize++;
    }


    //printf("twiddle values 3\n");
    while (besti > alo && bestj > blo &&
            call_obj(isbjunk, PyList_GET_ITEM(b, bestj-1)) &&
            list_items_eq(a, besti-1, b, bestj-1))
    {
        besti--;
        bestj--;
        bestsize++;
    }

    //printf("twiddle values 4\n");
    while (besti+bestsize < ahi && bestj+bestsize < bhi &&
            call_obj(isbjunk, PyList_GET_ITEM(b, bestj+bestsize)) &&
            list_items_eq(a, besti+bestsize, b, bestj+bestsize))
    {
        bestsize++;
    }

    //printf("helper done\n");
    *_besti = besti;
    *_bestj = bestj;
    *_bestsize = bestsize;
}


//
// A very simple C reimplementation of Python 2.7's
// difflib.SequenceMatcher.find_longest_match()
//
// The algorithm is identical (right down to using Python dicts and lists for
// local variables), but the c version runs in 1/4 the time.
//
static PyObject *
find_longest_match(PyObject *module, PyObject *args)
{
    long alo, ahi, blo, bhi;
    long besti, bestj, bestsize;
    PyObject *self, *a, *b, *isbjunk;

    if (!PyArg_ParseTuple(args, "Ollll", &self, &alo, &ahi, &blo, &bhi)) {
        PyErr_SetString(PyExc_ValueError, "find_longest_match parameters not as expected");
        return NULL;
    }

    assert(self);

    //printf("check junk\n");
    /* Slight speedup - if we have no junk, don't bother calling isbjunk lots */
    {
        PyObject *nojunk = PyObject_GetAttrString(self, "_nojunk");
        if (nojunk && PyObject_IsTrue(nojunk))
        {
            isbjunk = NULL;
        }
        else
        {
            PyErr_Clear();
            isbjunk = PyObject_GetAttrString(self, "isbjunk");
            assert(isbjunk);
            if (!PyCallable_Check(isbjunk)) {
                PyErr_SetString(PyExc_RuntimeError, "isbjunk not callable");
                return NULL;
            }
        }
        if (nojunk)
            Py_DECREF(nojunk);
    }

    //printf("get members\n");
    // FIXME: Really should support non-list sequences for a and b
    a = PyObject_GetAttrString(self, "a");
    b = PyObject_GetAttrString(self, "b");
    if (!PyList_Check(a) || !PyList_Check(b))
        return NULL;

    // This function actually does the work, the rest is just window dressing.
    _find_longest_match_worker(self, a, b, isbjunk, alo, ahi, blo, bhi, &besti, &bestj, &bestsize);

    //printf("done\n");

    Py_DECREF(a);
    Py_DECREF(b);
    if (isbjunk)
        Py_DECREF(isbjunk);

    return Py_BuildValue("iii", besti, bestj, bestsize);
}

/*
    def __helper(self, alo, ahi, blo, bhi, answer):
        i, j, k = x = self.find_longest_match(alo, ahi, blo, bhi)
        # a[alo:i] vs b[blo:j] unknown
        # a[i:i+k] same as b[j:j+k]
        # a[i+k:ahi] vs b[j+k:bhi] unknown
        if k:
            if alo < i and blo < j:
                self.__helper(alo, i, blo, j, answer)
            answer.append(x)
            if i+k < ahi and j+k < bhi:
                self.__helper(i+k, ahi, j+k, bhi, answer)
*/

static void
matching_block_helper(PyObject *self, PyObject *a, PyObject *b, PyObject *isjunk, PyObject *answer, const long alo, const long ahi, const long blo, const long bhi)
{
    long i, j, k;
    //printf("matching_block_helper 1\n");
    _find_longest_match_worker(self, a, b, isjunk, alo, ahi, blo, bhi, &i, &j, &k);
    //printf("matching_block_helper 2\n");

    if (k) {
        PyObject *p = Py_BuildValue("(iii)", i, j, k);
        if (alo < i && blo < j)
            matching_block_helper(self, a, b, isjunk, answer, alo, i, blo, j);
        PyList_Append(answer, p);
        Py_DECREF(p);
        if (i+k < ahi && j+k < bhi)
            matching_block_helper(self, a, b, isjunk, answer, i+k, ahi, j+k, bhi);
    }
    //printf("matching_block_helper 3\n");
}

static PyObject *
matching_blocks(PyObject *module, PyObject *args)
{
    PyObject *self, *a, *b, *isbjunk, *matching;
    long la, lb;

    if (!PyArg_ParseTuple(args, "O", &self)) {
        PyErr_SetString(PyExc_ValueError, "expected one argument, self");
        return NULL;
    }

    //printf("matching_blocks 1\n");
    /* Slight speedup - if we have no junk, don't bother calling isbjunk lots */
    {
        PyObject *nojunk = PyObject_GetAttrString(self, "_nojunk");
        if (nojunk && PyObject_IsTrue(nojunk))
        {
            isbjunk = NULL;
        }
        else
        {
            PyErr_Clear();
            isbjunk = PyObject_GetAttrString(self, "isbjunk");
            assert(isbjunk);
            if (!PyCallable_Check(isbjunk)) {
                PyErr_SetString(PyExc_RuntimeError, "isbjunk not callable");
                return NULL;
            }
        }
        if (nojunk)
            Py_DECREF(nojunk);
    }

    // FIXME: Really should support non-list sequences for a and b
    //printf("matching_blocks 2\n");
    a = PyObject_GetAttrString(self, "a");
    b = PyObject_GetAttrString(self, "b");
    if (!PyList_Check(a) || !PyList_Check(b)) {
        PyErr_SetString(PyExc_ValueError, "cdifflib only supports lists for both sequences");
        return NULL;
    }

    //printf("matching_blocks 3\n");
    la = PyList_GET_SIZE(a);
    lb = PyList_GET_SIZE(b);

    matching = PyList_New(0);

    matching_block_helper(self, a, b, isbjunk, matching, 0, la, 0, lb);

    //printf("matching_blocks 4\n");
    if (isbjunk)
        Py_DECREF(isbjunk);
    Py_DECREF(a);
    Py_DECREF(b);
    // don't decrement matching, put it straight in to the return val
    return Py_BuildValue("N", matching);
}


static PyObject *
chain_b(PyObject *module, PyObject *args)
{
    long n;
    Py_ssize_t i;

    // These are temporary and are decremented after use
    PyObject *b, *isjunk, *fast_b, *self;

    // These are needed through the function and are decremented at the end
    PyObject *junk = NULL, *popular = NULL, *b2j = NULL, *retval = NULL, *autojunk = NULL;

    //printf("chain_b\n");

    if (!PyArg_ParseTuple(args, "O", &self))
        goto error;

    b = PyObject_GetAttrString(self, "b");
    if (b == NULL || b == Py_None)
        goto error;
    b2j = PyDict_New();
    PyObject_SetAttrString(self, "b2j", b2j);

    /* construct b2j here */
    //printf("construct b2j\n");
    assert(PySequence_Check(b));
    fast_b = PySequence_Fast(b, "accessing sequence 2");
    Py_DECREF(b);
    n = PySequence_Fast_GET_SIZE(fast_b);
    for (i = 0; i < n; i++)
    {
        PyObject *iint;
        PyObject *indices;
        PyObject *elt = PySequence_Fast_GET_ITEM(fast_b, i);
        assert(elt && elt != Py_None);
        indices = PyDict_GetItem(b2j, elt);
        assert(indices == NULL || indices != Py_None);
        if (indices == NULL)
        {
            if (PyErr_Occurred())
            {
                if (!PyErr_ExceptionMatches(PyExc_KeyError))
                {
                    Py_DECREF(fast_b);
                    goto error;
                }
                PyErr_Clear();
            }

            indices = PyList_New(0);
            PyDict_SetItem(b2j, elt, indices);
            Py_DECREF(indices);
        }
        iint = PyInt_FromLong(i);
        PyList_Append(indices, iint);
        Py_DECREF(iint);
    }
    Py_DECREF(fast_b);

    assert(!PyErr_Occurred());

    //printf("determine junk\n");
    junk = PySet_New(NULL);
    isjunk = PyObject_GetAttrString(self, "isjunk");
    if (isjunk != NULL && isjunk != Py_None)
    {
        PyObject *keys = PyDict_Keys(b2j);
        PyObject *fastkeys;
        assert(PySequence_Check(keys));
        fastkeys = PySequence_Fast(keys, "dict keys");
        Py_DECREF(keys);
        /* call isjunk here */
        for (i = 0; i < PySequence_Fast_GET_SIZE(fastkeys); i++)
        {
            PyObject *elt = PySequence_Fast_GET_ITEM(fastkeys, i);
            if (call_obj(isjunk, elt))
            {
                PySet_Add(junk, elt);
                PyDict_DelItem(b2j, elt);
            }
        }
        Py_DECREF(fastkeys);
        Py_DECREF(isjunk);
    }

    /* build autojunk here */
    //printf("build autojunk\n");
    popular = PySet_New(NULL);
    autojunk = PyObject_GetAttrString(self, "autojunk");
    assert(autojunk != NULL);
    if (PyObject_IsTrue(autojunk) && n >= 200) {
        long ntest = n/100 + 1;
        PyObject *items = PyDict_Items(b2j);
        long b2jlen = PyList_GET_SIZE(items);
        for (i = 0; i < b2jlen; i++)
        {
            PyObject *tuple = PyList_GET_ITEM(items, i);
            PyObject *elt = PyTuple_GET_ITEM(tuple, 0);
            PyObject *idxs = PyTuple_GET_ITEM(tuple, 1);

            assert(PyList_Check(idxs));

            if (PyList_GET_SIZE(idxs) > ntest)
            {
                PySet_Add(popular, elt);
                PyDict_DelItem(b2j, elt);
            }
        }
        Py_DECREF(items);
    }

    retval = Py_BuildValue("OO", junk, popular);
    assert(!PyErr_Occurred());

error:
    if (b2j)
        Py_DECREF(b2j);
    if (junk)
        Py_DECREF(junk);
    if (popular)
        Py_DECREF(popular);
    if (autojunk)
        Py_DECREF(autojunk);
    return retval;
}

//
// Define functions in this module
//
static PyMethodDef CDiffLibMethods[4] = {
    {"find_longest_match", find_longest_match, METH_VARARGS,
        "c implementation of difflib.SequenceMatcher.find_longest_match"},
    {"chain_b", chain_b, METH_VARARGS,
        "c implementation of most of difflib.SequenceMatcher.__chain_b"},
    {"matching_blocks", matching_blocks, METH_VARARGS,
        "c implementation of part of difflib.SequenceMatcher.get_matching_blocks"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

//
// Module init entrypoint.
//
PyMODINIT_FUNC
init_cdifflib(void)
{
    PyObject *m;

    m = Py_InitModule("_cdifflib", CDiffLibMethods);
    if (m == NULL)
        return;
    // No special initialisation to do at the moment..
}

#endif // PY_MAJOR_VERSION == 2
