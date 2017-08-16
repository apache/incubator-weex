install:
    xcopy "$(SRCROOT)\*.cmake" "$(DSTROOT)\AppleInternal\tools\cmake" /e/v/i/h/y
    xcopy "$(SRCROOT)\tools\scripts\*" "$(DSTROOT)\AppleInternal\tools\scripts" /e/v/i/h/y
    xcopy "$(SRCROOT)\tools\vsprops\*" "$(DSTROOT)\AppleInternal\tools\vsprops" /e/v/i/h/y
