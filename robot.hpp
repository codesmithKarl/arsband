// =====================================================================================
//       Filename:  robot.hpp
// 
//    Description:  TODO
// 
//        Created:  22/02/09 14:57:24
//         Author:  Karl Miller (km), karl.miller.km@gmail.com
// =====================================================================================

#ifndef ARSBAND_ROBOT_HPP
#define ARSBAND_ROBOT_HPP

#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include "telnet_client.hpp"

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


namespace arsband
{
  class robot : boost::noncopyable
  {
    std::string incoming_buffer;
    bool new_text_in_buffer;
    completed_read<robot> func;
    boost::condition_variable cond;
    boost::mutex mut;
    telnet_client client;
    boost::thread thread;

    public:
    robot(boost::asio::io_service& io_service, 
          boost::asio::ip::tcp::resolver::iterator io_iterator);

    ~robot();

    void activate();

    void append_text_to_buffer(const char* msg, int len);

    private:
    void wait_for_new_text();

    void write_line(std::string& line);
  };
}

#endif //guard
