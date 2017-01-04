/*
 * SignalLoggerStd.cpp
 *
 *  Created on: Sep 23, 2016
 *      Author: Gabriel Hottiger
 */

#include "signal_logger_std/SignalLoggerStd.hpp"

#include <algorithm>

namespace signal_logger_std {

SignalLoggerStd::SignalLoggerStd(const std::string & loggerPrefix):
        signal_logger::SignalLoggerBase(loggerPrefix),
        file_(),
        headerStream_(std::ostringstream::in | std::ostringstream::out),
        dataStream_(std::ios::in | std::ios::out | std::ios::binary)
{

}

SignalLoggerStd::~SignalLoggerStd()
{

}

//! Save all the buffered data into a log file
bool SignalLoggerStd::workerSaveData(const std::string & logFileName, signal_logger::LogFileType logfileType) {

  if(noCollectDataCallsCopy_ == 0) { return true; }

  if(logfileType != signal_logger::LogFileType::BINARY) { return false; }

  // Clear streams
  headerStream_.str(std::string());
  dataStream_.str(std::string());

  // Fill streams
  timeElement_->saveDataToLogFile(timeElement_->getTimeBufferCopy(), noCollectDataCallsCopy_, signal_logger::LogFileType::BINARY);

  for(auto & elem : enabledElements_) {

    if(elem.second->second->isCopySaved())
    {
      elem.second->second->saveDataToLogFile(timeElement_->getTimeBufferCopy(), noCollectDataCallsCopy_, signal_logger::LogFileType::BINARY);
    }
  }

  // Write string header
  file_.open(logFileName, std::ios::out | std::ios::trunc);
  file_ << "# Log File: " << logFileName << std::endl;
  file_ << "# Time synchronization offset: " << std::endl;
  file_ << noCollectDataCalls_.load() << std::endl;
  file_ << "# Number of Log Elements: " << std::endl;
  std::string headerString = headerStream_.str();
  file_ << std::count(headerString.begin(), headerString.end(), '\n') << std::endl;
  file_ << "# (Element Name) (Data Size In Bytes) (No Data Points) (Divider) (Buffer looping (1 or 0))" << std::endl;
  file_ << headerString; // Has and endl from the last element
  file_ << "# Binary Data" << std::endl;
  file_.close();

  // Write binary data
  file_.open(logFileName, std::ios::out | std::ios::app | std::ios::binary);
  file_ << dataStream_.rdbuf();

  // Close file
  file_.close();

  return true;
}

bool SignalLoggerStd::resetTimeLogElement(signal_logger::BufferType buffertype, double maxLogTime) {

  timeElement_.reset(new signal_logger_std::LogElementStd<signal_logger::TimestampPair>( &logTime_, loggerPrefix_ + std::string{"/time"},
                                                                                         "[s/ns]", 1, signal_logger::LogElementAction::SAVE,
                                                                                         maxLogTime*updateFrequency_, buffertype,
                                                                                         &headerStream_, &dataStream_));
  return true;
}

} /* namespace signal_logger_std */
