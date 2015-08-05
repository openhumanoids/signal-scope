'''
This example demonstrates how to load plotting functions from
a separate file.  This is useful if you want to have several
plot files that share code.
'''

# The __file__ variable contains the path to this file.
# We'll use this filepath to locate util_example.py which
# is contained in the same directory.  We can use execfile()
# to run the utility script in this scope.
execfile(os.path.join(os.path.dirname(__file__), 'util_example.py'))


# the function makePlots is defined in util_example.py
makePlots(['l_leg_kny','r_leg_kny'])
