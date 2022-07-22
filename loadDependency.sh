#!/bin/bash
git clone -b asio-1-18-2 --single-branch https://github.com/chriskohlhoff/asio.git
mv asio/asio/include/* inc/
mv asio/asio/COPYING inc/asio
mv asio/asio/LICENSE_1_0.txt inc/asio
rm -rf asio
rm -rf inc/.gitignore
rm -rf inc/Makefile.am
