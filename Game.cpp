#include "Game.h"

#include <iostream>
#include <fstream>

Game::Game(const std::string &config)
{
    init(config);
}

void Game::init(const std::string &path)
{
    // TODO: read in config file here, use premade PlayerConfig, EnemyConfig, and BulletConfig structs

    std::ifstream fin(path);

    // need to handle the first two lines of the config file
    /*
        fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V >> m_playerConfig.S;
        fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX;
        fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L >> m_bulletConfig.S;
    */
    // setup deafult window parameters
    m_window.create(sf::VideoMode(1280, 720), "SFML window");
    m_window.setFramerateLimit(60);

    spawnPlayer();
}

void Game::run()
{

    // TO DO: add pause functionality here, some systems should function while paused (rendering), others should not
    // (movement,input)

    while (m_running)
    {
        m_entities.update();

        if (!m_paused)
        {
            sLifespan();
            sEnemySpawner();
            sMovement();
            sCollision();
            sUserInput();
        }

        sRender();

        m_currentFrame++;
    }
}

void Game::setPaused(bool paused)
{
    m_paused = paused;
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
    // to do : finish adding all properties of the plaeyr with the correct values from the config
    // currently a sample is hardcoded, but we want to read from the config file (like m_playerConfig.V instead of 8)

    // we create every entity by calling EntityManager.addEntity(tag)

    auto entity = m_entities.addEntity("player");

    // spawn in middle of screen
    float mx = m_window.getSize().x / 2.0f;
    float my = m_window.getSize().y / 2.0f;

    entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(1.0f, 1.0f), 0.0f);

    entity->cShape = std::make_shared<CShape>(32.0f, 8, sf::Color(10, 10, 10), sf::Color(255, 0, 0), 4.0f);

    entity->cInput = std::make_shared<CInput>();

    entity->cScore = std::make_shared<CScore>(0);

    m_player = entity;
}

// spawn enemy at random position
void Game::spawnEnemy()
{
    // to do: finish adding all properties of the enemy with the correct values from the config
    auto entity = m_entities.addEntity("enemy");
    // spawn enemy within bounds of window
    float ex = rand() % m_window.getSize().x;
    float ey = rand() % m_window.getSize().y;

    // hardcode example for now
    entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), Vec2(1.0f, 1.0f), 0.0f);
    entity->cShape = std::make_shared<CShape>(32.0f, 8, sf::Color(10, 10, 10), sf::Color(0, 0, 255), 4.0f);
    entity->cCollision = std::make_shared<CCollision>(32.0f);
    entity->cScore = std::make_shared<CScore>(10);
    // record when the most recent enemy was spawned
    m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a big one explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    // to do: spawn small enemies around the position of the big enemy
    std::cout << "Spawning small enemies" << std::endl;

    // get points of the enemy
    size_t points = e->cShape->circle.getPointCount();
    std::cout << "Points: " << points << std::endl;

    for (size_t i = 0; i < points; i++)
    {
        auto entity = m_entities.addEntity("smallenemy");
        float angle = 360.0f / points * i;                                      // Calculate angle for even distribution
        float posX = e->cTransform->pos.x + cos(angle * M_PI / 180.0f) * 50.0f; // Adjust radius as needed
        float posY = e->cTransform->pos.y + sin(angle * M_PI / 180.0f) * 50.0f; // Adjust radius as needed

        entity->cTransform = std::make_shared<CTransform>(Vec2(posX, posY), Vec2(1.0f, 1.0f), angle);
        entity->cShape = std::make_shared<CShape>(16.0f, 8, sf::Color(100, 100, 100), sf::Color(0, 0, 255), 4.0f);
        entity->cCollision = std::make_shared<CCollision>(16.0f);
        entity->cScore = std::make_shared<CScore>(20);
        entity->cLifespan = std::make_shared<CLifespan>(60);
    }
    // when create the smaller enemy, read the values of the original enemy
    // spawn a number of small enemies equal to the vertices of the original enemy
    // set each to same color ,half the size
    // worth double points
}

// spawn a bullet from the entity at the mouse position
void Game::spawnBullet(std::shared_ptr<Entity> e, const Vec2 &target)
{
    // to do: spawn a bullet from the entity at the mouse position

    std::cout << "Spawning bullet" << std::endl;
    auto bullet = m_entities.addEntity("bullet");

    // bullet speed is given as a scalar speed, set the velocity using normalized vector calculation
    Vec2 dir = target - m_player->cTransform->pos;
    dir.normalize();

    // spawn the bullet from the player position to the mouse position
    bullet->cTransform = std::make_shared<CTransform>(m_player->cTransform->pos, dir * 0.02, 0.0f);
    bullet->cShape = std::make_shared<CShape>(8.0f, 8, sf::Color(255, 255, 255), sf::Color(0, 255, 0), 4.0f);
    bullet->cLifespan = std::make_shared<CLifespan>(60);
    bullet->cCollision = std::make_shared<CCollision>(8.0f);
}

// spawn a special weapon from the entity
void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // to do: spawn a special weapon from the entity
    // special weapon speed is given as a scalar speed, set the velocity using normalized vector calculation
}

