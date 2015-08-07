#include "builtinmessages.h"
#include "signalhandler.h"
#include "jointnames.h"
#include <cassert>
#include <QDebug>



#include <lcmtypes/bot_core.hpp>
#include <lcmtypes/microstrain_comm.hpp>
#include <lcmtypes/vicon.hpp>
#include "lcmtypes/pronto/atlas_raw_imu_batch_t.hpp"
#include "lcmtypes/pronto/atlas_state_t.hpp"
#include "lcmtypes/pronto/atlas_state_extra_t.hpp"
#include "lcmtypes/pronto/atlas_status_t.hpp"
#include "lcmtypes/pronto/foot_contact_estimate_t.hpp"
#include "lcmtypes/pronto/robot_state_t.hpp"
#include "lcmtypes/mav/filter_state_t.hpp"



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

define_array_handler(RobotStateJointPositionHandler, pronto::robot_state_t, joint_position, JointNames::jointNames());
define_array_handler(RobotStateJointVelocityHandler, pronto::robot_state_t, joint_velocity, JointNames::jointNames());
define_array_handler(RobotStateJointEffortHandler, pronto::robot_state_t, joint_effort, JointNames::jointNames());

define_field_field_field_handler(RobotStatePoseTranslationXHandler, pronto::robot_state_t, pose, translation, x);
define_field_field_field_handler(RobotStatePoseTranslationYHandler, pronto::robot_state_t, pose, translation, y);
define_field_field_field_handler(RobotStatePoseTranslationZHandler, pronto::robot_state_t, pose, translation, z);

define_field_field_field_handler(RobotStatePoseRotationWHandler, pronto::robot_state_t, pose, rotation, w);
define_field_field_field_handler(RobotStatePoseRotationXHandler, pronto::robot_state_t, pose, rotation, x);
define_field_field_field_handler(RobotStatePoseRotationYHandler, pronto::robot_state_t, pose, rotation, y);
define_field_field_field_handler(RobotStatePoseRotationZHandler, pronto::robot_state_t, pose, rotation, z);

define_field_field_field_handler(RobotStateTwistLinearVelocityXHandler, pronto::robot_state_t, twist, linear_velocity, x);
define_field_field_field_handler(RobotStateTwistLinearVelocityYHandler, pronto::robot_state_t, twist, linear_velocity, y);
define_field_field_field_handler(RobotStateTwistLinearVelocityZHandler, pronto::robot_state_t, twist, linear_velocity, z);

define_field_field_field_handler(RobotStateTwistAngularVelocityXHandler, pronto::robot_state_t, twist, angular_velocity, x);
define_field_field_field_handler(RobotStateTwistAngularVelocityYHandler, pronto::robot_state_t, twist, angular_velocity, y);
define_field_field_field_handler(RobotStateTwistAngularVelocityZHandler, pronto::robot_state_t, twist, angular_velocity, z);

define_field_field_handler(RobotStateForceTorqueLFootForceZHandler, pronto::robot_state_t, force_torque, l_foot_force_z);
define_field_field_handler(RobotStateForceTorqueLFootTorqueXHandler, pronto::robot_state_t, force_torque, l_foot_torque_x);
define_field_field_handler(RobotStateForceTorqueLFootTorqueYHandler, pronto::robot_state_t, force_torque, l_foot_torque_y);

define_field_field_handler(RobotStateForceTorqueRFootForceZHandler, pronto::robot_state_t, force_torque, r_foot_force_z);
define_field_field_handler(RobotStateForceTorqueRFootTorqueXHandler, pronto::robot_state_t, force_torque, r_foot_torque_x);
define_field_field_handler(RobotStateForceTorqueRFootTorqueYHandler, pronto::robot_state_t, force_torque, r_foot_torque_y);

define_field_array_handler(RobotStateForceTorqueLHandForceHandler, pronto::robot_state_t, force_torque, l_hand_force, createIndexList(3));
define_field_array_handler(RobotStateForceTorqueLHandTorqueHandler, pronto::robot_state_t, force_torque, l_hand_torque, createIndexList(3));
define_field_array_handler(RobotStateForceTorqueRHandForceHandler, pronto::robot_state_t, force_torque, r_hand_force, createIndexList(3));
define_field_array_handler(RobotStateForceTorqueRHandTorqueHandler, pronto::robot_state_t, force_torque, r_hand_torque, createIndexList(3));



// atlas_state_t

define_array_handler(AtlasStateJointPositionHandler, pronto::atlas_state_t, joint_position, JointNames::jointNames());
define_array_handler(AtlasStateJointVelocityHandler, pronto::atlas_state_t, joint_velocity, JointNames::jointNames());
define_array_handler(AtlasStateJointEffortHandler, pronto::atlas_state_t, joint_effort, JointNames::jointNames());

define_field_field_handler(AtlasStateForceTorqueLFootForceZHandler, pronto::atlas_state_t, force_torque, l_foot_force_z);
define_field_field_handler(AtlasStateForceTorqueLFootTorqueXHandler, pronto::atlas_state_t, force_torque, l_foot_torque_x);
define_field_field_handler(AtlasStateForceTorqueLFootTorqueYHandler, pronto::atlas_state_t, force_torque, l_foot_torque_y);

define_field_field_handler(AtlasStateForceTorqueRFootForceZHandler, pronto::atlas_state_t, force_torque, r_foot_force_z);
define_field_field_handler(AtlasStateForceTorqueRFootTorqueXHandler, pronto::atlas_state_t, force_torque, r_foot_torque_x);
define_field_field_handler(AtlasStateForceTorqueRFootTorqueYHandler, pronto::atlas_state_t, force_torque, r_foot_torque_y);

