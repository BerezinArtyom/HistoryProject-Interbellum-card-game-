#include <SFML/Graphics.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <windows.h>
#include <random>

template<typename T>
[[nodiscard]] constexpr sf::Vector2<T> lerp(
    const sf::Vector2<T>& a, const sf::Vector2<T>& b, float t) noexcept
{
    t = std::clamp(t, 0.f, 1.f);
    return a + (b - a) * t;
}

class TextBox : public sf::Drawable
{
public:
    TextBox(const sf::Font& font, unsigned int characterSize = 24) noexcept
        : text(font)
    {
        text.setCharacterSize(characterSize);
        text.setFillColor(sf::Color::White);
    }

    void setText(const sf::String& str) noexcept
    {
        text.setString(str);
        wrapText();
    }

    void setPosition(const sf::Vector2f& pos) noexcept
    {

        text.setPosition(pos);

    }

    void setScale(const sf::Vector2f& s) noexcept
    {
        text.setScale(s);
    }

    void setBounds(const sf::Vector2f& b) noexcept
    {
        bounds = b;
        wrapText();
    }

    void setRotation(float angleDegrees) noexcept
    {
        text.setRotation(sf::degrees(angleDegrees));
    }

    float getRotation() const noexcept
    {
        return text.getRotation().asDegrees();
    }


    void setOrigin(const sf::Vector2f origin) noexcept
    {
        text.setOrigin(origin);
    }
private:
    sf::Text     text;
    sf::Vector2f bounds = { 1000.f, 1000.f };


    sf::FloatRect getLocalBounds() const noexcept
    {
        return text.getLocalBounds();
    }
    void wrapText() noexcept
    {
        sf::String original = text.getString();
        sf::String wrapped;
        sf::String line;

        for (auto ch : original)
        {
            if (ch == '\n')
            {
                wrapped += line + '\n';
                line.clear();
                continue;
            }

            line += ch; // сначала добавляем символ

            text.setString(line);
            if (text.getLocalBounds().size.x >= bounds.x)
            {
                // убираем последний символ, переносим его на новую строку
                line.erase(line.getSize() - 1);
                wrapped += line + '\n';
                line.clear();
                line += ch;
            }
        }

        // добавляем последнюю строку
        wrapped += line;

        text.setString(wrapped);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(text, states);
    }
};
//*

//*/
struct CardModifiers
{
    float    s_ideology = 1.f;
    float    s_finance = 1.f;
    float    s_moral = 1.f;
    float    s_influence = 1.f;

    float    g_finance = 1.f;
    float    g_moral = 1.f;
    float    g_influence = 1.f;

    float    b_agentNet = 1.f;
    float    b_redChance = 1.f;
    float    b_yellowChance = 1.f;
    float    b_greenChance = 1.f;

    void operator+(const CardModifiers& other) noexcept
    {
        s_ideology += other.s_ideology;
        s_finance += other.s_finance;
        s_moral += other.s_moral;
        s_influence += other.s_influence;

        g_finance += other.g_finance;
        g_moral += other.g_moral;
        g_influence += other.g_influence;

        b_agentNet += other.b_agentNet;
        b_redChance += other.b_redChance;
        b_yellowChance += other.b_yellowChance;
        b_greenChance += other.b_greenChance;
    }

};
struct CountryStates
{
    float    s_ideology = 0.f;
    int      s_finance = 0;
    float    s_moral = 1.f;
    float    s_influence = 0.5f;

    int      g_finance = 0;
    float    g_moral = 1.f;
    float    g_influence = 0.5f;

    float    b_agentNet = 0.f;
    float    b_redChance = 33.3f;
    float    b_yellowChance = 33.3f;
    float    b_greenChance = 33.3f;


