#include "Log.h"
#include "App.h"
#include "M_Render.h"
#include "M_FileSystem.h"
#include "M_Textures.h"
#include "M_Map.h"
#include "M_FogOfWar.h"
#include "M_GUI.h"
#include "GUILabel.h"
#include <math.h>
#include "Brofiler/Brofiler.h"
#include <string.h>

M_Map::M_Map(bool startEnabled) : Module(startEnabled), mapLoaded(false)
{
	name.assign("map");
}

// Destructor
M_Map::~M_Map()
{}

// Called before render is available
bool M_Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder.assign(config.child("folder").child_value());

	return ret;
}

void M_Map::Draw()
{
	BROFILER_CATEGORY("MAP Draw", Profiler::Color::Green);
	if (mapLoaded == false)
		return;

	std::list<MapLayer*>::iterator item = data.layers.begin();
	std::list<MapLayer*>::iterator end = data.layers.end();

	while (item != end)
	{
		MapLayer* layer = item._Ptr->_Myval;

		if (layer->properties.Get("Nodraw") == true)
		{
			if ((layer->properties.Get("Nodraw") != 0 || layer->properties.Get("Navigation") == 1) && app->debug != true)
			{
				item++;
				continue;
			}
		}
		for (int y = 0; y < data.height; ++y)
		{
			for (int x = 0; x < data.width; ++x)
			{
				int tile_id = layer->Get(x, y);
				if (tile_id > 0)
				{
					TileSet* tileset = GetTilesetFromTileId(tile_id);

					SDL_Rect r = tileset->GetTileRect(tile_id);
					iPoint pos = MapToWorld(x, y);

					//TODO: this should be temporary until we find out what happens, also,TODO solve InsideRenderTarget: after moving the camera doesnt work.
					//if (App->render->camera->InsideRenderTarget(pos.x, pos.y))
					app->render->Blit(tileset->texture, pos.x - data.tileWidth, pos.y - data.tileHeight, &r);
				}
			}
		}
		item++;
	}
}

int Properties::Get(const char* value, int defaultValue) const
{
	std::list<Property*>::const_iterator item = list.begin();

	while(item != list.end())
	{
		if((*item)->name == value)
			return (*item)->value;
		item++;
	}

	return defaultValue;
}

TileSet* M_Map::GetTilesetFromTileId(int id) const
{
	std::list<TileSet*>::const_iterator item = data.tilesets.begin();
	TileSet* set = (*item);

	while(item != data.tilesets.end())
	{
		if(id < (*item)->firstgid)
		{
			set = (item._Ptr->_Prev->_Myval);//-1 wasn't working
			break;
		}
		set = (*item);
		item++;
	}

	return set;
}

