# How to Contribute

Welcome to create [pull requests](https://github.com/apache/incubator-weex/compare) or join in our [mailing list](http://mail-archives.apache.org/mod_mbox/incubator-weex-dev/) for the bug fix, doc, example, suggestion and anything.

## Join in Weex Mailing List

In Weex community all discussions happen on mailing list.

Just send an email to `dev-subscribe@weex.incubator.apache.org` and follow the instructions to subscribe Weex dev mailing list. Then you will receive discussions and community messages with your personal email. You can also reply back or publish your opinions freely to join the community discussion.

You can also access the mail archives through the [web portal](http://mail-archives.apache.org/mod_mbox/incubator-weex-dev/).

*To unsubscribe the mailing list: send an email to `dev-unsubscribe@weex.incubator.apache.org` and follow the instructions.*

Besides Weex dev mailing list channel, there are `commits` and `user ` channels available for subscription. Check them out [here](http://mail-archives.apache.org/mod_mbox/#weex.incubator).

## Branch Management

### Principles

* Use `rebase` instead of `merge` when merging PRs when you work on a single branch, in order to keep it simple and clean for review.
* Only use `merge` when publishing beta and Apache release, or merging back changes on temporary branches back to `master` branch.
* **DO NOT USE FEATURE BRANCH** if it is really needed.

![Branch Management](https://user-images.githubusercontent.com/700736/42321369-4587dc10-808b-11e8-9d8c-6356fde71244.png)

### Permanent Branches

0. `master` branch
    * `master` is the development branch, which tracks the bleeding-edge daily development works and is unstable from time to time.
1. `beta` branch
    * It is the beta publish branch, we take is as the project's `Beta Release Channel`, each commit on this branch tracks a beta version, with a `beta release tag` attached to it.
2. `release` branch
    * The `release` is the Apache release publish branch, we take it as the project's `Stable Release Channel`, each commit on it tracks a release version, with a `release tag` attached to it.

### Temporary Branches

0. beta candidates branches
    * A beta candidate branch tracks a beta release process.
    * A beta version will be released weekly, usually a beta branch will be branched off from `master` branch every Thursday evening to start a beta release, bug fixes for the release goes to this branch, and it will be released on the next Monday morning, after merging into the `beta` branch, a beta release tag will be attached to the specific commit on `beta` branch to identify a beta release.
    * The branch name should be `beta-0.xx.x`, a beta release can only grow the last digit of the version number as the beta version number.
    * The tag name for a beta release should be `beta-0.xx.x`.
    * This beta candidate branch will be removed soon after it is merged into the `beta` branch.
1. release candidates branches
    * A release candidate branch tracks an Apache release process.
    * A release candidate branch will be branched off from the `beta` branch when we propose an Apache release, the frequency would be per month. We will follow the release process on this branch, including signoff, RC review, etc., after the Apache release vote, this branch will merge into `release` branch and a release tag will be attached to the merge commit to identifying an Apache release version.
    * The branch name should be `rc-0.xx.0`, and it grows the middle digits as a release number.
    * The release tag name for an Apache release should be `v0.xx.0`.
    * This release candidate branch will be removed soon after it is merged into the `release` branch.
2. feature branches
    * Usually, it is not recommended to open a feature branch unless you are working on a breaking change feature which affects master branch a lot; or you are working on something for the next release, which is not supposed to be published with this release version.
    * A feature branch can only be branched off from the `master` branch, and can only be merged back to `master` branch when complete. It is not allowed to start a beta or Apache release from feature branch. It should be deleted soon after it is merged back to `master`.
    * Start a discussion in the mailing list and get approval from PMC members before you want to branch off a feature branch.

## Commit Log

```
[{module|issueID}] {title}

{summary}
```

* `{module|issueID}`
    * Use a module name as a heads-up for the major changes in this commit.
    * Or use an Apache JIRA issue ID: `[Weex-xxx]` to track the changes for the issue.
    * The available module name includes but not limited to android, ios, jsfm, html5, component, doc, example, test, etc.
* `{title}`
    * A summary for your commit, no more than 80 characters including the module or issue ID.
* `{summary}`
    * Explain a little bit more about what you have changed in your commit, how does it design, potential impact on other modules, or what is your next move. More detailed you explain, more easily the codebase gets maintained.

A good commit log should look like this:

```
commit e110f0a32b6cfe6d0adb006a931f6d7fd9de7c01
Author: Adam Feng <cxfeng1@gmail.com>
Date:   Mon Jun 11 18:34:26 2018 +0800

  [Script] release preparing:

  1. add missing modules in package.json
  2. move html5 folder to runtime folder in release_file.rules
  3. modify js-framework path in build_from_source.sh
  4. add NDK environment setup to HOW-TO-BUILD.md
```

## Pull Request

You can [create pull requests](https://github.com/apache/incubator-weex/compare) in GitHub.

1. First, we suggest you have some discussion with the community (commonly in our mailing list) before you code.
2. Fork repo from [https://github.com/apache/incubator-weex/](https://github.com/apache/incubator-weex/)
3. Finish the job you want to do.
4. Create a pull request.

## Code Style Guide

### Objective-C

* Tabs for indentation (not space)
* `*` operator goes with the variable name (e.g. Type *variable;)
* Function definitions: place each brace on its own line.
* Other braces: place the open brace on the line preceding the code block; place the close brace on its own line.
* Use `#pragma marks` to categorize methods into functional groupings and protocol implementations
* Follow other guidelines on [GitHub Objective-C Style Guide](https://github.com/github/objective-c-style-guide)

### Java & Android

* Use [Google Java Style](https://google.github.io/styleguide/javaguide.html) as basic guidelines of java code.
* Follow [AOSP Code Style](https://source.android.com/source/code-style.html) for rest of android related code style.
