# Build instructions for OSX

1. Update Qt to v5.10+

        brew install qt5

2. Download Spectral source

        git clone https://gitlab.com/b0/spectral
        cd spectral

3. Pull in the dependencies

        git submodule init
        git submodule update

4. Build Spectral

        /usr/local/Cellar/qt5/5.10.1/bin/qmake 
        make

5. Open Spectral

        open spectral.app/Contents/MacOS/spectral
