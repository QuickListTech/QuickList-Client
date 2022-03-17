// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <fstream>
#include <iostream>

/**
 * @todo write docs
 */
class Log
{
public:
    Log();
    ~Log();
    void open();

    void file(std::string const& file) {
        file_ = file;
    }

    std::string const &file() const {
        return file_;
    }
    void close();

    template<typename T>
    Log& operator<<(T const &t)
    {
        std::lock_guard<std::mutex> lock(mtx_);

        if (out_.is_open()) {
            out_ << t;
            out_.flush();
        }

        std::cout << t;

        return *this;
    }

    Log& info();
    Log& warn();
    Log& err();
    Log& debug();

    typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
    typedef CoutType& (*StandardEndLine)(CoutType&);

    Log& operator<<(StandardEndLine manip);
private:
    std::ofstream out_;
    std::mutex mtx_;
    std::string file_;
};

extern Log logger;

#endif // LOG_H
