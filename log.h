// SPDX-FileCopyrightText: 2022 Petr Janda admin@quicklist.tech
// SPDX-License-Identifier: Apache-2.0

#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <fstream>
#include <iostream>
#include <syncstream>

/**
 * Simple logging facility outputs to both file and cout.
 * It implements basic stream operator<< and std::endl
 * Supports a number of logging levels and is thread safe
 * Exposes the "logger" variable globally via an extern
 */
class Log
{
public:
    Log();
    ~Log();
    void open();

    void file ( std::string const& file )
    {
        file_ = file;
    }

    std::string const &file() const
    {
        return file_;
    }
    void close();

    template<typename T>
    Log& operator<< ( T const &t )
    {
        sline_ << t;
        scout_ << t;

        return *this;
    }

    Log& info();
    Log& warn();
    Log& err();
    Log& debug();

    typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
    typedef CoutType& ( *StandardEndLine ) ( CoutType& );

    Log& operator<< ( StandardEndLine manip );
private:
    std::ofstream fout_;
    std::ostringstream line_;
    std::osyncstream sline_;
    std::osyncstream scout_;
    std::mutex mtx_;
    std::string file_;

    std::string timeStr() const;
};

extern Log logger;

#endif // LOG_H
