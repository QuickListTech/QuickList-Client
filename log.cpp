// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "log.h"

using std::string;

Log::Log() : line_(), sline_(line_), scout_(std::cout)
{

}

Log::~Log()
{
     close();
}

void Log::open()
{
     if ( file_ != "" ) {
          fout_.open ( file_, std::ios::out|std::ios::app );

          if ( !fout_ ) {
               std::cerr << "Unable to open file " << file_ << std::endl;
          }
     }
}

void Log::close()
{
     fout_.close();
}

Log& Log::operator<< ( StandardEndLine manip )
{
     manip ( sline_ );
     manip ( scout_ );

     sline_.emit();
     scout_.emit();

     {
          std::lock_guard<std::mutex> lock ( mtx_ );

          if (fout_.is_open()) {
               fout_ << line_.str();
          }

          line_.str("");
     }

     return *this;
}

Log& Log::warn()
{
    *this << timeStr() << " WARN: ";
    return *this;
}

Log& Log::info()
{
     *this << timeStr() << " INFO: ";
     return *this;
}

Log& Log::debug()
{
     *this << timeStr() << " DEBUG: ";
     return *this;
}

Log& Log::err()
{
     *this << timeStr() << " ERR: ";
     return *this;
}

string Log::timeStr() const
{
    char tt[100];
    time_t now = time(nullptr);
    auto tm_info = localtime(&now);

    int sz = strftime(tt, 100, "%Y-%m-%d %H:%M:%S", tm_info);
    return string(tt, sz);
}

