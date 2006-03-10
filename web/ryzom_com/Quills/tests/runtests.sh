#!/bin/bash
#
# Example test runner shell script.  
#
# Change as needed but don't commit your changes.
#

# full path to the python interpretor
export PYTHON="/usr/bin/python2.3"

# path to ZOPE_HOME/lib/python
export SOFTWARE_HOME="/opt/zope/lib/python"

${PYTHON} runalltests.py
