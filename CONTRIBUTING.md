# How to Contribute

Welcome to create [pull requests](https://github.com/apache/incubator-weex/compare) or join in our [mailing list](http://mail-archives.apache.org/mod_mbox/incubator-weex-dev/) for bugfix, doc, example, suggestion and anything.

## Join in Weex Mailing List

In Weex community all discussion will happen on mailing list.

Just send an email to `dev-subscribe@weex.incubator.apache.org` and follow the instructions to subscribe Weex dev mailing list. And then you will receive all discussions and community messages by your personal email. In the same time you can freely send your own emails to join in us.

At the same time you can see the archives of all the mails through the web: [http://mail-archives.apache.org/mod_mbox/incubator-weex-dev/](http://mail-archives.apache.org/mod_mbox/incubator-weex-dev/)

*If you won't follow the mailing list any more. There is another way to unsubscribe it: send an email to `dev-unsubscribe@weex.incubator.apache.org` and follow the instructions.*

Besides Weex dev mailing list, we also have some other mailing lists for you. You can check them out here: [http://mail-archives.apache.org/mod_mbox/#weex.incubator](http://mail-archives.apache.org/mod_mbox/#weex.incubator)

## Branch Management

```
release
 ↑
{version}
 ↑
master         <--- PR(feature/hotfix/typo)
```

0. `master` branch
    0. `master` is the stable developing branch.
    0. ***It's RECOMMENDED to commit hotfix (like typo) or feature PR to `master `***.
0. `{version}` branch
    0. `{version}` is used for every version which we consider for stable publish.
    0. e.g. `v0.16`
0. `release` branch
    0. `release` is the latest release branch,we will make tag and publish version on this branch.

### Branch Name For PR

```
{module}-{action}-{shortName}
```

* `{module}`, see [commit log module](#commit-log)
* `{action}`
    * `feature`: checkout from `{module}` and merge to `{module}` later. If `{module}` not exists, merge to `dev`
    * `bugfix`: like `feature`, for bugfix only
    * `hotfix`: checkout from `master` or release `tag`, merge to `master` and `{module}` later. If `{module}` not exists, merge to `dev`

for example:

* `android-bugfix-memory`
* `jsfm-feature-communication`
* `android-hotfix-compute-layout`

## Commit Log

```
{action} [{module}] {description}
```

* `{action}`
    * `+` add
    * `*` update or bugfix
    * `-` remove
* `{module}`
    * Including: android, ios, jsfm, html5, component, doc, website, example, test, all 
* `{description}`
    * Just make it as clear and simple as possible.

for example:

* `+ [android] close #123, add refreshing for WebView`
* `* [doc] fix #123, update video auto-play property`
* `- [example] remove abc`

## Pull Request

You can [create pull requests](https://github.com/apache/incubator-weex/compare) in GitHub.

1. First we suggest you have some discussion with the community (commonly in our mailing list) before you code.
2. Fork repo from [https://github.com/apache/incubator-weex/](https://github.com/apache/incubator-weex/)
3. Finish the job you want to do.
4. Create a pull request.

## Code Style Guide

### Objective-C

* Tabs for indentation(not spaces)
* `*` operator goes with the variable name (e.g. Type *variable;)
* Function definitions: place each brace on its own line.
* Other braces: place the open brace on the line preceding the code block; place the close brace on its own line.
* Use `#pragma marks` to categorize methods into functional groupings and protocol implementations
* Follow other guidelines on [GitHub Objective-C Style Guide](https://github.com/github/objective-c-style-guide)

### Java & Android 

* Use [Google Java Style](https://google.github.io/styleguide/javaguide.html) as basic guidelines of java code.
* Follow [AOSP Code Style](https://source.android.com/source/code-style.html) for rest of android related code style.
