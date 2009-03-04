// =====================================================================================
//       Filename:  client_buffer.hpp
//    Description:  threaded buffer between robot and telnet
//        Created:  04/03/09 13:22:51
//         Author:  Karl Miller (km), karl.miller.km@gmail.com
// =====================================================================================
 
#ifndef ARSBAND_CLIENT_BUFFER_HPP
#define ARSBAND_CLIENT_BUFFER_HPP

#include <boost/thread.hpp>
#include "telnet_client.hpp"

template<class buffer_type>
struct completed_read : public completed_read_func
{
  completed_read(buffer_type& b) : buffer(b) {}

  virtual void operator()(const char* msg, int len)
  {
    buffer.append_text_to_buffer(msg, len);
  }

  private:
  buffer_type& buffer;
};

namespace arsband
{
  class robot;

  class client_buffer
  {
    friend class robot;
    std::string buffer;
    bool new_text_in_buffer;
    completed_read<client_buffer> func;
    boost::condition_variable cond;
    boost::mutex mut;
    boost::thread thread;

    public:
    client_buffer();
    ~client_buffer();
    void start_thread(boost::asio::io_service& io_service); 
    void append_text_to_buffer(const char*, int);
    void wait_for_new_text();
    void clear_buffer();
    std::string operator()() const;
  };
}

#endif
