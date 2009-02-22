/*
 * =====================================================================================
 *
 *       Filename:  telnet.cpp
 *        Created:  21/02/09 19:17:22
 *         Author:  Karl Miller (km), karl.miller.km@gmail.com
 *
 * =====================================================================================
 */

// TODO: refactor arsband_robot to seperate header/source files with namespace
// TODO: write a mock telnet_client and automate tests for the following functionalities:
//
// TODO: recognise some text (connect, logon, response to 'say time()')
// TODO: recognise others present
// TODO: greet others present by name when logging on
// TODO: respond to greetings directed at self (know own name)
// TODO: recognise logons
// TODO: greet new logons
// TODO: logoff after number of new logons
// TODO: multiple robots


#include "telnet_client.hpp"

/* telnet.cpp
   A simple demonstration telnet client with Boost asio

Parameters:
hostname or address
port - typically 23 for telnet service

To end the application, send Ctrl-C on standard input
*/

#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/utility.hpp>

using boost::asio::ip::tcp;


template<class robot_type>
struct completed_read : public completed_read_func
{
  completed_read(robot_type& r) : robot(r) {}

  virtual void operator()(const char* msg, int len)
  {
    robot.append_text_to_buffer(msg, len);
  }

  private:
  robot_type& robot;
};

class arsband_robot : boost::noncopyable
{
  std::string incoming_buffer;
  bool new_text_in_buffer;
  completed_read<arsband_robot> func;
  boost::condition_variable cond;
  boost::mutex mut;
  telnet_client client;
  boost::thread thread;

  public:
  arsband_robot(boost::asio::io_service& io_service, tcp::resolver::iterator io_iterator)
    : incoming_buffer(),
      new_text_in_buffer(false),
      func(*this), 
      cond(),
      mut(),
      client(io_service, io_iterator, &func), 
      thread(boost::bind(&boost::asio::io_service::run, &io_service))
  {}

  ~arsband_robot()
  {
    client.close(); // close the telnet client connection
    thread.join(); // wait for the IO service thread to close
  }

  void activate() 
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

  void append_text_to_buffer(const char* msg, int len)
  {
    {
      boost::lock_guard<boost::mutex> lock(mut);
      incoming_buffer.append(msg, len);
      new_text_in_buffer = true;  
    }
    cond.notify_one();
  }

  private:
  void wait_for_new_text()
  {
    boost::unique_lock<boost::mutex> lock(mut);
    while(!new_text_in_buffer)
    {
      cond.wait(lock);
    }
    std::cout << "\n" << incoming_buffer << "\n";
    incoming_buffer.clear();
    new_text_in_buffer = false;
  }

  void write_line(std::string& line)
  {
    for(unsigned i=0; i<line.length(); i++)
    {
      if (client.active())
      {
        client.write(line[i]);
        std::cout << line[i];
      }
      else
        break;
    }
  }
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: telnet <host> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;
    
    // resolve the host name and port number to an iterator that can be used to connect to the server
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], argv[2]);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    arsband_robot robot(io_service, iterator);
    robot.activate();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

