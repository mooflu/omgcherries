// Description:
//   main. Ready, set, go!
//
// Copyright (C) 2008 Frank Becker
//

#include "SDL.h"  //needed for SDL_main

#include "Trace.hpp"
#include "Constants.hpp"
#include "Config.hpp"
#include "Input.hpp"
#include "Game.hpp"
#include "GameState.hpp"
#include "Endian.hpp"
#include "ResourceManager.hpp"
#include "GetDataPath.hpp"
#ifndef IPHONE
//#include "PreLaunch.hpp"
//#include "PostLaunch.hpp"
#endif
using namespace std;

class NoopStreamBuf : public std::streambuf {
public:
    NoopStreamBuf() {}

protected:
    virtual int_type overflow(int_type c) { return c; }
};

void checkEndian(void) {
    if (::isLittleEndian()) {
        LOG_INFO << "Setting up for little endian." << endl;
    } else {
        LOG_INFO << "Setting up for big endian." << endl;
    }
}

#ifndef IPHONE
void showInfo() {
    LOG_INFO << "----------------------------------" << endl;
    LOG_INFO << GAMETITLE << " " << GAMEVERSION << " - " << __TIME__ << " " << __DATE__ << endl;
    LOG_INFO << "Copyright (C) 2004-2023 by Frank Becker" << endl;
    LOG_INFO << "Visit http://games.mooflu.com" << endl;
    LOG_INFO << "----------------------------------" << endl;
}

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <png.h>
#include <zlib.h>
#include <physfs.h>

void showVersions(void) {
    SDL_version sdlVer;
    SDL_GetVersion(&sdlVer);

    PHYSFS_Version physFSVer;
    PHYSFS_getLinkedVersion(&physFSVer);

    LOG_INFO << "SDL Version " << (int)sdlVer.major << "." << (int)sdlVer.minor << "." << (int)sdlVer.patch << endl;
    const SDL_version* isdl = IMG_Linked_Version();
    LOG_INFO << "SDL_image Version " << (int)isdl->major << "." << (int)isdl->minor << "." << (int)isdl->patch << endl;
    const SDL_version* msdl = Mix_Linked_Version();
    LOG_INFO << "SDL_mixer Version " << (int)msdl->major << "." << (int)msdl->minor << "." << (int)msdl->patch << endl;
    LOG_INFO << "PhysFS Version " << (int)physFSVer.major << "." << (int)physFSVer.minor << "." << (int)physFSVer.patch
             << endl;
    LOG_INFO << "zlib Version " << zlibVersion() << endl;
    LOG_INFO << "PNG Version " << png_get_header_version(NULL) << endl;
}
#endif

#ifdef IPHONE
bool startupGame(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef IPHONEDIST
#warning Logging off
    Trace::SetStreamBuffer(new NoopStreamBuf());
#endif

#ifndef IPHONE
    showInfo();
    showVersions();
#endif

    checkEndian();

    string resourceFilePath = "resource.dat";
#if defined(__APPLE__) || defined(WIN32) || defined(EMSCRIPTEN)
    string writeSubdir = "OMGCherries";
#else
    string writeSubdir = ".omgcherries";
#endif
    ResourceManagerS::instance()->setWriteDirectory(writeSubdir);  // TODO: handle windows

    if (!ResourceManagerS::instance()->addResourceBundle(getDataPath() + resourceFilePath, "/")) {
        LOG_WARNING << "resource.dat not found. Trying data directory." << endl;
#ifdef VCPP
        ResourceManagerS::instance()->addResourceDirectory("../../data");
#else
        ResourceManagerS::instance()->addResourceBundle("data", "/");
#endif
        if (ResourceManagerS::instance()->getResourceSize("system/config.txt") < 0) {
            LOG_ERROR << "Sorry, unable to find game data!" << endl;
            return 1;
        }
    }

    Config* cfg = ConfigS::instance();

    // register config handler(s)
    //cfg->registerConfigHandler( InputS::instance());

    // read config file...
    cfg->updateFromFile();

    // process command line arguments...
    cfg->updateFromCommandLine(argc, argv);

    // to dump or not to dump...
    cfg->getBoolean("developer", GameState::isDeveloper);
    if (GameState::isDeveloper) {
        cfg->dump();
    }

#ifndef IPHONE
    //preLaunch(); //show demo dialog or splash screen
    // get ready!
    if (GameS::instance()->init()) {
        // let's go!
        GameS::instance()->run();
    }
#else
    return GameS::instance()->init();
}

int shutdownGame(void) {
#endif

    // Fun is over. Cleanup time...
    GameS::cleanup();

#ifndef IPHONE
    //postLaunch(); //show demo "Buy Now" dialog
#endif
    ConfigS::cleanup();
    ResourceManagerS::cleanup();

    LOG_INFO << "Cleanup complete. Ready to Exit." << endl;

#ifndef IPHONE
    showInfo();
#endif

    // See ya!
    return 0;
}
