#include "Game.h"

#include <iostream>
#include <fstream>

Game::Game(const std::string &config)
{
    init(config);
}

void Game::init(const std::string &path)
{
    // Read in config file, use premade PlayerConfig, EnemyConfig, and BulletConfig structs

    std::ifstream fin(path);
    std::string next;
    int width, height, frameLimit;
    bool fullscreen;
    std::string fontPath;
    int fontSize, fontR, fontG, fontB;

    // Read in Window from Config
    if (fin >> next)
    {
        fin >> width >> height >> frameLimit >> fullscreen;
    }
    // Read in font from Config
    if (fin >> next)
    {
        fin >> fontPath >> fontSize >> fontR >> fontG >> fontB;
        m_font.loadFromFile(fontPath);
        m_text.setFont(m_font);
        m_text.setCharacterSize(fontSize);
        m_text.setFillColor(sf::Color(fontR, fontG, fontB));
    }

    while (fin >> next)
    {
        fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V >> m_playerConfig.S;
        fin >> next;
        fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
        fin >> next;
        fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
    }
    // setup deafult window parameters
    if (!fullscreen)
    {
        m_window.create(sf::VideoMode(width, height), "SFML window");
    }
    else
    {
        m_window.create(sf::VideoMode(width, height), "SFML window", sf::Style::Fullscreen);
    }

    m_window.setFramerateLimit(frameLimit);

    spawnPlayer();
}

void Game::run()
{

    while (m_running)
    {
        m_entities.update();

        if (!m_paused)
        {
            sLifespan();
            sEnemySpawner();
            sMovement();
            sCollision();

            if (m_currentFrame - m_weaponCooldownTimer > m_weaponCooldown)
            {
                m_weaponAvailable = true;
            }
        }

        sUserInput();
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

    // we create every entity by calling EntityManager.addEntity(tag)

    auto entity = m_entities.addEntity("player");

    // spawn in middle of screen
    float mx = m_window.getSize().x / 2.0f;
    float my = m_window.getSize().y / 2.0f;

    entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(1.0f, 1.0f), 0.0f);

    entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

    entity->cInput = std::make_shared<CInput>();

    entity->cScore = std::make_shared<CScore>(0);

    entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

    m_player = entity;
}

// spawn enemy at random position
void Game::spawnEnemy()
{

    auto entity = m_entities.addEntity("enemy");
    // spawn enemy within bounds of window
    float ex = rand() % m_window.getSize().x;
    float ey = rand() % m_window.getSize().y;

    // random number of vertices for each enemy between VMIN and VMAX
    int numPoints = rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN) + m_enemyConfig.VMIN;

    // random velocity for each enemy between SMIN and SMAX
    float range = m_enemyConfig.SMAX - m_enemyConfig.SMIN;
    float fraction = (float)rand() / RAND_MAX;
    float velocity = m_enemyConfig.SMIN + fraction * range;

    // random color
    int r = rand() % 255;
    int g = rand() % 255;
    int b = rand() % 255;

    // add components to enemy
    entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), Vec2(velocity, velocity), 0.0f);
    entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, numPoints, sf::Color(r, g, b), sf::Color(b, g, r), m_enemyConfig.OT);
    entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
    entity->cScore = std::make_shared<CScore>(100 * numPoints);
    // record when the most recent enemy was spawned
    m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a big one explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    // when create the smaller enemy, read the values of the original enemy
    // spawn a number of small enemies equal to the vertices of the original enemy
    // set each to same color ,half the size
    // worth double points

    // get number of points of the enemy
    size_t points = e->cShape->circle.getPointCount();
    float r = e->cShape->circle.getRadius() / 2;
    sf::Color fill = e->cShape->circle.getFillColor();
    sf::Color outline = e->cShape->circle.getOutlineColor();
    int thickness = e->cShape->circle.getOutlineThickness();
    float cr = e->cCollision->radius / 2;

    for (size_t i = 0; i < points; i++)
    {
        auto entity = m_entities.addEntity("smallenemy");
        float angle = 360.0f / points * i;                                      // Calculate angle for even distribution
        float posX = e->cTransform->pos.x + cos(angle * M_PI / 180.0f) * 50.0f; // Adjust radius as needed
        float posY = e->cTransform->pos.y + sin(angle * M_PI / 180.0f) * 50.0f; // Adjust radius as needed
        float velocityX = cos(angle * M_PI / 180.0f);                           // Calculate x-component based on angle
        float velocityY = sin(angle * M_PI / 180.0f);                           // Calculate y-component based on angle

        entity->cTransform = std::make_shared<CTransform>(Vec2(posX, posY), Vec2(velocityX, velocityY), angle);
        entity->cShape = std::make_shared<CShape>(r / 2, points, fill, outline, thickness);
        entity->cCollision = std::make_shared<CCollision>(cr);
        entity->cScore = std::make_shared<CScore>(2 * e->cScore->score);
        entity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
    }
}

