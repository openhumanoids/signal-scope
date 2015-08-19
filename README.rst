Introduction
============
signal-scope is a live plotting tool for LCM message fields.


Build Instructions
==================

The required system dependencies are:

  - LCM 1.0 or greater
  - Qt 4.8
  - Qwt 6.0 or greater
  - Python 2.7

To install dependencies on Ubuntu::

  sudo apt-get install python-dev libqt4-dev libqwt-dev

To install dependencies on Mac homebrew::

  brew install qt qwt

Next install LCM from source if you don't already have it.
Visit the lcm webpage: https://lcm-proj.github.io/build_instructions.html

Finally, to build the signal-scope software, run::

  make externals
  make

The output will be build/bin/signal-scope.


Examples
========

See the examples directory for scripts demonstrating
the basic usage of signal-scope.  Launch a python
script by using the File --> Open menu or by passing
the script filename on the command line to signal-scope.
