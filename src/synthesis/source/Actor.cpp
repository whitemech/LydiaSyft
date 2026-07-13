#include "Actor.h"

namespace Syft {

Actor::Actor(Role role, int id) : role_(role), id_(id) {}

bool Actor::is_agent() const {
    return role_ == Role::MainAgent || role_ == Role::PeerAgent;
}

bool Actor::is_environment() const {
    return role_ == Role::Environment;
}
Role Actor::role() const { return role_;}
int Actor::id() const { return id_;}

bool Actor::operator==(const Actor &other) const {
    return role_ == other.role_ && id_ == other.id_;
}

bool Actor::operator!=(const Actor &other) const {
    return !(*this == other);
}

Actor Actor::Environment() { return Actor(Role::Environment, -1);}
Actor Actor::MainAgent() { return Actor(Role::MainAgent, 0);}
Actor Actor::PeerAgent(int i) { return Actor(Role::PeerAgent, i);}

}