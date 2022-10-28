# Chrome Android Dependency Analysis Tool
## Overview
As part of Chrome Modularization, this directory contains various tools for
analyzing the dependencies contained within the Chrome Android project.

## Usage
Start by generating a JSON dependency file with a snapshot of the dependencies
for your JAR files using the **JSON dependency generator** command-line tool.

This snapshot file can then be used as input for various other
analysis tools listed below.

## Command-line tools
The usage information for any of the following tools is also accessible via
`toolname -h` or `toolname --help`.
#### JSON Dependency Generator
Runs [jdeps](https://docs.oracle.com/javase/8/docs/technotes/tools/unix/jdeps.html)
(dependency analysis tool) on all JARs a root build target depends
on and writes the resulting dependency graph into a JSON file. The default
root build target is chrome/android:monochrome_public_bundle.
```
usage: generate_json_dependency_graph.py [-h] -C BUILD_OUTPUT_DIR -o OUTPUT
                                         [-t TARGET] [-j JDEPS_PATH]

optional arguments:
  -t TARGET, --target TARGET
                        Root build target.
  -j JDEPS_PATH, --jdeps-path JDEPS_PATH
                        Path to the jdeps executable.

required arguments:
  -C BUILD_OUTPUT_DIR, --build_output_dir BUILD_OUTPUT_DIR
                        Build output directory.
  -o OUTPUT, --output OUTPUT
                        Path to the file to write JSON output to. Will be
                        created if it does not yet exist and overwrite
                        existing content if it does.
```
#### Class Dependency Audit
Given a JSON dependency graph, output the class-level dependencies for a given
list of classes.

An example is given at the end of this page. To see the options:
```
tools/android/dependency_analysis/print_class_dependencies.py -h
```
#### Package Dependency Audit
Given a JSON dependency graph, output the package-level dependencies for a
given package and the class dependencies comprising those dependencies.

An example is given at the end of this page. To see the options:
```
tools/android/dependency_analysis/print_package_dependencies.py -h
```
#### Package Cycle Counting
Given a JSON dependency graph, counts package cycles up to a given size.

To see the options:
```
tools/android/dependency_analysis/count_cycles.py -h
```

## Example Usage
This Linux example assumes Chromium is contained in a directory `~/cr`
and that Chromium has been built as per the instructions
[here](https://chromium.googlesource.com/chromium/src/+/main/docs/linux/build_instructions.md),
although the only things these assumptions affect are the file paths.
```
$ tools/android/dependency_analysis/generate_json_dependency_graph.py -C out/Debug -o ~/graph.json
>>> Running jdeps and parsing output...
>>> Parsed class-level dependency graph, got 3239 nodes and 19272 edges.
>>> Created package-level dependency graph, got 500 nodes and 4954 edges.
>>> Dumping JSON representation to ~/graph.json.

tools/android/dependency_analysis/print_class_dependencies.py -f ~/graph.json -c AppHooks
>>> Printing class dependencies for org.chromium.chrome.browser.AppHooks:
>>> 35 inbound dependency(ies) for org.chromium.chrome.browser.AppHooks:
>>> 	org.chromium.chrome.browser.AppHooksImpl
>>> 	org.chromium.chrome.browser.ChromeActivity
>>> ...

tools/android/dependency_analysis/print_package_dependencies.py -f ~/graph.json -p chrome.browser
>>> Printing package dependencies for org.chromium.chrome.browser:
>>> 121 inbound dependency(ies) for org.chromium.chrome.browser:
>>> 	org.chromium.chrome.browser.about_settings -> org.chromium.chrome.browser
>>> 	1 class edge(s) comprising the dependency:
>>> 		AboutChromeSettings -> ChromeVersionInfo
>>> ...
```
