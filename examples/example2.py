'''
This example shows how to define multiple signals at once.

Note, the function addSignals (plural) is used instead of addSignal (singular).

In this example, pos is an array, and the next argument is a list of indices.
A new signal will be added for each index in the list.
'''


addSignals('POSE_BODY', msg.utime, msg.pos, [0, 1, 2])
