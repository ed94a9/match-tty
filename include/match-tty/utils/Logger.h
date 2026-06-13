#pragma once

#include <quill/Logger.h>
#include <quill/LogMacros.h>
#include <quill/SimpleSetup.h>
#include <string_view>

namespace mtty {

void initLogger(std::string_view log_file = "match-tty.log");
quill::Logger* logger();

} // namespace mtty

#define QLOG_TRACE(...)  do { if (auto* l = ::mtty::logger()) QUILL_LOG_TRACE_L1(l, ##__VA_ARGS__); } while(0)
#define QLOG_DEBUG(...)  do { if (auto* l = ::mtty::logger()) QUILL_LOG_DEBUG(l, ##__VA_ARGS__); } while(0)
#define QLOG_INFO(...)   do { if (auto* l = ::mtty::logger()) QUILL_LOG_INFO(l, ##__VA_ARGS__); } while(0)
#define QLOG_WARN(...)   do { if (auto* l = ::mtty::logger()) QUILL_LOG_WARNING(l, ##__VA_ARGS__); } while(0)
#define QLOG_ERROR(...)  do { if (auto* l = ::mtty::logger()) QUILL_LOG_ERROR(l, ##__VA_ARGS__); } while(0)