// atlas_state_extra_t

define_array_handler(AtlasStateExtraJointPositionOutHandler, pronto::atlas_state_extra_t, joint_position_out, JointNames::jointNames());
define_array_handler(AtlasStateExtraJointVelocityOutHandler, pronto::atlas_state_extra_t, joint_velocity_out, JointNames::jointNames());
define_array_handler(AtlasStateExtraJointPressurePosHandler, pronto::atlas_state_extra_t, psi_pos, JointNames::jointNames());
define_array_handler(AtlasStateExtraJointPressureNegHandler, pronto::atlas_state_extra_t, psi_neg, JointNames::jointNames());

// atlas_raw_imu_batch_t

define_array_array_handler(AtlasRawIMUBatchIMUDeltaRotation, pronto::atlas_raw_imu_batch_t, raw_imu, delta_rotation,  createIndexList(15), createIndexList(3));
define_array_array_handler(AtlasRawIMUBatchIMULinearAcceleration, pronto::atlas_raw_imu_batch_t, raw_imu, linear_acceleration, createIndexList(15), createIndexList(3));

// atlas_raw_imu_t ( a single message broken out from the above message )
define_array_handler(AtlasRawIMUPacketDeltaRotation, pronto::atlas_raw_imu_t, delta_rotation, createIndexList(3));
define_array_handler(AtlasRawIMUPacketLinearAcceleration, pronto::atlas_raw_imu_t, linear_acceleration, createIndexList(3));

// atlas_raw_imu_t ( a single message broken out from the above message )
define_array_handler(MicrostrainINSGyro, microstrain::ins_t, gyro, createIndexList(3));
define_array_handler(MicrostrainINSAccel, microstrain::ins_t, accel, createIndexList(3));

// pose_t
define_array_handler(PoseTypePositionHandler, bot_core::pose_t, pos, createIndexList(3));
define_array_handler(PoseTypeVelocityHandler, bot_core::pose_t, vel, createIndexList(3));
define_array_handler(PoseTypeOrientationHandler, bot_core::pose_t, orientation, createIndexList(4));
define_array_handler(PoseTypeRotationRateHandler, bot_core::pose_t, rotation_rate, createIndexList(3));
define_array_handler(PoseTypeAcceleration, bot_core::pose_t, accel, createIndexList(3));



// vicon body_t
define_array_handler(ViconBodyTransHandler, vicon::body_t, trans, createIndexList(3));
define_array_handler(ViconBodyQuatHandler, vicon::body_t, quat, createIndexList(4));

// atlas_status_t
define_field_handler(AtlasStatusPumpInletPressure, pronto::atlas_status_t, pump_inlet_pressure);
define_field_handler(AtlasStatusPumpSupplyPressure, pronto::atlas_status_t, pump_supply_pressure);
define_field_handler(AtlasStatusPumpReturnPressure, pronto::atlas_status_t, pump_return_pressure);
define_field_handler(AtlasStatusAirSumpPressure, pronto::atlas_status_t, air_sump_pressure);
define_field_handler(AtlasStatusPumpRPM, pronto::atlas_status_t, current_pump_rpm);
define_field_handler(AtlasStatusBehavior, pronto::atlas_status_t, behavior);



// foot_contact_estimate_t
define_field_handler(FootContactLeft, pronto::foot_contact_estimate_t, left_contact);
define_field_handler(FootContactRight, pronto::foot_contact_estimate_t, right_contact);

// mav_filter_state_t
define_array_handler(MavStateHandler, mav::filter_state_t, state, createIndexList(21));



void BuiltinMessages::registerBuiltinChannels(SignalHandlerFactory& factory)
{

  QStringList channels;
  channels
    << "ATLAS_COMMAND"
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
    factory.registerClass<AtlasStateJointPositionHandler>();
    factory.registerClass<AtlasStateJointVelocityHandler>();
    factory.registerClass<AtlasStateJointEffortHandler>();
    factory.registerClass<AtlasStateForceTorqueLFootForceZHandler>();
    factory.registerClass<AtlasStateForceTorqueLFootTorqueXHandler>();
    factory.registerClass<AtlasStateForceTorqueLFootTorqueYHandler>();
    factory.registerClass<AtlasStateForceTorqueRFootForceZHandler>();
    factory.registerClass<AtlasStateForceTorqueRFootTorqueXHandler>();
    factory.registerClass<AtlasStateForceTorqueRFootTorqueYHandler>();
    factory.registerClass<AtlasStateExtraJointPositionOutHandler>();
    factory.registerClass<AtlasStateExtraJointVelocityOutHandler>();
    factory.registerClass<AtlasStateExtraJointPressurePosHandler>();
    factory.registerClass<AtlasStateExtraJointPressureNegHandler>();
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
    factory.registerClass<PoseTypeAcceleration>();
    factory.registerClass<ViconBodyTransHandler>();
    factory.registerClass<ViconBodyQuatHandler>();
    factory.registerClass<AtlasStatusPumpInletPressure>();
    factory.registerClass<AtlasStatusPumpSupplyPressure>();
    factory.registerClass<AtlasStatusPumpReturnPressure>();
    factory.registerClass<AtlasStatusAirSumpPressure>();
    factory.registerClass<AtlasStatusPumpRPM>();
    factory.registerClass<AtlasStatusBehavior>();
    factory.registerClass<FootContactLeft>();
    factory.registerClass<FootContactRight>();
    factory.registerClass<MavStateHandler>();
}
