# Copyright 2012 Dean Michael Berris <dberris@google.com>
# Copyright 2012 Google, Inc.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
# CMakeLists.txt

include_directories(${CHAIN_SOURCE_DIR})
add_executable(usage usage.cpp)
add_test(usage usage)
