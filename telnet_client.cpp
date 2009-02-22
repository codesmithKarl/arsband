/*
 * =====================================================================================
 *       Filename:  telnet_client.cpp
 *
 *    Description:  a simple telnet client adapted from a posting to boost-users by 
 *                  Jeff Gray on 2008-09-30
 *
 *        Created:  21/02/09 23:50:02
 *         Author:  Karl Miller (km), karl.miller.km@gmail.com
 * =====================================================================================
 */

#include "telnet_client.hpp"

#include <iostream>
#include <boost/bind.hpp>

typedef boost::asio::ip::tcp asio_tcp;

telnet_client::telnet_client(boost::asio::io_service& io_service, 
                             asio_tcp::resolver::iterator endpoint_iterator, 
                             completed_read_func* f)
: active_(true), io_service_(io_service), socket_(io_service),
  connect_timer_(io_service), connection_timeout(boost::posix_time::seconds(3)),
  func_(f)
{
  connect_start(endpoint_iterator);
}
 
// pass the write data to the do_write function via the io service in the other thread
void telnet_client::write(const char msg)
{
  io_service_.post(boost::bind(&telnet_client::do_write, this, msg));
}

// call the do_close function via the io service in the other thread
void telnet_client::close()
{
  std::cerr << "Closing by request\n";
  active_ = false;
  io_service_.post(boost::bind(&telnet_client::do_close, this, boost::system::error_code()));
}
 
// return true if the socket is still active
bool telnet_client::active()
{
  return active_;
}

// asynchronously connect a socket to the specified remote endpoint and call connect_complete when it completes or fails
void telnet_client::connect_start(asio_tcp::resolver::iterator endpoint_iterator)
  {
  asio_tcp::endpoint endpoint = *endpoint_iterator;
  socket_.async_connect(endpoint,
                        boost::bind(&telnet_client::connect_complete,
                                    this,
                                    boost::asio::placeholders::error,
                                    ++endpoint_iterator));
  // start a timer that will expire and close the connection if the connection cannot connect within a certain time
  connect_timer_.expires_from_now(connection_timeout); //boost::posix_time::seconds(connection_timeout));
  connect_timer_.async_wait(boost::bind(&telnet_client::do_close, this, boost::asio::placeholders::error));
}

// the connection to the server has now completed or failed and returned an error
void telnet_client::connect_complete(const boost::system::error_code& error, asio_tcp::resolver::iterator )
{
  if (!error) // success, so start waiting for read data
  {
    connect_timer_.cancel(); // the connection was successful, so cancel the timeout
    read_start();
  }
  else
  {
    std::cerr << "Connection failed\n";
    do_close(error);
  }
}

// Start an asynchronous read and call read_complete when it completes or fails
void telnet_client::read_start(void)
{
  socket_.async_read_some(boost::asio::buffer(read_msg_, max_read_length),
                          boost::bind(&telnet_client::read_complete,
                                      this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
}

// the asynchronous read operation has now completed or failed and returned an error
void telnet_client::read_complete(const boost::system::error_code& error, size_t bytes_transferred)
{
  if (!error)
  { // read completed, so process the data
//    std::cout.write(read_msg_, bytes_transferred); // echo to standard output
    (*func_)(read_msg_, bytes_transferred);
    read_start(); // start waiting for another asynchronous read again
  }
  else
  {
    do_close(error);
  }
}

// callback to handle write call from outside this class
void telnet_client::do_write(const char msg)
{
  bool write_in_progress = !write_msgs_.empty(); // is there anything currently being written?
  write_msgs_.push_back(msg); // store in write buffer
  if (!write_in_progress) // if nothing is currently being written, then start
    write_start();
}

// Start an asynchronous write and call write_complete when it completes or fails
void telnet_client::write_start(void)
{
  boost::asio::async_write(socket_,
                           boost::asio::buffer(&write_msgs_.front(), 1),
                           boost::bind(&telnet_client::write_complete,
                                       this,
                                       boost::asio::placeholders::error));
}

// the asynchronous read operation has now completed or failed and returned an error
void telnet_client::write_complete(const boost::system::error_code& error)
{
  if (!error)
  { // write completed, so send next write data
    write_msgs_.pop_front(); // remove the completed data
    if (!write_msgs_.empty()) // if there is anthing left to be written
      write_start(); // then start sending the next item in the buffer
  }
  else
  {
    std::cerr << "Write completed with an error.\n";
    do_close(error);
  }
}

// something has gone wrong, so close the socket & make this object inactive
void telnet_client::do_close(const boost::system::error_code& error)
{
  if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
    return; // ignore it because the connection cancelled the timer
  if (error)
    std::cerr << "Error: " << error.message() << std::endl; // show the error message
  else
    if (active_)
      std::cout << "Error: Connection did not succeed.\n";
  socket_.close();
  active_ = false;
}


