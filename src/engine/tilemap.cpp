#include "tilemap.h"

TileMap::TileMap(const char* path, SDL_Renderer* ren)
    : m_mapWidth(0), m_mapHeight(0),
      m_tileWidth(0), m_tileHeight(0),
      m_firstGid(1), m_columns(0), m_tileCount(0)
{
    loadTMX(path, ren);
}

void TileMap::loadTMX(const char* path, SDL_Renderer* ren)
{
    XMLDocument doc;
    if (doc.LoadFile(path) != XML_SUCCESS)
    {
        std::cout << "Failed to load TMX: " << path << "\n";
        exit(1);
    }

    XMLElement* map = doc.FirstChildElement("map");

    m_mapWidth   = map->IntAttribute("width");
    m_mapHeight  = map->IntAttribute("height");
    m_tileWidth  = map->IntAttribute("tilewidth");
    m_tileHeight = map->IntAttribute("tileheight");

    // parse all tilesets
    XMLElement* tileset = map->FirstChildElement("tileset");
    while (tileset)
    {
        TilesetInfo info;
        info.firstGid  = tileset->IntAttribute("firstgid");
        info.columns   = tileset->IntAttribute("columns");
        info.tileCount = tileset->IntAttribute("tilecount");

        const char* tsName = tileset->Attribute("name");
        info.name = tsName ? tsName : "";

        XMLElement* image = tileset->FirstChildElement("image");
        const char* tilesetPath = image->Attribute("source");
        Texture texture;
        info.tex = texture.loadTex(ren, tilesetPath);

        m_tilesets.push_back(info);
        tileset = tileset->NextSiblingElement("tileset");
    }

    // parse tile layers
    XMLElement* layer = map->FirstChildElement("layer");
    while (layer)
    {
        TileLayer tl;

        const char* name = layer->Attribute("name");
        tl.name = name ? name : "";

        XMLElement* data = layer->FirstChildElement("data");
        if (data)
        {
            const char* raw = data->GetText();
            if (!raw)
            {
                std::cout << "Layer data empty!\n";
                layer = layer->NextSiblingElement("layer");
                continue;
            }

            std::string csv(raw);
            std::stringstream ss(csv);
            std::string token;

            while (std::getline(ss, token, ','))
            {
                size_t start = token.find_first_not_of(" \n\r\t");
                if (start == std::string::npos) continue;

                size_t end = token.find_last_not_of(" \n\r\t");
                token = token.substr(start, end - start + 1);

                try
                {
                    uint32_t gid = std::stoul(token);
                    tl.tiles.push_back(gid);
                }
                catch (const std::exception& e)
                {
                    std::cout << "Invalid tile token: [" << token << "]\n";
                    throw;
                }
            }
        }

        m_layers.push_back(tl);
        layer = layer->NextSiblingElement("layer");
    }

    // parse object groups
    XMLElement* objGroup = map->FirstChildElement("objectgroup");
    while (objGroup)
    {
        const char* name = objGroup->Attribute("name");

        // collision
        if (name && strcmp(name, "col") == 0)
        {
            XMLElement* obj = objGroup->FirstChildElement("object");
            while (obj)
            {
                SDL_Rect r;
                r.x = (int)(obj->FloatAttribute("x")      * MAP_RENDER_SCALE);
                r.y = (int)(obj->FloatAttribute("y")      * MAP_RENDER_SCALE);
                r.w = (int)(obj->FloatAttribute("width")  * MAP_RENDER_SCALE);
                r.h = (int)(obj->FloatAttribute("height") * MAP_RENDER_SCALE);
                m_solidRects.push_back(r);
                obj = obj->NextSiblingElement("object");
            }
        }

        if (name && strcmp(name, "spawns") == 0)
        {
            XMLElement* obj = objGroup->FirstChildElement("object");
            while (obj)
            {
                const char* spawnName = obj->Attribute("name");
                const char* spawnType = obj->Attribute("type"); // Tiled "class" field
 
                if (spawnName)
                {
                    // no type or player type = player spawn point
                    if (!spawnType || strcmp(spawnType, "player") == 0 || strcmp(spawnType, "") == 0)
                    {
                        SpawnPoint sp;
                        sp.name = spawnName;
                        sp.x    = obj->FloatAttribute("x");
                        sp.y    = obj->FloatAttribute("y");
                        m_spawnPoints[sp.name] = sp;
                        std::cout << "Spawn point: " << sp.name
                                  << " at " << sp.x << ", " << sp.y << "\n";
                    }
                    // enemy spawn points
                    else if (strcmp(spawnType, "enemy") == 0)
                    {
                        EnemySpawnPoint esp;
                        esp.id = spawnName;
                        esp.x  = obj->FloatAttribute("x");
                        esp.y  = obj->FloatAttribute("y");
 
                        // Read custom properties: enemyType, maxCount
                        XMLElement* props = obj->FirstChildElement("properties");
                        if (props)
                        {
                            XMLElement* prop = props->FirstChildElement("property");
                            while (prop)
                            {
                                const char* propName = prop->Attribute("name");
                                const char* propVal  = prop->Attribute("value");
                                if (propName && propVal)
                                {
                                    if (strcmp(propName, "enemyType") == 0)
                                        esp.enemyType = propVal;
                                    if (strcmp(propName, "maxCount") == 0)
                                        esp.maxCount = std::stoi(propVal);
                                }
                                prop = prop->NextSiblingElement("property");
                            }
                        }
 
                        if (!esp.enemyType.empty())
                        {
                            m_enemySpawnPoints.push_back(esp);
                            std::cout << "Enemy spawn: " << esp.id
                                      << " type=" << esp.enemyType
                                      << " max=" << esp.maxCount
                                      << " at " << esp.x << ", " << esp.y << "\n";
                        }
                        else
                        {
                            std::cout << "Warning: enemy spawn '" << esp.id
                                      << "' has no enemyType property - skipped\n";
                        }
                    }
                }
                obj = obj->NextSiblingElement("object");
            }
        }

        // interactables
        if (name && strcmp(name, "interactables") == 0)
        {
            XMLElement* obj = objGroup->FirstChildElement("object");
            while (obj)
            {
                int tx = (int)(obj->FloatAttribute("x") / m_tileWidth);
                int ty = (int)(obj->FloatAttribute("y") / m_tileHeight);

                const char* objName = obj->Attribute("name");

                std::string text       = "";
                std::string objType    = "";
                std::string targetName = "";
                std::string layerName  = "objects";
                std::string toMap      = "";
                std::string toSpawn    = "";
                bool toggleable        = false;

                XMLElement* props = obj->FirstChildElement("properties");
                if (props)
                {
                    XMLElement* prop = props->FirstChildElement("property");
                    while (prop)
                    {
                        const char* propName = prop->Attribute("name");
                        const char* propVal  = prop->Attribute("value");

                        if (propName && propVal)
                        {
                            if (strcmp(propName, "text")       == 0) text       = propVal;
                            if (strcmp(propName, "type")       == 0) objType    = propVal;
                            if (strcmp(propName, "target")     == 0) targetName = propVal;
                            if (strcmp(propName, "toggleable") == 0) toggleable = strcmp(propVal, "true") == 0;
                            if (strcmp(propName, "layer")      == 0) layerName  = propVal;
                            if (strcmp(propName, "to")         == 0) toMap      = propVal; 
                            if (strcmp(propName, "spawn")      == 0) toSpawn    = propVal;  
                        }
                        prop = prop->NextSiblingElement("property");
                    }
                }

                if (objType == "sign" && !text.empty())
                {
                    auto sign = std::make_unique<Sign>(tx, ty, text);
                    m_interactables.push_back(std::move(sign));
                }
                else if (objType == "chest")
                {
                    int openedGid = getFirstGid("chest-opened");

                    auto chest = std::make_unique<Chest>(tx, ty, text, openedGid);
                    chest->name = objName ? objName : "";

                    std::vector<uint32_t>* objectTiles = nullptr;
                    for (auto& l : m_layers)
                        if (l.name == "objects")
                            objectTiles = &l.tiles;

                    int ctX  = tx;
                    int ctY  = ty;
                    int mapW = m_mapWidth;

                    chest->onInteract = [objectTiles, ctX, ctY, mapW, openedGid]()
                    {
                        if (!objectTiles) return;
                        int index = ctY * mapW + ctX;
                        if (index >= 0 && index < (int)objectTiles->size())
                            (*objectTiles)[index] = openedGid;
                    };

                    m_interactables.push_back(std::move(chest));
                }
                else if (objType == "door" || objType == "closed_door")
                {
                    int closedGid = objType == "closed_door" ? pixelToGid("world", 32, 112) : 0;
                    int openedGid = objType == "closed_door" ? pixelToGid("world", 48, 112) : 0;

                    auto door = std::make_unique<Door>(tx, ty, openedGid, closedGid);
                    door->name    = objName ? objName : "";
                    door->toMap   = toMap;
                    door->toSpawn = toSpawn;

                    m_interactables.push_back(std::move(door));
                }
                else if (objType == "button")
                {
                    auto button = std::make_unique<Button>(tx, ty);
                    button->name       = objName ? objName : "";
                    button->targetName = targetName;
                    button->toggleable = toggleable;
                    button->upGid      = getFirstGid("button-up");
                    button->downGid    = getFirstGid("button-down");
                    button->layerName  = layerName;
                    m_interactables.push_back(std::move(button));
                }

                obj = obj->NextSiblingElement("object");
            }

            // wire buttons to doors
            for (auto& interactable : m_interactables)
            {
                if (interactable->type != InteractType::BUTTON) continue;
                Button* button = static_cast<Button*>(interactable.get());

                // find the layer this button actually lives on
                std::vector<uint32_t>* buttonTiles = nullptr;
                for (auto& l : m_layers)
                {
                    if (l.name == button->layerName)
                    {
                        buttonTiles = &l.tiles;
                        break;
                    }
                }

                button->objectTiles = buttonTiles;
                button->mapW = m_mapWidth;

                // wire to target door - door still swaps on objects layer
                std::vector<uint32_t>* objectTiles = nullptr;
                for (auto& l : m_layers)
                    if (l.name == "objects")
                        objectTiles = &l.tiles;

                for (auto& other : m_interactables)
                {
                    if (other->type != InteractType::DOOR) continue;
                    if (other->name != button->targetName)  continue;

                    Door* targetDoor = static_cast<Door*>(other.get());
                    int   mapW       = m_mapWidth;

                    button->onInteract = [this, targetDoor, objectTiles, mapW]()
                    {
                        targetDoor->setOpen(!targetDoor->open, objectTiles, mapW);
                    };
                    break;
                }
            }
        }

        objGroup = objGroup->NextSiblingElement("objectgroup");
    }
}

