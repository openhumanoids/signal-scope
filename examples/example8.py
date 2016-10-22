'''
This example shows how to define a custom function that outputs multiple values.
The function must return two values, the timestamp in microseconds
and an array of signal values.  The function documentation and the key
are used to define a label or name for the signal, unless labels are
explicitly specified.
'''

def myFunction(msg):
    '''velocity in mph'''
    return msg.utime, [v*2.23694 for v in msg.vel]


addSignalFunctions('POSE_BODY', myFunction, [0, 1, 2])