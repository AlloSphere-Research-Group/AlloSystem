#!/bin/sh

OS=$(uname -s)

# OS X
if [ "$OS" = "Darwin" ]; then
	{
		system_profiler SPSoftwareDataType
		echo '----------------------------'
		uname -a
		echo '----------------------------'
		gcc -v -x c++ /dev/null -fsyntax-only
		echo '----------------------------'
		xcodebuild -version
	} >> bug_report_system_info.txt 2>&1 # Also pickup stderr.
elif [ "$OS" = "Linux" ]; then
	{
		lsb_release -idrc
		echo '----------------------------'
		uname -a
		echo '----------------------------'
		gcc -v -x c++ /dev/null -fsyntax-only
	} >> bug_report_system_info.txt 2>&1
fi
