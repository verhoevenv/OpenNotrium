OpenNotrium
===========

[![Build Status](https://travis-ci.com/verhoevenv/OpenNotrium.svg?branch=master)](https://travis-ci.com/verhoevenv/OpenNotrium)

Open source version of Notrium.

Notrium is a top-down survival game originally developed by Ville Mönkkönen.

Join the chat at https://gitter.im/verhoevenv/OpenNotrium

Dependencies
------------

* SDL 2.0
* SDL_mixer 2.0
* SDL_image 2.0
* PhysFS
* CMake for building


How to build - Linux
--------------------

Underneath, Ubuntu is assumed. This probably only affects the package manager, `apt-get`. Please refer to the documentation of your own distro if you are not sure how to execute the following steps.
We will also assume you have the OpenNotrium source files checked out in `~/OpenNotrium`

1. Install CMake

        sudo apt-get install cmake

2. Install dependencies

        sudo apt-get install build-essential libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libphysfs-dev

3. Go to the OpenNotrium source directory and create a new directory for build output

        cd ~/OpenNotrium
        mkdir build
        cd build

4. From this new directory, let cmake generate the Makefile. CMake should end with a message about build files being written. Fix any errors before continuing!

        cmake ..

5. Build OpenNotrium. This also shouldn't produce any errors. A few warnings is fine.

        make

6. You should have the executable OpenNotrium in your build directory. We will create a seperate directory from which to run the program. We also need the runtime files there.

        cd ..
        mkdir out
        cp build/OpenNotrium out
        cp -r runtime_files/* out

7. Run OpenNotrium!

        cd out
        ./OpenNotrium


How to build - Mac
--------------------

We highly suggest using a package manager for Mac, either fink, MacPorts or Homebrew. Underneath Homebrew is used.

1. Get XCode command line tools running

    You should search online for this. It involves registering an Apple developer account, downloading XCode, and installing the command line tools from there.

2. Install CMake

        brew install cmake

3. Install dependencies

        brew install sdl2 sdl2_mixer physfs

4. Follow Linux guidelines from step 3 onwards.


How to build - Windows - Visual Studio
--------------------

These instructions have been tested on Visual Studio Community 2013 and 2015, which are both free to download. Other versions of Visual Studio might or might not work, but the instructions below should still apply. Alternatively, it might be possible to use MinGW to build OpenNotrium, if you manage to do so, please let us know!

1. Install CMake

    Find it from the website.

2. Build physfs

    Physfs doesn't provide binaries on Windows. Google, download and unpack. Then, follow the instructions they provide in INSTALL.txt, which is mainly to use CMake.

3. Place the created libraries in the physfs directory, under a `lib` subdirectory. That is where CMake expects to find them

4. Download and unpack [SDL 2.0](https://www.libsdl.org/download-2.0.php), [SDL_image 2.0](https://www.libsdl.org/projects/SDL_image/) and [SDL_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/).

    Make sure to get the VS-development archives. It is highly recommended to unpack the libraries to the same directory, this helps with the next steps.

5. Create environment variables for the libraries so CMake can find them. You can do this by typing the below in a command prompt. Alternatively, use the control panel. Set the paths to where you unpacked.

        setx SDL2DIR c:\SDL2
        setx PHYSFSDIR c:\physfs

6. Create a project file using CMake

        Similar to steps 3 and 4 under the Linux howto:
        
        mkdir build
        cd build
        :: to build with VS2013:
        cmake .. -G "Visual Studio 12"
        :: to build with VS2015:
        cmake .. -G "Visual Studio 14"
        
        Alternatively, use cmake-gui:

    1. _Where is the source code_: browse to where you placed the OpenNotrium sources
    2. _Where to build the binaries_: copy-paste the above, add `/build` to it to build in a subdirectory
    3. Click `Configure`. Select your preferred option for any dialog the pops up.
    4. The messages at the bottom should end with `Configuring done`, and the configuration options above should be filled out with proper paths. If not, fix errors. Probably your environment variables aren't set up properly, or the physfs libs aren't in a `/libs` directory.
    5. Click `Generate`.


7. Build OpenNotrium by opening the Visual Studio solution file (under the /build subdirectory) and doing a build. If you want an optimized executable, make sure the "Release" target is set. For development purposes, the "Debug" target is ok.

8. Create a new directory named `out` under the OpenNotrium source directory. Copy both the executable (should be under `build\Debug\OpenNotrium.exe`) and the contents of `runtime_files` to it.

9. Search the library directories for any necessary `.dll` files, and copy these to the new `out` directory. This may depend on your system, but you might want the x86 versions of:

    * for physfs

        - physfs.dll

    * for SDL

        - SDL2.dll

    * for SDL_image

        - SDL2_image.dll
        - libjpeg-9.dll
        - libpng16-16.dll
        - libtiff-5.dll
        - libwebp-4.dll
        - zlib1.dll

    * for SDL_mixer

        - SDL2_mixer.dll
        - libFLAC-8.dll
        - libmikmod-2.dll
        - libogg-0.dll
        - libvorbis-0.dll
        - libvorbisfile-3.dll
        - smpeg2.dll
