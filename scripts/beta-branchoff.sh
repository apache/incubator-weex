#!/usr/bin/env bash

dir_name=$(dirname $0)
source ${dir_name}/common.sh

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

function latestNotRCVersion () {
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

read -e -p "Your upstream git remote is? [origin]"
remote=${REPLY:="origin"}

info "Fetching latest changes and prune ${LGREEN}$remote${RESTORE}..."
git fetch -p -t "$remote"
if [ $? ]; then
  error "It is not able to fetch codes from your $remote, check if there is any issue."
  exit 1
fi
git remote prune "$remote"
info "${GREEN}Done.${RESTORE}"

latest_ver=$(latestNotRCVersion)
beta_ver="0.0.1-beta1"
if [ -z "$latest_ver" ]; then
  echo "Use ${YELLOW}$beta_ver${RESTORE} as the version number."
else
  beta_ver=$(increaseBetaVersion "$latest_ver")
fi

beta_branch="$beta_ver-$(date "+%y%m%d")"

info "\nThe latest non-RC version is: ${CYAN}'$latest_ver'${RESTORE}"
info "This beta version number should be: ${CYAN}'$beta_ver'${RESTORE}"
info "The beta release branch name should be: ${CYAN}'$beta_branch'${RESTORE}."

is_remote_branch_exist=$(git branch -r | awk '/'"$beta_ver"'/ {print $0}')
if [ "$is_remote_branch_exist" ]; then
  warning "Remote branch with beta version $beta_ver already exists, will quit."
  exit 1
fi

latest_sha=$(git show --format=%h -s)
notice "\nMake sure the latest commit SHA on master is:"
notice "${GREEN}$latest_sha${RESTORE}"
notice "If it is not the branch-off point you want, update the $remote/master first."
notice "Also make sure you have the permisison to push branch onto your $remote."

read -e -p "Continue? [Y/n] " -n 1
shouldContinue=${REPLY:=y}
if [ ${shouldContinue,,} = n ]; then
  info "Sure, bye."
  exit 0
fi

if [ "$(gitRepoModified)" ]; then
  read -e -p "Your working directory is modified, stash it? [Y/n]" -n 1
  shouldStash=${REPLY:=y}
  if [ ${shouldStash,,} = y ]; then
    git stash
  else
    info "Change ignored."
  fi
fi

echo -e "\nCreating local branch: ${YELLOW}$beta_branch${RESTORE}..."
git checkout -B "$beta_branch" "$latest_sha"

echo -e "\nPushing ${YELLOW}$beta_branch${RESTORE} onto $remote..."
git push "$remote" "$beta_branch":"$beta_branch"
git branch --set-upstream-to="$remote/$beta_branch"
echo "${GREEN}Done.${RESTORE}"
