/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *    Description:  runs an ArsBand robot 
 *     Parameters:  hostname or address, port
 *        Created:  21/02/09 19:17:22
 *         Author:  Karl Miller (km), karl.miller.km@gmail.com
 *
 * =====================================================================================
 */

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


#include <iostream>
#include <boost/asio.hpp>
typedef boost::asio::ip::tcp asio_tcp;

#include "robot.hpp"


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
    asio_tcp::resolver resolver(io_service);
    asio_tcp::resolver::query query(argv[1], argv[2]);
    asio_tcp::resolver::iterator iterator = resolver.resolve(query);

    arsband::robot robot(io_service, iterator);
    robot.activate();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

