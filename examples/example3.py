'''
This example shows how to define a custom function.
The function must return two values, the timestamp in microseconds
and the signal value.  The function documentation is used to define
a label or name for the signal.
'''


import numpy

def myFunction(msg):
    '''velocity magnitude'''
    return msg.utime, numpy.linalg.norm(msg.vel)


addSignalFunction('POSE_BODY', myFunction)
