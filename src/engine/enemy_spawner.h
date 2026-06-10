#pragma once
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <memory>
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

    // queue all live enemies to render queue. call ts in render loop
    void queueForRender(Camera& cam);

    // disable spawn point. call ts in situations like after a boss fight
    void disable(const std::string& id);

    // enabled by default
    void enable(const std::string& id);

    // direct access to m_enemies
    const std::vector<std::unique_ptr<AI>>& getEnemies() const { return m_enemies; }

private:
    struct SlotState
    {
        std::vector<int> liveIndices; // indices to m_enemies
        float cooldown = 0.0f;
        bool disabled = false;
    };

    void spawnAt(int pointIdx);

    bool isUnique(const std::string& type);

    int countAlive(const std::string &type) const;

    const EnemyDef* defFor(const std::string& type) const;

    std::vector<EnemySpawnPoint>     m_points;
    SceneConfig                      m_config;
    std::vector<SlotState>           m_state;
    std::vector<std::unique_ptr<AI>> m_enemies;
    Factory                          m_factory;
};
