/*
 * SignalLoggerRos.cpp
 *
 *  Created on: Sep 23, 2016
 *      Author: Gabriel Hottiger
 */

#include "signal_logger_ros/SignalLoggerRos.hpp"
#include "boost/filesystem.hpp"

namespace signal_logger_ros {

SignalLoggerRos::SignalLoggerRos(ros::NodeHandle * nh,
                                 bool saveToBagFile):
                                signal_logger_std::SignalLoggerStd(),
                                nh_(nh),
                                saveToBagFile_(saveToBagFile),
                                bagWriter_(nullptr)
{
  getLoggerConfigurationService_ = nh_->advertiseService("/sl_ros/get_logger_configuration", &SignalLoggerRos::getLoggerConfiguration, this);
  getLoggerElementService_ = nh_->advertiseService("/sl_ros/get_logger_element", &SignalLoggerRos::getLoggerElement, this);
  setLoggerElementService_ = nh_->advertiseService("/sl_ros/set_logger_element", &SignalLoggerRos::setLoggerElement, this);
  startLoggerService_ = nh_->advertiseService("/sl_ros/start_logger", &SignalLoggerRos::startLogger, this);
  stopLoggerService_ = nh_->advertiseService("/sl_ros/stop_logger", &SignalLoggerRos::stopLogger, this);
  saveLoggerDataService_ = nh_->advertiseService("/sl_ros/save_logger_data", &SignalLoggerRos::saveLoggerData, this);
  loadLoggerScriptService_ = nh_->advertiseService("/sl_ros/load_logger_script", &SignalLoggerRos::loadLoggerScript, this);
  isLoggerRunningService_  =  nh_->advertiseService("/sl_ros/is_logger_running", &SignalLoggerRos::isLoggerRunning, this);
}

SignalLoggerRos::~SignalLoggerRos()
{
  this->cleanup();
}


bool SignalLoggerRos::workerSaveData(const std::string & logFileName) {

  if(!saveToBagFile_) { return signal_logger_std::SignalLoggerStd::workerSaveData(logFileName); }

  // Open a new file
  std::string bagFileName = logFileName + std::string{".bag"};
  bagWriter_.reset(new bageditor::BagWriter(bagFileName));

  // Write bag file
  for(auto & elem : enabledElements_) {
    if(elem.second->second->isSaved())
    {
      elem.second->second->saveDataToLogFile(*timeElement_, noCollectDataCalls_.load());
    }
  }

  // Close file
  bagWriter_.reset();

  return true;
}


bool SignalLoggerRos::getLoggerConfiguration(signal_logger_msgs::GetLoggerConfigurationRequest& req,
                                            signal_logger_msgs::GetLoggerConfigurationResponse& res) {
  for(auto & elem : this->logElements_)
  {
    res.log_element_names.push_back(elem.first);
  }
  res.collect_frequency = updateFrequency_;
  res.logger_namespace = "/log";
  res.script_filepath = collectScriptFileName_;
  if(collectScriptFileName_.compare(0,1,"/") != 0)
  {
    res.script_filepath.insert(0, boost::filesystem::current_path().string() + std::string{"/"});
  }

  return true;
}

bool SignalLoggerRos::getLoggerElement(signal_logger_msgs::GetLoggerElement::Request& req,
                                       signal_logger_msgs::GetLoggerElement::Response& res)
{
  signal_logger_msgs::LogElement elem_msg;
  res.success = logElementtoMsg(req.name, elem_msg);
  res.log_element = elem_msg;
  return true;
}

bool SignalLoggerRos::setLoggerElement(signal_logger_msgs::SetLoggerElement::Request& req,
                                       signal_logger_msgs::SetLoggerElement::Response& res) {
  if(isCollectingData_)
  {
    res.success = false;
  } else {
    res.success = msgToLogElement(req.log_element);
  }

  return true;
}

bool SignalLoggerRos::startLogger(std_srvs::TriggerRequest& req,
                                  std_srvs::TriggerResponse& res) {
  res.success =  SignalLoggerBase::startLogger();
  return true;
}

bool SignalLoggerRos::stopLogger(std_srvs::TriggerRequest& req,
                                 std_srvs::TriggerResponse& res) {
  res.success =  SignalLoggerBase::stopLogger();
  return true;
}

bool SignalLoggerRos::saveLoggerData(std_srvs::TriggerRequest& req,
                                     std_srvs::TriggerResponse& res) {
  res.success = SignalLoggerBase::saveLoggerData();
  return true;
}


bool SignalLoggerRos::isLoggerRunning(std_srvs::TriggerRequest& req,
                                      std_srvs::TriggerResponse& res) {
  res.success = this->isCollectingData_;
  return true;
}

bool SignalLoggerRos::loadLoggerScript(signal_logger_msgs::LoadLoggerScriptRequest& req,
                                       signal_logger_msgs::LoadLoggerScriptResponse& res) {
  if(isCollectingData_)
  {
    res.success = false;
  } else {
    res.success = SignalLoggerBase::readDataCollectScript(req.filepath);
  }

  return true;
}

bool SignalLoggerRos::logElementtoMsg(const std::string & name, signal_logger_msgs::LogElement & msg)
{
  if( logElements_.find(name) == logElements_.end()) { return false; }

  msg.name = logElements_.at(name)->getName();
  msg.is_logged = logElements_.at(name)->isEnabled();
  msg.divider = logElements_.at(name)->getDivider();
  msg.buffer_size = logElements_.at(name)->getBufferSize();
  msg.no_items_in_buffer = logElements_.at(name)->noItemsInBuffer();
  msg.no_unread_items_in_buffer = logElements_.at(name)->noUnreadItemsInBuffer();

  switch(logElements_.at(name)->getAction()) {
    case signal_logger::LogElementAction::NONE:
      msg.action = signal_logger_msgs::LogElement::ACTION_NONE;
      break;
    case signal_logger::LogElementAction::SAVE_AND_PUBLISH:
      msg.action = signal_logger_msgs::LogElement::ACTION_SAVE_AND_PUBLISH;
      break;
    case signal_logger::LogElementAction::SAVE:
      msg.action = signal_logger_msgs::LogElement::ACTION_SAVE;
      break;
    case signal_logger::LogElementAction::PUBLISH:
      msg.action = signal_logger_msgs::LogElement::ACTION_PUBLISH;
      break;
    default:
      MELO_ERROR("Undefined action!");
      break;
  }

  switch(logElements_.at(name)->getBufferType()) {
    case signal_logger::BufferType::FIXED_SIZE:
      msg.buffer_type = signal_logger_msgs::LogElement::BUFFERTYPE_FIXED_SIZE;
      break;
    case signal_logger::BufferType::LOOPING:
      msg.buffer_type = signal_logger_msgs::LogElement::BUFFERTYPE_LOOPING;
      break;
    case signal_logger::BufferType::EXPONENTIALLY_GROWING:
      msg.buffer_type = signal_logger_msgs::LogElement::BUFFERTYPE_EXPONENTIALLY_GROWING;
      break;
    default:
      MELO_ERROR("Undefined buffer type!");
      break;
  }

  return true;
}

bool SignalLoggerRos::msgToLogElement(const signal_logger_msgs::LogElement & msg)
{
  auto element_iterator  = logElements_.find(msg.name);
  if( element_iterator == logElements_.end()) { return false; }

  logElements_.at(msg.name)->setIsEnabled(msg.is_logged);
  if(msg.is_logged)
  {
    enabledElements_.insert(std::pair<std::string, signal_logger::SignalLoggerBase::LogElementMapIterator>(msg.name, element_iterator));
  }
  else
  {
    enabledElements_.erase(msg.name);
  }

  logElements_.at(msg.name)->setDivider(msg.divider);
  logElements_.at(msg.name)->setBufferSize(msg.buffer_size);

  switch(msg.action) {
    case signal_logger_msgs::LogElement::ACTION_NONE:
      logElements_.at(msg.name)->setAction(signal_logger::LogElementAction::NONE);
      break;
    case signal_logger_msgs::LogElement::ACTION_SAVE_AND_PUBLISH:
      logElements_.at(msg.name)->setAction(signal_logger::LogElementAction::SAVE_AND_PUBLISH);
      break;
    case signal_logger_msgs::LogElement::ACTION_SAVE:
      logElements_.at(msg.name)->setAction(signal_logger::LogElementAction::SAVE);
      break;
    case signal_logger_msgs::LogElement::ACTION_PUBLISH:
      logElements_.at(msg.name)->setAction(signal_logger::LogElementAction::PUBLISH);
      break;
    default:
      MELO_ERROR("Undefined action!");
      break;
  }

  switch(msg.buffer_type) {
    case signal_logger_msgs::LogElement::BUFFERTYPE_FIXED_SIZE:
      logElements_.at(msg.name)->setBufferType(signal_logger::BufferType::FIXED_SIZE);
      break;
    case signal_logger_msgs::LogElement::BUFFERTYPE_LOOPING:
      logElements_.at(msg.name)->setBufferType(signal_logger::BufferType::LOOPING);
      break;
    case signal_logger_msgs::LogElement::BUFFERTYPE_EXPONENTIALLY_GROWING:
      logElements_.at(msg.name)->setBufferType(signal_logger::BufferType::EXPONENTIALLY_GROWING);
      break;
    default:
      MELO_ERROR("Undefined buffer type!");
      break;
  }

  return true;
}

bool SignalLoggerRos::cleanup() {
  signal_logger_std::SignalLoggerStd::cleanup();

  getLoggerConfigurationService_.shutdown();
  getLoggerElementService_.shutdown();
  setLoggerElementService_.shutdown();
  startLoggerService_.shutdown();
  stopLoggerService_.shutdown();
  saveLoggerDataService_.shutdown();
  loadLoggerScriptService_.shutdown();
  isLoggerRunningService_.shutdown();
  nh_ = nullptr;
  return true;
}


signal_logger::TimestampPair SignalLoggerRos::getCurrentTime() {
  signal_logger::TimestampPair timeStamp;

  // Get time in seconds and nanoseconds
  ros::Time curr_time = ros::Time::now();
  timeStamp.first = curr_time.sec;
  timeStamp.second = curr_time.nsec;

  return timeStamp;
}



} /* namespace signal_logger */
