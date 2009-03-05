/*
 * =====================================================================================
 *       Filename:  robot_fsm.hpp
 *    Description:  FSM for ArsBand robot
 *        Created:  03/05/2009 11:38:17 AM
 *         Author:  Karl Miller (%km%), karl.miller.km@gmail.com
 * =====================================================================================
 */

#ifndef ARSBAND_ROBOT_FSM_HPP
#define ARSBAND_ROBOT_FSM_HPP

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/mpl/list.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

namespace sc = boost::statechart;
namespace mpl = boost::mpl;
namespace btime = boost::posix_time;

namespace arsband
{
  struct EvReceived : sc::event< EvReceived >
  {
    static btime::ptime then;
    EvReceived(std::string msg)
    { 
      btime::ptime now = btime::second_clock::local_time();
      if (now - then > btime::seconds(1))
      {
        std::cerr << "\nReceived: " << now << "\n"; 
        then = now;
      }
      else
      {
        std::cerr << "\n" << btime::to_simple_string(now - then) << "\n";
      }
      std::cerr << msg;
    }
  };
  btime::ptime EvReceived::then = btime::second_clock::local_time() - btime::seconds(2);

  struct MUSH;
  struct robot::fsm : sc::state_machine< robot::fsm, MUSH >
  {
    fsm()
    {
      initiate();
    };
  };
  struct MUSH : sc::simple_state< MUSH, robot::fsm >
  {
    typedef sc::transition< EvReceived, MUSH > reactions;
    MUSH()
    {
      std::cerr << "new FSM state: MUSH\n";
    };
  };
}

#endif

