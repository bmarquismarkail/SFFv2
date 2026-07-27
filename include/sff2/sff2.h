#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace sff2 {

struct PaletteEntry {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
    PaletteEntry() {}
    PaletteEntry(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
        : r(red), g(green), b(blue), a(alpha) {}
};

// Values stored in an SFFv2 sprite node.
enum class SpriteFormat : uint8_t {
    Raw = 0,
    Rle8 = 2,
    Rle5 = 3,
    Lz5 = 4,
    Png8 = 10,
    Png24 = 11,
    Png32 = 12
};

inline bool isIndexed(SpriteFormat format) {
    return format == SpriteFormat::Rle8 || format == SpriteFormat::Rle5 ||
           format == SpriteFormat::Lz5 || format == SpriteFormat::Png8;
}

inline bool isCompressed(SpriteFormat format) {
    return format == SpriteFormat::Rle8 || format == SpriteFormat::Rle5 ||
           format == SpriteFormat::Lz5;
}

struct Axis {
    int16_t x = 0;
    int16_t y = 0;
    Axis() {}
    Axis(int16_t horizontal, int16_t vertical)
        : x(horizontal), y(vertical) {}
};

struct Sprite {
    uint16_t group = 0;
    uint16_t index = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    Axis axis;
    SpriteFormat format = SpriteFormat::Raw;
    uint8_t color_depth = 8;

    // Index into SFFFile::palettes(), as stored by SFFv2 (not a group number).
    uint16_t palette_index = 0;
    bool linked = false;
    uint16_t link_index = 0;
    bool uses_tdata = false;

    // Decoded pixels. Indexed images contain one byte per pixel. Raw 24/32-bit
    // images contain tightly packed RGB/RGBA bytes.
    std::vector<uint8_t> pixels;
    bool decoded = false;

    // Original payload, retained for unsupported PNG decoding and lossless
    // read/write. Applications normally do not need to use this field.
    std::vector<uint8_t> encoded_data;
};

struct Palette {
    uint16_t group = 0;
    uint16_t index = 0;
    uint16_t color_count = 0;
    bool linked = false;
    uint16_t link_index = 0;
    std::vector<PaletteEntry> entries;
};

class FileStream {
public:
    virtual ~FileStream() {}
    virtual bool open(const std::string& path, bool write) = 0;
    virtual bool close() = 0;
    virtual bool seek(uint32_t position) = 0;
    virtual uint32_t tell() const = 0;
    virtual bool read(void* destination, size_t size) = 0;
    virtual bool write(const void* source, size_t size) = 0;
    virtual uint64_t size() const = 0;
    virtual bool isOpen() const = 0;
};

std::unique_ptr<FileStream> createStdFileStream();

class SFFFile {
public:
    struct Info {
        std::string signature;
        // Packed as major.minor1.minor2.minor3, e.g. 0x02000100 for 2.0.1.0.
        uint32_t version = 0;
        uint32_t compatible_version = 0;
        uint32_t sprite_table_offset = 0;
        uint32_t palette_table_offset = 0;
        uint32_t ldata_offset = 0;
        uint32_t ldata_length = 0;
        uint32_t tdata_offset = 0;
        uint32_t tdata_length = 0;
        size_t sprite_count = 0;
        size_t palette_count = 0;
    };

    SFFFile();
    ~SFFFile();
    SFFFile(const SFFFile&) = delete;
    SFFFile& operator=(const SFFFile&) = delete;
    SFFFile(SFFFile&& other) noexcept;
    SFFFile& operator=(SFFFile&& other) noexcept;

    bool open(const std::string& path);
    // The supplied stream must already be open for reading.
    bool open(std::unique_ptr<FileStream> stream);
    void close();
    bool isOpen() const;

    const Info& info() const { return info_; }
    const std::vector<Palette>& palettes() const { return palettes_; }
    const std::vector<Sprite>& sprites() const { return sprites_; }

    Sprite* findSprite(uint16_t group, uint16_t index);
    const Sprite* findSprite(uint16_t group, uint16_t index) const;
    Palette* findPalette(uint16_t group, uint16_t index);
    const Palette* findPalette(uint16_t group, uint16_t index) const;

    bool decodeSprite(Sprite& sprite) const;
    bool decodeAll();
    bool addPalette(const Palette& palette);
    bool addSprite(const Sprite& sprite);

    bool save(const std::string& path);
    // The supplied stream must already be open for writing.
    bool save(std::unique_ptr<FileStream> stream);
    const std::string& lastError() const { return last_error_; }

private:
    bool readHeader();
    bool readPalettes();
    bool readSprites();
    bool fail(const std::string& message) const;

    Info info_;
    std::vector<Palette> palettes_;
    std::vector<Sprite> sprites_;
    std::unique_ptr<FileStream> stream_;
    bool loaded_ = false;
    mutable std::string last_error_;
};

} // namespace sff2
