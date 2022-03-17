// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "log.h"

Log::Log()
{
}

Log::~Log()
{
     close();
}

void Log::open()
{
    if (file_ != "") {
        out_.open ( file_, std::ios::out|std::ios::app );

        if (!out_) {
            std::cerr << "Unable to open file " << file_ << std::endl;
        }
    }
}

void Log::close()
{
    out_.close();
}

Log& Log::operator<< ( StandardEndLine manip )
{
     std::lock_guard<std::mutex> lock(mtx_);

     manip ( out_ );
     manip ( std::cout );

     return *this;
}

Log& Log::warn()
{
    *this << "WARN: ";
    return *this;
}

Log& Log::info()
{
    *this << "INFO: ";
    return *this;
}

Log& Log::debug()
{
    *this << "DEBUG: ";
    return *this;
}

Log& Log::err()
{
    *this << "ERR: ";
    return *this;
}
