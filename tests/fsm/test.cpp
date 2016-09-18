#include <type_traits>
#include <cassert>
#include <vector>
#include <ctime>
#include "tmpl/mpl_print.hpp"
//#include "tmpl/tmpl.h"

struct play { };
struct open_close { };
struct cd_detected {
  cd_detected (char const *, std::vector<std::clock_t> const & ) { }
};
struct cd_pause {};
struct stop {};

#include "StateMachine.hpp"

// concrete FSM implementation 
class player : public fsm::StateMachine<player>
{
 private:
    // the list of FSM states
    enum states {
        Empty, Open, Stopped, Playing, Paused
      , initial_state = Empty
    };

    void start_playback(play const&);
    void open_drawer(open_close const&);
    void close_drawer(open_close const&);
    void store_cd_info(cd_detected const&);
    void stop_playback(stop const&);
    void pause_playback(cd_pause const&);
    void resume_playback(play const&);
    void stop_and_open(open_close const&);

    friend class StateMachine<player>;
    typedef player p; // makes transition table cleaner

    // transition table
    using transition_table = mp_typelist<
    //    Start     Event         Next      Action
    //  +---------+-------------+---------+---------------------+
    row < Stopped , play        , Playing , &p::start_playback  >,
    row < Stopped , open_close  , Open    , &p::open_drawer     >,
    //  +---------+-------------+---------+---------------------+
    row < Open    , open_close  , Empty   , &p::close_drawer    >,
    //  +---------+-------------+---------+---------------------+
    row < Empty   , open_close  , Open    , &p::open_drawer     >,
    row < Empty   , cd_detected , Stopped , &p::store_cd_info   >,
    //  +---------+-------------+---------+---------------------+
    row < Playing , stop        , Stopped , &p::stop_playback   >,
    row < Playing , cd_pause       , Paused  , &p::pause_playback  >,
    row < Playing , open_close  , Open    , &p::stop_and_open   >,
    //  +---------+-------------+---------+---------------------+
    row < Paused  , play        , Playing , &p::resume_playback >,
    row < Paused  , stop        , Stopped , &p::stop_playback   >,
    row < Paused  , open_close  , Open    , &p::stop_and_open   >>;
    //  +---------+-------------+---------+---------------------+


/*typedef fsm::event_dispatcher<
      row<Stopped, play, Playing, &player::start_playback>
    , fsm::event_dispatcher<
          row<Paused, play, Playing, &player::resume_playback>
        , fsm::default_event_dispatcher
      >
  > dummy;*/
};

void player::start_playback(play const&) {}
void player::open_drawer(open_close const&) {}
void player::close_drawer(open_close const&) {}
void player::store_cd_info(cd_detected const&) {}
void player::stop_playback(stop const&) {}
void player::pause_playback(cd_pause const&) {}
void player::resume_playback(play const&) {}
void player::stop_and_open(open_close const&) {}


int main()
{
    player p;                      // An instance of the FSM

    p.ProcessEvent(open_close()); // user opens CD player
#if 0
    p.ProcessEvent(open_close()); // inserts CD and closes
    p.ProcessEvent(               // CD is detected
        cd_detected(
             "louie, louie"
           , std::vector<std::clock_t>( /* track lengths */ )
        )
    );
    p.ProcessEvent(play());       // etc.
    p.ProcessEvent(cd_pause());
    p.ProcessEvent(play());
    p.ProcessEvent(stop());
#endif

    return 0;
}
