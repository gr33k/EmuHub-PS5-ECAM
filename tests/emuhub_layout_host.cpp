#include "../port/app/paths.h"
#include "../port/app/reorganize.h"
#include <cassert>
#include <filesystem>
#include <string>
int main(int argc, char** argv) {
    assert(argc == 2);
    assert(ps5paths::Root() == "/data/ps5cemu");
    assert(ps5paths::CemuRoot() == "/data/ps5cemu");
    assert(ps5paths::Games() == "/data/ps5cemu/games");
    assert(ps5paths::Mlc() == "/data/ps5cemu/mlc01");
    assert(ps5paths::Moved("/data/ps5cemu/games/140 (USA).wua") == "/data/ps5cemu/games/140 (USA).wua");
    assert(ps5reorganize::Look(ps5paths::Root(), ps5paths::kDefaultRoot).separate.empty());
    std::filesystem::path stage = argv[1];
    std::filesystem::create_directories(stage);
    assert(!ps5reorganize::Ask(stage.string(), ps5reorganize::Request::Separate));
    assert(!std::filesystem::exists(stage / "reorganize.txt"));
}
