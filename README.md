## Build
Requires building the fungine submodule first.
Instructions can be found here https://github.com/Mopp123/fungine

### Unix
Run `make` or `make build`.
These creates `build` directory containing the built executable.
All required shared libraries are also copied into this directory so
you should be able to just run the executable.

## Run
To run you also need to have "res" dir in same directory as the executable.
This can be downloaded from here: https://drive.google.com/drive/folders/1nLculCdtXxjtFpnJiZ4N8uuBhs9JgNER?usp=sharing

On unix, you need to set your current directory to the executable's dir for
it to be able to load the resources correctly.

Some small settings can be adjusted modifying the res directory's config.txt.
