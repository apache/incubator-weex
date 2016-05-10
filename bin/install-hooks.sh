#!/bin/sh
ln -sF ../../bin/pre-commit.sh .git/hooks/pre-commit && chmod +x .git/hooks/pre-commit 
ln -sF ../../bin/commit-msg.sh .git/hooks/commit-msg && chmod +x .git/hooks/commit-msg 
ln -sF ../../bin/pre-push.sh .git/hooks/pre-push && chmod +x .git/hooks/pre-push 