iPoint M_Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	if(data.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x * data.tileWidth;
		ret.y = y * data.tileHeight;
	}
	else if(data.type == MAPTYPE_ISOMETRIC)
	{
		//ret.x = (x - y) * (int)(data.tileWidth * 0.5f) - data.tileWidth * 0.5f;
		//ret.y = (x + y) * (int)(data.tileHeight * 0.5f) + (x + y);

		//How it should be:
		int halfWidth = data.tileWidth * 0.5f;
		int halfHeight = data.tileHeight * 0.5f;
		ret.x = (x - y) * halfWidth;
		ret.y = (x + y) * halfHeight;

	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

iPoint M_Map::WorldToMap(int x, int y) const
{
	iPoint ret(x + data.tileWidth * 0.5f, y);

	if(data.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x / data.tileWidth;
		ret.y = y / data.tileHeight;
	}
	else if(data.type == MAPTYPE_ISOMETRIC)
	{
		float halfWidth = (data.tileWidth + 1) * 0.5f;//MAGIC NUMBERS
		float halfHeight = (data.tileHeight + 2) * 0.5f;//MAGIC NUMBERS

		/*float pX = (((ret.x / halfWidth) + (ret.y / halfHeight)) * 0.5f);
		float pY = (((ret.y / halfHeight) - (ret.x / halfWidth)) * 0.5f);

		ret.x = (pX > (floor(pX) + 0.5f)) ? ceil(pX) : floor(pX);
		ret.y = (pY > (floor(pY) + 0.5f)) ? ceil(pY) : floor(pY);

		if (ret.x <= 0)ret.x = 0;
		else if (ret.x >= 120)ret.x = 120;
		if (ret.y <= 0)ret.y = 0;
		else if (ret.y >= 120)ret.y = 120;*/

		//HOW IT SHOULD BE :
		halfWidth = data.tileWidth * 0.5f;
		halfHeight = data.tileHeight * 0.5f;

		ret.x = (x / halfWidth + y / halfHeight) * 0.5f;
		ret.y = (y / halfHeight - (x / halfWidth)) * 0.5f;

		ret.x = (ret.x > (floor(ret.x) + 0.5f)) ? ceil(ret.x) : floor(ret.x);
		ret.y = (ret.y > (floor(ret.y) + 0.5f)) ? ceil(ret.y) : floor(ret.y);

	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

iPoint M_Map::MapToWorldCenter(int x, int y) const
{
	iPoint ret = MapToWorld(x, y);

	ret.x += data.tileWidth * 0.5f;
	ret.y += data.tileHeight * 0.5f - MARGIN;

	return ret;
}

SDL_Rect TileSet::GetTileRect(int id) const
{
	int relative_id = id - firstgid;
	SDL_Rect rect;
	rect.w = tileWidth;
	rect.h = tileHeight;
	rect.x = margin + ((rect.w + spacing) * (relative_id % numTilesWidth));
	rect.y = margin + ((rect.h + spacing) * (relative_id / numTilesWidth));
	return rect;
}

// Called before quitting
bool M_Map::CleanUp()
{
	LOG("Unloading map");

	// Remove all tilesets
	std::list<TileSet*>::iterator item;
	item = data.tilesets.begin();

	while (item != data.tilesets.end())
	{
		RELEASE((*item));
		item++;
	}
	data.tilesets.clear();

	// Remove all layers
	std::list<MapLayer*>::iterator item2;
	item2 = data.layers.begin();

	while(item2 != data.layers.end())
	{
		RELEASE((*item2));
		item2++;
	}
	data.layers.clear();

	// Clean up the pugui tree
	mapFile.reset();

	return true;
}

// Load new map
bool M_Map::Load(const char* path)
{
	bool ret = true;
	std::string tmp = folder + std::string(path);

	char* buf;
	int size = app->fs->Load(tmp.c_str(), &buf);
	pugi::xml_parse_result result = mapFile.load_buffer(buf, size);

	RELEASE(buf);

	if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", path, result.description());
		ret = false;
	}

	// Load general info ----------------------------------------------
	if(ret == true)
	{
		ret = LoadMap();
	}

	// Load all tilesets info ----------------------------------------------
	pugi::xml_node tileset;
	for(tileset = mapFile.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if(ret == true)
		{
			ret = LoadTilesetDetails(tileset, set);
		}

		if(ret == true)
		{
			ret = LoadTilesetImage(tileset, set);
		}

		data.tilesets.push_back(set);
	}

	// Load layer info ----------------------------------------------
	pugi::xml_node layer;
	for(layer = mapFile.child("map").child("layer"); layer && ret; layer = layer.next_sibling("layer"))
	{
		MapLayer* lay = new MapLayer();

		ret = LoadLayer(layer, lay);

		if(ret == true)
			data.layers.push_back(lay);
	}

	if(ret == true)
	{
		LOG("Successfully parsed map XML file: %s", path);
		LOG("width: %d height: %d", data.width, data.height);
		LOG("tile_width: %d tile_height: %d", data.tileWidth, data.tileHeight);

		std::list<TileSet*>::iterator item = data.tilesets.begin();
		while(item != data.tilesets.end())
		{
			TileSet* s = item._Ptr->_Myval;
			LOG("Tileset ----");
			LOG("name: %s firstgid: %d", s->name.c_str(), s->firstgid);
			LOG("tile width: %d tile height: %d", s->tileWidth, s->tileHeight);
			LOG("spacing: %d margin: %d", s->spacing, s->margin);
			item._Ptr = item._Ptr->_Next;
		}

		std::list<MapLayer*>::iterator item_layer = data.layers.begin();
		while(item_layer != data.layers.end())
		{
			MapLayer* l = item_layer._Ptr->_Myval;
			LOG("Layer ----");
			LOG("name: %s", l->name.c_str());
			LOG("tile width: %d tile height: %d", l->width, l->height);
			item_layer._Ptr = item_layer._Ptr->_Next;
		}

		//Define map area 
		SDL_Rect mapArea;
		mapArea.x = ((data.width) * data.tileWidth) * -0.5;
		mapArea.y = -app->render->camera->GetPosition().y;
		mapArea.w = data.width * data.tileWidth;
		mapArea.h = data.height * data.tileHeight + data.height;

		// Define map quadtree area
		mapQuadtree.SetBoundaries(mapArea);
		mapQuadtree.SetMaxObjects(10);

		//Fill the draw quad tree with all the tiles coordinates
		uint fails = 0;
		for (uint y = 0; y < data.height; y++)
		{
			for (uint x = 0; x < data.width; x++)
			{
				iPoint loc = MapToWorldCenter(x, y);
				if (!mapQuadtree.Insert(iPoint(x, y), &loc)) fails++;
			}
		}

		LOG("Map QuadTree generated with: %i errors", fails);
	}


	for (auto element : data.layers)
	{
		if (strcmp(element->name.c_str(), "terrain") == 0)
		{
			for (int x = 0; x < element->height; ++x)
			{
				for (int y = 0; y < element->width; ++y)
				{
					iPoint worldPoint = MapToWorld(x, y);
					std::string str_x = std::to_string(x);
					std::string str_y = std::to_string(y);
					std::string str_c = "(";
					str_c.append(str_x);
					str_c.append(", ");
					str_c.append(str_y);
					str_c.append(")");

					GUILabel* label = new GUILabel(std::string(str_c), STANDARD_PRESET);
					label->SetColor({ 170,0,255,255 });
					label->SetInteractive(false);
					label->SetText(str_c.c_str(), SMALL);
					label->SetCanFocus(false);
					//label->SetRectangle(10, 10, 10, 10);
					label->SetGlobalPos(worldPoint.x-(label->GetLocalRect().w/2), worldPoint.y - (label->GetLocalRect().h / 2));
					label->FollowScreen(false);
					label->SetVisible(true);
					//label->SetDrawPosition(worldPoint.x, worldPoint.y);
					app->gui->mapDebugList.push_back(label);
				}
			}
		}
	}


	mapLoaded = ret;

	return ret;
}

// Load map general properties
bool M_Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = mapFile.child("map");

	if(map == nullptr)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		data.width = map.attribute("width").as_int();
		data.height = map.attribute("height").as_int();
		data.tileWidth = map.attribute("tilewidth").as_int();
		data.tileHeight = map.attribute("tileheight").as_int();
		std::string bg_color(map.attribute("backgroundcolor").as_string());

		data.backgroundColor.r = 0;
		data.backgroundColor.g = 0;
		data.backgroundColor.b = 0;
		data.backgroundColor.a = 0;

		if(bg_color.size() > 0)
		{
			std::string red, green, blue;
			red = bg_color.substr(1, 2);	//TODO: Parameters may be ivalid. Check 0, 1
			green = bg_color.substr(3, 4);			 //TODO: Parameters may be ivalid. Check 1, 1
			blue = bg_color.substr(5, 6);			 //TODO: Parameters may be ivalid. Check 2, 1

			int v = 0;

			sscanf_s(red.c_str(), "%x", &v);
			if(v >= 0 && v <= 255) data.backgroundColor.r = v;

			sscanf_s(green.c_str(), "%x", &v);
			if(v >= 0 && v <= 255) data.backgroundColor.g = v;

			sscanf_s(blue.c_str(), "%x", &v);
			if(v >= 0 && v <= 255) data.backgroundColor.b = v;
		}

		std::string orientation(map.attribute("orientation").as_string());

		if(orientation == "orthogonal")
		{
			data.type = MAPTYPE_ORTHOGONAL;
		}
		else if(orientation == "isometric")
		{
			data.type = MAPTYPE_ISOMETRIC;
		}
		else if(orientation == "staggered")
		{
			data.type = MAPTYPE_STAGGERED;
		}
		else
		{
			data.type = MAPTYPE_UNKNOWN;
		}
	}


	return ret;
}

