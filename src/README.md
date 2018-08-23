


# Dependencies

get and extract the latest version of ffmpeg

    wget https://ffmpeg.org/releases/ffmpeg-4.0.2.tar.bz2
    tar -xf ffmpeg-4.0.2.tar.bz2
    ln -s ./ffmpeg-4.0.2 ./ffmpeg
    cd ffmpeg

On Ubunutu, ensure yasm is installed and up to date

    sudo apt-get install yasm

configure and build

    ./configure
    make -j 8