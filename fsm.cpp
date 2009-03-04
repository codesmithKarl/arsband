// =====================================================================================
//       Filename:  fsm.cpp
// 
//    Description:  Finite State Machine for Arsband Borg
// 
//        Created:  26/02/09 23:49:10
//         Author:  Karl Miller (km), karl.miller.km@gmail.com
// =====================================================================================


#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/mpl/list.hpp>
#include <iostream>

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

// states hold local variables
// so we need a MUSH-wide state to hold MUSH-wide variables
// --> MUSH machine
// --> Connection state
// --> substates: Disconnected, LoggedOff, and LoggedOn

struct EvConnect : sc::event< EvConnect >
{
  EvConnect() { std::cout << "connecting...\n"; } 
};

struct EvLogon : sc::event< EvLogon >
{
  EvLogon() { std::cout << "logging on ...\n"; }
};

struct EvDisconnect : sc::event< EvDisconnect >
{
  EvDisconnect() { std::cout << "disconnecting...\n"; }
};

struct EvLogOff : sc::event< EvLogOff >
{
  EvLogOff() { std::cout << "logging off...\n"; }
};

struct Connection; 
struct MUSH : sc::state_machine< MUSH, Connection > {};

struct Disconnected;
struct LoggedOn;
struct LoggedOff;

struct Connection : sc::simple_state< Connection, MUSH, Disconnected >
{
  Connection()
  {
    std::cout << "Beginning connection to MUSH on localhost:port\n";
  };
  ~Connection()
  {
    std::cout << "Finishing connection to MUSH.\n";
  };
};

struct LoggedOn : sc::simple_state< LoggedOn, Connection >
{
  typedef mpl::list<
    sc::transition< EvDisconnect, Disconnected >,
    sc::transition< EvLogOff, LoggedOff >
    > reactions;
  LoggedOn() { std::cout << "Last connect from somewhere@sometime\n"; }
};

struct LoggedOff : sc::simple_state< LoggedOff, Connection >
{
  typedef mpl::list<
    sc::transition< EvLogon, LoggedOn >,
    sc::transition< EvDisconnect, Disconnected >
    > reactions;
  LoggedOff() { std::cout << "Welcome to ArsBand MUSH\n"; }
};

struct Disconnected : sc::simple_state< Disconnected, Connection >
{
  typedef sc::transition< EvConnect, LoggedOff > reactions;
  Disconnected() { std::cout << "No contact from MUSH\n"; }
};

int main()
{
  MUSH myMUSH;
  myMUSH.initiate();
  myMUSH.process_event(EvConnect());
  myMUSH.process_event(EvLogon());
  myMUSH.process_event(EvDisconnect());
  myMUSH.process_event(EvConnect());
  myMUSH.process_event(EvLogon());
  myMUSH.process_event(EvLogOff());
  myMUSH.process_event(EvDisconnect());
  return 0;
}

