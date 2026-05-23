#include <match-tty/utils/Logger.h>
#include <quill/Quill.h>
#include <vector>

namespace mtty {

namespace {
quill::Logger* g_logger = nullptr;
}

void initLogger(std::string_view log_file) {
    quill::start();

    auto file_h = quill::file_handler(std::string(log_file));
    auto stdout_h = quill::stdout_handler();

    std::vector<std::shared_ptr<quill::Handler>> handlers;
    handlers.push_back(std::move(file_h));
    handlers.push_back(std::move(stdout_h));

    g_logger = quill::create_logger("match-tty", std::move(handlers));
}

quill::Logger* logger() {
    return g_logger;
}

} // namespace mtty