bool M_Map::LoadTilesetDetails(pugi::xml_node& tilesetNode, TileSet* set)
{
	bool ret = true;
	set->name.assign(tilesetNode.attribute("name").as_string());
	set->firstgid = tilesetNode.attribute("firstgid").as_int();
	set->tileWidth = tilesetNode.attribute("tilewidth").as_int();
	set->tileHeight = tilesetNode.attribute("tileheight").as_int();
	set->margin = tilesetNode.attribute("margin").as_int();
	set->spacing = tilesetNode.attribute("spacing").as_int();
	pugi::xml_node offset = tilesetNode.child("tileoffset");

	if(offset != nullptr)
	{
		set->offsetX = offset.attribute("x").as_int();
		set->offsetY = offset.attribute("y").as_int();
	}
	else
	{
		set->offsetX = 0;
		set->offsetY = 0;
	}

	return ret;
}

bool M_Map::LoadTilesetImage(pugi::xml_node& tilesetNode, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tilesetNode.child("image");

	if(image == nullptr)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		set->texture = app->tex->Load(PATH(folder.c_str(), image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, nullptr, nullptr, &w, &h);
		set->texWidth = image.attribute("width").as_int();

		if(set->texWidth <= 0)
		{
			set->texWidth = w;
		}

		set->texHeight = image.attribute("height").as_int();

		if(set->texHeight <= 0)
		{
			set->texHeight = h;
		}

		set->numTilesWidth = set->texWidth / set->tileWidth;
		set->numTilesHeight = set->texHeight / set->tileHeight;
	}

	return ret;
}

