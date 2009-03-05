// =====================================================================================
//       Filename:  robot.cpp
// 
//    Description: basic ArsBand robot 
// 
//        Created:  22/02/09 15:01:25
//         Author:  Karl Miller (km), karl.miller.km@gmail.com
// =====================================================================================

#include "robot.hpp"

#include <iostream>
#include <boost/regex.hpp>

#include "client_buffer.hpp"

// TODO: refactor towards FSM
// --> a single state FSM that copies all data from client to a log
// --> connection states

namespace arsband
{
  robot::robot(boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::iterator io_iterator)
    : buffer(new client_buffer()), 
    client(io_service, io_iterator, &buffer->func) 
  {
    buffer->start_thread(io_service);
  }

  robot::~robot()
  {
    client.close(); // close the telnet client connection
  }

  void robot::activate() 
  {
    check_connection_text();

    check_logon_text();

    check_time_report();

    std::string saybye = "say Bye!\n";
    send_line(saybye);
    std::string logout = "QUIT\n";
    send_line(logout);
  }

  void robot::check_connection_text()
  {
    buffer->wait_for_new_text();

    boost::regex welcome("^Welcome to ArsBand MUSH$");
    if(boost::regex_search((*buffer)(), welcome))
    {
      std::cerr << "Welcomed.\n";
      buffer->clear_buffer();

      std::string login = "connect Robot robot\n";
      send_line(login);
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
    buffer->wait_for_new_text();

    boost::regex end_login("^Last connect");
    if(boost::regex_search((*buffer)(), end_login))
    {
      std::cerr << "Completed login.\n";
    }
    else
    {
      std::cerr << "Could not find end of login\n";
    }

    boost::smatch what_room;
    boost::regex room_name("^(.*)\\(#\\d+R.*\\)$");
    if(boost::regex_search((*buffer)(), what_room, room_name, boost::match_not_dot_newline))
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

    buffer->clear_buffer();

    std::string sayhi = "say Hi!\n";
    send_line(sayhi);
  }

  void robot::check_time_report()
  {
    std::string saytime = "say time()\n";
    send_line(saytime);

    buffer->wait_for_new_text();
    //TODO: wait for time report
    buffer->clear_buffer();
  }

  void robot::send_line(std::string& line)
  {
    if (client.active())
    {
      client.write(line.c_str(), line.length());
      std::cerr << line;
    }
  }

}

