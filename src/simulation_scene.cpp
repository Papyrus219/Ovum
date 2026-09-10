#include <Ovum/simulation_scene.hpp>
#include <iostream>
#include <print>

void ovum::Simulation_scene::Init(eruptor::resource::Resource_manager & resource_manager)
{
    blob_handle = resource_manager.Get_model_handle( "blob_model" );
    food_handle = resource_manager.Get_model_handle( "ball_model" );
    light_source_handle = resource_manager.Get_model_handle( "light_source_model" );

    this->resource_manager = &resource_manager;
}

ovum::Simulation_scene::Simulation_scene(const Simulation_scene & other): eruptor::scene::Scene{ other }, free_objects{ other.free_objects }, floor{ other.floor }, entieties{ other.entieties }, food{ other.food }, light_sources{ other.light_sources }, blob_handle{ other.blob_handle }, food_handle{ other.food_handle }, light_source_handle{ other.light_source_handle }, next_entity_alias_id{ other.next_entity_alias_id }, next_food_alias_id{ other.next_food_alias_id }, next_light_source_alias_id{ other.next_light_source_alias_id }
{

}

ovum::Simulation_scene::Simulation_scene(Simulation_scene && other): eruptor::scene::Scene{ std::move( other ) }, free_objects{ std::move(other.free_objects) }, floor{ other.floor }, entieties{ std::move( other.entieties ) }, food{ std::move( other.food ) }, light_sources{ std::move( other.light_sources ) }, blob_handle{ other.blob_handle }, food_handle{ other.food_handle }, light_source_handle{ other.light_source_handle }, next_entity_alias_id{ other.next_entity_alias_id }, next_food_alias_id{ other.next_food_alias_id }, next_light_source_alias_id{ other.next_light_source_alias_id }
{

}

ovum::Simulation_scene::Simulation_scene(const eruptor::scene::Scene & other): eruptor::scene::Scene{ other }
{

}

ovum::Simulation_scene::Simulation_scene(eruptor::scene::Scene && other): eruptor::scene::Scene{ std::move( other ) }
{

}

ovum::Simulation_scene & ovum::Simulation_scene::operator=(const Simulation_scene & other)
{
    eruptor::scene::Scene::operator=( other );

    this->free_objects = other.free_objects;
    this->floor = other.floor;
    this->entieties = other.entieties;
    this->food = other.food;
    this->light_sources = other.light_sources;
    this->blob_handle = other.blob_handle;
    this->food_handle = other.food_handle;
    this->light_source_handle = other.light_source_handle;
    this->next_entity_alias_id = other.next_entity_alias_id;
    this->next_food_alias_id = other.next_food_alias_id;
    this->next_light_source_alias_id = other.next_light_source_alias_id;
    this->resource_manager = other.resource_manager;

    return *this;
}

ovum::Simulation_scene & ovum::Simulation_scene::operator=(Simulation_scene && other)
{
    eruptor::scene::Scene::operator=( std::move( other ) );

    this->free_objects = std::move(other.free_objects);
    this->floor = std::move( other.floor );
    this->entieties = std::move( other.entieties );
    this->food = std::move( other.food );
    this->light_sources = std::move( other.light_sources );
    this->blob_handle = other.blob_handle;
    this->food_handle = other.food_handle;
    this->light_source_handle = other.light_source_handle;
    this->next_entity_alias_id = other.next_entity_alias_id;
    this->next_food_alias_id = other.next_food_alias_id;
    this->next_light_source_alias_id = other.next_light_source_alias_id;
    this->resource_manager = other.resource_manager;

    return *this;
}

ovum::Simulation_scene & ovum::Simulation_scene::operator=(const eruptor::scene::Scene & other)
{
    eruptor::scene::Scene::operator=( std::move( other ) );

    return *this;
}

ovum::Simulation_scene & ovum::Simulation_scene::operator=(eruptor::scene::Scene && other)
{
    eruptor::scene::Scene::operator=( std::move( other ) );

    return *this;
}

