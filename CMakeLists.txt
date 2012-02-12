# Copyright 2012 Dean Michael Berris <dberris@google.com>
# Copyright 2012 Google, Inc.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
# CMakeLists.txt

cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "-std=c++0x")
set(CMAKE_VERBOSE_MAKEFILE true)
project(CHAIN)

enable_testing()
add_subdirectory(test)
