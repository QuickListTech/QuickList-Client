// SPDX-FileCopyrightText: 2022 Petr Janda admin@quicklist.tech
// SPDX-License-Identifier: Apache-2.0

#ifndef LOG_H
#define LOG_H

#include <boost/log/trivial.hpp>

void logInit(std::string const &logFilePath, std::string const &logLevel);

#endif