uint32_t ovum::Simulation_scene::Add_entity()
{
    if(free_objects.empty())
    {
        render_objects.push_back({});
        entieties.push_back({});
        entieties.back().render_object_id = render_objects.size() - 1;

        auto alias = "Blob_" + std::to_string(next_entity_alias_id++);
        auto [it, inserted] = objects_aliases.insert({alias, entieties.back().render_object_id});
        reverse_object_aliases.insert({entieties.back().render_object_id, it->first});
    }
    else
    {
        entieties.push_back({});
        entieties.back().render_object_id = free_objects.front();

        auto alias = "Blob_" + std::to_string(next_entity_alias_id++);
        auto [it, inserted] = objects_aliases.insert({alias, entieties.back().render_object_id});
        reverse_object_aliases.insert({entieties.back().render_object_id, it->first});

        free_objects.pop();
    }

    render_objects[ entieties.back().render_object_id ].Reset();
    render_objects[ entieties.back().render_object_id ].Set_model(entieties.back().render_object_id, blob_handle);
    render_objects[ entieties.back().render_object_id ].Set_position( {0, 0.1, 0} );
    render_objects[ entieties.back().render_object_id ].snap_y = 0.1;
    render_objects[ entieties.back().render_object_id ].Set_scale( {6, 6, 6});
    render_objects[ entieties.back().render_object_id ].color = eruptor::resource::Color{100, 200, 60};
    render_objects[ entieties.back().render_object_id ].shading_type = eruptor::scene::Shading_type::OPAQUE;

    return entieties.size() - 1;
}

uint32_t ovum::Simulation_scene::Add_food()
{
    if(free_objects.empty())
    {
        render_objects.push_back({});
        food.push_back({});
        food.back().render_object_id = render_objects.size() - 1;

        auto alias = "Food_" + std::to_string(next_food_alias_id++);
        auto [it, inserted] = objects_aliases.insert({alias, food.back().render_object_id});
        if(!inserted)
        {
            std::println(std::cerr, "Alias collision: {}", alias);
        }

        reverse_object_aliases.insert({food.back().render_object_id, it->first});
    }
    else
    {
        food.push_back({});
        food.back().render_object_id = free_objects.front();

        auto alias = "Food_" + std::to_string(next_food_alias_id++);
        auto [it, inserted] = objects_aliases.insert({alias, food.back().render_object_id});
        if(!inserted)
        {
            std::println(std::cerr, "Alias collision: {}", alias);
        }

        reverse_object_aliases.insert({food.back().render_object_id, it->first});

        free_objects.pop();
    }

    render_objects[ food.back().render_object_id ].Reset();
    render_objects[ food.back().render_object_id ].Set_model(food.back().render_object_id, food_handle);
    render_objects[ food.back().render_object_id ].Set_position( {0, 0.1, 0} );
    render_objects[ food.back().render_object_id ].snap_y = 0.1;
    render_objects[ food.back().render_object_id ].Set_scale( {0.2, 0.2, 0.2} );
    render_objects[ food.back().render_object_id ].color = eruptor::resource::Color{60, 60, 200};
    render_objects[ food.back().render_object_id ].shading_type = eruptor::scene::Shading_type::OPAQUE;

    return food.size() - 1;
}

uint32_t ovum::Simulation_scene::Add_light_source()
{
    if(light_sources.size() == 16) return 0;

    if(free_objects.empty())
    {
        render_objects.push_back({});
        light_sources.push_back({});
        light_sources.back().render_object_id = render_objects.size() - 1;

        auto alias = "light_source_" + std::to_string(next_light_source_alias_id++);
        auto [it, inserted] = objects_aliases.insert({alias, light_sources.back().render_object_id});
        if(!inserted)
        {
            std::println(std::cerr, "Alias collision: {}", alias);
        }

        reverse_object_aliases.insert({light_sources.back().render_object_id, it->first});
    }
    else
    {
        light_sources.push_back({});
        light_sources.back().render_object_id = free_objects.front();

        auto alias = "light_source_" + std::to_string(next_light_source_alias_id++);
        auto [it, inserted] = objects_aliases.insert({alias, light_sources.back().render_object_id});
        if(!inserted)
        {
            std::println(std::cerr, "Alias collision: {}", alias);
        }

        reverse_object_aliases.insert({light_sources.back().render_object_id, it->first});

        free_objects.pop();
    }

    render_objects[ light_sources.back().render_object_id ].Reset();
    render_objects[ light_sources.back().render_object_id ].Set_model(light_sources.back().render_object_id, light_source_handle);
    render_objects[ light_sources.back().render_object_id ].Set_position( {0, 0.1, 0} );
    render_objects[ light_sources.back().render_object_id ].snap_y = 0.1;
    render_objects[ light_sources.back().render_object_id ].Set_scale( {0.2, 0.2, 0.2});
    render_objects[ light_sources.back().render_object_id ].color = eruptor::resource::Color{255, 255, 255};
    render_objects[ light_sources.back().render_object_id ].shading_type = eruptor::scene::Shading_type::LIGHT_CASTER;

    return light_sources.size() - 1;
}

