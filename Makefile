build:
	python setup.py build

install:
	python setup.py install

clean:
	python setup.py clean
	rm -rf build/
	rm -f *.pyc
	rm -f tests/*.pyc

test:
	python -m unittest discover -v -s . -t . -p *_tests.py
