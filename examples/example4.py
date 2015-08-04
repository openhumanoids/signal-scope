
# Some lcm messages define an array of floats and an array of strings.
# This example shows how to use a string array to find an index into the float array.

addSignal('EST_ROBOT_STATE', msg.utime, msg.joint_position[msg.joint_name['l_leg_kny']])


# or define many signals at once:

joints = ['l_leg_hpx', 'l_leg_hpy', 'l_leg_hpz', 'l_leg_kny', 'l_leg_akx', 'l_leg_aky']
addSignals('EST_ROBOT_STATE', msg.utime, msg.joint_position, joints, keyLookup=msg.joint_name)



# You can also define your own function to do the string lookup.
# Note, in the following function, the string array is searched
# for every message that is received, but by using the syntax above,
# the string lookup is performed just once and then cached.

def myFunction(msg):
    '''l_leg_kny position'''
    idx = msg.joint_name.index('l_leg_kny')
    return msg.utime, msg.joint_position[idx]

addSignalFunction('EST_ROBOT_STATE', myFunction)