    void applyOther(const CountryStates& other, const CardModifiers& modifiers = {})
    {
        s_ideology += other.s_ideology * modifiers.s_ideology;
        s_finance += other.s_finance * modifiers.s_finance;
        s_moral += other.s_moral * modifiers.s_moral;
        s_influence += other.s_influence * modifiers.s_influence;

        g_finance += other.g_finance * modifiers.g_finance;
        g_moral += other.g_moral * modifiers.g_moral;
        g_influence += other.g_influence * modifiers.g_influence;

        b_agentNet += other.b_agentNet * modifiers.b_agentNet;
        b_redChance += other.b_redChance * modifiers.b_redChance;
        b_yellowChance += other.b_yellowChance * modifiers.b_yellowChance;
        b_greenChance += other.b_greenChance * modifiers.b_greenChance;
    }
};
class Card : public sf::Drawable
{
public:
    // 1 - soviet, 2 - german, 3 - brittish
    int cardType = 1;
    Card(const sf::Texture& cardsTexture,
        const sf::Font& font,
        const sf::IntRect& textureRect)
        : cardSprite(cardsTexture)
        , description(font, 32)
        , cardTextureRect(textureRect)
    {
        cardSprite.setTextureRect(textureRect);


    
        sf::Vector2f bounds = cardSprite.getGlobalBounds().size;
        cardSprite.setOrigin({ bounds.x / 2.f, bounds.y / 2.f });

        descMargin = bounds.x * 0.1f;
        description.setBounds({ bounds.x - 2*descMargin, bounds.y / 2.f });
        setScale({ 1.0f, 1.0f });

    }


    void setRotation(float angle) noexcept
    {
        targetRotation = angle;
    }

    void update(double deltaTime, double swapSpeed) noexcept
    {
        flipProgress += deltaTime * swapSpeed * (opened ? 1.0 : -1.0);
        flipProgress = std::clamp(flipProgress, 0.0, 1.0);

        double t = flipProgress < 0.5
            ? 4.0 * flipProgress * flipProgress * flipProgress
            : 1.0 - std::pow(-2.0 * flipProgress + 2.0, 3.0) / 2.0;

        swapState = static_cast<float>(t * 2.0 - 1.0);   // [-1 .. +1]

        cardSprite.setScale({ scale.x * swapState, scale.y });
        description.setScale({ scale.x * swapState, scale.y });

        uint8_t cardVariant = swapState >= 0.f ? 1 : 0;
        cardSprite.setTextureRect(
            sf::IntRect({ cardTextureRect.size.x * cardVariant * cardType, 0 },
                { cardTextureRect.size.x, cardTextureRect.size.y }));

        if (moveProgress < 1.f)
        {
            moveProgress += static_cast<float>(deltaTime) * moveSpeed;
            moveProgress = std::min(moveProgress, 1.f);

            float mt = 1.f - std::pow(1.f - moveProgress, 3.f);
            sf::Vector2f pos = startPos + (endPos - startPos) * mt;

            cardSprite.setPosition(pos);
            description.setPosition(pos);

         
            sf::Vector2f cardBounds = cardSprite.getLocalBounds().size;

       
            sf::Vector2f descOrigin = { cardBounds.x / 2.f - descMargin, 0.f };

            description.setOrigin(descOrigin);
        }

        float k = std::min(1.f, static_cast<float>(deltaTime) * rotationLerpSpeed);
        currentRotation += (targetRotation - currentRotation) * k;

        cardSprite.setRotation(sf::degrees(currentRotation));
        description.setRotation(currentRotation);


    }

    void swap() noexcept
    {
        opened = !opened;
    }

    void teleport(sf::Vector2f position) noexcept
    {
        startPos = position;
        endPos = position;
        cardSprite.setPosition(position);


        sf::Vector2f cardSize = {
            static_cast<float>(cardTextureRect.size.x) * scale.x,
            static_cast<float>(cardTextureRect.size.y) * scale.y
        };
        description.setPosition({ position.x - cardSize.x / 2.f, position.y });


        currentRotation = targetRotation;
        cardSprite.setRotation(sf::degrees(currentRotation));
        description.setRotation(currentRotation);
    }
    
    void moveTo(sf::Vector2f position) noexcept
    {
        startPos = cardSprite.getPosition();
        endPos = position;
        moveProgress = 0.f;
    }

    void setScale(const sf::Vector2f& s) noexcept
    {
        scale = s;

    }

    void setDescription(const sf::String& text) noexcept
    {
        description.setText(text);
    }

    float moveSpeed = 2.f;
    float rotationLerpSpeed = 8.f;
    bool   opened = false;
    CountryStates deltastatesYesChoice;
    CountryStates deltastatesNoChoice;
    double flipProgress = 1.0;
private:
    sf::Vector2f scale = { 1.f, 1.f };
    sf::Vector2f startPos;
    sf::Vector2f endPos;



    float  swapState = 1.f;
    float moveProgress = 1.f;

    float currentRotation = 0.f;
    float targetRotation = 0.f;

    sf::Sprite  cardSprite;
    TextBox     description;
    float descMargin;
    sf::IntRect cardTextureRect;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(cardSprite, states);

        if (swapState > 0.f)
            target.draw(description, states);
    }
};



