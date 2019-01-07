#!/bin/bash

# Bash script to install Waspmote IDE with UIO sketches and libraries
# Linux ONLY
#
# Simon Filhol, David Ibanez
#
#
# WARNING: before running this script make sure to have git setup with ssh cloning capabilities

printf "WARNING: before running this script make sure to have git setup with ssh cloning capabilities"
printf "\n\t Also, make sure your github account is set up with a ssh key! \n"
printf "\n\n"
printf "Creating folder and clone git UIO repositories ...\n"
mkdir waspmote-uio
cd waspmote-uio
git clone git@github.com:spectraphilic/waspmoteapi-uio.git -b v038
git clone git@github.com:spectraphilic/wasp_sketches.git

# Download Waspmote IDE, and install it
printf "Downloading Waspmote IDE, and installing it ...\n"
mkdir waspmote-pro-ide-v06.12
cd waspmote-pro-ide-v06.12
wget http://downloads.libelium.com/waspmote-pro-ide-v06.12-linux64.zip
unzip waspmote-pro-ide-v06.12-linux64.zip
./install.sh

# create simulink to our own copy of the librayr API
printf "creating symlink to our own copy of the librayr API ...\n"
mv libraries libraries.bak
ln -s ../waspmoteapi-uio/libraries

cd hardware/waspmote/avr/cores/
mv waspmote-api waspmote-api.bak
ln -s ../../../../../waspmoteapi-uio/waspmote-api


printf "Installation almost finished... \n\n Go to the Waspmote IDE Preference: \n\t -change the Sketchbook Location to .../wasp_sketches \n\t - accept changes"
printf "\n"
printf "For more information see \n https://github.com/spectraphilic/wasp_sketches"
