#include <sff2/sff2.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>
#include <utility>

namespace sff2 {

namespace {

const uint32_t kHeaderSize = 512;
const uint32_t kSpriteNodeSize = 28;
const uint32_t kPaletteNodeSize = 16;
const uint32_t kMaximumItems = 1000000;
const char kSignature[12] = {'E','l','e','c','b','y','t','e','S','p','r','\0'};

uint32_t align4(uint32_t value) { return (value + 3u) & ~3u; }

bool addWithin(uint64_t offset, uint64_t length, uint64_t limit) {
    return offset <= limit && length <= limit - offset;
}

bool readU8(FileStream& stream, uint8_t& value) {
    return stream.read(&value, 1);
}

bool readU16(FileStream& stream, uint16_t& value) {
    uint8_t bytes[2];
    if (!stream.read(bytes, sizeof(bytes))) return false;
    value = static_cast<uint16_t>(bytes[0]) |
            static_cast<uint16_t>(static_cast<uint16_t>(bytes[1]) << 8);
    return true;
}

bool readU32(FileStream& stream, uint32_t& value) {
    uint8_t bytes[4];
    if (!stream.read(bytes, sizeof(bytes))) return false;
    value = static_cast<uint32_t>(bytes[0]) |
            (static_cast<uint32_t>(bytes[1]) << 8) |
            (static_cast<uint32_t>(bytes[2]) << 16) |
            (static_cast<uint32_t>(bytes[3]) << 24);
    return true;
}

bool writeU8(FileStream& stream, uint8_t value) {
    return stream.write(&value, 1);
}

bool writeU16(FileStream& stream, uint16_t value) {
    const uint8_t bytes[2] = {
        static_cast<uint8_t>(value),
        static_cast<uint8_t>(value >> 8)
    };
    return stream.write(bytes, sizeof(bytes));
}

bool writeU32(FileStream& stream, uint32_t value) {
    const uint8_t bytes[4] = {
        static_cast<uint8_t>(value),
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value >> 16),
        static_cast<uint8_t>(value >> 24)
    };
    return stream.write(bytes, sizeof(bytes));
}

uint32_t unpackVersion(const uint8_t bytes[4]) {
    // On disk: minor3, minor2, minor1, major.
    return (static_cast<uint32_t>(bytes[3]) << 24) |
           (static_cast<uint32_t>(bytes[2]) << 16) |
           (static_cast<uint32_t>(bytes[1]) << 8) |
           static_cast<uint32_t>(bytes[0]);
}

void packVersion(uint32_t version, uint8_t bytes[4]) {
    bytes[0] = static_cast<uint8_t>(version);
    bytes[1] = static_cast<uint8_t>(version >> 8);
    bytes[2] = static_cast<uint8_t>(version >> 16);
    bytes[3] = static_cast<uint8_t>(version >> 24);
}

size_t expectedPixelBytes(const Sprite& sprite) {
    const uint64_t pixels = static_cast<uint64_t>(sprite.width) * sprite.height;
    const uint64_t bytes_per_pixel = sprite.color_depth == 24 ? 3 :
                                     sprite.color_depth == 32 ? 4 : 1;
    const uint64_t bytes = pixels * bytes_per_pixel;
    return bytes > std::numeric_limits<size_t>::max()
        ? 0 : static_cast<size_t>(bytes);
}

bool decodeRle8(const std::vector<uint8_t>& source, size_t expected,
                std::vector<uint8_t>& output) {
    output.clear();
    output.reserve(expected);
    size_t position = 0;
    while (output.size() < expected) {
        if (position >= source.size()) return false;
        uint8_t color = source[position++];
        size_t count = 1;
        if ((color & 0xc0u) == 0x40u) {
            count = color & 0x3fu;
            if (count == 0 || position >= source.size() ||
                count > expected - output.size()) return false;
            color = source[position++];
        }
        output.insert(output.end(), count, color);
    }
    return true;
}

bool decodeRle5(const std::vector<uint8_t>& source, size_t expected,
                std::vector<uint8_t>& output) {
    output.clear();
    output.reserve(expected);
    size_t position = 0;
    while (output.size() < expected) {
        if (position + 2 > source.size()) return false;
        // RLE5 stores each run length minus one.
        size_t run_length = static_cast<size_t>(source[position++]) + 1u;
        uint8_t packet = source[position++];
        size_t data_length = packet & 0x7fu;
        uint8_t color = 0;
        if ((packet & 0x80u) != 0) {
            if (position >= source.size()) return false;
            color = source[position++];
        }
        if (run_length > expected - output.size()) return false;
        output.insert(output.end(), run_length, color);
        for (size_t item = 0; item < data_length; ++item) {
            if (position >= source.size() || output.size() >= expected) return false;
            packet = source[position++];
            run_length = static_cast<size_t>(packet >> 5) + 1u;
            color = packet & 0x1fu;
            if (run_length > expected - output.size()) return false;
            output.insert(output.end(), run_length, color);
        }
    }
    return output.size() == expected;
}

bool decodeLz5(const std::vector<uint8_t>& source, size_t expected,
               std::vector<uint8_t>& output) {
    output.clear();
    output.reserve(expected);
    size_t position = 0;
    uint8_t recycled = 0;
    unsigned recycled_bits = 0;
    while (output.size() < expected) {
        if (position >= source.size()) return false;
        const uint8_t control = source[position++];
        for (unsigned bit = 0; bit < 8 && output.size() < expected; ++bit) {
            if (position >= source.size()) return false;
            uint8_t packet = source[position++];
            if ((control & (1u << bit)) == 0) {
                size_t count;
                uint8_t color;
                if ((packet & 0xe0u) == 0) {
                    if (position >= source.size()) return false;
                    count = static_cast<size_t>(source[position++]) + 8u;
                    color = packet & 0x1fu;
                } else {
                    count = packet >> 5;
                    color = packet & 0x1fu;
                }
                if (count > expected - output.size()) return false;
                output.insert(output.end(), count, color);
            } else {
                size_t distance;
                size_t count;
                if ((packet & 0x3fu) == 0) {
                    if (position + 2 > source.size()) return false;
                    distance = (static_cast<size_t>(packet) << 2) |
                               source[position++];
                    distance += 1;
                    count = static_cast<size_t>(source[position++]) + 3u;
                } else {
                    count = static_cast<size_t>(packet & 0x3fu) + 1u;
                    recycled |= static_cast<uint8_t>((packet & 0xc0u) >> recycled_bits);
                    recycled_bits += 2;
                    if (recycled_bits < 8) {
                        if (position >= source.size()) return false;
                        distance = static_cast<size_t>(source[position++]) + 1u;
                    } else {
                        distance = static_cast<size_t>(recycled) + 1u;
                        recycled = 0;
                        recycled_bits = 0;
                    }
                }
                if (distance == 0 || distance > output.size() ||
                    count > expected - output.size()) return false;
                for (size_t item = 0; item < count; ++item)
                    output.push_back(output[output.size() - distance]);
            }
        }
    }
    return true;
}

std::vector<uint8_t> encodeRle8(const std::vector<uint8_t>& pixels) {
    std::vector<uint8_t> output;
    size_t position = 0;
    while (position < pixels.size()) {
        size_t count = 1;
        while (count < 63 && position + count < pixels.size() &&
               pixels[position + count] == pixels[position]) ++count;
        const bool reserved_literal = (pixels[position] & 0xc0u) == 0x40u;
        if (count >= 2 || reserved_literal) {
            output.push_back(static_cast<uint8_t>(0x40u | count));
            output.push_back(pixels[position]);
            position += count;
        } else {
            output.push_back(pixels[position++]);
        }
    }
    return output;
}

bool validFormat(uint8_t value) {
    return value == 0 || value == 2 || value == 3 || value == 4 ||
           value == 10 || value == 11 || value == 12;
}

class StdFileStream : public FileStream {
public:
    StdFileStream() : file_(NULL) {}
    ~StdFileStream() { close(); }

