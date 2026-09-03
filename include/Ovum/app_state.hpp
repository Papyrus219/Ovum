#ifndef OVUM_APP_STATE_HPP
#define OVUM_APP_STATE_HPP

#include <Eruptor/event/event.hpp>
#include <string_view>
#include <Eruptor/eruptor.hpp>
#include <Ovum/simulation_scene.hpp>
#include <Ovum/gp_communicator.hpp>

namespace ovum
{

class App;

class App_state
{
public:
    virtual void Init(App & app) = 0;
    virtual void Enter_state() = 0;

    virtual void Update() = 0;
    virtual void Render() = 0;
    virtual void React_to_event(const eruptor::event::Event & event) = 0;

    void Assign_app(App & app) {this->app = &app;}
    virtual std::string_view Get_state_name() = 0;

    ovum::App * app{};
};

}

#endif // OVUM_APP_STATE_HPP