void ovum::Simulation_scene::Remove_entity(uint32_t render_object_id)
{
    auto it = std::find_if(entieties.begin(), entieties.end(),
              [render_object_id](const Entiety_data & entity){return entity.render_object_id == render_object_id;});

    if(it == entieties.end())
    {
        std::println("Warming! Trying to remove entity with unknown render_id: {}", render_object_id);
        return;
    }

    auto alias_it = reverse_object_aliases.find(render_object_id);
    if(alias_it != reverse_object_aliases.end())
    {
        objects_aliases.erase( objects_aliases.find( std::string{alias_it->second} ) );
        reverse_object_aliases.erase( alias_it );
    }

    free_objects.push( render_object_id );
    render_objects[ render_object_id ].is_active = false;
    entieties.erase( it );
}

void ovum::Simulation_scene::Remove_food(uint32_t render_object_id)
{
    auto it = std::find_if(food.begin(), food.end(),
              [render_object_id](const Food_data & food){return food.render_object_id == render_object_id;});

    if(it == food.end())
    {
        std::println("Warming! Trying to remove food with unknown render_id: {}", render_object_id);
        return;
    }

    auto alias_it = reverse_object_aliases.find(render_object_id);
    if(alias_it != reverse_object_aliases.end())
    {
        objects_aliases.erase( objects_aliases.find( std::string{alias_it->second} ) );
        reverse_object_aliases.erase( alias_it );
    }

    free_objects.push( render_object_id );
    render_objects[ render_object_id ].is_active = false;
    food.erase( it );
}

void ovum::Simulation_scene::Remove_light_source(uint32_t render_object_id)
{
    auto it = std::find_if(light_sources.begin(), light_sources.end(),
                           [render_object_id](const Light_source_data & light_source){return light_source.render_object_id == render_object_id;});

    if(it == light_sources.end())
    {
        std::println("Warming! Trying to remove light_source with unknown render_id: {}", render_object_id);
        return;
    }

    auto alias_it = reverse_object_aliases.find(render_object_id);
    if(alias_it != reverse_object_aliases.end())
    {
        objects_aliases.erase( objects_aliases.find( std::string{alias_it->second} ) );
        reverse_object_aliases.erase( alias_it );
    }

    free_objects.push( render_object_id );
    render_objects[ render_object_id ].is_active = false;
    light_sources.erase( it );
}

std::optional< std::reference_wrapper<ovum::Entiety_data> > ovum::Simulation_scene::Get_if_is_entiety(uint32_t id)
{
    for(auto i{0UZ}; i < entieties.size(); i++)
    {
        if(entieties[i].render_object_id == id)
        {
            return entieties[i];
        }
    }

    return std::nullopt;
}

std::optional< std::reference_wrapper<ovum::Food_data> > ovum::Simulation_scene::Get_if_is_food(uint32_t id)
{
    for(auto i{0UZ}; i < food.size(); i++)
    {
        if(food[i].render_object_id == id)
        {
            return food[i];
        }
    }

    return std::nullopt;
}

std::optional<std::reference_wrapper<ovum::Light_source_data>> ovum::Simulation_scene::Get_if_is_light_source(uint32_t id)
{
    for(auto i{0UZ}; i < light_sources.size(); i++)
    {
        if(light_sources[i].render_object_id == id)
        {
            return light_sources[i];
        }
    }

    return std::nullopt;
}

void ovum::Simulation_scene::Remove_element(uint32_t id)
{
    render_objects.erase( render_objects.begin() + id );

    if(floor == id) floor = 0;

    for(auto & entity : entieties)
    {
        if( entity.render_object_id == id)
        {
            entity.render_object_id = 0;
            return;
        }
    }

    for(auto & food : food)
    {
        if( food.render_object_id == id )
        {
            food.render_object_id = 0;
            return;
        }
    }
}

void ovum::Entiety_data::Reset()
{
    food_eaten = 0;

    ai_data.state = Ai_state::RESTING;
}

void ovum::Entiety_data::Eat()
{
    food_eaten++;

    if(food_eaten >= 2)
    {
        ai_data.state = Ai_state::RETURN;
    }
}

