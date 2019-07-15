This is a simple GUI timer that can count up and down, and play sound when reaches 0.

## Use instructions
### Overview
The timer show hours, minutes and seconds from left to right. You can set the time by clicking on the section of the timer and typing in the numbers. <br />
On the right side there are three buttons:
1. Top button, starts the timer counting up.
2. Bottom button, starts the timer counting down.
3. Middle button pauses the timer, or if long pressed rests to the value that was set before the countdown started.

When the timer reaches zero it plays a sound. Unless specified otherwise it would be the melno.wav file in the "current directory". In this alarm state all controls are disabled except the middle button which will stop the sound and reset the timer when pressed.<br />
There a usual concept of focus that works with mouse and keyboard controls for it. Tab and Shift+Tab keys will move the focus between the controls. When focused on a section of time display you can input a number. When focused on a button you can press it with the Space key (long press would also work).<br />
Note that you can start several instances of the application at the same time, they don't remember any state and should not interfere with each other.

### Special secret dangerous command line parameters
When starting the application from command line there are 8 positional parameters that you can specify:
1. The name of the audio file to be played when timer reaches 0 (default: ./melno.wav). Currently only WAV format is supported. You can specify an empty string to disable sound.
2. The hours to set the timer to (default: 0).
3. The minutes to set the timer to (default: 0).
4. The seconds to set the timer to (default: 0).
5. The primary/foreground color of the UI, specified as a number/code. (default: 0x009dff).
6. The secondary/background color of the UI, specified as a number/code. (default: 0x000000).
7. The frame time in milliseconds (default: 33).
8. The frame time when the application is minimized, in milliseconds (default: 256)

Any invalid input (non existent files, wrong file formats, wrong number formats, etc.) will gracefully crash the application.<br />
The audio file is loaded into memory in its entirety when the application starts, so be careful with large files.



## Build instructions
Below are build instructions for GNU systems (including cygwin on Windows), primarily using `make`. If that does not suit your needs it should not be hard to set this project up with your favorite build system IDE. See the dependencies in the next section. <br />
[Basic setup](docs/1_basic_setup.md) <br />
[Compile and run](docs/2_compile_and_run.md) <br />
[More on some of these makeshift tools](docs/3_more_on_tools.md)

## Dependencies
[libsdl2](https://libsdl.org) <br />
[libsimple_graphical](https://notabug.org/namark/libsimple_graphical) <br />
[libsimple_interactive](https://notabug.org/namark/libsimple_interactive) <br />
[libsimple_musical](https://notabug.org/namark/libsimple_musical) <br />
[libsimple_geom](https://notabug.org/namark/libsimple_geom) <br />
[libsimple_support](https://notabug.org/namark/libsimple_support) <br />
[libsimple_sdlcore](https://notabug.org/namark/libsimple_sdlcore) <br />
[cpp_tools](https://notabug.org/namark/cpp_tools) <br />

The SDL2 headers are expected to be in a directory named SDL2. libsimple are all static libraries. cpp_tools is only necessary for building with GNU make as outlined in the previous section.<br />
There are also source archives in releases, which have all the simple libraries included, so using that you won't have to individually setup those.

## Licensing
COPYRIGHT and LICENSE apply to all the files in this repository unless otherwise noted in the files themselves.
