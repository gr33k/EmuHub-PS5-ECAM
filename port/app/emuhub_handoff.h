// SPDX-License-Identifier: GPL-3.0-or-later
// One-use EmuHub Wii U request. PS5 LoadExec may discard argv, so the selected
// game is passed in the title's config directory before the provider starts.
#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <sys/stat.h>

namespace emuhub_handoff {
struct Request {
    std::uint64_t key_hash = 0;
    std::string rom;
};

inline int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

inline std::optional<Request> parse(std::string_view text) {
    constexpr std::string_view magic = "EMUHUB-WIIU-1\n";
    if (!text.starts_with(magic)) return std::nullopt;
    text.remove_prefix(magic.size());
    if (text.size() < 18 || text[16] != '\n' || text.back() != '\n') return std::nullopt;
    Request result;
    for (int i = 0; i < 16; ++i) {
        const int digit = hex_digit(text[i]);
        if (digit < 0) return std::nullopt;
        result.key_hash = (result.key_hash << 4) | static_cast<unsigned>(digit);
    }
    text.remove_prefix(17);
    text.remove_suffix(1);
    if (text == "/data/ps5cemu/games/140 (USA).wua" && result.key_hash == 0x29194233d27c6b72ULL) {
        result.rom = text;
        return result;
    }
    constexpr std::string_view prefix = "/app0/content/eh-";
    constexpr std::string_view suffix = ".wua";
    if (text.size() != prefix.size() + 16 + suffix.size() ||
        !text.starts_with(prefix) || !text.ends_with(suffix)) return std::nullopt;
    for (int i = 0; i < 16; ++i) {
        const int digit = hex_digit(text[prefix.size() + i]);
        if (digit < 0 || static_cast<unsigned>(digit) != ((result.key_hash >> (60 - 4 * i)) & 15))
            return std::nullopt;
    }
    result.rom = text;
    return result;
}

inline std::optional<Request> consume(const char* path) {
    FILE* file = std::fopen(path, "rb");
    if (!file) return std::nullopt;
    struct stat info{};
    char bytes[256]{};
    const bool bounded = fstat(fileno(file), &info) == 0 && S_ISREG(info.st_mode) &&
        info.st_size > 0 && info.st_size <= static_cast<off_t>(sizeof bytes);
    const size_t count = bounded ? std::fread(bytes, 1, static_cast<size_t>(info.st_size), file) : 0;
    const bool complete = bounded && count == static_cast<size_t>(info.st_size) && std::ferror(file) == 0;
    std::fclose(file);
    // A failed remove leaves a replayable request. Do not launch in that case.
    if (std::remove(path) != 0 || !complete) return std::nullopt;
    return parse(std::string_view(bytes, count));
}
} // namespace emuhub_handoff
