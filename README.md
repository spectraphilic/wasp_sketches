Waspmote sketches and libraries developed by LATICE group at UiO.

Current developers:
- John Hulth	([john.hulth@geo.uio.no](john.hulth@geo.uio.no))
- Simon Filhol 	([simon.filhol@geo.uio.no](simon.filhol@geo.uio.no))

# Installation

(1) Let's say we have everything in one folder. First clone our projects, our
fork of waspmoteapi and the sketches:

    $ mkdir waspmote-uio
    $ cd waspmote-uio
    $ git clone git@github.com:spectraphilic/waspmoteapi-uio.git
    $ git clone git@github.com:spectraphilic/wasp_sketches.git

(2) Download and install the IDE:

    $ mkdir waspmote-pro-ide-v06.02
    $ cd waspmote-pro-ide-v06.02
    $ wget http://downloads.libelium.com/waspmote-pro-ide-v06.02-linux64.zip
    $ unzip waspmote-pro-ide-v06.02-linux64.zip
    $ ./install.sh

(3) Replace the libraries from the IDE by those in our fork of waspmoteapi:

    $ cd waspmote-pro-ide-v06.02
    $ mv libraries libraries.bak
    $ ln -s ../waspmoteapi-uio/libraries

(4) Open the Preferences dialog in the IDE and change the "Sketchbook location"
to point to the sketches project (where the libraries folder is). For example:

    [...]/wasp_sketches

(4 alternative) Open the Preferences dialog in the IDE to find out where is
your "Sketchbook location". For example mine is "~/Arduino". Then change
directory to the libraries folder within:

    $ cd ~/Arduino/libraries

Create symbolic links to the libraries needed:

    $ ln -s [...]/wasp_libraries/Coroutines
    $ ln -s [...]/wasp_libraries/WString
    $ ln -s [...]/wasp_libraries/SDI12
    $ ln -s [...]/wasp_libraries/WaspUIO


# Contents

See the README files within the libraries and sketches folders for a
drescription of the libraries and sketches.
