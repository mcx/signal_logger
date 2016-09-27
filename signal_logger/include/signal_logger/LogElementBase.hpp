/*
 * LogElementBase.hpp
 *
 *  Created on: Sep 22, 2016
 *      Author: Gabriel Hottiger
 */

#pragma once

// Signal logger
#include "signal_logger/LogElementInterface.hpp"

// Eigen
#include "Eigen/Core"

namespace signal_logger {

//! Implementation for all types except Eigen Matrices
template <typename ValueType_, typename IsEigenType_ = void>
class LogElementBase: public LogElementInterface
{
 public:
  /** Constructor
   * @param ptr     pointer to the data that shall be logged
   * @param name    name of the log element
   * @param unit    unit of the logged data
   * @param buffer_size size of the buffer (old elements will always be overwritten)
   * @return log element
  */
  LogElementBase(ValueType_ * ptr,
                 const std::string & name,
                 const std::string & unit,
                 const std::size_t buffer_size) :
    LogElementInterface(new Buffer<ValueType_>(buffer_size), typeid(ValueType_), name, unit),
    ptr_(ptr)
  {

  }

  //! Destructor
  virtual ~LogElementBase()
  {

  }

  //! Push data to the buffer
  void collectData()
  {
    push_front<ValueType_>(*ptr_);
  }

  //! Read value from Buffer
  void readDataFromBuffer(ValueType_* pItem)
  {
    pop_back<ValueType_>(pItem);
  }

 protected:
  ValueType_ * ptr_;

};

//! Implementation for Eigen Matrices
template <typename ValueType_>
class LogElementBase<ValueType_, typename std::enable_if<std::is_base_of<Eigen::MatrixBase<ValueType_>, ValueType_>::value>::type> : public LogElementInterface
{
 public:
  /** Constructor
   * @param ptr     pointer to the data that shall be logged
   * @param name    name of the log element
   * @param unit    unit of the logged data
   * @param buffer_size size of the buffer, since buffer is of type double this is
   *                    multiplied by nr_rows and nr_cols (old elements will always be overwritten)
   * @return log element
  */
  LogElementBase(ValueType_ * ptr,
                 const std::string & name,
                 const std::string & unit,
                 const std::size_t buffer_size) :
    LogElementInterface(new Buffer<typename ValueType_::Scalar>(buffer_size*ptr->rows()*ptr->cols()), typeid(typename ValueType_::Scalar), name, unit),
    ptr_(ptr),
    no_rows_(ptr->rows()),
    no_cols_(ptr->cols())
  {

  }

  //! Destructor
  virtual ~LogElementBase()
  {

  }

  //! Loop through matrix and push data to the buffer
  void collectData()
  {
    for (int r=0; r<no_rows_; r++)
    {
      for (int c=0; c<no_cols_; c++)
      {
        push_front<typename ValueType_::Scalar>( (*ptr_)(r,c) );
      }
    }
  }

  //! Read value from Buffer
  void readDataFromBuffer(ValueType_* pItem)
  {
    // Resize data to proper size
    pItem->resize(no_rows_, no_cols_);

    for (int r=0; r<no_rows_; r++)
    {
      for (int c=0; c<no_cols_; c++)
      {
        pop_back<typename ValueType_::Scalar>(&(*pItem)(r,c));
      }
    }
  }

  std::size_t rows()
  {
    return no_rows_;
  }

  std::size_t cols()
  {
    return no_cols_;
  }

 protected:
  ValueType_ * ptr_;

 private:
  std::size_t no_rows_;
  std::size_t no_cols_;

};

} /* namespace signal_logger */
