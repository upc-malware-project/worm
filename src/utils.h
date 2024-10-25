#pragma once
#include <libc/macros.h>
#include <libc/nt/runtime.h>
#include <libc/nt/process.h>
#include <libc/nt/enum/formatmessageflags.h>
#include <libc/nt/enum/lang.h>

static char16_t* getErrorString() {
    static char16_t msg[256];
    int err = GetLastError();

    if (err == 0) {
        return nullptr;
    }

    FormatMessage(kNtFormatMessageFromSystem | kNtFormatMessageIgnoreInserts,
                  nullptr,
                  err,
                  MAKELANGID(kNtLangNeutral, kNtSublangDefault),
                  msg,
                  ARRAYLEN(msg),
                  nullptr);
    return msg;
}
