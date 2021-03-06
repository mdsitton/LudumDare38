#include "config.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <iterator>
#include <SDL.h>
#include <spdlog/spdlog.h>

#include "game.hpp"

// Eventually we will want to load configuration files somewhere in here.
// This also means the VFS needs to be setup here as well
int main(int argc, char** argv)
{

    std::shared_ptr<spdlog::logger> logger;

    try {
        std::vector<spdlog::sink_ptr> sinks;

        // I would like to use the sink that does color output to the console
        // but it doesnt yet work for windows.
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());

        // Probably will want to switch this to ostream_sink_mt once
        // the vfs is setup fully.
        sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_mt>("planetgame.log"));

        logger = std::make_shared<spdlog::logger>("default", std::begin(sinks), std::end(sinks));
        spdlog::register_logger(logger);

        // Should be set by the configuration eventually.
        logger->set_level(spdlog::level::info);

    } catch (const spdlog::spdlog_ex& err) {
        std::cout << "Logging Failed: " << err.what() << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", err.what(), nullptr);
        return 1;
    }

    try {
        PlanetGame::GameManager game;
        game.start();
    } catch (std::runtime_error &err) {
        logger->critical("Runtime Error:\n{}", err.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", err.what(), nullptr);
        return 1;
    }
    return 0;
}
