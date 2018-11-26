#!/usr/bin/env bash

RESTORE=$(echo -en '\033[0m')
RED=$(echo -en '\033[00;31m')
GREEN=$(echo -en '\033[00;32m')
YELLOW=$(echo -en '\033[00;33m')
BLUE=$(echo -en '\033[00;34m')
MAGENTA=$(echo -en '\033[00;35m')
PURPLE=$(echo -en '\033[00;35m')
CYAN=$(echo -en '\033[00;36m')
LIGHTGRAY=$(echo -en '\033[00;37m')
LRED=$(echo -en '\033[01;31m')
LGREEN=$(echo -en '\033[01;32m')
LYELLOW=$(echo -en '\033[01;33m')
LBLUE=$(echo -en '\033[01;34m')
LMAGENTA=$(echo -en '\033[01;35m')
LPURPLE=$(echo -en '\033[01;35m')
LCYAN=$(echo -en '\033[01;36m')
WHITE=$(echo -en '\033[01;37m')

function info () {
  echo -e $*
}

function notice () {
  echo -e "${CYAN}$*${RESTORE}"
}

function warning () {
  echo -e "${YELLOW}$*${RESTORE}"
}

function error () {
  echo -e "${RED}$*${RESTORE}"
}

function prepareWorkingDir () {
  read -e -p "Your upstream git remote is? [origin]"
  export remote=${REPLY:="origin"}

  info "Fetching latest changes and prune ${LGREEN}$remote${RESTORE}..."
  git fetch -p -t "$remote" || { error "It is not able to fetch codes from your $remote, check if there is any issue."; exit 1; }

  git remote prune "$remote"
  info "${GREEN}Done.${RESTORE}"
}

function isValidVersion () {
  echo "$1" | awk '/^[0-9]+\.[0-9]+(\.[0-9]+){0,2}(-rc[0-9]+|-beta[0-9]+)?$/ {print $0}'
}

function isBetaVersion () {
  echo "$1" | awk '/^[0-9]+\.[0-9]+(\.[0-9]+){0,2}-beta[0-9]+$/ {print $0}'
}

function isRCVersion () {
  echo "$1" | awk '/^[0-9]+\.[0-9]+(\.[0-9]+){0,2}-rc[0-9]+$/ {print $0}'
}

function latestVersion () {
  git tag -l | awk '/^0\.[0-9]+(\.[0-9]+){0,2}(-rc[0-9]+|-beta[0-9]+)?$/ { print $0 }' | sort -rV | head -n 1
}

function latestNonRCVersion () {
  git tag -l | awk '/^0\.[0-9]+(\.[0-9]+){0,2}(-beta[0-9]+)?$/ { print $0 }' | sort -rV | head -n 1
}

function betaNumber () {
  echo "$1" | awk -F 'beta' '{print $2}'
}

function rcNumber () {
  echo "$1" | awk -F 'rc' '{print $2}'
}

function gitRepoModified () {
  git status --short | awk '/^[ M][ M] / { print $0 }'
}

function stashLocalChanges () {
  if [ "$(gitRepoModified)" ]; then
    read -e -p "Your working directory is modified, stash it? [Y/n]" -n 1
    shouldStash=${REPLY:=y}
    if [ ${shouldStash,,} = y ]; then
      git stash
    else
      info "Change ignored."
    fi
  fi
}

function increaseBetaVersion () {
  if [ $(isRCVersion "$1") ]; then
    # Unable to increase beta version number on a RC version, exit.
    return 1
  fi
  if [ $(isBetaVersion "$1") ]; then
    echo "$1" | awk -F 'beta' '{printf("%sbeta%d\n", $1, $2+1)}'
  else
    echo "$1" | awk -F '.' '/^([0-9]+\.){1,3}([0-9]+)$/ {
      for(i=1;i<=NF;i++) {
        if (i==NF) {
          printf("%d-beta1\n", $i+1)
        } else {
          printf("%d.", $i)
        }
      }
    }'
  fi
}

function increaseRCVersion () {
  if [ $(isBetaVersion "$1") ]; then
    # Unable to increase rc version number on a beta version, exit.
    return 1
  fi
  if [ $(isRCVersion "$1") ]; then
    echo "$1" | awk -F 'rc' '{printf("%src%d\n", $1, $2+1)}'
  else
    echo "$1" | awk -F '.' '/^([0-9]+\.){1,3}([0-9]+)$/ {
      for(i=1;i<=NF;i++) {
        if (i==NF) {
          printf("%d-rc1\n", $i+1)
        } else {
          printf("%d.", $i)
        }
      }
    }'
  fi
}
