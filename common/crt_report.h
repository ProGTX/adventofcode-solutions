#ifndef AOC_CRT_REPORT_H
#define AOC_CRT_REPORT_H

#include "compiler.h"

// On MSVC a failing assert() - or one of the STL's own _STL_VERIFY checks in a
// Debug build - reports through _CrtDbgReport, which defaults to a modal
// message box. Under ctest that blocks the whole run until the test times out,
// and the message never makes it into the test log.
//
// Route those reports to stderr instead, and stop abort() from handing the
// process off to Windows Error Reporting, so a failing test dies immediately
// with its diagnostic on stderr where ctest can capture it.

#if defined(AOC_COMPILER_MSVC)

#ifdef AOC_INCLUDE_FROM_COMMON
#include <cstdlib>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#endif

namespace aoc::detail {

struct crt_report_to_stderr_t {
  crt_report_to_stderr_t() {
    // Keep the "abort() has been called" text, drop the WER / JIT-debugger
    // dialog that would otherwise wait for someone to click it
    _set_abort_behavior(_WRITE_ABORT_MSG, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#ifdef _DEBUG
    // _CrtSetReportMode is only meaningful against the debug CRT
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
  }
};

// Runs before main, so it covers asserts from anywhere in the test
inline const crt_report_to_stderr_t crt_report_to_stderr{};

} // namespace aoc::detail

#endif // AOC_COMPILER_MSVC

#endif // AOC_CRT_REPORT_H
