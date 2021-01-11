#pragma once

#include "Common.h"
#include "../botpch.h"
#include "strategy/AiObject.h"

using namespace std::placeholders;



namespace ai
{
    //Extension of WorldLocation with distance functions.
    class WorldPosition
    {
    public:
        WorldPosition() { wLoc = WorldLocation(); }
        WorldPosition(uint32 mapid, float x, float y, float z, float orientation) { wLoc = WorldLocation(mapid, x, y, z, orientation); }
        WorldPosition(Player* bot) { wLoc = WorldLocation(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation()); }

        uint32 getMapId() { return wLoc.mapid; }

        WorldPosition relPoint(WorldPosition* center) { return WorldPosition(wLoc.mapid, wLoc.coord_x - center->wLoc.coord_x, wLoc.coord_x - center->wLoc.coord_x, wLoc.coord_z - center->wLoc.coord_z, wLoc.orientation); }

        float size() { return sqrt(pow(wLoc.coord_x, 2.0) + pow(wLoc.coord_y, 2.0) + pow(wLoc.coord_z, 2.0)); }
        float distance(WorldPosition* center) { return wLoc.mapid == center->getMapId() ? relPoint(center).size() : 100000; };
        WorldLocation getLocation() { return wLoc; }

        void addVisitor() { visitors++; }
        void remVisitor() { visitors--; }
        int getVistitor() { return visitors; }
    private:
        WorldLocation wLoc;
        int visitors = 0;
    };

    //A destination for a bot to travel to and do something.
    class TravelDestination
    {
    public:
        TravelDestination() {}        
        TravelDestination(float radiusMin1, float radiusMax1) { radiusMin = radiusMin1; radiusMax = radiusMax1; }
        TravelDestination(vector<WorldPosition*> points1, float radiusMin1, float radiusMax1) { points = points1;  radiusMin = radiusMin1; radiusMax = radiusMax1; }

        void addPoint(WorldPosition* pos) { points.push_back(pos); }
        void setExpireDelay(uint32 delay) { expireDelay = delay; }
        void setCooldownDelay(uint32 delay) { cooldownDelay = delay; }
        void setMaxVisitors(int maxVisitors1 = 0, int maxVisitorsPerPoint1 = 0) { maxVisitors = maxVisitors1; maxVisitorsPerPoint = maxVisitorsPerPoint1; }

        vector<WorldPosition*> getPoints(int max = -1);
        uint32 getExpireDelay() { return expireDelay; }
        uint32 getCooldownDelay() { return cooldownDelay; }
        void addVisitor() { visitors++; }
        void remVisitor() { visitors--; }

        virtual bool isActive(Player* bot) { return false; }
        bool isFull(bool ignoreFull = false);

        virtual string getName() { return "TravelDestination"; }
        virtual uint32 getEntry() { return NULL; }

        WorldPosition* nearestPoint(WorldPosition* pos);
        float distanceTo(WorldPosition* pos) { return nearestPoint(pos)->distance(pos); }
        virtual bool isIn(WorldPosition* pos) { return distanceTo(pos) <= radiusMin; }
        virtual bool isOut(WorldPosition* pos) { return distanceTo(pos) > radiusMax; }

        vector<WorldPosition*> touchingPoints(WorldPosition* pos);
        vector<WorldPosition*> sortedPoints(WorldPosition* pos);
        vector<WorldPosition*> nextPoint(WorldPosition* pos, bool ignoreFull = true);
    protected:
        vector<WorldPosition*> points;
        float radiusMin = 0;
        float radiusMax = 0;

        int visitors = 0;
        int maxVisitors = 0;
        int maxVisitorsPerPoint = 0;
        uint32 expireDelay = 5 * 1000;
        uint32 cooldownDelay = 60 * 1000;
    };

    //A travel target that is always inactive and jumps to cooldown.
    class NullTravelDestination : public TravelDestination
    {
    public:
        NullTravelDestination(int coodownDelay1 = 5 * 60 * 1000) : TravelDestination() { cooldownDelay = coodownDelay1;};

        virtual bool isActive(Player* bot) { return false; }

        virtual string getName() { return "NullTravelDestination"; }

        virtual bool isIn(WorldPosition* pos) { return true; }
        virtual bool isOut(WorldPosition* pos) { return false; }

    protected:
    };


    //A travel target specifically related to a quest.
    class QuestTravelDestination : public TravelDestination
    {
    public:
        QuestTravelDestination(uint32 questId1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1) { questId = questId1; questTemplate = sObjectMgr.GetQuestTemplate(questId);
        }

        Quest const* GetQuestTemplate() { return questTemplate;  }

        virtual bool isActive(Player* bot) { return bot->IsActiveQuest(questId); }

        virtual string getName() { return "QuestTravelDestination"; }
        virtual uint32 getEntry() { return NULL; }
    protected:
        uint32 questId;
        Quest const* questTemplate;
    };

    //A quest giver or taker.
    class QuestRelationTravelDestination : public QuestTravelDestination
    {
    public:
        QuestRelationTravelDestination(uint32 quest_id1, uint32 entry1, uint32 relation1, float radiusMin1, float radiusMax1) :QuestTravelDestination(quest_id1, radiusMin1, radiusMax1) { entry = entry1; relation = relation1; }

        virtual bool isActive(Player* bot);

