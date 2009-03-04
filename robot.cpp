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
#include <boost/regex.hpp>

typedef boost::asio::ip::tcp asio_tcp;

namespace arsband
{
  robot::robot(boost::asio::io_service& io_service, asio_tcp::resolver::iterator io_iterator)
    : incoming_buffer(), new_text_in_buffer(false), func(*this), cond(), mut(),
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
    check_connection_text();

    check_logon_text();

    check_time_report();

    std::string saybye = "say Bye!\n";
    write_line(saybye);
    std::string logout = "QUIT\n";
    write_line(logout);
  }

  void robot::check_connection_text()
  {
    wait_for_new_text();

    boost::regex welcome("^Welcome to ArsBand MUSH$");
    if(boost::regex_search(incoming_buffer, welcome))
    {
      std::cerr << "Welcomed.\n";
      clear_buffer();

      std::string login = "connect Robot robot\n";
      write_line(login);
    }
    else
    {
      std::cerr << "Could not find 'welcome'\n";
      // TODO: disrupt control flow somehow
      // TODO: FSM??
    }
  }

  void robot::check_logon_text()
  {
    wait_for_new_text();

    boost::regex end_login("^Last connect");
    if(boost::regex_search(incoming_buffer, end_login))
    {
      std::cerr << "Completed login.\n";
    }
    else
    {
      std::cerr << "Could not find end of login\n";
    }

    boost::smatch what_room;
    boost::regex room_name("^(.*)\\(#\\d+R.*\\)$");
    if(boost::regex_search(incoming_buffer, what_room, room_name, boost::match_not_dot_newline))
    {
      std::cerr << "Location: [" << what_room.str(1) << "]\n";
    }
    else
    {
      std::cerr << "Could not find room name.\n";
    }

    boost::smatch what_contents;
    boost::regex contents("^Contents:$");
    if(boost::regex_search(what_room.suffix().str(), what_contents, contents))
    {
      std::cerr << "Location desc:\n";
      what_contents.format(std::ostream_iterator<const char>(std::cerr), "[$`]\n"); 
    }
    else
    {
      std::cerr << "Could not find end of desc.\n";
    }

    boost::smatch what_exits;
    boost::regex exits("^Obvious exits:$");
    if(boost::regex_search(what_contents.suffix().str(), what_exits, exits)) 
    {
      std::cerr << "Contents:\n";
      // TODO: grab a name from each line here
      what_exits.format(std::ostream_iterator<const char>(std::cerr), "[$`]\n"); 
      std::cerr << "Exits:\n";
      what_exits.format(std::ostream_iterator<const char>(std::cerr), "[$']\n"); 
    }
    else
    {
        std::cerr << "Could not find end of contents.\n";
    }

    clear_buffer();

    std::string sayhi = "say Hi!\n";
    write_line(sayhi);
  }

  void robot::check_time_report()
  {
    std::string saytime = "say time()\n";
    write_line(saytime);

    wait_for_new_text();
    //TODO: wait for time report
    clear_buffer();
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
    new_text_in_buffer = false;
    std::cerr << "\n" << incoming_buffer << "\n";
  }

  void robot::clear_buffer()
  {
    incoming_buffer.clear();
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

