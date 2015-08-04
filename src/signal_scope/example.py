
import math

'''
import drc
import drake
import bot_core

loadMessageTypes(drc)
loadMessageTypes(bot_core)
loadMessageTypes(drake)
'''


joints = ['l_leg_hpx', 'l_leg_hpy', 'l_leg_hpz', 'l_leg_kny', 'l_leg_akx', 'l_leg_aky']
keyLookup = msg.joint_name


addPlot(timeWindow=25, yLimits=[-math.pi, math.pi])
addSignals('EST_ROBOT_STATE', msg.utime, msg.joint_position, joints, keyLookup=keyLookup)

addPlot(yLimits=[-10, 10])
addSignals('EST_ROBOT_STATE', msg.utime, msg.joint_velocity, joints, keyLookup=keyLookup)

addPlot()
addSignals('EST_ROBOT_STATE', msg.utime, msg.joint_effort, joints, keyLookup=keyLookup)

