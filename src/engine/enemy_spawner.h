#pragma once
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <memory>
#include <cmath>
#include <cstdlib>
#include "globals/Actor.h"  // EnemySpawnPoint, EnemyDef
#include "globals/globals.h"
#include "AI_base.h"

class EnemySpawner
{
public:
    // with a type and world position, return a new heap
    using Factory = std::function<std::unique_ptr<AI>(const std::string& type, float x, float y)>;

    // call ts after loading tilemap
    void init(const std::vector<EnemySpawnPoint>& points, const SceneConfig& config);

    void setFactory(Factory factory) { m_factory = std::move(factory); }

    // call ts in update loop
    void update(float dt, const std::vector<SDL_Rect>& solids, SDL_Rect mapBounds, float px, float py, Player& player);

    void separateAll(const std::vector<SDL_Rect>& solids);

    // queue all live enemies to render queue. call ts in render loop
    void queueForRender(Camera& cam);

    // disable spawn point. call ts in situations like after a boss fight
    void disable(const std::string& id);

    // enabled by default
    void enable(const std::string& id);

    const std::vector<std::unique_ptr<AI>>& getEnemies() const { return m_enemies; }

    // Register a one-shot ambush spawn for a room: `count` enemies of `enemyType`
    // scattered randomly within `radius` (tile-space units, unscaled) of (centerX, centerY).
    // Call once, typically right after Scene construction (e.g. from onEnter),
    // then fire it later with triggerRoomBurst(). Safe to call multiple times
    // to register different rooms/types.
    void registerRoomBurst(const std::string& roomId, const std::string& enemyType, float centerX, float centerY, float radius, int count)
    {
        for (auto& b : m_roomBursts)
            if (b.roomId == roomId) return; // block multiple bursts from registering if re-entering a room
        m_roomBursts.push_back({roomId, enemyType, centerX, centerY, radius, count, false});
    }

    // Spawns every enemy for this room's burst at once, avoiding solids.
    // No-ops if this room's burst was already triggered or was never registered.
    // Returns true if it actually spawned anything.
    bool triggerRoomBurst(const std::string& roomId, const std::vector<SDL_Rect>& solids);

    // True once a given room's burst has fired (so scripts can gate on it,
    // e.g. only trigger once when the player walks in).
    bool roomBurstTriggered(const std::string& roomId) const
    {
        for (const auto& b : m_roomBursts)
            if (b.roomId == roomId) return b.triggered;
        return false;
    }

    // I don't feel like putting this in another file calm down buddy
    bool isRoomCleared(const std::string& roomId)
    {
        if (m_clearedRooms.count(roomId)) return true;

        bool anyFound = false;

        // slot-based (per-point, respawning) enemies
        for (int i = 0; i < (int)m_points.size(); i++)
        {
            if (m_points[i].roomId != roomId) continue;
            anyFound = true;

            if (!m_state[i].liveIndices.empty())   return false;
            if (m_state[i].spawnedTotal < m_points[i].maxCount) return false;
        }

        // burst (one-shot ambush) enemies for this room
        for (const auto& burst : m_roomBursts)
        {
            if (burst.roomId != roomId) continue;
            anyFound = true;

            if (!burst.triggered) return false; // hasn't happened yet - not "cleared", just pending
        }

        // any burst-spawned enemies from this room still alive?
        for (const auto& tag : m_enemyRoomId)
            if (tag == roomId) return false;

        if (anyFound)
            m_clearedRooms.insert(roomId);

        return anyFound;
    }

private:
    struct SlotState
    {
        std::vector<int> liveIndices; // indices to m_enemies
        float cooldown = 0.0f;
        bool disabled = false;
        int spawnedTotal = 0;
    };

    void tryPush(AI& enemy, float dx, float dy, const std::vector<SDL_Rect>& solids);

    void spawnAt(int pointIdx);

    bool isUnique(const std::string& type);

    int countAlive(const std::string &type) const;

    const EnemyDef* defFor(const std::string& type) const;

    // spawns one enemy of `type` at a random point within `radius` of (cx, cy),
    // retrying a few times to avoid solids. Falls back to (cx, cy) if no clear
    // spot is found. Tags the enemy with `roomId` so isRoomCleared() can see it.
    // Returns false only if the factory itself refuses (unknown type).
    bool spawnBurstAt(const std::string& roomId, const std::string& type, float cx, float cy, float radius, const std::vector<SDL_Rect>& solids);

    struct RoomBurst
    {
        std::string roomId;
        std::string enemyType;
        float centerX, centerY;
        float radius;
        int count;
        bool triggered;
    };

    std::vector<EnemySpawnPoint>     m_points;
    SceneConfig                      m_config;
    std::vector<SlotState>           m_state;
    std::vector<std::unique_ptr<AI>> m_enemies;
    std::vector<std::string>         m_enemyRoomId; // parallel to m_enemies; "" for slot-spawned enemies
    Factory                          m_factory;
    std::set<std::string> m_clearedRooms;
    std::vector<RoomBurst> m_roomBursts;
};
