# Bash script to install Waspmote IDE with UIO sketches and libraries
# Linux ONLY
#
# Simon Filhol, David Ibanez
#
#
# WARNING: before running this script make sure to have git setup with ssh cloning capabilities

mkdir waspmote-uio
cd waspmote-uio
git clone git@github.com:spectraphilic/waspmoteapi-uio.git
git clone git@github.com:spectraphilic/wasp_sketches.git

# Download Waspmote IDE, and install it
mkdir waspmote-pro-ide-v06.02
cd waspmote-pro-ide-v06.02
wget http://downloads.libelium.com/waspmote-pro-ide-v06.02-linux64.zip
unzip waspmote-pro-ide-v06.02-linux64.zip
./install.sh

# create simulink to our own copy of the librayr API
cd waspmote-pro-ide-v06.02
mv libraries libraries.bak
ln -s ../waspmoteapi-uio/libraries

ln -s ../../../../../waspmoteapi-uio/waspmote-api

printf "Installation almost finished... \n\n Go to the Waspmote IDE Preference: \n\t -change the Sketchbook Location to .../wasp_sketches \n\t - accept changes"
printf "\n"
printf "For more information see \n https://github.com/spectraphilic/wasp_sketches"