// spawn a bullet from the entity at the mouse position
void Game::spawnBullet(std::shared_ptr<Entity> e, const Vec2 &target)
{
    // spawn a bullet from the entity at the mouse position
    auto bullet = m_entities.addEntity("bullet");

    // bullet speed is given as a scalar speed, set the velocity using normalized vector calculation
    Vec2 dir = target - m_player->cTransform->pos;
    dir.normalize();
    Vec2 bulletVelocity = dir * m_bulletConfig.S;

    // spawn the bullet from the player position to the mouse position
    bullet->cTransform = std::make_shared<CTransform>(m_player->cTransform->pos, bulletVelocity, 0.0f);
    bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
    bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
    bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
}

// spawn a special weapon from the entity
void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{

    for (size_t i = 0; i < 15; i++)
    {
        auto e = m_entities.addEntity("bullet");
        float angle = 360.0f / 15 * i;
        float posX = entity->cTransform->pos.x + cos(angle * M_PI / 180.0f) * 50.0f;
        float posY = entity->cTransform->pos.y + sin(angle * M_PI / 180.0f) * 50.0f;
        float velocityX = cos(angle * M_PI / 180.0f);
        float velocityY = sin(angle * M_PI / 180.0f);

        e->cTransform = std::make_shared<CTransform>(Vec2(posX, posY), Vec2(velocityX, velocityY), angle);
        e->cShape = std::make_shared<CShape>(12, 12, sf::Color::Red, sf::Color::White, 1.0f);
        e->cCollision = std::make_shared<CCollision>(12);
        e->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
    }

    for (size_t i = 0; i < 15; i++)
    {
        auto e = m_entities.addEntity("bullet");
        float angle = 360.0f / 15 * i;
        float posX = entity->cTransform->pos.x + cos(angle * M_PI / 180.0f) * 60.0f;
        float posY = entity->cTransform->pos.y + sin(angle * M_PI / 180.0f) * 60.0f;
        float velocityX = cos(angle * M_PI / 180.0f);
        float velocityY = sin(angle * M_PI / 180.0f);

        e->cTransform = std::make_shared<CTransform>(Vec2(posX, posY), Vec2(velocityX, velocityY), angle);
        e->cShape = std::make_shared<CShape>(12, 12, sf::Color::Red, sf::Color::White, 1.0f);
        e->cCollision = std::make_shared<CCollision>(12);
        e->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
    }
}

// update the movement of all entities
void Game::sMovement()
{
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

    // movement and speed update
    m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
    m_player->cTransform->pos.y += m_player->cTransform->velocity.y;

    // keep the player within the window bounds
    PlayerBounds();

    // bullet movement
    for (auto e : m_entities.getEntities("bullet"))
    {
        e->cTransform->pos.x += e->cTransform->velocity.x;
        e->cTransform->pos.y += e->cTransform->velocity.y;
    }

    // enemy movement
    for (auto e : m_entities.getEntities("enemy"))
    {
        // bounce the enemy off the walls
        if (e->cTransform->pos.x - e->cCollision->radius < 0 || e->cTransform->pos.x + e->cCollision->radius > m_window.getSize().x)
        {
            e->cTransform->velocity.x *= -1;
        }

        if (e->cTransform->pos.y - e->cCollision->radius < 0 || e->cTransform->pos.y + e->cCollision->radius > m_window.getSize().y)
        {
            e->cTransform->velocity.y *= -1;
        }

        e->cTransform->pos.x += e->cTransform->velocity.x;
        e->cTransform->pos.y += e->cTransform->velocity.y;
    }

    // small enemy movement
    for (auto e : m_entities.getEntities("smallenemy"))
    {

        e->cTransform->pos.x += e->cTransform->velocity.x;
        e->cTransform->pos.y += e->cTransform->velocity.y;
    }
}

