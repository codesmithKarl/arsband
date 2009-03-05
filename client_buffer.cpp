// =====================================================================================
//       Filename:  client_buffer.cpp
//    Description:  threaded/buffer between robot and telnet
//        Created:  04/03/09 13:19:46
//         Author:  Karl Miller (km), karl.miller.km@gmail.com
// =====================================================================================

#include "client_buffer.hpp"

#include <boost/bind.hpp>

namespace arsband
{
  client_buffer::client_buffer() 
    : buffer(), new_text_in_buffer(), func(*this), cond(), mut(), thread() 
  {}

  void client_buffer::start_thread(boost::asio::io_service& io_service)
  {
    thread = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));
  }

  void client_buffer::append_text_to_buffer(const char* msg, size_t len)
  {
    {
      boost::lock_guard<boost::mutex> lock(mut);
      buffer.append(msg, len);
      new_text_in_buffer = true;  
    }
    cond.notify_one();
  }

  void client_buffer::wait_for_new_text()
  {
    boost::unique_lock<boost::mutex> lock(mut);
    while(!new_text_in_buffer)
    {
      cond.wait(lock);
    }
    new_text_in_buffer = false;
    std::cerr << "\n" << buffer << "\n";
  }

  std::string client_buffer::operator()() const
  {
    return buffer;
  }

  void client_buffer::clear_buffer()
  {
    buffer.clear();
  }

  client_buffer::~client_buffer()
  {
    thread.join(); // wait for the IO service thread to close
  }
}

