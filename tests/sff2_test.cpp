#include <sff2/sff2.h>

#include <cstdio>
#include <exception>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace {

class CheckFailure : public std::exception {};

void check(bool condition, const char* expression, const char* file, int line) {
    if (condition) return;
    std::fprintf(stderr, "%s:%d: check failed: %s\n", file, line, expression);
    throw CheckFailure();
}

#define CHECK(expression) \
    check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)

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
    CHECK(output.addPalette(palette));

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
    CHECK(output.addSprite(first));

    // Verify that multiple payload slots are independently written.
    sff2::Sprite second = first;
    second.group = 10;
    second.index = 5;
    second.width = 2;
    second.height = 2;
    second.pixels = std::vector<uint8_t>{1, 2, 3, 0};
    CHECK(output.addSprite(second));
    CHECK(!output.isOpen());
    CHECK(output.save(path));
    CHECK(!output.isOpen());

    // Assert key offsets against the published SFFv2 header, independently of
    // the reader (signature, sprite table at byte 36, palette table at 44).
    {
        std::ifstream binary(path.c_str(), std::ios::binary);
        std::vector<unsigned char> header(52);
        binary.read(reinterpret_cast<char*>(header.data()), header.size());
        CHECK(binary.gcount() == static_cast<std::streamsize>(header.size()));
        CHECK(std::string(reinterpret_cast<char*>(header.data()), 11) ==
               "ElecbyteSpr");
        CHECK(header[15] == 2);
        CHECK(header[16] == 0 && header[20] == 0); // reserved words
        CHECK(header[24] == 0 && header[25] == 0 &&
               header[26] == 0 && header[27] == 2); // compatible 2.0.0.0
        CHECK(header[36] == 0 && header[37] == 2); // 512, little endian
        CHECK(header[44] == 56 && header[45] == 2); // 512 + 2 * 28
    }

    sff2::SFFFile input;
    CHECK(input.open(path));
    CHECK(input.isOpen());
    CHECK(input.info().signature == "ElecbyteSpr");
    CHECK(input.info().version == 0x02000100u);
    CHECK(input.info().compatible_version == 0x02000000u);
    CHECK(input.sprites().size() == 2);
    CHECK(input.palettes().size() == 1);

    sff2::Sprite* loaded_first = input.findSprite(0, 0);
    sff2::Sprite* loaded_second = input.findSprite(10, 5);
    CHECK(loaded_first && loaded_second);
    CHECK(input.decodeSprite(*loaded_first));
    CHECK(input.decodeSprite(*loaded_second));
    CHECK(loaded_first->pixels == first.pixels);
    CHECK(loaded_second->pixels == second.pixels);
    CHECK(loaded_first->axis.x == 2 && loaded_first->axis.y == 7);

    const sff2::Palette* loaded_palette = input.findPalette(1, 1);
    CHECK(loaded_palette && loaded_palette->entries.size() == 4);
    CHECK(loaded_palette->entries[1].r == 255);

    const std::string copy = temporaryPath("sff2-roundtrip-copy-test.sff");
    CHECK(input.save(copy));
    CHECK(input.isOpen());
    sff2::SFFFile reopened;
    CHECK(reopened.open(copy));
    sff2::Sprite* copied = reopened.findSprite(10, 5);
    CHECK(copied && reopened.decodeSprite(*copied));
    CHECK(copied->pixels == second.pixels);

    std::remove(path.c_str());
    std::remove(copy.c_str());
}

void testOriginalCompressionSemantics() {
    const std::string rle5_path = temporaryPath("sff2-rle5-test.sff");
    {
        sff2::SFFFile file;
        sff2::Sprite sprite;
        sprite.width = 5;
        sprite.height = 1;
        sprite.format = sff2::SpriteFormat::Rle5;
        sprite.color_depth = 5;
        // Decoded length prefix, then:
        // two 7s (stored run 1), one data packet, three 3s (stored run 2).
        sprite.encoded_data =
            std::vector<uint8_t>{5, 0, 0, 0, 1, 0x81, 7, 0x43};
        CHECK(file.addSprite(sprite));
        CHECK(file.save(rle5_path));
    }
    {
        sff2::SFFFile file;
        CHECK(file.open(rle5_path));
        sff2::Sprite* sprite = file.findSprite(0, 0);
        CHECK(sprite && file.decodeSprite(*sprite));
        CHECK(sprite->pixels == std::vector<uint8_t>({7, 7, 3, 3, 3}));
    }
    std::remove(rle5_path.c_str());

    const std::string lz5_path = temporaryPath("sff2-lz5-test.sff");
    {
        sff2::SFFFile file;
        sff2::Sprite sprite;
        sprite.width = 6;
        sprite.height = 1;
        sprite.format = sff2::SpriteFormat::Lz5;
        sprite.color_depth = 5;
        // Decoded length prefix, then a short RLE run of three 2s followed by
        // a short LZ copy of three bytes from distance three.
        sprite.encoded_data =
            std::vector<uint8_t>{6, 0, 0, 0, 0x02, 0x62, 0x02, 0x02};
        CHECK(file.addSprite(sprite));
        CHECK(file.save(lz5_path));
    }
    {
        sff2::SFFFile file;
        CHECK(file.open(lz5_path));
        sff2::Sprite* sprite = file.findSprite(0, 0);
        CHECK(sprite && file.decodeSprite(*sprite));
        CHECK(sprite->pixels ==
               std::vector<uint8_t>({2, 2, 2, 2, 2, 2}));
    }
    std::remove(lz5_path.c_str());

    const std::string long_lz5_path =
        temporaryPath("sff2-long-lz5-test.sff");
    {
        sff2::SFFFile file;
        sff2::Sprite sprite;
        sprite.width = 7;
        sprite.height = 1;
        sprite.format = sff2::SpriteFormat::Lz5;
        sprite.color_depth = 5;
        // Three 1s followed by a long LZ copy: distance 3, length 4.
        sprite.encoded_data =
            std::vector<uint8_t>{7, 0, 0, 0, 0x02, 0x61, 0, 2, 1};
        CHECK(file.addSprite(sprite));
        CHECK(file.save(long_lz5_path));
    }
    {
        sff2::SFFFile file;
        CHECK(file.open(long_lz5_path));
        sff2::Sprite* sprite = file.findSprite(0, 0);
        CHECK(sprite && file.decodeSprite(*sprite));
        CHECK(sprite->pixels ==
               std::vector<uint8_t>({1, 1, 1, 1, 1, 1, 1}));
    }
    std::remove(long_lz5_path.c_str());
}

