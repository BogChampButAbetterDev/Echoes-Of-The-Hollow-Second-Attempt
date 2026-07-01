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
m_sm(nullptr),
m_cam(Camera(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0)),
m_font(nullptr),
m_cursor(nullptr),
m_dialogueTex(nullptr),
m_fps(0),
m_fpsTimer(0),
m_frameCount(0)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    Font::init();
    m_win = createWin();
    m_ren = Render(m_win);
    m_sm = SceneManager(m_ren.renderer);
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
    {
        std::string text;

        if (closest->type == InteractType::NPC)
        {
            NPC* npc = static_cast<NPC*>(closest);
            text = npc->getText(m_story);
            
            if (npc->isImportant)
            {
                if (!npc->name.empty())
                {
                    m_story.set("met_" + npc->name);
                }
            }
        }
        else
            m_ui->openDialogue(m_ren.renderer, closest->data);
    }
}

void Game::checkContact()
{
    for (auto& interactable : m_currentScene->map.getInteractables())
    {
        if (!interactable->activateOnContact) continue;

        SDL_Rect trigger =
        {
            interactable->x * MAP_RENDER_SCALE,
            interactable->y * MAP_RENDER_SCALE,
            interactable->w * MAP_RENDER_SCALE,
            interactable->h * MAP_RENDER_SCALE
        };

        SDL_Rect player = m_player.getFootRect();

        if (SDL_HasIntersection(&player, &trigger))
        {
            bool wasPressedBefore = false;
            if (interactable->type == InteractType::BUTTON)
                wasPressedBefore = static_cast<Button*>(interactable.get())->pressed;

            if (interactable->type == InteractType::BUTTON && !wasPressedBefore)
            {
                interactable->interact();
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
            if (interactable->type == InteractType::ITEM)
            {
                WorldItem* item = static_cast<WorldItem*>(interactable.get());
                if (item->collected) return;

                interactable->interact();

                m_inventory.add(item->id);
                m_story.set("has_" + item->id);

                m_ui->openDialogue(m_ren.renderer, "You got the " + item->id);
            }
        }
    }
}

void Game::checkDoorTransitions()
{
    for (auto& interactable : m_currentScene->map.getInteractables())
    {
        if (interactable->type != InteractType::LOADZONE) continue;

        Door* door = static_cast<Door*>(interactable.get());

        if (!door->isTransitionDoor()) continue;

        // if door has open/close state, only transition when open
        if (door->closedGid > 0 && !door->open) continue;

        SDL_Rect zone =
        {
            door->x * MAP_RENDER_SCALE,
            door->y * MAP_RENDER_SCALE,
            door->w * MAP_RENDER_SCALE,
            door->h * MAP_RENDER_SCALE
        };

        SDL_Rect player = m_player.getFootRect();

        if (SDL_HasIntersection(&player, &zone))
        {
            m_sm.requestScene(door->toMap, door->toSpawn);
            return;
        }
    }
}

void Game::loadScene(const std::string& mapId, const std::string& spawnName)
{
    std::cout << "loading scene\n";
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
    {
        const SceneConfig& cfg = m_allScenes.getConfig(mapId);
        m_scenes.emplace(mapId, Scene(mapId, m_ren.renderer, cfg));
         

        // wire the factory so the spawner knows how to make enemies.
        // add new else if beranches for every new enemy
         
        m_scenes[mapId].spawner.setFactory(
        [this](const std::string& type, float x, float y) -> std::unique_ptr<AI>
        {
             
            if (type == "bee")
            {
                auto b = std::make_unique<Bee>(m_ren.renderer, v2{x, y});
                 
                b->init(m_ren.renderer);
                 
                return b;
            }
             
            return nullptr;
        });
    }

     
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
    {
         
        m_player.setPosition(it->second.x, it->second.y);
         
    }
    else
    {
        m_player.setPosition(0, 0);
    }

    // update camera for new map bounds
    m_cam = Camera(SCREEN_WIDTH, SCREEN_HEIGHT,
        m_currentScene->map.m_mapWidth  * m_currentScene->map.m_tileWidth  * MAP_RENDER_SCALE,
        m_currentScene->map.m_mapHeight * m_currentScene->map.m_tileHeight * MAP_RENDER_SCALE);

    m_player.input.controller = m_cursor->input.controller;

    if (!m_allScenes.getConfig(m_currentScene->id).areaName.empty())
    {
        std::cout << "show label\n";
        m_ui->showAreaLabel(m_ren.renderer, m_allScenes.getConfig(m_currentScene->id).areaName);
    }
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

        pollInput(m_ui->hasOpenScreen() ? m_cursor->input : m_player.input, running);

        if (m_player.input.start && !m_ui->hasOpenScreen())
        {
            m_ui->openMenu(Menus::pauseMenu(m_ren.renderer, this));
        }

        if (!running) break;

        if (!m_ui->hasOpenScreen())
        {
            bool interactPressed = m_player.input.interact && !prevInteract;
            prevInteract = m_player.input.interact;

            if (interactPressed)
            {
                if (m_ui->isDialogueOpen())
                    m_ui->advance(m_ren.renderer);
                else
                    checkInteraction();
            }

            const auto& solids = m_currentScene->map.getSolidRects();
            SDL_Rect mapBounds = 
            {
                0, 0,
                m_currentScene->map.m_mapWidth  * m_currentScene->map.m_tileWidth  * MAP_RENDER_SCALE,
                m_currentScene->map.m_mapHeight * m_currentScene->map.m_tileHeight * MAP_RENDER_SCALE
            };

            m_cam.update(delta,
                m_player.getX() * MAP_RENDER_SCALE,
                m_player.getY() * MAP_RENDER_SCALE);

            if (!m_ui->isDialogueOpen() && !m_cam.inCutscene && !m_sm.isTransitioning())
            {
                m_player.update(delta, m_cam, solids, m_story);
                checkContact();
                checkDoorTransitions();
            }

            m_currentScene->spawner.update(delta, solids, mapBounds,
                                    m_player.getX(), m_player.getY(), m_player);

            m_sm.update(delta, *this);
        }
        else
        {
            m_cursor->update(delta);
            m_ui->handleInput(*m_cursor);
        }

        m_ui->update(delta);

        m_ren.beginFrame();
        if (!m_ui->hasOpenScreen())
        {   
            m_currentScene->map.renderGround(m_ren.renderer, m_cam);
            m_currentScene->map.renderObjects(m_ren.renderer, m_cam);
            m_currentScene->map.renderItems(m_cam);
            m_player.queueForRender(m_cam);
            m_currentScene->spawner.queueForRender(m_cam);
        }

        m_ren.renderFromQueue();
        if (!m_ui->hasOpenScreen()) m_currentScene->map.renderOverhead(m_ren.renderer, m_cam);
        m_sm.render(m_ren.renderer); // render fade
        m_ui->render(m_ren.renderer);
        if (m_ui->hasOpenScreen()) m_cursor->render(m_ren.renderer);

        bool showNoControllerWarning = m_ui->hasOpenScreen() ? m_cursor->input.noControllerWarning : m_player.input.noControllerWarning;

        #if USING_CONTROLLER
            if (showNoControllerWarning)
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

void Game::quitToMenu()
{
    m_currentScene = nullptr;
    m_scenes.clear();
    m_ui->openMenu(Menus::mainMenuDEBUG(m_ren.renderer, this));
}

SDL_Window *Game::createWin()
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
    m_font = new Font(FONT_PATH("font.ttf"), TEXT_SIZE_DEFAULT);
    m_ui = new UI(m_font);
    m_cursor = new Cursor(m_ren.renderer);

    m_player = Player(m_ren.renderer, {0, 0});

    m_ui->openMenu(Menus::mainMenuDEBUG(m_ren.renderer, this));
}

void Game::end()
{
    Texture::clearCache();
    if (m_dialogueTex) SDL_DestroyTexture(m_dialogueTex);
    delete m_cursor;
    m_cursor = nullptr;
    delete m_ui;
    m_ui = nullptr;
    delete m_font;
    m_font = nullptr;
    Font::quit();
    SDL_DestroyRenderer(m_ren.renderer);
    SDL_DestroyWindow(m_win);
    SDL_Quit();
}