int TileMap::getFirstGid(const std::string& tilesetName) const
{
    for (const auto& ts : m_tilesets)
        if (ts.name == tilesetName)
            return ts.firstGid;
    std::cout << "Tileset not found: " << tilesetName << "\n";
    return -1;
}

int TileMap::pixelToGid(const std::string& tilesetName, int pixelX, int pixelY) const
{
    for (const auto& ts : m_tilesets)
    {
        if (ts.name == tilesetName)
        {
            int col     = pixelX / m_tileWidth;
            int row     = pixelY / m_tileHeight;
            int localId = row * ts.columns + col;
            return ts.firstGid + localId;
        }
    }
    std::cout << "Tileset not found: " << tilesetName << "\n";
    return -1;
}

void TileMap::setTile(const std::string& layerName, int tx, int ty, int gid)
{
    for (auto& layer : m_layers)
    {
        if (layer.name == layerName)
        {
            int index = ty * m_mapWidth + tx;
            if (index >= 0 && index < (int)layer.tiles.size())
                layer.tiles[index] = gid;
            return;
        }
    }
    std::cout << "Layer not found: " << layerName << "\n";
}

const TilesetInfo* TileMap::getTilesetForGid(int gid) const
{
    const TilesetInfo* result = nullptr;
    for (const auto& ts : m_tilesets)
        if (gid >= ts.firstGid && (result == nullptr || ts.firstGid > result->firstGid))
            result = &ts;
    return result;
}

