#include "enemy_spawner.h"

void EnemySpawner::init(const std::vector<EnemySpawnPoint> &points, const SceneConfig &config)
{
    m_points = points;
    m_config = config;

    m_state.clear();
    m_state.resize(m_points.size());

    for(auto& st : m_state) 
    {
        st.cooldown = 0.0f; 
        st.disabled = false;
    }
}

void EnemySpawner::update(float dt, const std::vector<SDL_Rect> &solids, SDL_Rect mapBounds, float px, float py, Player &player)
{
    for (auto& enemy : m_enemies)
        enemy->update(dt, solids, px, py, mapBounds, player);

    // tick spawn cooldowns and attempt to spawn
    for (int i = 0; i < (int)m_points.size(); i++)
    {
        const EnemySpawnPoint& sp = m_points[i];
        SlotState&             st = m_state[i];

        if (st.cooldown > 0.0f)
        {
            st.cooldown -= dt;
            continue;
        }

        // check if current slot is empty and spawn if it is
        if (!st.liveIndices.empty()) continue; 

        // check if this is unique and currently alive anywhere else
        if (isUnique(sp.enemyType) && countAlive(sp.enemyType) > 0) continue;

        spawnAt(i);
    }

    // remove dead enemies
    // check in reverse order
    for (int ei = (int)m_enemies.size() - 1; ei >= 0; ei--)
    {
        if (m_enemies[ei]->health > 0) continue;

        // tell the slot it lost an enemy
        for (auto& st : m_state)
        {
            auto it = std::find(st.liveIndices.begin(), st.liveIndices.end(), ei);
            if (it != st.liveIndices.end())
            {
                st.liveIndices.erase(it);

                // start respawn cooldown
                const EnemyDef* def = defFor(m_points[&st - m_state.data()].enemyType);
                st.cooldown = def ? def->respawnDelay : 5.0f;
                break;
            }
        }

        // patch indices in slots that pointed to the past ei
        for (auto& st : m_state)
            for (auto& idx : st.liveIndices)
                if (idx > ei) idx--;
        
        m_enemies.erase(m_enemies.begin() + ei);
    }
}

void EnemySpawner::queueForRender(Camera &cam)
{
    for (auto& enemy : m_enemies)
        enemy->queueForRender(cam);
}

void EnemySpawner::disable(const std::string &id)
{
    for (int i = 0; i < (int)m_points.size(); i++)
        if (m_points[i].id == id)
            m_state[i].disabled = true;
}

void EnemySpawner::enable(const std::string &id)
{
    for (int i = 0; i < (int)m_points.size(); i++)
        if (m_points[i].id == id)
            m_state[i].disabled = false;
}

void EnemySpawner::spawnAt(int pointIdx)
{
    if (!m_factory) return; 

    SlotState&              st = m_state[pointIdx];
    const EnemySpawnPoint&  sp = m_points[pointIdx];

    if (st.disabled) return;

    auto enemy = m_factory(sp.enemyType, sp.x * MAP_RENDER_SCALE, sp.y * MAP_RENDER_SCALE);
    if (!enemy) return;

    st.liveIndices.push_back((int)m_enemies.size());
    m_enemies.push_back(std::move(enemy));

    // apply a stagger to not fire every point every frame
    st.cooldown = 0.1f * (pointIdx & 5);
}

bool EnemySpawner::isUnique(const std::string &type)
{
    const EnemyDef* d = defFor(type);
    return d && d->unique;
}

int EnemySpawner::countAlive(const std::string &type) const
{
    int count = 0;
    for (int i = 0; i < (int)m_points.size(); i++)
    {
        if (m_points[i].enemyType == type)
            count += (int)m_state[i].liveIndices.size();
    }
    return count;
}

const EnemyDef* EnemySpawner::defFor(const std::string &type) const
{
    for (const auto& d : m_config.enemyDefs)
        if (d.type == type) return &d;
    return nullptr;
}
