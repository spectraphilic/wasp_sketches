Waspmote sketches and libraries developed by LATICE group at UiO.

Current developers:
- J. David Ibáñez
- Simon Filhol 	([simon.filhol@geo.uio.no](simon.filhol@geo.uio.no))
- John Hulth	([john.hulth@geo.uio.no](john.hulth@geo.uio.no))

# Installation

## Option 1 (Linux only)
1. Download the bash script installUIOapi.sh
2. Make sure you have ssh and git installed on your computer
3. run the bash file in your terminal and follow all instruction with

```bash
# create a folder github if needed
mkdir github
cd github

sh installUIOapi.sh
```


## Option 2

(1) Let's say we have everything in one folder. First clone our projects, our
fork of waspmoteapi and the sketches:

    $ mkdir waspmote-uio
    $ cd waspmote-uio
    $ git clone git@github.com:spectraphilic/waspmoteapi-uio.git -b v044
    $ git clone git@github.com:spectraphilic/wasp_sketches.git

(2) Download and install the IDE:

    $ mkdir waspmote-pro-ide-v06.19-linux64
    $ cd waspmote-pro-ide-v06.19-linux64
    $ wget http://downloads.libelium.com/waspmote-pro-ide-v06.19-linux64.zip
    $ unzip waspmote-pro-ide-v06.19-linux64.zip
    $ ./install.sh

(3) Replace the libraries from the IDE by those in our fork of waspmoteapi:

    $ cd waspmote-pro-ide-v06.19-linux64
    $ mv libraries libraries.bak
    $ ln -s ../waspmoteapi-uio/libraries
    $ cd hardware/waspmote/avr/cores
    $ mv waspmote-api waspmote-api.bak
    $ ln -s ../../../../../waspmoteapi-uio/waspmote-api

(4) Open the Preferences dialog in the IDE and change the "Sketchbook location"
to point to the sketches project (where the libraries folder is). For example:

    [...]/wasp_sketches


# Using a different compiler

It's encouraged to not use the compiler included in the IDE, because it's too
old (GCC 4.9). A newer compiler will give more and better errors and warnings.

To do this first you need to install a crosscompiler for AVR. This is easy in
Debian and Ubuntu (though the compiler won't be much newer, just 5.4):

    apt-get install avr-libc gcc-avr

Now, to change the compiler used edit the hardware/waspmote/avr/platform.txt
file and do the following changes:

    $ vi hardware/waspmote/avr/platform.txt
    [...]
    #compiler.path={runtime.tools.avr-gcc.path}/bin/
    compiler.path=/usr/bin/
    [...]


# Contents

See the README files within the libraries and sketches folders for a
description of the libraries and sketches.
