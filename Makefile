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
	python setup.py sdist
	python setup.py bdist_egg
	python setup.py bdist_wheel
	python3.4 setup.py bdist_egg
	python3.4 setup.py bdist_wheel
	python3.6 setup.py bdist_egg
	python3.6 setup.py bdist_wheel
	python3.7 setup.py bdist_egg
	python3.7 setup.py bdist_wheel
	twine check dist/*
	twine upload dist/*
