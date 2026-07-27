#include <sff2/sff2.h>

#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: sff2_example input.sff output.sff\n";
        return 2;
    }

    sff2::SFFFile file;
    if (!file.open(argv[1])) {
        std::cerr << "Open failed: " << file.lastError() << '\n';
        return 1;
    }

    std::cout << file.info().signature << " version 0x" << std::hex
              << file.info().version << std::dec << '\n'
              << file.sprites().size() << " sprites, "
              << file.palettes().size() << " palettes\n";

    if (!file.sprites().empty()) {
        sff2::Sprite& sprite = *file.findSprite(
            file.sprites()[0].group, file.sprites()[0].index);
        std::cout << "First sprite: " << sprite.group << ',' << sprite.index
                  << " (" << sprite.width << 'x' << sprite.height << ")\n";
        if (!file.decodeSprite(sprite))
            std::cout << "Pixels not decoded: " << file.lastError() << '\n';
    }

    if (!file.save(argv[2])) {
        std::cerr << "Save failed: " << file.lastError() << '\n';
        return 1;
    }
    return 0;
}
