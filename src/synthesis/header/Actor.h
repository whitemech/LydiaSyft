#ifndef ACTOR_H
#define ACTOR_H

#include <string>

namespace Syft {
    enum class Role{ 
        Environment,
        MainAgent,
        PeerAgent
    };

    class Actor{
    private:
        Role role_;
        int id_;
    public:
        Actor(Role role, int id);

        bool is_agent() const;
        bool is_environment() const;
        
        Role role() const;
        int id() const;

        bool operator == (const Actor& other) const;
        bool operator != (const Actor& other) const;

        static Actor Environment();
        static Actor MainAgent();
        static Actor PeerAgent(int i);

    };
}

#endif