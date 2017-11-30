install:
	virtualenv venv27
	./venv27/bin/pip install -r requirements.txt
	mkdir -p log
	mkdir -p data
	bash supervisor.sh > supervisor.conf
