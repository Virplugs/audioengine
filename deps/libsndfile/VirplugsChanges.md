# Changes made to RtAudio for the use in Virplugs

Base: 1.0.29

- `src/g72x.c`: Renamed file to `_g72x.c`, as filenames need to be unique for node-gyp (there is also a file called `g72x.c` in `src/G72x/`).
