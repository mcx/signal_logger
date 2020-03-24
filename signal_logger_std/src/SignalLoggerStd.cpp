/*!
 * @file     SignalLoggerStd.cpp
 * @author   Gabriel Hottiger
 * @date     Sep 23, 2016
 * @brief    Implementation of a Std signal logger. Provides binary file storing functionality.
 */

#include "signal_logger_std/SignalLoggerStd.hpp"

namespace signal_logger_std {

SignalLoggerStd::SignalLoggerStd():
        signal_logger::SignalLoggerBase(),
        file_(),
        textStream_(std::ostringstream::in | std::ostringstream::out),
        binaryStream_(std::ios::in | std::ios::out | std::ios::binary)
{

}

SignalLoggerStd::~SignalLoggerStd()
{

}

//! Save all the buffered data into a log file
bool SignalLoggerStd::workerSaveData(const std::string & logFileName, const signal_logger::LogFileTypeSet & logfileTypes) {

  if(noCollectDataCallsCopy_ == 0) {
    MELO_WARN_STREAM("[SignalLoggerStd] Could not save logger data! Data count is zero.");
    return true;
  }

  bool success = true;

  // Loop through file types and store data
  for(const auto fileType : logfileTypes) {

    if(fileType == signal_logger::LogFileType::BINARY || fileType == signal_logger::LogFileType::CSV) {
      // Clear streams
      textStream_.str(std::string());
      textStream_.precision(5);
      binaryStream_.str(std::string());

      {
        boost::unique_lock<boost::shared_mutex> saveLoggerLock(loggerMutex_);
        // Fill streams
        timeElement_->saveDataToLogFile(*timeElement_, noCollectDataCallsCopy_, fileType);

        for(auto & elem : enabledElements_) {

          if(elem->second->getCopyOptions().isSaved())
          {
            elem->second->saveDataToLogFile(*timeElement_, noCollectDataCallsCopy_, fileType);
          }
        }
      }

      if(fileType == signal_logger::LogFileType::BINARY) {
        const std::string binaryLogFileName = logFileName + ".silo";
        // Write string header
        file_.open(binaryLogFileName, std::ios::out | std::ios::trunc);
        if(file_.is_open()) {
          file_ << "#  ---------------------------------------------------------- #" << std::endl;
          file_ << "# | DO NOT EDIT THIS FILE! It could corrupt the binary data! | " << std::endl;
          file_ << "#  ---------------------------------------------------------- #" << std::endl;
          file_ << "# Log File: " << binaryLogFileName << std::endl;
          file_ << "# Time synchronization offset: " << std::endl;
          file_ << noCollectDataCallsCopy_.load() << std::endl;
          file_ << "# Number of Log Elements: " << std::endl;
          std::string headerString = textStream_.str();
          file_ << std::count(headerString.begin(), headerString.end(), '\n') << std::endl;
          file_ << "# (Element Name) (Data Size In Bytes) (No Data Points) (Divider) (Buffer looping (1 or 0)) (Data Type)" << std::endl;
          file_ << headerString; // Has and endl from the last element
          file_ << "# Binary Data" << std::endl;
          file_.close();
        } else {
          MELO_WARN_STREAM("[SignalLoggerStd] Could not save binary log file! Could not open file " << binaryLogFileName << ".");
          success = false;
          continue;
        }

        // Write binary data
        file_.open(binaryLogFileName, std::ios::out | std::ios::app | std::ios::binary);
        if(file_.is_open()) {
          file_ << binaryStream_.rdbuf();
          file_.close();
        } else {
          MELO_WARN_STREAM("[SignalLoggerStd] Could not save binary log file! Could not open file " << binaryLogFileName << " in binary mode.");
          success = false;
          continue;
        }
      } else if(fileType == signal_logger::LogFileType::CSV) {
        const std::string csvLogFileName = logFileName + ".csv";
        file_.open(csvLogFileName, std::ios::out | std::ios::trunc);
        if(file_.is_open()) {
          file_ << textStream_.str();
          file_.close();
        } else {
          MELO_WARN_STREAM("[SignalLoggerStd] Could not save csv log file! Could not open file " << csvLogFileName << ".");
          success = false;
          continue;
        }
      } else {
        MELO_WARN_STREAM("[SignalLoggerStd] Could not save log file! File type not supported.");
        success = false;
        continue;
      }
    }
  }

  return success;
}

void SignalLoggerStd::initTimeLogElement() {
  timeElement_.reset( new signal_logger_std::LogElementStd<signal_logger::TimestampPair>(
    &logTime_, signal_logger::BufferType::FIXED_SIZE, 0, options_.loggerPrefix_ + std::string{"/time"},
    "[s/ns]", 1, signal_logger::LogElementAction::SAVE , &textStream_, &binaryStream_ ) );
}

} /* namespace signal_logger_std */