void testLinks() {
    const std::string path = temporaryPath("sff2-links-test.sff");
    sff2::SFFFile file;

    sff2::Palette palette;
    palette.group = 1;
    palette.index = 1;
    palette.entries.push_back(sff2::PaletteEntry(0, 0, 0, 0));
    CHECK(file.addPalette(palette));
    sff2::Palette linked_palette;
    linked_palette.group = 1;
    linked_palette.index = 2;
    linked_palette.linked = true;
    linked_palette.link_index = 0;
    linked_palette.color_count = 1;
    CHECK(file.addPalette(linked_palette));

    sff2::Sprite sprite;
    sprite.width = sprite.height = 1;
    sprite.format = sff2::SpriteFormat::Raw;
    sprite.color_depth = 8;
    sprite.pixels.push_back(0);
    sprite.decoded = true;
    CHECK(file.addSprite(sprite));
    sff2::Sprite linked;
    linked.group = 1;
    linked.linked = true;
    linked.link_index = 0;
    linked.width = linked.height = 1;
    linked.color_depth = 8;
    CHECK(file.addSprite(linked));
    CHECK(file.save(path));

    sff2::SFFFile input;
    CHECK(input.open(path));
    sff2::Sprite* loaded = input.findSprite(1, 0);
    CHECK(loaded && loaded->linked);
    CHECK(input.decodeSprite(*loaded));
    CHECK(loaded->pixels.size() == 1 && loaded->pixels[0] == 0);
    CHECK(input.findPalette(1, 2)->entries.size() == 1);
    std::remove(path.c_str());
}

void testRejectsEmptyNonLinkedPayloads() {
    sff2::SFFFile file;

    sff2::Palette palette;
    CHECK(!file.addPalette(palette));
    CHECK(file.lastError().find("entry") != std::string::npos);

    sff2::Sprite encoded_sprite;
    CHECK(!file.addSprite(encoded_sprite));
    CHECK(file.lastError().find("payload") != std::string::npos);

    sff2::Sprite decoded_sprite;
    decoded_sprite.decoded = true;
    CHECK(!file.addSprite(decoded_sprite));
    CHECK(file.lastError().find("payload") != std::string::npos);

    palette.entries.push_back(sff2::PaletteEntry());
    CHECK(file.addPalette(palette));
    file.findPalette(0, 0)->entries.clear();
    CHECK(!file.save(temporaryPath("sff2-empty-palette-test.sff")));
    CHECK(file.lastError().find("payload") != std::string::npos);
    std::remove("sff2-empty-palette-test.sff");

    sff2::SFFFile sprite_file;
    decoded_sprite.width = decoded_sprite.height = 1;
    decoded_sprite.color_depth = 8;
    decoded_sprite.pixels.push_back(0);
    CHECK(sprite_file.addSprite(decoded_sprite));
    sprite_file.findSprite(0, 0)->pixels.clear();
    CHECK(!sprite_file.save(temporaryPath("sff2-empty-sprite-test.sff")));
    CHECK(sprite_file.lastError().find("payload") != std::string::npos);
    std::remove("sff2-empty-sprite-test.sff");
}

void testBadFiles() {
    const std::string truncated = temporaryPath("sff2-truncated-test.sff");
    {
        std::ofstream stream(truncated.c_str(), std::ios::binary);
        stream.write("ElecbyteSpr", 11);
    }
    sff2::SFFFile file;
    CHECK(!file.open(truncated));
    CHECK(!file.lastError().empty());
    CHECK(!file.isOpen());
    std::remove(truncated.c_str());
}

void testStreamContract() {
    sff2::SFFFile file;
    std::unique_ptr<sff2::FileStream> unopened = sff2::createStdFileStream();
    CHECK(!file.open(std::move(unopened)));
    CHECK(file.lastError().find("not open") != std::string::npos);
}

} // namespace

int main() {
    struct Test {
        const char* name;
        void (*run)();
    };
    const Test tests[] = {
        {"testCreateAndRoundTrip", testCreateAndRoundTrip},
        {"testLinks", testLinks},
        {"testRejectsEmptyNonLinkedPayloads", testRejectsEmptyNonLinkedPayloads},
        {"testOriginalCompressionSemantics", testOriginalCompressionSemantics},
        {"testBadFiles", testBadFiles},
        {"testStreamContract", testStreamContract},
    };

    bool passed = true;
    for (size_t item = 0; item < sizeof(tests) / sizeof(tests[0]); ++item) {
        try {
            tests[item].run();
        } catch (const CheckFailure&) {
            std::fprintf(stderr, "%s failed\n", tests[item].name);
            passed = false;
        }
    }
    return passed ? 0 : 1;
}
