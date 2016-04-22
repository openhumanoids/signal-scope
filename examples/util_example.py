

def makePlots(joints):
    '''
    This function creates three plots for joint position, velocity, and effort.
    Each plot displays signals for measured values and commanded values.
    The input argument is a list of strings representing the joint names to plot.
    '''

    # string arrays for EST_ROBOT_STATE and ATLAS_COMMAND
    jn = msg.joint_name
    jns = msg.joint_names

    # position plot
    addPlot(yLimits=[-2.5, 2.5])
    for joint in joints:
        addSignal('EST_ROBOT_STATE', msg.utime, msg.joint_position[jn[joint]])
        addSignal('ATLAS_COMMAND', msg.utime, msg.position[jns[joint]])


    # effort plot
    addPlot(yLimits=[-100, 100])
    for joint in joints:
        addSignal('EST_ROBOT_STATE', msg.utime, msg.joint_effort[jn[joint]])
        addSignal('ATLAS_COMMAND', msg.utime, msg.effort[jns[joint]])


    # velocity plot
    addPlot(yLimits=[-2, 2])
    for joint in joints:
        addSignal('EST_ROBOT_STATE', msg.utime, msg.joint_velocity[jn[joint]])
        addSignal('ATLAS_COMMAND', msg.utime, msg.velocity[jns[joint]])

# you can select the point size, time window and curve style for all plots with the function formatOptions
# note that you should call it after you have added all your plots
setFormatOptions(pointSize=4,timeWindow=30.0,curveStyle="lines")