Tracing related scripts
=======================

This folder contains two tracing related scripts. `profile-chrome-startup.sh` is
a wrapper around build/android/adb_profile_chrome_startup except that it allows
you to run the startup tracing with the same parameters multiple times (plus a
few other things). The output of `profile-chrome-startup.sh` (or
build/android/adb_profile_chrome_startup) can then be processed by
`systrace-extract-startup.py`. `systrace-extract-startup.py` takes a directory
and a name prefix of multiple tracing reports and extracts some events from them
and then print it out as a tab separated table for you to copy and paste into
sheets for further numerical analysys. Both of these scripts together allow you
to run startup tracing on a device multiple times, extract the startup trace
events you are tracking from all the trace files and output it in an easy way
for you to perform numerical analysis on the results.


profile-chrome-startup.sh
-------------------------

Some useful things that the script does:

 - Numbers traces from repeated invocations. I.e. you can invoke
   profile-chrome-startup.sh foo/bar 10 times, which would produce
   foo/bar-10s.html, foo/bar-10s~1.html, foo/bar-10s~2.html …
   foo/bar-10s~9.html. Note that default duration (10s) is appended to the path,
   same is true for most other options.

 - Captures logcat of the tracing duration, and greps for processes started /
   killed. I added this option in early days, when Chrome was causing startup of
   various GMS processes, which was severely affecting startup time (especially
   on 512MiB). It's a good idea to keep an eye on "Processes started / died"
   section of the script output. Generally you should see only browser/renderer
   there. There might be some unrelated processes, but if you reliably see a
   processes being started with Chrome, that's a problem. (There is also --killg
   option, which causes the script to kill some GMS processes, but I'm not sure
   if it's relevant anymore).

 - Automatically grants READ_EXTERNAL_STORAGE / WRITE_EXTERNAL_STORAGE permissions.

 - Parses resulting trace file and extracts various startup timings (see below).

 - Has --repeat=N option, which causes script to do N measurements, and

Only the output_tag is required all other arguments would be set to reasonable
defaults.

```
  usage: profile-chrome-startup.sh <output_tag> [arguments]

  arguments:

  output_tag
      this is the directory+filename prefix for all the trace output starting from
      the current working directory. eg: w/android_go_512/with_patch
  --browser=BROWSER
      one of canary, dev, beta, stable or chrome (default). chrome refers to clankium
  --url=URL
      start chrome with URL
  --cold
      cold start chrome ie. drop page caches
  --warm
      (default) the opposite of cold
  --atrace=category1,category2
      enable only these categories for tracing
  --killg
      kills gapps and gms processes before starting tracing
  --trace-time=N
      trace for the first N seconds (default 10)
  --repeat=N
      repeat the tracing N times
  --analyze
      instead of tracing analyse the trace files (and meminfo files if applicable)
      of all trace runs with output_tag
  --meminfo
      runs dumpsys meminfo on the device before and after each trace
  --print-trace-events=EVENTS
      which events to extract from the tracefile to display
  --webapk=PACKAGE_NAME
      the WebAPK package name for WebAPK startup tracing
  --extra_chrome_categories=category1,category2
      extra chrome tracing categories to record
```

systrace-extract-startup.py
---------------------------

```
  usage: systrace-extract-startup.py [-h] [--print-none-histograms]
                                     [--print-events PRINT_EVENTS]
                                     [--experimental] [--report] [--csv]
                                     file_or_glob

  positional arguments:
    file_or_glob

  optional arguments:
    -h, --help            show this help message and exit
    --print-none-histograms
                          Print histograms with None values.
    --print-events PRINT_EVENTS
                          Print events matching the specified regex.
    --experimental        Enable experimental stuff.
    --report              Present information as a tab-separated table.
    --csv                 Separate report values by commas (not tabs).
```

