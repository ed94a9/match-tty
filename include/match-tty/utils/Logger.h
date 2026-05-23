#pragma once

#include <quill/Quill.h>
#include <string_view>

namespace mtty {

void initLogger(std::string_view log_file = "match-tty.log");
quill::Logger* logger();

} // namespace mtty

#define QLOG_TRACE(...)  QUILL_LOG_TRACE_L1(::mtty::logger(), ##__VA_ARGS__)
#define QLOG_DEBUG(...)  QUILL_LOG_DEBUG(::mtty::logger(), ##__VA_ARGS__)
#define QLOG_INFO(...)   QUILL_LOG_INFO(::mtty::logger(), ##__VA_ARGS__)
#define QLOG_WARN(...)   QUILL_LOG_WARNING(::mtty::logger(), ##__VA_ARGS__)
#define QLOG_ERROR(...)  QUILL_LOG_ERROR(::mtty::logger(), ##__VA_ARGS__)