// update the movement of all entities
void Game::sMovement()
{
    // to do: update the movement of all entities (not just player)
    // reaad the m_player->cInput to determine if player is moving

    // clear velocity at the start of the frame
    m_player->cTransform->velocity = {0.0f, 0.0f};

    // implement player movement
    if (m_player->cInput->up)
    {
        m_player->cTransform->velocity.y = -5.0f;
    }
    if (m_player->cInput->down)
    {
        m_player->cTransform->velocity.y = 5.0f;
    }
    if (m_player->cInput->left)
    {
        m_player->cTransform->velocity.x = -5.0f;
    }
    if (m_player->cInput->right)
    {
        m_player->cTransform->velocity.x = 5.0f;
    }

    // sample movement and speed update
    m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
    m_player->cTransform->pos.y += m_player->cTransform->velocity.y;

    // bullet movement
    for (auto e : m_entities.getEntities("bullet"))
    {
        e->cTransform->pos.x += e->cTransform->velocity.x;
        e->cTransform->pos.y += e->cTransform->velocity.y;
    }
}

void Game::sLifespan()
{
    // to do: update the lifespan of all entities
    // for all entities:
    // if no lifespan component, skip it
    // if an entity has a lifespan, decrement it
    // if it has lifespan and is alive, scale its alpha channel to represent remaining lifespan
    //

    for (auto e : m_entities.getEntities())
    {
        if (!e->cLifespan)
        {
            continue;
        }

        else if (e->cLifespan->remaining > 0)
        {
            e->cLifespan->remaining--;

            if (e->cLifespan->remaining <= 0)
            {
                e->destroy();
                // std::cout << "Destroying entity" << std::endl;
            }
            else
            {
                // scale alpha channel to represent remaining lifespan
                e->cShape->circle.setFillColor(sf::Color(e->cShape->circle.getFillColor().r, e->cShape->circle.getFillColor().g, e->cShape->circle.getFillColor().b, e->cLifespan->remaining * 255 / e->cLifespan->total));
            }
        }
    }
}

void Game::sCollision()
{

    for (auto b : m_entities.getEntities("bullet"))
    {
        for (auto e : m_entities.getEntities("enemy"))
        {
            if (!e->cCollision)
            {
                continue;
            }

            // check if circles are overlapping for collisions
            if (b->cTransform->pos.dist(e->cTransform->pos) < b->cCollision->radius + e->cCollision->radius)
            {

                // destroy both entities
                b->destroy();
                spawnSmallEnemies(e);
                // increase the player's score
                m_player->cScore->score += e->cScore->score;

                e->destroy();

                std::cout << "Playerscore: " << m_player->cScore->score << std::endl;
            }
        }
    }

    for (auto b : m_entities.getEntities("bullet"))
    {
        for (auto e : m_entities.getEntities("smallenemy"))
        {
            if (!e->cCollision)
            {
                continue;
            }

            // check if circles are overlapping for collisions
            if (b->cTransform->pos.dist(e->cTransform->pos) < b->cCollision->radius + e->cCollision->radius)
            {

                // destroy both entities
                b->destroy();

                // increase the player's score
                m_player->cScore->score += e->cScore->score;

                e->destroy();

                std::cout << "Playerscore: " << m_player->cScore->score << std::endl;
            }
        }
    }

    // to do: update the collision of all entities
    // use collision  radius, not shape radius
    // for all entities:
    // if no collision component, skip it
    // if an entity has a collision, check for collision with all other entities
    // if a collision occurs, call the onCollision function of both entities
}

void Game::sEnemySpawner()
{
    // to do: update the enemy spawner
    // if the current frame is greater than the last enemy spawn time + the spawn interval, spawn an enemy
    // use m_currentFrame - m_lastEnemySpawnTime to determine how long since last spawn
    // spawning every 3 seconds at 60 fps
    if (m_currentFrame - m_lastEnemySpawnTime > 3 * 60)
        spawnEnemy();
}

void Game::sRender()
{
    // TODO: change code to draw all of the entities

    m_window.clear();

    m_player->cShape->circle.setPosition(m_player->cTransform->pos.x, m_player->cTransform->pos.y);

    // set rotation and shape based on transform angle
    m_player->cTransform->angle += 1.0f;
    m_player->cShape->circle.setRotation(m_player->cTransform->angle);
    m_window.draw(m_player->cShape->circle);

    // need to add all entities
    for (auto e : m_entities.getEntities())
    {
        if (!e->isActive())
        {
            continue;
        }
        e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

        // set rotation and shape based on transform angle
        e->cTransform->angle += 1.0f;
        e->cShape->circle.setRotation(e->cTransform->angle);

        m_window.draw(e->cShape->circle);
    }

    m_window.display();
}

void Game::sUserInput()
{

    // TODO: handle user input here, do not implement player's movement logic here

    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_running = false;
        }

        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::W:
                m_player->cInput->up = true;
                break;
            case sf::Keyboard::S:
                m_player->cInput->down = true;
                break;
            case sf::Keyboard::A:
                m_player->cInput->left = true;
                break;
            case sf::Keyboard::D:
                m_player->cInput->right = true;
                break;

            default:
                break;
            }
        }
        // this event is triggered when a key is released

        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::W:
                m_player->cInput->up = false;
                break;
            case sf::Keyboard::S:
                m_player->cInput->down = false;
                break;
            case sf::Keyboard::A:
                m_player->cInput->left = false;
                break;
            case sf::Keyboard::D:
                m_player->cInput->right = false;
                break;
            default:
                break;
            }
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            std::cout << "Left button clicked at " << event.mouseButton.x << " " << event.mouseButton.y << std::endl;

            // call spawnbullet here
            spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
        }

        if (event.mouseButton.button == sf::Mouse::Right)
        {
            std::cout << "Right button clicked at " << event.mouseButton.x << " " << event.mouseButton.y << std::endl;

            // TODO: call spawnSpecialWeapon here
        }
    }
}