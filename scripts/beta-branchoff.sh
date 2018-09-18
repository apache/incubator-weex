#!/usr/bin/env bash

dir_name=$(dirname $0)
source ${dir_name}/common.sh

prepareWorkingDir

latest_ver=$(latestNonRCVersion)
beta_ver="0.0.1-beta1"
if [ -z "$latest_ver" ]; then
  info "Use ${YELLOW}$beta_ver${RESTORE} as the version number."
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

stashLocalChanges

info "\nCreating local branch: ${YELLOW}$beta_branch${RESTORE}..."
git checkout -B "$beta_branch" "$latest_sha"

info "\nPushing ${YELLOW}$beta_branch${RESTORE} onto $remote...\n"
git push "$remote" "$beta_branch":"$beta_branch"
git branch --set-upstream-to="$remote/$beta_branch"
info "\n${GREEN}Done.${RESTORE}"
info "\nNOTICE: Now you've started the release process for $beta_ver, please work on the branch $beta_branch for bug fixes, no feature allowed submitting on it. Keep it sync with $remote/$beta_branch before your release."
