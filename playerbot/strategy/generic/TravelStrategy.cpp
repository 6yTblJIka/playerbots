#include "botpch.h"
#include "../../playerbot.h"
#include "TravelStrategy.h"

using namespace ai;

TravelStrategy::TravelStrategy(PlayerbotAI* ai) : Strategy(ai)
{
}

NextAction** TravelStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("travel", ACTION_MOVE + 10), NULL);
}

void TravelStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        //"random",
        "no travel target",
        NextAction::array(0, new NextAction("choose travel target", ACTION_MOVE + 11), NULL)));

    triggers.push_back(new TriggerNode(
        "far from travel target",
        NextAction::array(0, new NextAction("move to travel target", ACTION_MOVE + 11), NULL)));
}
