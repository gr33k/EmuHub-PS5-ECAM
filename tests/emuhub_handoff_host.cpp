#include "../port/app/emuhub_handoff.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

int main() {
    using emuhub_handoff::parse;
    constexpr auto cached = "EMUHUB-WIIU-1\n0123456789abcdef\n/app0/content/eh-0123456789abcdef.wua\n";
    constexpr auto legacy = "EMUHUB-WIIU-1\n29194233d27c6b72\n/data/ps5cemu/games/140 (USA).wua\n";
    assert(parse(cached) && parse(cached)->rom == "/app0/content/eh-0123456789abcdef.wua");
    assert(parse(legacy) && parse(legacy)->key_hash == 0x29194233d27c6b72ULL);
    assert(!parse("EMUHUB-WIIU-1\n0123456789abcdeg\n/app0/content/eh-0123456789abcdeg.wua\n"));
    assert(!parse("EMUHUB-WIIU-1\n0123456789abcdef\n/app0/content/eh-1123456789abcdef.wua\n"));
    assert(!parse("EMUHUB-WIIU-1\n0123456789abcdef\n/data/homebrew/PPSA91337/content/../save.wua\n"));
    assert(!parse("EMUHUB-WIIU-1\n0123456789abcdef\n/data/ps5cemu/games/140 (USA).wua\n"));
    assert(!parse("EMUHUB-WIIU-1\n0123456789abcdef\n/app0/content/eh-0123456789abcdef.wua"));
    char path[] = "/tmp/emuhub-wiiu-request-XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    assert(write(fd, cached, std::strlen(cached)) == static_cast<ssize_t>(std::strlen(cached)));
    close(fd);
    const auto request = emuhub_handoff::consume(path);
    assert(request && request->key_hash == 0x0123456789abcdefULL);
    assert(access(path, F_OK) != 0);
    assert(!emuhub_handoff::consume(path));
    return 0;
}
