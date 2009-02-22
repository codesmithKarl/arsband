/*
 * =====================================================================================
 *       Filename:  telnet_client.hpp
 *
 *    Description:  a simple telnet client adapted from a posting to boost-users by 
 *                  Jeff Gray on 2008-09-30
 *
 *        Created:  21/02/09 23:36:25
 *         Author:  Karl Miller (km), karl.miller.km@gmail.com
 * =====================================================================================
 */

#ifndef TELNET_CLIENT_HPP
#define TELNET_CLIENT_HPP

#include <deque>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

// using = asio
using boost::asio::ip::tcp;


// clients override operator()
struct completed_read_func
{
//  virtual void operator()(const char* msg, int len)
  virtual void operator()(const char*, int) 
  {
    std::cout << "got something\n";
//    std::cout.write(msg, len);
  }
};


class telnet_client
{
  public:
    telnet_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator, completed_read_func* f);

    // pass the write data to the do_write function via the io service in the other thread
    void write(const char msg);

    // call the do_close function via the io service in the other thread
    void close();

    // return true if the socket is still active
    bool active();

  private:

    // maximum amount of data to read in one operation
    static const int max_read_length = 512;

    // asynchronously connect a socket to the specified remote endpoint and call connect_complete when it completes or fails
    void connect_start(tcp::resolver::iterator endpoint_iterator);

    // the connection to the server has now completed or failed and returned an error
    void connect_complete(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);

    // Start an asynchronous read and call read_complete when it completes or fails
    void read_start(void);

    // the asynchronous read operation has now completed or failed and returned an error
    void read_complete(const boost::system::error_code& error, size_t bytes_transferred);

    // callback to handle write call from outside this class
    void do_write(const char msg);

    // Start an asynchronous write and call write_complete when it completes or fails
    void write_start(void);

    // the asynchronous read operation has now completed or failed and returned an error
    void write_complete(const boost::system::error_code& error);

    // something has gone wrong, so close the socket & make this object inactive
    void do_close(const boost::system::error_code& error);

  private:
    bool active_; // remains true while this object is still operating
    boost::asio::io_service& io_service_; // the main IO service that runs this connection
    tcp::socket socket_; // the socket this instance is connected to
    boost::asio::deadline_timer connect_timer_;
    boost::posix_time::time_duration connection_timeout; // time to wait for the connection to succeed
    char read_msg_[max_read_length]; // data read from the socket
    std::deque<char> write_msgs_; // buffered write data
    completed_read_func* func_;
};

#endif // include guard

