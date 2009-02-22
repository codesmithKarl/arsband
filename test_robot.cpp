// =====================================================================================
//       Filename:  test_robot.cpp
// 
//    Description:  
// 
//        Created:  22/02/09 17:48:18
//         Author:  Karl Miller (km), karl.miller.km@gmail.com
// =====================================================================================

#include <string>
#include <UnitTest++/UnitTest++.h>

#include "robot.hpp"

std::vector<std::string> mock_mush_output;
std::vector<std::string>::iterator mock_msg;
std::vector<std::string> robot_replies;

// -------------------------------------------------------------------
// mock telnet_client
#include <boost/asio.hpp>
telnet_client::telnet_client(boost::asio::io_service& io_service, 
                             boost::asio::ip::tcp::resolver::iterator, 
                             completed_read_func* f)
: active_(true), io_service_(io_service), socket_(io_service),
  connect_timer_(io_service), connection_timeout(boost::posix_time::seconds(3)),
  func_(f)
{
  std::string connect = "Welcome to ArsBand MUSH";
  (*func_)(connect.c_str(), connect.length());
  mock_msg = mock_mush_output.begin();
}

void telnet_client::write(const char* msg, int len)
{
  std::string reply(msg, len);
  robot_replies.push_back(reply);

  std::string mock_stmt("???\n");
  if (mock_msg != mock_mush_output.end())
    mock_stmt = *mock_msg++;
  if (active_)
    (*func_)(mock_stmt.c_str(), mock_stmt.length());
}

void telnet_client::close() { active_ = false; }

bool telnet_client::active()
{
  return active_;
}
// -------------------------------------------------------------------


TEST(CommunicateInCorrectOrder)
{
  std::string expected_reply = "connect Robot robot\n";
  mock_mush_output.push_back("Last connect was from\n");
  mock_mush_output.push_back("You say \"Hi!\"\n");
  mock_mush_output.push_back("You say \"Mon Feb 23 00:36:14 2009\"\n");
  mock_mush_output.push_back("You say \"Bye!\"\n");

  boost::asio::io_service io_service;
  boost::asio::ip::tcp::resolver::iterator iterator;

  {
    arsband::robot robot(io_service, iterator);
    robot.activate();
  }

  CHECK_EQUAL(5, robot_replies.size());
  CHECK_EQUAL("connect Robot robot\n", robot_replies[0]);
  CHECK_EQUAL("say Hi!\n", robot_replies[1]);
  CHECK_EQUAL("say time()\n", robot_replies[2]);
  CHECK_EQUAL("say Bye!\n", robot_replies[3]);
  CHECK_EQUAL("QUIT\n", robot_replies[4]);
}

int main()
{
    std::cerr << "------------------ROBOT SPEC-----------------\n";
    int r = UnitTest::RunAllTests();
    std::cerr << "--------------------------------------------\n";
    std::cerr.flush();
    return r;
}

