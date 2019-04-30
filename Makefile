build:
	python setup.py build

install:
	python setup.py install

test:
	python setup.py test

clean:
	python setup.py clean
	rm -rf build/
	rm -rf dist/
	rm -rf CDiffLib.egg-info
	rm -f _cdifflib.so
	rm -f *.pyc
	rm -f tests/*.pyc

upload:
	python setup.py sdist upload
	python setup.py bdist upload
	python setup.py bdist_egg upload
	python setup.py bdist_wheel upload
	python3.4 setup.py bdist_egg upload
	python3.4 setup.py bdist_wheel upload
	python3.6 setup.py bdist_egg upload
	python3.6 setup.py bdist_wheel upload
	python3.7 setup.py bdist_egg upload
	python3.7 setup.py bdist_wheel upload