void TileMap::renderLayer(SDL_Renderer* ren, Camera& cam, TileLayer& layer)
{
    for (int i = 0; i < (int)layer.tiles.size(); i++)
    {
        int gid = layer.tiles[i];
        if (gid == 0) continue;

        const TilesetInfo* ts = getTilesetForGid(gid);
        if (!ts) continue;

        int localId = gid - ts->firstGid;

        SDL_Rect src;
        src.x = (localId % ts->columns) * m_tileWidth;
        src.y = (localId / ts->columns) * m_tileHeight;
        src.w = m_tileWidth;
        src.h = m_tileHeight;

        SDL_Rect dst;
        dst.x = cam.toScrX((i % m_mapWidth) * m_tileWidth  * MAP_RENDER_SCALE);
        dst.y = cam.toScrY((i / m_mapWidth)  * m_tileHeight * MAP_RENDER_SCALE);
        dst.w = m_tileWidth  * MAP_RENDER_SCALE;
        dst.h = m_tileHeight * MAP_RENDER_SCALE;

        if (dst.x + dst.w < 0 || dst.x > cam.w) continue;
        if (dst.y + dst.h < 0 || dst.y > cam.h) continue;

        SDL_RenderCopy(ren, ts->tex, &src, &dst);
    }
}

void TileMap::renderLayerToQueue(Camera& cam, TileLayer& layer)
{
    for (int i = 0; i < (int)layer.tiles.size(); i++)
    {
        int gid = layer.tiles[i];
        if (gid == 0) continue;

        const TilesetInfo* ts = getTilesetForGid(gid);
        if (!ts) continue;

        int localId = gid - ts->firstGid;

        SDL_Rect src;
        src.x = (localId % ts->columns) * m_tileWidth;
        src.y = (localId / ts->columns) * m_tileHeight;
        src.w = m_tileWidth;
        src.h = m_tileHeight;

        int screenX = cam.toScrX((i % m_mapWidth) * m_tileWidth  * MAP_RENDER_SCALE);
        int screenY = cam.toScrY((i / m_mapWidth)  * m_tileHeight * MAP_RENDER_SCALE);
        int dstW    = m_tileWidth  * MAP_RENDER_SCALE;
        int dstH    = m_tileHeight * MAP_RENDER_SCALE;

        if (screenX + dstW < 0 || screenX > cam.w) continue;
        if (screenY + dstH < 0 || screenY > cam.h) continue;

        submit(ts->tex, src, screenX, screenY, dstW, dstH);
    }
}

void TileMap::renderGround(SDL_Renderer* ren, Camera& cam)
{
    for (auto& layer : m_layers)
        if (layer.name == "ground")
            renderLayer(ren, cam, layer);
}

void TileMap::renderObjects(SDL_Renderer* ren, Camera& cam)
{
    for (auto& layer : m_layers)
        if (layer.name == "objects")
            renderLayerToQueue(cam, layer);
}

void TileMap::renderOverhead(SDL_Renderer* ren, Camera& cam)
{
    for (auto& layer : m_layers)
        if (layer.name == "overhead")
            renderLayer(ren, cam, layer);
}