    bool open(const std::string& path, bool write) {
        close();
        file_ = std::fopen(path.c_str(), write ? "w+b" : "rb");
        return file_ != NULL;
    }
    bool close() {
        if (!file_) return true;
        const int result = std::fclose(file_);
        file_ = NULL;
        return result == 0;
    }
    bool seek(uint32_t position) {
        return file_ && std::fseek(file_, static_cast<long>(position), SEEK_SET) == 0;
    }
    uint32_t tell() const {
        if (!file_) return 0;
        const long result = std::ftell(file_);
        return result < 0 ? 0 : static_cast<uint32_t>(result);
    }
    bool read(void* destination, size_t size) {
        return size == 0 || (file_ && std::fread(destination, 1, size, file_) == size);
    }
    bool write(const void* source, size_t size) {
        return size == 0 || (file_ && std::fwrite(source, 1, size, file_) == size);
    }
    uint64_t size() const {
        if (!file_) return 0;
        const long current = std::ftell(file_);
        if (current < 0 || std::fseek(file_, 0, SEEK_END) != 0) return 0;
        const long end = std::ftell(file_);
        std::fseek(file_, current, SEEK_SET);
        return end < 0 ? 0 : static_cast<uint64_t>(end);
    }
    bool isOpen() const { return file_ != NULL; }

private:
    mutable std::FILE* file_;
};

} // namespace