        virtual string getName() { return "QuestRelationTravelDestination"; }
        virtual uint32 getEntry() { return entry; }
        virtual uint32 getRelation() { return relation; }
    private:
        uint32 relation;
        int32 entry;
    };

    //A quest destination container for quick lookup of all destinations related to a quest.
    struct QuestContainer
    {
        vector<QuestTravelDestination *> questGivers;
        vector<QuestTravelDestination *> questTakers;
        vector<QuestTravelDestination *> questObjectives;
    };

    //A quest objective (creature/gameobject to grind/loot)
    class QuestObjectiveTravelDestination : public QuestTravelDestination
    {
    public:
        QuestObjectiveTravelDestination(uint32 quest_id1, uint32 entry1, int objective1, float radiusMin1, float radiusMax1) : QuestTravelDestination(quest_id1, radiusMin1, radiusMax1) {
            objective = objective1; entry = entry1;
        }

        bool isCreature() { return GetQuestTemplate()->ReqCreatureOrGOId[objective] > 0; }

        int32 ReqCreature() {
            return isCreature() ? GetQuestTemplate()->ReqCreatureOrGOId[objective] : 0;
        }
        int32 ReqGOId() {
            return !isCreature() ? abs(GetQuestTemplate()->ReqCreatureOrGOId[objective]) : 0;
        }
        uint32 ReqCount() { return GetQuestTemplate()->ReqCreatureOrGOCount[objective]; }

        virtual bool isActive(Player* bot);

        virtual string getName() { return "QuestObjectiveTravelDestination"; }

        virtual uint32 getEntry() { return entry; }
    private:
        int objective;
        int32 entry;
    };

    enum TravelStatus
    {
        TRAVEL_STATUS_NONE = 0,
        TRAVEL_STATUS_PREPARE = 1,
        TRAVEL_STATUS_TRAVEL = 2,
        TRAVEL_STATUS_WORK = 3,
        TRAVEL_STATUS_COOLDOWN = 4,  
        TRAVEL_STATUS_EXPIRED = 5,
        MAX_TRAVEL_STATUS
    };

    //Current target and location for the bot to travel to.
    //The flow is as follows:
    //PREPARE   (wait until no loot is near)
    //TRAVEL    (move towards target until close enough) (rpg and grind is disabled)
    //WORK      (grind/rpg until the target is no longer active) (rpg and grind is enabled on quest mobs)
    //COOLDOWN  (wait some time free to do what the bot wants)
    //EXPIRE    (if any of the above actions take too long pick a new target)
    class TravelTarget : AiObject
    {
    public:
        TravelTarget(PlayerbotAI* ai) : AiObject(ai) {};
        TravelTarget(PlayerbotAI* ai, TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1 = false) : AiObject(ai) { setTarget(tDestination1, wPosition1); groupCopy = groupCopy1; }
        
        void setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1);
        void setStatus(TravelStatus status);
        void setExpireIn(uint32 expireMs) { statusTime = getExpiredTime() + expireMs; }

        void copyTarget(TravelTarget* target);

        float distance(Player* bot) { WorldPosition pos(bot);  return wPosition->distance(&pos); };
        WorldLocation getLocation() { return wPosition->getLocation(); };
        WorldPosition* getPosition() { return wPosition; };
        TravelDestination* getDestination() { return tDestination; };
        PlayerbotAI* getAi() { return ai; }

        uint32 getExpiredTime() { return WorldTimer::getMSTime() - startTime; }
        uint32 getTimeLeft() { return statusTime - getExpiredTime(); }
        uint32 getMaxTravelTime() { return (1000.0 * distance(bot)) / bot->GetSpeed(MOVE_RUN); }

        bool isTraveling(); 
        bool isActive();   
        bool isWorking();
        bool isPreparing();

        bool isGroupCopy() { return groupCopy; };
    protected:
        TravelStatus m_status = TRAVEL_STATUS_NONE;

        uint32 startTime = WorldTimer::getMSTime();
        uint32 statusTime = 0;

        bool groupCopy = false;

        TravelDestination* tDestination = NULL;
        WorldPosition* wPosition = NULL;
    };

    //General container for all travel destinations.
    class TravelMgr
    {
    public:
        TravelMgr() {};
        void Clear();
        void LoadQuestTravelTable();

        vector <WorldPosition*> getNextPoint(WorldPosition* center, vector<WorldPosition*> points);
        QuestStatusData* getQuestStatus(Player* bot, uint32 questId);
        bool getObjectiveStatus(Player* bot, Quest const* pQuest, int objective);
        uint32 getDialogStatus(Player* pPlayer, uint32 questgiver, Quest const* pQuest);
        vector<QuestTravelDestination *> getQuestTravelDestinations(Player* bot, uint32 questId = -1, bool ignoreFull = false);

        NullTravelDestination* nullTravelDestination = new NullTravelDestination();
        WorldPosition* nullWorldPosition = new WorldPosition();
    protected:
        void logQuestError(uint32 errorNr, Quest * quest, uint32 objective = 0, uint32 unitId = 0, uint32 itemId = 0);

        //vector<pair<uint32, QuestTravelDestination *>> questTravelDestinations;
        vector<QuestTravelDestination*> questGivers;

        UNORDERED_MAP<uint32, QuestContainer *> quests;

        UNORDERED_MAP<int32, WorldPosition> pointsMap;
    };
}

#define sTravelMgr MaNGOS::Singleton<TravelMgr>::Instance()