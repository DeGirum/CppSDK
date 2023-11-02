#!/bin/bash
if [ ! -f "inc/asio.hpp" ]; then
  git clone -b asio-1-18-2 --single-branch https://github.com/chriskohlhoff/asio.git
  mv asio/asio/include/* inc/
  mv asio/asio/COPYING inc/asio
  mv asio/asio/LICENSE_1_0.txt inc/asio
  rm -rf asio
  rm -rf inc/.gitignore
  rm -rf inc/Makefile.am
fi
if [ ! -f "inc/httplib.hpp" ]; then
  git clone -b v0.13.3 --single-branch https://github.com/yhirose/cpp-httplib.git
  mv cpp-httplib/httplib.h inc/
  rm -rf cpp-httplib
fi
