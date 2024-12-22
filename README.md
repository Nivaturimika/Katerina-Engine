# Kat's PA Engine

Kat's PA Engine is a fork of the Project Alice codebase (an open source "retro clone" of Victoria 2). But more geared towards a purist audience. If you do prefer a "different" flavour of Victoria 2, feel free to checkout [Project Alice](https://github.com/schombert/Project-Alice/) too!

Join us on [discord](https://discord.gg/3WNyk7SWvf).

## How to install and play
- [Download the latest release here](https://github.com/Nivaturimika/Katerina-Engine/releases)
- Extract the archive and move the executable files and the assets folder into your main Victoria 2 directory. In other words, **KatEngine.exe should end up in the same folder as v2game.exe**.
- IF YOU INSTALLED A PREVIOUS DEMO OR RELEASE: I suggest that you delete it and the old assets folder before copying over the new one, as an extra precaution.
- Start the game by running KatEngine_Launcher.exe.
- Mods may or may not run with more or less problems (see the October update and the compatibility patches channel in the discord for more information about particular mods).
- Read up the [User and multiplayer guide](https://github.com/Nivaturimika/Katerina-Engine/blob/main/docs/user_guide.md) for further information and troubleshooting

## Interested in contributing?

We would recommend joining the Discord as the main development activity occurs there.

### Building the project

The build uses CMake and should "just work", with some annoying exceptions.

#### Windows

1. You will need a full copy of git installed (https://git-scm.com/downloads), as the one that ships with Visual Studio is not sufficient for CMake, for some reason.
2. Make sure that you have a relatively up-to-date version of the Windows SDK installed (you can update your local version of the Windows SDK through the Visual Studio installer).
3. Open the project in Visual Studio and let it configure (CMake should run in the output window and download dependencies; this may take some time).
4. Go look at the "Final Touches" section at the bottom of this page.

If you experience problems with audio playback, you may wish to install Windows Media Player and Windows Media Feature Pack (https://support.microsoft.com/en-us/windows/get-windows-media-player-81718e0d-cfce-25b1-aee3-94596b658287) to be able to properly playback MP3 files (the music jukebox).

#### Linux (Debian-based distro)

Make sure to install the required dependencies.

```bash
sudo apt update
sudo apt install git build-essential clang cmake libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libicu-dev
```

After the installation is done, download the `cmake-tools` extension for Visual Studio Code and open the project. CMake should automatically configure the project for you.

If you get the error, `/usr/bin/ld: cannot find -lstdc++`, it might be caused by a broken gcc-12 package in Ubuntu jammy.
```bash
sudo apt remove gcc-12
```
https://stackoverflow.com/questions/67712376/after-updating-gcc-clang-cant-find-libstdc-anymore

(Linux only) The version of Intel's TBB library we use seems to fail to compile if you have any spaces in the path, so you need to make sure that wherever you put the project has no spaces anywhere in its path (yes, this seems dumb to me too).

#### Linux (Generic)

This is for non-Debian based distributions, utilities needed:
Basic Compiling Tools, (C Compiler & C++ Compiler)
CMake
Git
onetbb
GLFW3
X11 (support *should* exist for wayland but there is not guarantee.)

With Arch-based distrobutions you can use the following:
```bash
yay cmake onetbb
```

From here compiling is straightforward
1. `cd Project-KatEngine`
2. do something to make the debugger launch the program in your V2 directory so that it can find the game files (I don't know how this is done on linux)
4. `cmake -B build . -DCMAKE_BUILD_TYPE=Debug -DARCHITECTURE=x64`
5. `cmake --build build -j$(nproc)`

#### Targetting for AArch64 and ARM

Install an Aarch64 cross compiler (if your host arch isn't ARM):
`yay aarch64-linux-gnu-gcc`

Then build as such:
```sh
mkdir -p build-arm
cd build-arm
cmake -DARCHITECTURE=aarch64 -DIMPORT_PATH=${PWD}/../build -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc ..
cmake --build .
```

#### MacOS

If you manage to build this shit in MacOS, tell me how you had such patience and sanity to do so.

#### Final touches

Because the project in its current state needs to use the existing game files (as a source of graphics, for starters), everyone needs to tell the compiler where their copy of the game is installed and to put the new files in that directory as well.

Copy the assets folder to your V2 directory. **Note: when the asset files are updated by you or someone else you will need to copy any changed files to your V2 directory and rebuild any scenarios. Not doing so will probably result in crashes**. Then, you will need to configure your debugger to launch KatEngine and the Launcher *as if* they were located in your V2 directory. For visual studio and visual studio code you can do that by creating a launch configuration file. (See [here for Visual Studio](https://learn.microsoft.com/en-us/visualstudio/ide/customize-build-and-debug-tasks-in-visual-studio?view=vs-2022) and [here for VS code](https://code.visualstudio.com/docs/cpp/launch-json-reference)). The contents of that file will look something like mine (copied below) except with the directory location changed to point to your V2 directory.

```
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "project": "CMakeLists.txt",
      "projectTarget": "KatEngine_Launcher.exe (Launcher\\KatEngine_Launcher.exe)",
      "name": "KatEngine_Launcher.exe (Launcher\\KatEngine_Launcher.exe)",
      "currentDir": "C:\\programs\\V2"
    },
    {
      "type": "default",
      "project": "CMakeLists.txt",
      "projectTarget": "KatEngine.exe",
      "name": "KatEngine.exe",
      "currentDir": "C:\\programs\\V2",
      "args": [
        ""
      ]
    }
  ]
}
```

Note that `args` contains the command line parameters to launch the program with. If you want to debug a specific scenario file, you would change it to something like `"9DCA2D56-0.bin"`.
