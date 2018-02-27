### Build instructions for OSX

brew install qt5
# requires 5.10 or later, you may need brew update qt5 instead

git clone https://gitlab.com/b0/matrique
cd matrique

# pull in libqmatrixclient
git submodule init
git submodule update

/usr/local/Cellar/qt5/5.10.1/bin/qmake 
make

open matrique.app/Contents/MacOS/matrique 
