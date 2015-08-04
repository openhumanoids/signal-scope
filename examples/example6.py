'''
This example shows how to load extra LCM types.
By default, LCM types are searched for automatically.
All directories in the python path are searched for lcm
type packages (modules).  This example shows how to load
additional packages that are not in the python path.
'''


# This reads every .py file in the given directory and catalogs
# everything that is an lcm type python definition file.
findLCMTypes(os.path.expanduser('~/path/to/lcmtypes'))


# This looks for lcm packages.  An lcm package is a directory
# that contains an __init__.py file and lcm type python files.
findLCMModules(os.path.expanduser('~/path/to/lcmtypes'))


# This variable holds the python path.
# Each path in sys.path is searched automatically for
# lcm packages.  You can add a new path to this list by
# setting the PYTHONPATH environment variable, for example:
#
# export PYTHONPATH=$PYTHONPATH:/path/to/you/lcmtypes
#
print sys.path


# print out all the available lcm types:
print getMessageTypeNames()