std::unique_ptr<FileStream> createStdFileStream() {
    return std::unique_ptr<FileStream>(new StdFileStream());
}

SFFFile::SFFFile() {}
SFFFile::~SFFFile() { close(); }

SFFFile::SFFFile(SFFFile&& other) noexcept { *this = std::move(other); }

SFFFile& SFFFile::operator=(SFFFile&& other) noexcept {
    if (this != &other) {
        close();
        info_ = other.info_;
        palettes_ = std::move(other.palettes_);
        sprites_ = std::move(other.sprites_);
        stream_ = std::move(other.stream_);
        loaded_ = other.loaded_;
        last_error_ = std::move(other.last_error_);
        other.info_ = Info();
        other.loaded_ = false;
    }
    return *this;
}

bool SFFFile::fail(const std::string& message) const {
    last_error_ = message;
    return false;
}

void SFFFile::close() {
    if (stream_) stream_->close();
    stream_.reset();
    info_ = Info();
    palettes_.clear();
    sprites_.clear();
    loaded_ = false;
}

bool SFFFile::isOpen() const { return loaded_; }

bool SFFFile::readHeader() {
    if (!stream_ || !stream_->isOpen()) return fail("Input stream is not open");
    if (stream_->size() < kHeaderSize || !stream_->seek(0))
        return fail("File is too small for an SFFv2 header");
    char signature[12];
    if (!stream_->read(signature, sizeof(signature)) ||
        std::memcmp(signature, kSignature, sizeof(signature)) != 0)
        return fail("Invalid SFF signature (expected ElecbyteSpr)");
    info_.signature.assign(signature, 11);

    uint8_t version[4], compatible[4];
    if (!stream_->read(version, 4))
        return fail("Incomplete SFF header");
    info_.version = unpackVersion(version);
    if ((info_.version >> 24) != 2)
        return fail("Unsupported SFF version (only SFFv2 is supported)");

    uint32_t reserved;
    // Two reserved words precede the compatibility version.
    for (int item = 0; item < 2; ++item)
        if (!readU32(*stream_, reserved)) return fail("Incomplete SFF header");
    if (!stream_->read(compatible, 4))
        return fail("Incomplete SFF header");
    info_.compatible_version = unpackVersion(compatible);
    // Two more reserved words complete the fixed header prefix.
    for (int item = 0; item < 2; ++item)
        if (!readU32(*stream_, reserved)) return fail("Incomplete SFF header");
    if (!readU32(*stream_, info_.sprite_table_offset) ||
        !readU32(*stream_, reserved) ||
        !readU32(*stream_, info_.palette_table_offset))
        return fail("Incomplete SFF header");
    info_.sprite_count = reserved;
    if (!readU32(*stream_, reserved)) return fail("Incomplete SFF header");
    info_.palette_count = reserved;
    if (!readU32(*stream_, info_.ldata_offset) ||
        !readU32(*stream_, info_.ldata_length) ||
        !readU32(*stream_, info_.tdata_offset) ||
        !readU32(*stream_, info_.tdata_length))
        return fail("Incomplete SFF header");

    const uint64_t file_size = stream_->size();
    if (info_.sprite_count > kMaximumItems || info_.palette_count > kMaximumItems)
        return fail("Unreasonable table item count");
    if (!addWithin(info_.sprite_table_offset,
                   info_.sprite_count * kSpriteNodeSize, file_size) ||
        !addWithin(info_.palette_table_offset,
                   info_.palette_count * kPaletteNodeSize, file_size) ||
        !addWithin(info_.ldata_offset, info_.ldata_length, file_size) ||
        !addWithin(info_.tdata_offset, info_.tdata_length, file_size))
        return fail("SFF header points outside the file");
    return true;
}

