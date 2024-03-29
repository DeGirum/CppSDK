if exist inc\asio.hpp goto httplib
git clone -b asio-1-18-2 --single-branch https://github.com/chriskohlhoff/asio.git
move asio\asio\include\*.* inc\
move asio\asio\include\asio inc\
move asio\asio\COPYING inc\asio
move asio\asio\LICENSE_1_0.txt inc\asio
rmdir /s /q asio
del /Q /F inc\.gitignore
del /Q /F inc\Makefile.am
:httplib
if exist inc\httplib.h exit
git clone -b v0.13.3 --single-branch https://github.com/yhirose/cpp-httplib.git
move cpp-httplib\httplib.h inc\
rmdir /s /q cpp-httplib
