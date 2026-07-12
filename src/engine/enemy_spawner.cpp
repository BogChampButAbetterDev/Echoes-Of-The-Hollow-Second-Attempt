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

    separateAll(solids );

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
        if (!m_enemies[ei]->canDeleteEntity()) continue;

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

void EnemySpawner::separateAll(const std::vector<SDL_Rect>& solids)
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

            float pushX_i = 0, pushY_i = 0, pushX_j = 0, pushY_j = 0;

            if (overlap.w < overlap.h)
            {
                float push = (overlap.w * 0.5f) / MAP_RENDER_SCALE;
                if (a.x < b.x) { pushX_i = -push; pushX_j = push; }
                else            { pushX_i = push;  pushX_j = -push; }
            }
            else
            {
                float push = (overlap.h * 0.5f) / MAP_RENDER_SCALE;
                if (a.y < b.y) { pushY_i = -push; pushY_j = push; }
                else            { pushY_i = push;  pushY_j = -push; }
            }

            tryPush(*m_enemies[i], pushX_i, pushY_i, solids );
            tryPush(*m_enemies[j], pushX_j, pushY_j, solids);
        }
    }
}

void EnemySpawner::tryPush(AI& enemy, float dx, float dy, const std::vector<SDL_Rect>& solids)
{
    float oldX = enemy.x, oldY = enemy.y;
    enemy.x += dx;
    enemy.y += dy;

    SDL_Rect r = enemy.getHitbox();
    for (const SDL_Rect& tile : solids)
    {
        if (SDL_HasIntersection(&r, &tile))
        {
            enemy.x = oldX;
            enemy.y = oldY;
            return; // don't shove this enemy into a wall; leave the overlap for this frame
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
    std::unique_ptr<AI> enemy;

    for (int attempt = 0; attempt < maxAttempts; attempt++)
    {
        float angle = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI;
        float dist  = std::sqrt((float)rand() / RAND_MAX) * radius;
        float tryX  = cx + std::cos(angle) * dist;
        float tryY  = cy + std::sin(angle) * dist;

        auto candidate = m_factory(type, tryX, tryY);
        if (!candidate) return false; // factory refused the type entirely, no point retrying

        SDL_Rect hb = candidate->getHitbox();
        bool blocked = false;
        for (const SDL_Rect& tile : solids)
        {
            if (SDL_HasIntersection(&hb, &tile)) { blocked = true; break; }
        }

        if (!blocked)
        {
            enemy = std::move(candidate);
            chosenX = tryX;
            chosenY = tryY;
            break;
        }

        // last attempt: keep it anyway rather than failing outright, but log it
        if (attempt == maxAttempts - 1)
        {
            std::cout << "[EnemySpawner] burst spawn couldn't find a clear spot for '" << type << "' near (" << cx << "," << cy << "), spawning anyway\n";
            enemy = std::move(candidate);
            chosenX = tryX;
            chosenY = tryY;
        }
    }

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
