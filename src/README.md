


# Dependencies

dependencies should be placed under an ./alien directory along side build
in order for CMAKE to find the artifacts

## FFmpeg

YASM is required on linux and OSX

On Ubunutu:

    sudo apt-get install yasm

On OSX:

    brew install yasm

get the latest version of ffmpeg

    wget https://ffmpeg.org/releases/ffmpeg-4.0.2.tar.bz2
    tar -xf ffmpeg-4.0.2.tar.bz2
    ln -s ./ffmpeg-4.0.2 ./ffmpeg
    cd ffmpeg

configure and build

    ./configure
    make -j 4

## FFTW

get the latest version of fftw

    wget http://www.fftw.org/fftw-3.3.8.tar.gz
    tar -xf fftw-3.3.8.tar.gz
    ln -s fftw-3.3.8 fftw
    cd fftw

configure and build, enable shared libraries

    ./configure --enable-shared
    make -j 4