#include "builtinmessages.h"
#include "signalhandler.h"
#include "jointnames.h"
#include <cassert>
#include <QDebug>



#include <lcmtypes/bot_core.hpp>
#include <lcmtypes/vicon.hpp>
#include "lcmtypes/pronto/filter_state_t.hpp"

#include "lcmtypes/drc/foot_contact_estimate_t.hpp"
#include "lcmtypes/drc/controller_debug_t.hpp"

namespace
{
  int ArrayIndexFromIntegerKey(const QString& key)
  {
    bool ok;
    int arrayIndex = key.toInt(&ok);
    return  ok ? arrayIndex : -1;
  }

  int ArrayIndexFromJointNameKey(const QString& key)
  {
    return JointNames::indexOfJointName(key);
  }

  int ArrayIndexFromKey(const QString& key)
  {
    int arrayIndex = ArrayIndexFromIntegerKey(key);
    if (arrayIndex >= 0)
    {
      return arrayIndex;
    }

    arrayIndex = ArrayIndexFromJointNameKey(key);
    if (arrayIndex >= 0)
    {
      return arrayIndex;
    }

    qDebug() << "Failed to convert array key: " << key;
    assert(arrayIndex >= 0);
    return arrayIndex;
  }

  int ArrayIndexFromKeys(const QList<QString>& keys, int keyIndex)
  {
    assert(keys.length() > keyIndex);
    return ArrayIndexFromKey(keys[keyIndex]);
  }

  QList<QString> createIndexList(int size)
  {
    QList<QString> indexList;
    for (int i = 0; i < size; ++i)
    {
      indexList << QString::number(i);
    }
    return indexList;
  }
}



//-----------------------------------------------------------------------------
#define compute_time_now \
  timeNow = SignalHandlerFactory::instance().getOffsetTime(msg.utime);


#define default_array_keys_function(className) \
  QList<QList<QString> > className::validArrayKeys() \
  { \
    return QList<QList<QString> >(); \
  }


//-----------------------------------------------------------------------------
#define declare_signal_handler(className) \
class className : public SignalHandler \
{ \
public: \
  className(const SignalDescription* desc); \
  virtual bool extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue); \
  static QString messageType(); \
  static QString fieldName(); \
  static QList<QList<QString> > validArrayKeys(); \
  virtual QString description(); \
protected: \
  int mArrayIndex; \
  int mArrayIndex2; \
  QString mArrayKey; \
  QString mArrayKey2; \
};


