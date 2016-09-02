#!/bin/bash
clang-format -i $(find . -name '*.[ch]')

# Check we have no typos.
which misspell 2>/dev/null >/dev/null
if [[ $? -eq 0 ]]; then
    misspell -error `find src -name '*.c' -or -name '*.h'`
fi
