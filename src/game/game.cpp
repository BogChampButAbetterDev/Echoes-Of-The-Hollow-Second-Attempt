#include "game.h"
#include "globals/globals.h"
#include "engine/input.h"

Game::Game() :
m_win(nullptr),
m_ren(),
m_player(),
running(true),
lastTime(0),
delta(0),
m_currentScene(nullptr),
m_cam(Camera(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0)),
m_font(nullptr),
m_dialogueTex(nullptr),
m_fps(0),
m_fpsTimer(0),
m_frameCount(0)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    Font::init();
    m_win = createWin();
    m_ren = Render(m_win);
    lastTime = SDL_GetPerformanceCounter();

    init();
}

void Game::checkInteraction()
{
    float px = m_player.getX();
    float py = m_player.getY();

    const float radius = 1.5f;

    Interactable* closest = nullptr;
    float closestDist = radius * m_currentScene->map.m_tileWidth;

    for (auto& interactable : m_currentScene->map.getInteractables())
    {
        if (interactable->activateOnContact) continue; // skip contact-only interactables

        float ix = interactable->tileX * m_currentScene->map.m_tileWidth;
        float iy = interactable->tileY * m_currentScene->map.m_tileHeight;

        float dx = px - ix;
        float dy = py - iy;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < closestDist)
        {
            closestDist = dist;
            closest = interactable.get();
        }
    }

    if (!closest) return;

    if (!closest->interact())
        m_ui.openDialogue(m_ren.renderer, closest->data);
}

void Game::checkContact()
{
    float px = m_player.getX();
    float py = m_player.getY();
    const float radius = 0.3f * m_currentScene->map.m_tileWidth;

    for (auto& interactable : m_currentScene->map.getInteractables())
    {
        if (!interactable->activateOnContact) continue;

        float ix = interactable->tileX * m_currentScene->map.m_tileWidth;
        float iy = interactable->tileY * m_currentScene->map.m_tileHeight;

        float dx = px - ix;
        float dy = py - iy;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < radius)
        {
            bool wasPressedBefore = false;
            if (interactable->type == InteractType::BUTTON)
                wasPressedBefore = static_cast<Button*>(interactable.get())->pressed;

            interactable->interact();

            if (interactable->type == InteractType::BUTTON && !wasPressedBefore)
            {
                Button* button = static_cast<Button*>(interactable.get());
                for (auto& other : m_currentScene->map.getInteractables())
                {
                    if (other->name == button->targetName)
                    {
                        float worldX = other->tileX * m_currentScene->map.m_tileWidth  * MAP_RENDER_SCALE;
                        float worldY = other->tileY * m_currentScene->map.m_tileHeight * MAP_RENDER_SCALE;
                        m_cam.panTo(worldX, worldY);
                        break;
                    }
                }
            }
        }
    }
}

void Game::checkDoorTransitions()
{
    float px = m_player.getX();
    float py = m_player.getY();
    const float radius = 0.5f * m_currentScene->map.m_tileWidth;

    for (auto& interactable : m_currentScene->map.getInteractables())
    {
        if (interactable->type != InteractType::DOOR) continue;

        Door* door = static_cast<Door*>(interactable.get());

        if (!door->isTransitionDoor()) continue;

        // if door has open/close state, only transition when open
        if (door->closedGid > 0 && !door->open) continue;

        float ix = door->tileX * m_currentScene->map.m_tileWidth;
        float iy = door->tileY * m_currentScene->map.m_tileHeight;

        float dx = px - ix;
        float dy = py - iy;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < radius)
        {
            startTransition(door->toMap, door->toSpawn);
            return;
        }
    }
}

void Game::startTransition(const std::string &mapId, const std::string &spawnName)
{
    if (m_fadestat != FadeState::NONE) return;
    m_pendingMap   = mapId;
    m_pendingSpawn = spawnName;
    m_fadestat     = FadeState::FADING_OUT;
    m_fadeAlpha    = 0;
}

void Game::updateFade()
{
    if (m_fadestat == FadeState::NONE) return;

    if (m_fadestat == FadeState::FADING_OUT)
    {
        fadeDirection = 1;
        m_fadeAlpha += fadeDirection * fadeSpeed * delta;
        if (m_fadeAlpha >= 255)
        {
            m_fadeAlpha = 255;
            //loadScene(m_pendingMap, m_pendingSpawn);
            shouldLoadScene = true;
            m_fadestat = FadeState::FADING_IN;
        }
    }
    else if (m_fadestat == FadeState::FADING_IN)
    {
        fadeDirection = -1;
        m_fadeAlpha += fadeDirection * fadeSpeed * delta;
        if (m_fadeAlpha <= 0)
        {
            m_fadeAlpha = 0;
            m_fadestat = FadeState::NONE;
        }
    }

    SDL_SetRenderDrawBlendMode(m_ren.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_ren.renderer, 0,0,0, m_fadeAlpha);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(m_ren.renderer, &overlay);
}

