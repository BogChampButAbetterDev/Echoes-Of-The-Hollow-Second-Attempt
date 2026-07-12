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

void EnemySpawner::update(float dt, const std::vector<SDL_Rect>& solids, SDL_Rect mapBounds, float px, float py, Player& player)
{
    for (auto& enemy : m_enemies)
        enemy->update(dt, solids, px, py, mapBounds, player);

    separateAll();

    // tick spawn cooldowns and attempt to spawn
    for (int i = 0; i < (int)m_points.size(); i++)
    {
        const EnemySpawnPoint& sp = m_points[i];
        SlotState&             st = m_state[i];

        if (st.disabled) continue;

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
        m_enemyRoomId.erase(m_enemyRoomId.begin() + ei);
    }
}

void EnemySpawner::separateAll()
{
    for (size_t i = 0; i < m_enemies.size(); i++)
    {
        if (m_enemies[i]->health <= 0) continue;

        for (size_t j = i + 1; j < m_enemies.size(); j++)
        {
            if (m_enemies[j]->health <= 0) continue;

            SDL_Rect a = m_enemies[i]->getHitbox();
            SDL_Rect b = m_enemies[j]->getHitbox();

            SDL_Rect overlap;
            if (!SDL_IntersectRect(&a, &b, &overlap)) continue;

            if (overlap.w < overlap.h)
            {
                float push = (overlap.w * 0.5f) / MAP_RENDER_SCALE;
                if (a.x < b.x) { m_enemies[i]->x -= push; m_enemies[j]->x += push; }
                else            { m_enemies[i]->x += push; m_enemies[j]->x -= push; }
            }
            else
            {
                float push = (overlap.h * 0.5f) / MAP_RENDER_SCALE;
                if (a.y < b.y) { m_enemies[i]->y -= push; m_enemies[j]->y += push; }
                else { m_enemies[i]->y += push; m_enemies[j]->y -= push; }
            }
        }
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

    SlotState& st = m_state[pointIdx];
    const EnemySpawnPoint& sp = m_points[pointIdx];

    if (st.disabled) return;

    auto enemy = m_factory(sp.enemyType, sp.x, sp.y);
    if (!enemy) return;

    st.liveIndices.push_back((int)m_enemies.size());
    st.spawnedTotal++;
    m_enemies.push_back(std::move(enemy));
    m_enemyRoomId.push_back(""); // slot-spawned enemies are tracked via m_state, not room tags

    // apply a stagger to not fire every point every frame
    st.cooldown = 0.1f * (pointIdx % 5);
}

bool EnemySpawner::triggerRoomBurst(const std::string& roomId, const std::vector<SDL_Rect>& solids)
{
    for (auto& burst : m_roomBursts)
    {
        if (burst.roomId != roomId) continue;
        if (burst.triggered) return false; // already fired, no-op

        burst.triggered = true;

        for (int i = 0; i < burst.count; i++)
            spawnBurstAt(burst.roomId, burst.enemyType, burst.centerX, burst.centerY, burst.radius, solids);

        return true;
    }

    return false; // no burst registered for this room
}

bool EnemySpawner::spawnBurstAt(const std::string& roomId, const std::string& type, float cx, float cy, float radius, const std::vector<SDL_Rect>& solids)
{
    if (!m_factory) return false;

    const int maxAttempts = 8;
    float chosenX = cx;
    float chosenY = cy;

    for (int attempt = 0; attempt < maxAttempts; attempt++)
    {
        // random point in a disc of `radius` around (cx, cy) - unscaled tile-space units
        float angle = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI;
        float dist  = std::sqrt((float)rand() / RAND_MAX) * radius; // sqrt for uniform area density
        float tryX  = cx + std::cos(angle) * dist;
        float tryY  = cy + std::sin(angle) * dist;

        SDL_Rect probe =
        {
            (int)(tryX * MAP_RENDER_SCALE),
            (int)(tryY * MAP_RENDER_SCALE),
            (int)(MAP_RENDER_SCALE),
            (int)(MAP_RENDER_SCALE)
        };

        bool blocked = false;
        for (const SDL_Rect& tile : solids)
        {
            if (SDL_HasIntersection(&probe, &tile)) { blocked = true; break; }
        }

        if (!blocked)
        {
            chosenX = tryX;
            chosenY = tryY;
            break;
        }
        // otherwise loop and try another random point; after maxAttempts we
        // fall back to the last attempted point (or the center if attempt 0 failed)
        chosenX = tryX;
        chosenY = tryY;
    }

    auto enemy = m_factory(type, chosenX, chosenY);
    if (!enemy) return false;

    m_enemies.push_back(std::move(enemy));
    m_enemyRoomId.push_back(roomId);
    return true;
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
