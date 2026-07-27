# sff2

A small C++11 library for reading, inspecting, creating, and writing Elecbyte
SFFv2 sprite files.

The library reads SFF 2.0 and 2.0.1 metadata, palettes, links, local data,
translated data, raw pixel data, RLE8, RLE5, and LZ5. It writes interoperable
SFF 2.0.1 files and encodes decoded indexed sprites as RLE8. PNG payloads are
retained and can be written back unchanged, but PNG decoding is intentionally
left to an image library chosen by the application.

## Build

```sh
cmake -S . -B build
cmake --build build
(cd build && ctest)
```

The public API is in `<sff2/sff2.h>`. The `sff2_example` program demonstrates
opening a file, inspecting it, decoding its first sprite when supported, and
writing a copy:

```sh
./build/sff2_example input.sff output.sff
```

## Basic use

```cpp
sff2::SFFFile file;
if (!file.open("character.sff")) {
    std::cerr << file.lastError() << '\n';
    return;
}

if (sff2::Sprite* sprite = file.findSprite(0, 0)) {
    if (file.decodeSprite(*sprite)) {
        // sprite->pixels now contains indexed, RGB, or RGBA pixels.
    }
}

if (!file.save("copy.sff"))
    std::cerr << file.lastError() << '\n';
```

Custom streams can be supplied through `FileStream`; input/output streams must
already be open before they are passed to `SFFFile`.