struct GameState
{
    float doomsdayClockProgress = 0.f;
    //uint16_t interbellumDay = 0;

    CountryStates countries;
};

class VisualParameter : public sf::Drawable
{
public:
    VisualParameter(const sf::Texture& paramsTexture,
        const sf::Vector2f& position,
        sf::IntRect textureRect,
        const sf::Color& fillColor,
        const sf::Vector2f& fillPadding = { 0, 0 }) noexcept
        : icon(paramsTexture), padding(fillPadding)
    {
        icon.setTextureRect(textureRect);

        sf::Vector2f iconSize = { static_cast<float>(textureRect.size.x),
                                   static_cast<float>(textureRect.size.y) };
        setSize(iconSize);
        setPosition(position);
        shape.setFillColor(fillColor);
        shape.setOutlineThickness(0);
    }

    void setPosition(const sf::Vector2f& pos) noexcept
    {
        position = pos;
        icon.setPosition(pos);
        shape.setPosition({ pos.x, pos.y + size.y });
    }

    void setSize(const sf::Vector2f& size) noexcept
    {
        this->size = size;
        shape.setSize(size - sf::Vector2f{ padding.x * 2, padding.y * 2 });
        shape.setOrigin({ size.x / 2, size.y });
    }

    void setValue(float value) noexcept
    {
        oldValue = currentValue;
        newValue = std::clamp(value, 0.f, 1.f);
    }

    void update(double deltaTime) noexcept
    {
        if (std::abs(oldValue - newValue) > 0.001f)
            oldValue += (newValue - oldValue) * static_cast<float>(deltaTime) * animSpeed;
        else
            oldValue = newValue;

        if (std::abs(currentValue - newValue) > 0.5f)
            currentValue += (newValue - currentValue) * static_cast<float>(deltaTime) * animSpeed * 3.f;
        else
            currentValue = newValue;
    }

    void setRotation(float degrees) noexcept
    {
        shape.setRotation(sf::degrees(degrees));
        icon.setRotation(sf::degrees(degrees));
    }
    void InsertValue(float f)
    {
        currentValue  = f;
        oldValue = f;
        newValue = f;
    }
    float currentValue = 0.f;
    float oldValue = 0.f;
    float newValue = 0.f;
private:
    sf::Sprite            icon;
    mutable sf::RectangleShape shape;

    sf::Vector2f size = { 100, 100 };
    sf::Vector2f position = { 0, 0 };
    sf::Vector2f padding = { 0, 0 };

  

    float animSpeed = 2.f;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        const sf::Color color = shape.getFillColor();
        shape.setSize(icon.getGlobalBounds().size);

        auto drawSized = [&](float heightMult, sf::Color c)
            {
                shape.setFillColor(c);
                sf::Vector2f s = { size.x, (size.y - padding.y * 2) * heightMult };
                shape.setSize(s);
                shape.setOrigin({ 0, s.y + padding.y });
                target.draw(shape, states);
            };

        if (oldValue > currentValue)
        {
            drawSized(oldValue, sf::Color::White);
            drawSized(currentValue, color);
        }
        else
        {
            sf::Color trns = color;
            trns.a = 0.5f;

            drawSized(currentValue, trns);
            drawSized(oldValue, color);
        }

        target.draw(icon, states);
    }
};


class Damper : public sf::Drawable
{
public:
    Damper(const sf::Texture& texture,
        float width, float height,
        float damperSpeed, float damperSeconds)
        : damper({ width, height })
        , damperSpeed(damperSpeed)
        , damperSeconds(damperSeconds)
    {
        damper.setTexture(&texture);
        damper.setPosition({ 0.f, -damper.getGlobalBounds().size.y });
    }

    void swap()
    {
        currentPlayer = (currentPlayer + 1) % totalPlayers;
        clock.restart();
        state = AnimationState::In;
        damper.setPosition({ 0.f, -damper.getGlobalBounds().size.y });
    }

    void update(double deltaTime) noexcept
    {
        switch (state)
        {
        case AnimationState::Idle: break;

        case AnimationState::In:
            damper.move({ 0.f, damperSpeed * static_cast<float>(deltaTime) });
            if (damper.getPosition().y >= 0.f)
            {
                damper.setPosition({ 0.f, 0.f });
                state = AnimationState::Closed;
                clock.restart();
            }
            break;

        case AnimationState::Closed:
            if (clock.getElapsedTime().asSeconds() >= damperSeconds)
                state = AnimationState::Out;
            break;

        case AnimationState::Out:
            damper.move({ 0.f, damperSpeed * static_cast<float>(deltaTime) });
            if (damper.getPosition().y >= damper.getGlobalBounds().size.y)
            {
                damper.setPosition({ 0.f, damper.getGlobalBounds().size.y });
                state = AnimationState::Idle;
            }
            break;
        }
    }