bool SFFFile::readPalettes() {
    palettes_.clear();
    palettes_.reserve(info_.palette_count);
    for (size_t item = 0; item < info_.palette_count; ++item) {
        const uint32_t node = info_.palette_table_offset +
                              static_cast<uint32_t>(item * kPaletteNodeSize);
        if (!stream_->seek(node)) return fail("Could not seek to palette node");
        Palette palette;
        uint32_t offset, length;
        if (!readU16(*stream_, palette.group) ||
            !readU16(*stream_, palette.index) ||
            !readU16(*stream_, palette.color_count) ||
            !readU16(*stream_, palette.link_index) ||
            !readU32(*stream_, offset) || !readU32(*stream_, length))
            return fail("Incomplete palette node");
        palette.linked = length == 0;
        if (palette.linked) {
            if (palette.link_index >= item)
                return fail("Palette link does not refer to an earlier node");
            palette.entries = palettes_[palette.link_index].entries;
        } else {
            if (length % 4 != 0 || length / 4 < palette.color_count ||
                !addWithin(offset, length, info_.ldata_length))
                return fail("Invalid palette data range");
            if (!stream_->seek(info_.ldata_offset + offset))
                return fail("Could not seek to palette data");
            palette.entries.resize(palette.color_count);
            for (size_t color = 0; color < palette.entries.size(); ++color) {
                uint8_t rgba[4];
                if (!stream_->read(rgba, 4)) return fail("Incomplete palette data");
                palette.entries[color].r = rgba[0];
                palette.entries[color].g = rgba[1];
                palette.entries[color].b = rgba[2];
                palette.entries[color].a = rgba[3];
                // SFF 2.0 does not store meaningful alpha.
                if (((info_.version >> 8) & 0xffu) == 0) {
                    palette.entries[color].a = color == 0 ? 0 : 255;
                }
            }
        }
        palettes_.push_back(palette);
    }
    return true;
}

bool SFFFile::readSprites() {
    sprites_.clear();
    sprites_.reserve(info_.sprite_count);
    for (size_t item = 0; item < info_.sprite_count; ++item) {
        const uint32_t node = info_.sprite_table_offset +
                              static_cast<uint32_t>(item * kSpriteNodeSize);
        if (!stream_->seek(node)) return fail("Could not seek to sprite node");
        Sprite sprite;
        uint16_t x, y, flags;
        uint8_t format;
        uint32_t offset, length;
        if (!readU16(*stream_, sprite.group) ||
            !readU16(*stream_, sprite.index) ||
            !readU16(*stream_, sprite.width) ||
            !readU16(*stream_, sprite.height) ||
            !readU16(*stream_, x) || !readU16(*stream_, y) ||
            !readU16(*stream_, sprite.link_index) ||
            !readU8(*stream_, format) ||
            !readU8(*stream_, sprite.color_depth) ||
            !readU32(*stream_, offset) || !readU32(*stream_, length) ||
            !readU16(*stream_, sprite.palette_index) ||
            !readU16(*stream_, flags))
            return fail("Incomplete sprite node");
        if (!validFormat(format)) return fail("Unknown sprite encoding");
        sprite.format = static_cast<SpriteFormat>(format);
        sprite.axis.x = static_cast<int16_t>(x);
        sprite.axis.y = static_cast<int16_t>(y);
        sprite.uses_tdata = (flags & 1u) != 0;
        sprite.linked = length == 0;
        if (sprite.linked) {
            if (sprite.link_index >= item)
                return fail("Sprite link does not refer to an earlier node");
        } else {
            const uint32_t package_length = sprite.uses_tdata
                ? info_.tdata_length : info_.ldata_length;
            const uint32_t package_offset = sprite.uses_tdata
                ? info_.tdata_offset : info_.ldata_offset;
            if (!addWithin(offset, length, package_length))
                return fail("Invalid sprite data range");
            if (!stream_->seek(package_offset + offset))
                return fail("Could not seek to sprite data");
            sprite.encoded_data.resize(length);
            if (!stream_->read(sprite.encoded_data.data(), length))
                return fail("Incomplete sprite data");
        }
        sprites_.push_back(std::move(sprite));
    }
    return true;
}

