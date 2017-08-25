Waspmote sketches and libraries developed by LATICE group at UiO.

Current developers:
- John Hulth	([john.hulth@geo.uio.no](john.hulth@geo.uio.no))
- Simon Filhol 	([simon.filhol@geo.uio.no](simon.filhol@geo.uio.no))

# Installation

Installation in 2 steps.

Replace the libraries from the ide by those in our fork of waspmoteapi:

    $ git clone https://github.com/spectraphilic/waspmoteapi-uio.git waspmoteapi
    $ cd [...]/waspmote-pro-ide-v06.02
    $ mv libraries libraries.bak

Open the Preferences dialog in the IDE and change the "Sketchbook location"
to point to the project root (where the libraries folder is). For example:

    [...]/wasp_sketches

*Alternative*: Open the Preferences dialog in the IDE to find out where is your
"Sketchbook location". For example mine is "~/Arduino". Then change directory
to the libraries folder within:

    $ cd ~/Arduino/libraries

Create symbolic links to the libraries needed:

    $ ln -s [...]/wasp_libraries/Coroutines
    $ ln -s [...]/wasp_libraries/WString
    $ ln -s [...]/wasp_libraries/SDI12
    $ ln -s [...]/wasp_libraries/WaspUIO


# Contents

See the README files within the libraries and sketches folders for a
drescription of the libraries and sketches.
