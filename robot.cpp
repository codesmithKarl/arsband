// =====================================================================================
//       Filename:  robot.cpp
// 
//    Description:  TODO
// 
//        Created:  22/02/09 15:01:25
//         Author:  Karl Miller (km), karl.miller.km@gmail.com
// =====================================================================================

#include "robot.hpp"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

typedef boost::asio::ip::tcp asio_tcp;

namespace arsband
{
  robot::robot(boost::asio::io_service& io_service, asio_tcp::resolver::iterator io_iterator)
    : incoming_buffer(),
    new_text_in_buffer(false),
    func(*this), 
    cond(),
    mut(),
    client(io_service, io_iterator, &func), 
    thread(boost::bind(&boost::asio::io_service::run, &io_service))
  {}

  robot::~robot()
  {
    client.close(); // close the telnet client connection
    thread.join(); // wait for the IO service thread to close
  }

  void robot::activate() 
  {
    wait_for_new_text();
    //TODO: check for connection text

    std::string login = "connect Robot robot\n";
    write_line(login);

    wait_for_new_text();
    //TODO: wait for logon

    std::string sayhi = "say Hi!\n";
    write_line(sayhi);

    std::string saytime = "say time()\n";
    write_line(saytime);

    wait_for_new_text();
    //TODO: wait for time report

    std::string saybye = "say Bye!\n";
    write_line(saybye);

    std::string logout = "QUIT\n";
    write_line(logout);
  }

  void robot::append_text_to_buffer(const char* msg, int len)
  {
    {
      boost::lock_guard<boost::mutex> lock(mut);
      incoming_buffer.append(msg, len);
      new_text_in_buffer = true;  
    }
    cond.notify_one();
  }

  void robot::wait_for_new_text()
  {
    boost::unique_lock<boost::mutex> lock(mut);
    while(!new_text_in_buffer)
    {
      cond.wait(lock);
    }
    std::cerr << "\n" << incoming_buffer << "\n";
    incoming_buffer.clear();
    new_text_in_buffer = false;
  }

  void robot::write_line(std::string& line)
  {
    if (client.active())
    {
      client.write(line.c_str(), line.length());
      std::cerr << line;
    }
  }

}