bool SFFFile::open(const std::string& path) {
    close();
    last_error_.clear();
    stream_ = createStdFileStream();
    if (!stream_->open(path, false)) {
        stream_.reset();
        return fail("Could not open input file: " + path);
    }
    if (!readHeader() || !readPalettes() || !readSprites()) {
        const std::string error = last_error_;
        close();
        last_error_ = error;
        return false;
    }
    loaded_ = true;
    return true;
}

bool SFFFile::open(std::unique_ptr<FileStream> stream) {
    close();
    last_error_.clear();
    stream_ = std::move(stream);
    if (!stream_ || !stream_->isOpen()) {
        stream_.reset();
        return fail("Supplied input stream is not open");
    }
    if (!readHeader() || !readPalettes() || !readSprites()) {
        const std::string error = last_error_;
        close();
        last_error_ = error;
        return false;
    }
    loaded_ = true;
    return true;
}

bool SFFFile::decodeSprite(Sprite& sprite) {
    last_error_.clear();
    if (sprite.decoded) return true;
    if (sprite.linked) {
        if (sprite.link_index >= sprites_.size())
            return fail("Sprite link index is out of range");
        Sprite& target = sprites_[sprite.link_index];
        if (&target == &sprite || !decodeSprite(target))
            return fail("Could not decode linked sprite");
        sprite.pixels = target.pixels;
        sprite.decoded = true;
        return true;
    }

    const size_t expected = expectedPixelBytes(sprite);
    if (expected == 0 && sprite.width != 0 && sprite.height != 0)
        return fail("Sprite dimensions overflow addressable memory");
    if (sprite.format == SpriteFormat::Raw) {
        if (sprite.encoded_data.size() != expected)
            return fail("Raw sprite payload size does not match its dimensions");
        sprite.pixels = sprite.encoded_data;
    } else if (isCompressed(sprite.format)) {
        if (sprite.encoded_data.size() < 4)
            return fail("Compressed sprite payload lacks its size prefix");
        const uint32_t declared = static_cast<uint32_t>(sprite.encoded_data[0]) |
            (static_cast<uint32_t>(sprite.encoded_data[1]) << 8) |
            (static_cast<uint32_t>(sprite.encoded_data[2]) << 16) |
            (static_cast<uint32_t>(sprite.encoded_data[3]) << 24);
        if (declared != expected)
            return fail("Compressed sprite size prefix does not match dimensions");
        const std::vector<uint8_t> data(sprite.encoded_data.begin() + 4,
                                        sprite.encoded_data.end());
        bool okay = false;
        if (sprite.format == SpriteFormat::Rle8)
            okay = decodeRle8(data, expected, sprite.pixels);
        else if (sprite.format == SpriteFormat::Rle5)
            okay = decodeRle5(data, expected, sprite.pixels);
        else if (sprite.format == SpriteFormat::Lz5)
            okay = decodeLz5(data, expected, sprite.pixels);
        if (!okay) return fail("Malformed or incomplete compressed sprite data");
    } else {
        return fail("PNG sprite decoding is not built in; encoded_data is available");
    }
    sprite.decoded = true;
    return true;
}

