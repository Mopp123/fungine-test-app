# Fungine Test Application
For demonstrating usage of my old game engine project.
NOTE: This contains the same tab issue as the fungine submodule
(originally written with "tab tabs" instead of spaces)

## Build
Requires building the fungine submodule first.
Instructions can be found here https://github.com/Mopp123/fungine

### Linux
Tested only on Ubuntu 22.04.
Run `make` or `make build`.
These creates `build` directory containing the built executable.
All required shared libraries are also copied into this directory so
you should be able to just run the executable.

## Run
To run you also need to have resources directory("res") in the same directory as the executable.
This can be downloaded from here: https://drive.google.com/drive/folders/1nLculCdtXxjtFpnJiZ4N8uuBhs9JgNER?usp=sharing
Disclaimer: I do not own any other assets than the terrain blendmap and heightmap. Others from https://opengameart.org/

On Linux, you need to set your current directory to the executable's dir for
it to be able to load the resources correctly.

Some small settings can be adjusted by modifying the res directory's config.txt.

### Controls
Move: W, A, S, D
Rotate camera: Hold left mouse and move mouse
Change light direction: Arrow keys

There was also set 'E' key to delete grass and palm tree objects for testing purposes which I couldn't bother removing...
