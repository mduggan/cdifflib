build:
	python setup.py build

install:
	python setup.py install

test:
	python setup.py test

clean:
	python setup.py clean
	rm -rf build/
	rm -rf CDiffLib.egg-info
	rm -f _cdifflib.so
	rm -f *.pyc
	rm -f tests/*.pyc