bool SFFFile::decodeAll() {
    for (size_t item = 0; item < sprites_.size(); ++item)
        if (!decodeSprite(sprites_[item])) return false;
    return true;
}

bool SFFFile::addPalette(const Palette& palette) {
    last_error_.clear();
    Palette value = palette;
    if (!value.linked) {
        if (value.entries.empty())
            return fail("Palette needs at least one entry");
        if (value.color_count == 0)
            value.color_count = static_cast<uint16_t>(value.entries.size());
        if (value.color_count != value.entries.size())
            return fail("Palette color_count does not match entries");
    } else if (value.link_index >= palettes_.size()) {
        return fail("Palette link index is out of range");
    }
    Palette* existing = findPalette(value.group, value.index);
    if (existing) *existing = value;
    else palettes_.push_back(value);
    info_.palette_count = palettes_.size();
    return true;
}

bool SFFFile::addSprite(const Sprite& sprite) {
    last_error_.clear();
    Sprite value = sprite;
    if (value.linked) {
        if (value.link_index >= sprites_.size())
            return fail("Sprite link index is out of range");
    } else if (value.decoded) {
        if (value.pixels.empty())
            return fail("Sprite needs decoded pixels or an encoded payload");
        const size_t expected = expectedPixelBytes(value);
        if (value.pixels.size() != expected)
            return fail("Sprite pixel size does not match dimensions and color depth");
    } else if (value.encoded_data.empty()) {
        return fail("Sprite needs decoded pixels or an encoded payload");
    }
    Sprite* existing = findSprite(value.group, value.index);
    if (existing) *existing = value;
    else sprites_.push_back(value);
    info_.sprite_count = sprites_.size();
    return true;
}

Sprite* SFFFile::findSprite(uint16_t group, uint16_t index) {
    for (size_t item = 0; item < sprites_.size(); ++item)
        if (sprites_[item].group == group && sprites_[item].index == index)
            return &sprites_[item];
    return NULL;
}

const Sprite* SFFFile::findSprite(uint16_t group, uint16_t index) const {
    for (size_t item = 0; item < sprites_.size(); ++item)
        if (sprites_[item].group == group && sprites_[item].index == index)
            return &sprites_[item];
    return NULL;
}

Palette* SFFFile::findPalette(uint16_t group, uint16_t index) {
    for (size_t item = 0; item < palettes_.size(); ++item)
        if (palettes_[item].group == group && palettes_[item].index == index)
            return &palettes_[item];
    return NULL;
}

const Palette* SFFFile::findPalette(uint16_t group, uint16_t index) const {
    for (size_t item = 0; item < palettes_.size(); ++item)
        if (palettes_[item].group == group && palettes_[item].index == index)
            return &palettes_[item];
    return NULL;
}

bool SFFFile::save(const std::string& path) {
    std::unique_ptr<FileStream> output = createStdFileStream();
    if (!output->open(path, true)) return fail("Could not create output file: " + path);
    return save(std::move(output));
}

