# this file works only with LF line ending
find . -not -path "./[build|\.git]*" | grep '^.*\.[ch]$' | xargs clang-format -style=file --verbose -i