//-----------------------------------------------------------------------------
#define define_array_handler(className, _messageType, _fieldName, _arrayKeyFunction) \
declare_signal_handler(className); \
className::className(const SignalDescription* desc) : SignalHandler(desc) \
{ \
   mArrayIndex = ArrayIndexFromKeys(desc->mArrayKeys, 0); \
   mArrayKey = desc->mArrayKeys[0]; \
} \
QList<QList<QString> > className::validArrayKeys() \
{ \
  QList<QList<QString> > arrayKeys; \
  arrayKeys << _arrayKeyFunction; \
  return arrayKeys; \
} \
bool className::extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue) \
{ \
  _messageType msg; \
  if (msg.decode(rbuf->data, 0, 1000000) < 0) \
  { \
    return false;\
  } \
  compute_time_now \
  signalValue = msg._fieldName[mArrayIndex]; \
  return true; \
} \
QString className::messageType() { return #_messageType ; } \
QString className::fieldName() { return #_fieldName ; } \
QString className::description() { return QString("%1.%2[%3]").arg(this->messageType()).arg(this->fieldName()).arg(this->mArrayKey); }


//-----------------------------------------------------------------------------
#define define_array_array_handler(className, _messageType, _fieldName1, _fieldName2, _arrayKeyFunction1, _arrayKeyFunction2) \
declare_signal_handler(className); \
className::className(const SignalDescription* desc) : SignalHandler(desc) \
{ \
   mArrayIndex = ArrayIndexFromKeys(desc->mArrayKeys, 0); \
   mArrayIndex2 = ArrayIndexFromKeys(desc->mArrayKeys, 1); \
   mArrayKey = desc->mArrayKeys[0]; \
   mArrayKey2 = desc->mArrayKeys[1]; \
} \
QList<QList<QString> > className::validArrayKeys() \
{ \
  QList<QList<QString> > arrayKeys; \
  arrayKeys << _arrayKeyFunction1 << _arrayKeyFunction2; \
  return arrayKeys; \
} \
bool className::extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue) \
{ \
  _messageType msg; \
  if (msg.decode(rbuf->data, 0, 1000000) < 0) \
  { \
    return false;\
  } \
  compute_time_now \
  signalValue = msg._fieldName1[mArrayIndex]._fieldName2[mArrayIndex2]; \
  return true; \
} \
QString className::messageType() { return #_messageType ; } \
QString className::fieldName() { return #_fieldName1"."#_fieldName2 ; } \
QString className::description() { return QString("%1.%2[%3].%4[%5]").arg(this->messageType()).arg(#_fieldName1).arg(this->mArrayKey).arg(#_fieldName2).arg(this->mArrayKey2); }


//-----------------------------------------------------------------------------
#define define_field_array_handler(className, _messageType, _fieldName1, _fieldName2, _arrayKeyFunction) \
declare_signal_handler(className); \
className::className(const SignalDescription* desc) : SignalHandler(desc) \
{ \
   mArrayIndex = ArrayIndexFromKeys(desc->mArrayKeys, 0); \
   mArrayKey = desc->mArrayKeys[0]; \
} \
QList<QList<QString> > className::validArrayKeys() \
{ \
  QList<QList<QString> > arrayKeys; \
  arrayKeys << _arrayKeyFunction; \
  return arrayKeys; \
} \
bool className::extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue) \
{ \
  _messageType msg; \
  if (msg.decode(rbuf->data, 0, 1000000) < 0) \
  { \
    return false;\
  } \
  compute_time_now \
  signalValue = msg._fieldName1._fieldName2[mArrayIndex]; \
  return true; \
} \
QString className::messageType() { return #_messageType ; } \
QString className::fieldName() { return #_fieldName1"."#_fieldName2 ; } \
QString className::description() { return QString("%1.%2.%3[%4]").arg(this->messageType()).arg(#_fieldName1).arg(#_fieldName2).arg(this->mArrayKey); }


//-----------------------------------------------------------------------------
#define define_field_handler(className, _messageType, _fieldName) \
declare_signal_handler(className); \
className::className(const SignalDescription* desc) : SignalHandler(desc) { } \
default_array_keys_function(className) \
bool className::extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue) \
{ \
  _messageType msg; \
  if (msg.decode(rbuf->data, 0, 1000000) < 0) \
  { \
    return false;\
  } \
  compute_time_now \
  signalValue = msg._fieldName; \
  return true; \
} \
QString className::messageType() { return #_messageType ; } \
QString className::fieldName() { return #_fieldName ; } \
QString className::description() { return QString("%1.%2").arg(this->messageType()).arg(this->fieldName()); }


//-----------------------------------------------------------------------------
#define define_field_field_handler(className, _messageType, _fieldName1, _fieldName2) \
declare_signal_handler(className); \
className::className(const SignalDescription* desc) : SignalHandler(desc) { } \
default_array_keys_function(className) \
bool className::extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue) \
{ \
  _messageType msg; \
  if (msg.decode(rbuf->data, 0, 1000000) < 0) \
  { \
    return false;\
  } \
  compute_time_now \
  signalValue = msg._fieldName1._fieldName2; \
  return true; \
} \
QString className::messageType() { return #_messageType ; } \
QString className::fieldName() { return #_fieldName1"."#_fieldName2 ; } \
QString className::description() { return QString("%1.%2").arg(this->messageType()).arg(this->fieldName()); }

//-----------------------------------------------------------------------------
#define define_field_field_field_handler(className, _messageType, _fieldName1, _fieldName2, _fieldName3) \
declare_signal_handler(className); \
className::className(const SignalDescription* desc) : SignalHandler(desc) { } \
default_array_keys_function(className) \
bool className::extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue) \
{ \
  _messageType msg; \
  if (msg.decode(rbuf->data, 0, 1000000) < 0) \
  { \
    return false;\
  } \
  compute_time_now \
  signalValue = msg._fieldName1._fieldName2._fieldName3; \
  return true; \
} \
QString className::messageType() { return #_messageType ; } \
QString className::fieldName() { return #_fieldName1"."#_fieldName2"."#_fieldName3 ; } \
QString className::description() { return QString("%1.%2").arg(this->messageType()).arg(this->fieldName()); }


// robot_state_t

define_array_handler(RobotStateJointPositionHandler, bot_core::robot_state_t, joint_position, JointNames::jointNames());
define_array_handler(RobotStateJointVelocityHandler, bot_core::robot_state_t, joint_velocity, JointNames::jointNames());
define_array_handler(RobotStateJointEffortHandler, bot_core::robot_state_t, joint_effort, JointNames::jointNames());

define_field_field_field_handler(RobotStatePoseTranslationXHandler, bot_core::robot_state_t, pose, translation, x);
define_field_field_field_handler(RobotStatePoseTranslationYHandler, bot_core::robot_state_t, pose, translation, y);
define_field_field_field_handler(RobotStatePoseTranslationZHandler, bot_core::robot_state_t, pose, translation, z);

define_field_field_field_handler(RobotStatePoseRotationWHandler, bot_core::robot_state_t, pose, rotation, w);
define_field_field_field_handler(RobotStatePoseRotationXHandler, bot_core::robot_state_t, pose, rotation, x);
define_field_field_field_handler(RobotStatePoseRotationYHandler, bot_core::robot_state_t, pose, rotation, y);
define_field_field_field_handler(RobotStatePoseRotationZHandler, bot_core::robot_state_t, pose, rotation, z);

define_field_field_field_handler(RobotStateTwistLinearVelocityXHandler, bot_core::robot_state_t, twist, linear_velocity, x);
define_field_field_field_handler(RobotStateTwistLinearVelocityYHandler, bot_core::robot_state_t, twist, linear_velocity, y);
define_field_field_field_handler(RobotStateTwistLinearVelocityZHandler, bot_core::robot_state_t, twist, linear_velocity, z);

define_field_field_field_handler(RobotStateTwistAngularVelocityXHandler, bot_core::robot_state_t, twist, angular_velocity, x);
define_field_field_field_handler(RobotStateTwistAngularVelocityYHandler, bot_core::robot_state_t, twist, angular_velocity, y);
define_field_field_field_handler(RobotStateTwistAngularVelocityZHandler, bot_core::robot_state_t, twist, angular_velocity, z);

define_field_field_handler(RobotStateForceTorqueLFootForceZHandler, bot_core::robot_state_t, force_torque, l_foot_force_z);
define_field_field_handler(RobotStateForceTorqueLFootTorqueXHandler, bot_core::robot_state_t, force_torque, l_foot_torque_x);
define_field_field_handler(RobotStateForceTorqueLFootTorqueYHandler, bot_core::robot_state_t, force_torque, l_foot_torque_y);

define_field_field_handler(RobotStateForceTorqueRFootForceZHandler, bot_core::robot_state_t, force_torque, r_foot_force_z);
define_field_field_handler(RobotStateForceTorqueRFootTorqueXHandler, bot_core::robot_state_t, force_torque, r_foot_torque_x);
define_field_field_handler(RobotStateForceTorqueRFootTorqueYHandler, bot_core::robot_state_t, force_torque, r_foot_torque_y);

define_field_array_handler(RobotStateForceTorqueLHandForceHandler, bot_core::robot_state_t, force_torque, l_hand_force, createIndexList(3));
define_field_array_handler(RobotStateForceTorqueLHandTorqueHandler, bot_core::robot_state_t, force_torque, l_hand_torque, createIndexList(3));
define_field_array_handler(RobotStateForceTorqueRHandForceHandler, bot_core::robot_state_t, force_torque, r_hand_force, createIndexList(3));
define_field_array_handler(RobotStateForceTorqueRHandTorqueHandler, bot_core::robot_state_t, force_torque, r_hand_torque, createIndexList(3));

// raw_imu_batch_t
define_array_array_handler(AtlasRawIMUBatchIMUDeltaRotation, bot_core::kvh_raw_imu_batch_t, raw_imu, delta_rotation,  createIndexList(15), createIndexList(3));
define_array_array_handler(AtlasRawIMUBatchIMULinearAcceleration, bot_core::kvh_raw_imu_batch_t, raw_imu, linear_acceleration, createIndexList(15), createIndexList(3));

// atlas_raw_imu_t ( a single message broken out from the above message )
define_array_handler(AtlasRawIMUPacketDeltaRotation, bot_core::kvh_raw_imu_t, delta_rotation, createIndexList(3));
define_array_handler(AtlasRawIMUPacketLinearAcceleration, bot_core::kvh_raw_imu_t, linear_acceleration, createIndexList(3));

// Different type
define_array_handler(MicrostrainINSGyro, bot_core::ins_t, gyro, createIndexList(3));
define_array_handler(MicrostrainINSAccel, bot_core::ins_t, accel, createIndexList(3));

// pose_t
define_array_handler(PoseTypePositionHandler, bot_core::pose_t, pos, createIndexList(3));
define_array_handler(PoseTypeVelocityHandler, bot_core::pose_t, vel, createIndexList(3));
define_array_handler(PoseTypeOrientationHandler, bot_core::pose_t, orientation, createIndexList(4));
define_array_handler(PoseTypeRotationRateHandler, bot_core::pose_t, rotation_rate, createIndexList(3));
define_array_handler(PoseTypeAcceleration, bot_core::pose_t, accel, createIndexList(3));

// six_axis_force_torque_t
define_array_handler(SixAxisForceTorqueTypeForceHandler, bot_core::six_axis_force_torque_t, force, createIndexList(3));
define_array_handler(SixAxisForceTorqueTypeMomentHandler, bot_core::six_axis_force_torque_t, moment, createIndexList(3));



// vicon body_t
define_array_handler(ViconBodyTransHandler, vicon::body_t, trans, createIndexList(3));
define_array_handler(ViconBodyQuatHandler, vicon::body_t, quat, createIndexList(4));

// controller_debug_t
define_field_handler(ControllerDebugRightFoot, drc::controller_debug_t, r_foot_contact);
define_field_handler(ControllerDebugLeftFoot, drc::controller_debug_t, l_foot_contact);
define_field_handler(ControllerDebugSolverInfo, drc::controller_debug_t, info);
define_array_handler(ControllerDebugQddDes, drc::controller_debug_t, qddot_des, createIndexList(34));
define_array_handler(ControllerDebugU, drc::controller_debug_t, u, createIndexList(28));
define_array_handler(ControllerDebugAlpha, drc::controller_debug_t, alpha, createIndexList(90));
define_array_handler(ControllerDebugZMPerr, drc::controller_debug_t, zmp_err, createIndexList(2));

define_array_handler(AtlasControlJointsPositionHandler, bot_core::atlas_command_t, position, JointNames::jointNames());
define_array_handler(AtlasControlJointsVelocityHandler, bot_core::atlas_command_t, velocity, JointNames::jointNames());
define_array_handler(AtlasControlJointsEffortHandler, bot_core::atlas_command_t, effort, JointNames::jointNames());

define_array_handler(AtlasControlJointsKQPHandler, bot_core::atlas_command_t, k_q_p, JointNames::jointNames());
define_array_handler(AtlasControlJointsKQIHandler, bot_core::atlas_command_t, k_q_i, JointNames::jointNames());
define_array_handler(AtlasControlJointsKQDPHandler, bot_core::atlas_command_t, k_qd_p, JointNames::jointNames());
define_array_handler(AtlasControlJointsKFPHandler, bot_core::atlas_command_t, k_f_p, JointNames::jointNames());
define_array_handler(AtlasControlJointsFFQDHandler, bot_core::atlas_command_t, ff_qd, JointNames::jointNames());
define_array_handler(AtlasControlJointsFFQDDPHandler, bot_core::atlas_command_t, ff_qd_d, JointNames::jointNames());
define_array_handler(AtlasControlJointsFFFDHandler, bot_core::atlas_command_t, ff_f_d, JointNames::jointNames());
define_array_handler(AtlasControlJointsFFConstHandler, bot_core::atlas_command_t, ff_const, JointNames::jointNames());
define_array_handler(AtlasControlJointsKEffortHandler, bot_core::atlas_command_t, k_effort, JointNames::jointNames());
define_field_handler(AtlasControlJointsDesiredControllerPeriodHandler, bot_core::atlas_command_t, desired_controller_period_ms);

// foot_contact_estimate_t
define_field_handler(FootContactLeft, drc::foot_contact_estimate_t, left_contact);
define_field_handler(FootContactRight, drc::foot_contact_estimate_t, right_contact);

// pronto_filter_state_t
define_array_handler(FilterStateHandler, pronto::filter_state_t, state, createIndexList(21));


void BuiltinMessages::registerBuiltinChannels(SignalHandlerFactory& factory)
{

  QStringList channels;
  channels
    << "ATLAS_COMMAND"
    << "ATLAS_FOOT_POS_EST"
    << "ATLAS_IMU_PACKET"
    << "ATLAS_IMU_PACKET_FILTERED"
    << "ATLAS_STATE"
    << "ATLAS_STATE_FILTERED"
    << "ATLAS_STATE_FILTERED_ALT"
    << "ATLAS_STATE_EXTRA"
    << "ATLAS_STATUS"
    << "EST_ROBOT_STATE"
    << "EST_ROBOT_STATE_KF"
    << "EST_ROBOT_STATE_LP"
    << "EST_ROBOT_STATE_ECHO"
    << "EXPD_ROBOT_STATE"
    << "FOOT_CONTACT_ESTIMATE"
    << "FOOT_CONTACT_CLASSIFY"
    << "FORCE_PLATE_DATA"
    << "INS_ERR_UPDATE"
    << "LATENCY"
    << "MICROSTRAIN_INS"
    << "MICROSTRAIN_INS_DIFF"
    << "MICROSTRAIN_INS_ACC_TEST"
    << "MICROSTRAIN_INS_VEL_TEST"
    << "POSE_BDI"
    << "POSE_BODY"
    << "POSE_BODY_ALT"
    << "POSE_BODY_FOVIS_VELOCITY"
    << "POSE_IMU_VELOCITY"
    << "POSE_BODY_LEGODO_VELOCITY"
    << "POSE_BODY_LEGODO_VELOCITY_FAIL"
    << "POSE_VICON"
    << "LOCK_ERROR"
    << "SCALED_ROBOT_STATE"
    << "SE_INS_POSE_STATE"
    << "SE_MATLAB_DATAFUSION_REQ"
    << "STATE_ESTIMATOR_POSE"
    << "STATE_ESTIMATOR_STATE"
    << "TRUE_ROBOT_STATE"
    << "VICON_ATLAS"
    << "CONTROLLER_DEBUG"
    ;

  factory.addChannels(channels);
}

void BuiltinMessages::registerBuiltinHandlers(SignalHandlerFactory& factory)
{
    factory.registerClass<RobotStateJointPositionHandler>();
    factory.registerClass<RobotStateJointVelocityHandler>();
    factory.registerClass<RobotStateJointEffortHandler>();
    factory.registerClass<RobotStatePoseTranslationXHandler>();
    factory.registerClass<RobotStatePoseTranslationYHandler>();
    factory.registerClass<RobotStatePoseTranslationZHandler>();
    factory.registerClass<RobotStatePoseRotationWHandler>();
    factory.registerClass<RobotStatePoseRotationXHandler>();
    factory.registerClass<RobotStatePoseRotationYHandler>();
    factory.registerClass<RobotStatePoseRotationZHandler>();
    factory.registerClass<RobotStateTwistLinearVelocityXHandler>();
    factory.registerClass<RobotStateTwistLinearVelocityYHandler>();
    factory.registerClass<RobotStateTwistLinearVelocityZHandler>();
    factory.registerClass<RobotStateTwistAngularVelocityXHandler>();
    factory.registerClass<RobotStateTwistAngularVelocityYHandler>();
    factory.registerClass<RobotStateTwistAngularVelocityZHandler>();
    factory.registerClass<RobotStateForceTorqueLFootForceZHandler>();
    factory.registerClass<RobotStateForceTorqueLFootTorqueXHandler>();
    factory.registerClass<RobotStateForceTorqueLFootTorqueYHandler>();
    factory.registerClass<RobotStateForceTorqueRFootForceZHandler>();
    factory.registerClass<RobotStateForceTorqueRFootTorqueXHandler>();
    factory.registerClass<RobotStateForceTorqueRFootTorqueYHandler>();
    factory.registerClass<RobotStateForceTorqueLHandForceHandler>();
    factory.registerClass<RobotStateForceTorqueLHandTorqueHandler>();
    factory.registerClass<RobotStateForceTorqueRHandForceHandler>();
    factory.registerClass<RobotStateForceTorqueRHandTorqueHandler>();
    factory.registerClass<AtlasRawIMUBatchIMUDeltaRotation>();
    factory.registerClass<AtlasRawIMUBatchIMULinearAcceleration>();
    factory.registerClass<AtlasRawIMUPacketDeltaRotation>();
    factory.registerClass<AtlasRawIMUPacketLinearAcceleration>();
    factory.registerClass<MicrostrainINSGyro>();
    factory.registerClass<MicrostrainINSAccel>();
    factory.registerClass<PoseTypePositionHandler>();
    factory.registerClass<PoseTypeVelocityHandler>();
    factory.registerClass<PoseTypeOrientationHandler>();
    factory.registerClass<PoseTypeRotationRateHandler>();
    factory.registerClass<SixAxisForceTorqueTypeForceHandler>();
    factory.registerClass<SixAxisForceTorqueTypeMomentHandler>();
    factory.registerClass<PoseTypeAcceleration>();
    factory.registerClass<ViconBodyTransHandler>();
    factory.registerClass<ViconBodyQuatHandler>();
    factory.registerClass<ControllerDebugRightFoot>();
    factory.registerClass<ControllerDebugLeftFoot>();
    factory.registerClass<ControllerDebugSolverInfo>();
    factory.registerClass<ControllerDebugQddDes>();
    factory.registerClass<ControllerDebugU>();
    factory.registerClass<ControllerDebugAlpha>();
    factory.registerClass<ControllerDebugZMPerr>();
    factory.registerClass<AtlasControlJointsPositionHandler>();
    factory.registerClass<AtlasControlJointsVelocityHandler>();
    factory.registerClass<AtlasControlJointsEffortHandler>();
    factory.registerClass<AtlasControlJointsKQPHandler>();
    factory.registerClass<AtlasControlJointsKQIHandler>();
    factory.registerClass<AtlasControlJointsKQDPHandler>();
    factory.registerClass<AtlasControlJointsKFPHandler>();
    factory.registerClass<AtlasControlJointsFFQDHandler>();
    factory.registerClass<AtlasControlJointsFFQDDPHandler>();
    factory.registerClass<AtlasControlJointsFFFDHandler>();
    factory.registerClass<AtlasControlJointsFFConstHandler>();
    factory.registerClass<AtlasControlJointsKEffortHandler>();
    factory.registerClass<AtlasControlJointsDesiredControllerPeriodHandler>();
    factory.registerClass<FootContactLeft>();
    factory.registerClass<FootContactRight>();
    factory.registerClass<FilterStateHandler>();
}