bool SFFFile::save(std::unique_ptr<FileStream> output) {
    last_error_.clear();
    if (!output || !output->isOpen()) return fail("Supplied output stream is not open");
    if (sprites_.size() > std::numeric_limits<uint32_t>::max() ||
        palettes_.size() > std::numeric_limits<uint32_t>::max())
        return fail("Too many table entries to save");

    struct Payload { uint32_t offset; std::vector<uint8_t> bytes; };
    std::vector<Payload> sprite_payloads(sprites_.size());
    std::vector<Payload> palette_payloads(palettes_.size());
    uint64_t data_length = 0;

    for (size_t item = 0; item < sprites_.size(); ++item) {
        Sprite& sprite = sprites_[item];
        if (sprite.linked) {
            if (sprite.link_index >= item) return fail("Sprite links must point backward");
            continue;
        }
        std::vector<uint8_t> payload;
        if (sprite.decoded) {
            if (sprite.pixels.empty())
                return fail("Sprite has no payload to save");
            if (sprite.pixels.size() != expectedPixelBytes(sprite))
                return fail("Sprite pixel size does not match dimensions");
            SpriteFormat write_format = sprite.format;
            if (write_format == SpriteFormat::Rle5 || write_format == SpriteFormat::Lz5)
                write_format = SpriteFormat::Rle8;
            if (write_format == SpriteFormat::Raw) {
                payload = sprite.pixels;
            } else if (write_format == SpriteFormat::Rle8) {
                payload.resize(4);
                const uint32_t size = static_cast<uint32_t>(sprite.pixels.size());
                payload[0] = static_cast<uint8_t>(size);
                payload[1] = static_cast<uint8_t>(size >> 8);
                payload[2] = static_cast<uint8_t>(size >> 16);
                payload[3] = static_cast<uint8_t>(size >> 24);
                const std::vector<uint8_t> encoded = encodeRle8(sprite.pixels);
                payload.insert(payload.end(), encoded.begin(), encoded.end());
                sprite.format = SpriteFormat::Rle8;
            } else if (!sprite.encoded_data.empty()) {
                payload = sprite.encoded_data;
            } else {
                return fail("PNG sprites require an original encoded payload");
            }
        } else {
            if (sprite.encoded_data.empty())
                return fail("Sprite has no payload to save");
            payload = sprite.encoded_data;
        }
        if (payload.empty())
            return fail("Sprite has no payload to save");
        data_length = align4(static_cast<uint32_t>(data_length));
        if (data_length + payload.size() > std::numeric_limits<uint32_t>::max())
            return fail("Sprite and palette data exceed the SFFv2 size limit");
        sprite_payloads[item].offset = static_cast<uint32_t>(data_length);
        sprite_payloads[item].bytes.swap(payload);
        data_length += sprite_payloads[item].bytes.size();
    }

    for (size_t item = 0; item < palettes_.size(); ++item) {
        Palette& palette = palettes_[item];
        if (palette.linked) {
            if (palette.link_index >= item) return fail("Palette links must point backward");
            continue;
        }
        if (palette.color_count == 0)
            palette.color_count = static_cast<uint16_t>(palette.entries.size());
        if (palette.entries.empty())
            return fail("Palette has no payload to save");
        if (palette.entries.size() != palette.color_count)
            return fail("Palette color_count does not match entries");
        data_length = align4(static_cast<uint32_t>(data_length));
        const uint64_t bytes = static_cast<uint64_t>(palette.entries.size()) * 4u;
        if (data_length + bytes > std::numeric_limits<uint32_t>::max())
            return fail("Sprite and palette data exceed the SFFv2 size limit");
        palette_payloads[item].offset = static_cast<uint32_t>(data_length);
        for (size_t color = 0; color < palette.entries.size(); ++color) {
            const PaletteEntry& entry = palette.entries[color];
            palette_payloads[item].bytes.push_back(entry.r);
            palette_payloads[item].bytes.push_back(entry.g);
            palette_payloads[item].bytes.push_back(entry.b);
            palette_payloads[item].bytes.push_back(entry.a);
        }
        data_length += bytes;
    }

    const uint64_t sprite_table = kHeaderSize;
    const uint64_t palette_table = sprite_table + sprites_.size() * kSpriteNodeSize;
    const uint64_t ldata = align4(static_cast<uint32_t>(
        palette_table + palettes_.size() * kPaletteNodeSize));
    if (ldata + data_length > std::numeric_limits<uint32_t>::max())
        return fail("Output file exceeds the SFFv2 32-bit offset limit");

    info_.signature = "ElecbyteSpr";
    info_.version = 0x02000100u;
    info_.compatible_version = 0x02000000u;
    info_.sprite_table_offset = static_cast<uint32_t>(sprite_table);
    info_.palette_table_offset = static_cast<uint32_t>(palette_table);
    info_.ldata_offset = static_cast<uint32_t>(ldata);
    info_.ldata_length = static_cast<uint32_t>(data_length);
    info_.tdata_offset = info_.ldata_offset + info_.ldata_length;
    info_.tdata_length = 0;
    info_.sprite_count = sprites_.size();
    info_.palette_count = palettes_.size();

    std::vector<uint8_t> zeros(kHeaderSize, 0);
    if (!output->seek(0) || !output->write(zeros.data(), zeros.size()))
        return fail("Could not reserve the SFF header");

    if (!output->seek(info_.sprite_table_offset))
        return fail("Could not seek to output sprite table");
    for (size_t item = 0; item < sprites_.size(); ++item) {
        const Sprite& sprite = sprites_[item];
        const uint32_t length = sprite.linked ? 0 :
            static_cast<uint32_t>(sprite_payloads[item].bytes.size());
        if (!sprite.linked && length == 0)
            return fail("Sprite has no payload to save");
        if (!writeU16(*output, sprite.group) || !writeU16(*output, sprite.index) ||
            !writeU16(*output, sprite.width) || !writeU16(*output, sprite.height) ||
            !writeU16(*output, static_cast<uint16_t>(sprite.axis.x)) ||
            !writeU16(*output, static_cast<uint16_t>(sprite.axis.y)) ||
            !writeU16(*output, sprite.link_index) ||
            !writeU8(*output, static_cast<uint8_t>(sprite.format)) ||
            !writeU8(*output, sprite.color_depth) ||
            !writeU32(*output, sprite_payloads[item].offset) ||
            !writeU32(*output, length) ||
            !writeU16(*output, sprite.palette_index) ||
            !writeU16(*output, 0))
            return fail("Could not write sprite table");
    }

    if (!output->seek(info_.palette_table_offset))
        return fail("Could not seek to output palette table");
    for (size_t item = 0; item < palettes_.size(); ++item) {
        const Palette& palette = palettes_[item];
        const uint32_t length = palette.linked ? 0 :
            static_cast<uint32_t>(palette_payloads[item].bytes.size());
        if (!palette.linked && length == 0)
            return fail("Palette has no payload to save");
        if (!writeU16(*output, palette.group) ||
            !writeU16(*output, palette.index) ||
            !writeU16(*output, palette.color_count) ||
            !writeU16(*output, palette.link_index) ||
            !writeU32(*output, palette_payloads[item].offset) ||
            !writeU32(*output, length))
            return fail("Could not write palette table");
    }

    for (size_t item = 0; item < sprite_payloads.size(); ++item) {
        if (sprite_payloads[item].bytes.empty()) continue;
        if (!output->seek(info_.ldata_offset + sprite_payloads[item].offset) ||
            !output->write(sprite_payloads[item].bytes.data(),
                           sprite_payloads[item].bytes.size()))
            return fail("Could not write sprite payload");
    }
    for (size_t item = 0; item < palette_payloads.size(); ++item) {
        if (palette_payloads[item].bytes.empty()) continue;
        if (!output->seek(info_.ldata_offset + palette_payloads[item].offset) ||
            !output->write(palette_payloads[item].bytes.data(),
                           palette_payloads[item].bytes.size()))
            return fail("Could not write palette payload");
    }

    if (!output->seek(0) || !output->write(kSignature, sizeof(kSignature)))
        return fail("Could not write SFF header");
    uint8_t version[4], compatible[4];
    packVersion(info_.version, version);
    packVersion(info_.compatible_version, compatible);
    if (!output->write(version, 4))
        return fail("Could not write SFF version");
    for (int item = 0; item < 2; ++item)
        if (!writeU32(*output, 0)) return fail("Could not write SFF header");
    if (!output->write(compatible, 4))
        return fail("Could not write compatible SFF version");
    for (int item = 0; item < 2; ++item)
        if (!writeU32(*output, 0)) return fail("Could not write SFF header");
    if (!writeU32(*output, info_.sprite_table_offset) ||
        !writeU32(*output, static_cast<uint32_t>(sprites_.size())) ||
        !writeU32(*output, info_.palette_table_offset) ||
        !writeU32(*output, static_cast<uint32_t>(palettes_.size())) ||
        !writeU32(*output, info_.ldata_offset) ||
        !writeU32(*output, info_.ldata_length) ||
        !writeU32(*output, info_.tdata_offset) ||
        !writeU32(*output, info_.tdata_length))
        return fail("Could not finish SFF header");
    loaded_ = true;
    return output->close() || fail("Could not close output stream");
}

} // namespace sff2
