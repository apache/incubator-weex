#!/bin/sh
commit_regex='^Merge.+|[+*-] \[android|ios|jsfm|html5|doc|website|example|test|all\] [^\n]{1,50}'

if ! grep -iqE "$commit_regex" "$1"; then
    echo "ERROR: commit log format is not correct! Please use this format:

{action} [{module}] {description}

    {action}:
        + add
        * update or bugfix
        - remove
    {module}
        Including: android, ios, jsfm, html5, doc, website, example, test, all

for example:
    + [android] add refreshing for WebView
    * [doc] update video auto-play property
    - [example] remove abc
        
See https://github.com/alibaba/weex/blob/dev/CONTRIBUTING.md#commit-log"

    exit 1
fi
# FIXME no effect after editor (like vim) exits 