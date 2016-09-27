#! /bin/sh
./gradlew clean testDebugUnitTest jacocoTestReportDebug
osascript -e 'display notification "Test Done!" with title "Weex Unit Test"'
