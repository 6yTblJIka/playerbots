#pragma once
#include "MovementActions.h"

namespace ai
{
	class ReviveFromCorpseAction : public MovementAction {
	public:
		ReviveFromCorpseAction(PlayerbotAI* ai) : MovementAction(ai, "revive") {}

    public:
        virtual bool Execute(Event event);
    };

	class SpiritHealerAction : public Action {
	public:
	    SpiritHealerAction(PlayerbotAI* ai) : Action(ai, "spirit healer") {}

    public:
        virtual bool Execute(Event event);
    };

}
