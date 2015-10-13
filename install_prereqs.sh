#!/bin/bash

case $1 in
  ("homebrew")
    brew install qt qwt ;;
  ("macports")
    port install qt4-mac qwt ;;
  ("ubuntu")
    apt-get install python-dev libqt4-dev libqwt-dev ;;
  ("cygwin")
    echo "WARNING: install_prereqs cygwin not implemented for this module" ;;
  (*)
    echo "Usage: ./install_prereqs.sh package_manager"
    echo "where package_manager is one of the following: "
    echo "  homebrew"
    echo "  macports"
    echo "  ubuntu"
    echo "  cygwin"
    exit 1 ;;
esac
