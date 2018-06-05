#!/bin/sh
#
# Syntax: cpp-style-check.sh [-r] <file/directory>
#
# Available options:
#   -r, enable recursively check and format, only available when input is a directory.
#

# Is recursion for dispose enable?
recursion=false

# The path of specified directory in which the files to be checked and lint
rootPath=""

acceptFileType=( "h" "cpp" "cc" "hpp" "c" )

function format() {
  local file=$1
  # Format code with google style
  clang-format -i -style=Google ${file}
}

function checkStyle() {
  local file=$1
  ${dirOfThisFile}/cpplint.py --root=${dirOfThisFile}/../Source ${file}
  echo
}

function dispose() {
  local curPath=$1

  local suffix="${curPath##*.}"
  if test -f ${curPath} && [[ " ${acceptFileType[@]} " =~ " ${suffix} " ]]
  then
    format ${curPath}
    checkStyle ${curPath}
  else
    local files=$(ls ${curPath})

    for file in ${files}
    do

    if test -f ${curPath}/${file}
    then
      dispose ${curPath}/${file}
    fi

    if test -d ${curPath}/${file} && $recursion
    then
      dispose ${curPath}/${file}
    fi

    done
  fi
}

# The path of directory which this shell script belongs to
dirOfThisFile="${0%/*}"

if ! test -d ${dirOfThisFile}
then
  dirOfThisFile=.
fi

for i in $@; do
  if [ "$i" = "-r" ]; then
    recursion=true
  else
    rootPath=$i
  fi
done

if ! test -e ${rootPath}
then
  echo ${rootPath} ' No such file or directory.'
  exit
fi

if test -d ${rootPath} && [ "${rootPath##*/}" = "" ]; then
  rootPath=${rootPath%/*}
fi

echo '### Start google-style lint and format \n'

dispose ${rootPath}

echo '### End google-style lint and format'
