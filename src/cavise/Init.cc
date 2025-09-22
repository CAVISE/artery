// STD
#include <mutex>

// plog
// #include <plog/Initializers/RollingFileInitializer.h>
// #include <plog/Log.h>
// #include <plog/Severity.h>

// local
#include <cavise/Init.h>

namespace
{

static struct State {
    std::once_flag init;
} state;

}  // namespace

void cavise::init()
{
    std::call_once(state.init, []() {
        // plog::init(plog::Severity::debug, "communication.log", 1024 * 1024, 2);
        // PLOG(plog::info) << "initalized logging, recording API calls";
    });
}