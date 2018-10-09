#!/usr/bin/env bash

# This script publishes the latest beta version of the project, merge beta changes to beta branch, and tags the beta release, then rebase the changes onto master branch, and cleanup temporary release branches as well.

dir_name=$(dirname $0)
source ${dir_name}/common.sh

command -v hub > /dev/null 2>&1 || { error "The signoff process needs command 'hub', Use 'brew install hub' to install it. exit."; exit 1; }

prepareWorkingDir

latest_ver=$(latestNonRCVersion)
beta_ver="0.0.1-beta1"
if [ -z "$latest_ver" ]; then
  info "Use ${YELLOW}$beta_ver${RESTORE} as the version number."
else
  beta_ver=$(increaseBetaVersion "$latest_ver")
fi

info "\nThe latest non-RC version is: ${CYAN}'$latest_ver'${RESTORE}"
info "This beta version number should be: ${CYAN}'$beta_ver'${RESTORE}"

remote_branch=$(git branch -r | awk '/'"$beta_ver"'-[0-9]+/ {print $1}')
if [ ! "$remote_branch" ]; then
  warning "Remote branch for beta version $beta_ver has not been created, which means you haven't started your beta release process. Run 'beta-branchoff.sh' to start the beta release process first."
  exit 1
fi

info "Remote branch name: $remote_branch"

latest_sha=$(git show $remote_branch --format=%h -s)
notice "\nMake sure the latest commit SHA on '$remote_branch' is:"
notice "${GREEN}$latest_sha${RESTORE}"
notice "If it is not the beta release point you want, update the '$remote_branch' first."
notice "Also make sure you have the permisison to push branch onto your $remote."

read -e -p "Continue? [Y/n] " -n 1
shouldContinue=${REPLY:=y}
if [ ${shouldContinue,,} = n ]; then
  info "Sure, bye."
  exit 0
fi

stashLocalChanges

beta_branch=$(git branch -r | awk '/'"$remote"'\/beta$/ {print $1}')
if [ ! "$beta_branch" ]; then
  error "Failed to find ${YELLOW}beta${RESTORE} branch on $remote, create it first before release."
  exit 1
fi

info "Merge beta release commit to beta branch...\n"
git checkout -B beta "$remote/beta"
git merge -q --no-summary --no-ff $latest_sha -m "Merge beta release: $beta_ver" --signoff
if [ $? -ne 0 ]; then
  error "\nFailed to merge beta release changes $latest_sha onto beta, please take a look and merge it manually."
  info "NOTICE: An interactive shell will be created to resolve the merge conflicts, please use ${YELLOW}git merge --continue${RESTORE} after conflicts get resolved, and ${YELLOW}exit${RESTORE} the interactive shell to continue.\n"
  bash
  notice "\nMerge conflicts resolved."
fi
info "Beta release changes merged into beta branch.\n"

git push "$remote" beta:beta
info "\n${GREEN}Done.${RESTORE}"

info "\nCreating beta release: ${YELLOW}$beta_ver${RESTORE}...\n"
hub release create -m "Beta Release: $beta_ver" -t "beta" $beta_ver
git fetch "$remote"
info "\n${GREEN}Released.${RESTORE}"

info "\nRebase beta release commit back onto $remote/master branch..."
local_branch=$(echo $remote_branch | awk -F '/' '{print $2}');
git checkout -B "$local_branch" "$remote_branch"
git rebase $remote/master
if [ $? -ne 0 ]; then
  error "\nFailed to rebase beta release changes $remote_branch onto master, please take a look and rebase it manually."
  info "NOTICE: An interactive shell will be created to resolve the rebase conflicts, please use ${YELLOW}git rebase --continue${RESTORE} after conflicts get resolved, and ${YELLOW}exit${RESTORE} the interactive shell to continue.\n"
  bash
  notice "\nRebase conflicts resolved."
fi
info "Beta release changes rebased $remote_branch onto master."
git push "$remote" "$local_branch":master
info "\n${GREEN}Done.${RESTORE}"

info "\nClean up local and remote beta release branches..."
git checkout master
git pull --rebase
git branch -D "$local_branch"
git push "$remote" :"$local_branch"
info "\n${GREEN}Done.${RESTORE}"
