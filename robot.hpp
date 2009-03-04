// =====================================================================================
//       Filename:  robot.hpp
// 
//    Description:  the basic ArsBand robot 
// 
//        Created:  22/02/09 14:57:24
//         Author:  Karl Miller (km), karl.miller.km@gmail.com
// =====================================================================================

#ifndef ARSBAND_ROBOT_HPP
#define ARSBAND_ROBOT_HPP

#include <boost/utility.hpp>

#include "telnet_client.hpp"

namespace arsband
{
  class client_buffer;

  class robot : boost::noncopyable
  {
    public:
    robot(boost::asio::io_service& io_service, 
          boost::asio::ip::tcp::resolver::iterator io_iterator);
    ~robot();

    void activate();

    private:
    void send_line(std::string& line);

    void check_connection_text();
    void check_logon_text();
    void check_time_report();

    private:
    std::auto_ptr<client_buffer> buffer; 
    telnet_client client;
  };
}

#endif //guard
