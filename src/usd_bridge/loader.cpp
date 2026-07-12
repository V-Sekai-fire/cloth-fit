#include "loader.hpp"

#include <cstdlib>

#include "cloth_fit_usd.h" // C ABI (cfusd_init) — resolved via the stub after load

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include "cfusd/cloth_fit_usd_stubs.h" // generated POSIX dlsym table (cfusd::)
#endif

namespace cfusd_loader
{
    namespace
    {
        bool g_loaded = false;
    } // namespace

    bool loaded()
    {
        return g_loaded;
    }

    bool load(const std::string &bridge_path)
    {
        if (g_loaded)
        {
            return true;
        }
#ifdef _WIN32
        // LOAD_WITH_ALTERED_SEARCH_PATH so the bridge's own deps (usd_ms.dll,
        // tbb12.dll) resolve from the same dir; the delay-load thunks then bind
        // to this already-loaded module.
        if (!bridge_path.empty())
        {
            HMODULE h = LoadLibraryExA(bridge_path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
            g_loaded = (h != nullptr);
        }
        else
        {
            g_loaded = true; // rely on the OS loader / delay-load search
        }
#else
        cfusd::StubPathMap paths;
        paths[cfusd::kModuleCloth_fit_usd] = {bridge_path};
        g_loaded = cfusd::InitializeStubs(paths);
#endif
        return g_loaded;
    }

    bool load_from_env()
    {
        const char *bridge = std::getenv("CFUSD_BRIDGE");
        if (!load(bridge != nullptr ? bridge : ""))
        {
            return false;
        }
        const char *plugin_dir = std::getenv("CFUSD_PLUGIN_DIR");
        cfusd_init(plugin_dir); // NULL -> the bridge's compiled-in default
        return true;
    }
} // namespace cfusd_loader
