Install
=======

Install system wide requirements:

    $ sudo apt-get install python-virtualenv
    $ sudo apt-get install mosquitto mosquitto-clients
    $ sudo apt-get install supervisor

Build:

    $ make install

Create a symbolic link, as root, to the supervisor configuration:

    $ sudo ln -s $PWD/supervisor.conf /etc/supervisor/conf.d/wsn.conf
    $ supervisorctl reread
    $ supervisorctl update
