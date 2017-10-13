---
title: 如何参与社区
type: guide
order: 6
has_chapter_content: true
version: 2.1
---

# 如何参与社区

## 加入 Weex 邮件列表 (Mailing List)

在 Weex 官方社区所有的信息都会汇聚于邮件列表。订阅开发邮件列表的方式是发送邮件到 `dev-subscribe@weex.incubator.apache.org` 然后你的邮箱会收到一封来自 Apache 的回信 (英文)，把 Apache 的这封回信再次回复过去，即可完成订阅。

这里需要强调的是 Apache 是一个面向全球的全英文的社区，所以我们希望您能够用英文在邮件列表中跟大家交流。

如果您确实对使用英文存在很大的疑虑或困惑，我们也非常欢迎大家来中文问答平台 [SegmentFault](https://segmentfault.com/t/weex) 提问，这里有非常多热心的开发者一起讨论和解答 Weex 相关的问题。

使用邮件列表的另外一个好处是它可以方便大家随时查阅之前的邮件记录: [http://mail-archives.apache.org/mod_mbox/incubator-weex-dev/](http://mail-archives.apache.org/mod_mbox/incubator-weex-dev/)

*如果您对邮件列表中的内容已经不再感兴趣，这里也有退订邮件列表的方法: 发送邮件至 `dev-unsubscribe@weex.incubator.apache.org` 并根据回信的提示进行二次回复确认，完成退订。*

除了开发 (dev) 邮件列表，我们还有另外几个的邮件列表供大家查阅：[http://mail-archives.apache.org/mod_mbox/#weex.incubator](http://mail-archives.apache.org/mod_mbox/#weex.incubator)

## 分支管理 (英)

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

### 用于PR的分支命名


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

## 提交日志格式 (英)

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

## 发送 Pull Request (英)

You can [create pull requests](https://github.com/apache/incubator-weex/compare) in GitHub.

1. First we suggest you have some discussion with the community (commonly in our mailing list) before you code.
2. Fork repo from [https://github.com/apache/incubator-weex/](https://github.com/apache/incubator-weex/)
3. Finish the job you want to do.
4. Create a pull request.

## 编码规范 (英)

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
