'''
This example shows how to create multiple plots.
Each call to addPlot() creates a new plot.  Calls to add new
signals will add the signals to the most recently added plot.
'''


joints = ['l_leg_hpx', 'l_leg_hpy', 'l_leg_hpz', 'l_leg_kny', 'l_leg_akx', 'l_leg_aky']
names = msg.joint_name


addPlot()
addSignals('EST_ROBOT_STATE', msg.utime, msg.joint_position, joints, keyLookup=names)


# you can assign the plot to a variable and reference it later
p = addPlot()
addSignals('EST_ROBOT_STATE', msg.utime, msg.joint_velocity, joints, keyLookup=names, plot=p)


# you can define the default time window and Y axis limits when you create a plot
addPlot(timeWindow=20, yLimits=[-50, 50])

# you can define custom colors for your signals
import colorsys
N = len(joints)
HSV_tuples = [(x*1.0/N, 1.0, 1.0) for x in range(N)]
RGB_tuples = map(lambda x: colorsys.hsv_to_rgb(*x), HSV_tuples)


addSignals('EST_ROBOT_STATE', msg.utime, msg.joint_effort, joints, keyLookup=names, colors=RGB_tuples)
