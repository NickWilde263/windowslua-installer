#!/bin/sh
if [ $# -eq 0 ]; then
  echo Expect filename!
  exit 1
fi

echo Archive size 0x$(stat -c%s "$1") bytes
echo "#include <stddef.h>" > "$1.c"
printf "const char $2[] = " >> "$1.c" 
(cat $1 | lua5.3 pack.lua) >> "$1.c"
printf ";\n" >> "$1.c"
printf "size_t $3 = sizeof($2);\n" >> "$1.c"
 