void Game::PlayerBounds()
{
    // keep the player within the window bounds
    if (m_player->cTransform->pos.x - m_player->cCollision->radius < 0)
    {
        m_player->cTransform->pos.x = m_player->cCollision->radius;
    }

    if (m_player->cTransform->pos.x + m_player->cCollision->radius > m_window.getSize().x)
    {
        m_player->cTransform->pos.x = m_window.getSize().x - m_player->cCollision->radius;
    }

    if (m_player->cTransform->pos.y - m_player->cCollision->radius < 0)
    {
        m_player->cTransform->pos.y = m_player->cCollision->radius;
    }

    if (m_player->cTransform->pos.y + m_player->cCollision->radius > m_window.getSize().y)
    {
        m_player->cTransform->pos.y = m_window.getSize().y - m_player->cCollision->radius;
    }
}

void Game::sLifespan()
{

    // for all entities:
    // if no lifespan component, skip it
    // if an entity has a lifespan, decrement it
    // if it has lifespan and is alive, scale its alpha channel to represent remaining lifespan

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
    // bullets colliding with large enemies
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

                // std::cout << "Playerscore: " << m_player->cScore->score << std::endl;
            }
        }
    }

    // bullets colliding with small enemies
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

                // std::cout << "Playerscore: " << m_player->cScore->score << std::endl;
            }
        }
    }

    // check if enemy collides with player
    for (auto e : m_entities.getEntities("enemy"))
    {
        if (!e->cCollision)
        {
            continue;
        }

        if (m_player->cTransform->pos.dist(e->cTransform->pos) < m_player->cCollision->radius + e->cCollision->radius)
        {
            // destroy the enemy
            e->destroy();
            // destroy the player and respawn in center
            m_player->destroy();
            spawnPlayer();
        }
    }
}

void Game::sEnemySpawner()
{
    // to do: update the enemy spawner
    // if the current frame is greater than the last enemy spawn time + the spawn interval, spawn an enemy
    // use m_currentFrame - m_lastEnemySpawnTime to determine how long since last spawn
    // spawning every 3 seconds at 60 fps
    if (m_currentFrame - m_lastEnemySpawnTime > m_enemyConfig.SI)
        spawnEnemy();
}

void Game::sRender()
{

    m_window.clear();

    m_player->cShape->circle.setPosition(m_player->cTransform->pos.x, m_player->cTransform->pos.y);

    // set rotation and shape based on transform angle
    m_player->cTransform->angle += 1.0f;
    m_player->cShape->circle.setRotation(m_player->cTransform->angle);
    m_window.draw(m_player->cShape->circle);

    // add all entities
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

    // draw the score
    m_text.setString("Score: " + std::to_string(m_player->cScore->score));
    m_window.draw(m_text);
    m_text.setPosition(0, 0);

    m_window.display();
}

void Game::sUserInput()
{

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
            case sf::Keyboard::Escape:
                m_running = false;
                break;
            case sf::Keyboard::P:
                setPaused(!m_paused);
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
            //  std::cout << "Left button clicked at " << event.mouseButton.x << " " << event.mouseButton.y << std::endl;

            // call spawnbullet here
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
            }

            if (event.mouseButton.button == sf::Mouse::Right && m_weaponAvailable)
            {
                // std::cout << "Right button clicked at " << event.mouseButton.x << " " << event.mouseButton.y << std::endl;

                spawnSpecialWeapon(m_player);
                m_weaponAvailable = false;
                m_weaponCooldownTimer = m_currentFrame;
            }
        }
    }
}