    enum class AnimationState { Idle, In, Closed, Out } state = AnimationState::Idle;
    uint8_t            currentPlayer = 0;
    const uint8_t      totalPlayers = 3;
private:
    sf::RectangleShape damper;
    const float        damperSpeed;
    const float        damperSeconds;


    sf::Clock clock;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(damper, states);
    }
};


class Game
{
private:
public:
    Game() noexcept
    {

    }
    void loadResources(const std::filesystem::path resourcesPath)
    {

    }
};
static std::mt19937 engine = std::mt19937(time(0));
inline float Random(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine);
};
//enum class Country { SovietUnion, Germany, Britain};
int main()
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    std::filesystem::current_path("..");
    const sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    std::vector<sf::Drawable*> objects;
    int windowWidth = 1600;
    int windowHeight = 800;
    // Создание окна в полноэкранном режиме
    sf::RenderWindow window(sf::VideoMode({ (unsigned int)windowWidth, (unsigned int)windowHeight }),
        "Interbellum the game",
        sf::Style::Titlebar | sf::Style::Close, sf::State::Fullscreen);
    windowWidth = window.getSize().x;
    windowHeight = window.getSize().y;
    window.setFramerateLimit(60);

    sf::Font cardFont;
    sf::Texture backgroundTex, cardTex, paramsTex, uniqueParamsTex, damperTex, britainTex;
    try
    {
        backgroundTex.loadFromFile("resources\\background1.jpg");
        cardTex.loadFromFile("resources\\cards.png");
        paramsTex.loadFromFile("resources\\common_params.png");
        uniqueParamsTex.loadFromFile("resources\\unique_params.png");
        damperTex.loadFromFile("resources\\damper.png");


        britainTex.loadFromFile("resources\\britain_params.png");

        cardFont.openFromFile("resources\\Preciosa.ttf");
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Failed to init resources: " << ex.what() << std::flush;
        return -1;
    }

    sf::RectangleShape background({ static_cast<float>(windowWidth),
                                    static_cast<float>(windowHeight) });
    backgroundTex.setRepeated(true);
    background.setTexture(&backgroundTex);
    objects.push_back(&background);
    
    
    Damper damper(damperTex, windowWidth, windowHeight, windowHeight * 4.f, 0.3f);
    //objects.push_back(&damper);

    
    



    VisualParameter moralParam(paramsTex,
        sf::Vector2f{ (float)((windowWidth / 2.f) - 130), (float)(windowHeight - 120) },
        sf::IntRect({ 0, 0 }, { 110, 110 }),
        sf::Color::Green,
        { 5.f, 5.f });
    moralParam.setValue(0.8f);
    objects.push_back(&moralParam);

    VisualParameter influenceParam(paramsTex,
        sf::Vector2f{ (float)((windowWidth / 2.f) + 130), (float)(windowHeight - 120) },
        sf::IntRect({ 220, 0 }, { 110, 110 }),
        sf::Color::Blue,
        { 5.f, 5.f });
    influenceParam.setValue(0.8f);
    objects.push_back(&influenceParam);

    VisualParameter financeParam(paramsTex,
        sf::Vector2f{ (float)(windowWidth / 2.f), (float)(windowHeight - 120) },
        sf::IntRect({ 110, 0 }, { 110, 110 }),
        sf::Color::Yellow,
        { 5.f, 5.f });

    objects.push_back(&financeParam);
    financeParam.setValue(0.8f);



    VisualParameter ideologyWin(uniqueParamsTex,
        sf::Vector2f{ (float)(windowWidth / 2.f) - 500, (float)(windowHeight - 238) },
        sf::IntRect({ 0, 0 }, { 218, 218 }),
        sf::Color::Red,
        { 5.f, 5.f });

  //  objects.push_back(&ideologyWin);
    ideologyWin.setValue(1.f);
    VisualParameter miliatarWin(uniqueParamsTex,
        sf::Vector2f{ (float)(windowWidth / 2.f) - 500, (float)(windowHeight - 238) },
        sf::IntRect({ 218, 0 }, { 218, 218 }),
        sf::Color::Red,
        { 5.f, 5.f });

   // objects.push_back(&miliatarWin);
    miliatarWin.setValue(1.f);






    VisualParameter yellowChance(britainTex, { 800,800 }, sf::IntRect{ {0,0}, {80,218} }, sf::Color::Blue, { 0,0 });
    yellowChance.setValue(0.33f);


    yellowChance.setRotation(45);
   // objects.push_back(&yellowChance);
    GameState gameState;
    
    try
    {
        std::ifstream cardsJson;
        cardsJson.open("resources\\cards.json");

        nlohmann::json cards = nlohmann::json::parse(cardsJson);
        std::cout << cards.dump() << "\n";

        std::cout << cards["common_cards"][0]["effects"]["moral"] << std::endl;

    }
    catch (const nlohmann::json::exception& ex)
    {
        std::cerr << "Failed to parse cards.json: " << ex.what() << std::flush;
        return -1;
    }

    std::vector<Card> cards;


    for (int i = 0; i < 10; i++)
    {
        cards.push_back(Card(cardTex, cardFont, sf::IntRect({ 0, 0 }, { 450, 620 })));
        cards.back().setDescription(L"Индустриализация!Строим заводыыыыы и делаем трактораааааа, нужно больше зерна от крестьяяяяяяян");
    }

 //   cards.back().setDescription(L"Индустриализация! Строим заводыыыыы и делаем трактораааааа, нужно больше зерна от крестьяяяяяяян");
    CardModifiers currentIvent;
    Card* current = &cards[0];

    float moral = 1.f;
    sf::Clock clock;


    current->moveTo({ (float)(windowWidth / 2), (float)(windowHeight / 2) });
    bool cardSwiped = 0;
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        while (auto eventOpt = window.pollEvent())
        {
            auto& event = eventOpt.value();

            if (event.is<sf::Event::Closed>())
                window.close();

            if (auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2i mousePos = { mouseEvent->position.x, mouseEvent->position.y };

                if (mouseEvent->button == sf::Mouse::Button::Left)
                {
                    if(current->opened)
                    if (mouseEvent->position.x > windowWidth / 2)
                    {
                        current->moveTo({ (float)3000, (float)(windowHeight/2) });
                        current->swap();
                        gameState.countries.applyOther(current->deltastatesYesChoice, currentIvent);
                        cardSwiped = 1;
                        damper.swap();
                    }
                    else
                    {
                        current->moveTo({ (float)-3000, (float)(windowHeight / 2) });
                        current->swap();
                        gameState.countries.applyOther(current->deltastatesNoChoice, currentIvent);
                        cardSwiped = 1;
                        damper.swap();
                    }
                }
                else if (mouseEvent->button == sf::Mouse::Button::Right)
                {
                    if(!current->opened)
                        current->swap();
                    
                }
            }
            if (auto* mouseEvent = event.getIf<sf::Event::MouseMoved>())
            {
                if (mouseEvent->position.x > windowWidth / 2)
                    current->setRotation(10.f);
                else
                    current->setRotation(-10.f);
            }
            else if (auto* keyEvent = event.getIf<sf::Event::KeyReleased>())
            {
                if (keyEvent->code == sf::Keyboard::Key::Space) damper.swap();
              //  else if (keyEvent->code == sf::Keyboard::Key::E) current->setRotation(10.f);
              //  else if (keyEvent->code == sf::Keyboard::Key::Q) current->setRotation(-10.f);
                else if (keyEvent->code == sf::Keyboard::Key::Escape) window.close();
            }
        }
        if (cardSwiped)
        {
            if (damper.state == Damper::AnimationState::Idle)
            {
                cardSwiped = 0;
                current->teleport({ 0, 0 });
                current = &cards[(int)Random(0, cards.size())];
                current->moveTo({ (float)(windowWidth / 2), (float)(windowHeight / 2) });
                if (damper.currentPlayer == 0)
                {
                    moralParam.InsertValue(gameState.countries.s_moral);
                    financeParam.InsertValue(gameState.countries.s_finance);
                    moralParam.InsertValue(gameState.countries.s_moral);
                    moralParam.InsertValue(gameState.countries.s_moral);
                }





            }
        }
       // current->update(deltaTime, 4.f);
        damper.update(deltaTime);
        moralParam.update(deltaTime);
        influenceParam.update(deltaTime);
        financeParam.update(deltaTime);

        yellowChance.update(deltaTime);

        window.clear();
        for (sf::Drawable* dr : objects)
            window.draw(*dr);


        current->update(deltaTime, 4.f);
 

        window.draw(*current);
        window.draw(damper);
        window.display();
    }

    return 0;
}