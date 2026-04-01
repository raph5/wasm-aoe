#!/usr/bin/env bash

function check_copyright {
  while read -r l; do
    copyright=$(rg Copyright < $l)
    if [[ -z "$copyright" ]]; then
      echo "$l is not copyrighted"
      some_file_not_copyrighted=1
    fi
  done
}

out=$(fd -e c -e h | check_copyright)
if [[ -z $out ]]; then
  echo "every c file is copyrighted"
  echo "every header file is copyrighted"
else
  echo "$out"
fi
