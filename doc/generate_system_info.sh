echo > bug_report_system_info.txt
system_profiler SPSoftwareDataType >> bug_report_system_info.txt
echo "----------------------------" >> bug_report_system_info.txt
uname -a >> bug_report_system_info.txt
echo "----------------------------" >> bug_report_system_info.txt
gcc -v -x c++ /dev/null -fsyntax-only 2>> bug_report_system_info.txt
echo "----------------------------" >> bug_report_system_info.txt
xcodebuild -version >> bug_report_system_info.txt