bool M_Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();
	LoadProperties(node, layer->properties);
	pugi::xml_node layer_data = node.child("data");

	if(layer_data == nullptr)
	{
		LOG("Error parsing map xml file: Cannot find 'layer/data' tag.");
		ret = false;
		RELEASE(layer);
	}
	else
	{
		layer->data = new uint[layer->width*layer->height];
		memset(layer->data, 0, layer->width*layer->height);

		int i = 0;
		for(pugi::xml_node tile = layer_data.child("tile"); tile; tile = tile.next_sibling("tile"))
		{
			layer->data[i++] = tile.attribute("gid").as_int(0);
		}
	}

	return ret;
}

// Load a group of properties from a node and fill a list with it
bool M_Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = false;

	pugi::xml_node data = node.child("properties");

	if(data != nullptr)
	{
		pugi::xml_node prop;

		for(prop = data.child("property"); prop; prop = prop.next_sibling("property"))
		{
			Properties::Property* p = new Properties::Property();

			p->name = prop.attribute("name").as_string();
			p->value = prop.attribute("value").as_int(0);

			properties.list.push_back(p);
		}
	}

	return ret;
}

bool M_Map::CreateWalkabilityMap(int& width, int& height, uchar** buffer) const
{
	bool ret = false;
	std::list<MapLayer*>::const_iterator item;
	item = data.layers.begin();

	for(item = data.layers.begin(); item != data.layers.end(); ++item)
	{
		MapLayer* layer = (*item);

		if(layer->properties.Get("Navigation", 0) == 0)
			continue;

		uchar* map = new uchar[layer->width*layer->height];
		memset(map, 1, layer->width*layer->height);

		for(int y = 0; y < data.height; ++y)
		{
			for(int x = 0; x < data.width; ++x)
			{
				int i = (y*layer->width) + x;

				int tile_id = layer->Get(x, y);
				TileSet* tileset = (tile_id > 0) ? GetTilesetFromTileId(tile_id) : nullptr;
				
				if(tileset != nullptr)
				{
					map[i] = (tile_id - tileset->firstgid) > 0 ? 0 : 1;
					/*TileType* ts = tileset->GetTileType(tile_id);
					if(ts != nullptr)
					{
						map[i] = ts->properties.Get("walkable", 1);
					}*/
				}
			}
		}

		*buffer = map;
		width = data.width;
		height = data.height;
		ret = true;

		break;
	}

	return ret;
}

void M_Map::DrawDebug()
{
}
