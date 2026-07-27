#include <sff2/sff2.h>

#include <cassert>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace {

std::string temporaryPath(const char* name) {
    return std::string(name);
}

void testCreateAndRoundTrip() {
    const std::string path = temporaryPath("sff2-roundtrip-test.sff");

    sff2::SFFFile output;
    sff2::Palette palette;
    palette.group = 1;
    palette.index = 1;
    palette.entries.push_back(sff2::PaletteEntry(0, 0, 0, 0));
    palette.entries.push_back(sff2::PaletteEntry(255, 0, 0, 255));
    palette.entries.push_back(sff2::PaletteEntry(0, 255, 0, 255));
    palette.entries.push_back(sff2::PaletteEntry(0, 0, 255, 255));
    assert(output.addPalette(palette));

    sff2::Sprite first;
    first.group = 0;
    first.index = 0;
    first.width = 4;
    first.height = 2;
    first.axis = sff2::Axis(2, 7);
    first.format = sff2::SpriteFormat::Rle8;
    first.color_depth = 8;
    first.palette_index = 0;
    first.pixels = std::vector<uint8_t>{0, 1, 1, 0, 2, 3, 3, 2};
    first.decoded = true;
    assert(output.addSprite(first));

    // Verify that multiple payload slots are independently written.
    sff2::Sprite second = first;
    second.group = 10;
    second.index = 5;
    second.width = 2;
    second.height = 2;
    second.pixels = std::vector<uint8_t>{1, 2, 3, 0};
    assert(output.addSprite(second));
    assert(output.save(path));

    // Assert key offsets against the published SFFv2 header, independently of
    // the reader (signature, sprite table at byte 36, palette table at 44).
    {
        std::ifstream binary(path.c_str(), std::ios::binary);
        std::vector<unsigned char> header(52);
        binary.read(reinterpret_cast<char*>(header.data()), header.size());
        assert(binary.gcount() == static_cast<std::streamsize>(header.size()));
        assert(std::string(reinterpret_cast<char*>(header.data()), 11) ==
               "ElecbyteSpr");
        assert(header[15] == 2);
        assert(header[36] == 0 && header[37] == 2); // 512, little endian
        assert(header[44] == 56 && header[45] == 2); // 512 + 2 * 28
    }

    sff2::SFFFile input;
    assert(input.open(path));
    assert(input.isOpen());
    assert(input.info().signature == "ElecbyteSpr");
    assert(input.info().version == 0x02000100u);
    assert(input.sprites().size() == 2);
    assert(input.palettes().size() == 1);

    sff2::Sprite* loaded_first = input.findSprite(0, 0);
    sff2::Sprite* loaded_second = input.findSprite(10, 5);
    assert(loaded_first && loaded_second);
    assert(input.decodeSprite(*loaded_first));
    assert(input.decodeSprite(*loaded_second));
    assert(loaded_first->pixels == first.pixels);
    assert(loaded_second->pixels == second.pixels);
    assert(loaded_first->axis.x == 2 && loaded_first->axis.y == 7);

    const sff2::Palette* loaded_palette = input.findPalette(1, 1);
    assert(loaded_palette && loaded_palette->entries.size() == 4);
    assert(loaded_palette->entries[1].r == 255);

    const std::string copy = temporaryPath("sff2-roundtrip-copy-test.sff");
    assert(input.save(copy));
    sff2::SFFFile reopened;
    assert(reopened.open(copy));
    sff2::Sprite* copied = reopened.findSprite(10, 5);
    assert(copied && reopened.decodeSprite(*copied));
    assert(copied->pixels == second.pixels);

    std::remove(path.c_str());
    std::remove(copy.c_str());
}

void testLinks() {
    const std::string path = temporaryPath("sff2-links-test.sff");
    sff2::SFFFile file;

    sff2::Palette palette;
    palette.group = 1;
    palette.index = 1;
    palette.entries.push_back(sff2::PaletteEntry(0, 0, 0, 0));
    assert(file.addPalette(palette));
    sff2::Palette linked_palette;
    linked_palette.group = 1;
    linked_palette.index = 2;
    linked_palette.linked = true;
    linked_palette.link_index = 0;
    linked_palette.color_count = 1;
    assert(file.addPalette(linked_palette));

    sff2::Sprite sprite;
    sprite.width = sprite.height = 1;
    sprite.format = sff2::SpriteFormat::Raw;
    sprite.color_depth = 8;
    sprite.pixels.push_back(0);
    sprite.decoded = true;
    assert(file.addSprite(sprite));
    sff2::Sprite linked;
    linked.group = 1;
    linked.linked = true;
    linked.link_index = 0;
    linked.width = linked.height = 1;
    linked.color_depth = 8;
    assert(file.addSprite(linked));
    assert(file.save(path));

    sff2::SFFFile input;
    assert(input.open(path));
    sff2::Sprite* loaded = input.findSprite(1, 0);
    assert(loaded && loaded->linked);
    assert(input.decodeSprite(*loaded));
    assert(loaded->pixels.size() == 1 && loaded->pixels[0] == 0);
    assert(input.findPalette(1, 2)->entries.size() == 1);
    std::remove(path.c_str());
}

void testBadFiles() {
    const std::string truncated = temporaryPath("sff2-truncated-test.sff");
    {
        std::ofstream stream(truncated.c_str(), std::ios::binary);
        stream.write("ElecbyteSpr", 11);
    }
    sff2::SFFFile file;
    assert(!file.open(truncated));
    assert(!file.lastError().empty());
    assert(!file.isOpen());
    std::remove(truncated.c_str());
}

void testStreamContract() {
    sff2::SFFFile file;
    std::unique_ptr<sff2::FileStream> unopened = sff2::createStdFileStream();
    assert(!file.open(std::move(unopened)));
    assert(file.lastError().find("not open") != std::string::npos);
}

} // namespace

int main() {
    testCreateAndRoundTrip();
    testLinks();
    testBadFiles();
    testStreamContract();
    return 0;
}