void Game::loadScene(const std::string& mapId, const std::string& spawnName)
{
    // save current scene state
    if (m_currentScene)
    {
        for (auto& interactable : m_currentScene->map.getInteractables())
        {
            if (interactable->name.empty()) continue;

            if (interactable->type == InteractType::CHEST)
            {
                Chest* chest = static_cast<Chest*>(interactable.get());
                if (chest->opened)
                    m_currentScene->triggeredInteractables.insert(interactable->name);
            }
            if (interactable->type == InteractType::BUTTON)
            {
                Button* button = static_cast<Button*>(interactable.get());
                if (button->pressed)
                    m_currentScene->triggeredInteractables.insert(interactable->name);
            }
        }
    }

    // load scene if not visited before
    if (m_scenes.find(mapId) == m_scenes.end())
        m_scenes.emplace(mapId, Scene(mapId, m_ren.renderer));

    m_currentScene = &m_scenes[mapId];

    // restore triggered state
    for (auto& interactable : m_currentScene->map.getInteractables())
    {
        if (interactable->name.empty()) continue;
        if (m_currentScene->triggeredInteractables.count(interactable->name) == 0) continue;

        if (interactable->type == InteractType::CHEST)
            static_cast<Chest*>(interactable.get())->opened = true;
        if (interactable->type == InteractType::BUTTON)
            static_cast<Button*>(interactable.get())->pressed = true;
    }

    // place player at spawn point
    const auto& spawns = m_currentScene->map.getSpawnPoints();
    auto it = spawns.find(spawnName);
    if (it != spawns.end())
        m_player.setPosition(it->second.x, it->second.y);
    else
    {
        std::cout << "Spawn point not found: " << spawnName << ", using 0,0\n";
        m_player.setPosition(0, 0);
    }

    // update camera for new map bounds
    m_cam = Camera(SCREEN_WIDTH, SCREEN_HEIGHT,
        m_currentScene->map.m_mapWidth  * m_currentScene->map.m_tileWidth  * MAP_RENDER_SCALE,
        m_currentScene->map.m_mapHeight * m_currentScene->map.m_tileHeight * MAP_RENDER_SCALE);
}

void Game::mainLoop()
{
    bool prevInteract = false;

    while (running)
    {
        Uint64 currentTime = SDL_GetPerformanceCounter();
        delta = (double)((currentTime - lastTime) / (double)SDL_GetPerformanceFrequency());
        lastTime = currentTime;

        m_frameCount++;
        m_fpsTimer += delta;

        if (m_fpsTimer >= 1.0f)
        {
            m_fps = m_frameCount;
            m_frameCount = 0;
            m_fpsTimer = 0.0f;
            std::string title = "The Paper Keep - FPS: " + std::to_string(m_fps);
            SDL_SetWindowTitle(m_win, title.c_str());
        }

        pollInput(m_player.input, running);

        if (!running) break;

        bool interactPressed = m_player.input.interact && !prevInteract;
        prevInteract = m_player.input.interact;

        if (interactPressed)
        {
            if (m_ui.isDialogueOpen())
                m_ui.advance(m_ren.renderer);
            else
                checkInteraction();
        }

        const auto& solids = m_currentScene->map.getSolidRects();

        m_cam.update(delta,
            m_player.getX() * MAP_RENDER_SCALE,
            m_player.getY() * MAP_RENDER_SCALE);

        if (!m_ui.isDialogueOpen() && !m_cam.inCutscene && m_fadestat == FadeState::NONE)
        {
            m_player.update(delta, m_cam, solids);
            checkContact();
            checkDoorTransitions();
        }

        m_ren.beginFrame();
        m_currentScene->map.renderGround(m_ren.renderer, m_cam);
        m_currentScene->map.renderObjects(m_ren.renderer, m_cam);
        m_player.queueForRender(m_cam);
        m_ren.renderFromQueue();
        m_currentScene->map.renderOverhead(m_ren.renderer, m_cam);
        m_ui.render(m_ren.renderer);
        updateFade();
        if (shouldLoadScene)
        {
            loadScene(m_pendingMap, m_pendingSpawn);
            shouldLoadScene = false;
        }

        #if USING_CONTROLLER
            if (m_player.input.noControllerWarning)
            {
                SDL_Rect shadow = {SCREEN_WIDTH/2 - 99, SCREEN_HEIGHT - 50, 198, 35};
                SDL_SetRenderDrawBlendMode(m_ren.renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(m_ren.renderer, 0, 0, 0, 160);
                SDL_RenderFillRect(m_ren.renderer, &shadow);
                
                // warning text
                std::string text = "NO CONTROLLER";
                SDL_Color color = {255, 0, 0, 255};

                SDL_Texture* tex = m_font->renderText(m_ren.renderer, text, color, 300);

                int w, h;
                SDL_QueryTexture(tex, NULL, NULL, &w, &h);

                SDL_Rect dst =
                {
                    SCREEN_WIDTH / 2 - w / 2,
                    SCREEN_HEIGHT - 50,
                    w,
                    h
                };

                SDL_RenderCopy(m_ren.renderer, tex, NULL, &dst);
                SDL_DestroyTexture(tex);
            }
        #endif

        m_ren.presentFrame();
    }
}

void Game::end()
{
    Texture::clearCache();
    if (m_dialogueTex) SDL_DestroyTexture(m_dialogueTex);
    Font::quit();
    SDL_DestroyRenderer(m_ren.renderer);
    SDL_DestroyWindow(m_win);
    SDL_Quit();
}

SDL_Window* Game::createWin()
{
    SDL_Window* win = SDL_CreateWindow(
        "you wont see this",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );

    if (!win)
    {
        std::cout << "Window creation failure\n";
        exit(1);
    }

    return win;
}

void Game::init()
{
    m_player = Player(m_ren.renderer, {0, 0});
    m_font = new Font(FONT_PATH("font.ttf"), TEXT_SIZE_DEFAULT);
    m_ui = UI(m_font);

    loadScene("testworld.tmx", "default");
}
