#include <match-tty/utils/Logger.h>
#include <vector>
#include <quill/Frontend.h>
#include <quill/Backend.h>
#include <quill/sinks/FileSink.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/core/FrontendOptions.h>
#include <quill/backend/SignalHandler.h>

namespace mtty {

namespace {
quill::Logger* g_logger = nullptr;
}

void initLogger(std::string_view log_file) {
    quill::Backend::start<quill::FrontendOptions>(quill::BackendOptions{}, quill::SignalHandlerOptions{});

    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(std::string(log_file));
    auto stdout_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("stdout");

    g_logger = quill::Frontend::create_or_get_logger("match-tty", {file_sink, stdout_sink});
}

quill::Logger* logger() {
    return g_logger;
}

} // namespace mtty
