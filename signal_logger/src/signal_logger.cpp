/*!
 * @file     signal_logger.cpp
 * @author   Christian Gehring, C. Dario Bellicoso, Gabriel Hottiger
 * @date     June 23, 2013
 * @brief    Providing add function depending on current logger type.
 */

// signal_logger
#include "signal_logger/signal_logger.hpp"
#include "signal_logger/SignalLoggerNone.hpp"

// STL
#include <memory>

namespace signal_logger {

template void add( const double & var,
                   const std::string & name,
                   const std::string & group,
                   const std::string & unit,
                   const std::size_t divider,
                   const LogElementAction action,
                   const BufferType bufferType,
                   const std::optional<std::size_t> bufferSize);

template void add( const float & var,
                   const std::string & name,
                   const std::string & group,
                   const std::string & unit,
                   const std::size_t divider,
                   const LogElementAction action,
                   const BufferType bufferType,
                   const std::optional<std::size_t> bufferSize);

template void add( const int & var,
                   const std::string & name,
                   const std::string & group,
                   const std::string & unit,
                   const std::size_t divider,
                   const LogElementAction action,
                   const BufferType bufferType,
                   const std::optional<std::size_t> bufferSize);

template void add( const Eigen::Vector3d & var,
                   const std::string & name,
                   const std::string & group,
                   const std::string & unit,
                   const std::size_t divider,
                   const LogElementAction action,
                   const BufferType bufferType,
                   const std::optional<std::size_t> bufferSize);


//! Initialize logger with standard logger.
std::shared_ptr<SignalLoggerBase> logger(new SignalLoggerNone());


LoggerType getLoggerType() {
  if( dynamic_cast<signal_logger_ros::SignalLoggerRos*>(logger.get()) != nullptr ) {
    return LoggerType::TypeRos;
  }

  if( dynamic_cast<signal_logger_std::SignalLoggerStd*>(logger.get()) != nullptr ) {
    return LoggerType::TypeStd;
  }

  if( dynamic_cast<signal_logger::SignalLoggerNone*>(logger.get()) != nullptr ) {
    return LoggerType::TypeNone;
  }

  return LoggerType::TypeUnknown;
}

void setSignalLoggerNone() {
  logger.reset(new signal_logger::SignalLoggerNone());
}

void setSignalLoggerStd() {
  logger.reset(new signal_logger_std::SignalLoggerStd());
}

#ifndef ROS2_BUILD
void setSignalLoggerRos(ros::NodeHandle* nh) {
#else /* ROS2_BUILD */
void setSignalLoggerRos(rclcpp::Node::SharedPtr nh) {
#endif /* ROS2_BUILD */
  logger.reset(new signal_logger_ros::SignalLoggerRos(nh));
}

#ifndef ROS2_BUILD
void setSignalLogger(const std::string& name, ros::NodeHandle* nh) {
#else /* ROS2_BUILD */
void setSignalLogger(const std::string& name, rclcpp::Node::SharedPtr nh) {
#endif /* ROS2_BUILD */
  if (name.compare("ros") == 0) {
    signal_logger::setSignalLoggerRos(nh);
  }
  else if (name.compare("std") == 0) {
    signal_logger::setSignalLoggerStd();
  }
  else {
    signal_logger::setSignalLoggerNone();
  }
}

} /* namespace signal_logger */
