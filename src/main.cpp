#include <SFML/Graphics.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <windows.h>
#include <random>

/*
class Static
{
public:
    static inline int windowWidth = 1600, windowHeight = 800, mousex, mousey;
};
*/
static inline int windowWidth = 1600, windowHeight = 800, mousex, mousey;
static inline sf::Font specialFont;
static inline sf::Font cardFont;
static inline sf::Texture backgroundTex, cardTex, specialcardTex, paramsTex, uniqueParamsTex, damperTex, britainTex, SpecialBackgroundTex;
static inline std::vector<sf::Texture> doomClockTexs;
static inline std::vector<sf::Drawable*> objects;
enum class Winner
{
    None,
    USSR,
    Germany,
    Britain

};
struct CardModifiers
{
    float    s_ideology = 1.f;
    float    s_finance = 1.f;
    float    s_moral = 1.f;
    float    s_influence = 1.f;
    float    s_power = 1.f;

    float    g_finance = 1.f;
    float    g_moral = 1.f;
    float    g_influence = 1.f;
    float    g_power = 1.f;

    float    b_agentNet = 1.f;
    float    b_redChance = 1.f;
    float    b_yellowChance = 1.f;
    float    b_greenChance = 1.f;

    float    doomsdayClockProgress = 1.f;

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
    float    s_ideology = 0.25f;
    float      s_finance = 0.15f;
    float    s_moral = 0.75f;
    float    s_influence = 0.20f;
    float    s_power = 0.20f;

    float      g_finance = 0.35;
    float    g_moral = 0.40f;
    float    g_influence = 0.20f;
    float    g_power = 0.20f;

    float    b_agentNet = 0.25f;
    float    b_redChance = 0.25f;
    float    b_yellowChance = 0.25f;
    float    b_greenChance = 0.25f;

    float doomsdayClockProgress = 0.f;

    void applyOther(const CountryStates& other, const CardModifiers& modifiers = {})
    {
        s_ideology += other.s_ideology * modifiers.s_ideology;
        s_finance += other.s_finance * modifiers.s_finance;
        s_moral += other.s_moral * modifiers.s_moral;
        s_influence += other.s_influence * modifiers.s_influence;
     //   s_power += other.s_power * modifiers.s_power;
        s_power += other.s_power * modifiers.s_power * ((s_finance + 1) / 2.f);

        g_finance += other.g_finance * modifiers.g_finance;
        g_moral += other.g_moral * modifiers.g_moral;
        g_influence += other.g_influence * modifiers.g_influence;
       // g_power += other.g_power * modifiers.g_power;
        g_power += other.g_power * modifiers.g_power * ((g_finance + 1) / 2.f);

        b_agentNet += other.b_agentNet * modifiers.b_agentNet;
        b_redChance += other.b_redChance * modifiers.b_redChance;
        b_yellowChance += other.b_yellowChance * modifiers.b_yellowChance;
        b_greenChance += other.b_greenChance * modifiers.b_greenChance;

        if (s_ideology > 1) s_ideology = 1; if (s_ideology < 0) s_ideology = 0;
        if (s_finance > 1) s_finance = 1; if (s_finance < 0) s_finance = 0;
        if (s_moral > 1) s_moral = 1; if (s_moral < 0) s_moral = 0;
        if (s_influence > 1) s_influence = 1; if (s_influence < 0) s_influence = 0;
        if (s_power > 1) s_power = 1; if (s_power < 0) s_power = 0;
        if (g_finance > 1) g_finance = 1; if (g_finance < 0) g_finance = 0; 
        if (g_moral > 1) g_moral = 1; if (g_moral < 0) g_moral = 0; 
        if (g_influence > 1) g_influence = 1; if (g_influence < 0) g_influence = 0; 
        if (g_power > 1) g_power = 1; if (g_power < 0) g_power = 0; 
        if (b_agentNet > 1) b_agentNet = 1; if (b_agentNet < 0) b_agentNet = 0; 
        if (b_redChance > 1) b_redChance = 1; if (b_redChance < 0) b_redChance = 0; 
        if (b_yellowChance > 1) b_yellowChance = 1; if (b_yellowChance < 0) b_yellowChance = 0; 
        if (b_greenChance > 1) b_greenChance = 1; if (b_greenChance < 0) b_greenChance = 0; 
        doomsdayClockProgress += other.doomsdayClockProgress * modifiers.doomsdayClockProgress;
    }
};

struct GameState
{
    //uint16_t interbellumDay = 0;
    bool warStarted = false;
    CountryStates countries;

    Winner determineWinner() noexcept
    {
        const float POWER_THRESHOLD = 0.7f; //Порог силы
        const float POWER_DIF = 10.f; //Разница силы СССР и Германии
        const float IDEOLOGY_THRESHOLD = 1.f; //Порог идеологии для мировой пролетарской революции (победа СССР)

        if (warStarted)
        {
            if (countries.s_power < POWER_THRESHOLD && countries.g_power < POWER_THRESHOLD)
                return Winner::Britain;

            if (abs(countries.s_power - countries.g_power) <= POWER_DIF)
                return Winner::Britain;

            if (countries.s_power < countries.g_power)
                return Winner::Germany;
            else
                return Winner::USSR;
        }
        else if (countries.s_ideology >= IDEOLOGY_THRESHOLD)
            return Winner::USSR;

        //Победитель не определён в текущем ходе
        return Winner::None;
    }
};


static inline GameState gameState;

sf::String systemToSfString(const std::string& str) {
    if (str.empty()) return sf::String();

    // Определяем размер необходимого буфера
    int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);

    // Конвертируем в WideChar (UTF-16)
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

    return sf::String(wstrTo);
}
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
        description.setBounds({ bounds.x - 2 * descMargin, bounds.y / 2.f });
        setScale({ 1.0f, 1.0f });

    }
    bool containsPoint(sf::Vector2f point) noexcept
    {
        return cardSprite.getGlobalBounds().contains(point);
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
            sf::IntRect({ cardTextureRect.size.x * (cardType), cardVariant * cardTextureRect.size.y },
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
    sf::IntRect cardTextureRect;
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

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(cardSprite, states);

        if (swapState > 0.f)
            target.draw(description, states);
    }
};




class VisualParameter : public sf::Drawable
{
public:
    VisualParameter(const sf::Texture& paramsTexture,
        const sf::Vector2f& position,
        sf::IntRect textureRect,
        const sf::Color& fillColor,
        const sf::Vector2f& fillPadding, float& value) noexcept
        : icon(paramsTexture), padding(fillPadding),
        currentValue(value)
    {

        icon.setTextureRect(textureRect);

        sf::Vector2f iconSize = { static_cast<float>(textureRect.size.x),
                                   static_cast<float>(textureRect.size.y) };
        setSize(iconSize);
        setPosition(position);
        shape.setFillColor(fillColor);
        shape.setOutlineThickness(0);

        pointShape.setRadius(10);
        pointShape.setFillColor(sf::Color::Black);
    }
    //*
    void setPosition(const sf::Vector2f& pos) noexcept
    {
        position = pos;
        icon.setPosition(pos);
        shape.setPosition({ pos.x, pos.y + size.y });
        pointShape.setPosition(pos + sf::Vector2f{ (size.x/2), (size.y/2)});
        pointShape.setOrigin({ 0, (size.y * 0.75f) });
    }
//*/
    void setSize(const sf::Vector2f& size) noexcept
    {
        this->size = size;
        shape.setSize(size - sf::Vector2f{ padding.x * 2, padding.y * 2 });
       // shape.setOrigin({ size.x / 2, size.y });
    }

    void setValue(float value) noexcept
    {
        /*
        oldValue = currentValue;
        newValue = std::clamp(value, 0.f, 1.f);
        */
    }

    void update(double deltaTime) noexcept
    {
        constexpr float delta = 30.f;
        oldValue += (currentValue - oldValue) / delta;
        if (abs((currentValue - oldValue)) < 1 / delta)
            oldValue = currentValue;
        /*
        if (std::abs(oldValue - newValue) > 0.001f)
            oldValue += (newValue - oldValue) * static_cast<float>(deltaTime) * animSpeed;
        else
            oldValue = newValue;

        if (std::abs(currentValue - newValue) > 0.5f)
            currentValue += (newValue - currentValue) * static_cast<float>(deltaTime) * animSpeed * 3.f;
        else
            currentValue = newValue;
            */
    }
    void setRotation(float degrees) noexcept
    {
        rotation = degrees;
    }
    /*
    void setRotation(float degrees) noexcept
    {
        float rotation = degrees;
        shape.setRotation(sf::degrees(degrees));
        icon.setRotation(sf::degrees(degrees));
        
        pointShape.setRotation(sf::degrees(degrees));
    }//*/
    void InsertValue(float f)
    {
        /*
        currentValue = std::clamp(f, 0.f, 1.f);
        oldValue = currentValue;
        newValue = currentValue;
        */
    }
    float& currentValue;
    float oldValue = 0;
    float& newValue = currentValue;
    sf::Vector2f scale = { 1.f, 1.f };
   // float oldValue = 0.f;
  //  float newValue = 0.f;
    bool shown = 1;
    bool hasPoint = 1;
//private:
    sf::Sprite            icon;
    mutable sf::RectangleShape shape;
    sf::CircleShape pointShape;
    sf::Vector2f size = { 100, 100 };
    sf::Vector2f position = { 0, 0 };
    sf::Vector2f padding = { 0, 0 };
    float rotation = 0.f;
   
    float animSpeed = 2.f;
   
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (!shown)
            return;
        const sf::Color color = shape.getFillColor();

        // Поворот вокруг центра иконки
        sf::Transform transform;
        sf::Vector2f center = {
            position.x + size.x * scale.x / 2.f,
            position.y + size.y * scale.y / 2.f
        };
        transform.translate(center);
        transform.rotate(sf::degrees(rotation));
        transform.translate(-center);

        sf::RenderStates rotatedStates = states;
        rotatedStates.transform *= transform;

        auto drawSized = [&](float heightMult, sf::Color c)
            {
                shape.setFillColor(c);
                sf::Vector2f s = {
                    (size.x - padding.x * 2) * scale.x,
                    (size.y - padding.y * 2) * scale.y * heightMult
                };
                shape.setSize(s);
                shape.setOrigin({ 0.f, s.y });
                shape.setPosition({
                    position.x + padding.x * scale.x,
                    position.y + size.y * scale.y - padding.y * scale.y
                    });
                shape.setRotation(sf::degrees(0));
                target.draw(shape, rotatedStates);
            };

        if (oldValue > currentValue)
        { 
            if (oldValue != currentValue)
            {
                int e = 0;
                e++;
            }
            drawSized(oldValue, sf::Color::White);
            drawSized(currentValue, color);
           
        }
        else
        {
            
            sf::Color trns = color;
            trns.a = 128;
            drawSized(currentValue, trns);
            drawSized(oldValue, color);
        }

        target.draw(icon, rotatedStates);
        if (hasPoint)
            target.draw(pointShape, rotatedStates);
        /*
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
        

        /*
        sf::Vector2f s = { size.x, (size.y - padding.y * 2) * currentValue };
        shape.setSize(s);
        shape.setPosition(icon.getPosition());
        target.draw(shape, states);
        
        target.draw(icon, states);
        if(hasPoint)
            target.draw(pointShape, states);
            */
    }
};
class SpecialCardDeck : public sf::Drawable
{
public:
    SpecialCardDeck(const sf::Texture& Texture,
        const sf::Vector2f& position,
        sf::IntRect textureRect,
        const sf::Vector2f& fillPadding = { 0, 0 }) noexcept
        : background(Texture), padding(fillPadding)
    {
        background.setTextureRect(textureRect);

        sf::Vector2f backgroundSize = { static_cast<float>(textureRect.size.x),
                                   static_cast<float>(textureRect.size.y) };
        setPosition(position);
    }

    void setPosition(const sf::Vector2f& pos) noexcept
    {
        position = pos;
        background.setPosition(pos);
        //shape.setPosition({ pos.x, pos.y + size.y });
    }

    void setSize(const sf::Vector2f& size) noexcept
    {
        this->size = size;
        background.setScale(size);
       // shape.setSize(size - sf::Vector2f{ padding.x * 2, padding.y * 2 });
       // shape.setOrigin({ size.x / 2, size.y });
    }

    bool containsPoint(sf::Vector2f point) noexcept
    {
        return background.getGlobalBounds().contains(point);
    }

    void update(double deltaTime, int x, int y) noexcept
    {
        //containsPoint({ (float)x,(float)y }) && 
        bool hov = containsPoint({ (float)x,(float)y });
        if (cards.size())
        {
            float sizebx = background.getTextureRect().size.x;
            float sizecx = cards[0].cardTextureRect.size.x;
            int i = 0;
            for (Card& c : cards)
            {
                int curent = (int)((((float)(x - position.x)) / sizebx) * cards.size());
                bool b = curent == i && hov;
                bool cur = curent < i && hov;
                c.moveTo({ (float)((windowWidth)-710 + (sizecx / 2) + (i * ((sizebx - sizecx) / (float)cards.size()))) + (cur ? 90.f : 0.f), (float)((windowHeight)-(410 / 2) - 50) + (b ? -120.f : 0.f)});
                i++;
            }
        }
        for (Card& c : cards)
        {
            c.update(deltaTime, 1);
        }
    }

    bool shown = 1;
    std::vector<Card> cards;
private:
    sf::Sprite background;

    sf::Vector2f size = { 100, 100 };
    sf::Vector2f position = { 0, 0 };
    sf::Vector2f padding = { 0, 0 };
    void AddCard()
    {

    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (!shown)
            return;
        for(int i = 0; i < cards.size(); i++)
            target.draw(cards[i], states);
        target.draw(background, states);
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

    void lock() noexcept
    {

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
            if (clock.getElapsedTime().asSeconds() >= damperSeconds && !locked)
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
    bool locked = false;
    sf::RectangleShape damper;
    const float        damperSpeed;
    const float        damperSeconds;


    sf::Clock clock;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(damper, states);
    }
};



static std::mt19937 engine = std::mt19937(time(0));
inline float Random(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine);
};
//enum class Country { SovietUnion, Germany, Britain};





sf::Color brighten(const sf::Color& source, float factor) noexcept
{
    return sf::Color(std::min(255, static_cast<int>(source.r * factor)),
        std::min(255, static_cast<int>(source.g * factor)),
        std::min(255, static_cast<int>(source.b * factor)),
        source.a);
}
class Button : public sf::Drawable
{

public:
    explicit Button(const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Color& fillColor, std::function<void()> callback) noexcept
        : fillColor(fillColor), callback(callback)
    {
        shape.setPosition(pos);
        shape.setSize(size);
        shape.setOrigin({ shape.getLocalBounds().size.x / 2 , shape.getLocalBounds().size.y / 2 });

        shape.setFillColor(fillColor);
        shape.setOutlineThickness(2.f);

    }

    void hover() noexcept
    {

        shape.setFillColor(brighten(fillColor, 1.2f));

        hovered = true;
    }
    void unhover() noexcept
    {
        shape.setFillColor(fillColor);
        hovered = false;
    }
    bool containsPoint(sf::Vector2f point) noexcept
    {
        return shape.getGlobalBounds().contains(point);
    }
    void click() const noexcept
    {
        if (visibility && hovered)
            callback();
    }

    void setVisible(bool visible) noexcept
    {
        visibility = visible;
    }
    bool isVisible() const noexcept
    {
        return visibility;
    }
private:
    bool visibility = true;
    bool hovered = false;
    std::function<void()> callback;

    sf::Color fillColor;
    sf::RectangleShape shape;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (visibility)
            target.draw(shape, states);
    }
};
class PlayerInterface
{
public:
    enum class State {left, rigth, center};
    State state;
};
class SovietInterface : public sf::Drawable, public PlayerInterface
{
public:
    VisualParameter moralParam;
    VisualParameter influenceParam;
    VisualParameter financeParam;
    VisualParameter ideologyWin;
    VisualParameter miliatarPower;
    SpecialCardDeck specialCardDeckS;
   
    SovietInterface() : moralParam(paramsTex,
            sf::Vector2f{ static_cast<float>(windowWidth / 2.f - 130 - 55), static_cast<float>(windowHeight - 120) },
            sf::IntRect({ 0, 0 }, { 110, 110 }),
            sf::Color::Green,
            { 5.f, 5.f }, gameState.countries.s_moral), influenceParam(paramsTex,
            sf::Vector2f{ (float)((windowWidth / 2.f) + 130 - 55), (float)(windowHeight - 120) },
            sf::IntRect({ 220, 0 }, { 110, 110 }),
            sf::Color::Blue,
            { 5.f, 5.f }, gameState.countries.s_influence), financeParam(paramsTex,
            sf::Vector2f{ (float)((windowWidth / 2.f) - 55), (float)(windowHeight - 120) },
            sf::IntRect({ 110, 0 }, { 110, 110 }),
            sf::Color::Yellow,
            { 5.f, 5.f }, gameState.countries.s_finance), ideologyWin(uniqueParamsTex,
            sf::Vector2f{ (float)(windowWidth / 2.f) - 600 - 55, (float)(windowHeight - 238) },
            sf::IntRect({ 0, 0 }, { 218, 218 }),
            sf::Color::Red,
            { 5.f, 5.f }, gameState.countries.s_ideology),
        miliatarPower(uniqueParamsTex,
            sf::Vector2f{ (float)(windowWidth / 2.f) - 360 - 55, (float)(windowHeight - 238) },
            sf::IntRect({ 218, 0 }, { 218, 218 }),
            sf::Color::Red,
            { 5.f, 5.f }, gameState.countries.s_power),
        specialCardDeckS(SpecialBackgroundTex,
            sf::Vector2f{ (float)((windowWidth)-710), (float)((windowHeight)-410) },
            sf::IntRect({ 0, 0 }, { 700, 400 }),
            { 5.f, 5.f })
    {
       
        for (int i = 0; i < 5; i++)
        {
            // 0 - soviet, 1 - german, 3 - brittish red, 4 - brittish yellow, 5 - brittish green
            int type = 0;
            specialCardDeckS.cards.push_back(Card(specialcardTex, cardFont, sf::IntRect({ 0, 0 }, { 374, 396 })));
            specialCardDeckS.cards.back().cardType = type;
            specialCardDeckS.cards.back().setScale({ 1.f,  1.f });
            specialCardDeckS.cards.back().teleport(sf::Vector2f{ (float)((windowWidth)-(710 / 2)), (float)((windowHeight)-(410 / 2)) });
        }
    }
    void SetCenter()
    {
        moralParam.setPosition(sf::Vector2f{ static_cast<float>(windowWidth / 2.f - 130 - 55), static_cast<float>(windowHeight - 120) });
        influenceParam.setPosition(sf::Vector2f{ (float)((windowWidth / 2.f) + 130 - 55), (float)(windowHeight - 120) });
        financeParam.setPosition(sf::Vector2f{ (float)((windowWidth / 2.f) - 55), (float)(windowHeight - 120) });
        ideologyWin.setPosition(sf::Vector2f{ (float)(windowWidth / 2.f) - 600 - 55, (float)(windowHeight - 238) });
        miliatarPower.setPosition(sf::Vector2f{ (float)(windowWidth / 2.f) - 360 - 55, (float)(windowHeight - 238) });
        
        moralParam.setRotation(0);
        influenceParam.setRotation(0);
        financeParam.setRotation(0);
        ideologyWin.setRotation(0);
        miliatarPower.setRotation(0);
        state = State::center;
    }
    void SetLeft()
    {
        moralParam.setPosition(sf::Vector2f{ static_cast<float>(0), static_cast<float>(100) });
        influenceParam.setPosition(sf::Vector2f{ (float)(120), (float)(50) });
        financeParam.setPosition(sf::Vector2f{ (float)(240), (float)(0) });
        miliatarPower.setPosition(sf::Vector2f{ (float)(400), (float)(50) });

        ideologyWin.setPosition(sf::Vector2f{ (float)(50), (float)(250) });

        moralParam.setRotation(150);
        influenceParam.setRotation(150);
        financeParam.setRotation(150);
        ideologyWin.setRotation(150);
        miliatarPower.setRotation(150);
        state = State::rigth;
    }
    void SetRight()
    {
        moralParam.setPosition(sf::Vector2f{ static_cast<float>(windowWidth - 150), static_cast<float>(100) });
        influenceParam.setPosition(sf::Vector2f{ (float)(windowWidth - 270), (float)(50) });
        financeParam.setPosition(sf::Vector2f{ (float)(windowWidth - 390), (float)(0) });
        miliatarPower.setPosition(sf::Vector2f{ (float)(windowWidth - 650), (float)(50) });

        ideologyWin.setPosition(sf::Vector2f{ (float)(windowWidth - 300), (float)(250) });

        moralParam.setRotation(-150);
        influenceParam.setRotation(-150);
        financeParam.setRotation(-150);
        ideologyWin.setRotation(-150);
        miliatarPower.setRotation(-150);
        state = State::left;
    }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        //*
        target.draw(moralParam, states);
        target.draw(influenceParam, states);
        target.draw(financeParam, states);
        target.draw(ideologyWin, states);
        target.draw(miliatarPower, states);
        if(state == State::center)
        target.draw(specialCardDeckS, states);
        //*/
    }
    void update(double deltatime)
    {
        //*
        moralParam.update(deltatime);
        influenceParam.update(deltatime);
        financeParam.update(deltatime);
        ideologyWin.update(deltatime);
        miliatarPower.update(deltatime);
        if(state == State::center)
        specialCardDeckS.update(deltatime, mousex, mousey);
        //*/
    }
};
class GermanInterface : public sf::Drawable, public PlayerInterface
{
public:
    VisualParameter moralParam;
    VisualParameter influenceParam;
    VisualParameter financeParam;
    VisualParameter miliatarPower;
    SpecialCardDeck specialCardDeckG;
    GermanInterface() : moralParam(paramsTex,
        sf::Vector2f{ static_cast<float>(windowWidth / 2.f - 130 - 55), static_cast<float>(windowHeight - 120) },
        sf::IntRect({ 0, 0 }, { 110, 110 }),
        sf::Color::Green,
        { 5.f, 5.f }, gameState.countries.g_moral), 
        influenceParam(paramsTex,
            sf::Vector2f{ (float)((windowWidth / 2.f) + 130 - 55), (float)(windowHeight - 120) },
            sf::IntRect({ 220, 0 }, { 110, 110 }),
            sf::Color::Blue,
            { 5.f, 5.f }, gameState.countries.g_influence), 
        financeParam(paramsTex,
                sf::Vector2f{ (float)((windowWidth / 2.f) - 55), (float)(windowHeight - 120) },
                sf::IntRect({ 110, 0 }, { 110, 110 }),
                sf::Color::Yellow, { 5.f, 5.f }, gameState.countries.g_finance),
        miliatarPower(uniqueParamsTex,
            sf::Vector2f{ (float)(windowWidth / 2.f) - 360 - 55, (float)(windowHeight - 238) },
            sf::IntRect({ 218, 0 }, { 218, 218 }),
            sf::Color::Red,
            { 5.f, 5.f }, gameState.countries.g_power),
        specialCardDeckG(SpecialBackgroundTex,
            sf::Vector2f{ (float)((windowWidth)-710), (float)((windowHeight)-410) },
            sf::IntRect({ 0, 0 }, { 700, 400 }),
            { 5.f, 5.f })
    {

        for (int i = 0; i < 5; i++)
        {
            int type = 1;
            specialCardDeckG.cards.push_back(Card(specialcardTex, cardFont, sf::IntRect({ 0, 0 }, { 374, 396 })));
            specialCardDeckG.cards.back().cardType = type;
            specialCardDeckG.cards.back().setScale({ 1.f,  1.f });
            specialCardDeckG.cards.back().teleport(sf::Vector2f{ (float)((windowWidth)-(710 / 2)), (float)((windowHeight)-(410 / 2)) });
        }
    }
    void SetCenter()
    {
        moralParam.setPosition(sf::Vector2f{ static_cast<float>(windowWidth / 2.f - 130 - 55), static_cast<float>(windowHeight - 120) });
        influenceParam.setPosition(sf::Vector2f{ (float)((windowWidth / 2.f) + 130 - 55), (float)(windowHeight - 120) });
        financeParam.setPosition(sf::Vector2f{ (float)((windowWidth / 2.f) - 55), (float)(windowHeight - 120) });
        miliatarPower.setPosition(sf::Vector2f{ (float)(windowWidth / 2.f) - 360 - 55, (float)(windowHeight - 238) });

        moralParam.setRotation(0);
        influenceParam.setRotation(0);
        financeParam.setRotation(0);
        miliatarPower.setRotation(0);
        state = State::center;
    }
    void SetLeft()
    {
        moralParam.setPosition(sf::Vector2f{ static_cast<float>(0), static_cast<float>(150) });
        influenceParam.setPosition(sf::Vector2f{ (float)(120), (float)(100) });
        financeParam.setPosition(sf::Vector2f{ (float)(240), (float)(50) });
        miliatarPower.setPosition(sf::Vector2f{ (float)(400), (float)(50)});


        moralParam.setRotation(150);
        influenceParam.setRotation(150);
        financeParam.setRotation(150);
        miliatarPower.setRotation(150);
        state = State::rigth;
    }
    void SetRight()
    {
        moralParam.setPosition(sf::Vector2f{ static_cast<float>(windowWidth - 100), static_cast<float>(100) });
        influenceParam.setPosition(sf::Vector2f{ (float)(windowWidth - 220), (float)(50) });
        financeParam.setPosition(sf::Vector2f{ (float)(windowWidth - 340), (float)(0) });
        miliatarPower.setPosition(sf::Vector2f{ (float)(windowWidth - 550), (float)(50) });


        moralParam.setRotation(-150);
        influenceParam.setRotation(-150);
        financeParam.setRotation(-150);
        miliatarPower.setRotation(-150);
        state = State::left;
    }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(moralParam, states);
        target.draw(influenceParam, states);
        target.draw(financeParam, states);
        target.draw(miliatarPower, states);
        if (state == State::center)
            target.draw(specialCardDeckG, states);
    }

    void update(double deltatime)
    {
        moralParam.update(deltatime);
        influenceParam.update(deltatime);
        financeParam.update(deltatime);
        miliatarPower.update(deltatime);
        if (state == State::center)
        specialCardDeckG.update(deltatime, mousex, mousey);
    }
};
class BrittishInterface : public sf::Drawable, public PlayerInterface
{
public:
    VisualParameter yellowChance;

    VisualParameter redChance;

    VisualParameter greenChance;
    SpecialCardDeck specialCardDeckB;
    Button buyYellowCard;

    Button buyRedCard;

    Button buyGreenCard;
    BrittishInterface() : yellowChance(britainTex, { (float)(windowWidth / 2) - 500,(float)(windowHeight - 270) }, sf::IntRect{ {0,0}, {80,218} }, sf::Color::Yellow, { 0,0 }, gameState.countries.b_yellowChance),
     redChance(britainTex, { (float)(windowWidth / 2) - 500 + 85,(float)(windowHeight - 270) }, sf::IntRect{ {80,0}, {80,218} }, sf::Color::Red, { 0,0 }, gameState.countries.b_redChance),
        greenChance(britainTex, { (float)(windowWidth / 2) - 500 + 85 * 2,(float)(windowHeight - 270) }, sf::IntRect{ {160,0}, {80,218} }, sf::Color::Green, { 0,0 }, gameState.countries.b_greenChance),
        specialCardDeckB(SpecialBackgroundTex,
            sf::Vector2f{ (float)((windowWidth)-710), (float)((windowHeight)-410) },
            sf::IntRect({ 0, 0 }, { 700, 400 }),
            { 5.f, 5.f }),
        buyYellowCard({ (float)(windowWidth / 2) - 460 + 320,(float)(windowHeight - 161) }, { 96,218 }, { 200,200,50 }, [&]() {
            }),
            buyRedCard({ (float)(windowWidth / 2) - 460 + 320 + 90,(float)(windowHeight - 161) }, { 96,218 }, { 200,50,50 }, [&]() {
        gameState.countries.doomsdayClockProgress += 0.1f;
                }),
        buyGreenCard({ (float)(windowWidth / 2) - 460 + 320 + 90 * 2,(float)(windowHeight - 161) }, { 96,218 }, { 50,200,50 }, [&]() {
            })
    {
        yellowChance.setValue(0.33f);
        redChance.setValue(0.33f);
        greenChance.setValue(0.33f);

        for (int i = 0; i < 5; i++)
        {
            specialCardDeckB.cards.push_back(Card(specialcardTex, cardFont, sf::IntRect({ 0, 0 }, { 374, 396 })));
            specialCardDeckB.cards.back().cardType = Random(2, 5);
            specialCardDeckB.cards.back().setScale({ 1.f,  1.f });
            specialCardDeckB.cards.back().teleport(sf::Vector2f{ (float)((windowWidth)-(710 / 2)), (float)((windowHeight)-(410 / 2)) });
        }
    }
    void SetCenter()
    {
        yellowChance.setPosition(sf::Vector2f{ (float)(windowWidth / 2) - 500,(float)(windowHeight - 270) });
        redChance.setPosition(sf::Vector2f{ (float)(windowWidth / 2) - 500 + 85,(float)(windowHeight - 270) });
        greenChance.setPosition(sf::Vector2f{ (float)(windowWidth / 2) - 500 + 85 * 2,(float)(windowHeight - 270) });

        yellowChance.setRotation(0);
        redChance.setRotation(0);
        greenChance.setRotation(0);
        state = State::center;
    }
    void SetLeft()
    {
        yellowChance.setPosition(sf::Vector2f{ static_cast<float>(50), static_cast<float>(150) });
        redChance.setPosition(sf::Vector2f{ static_cast<float>(150), static_cast<float>(100) });
        greenChance.setPosition(sf::Vector2f{ static_cast<float>(250), static_cast<float>(50) });

        yellowChance.setRotation(150);
        redChance.setRotation(150);
        greenChance.setRotation(150);
        state = State::rigth;
    }
    void SetRight()
    {
        yellowChance.setPosition(sf::Vector2f{ static_cast<float>(windowWidth - 150), static_cast<float>(100) });
        redChance.setPosition(sf::Vector2f{ static_cast<float>(windowWidth - 250), static_cast<float>(50) });
        greenChance.setPosition(sf::Vector2f{ static_cast<float>(windowWidth - 350), static_cast<float>(0) });

        yellowChance.setRotation(-150);
        redChance.setRotation(-150);
        greenChance.setRotation(-150);
        state = State::left;
    }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(yellowChance, states);
        target.draw(redChance, states);
        target.draw(greenChance, states);
        if (state == State::center)
        {
            target.draw(specialCardDeckB, states);
            target.draw(buyGreenCard, states);
            target.draw(buyRedCard, states);
            target.draw(buyYellowCard, states);
        }
    }

    void update(double deltatime)
    {
        if (buyYellowCard.containsPoint({(float)mousex, (float)mousey})) buyYellowCard.hover();
        else buyYellowCard.unhover();

        if (buyRedCard.containsPoint({ (float)mousex, (float)mousey })) buyRedCard.hover();
        else buyRedCard.unhover();

        if (buyGreenCard.containsPoint({ (float)mousex, (float)mousey })) buyGreenCard.hover();
        else buyGreenCard.unhover();
        yellowChance.update(deltatime);
        redChance.update(deltatime);
        greenChance.update(deltatime);
        if (state == State::center)
        specialCardDeckB.update(deltatime,mousex, mousey);
    }
};
int main()
{
    setlocale(1, "ru");
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    std::filesystem::current_path("..");
//    Static st;
  //  int& windowWidth = st.windowWidth;
  //  int& windowHeight = st.windowHeight;
    // Создание окна в полноэкранном режиме
    sf::RenderWindow window(sf::VideoMode({ (unsigned int)windowWidth, (unsigned int)windowHeight }),
        "Interbellum the game",
        sf::Style::Titlebar | sf::Style::Close, sf::State::Fullscreen);
    windowWidth = window.getSize().x;
    windowHeight = window.getSize().y;
    window.setFramerateLimit(60);

 //   sf::Font specialFont;

 //   sf::Font cardFont;
//    sf::Texture backgroundTex, cardTex, specialcardTex, paramsTex, uniqueParamsTex, damperTex, britainTex, SpecialBackgroundTex;
 //   std::vector<sf::Texture> doomClockTexs;
    doomClockTexs.resize(12);
    std::vector<sf::Sprite> doomClockStates;
    // doomClockStates.resize(12);
    try
    {
        for (int i = 0; i < 12; i++)
        {
            if (!doomClockTexs[i].loadFromFile("resources\\clock" + std::to_string(i + 1) + ".png"))
            {
                return -1;
            }
            doomClockStates.emplace_back(sf::Sprite(doomClockTexs[i], sf::IntRect({ 0,0 }, { 165, 165 })));
            // doomClockStates[i].setTexture(doomClockTexs[i]);
            doomClockStates[i].setPosition({ (float)(30), (float)(windowHeight - 170) });
        }
        // sf::Texture texture;
         /*
         // Загружаем изображение из файла
         if (!texture.loadFromFile("image.png")) {
             // Если файл не найден или поврежден, SFML выведет ошибку в консоль
             return -1;
         }
         doomClockStates[0].
             //*/
        backgroundTex.loadFromFile("resources\\background1.jpg");
        cardTex.loadFromFile("resources\\cards.png");
        specialcardTex.loadFromFile("resources\\SpecialCards.png");
        paramsTex.loadFromFile("resources\\common_params.png");
        uniqueParamsTex.loadFromFile("resources\\unique_params.png");
        damperTex.loadFromFile("resources\\damper.png");


        britainTex.loadFromFile("resources\\britain_params.png");
        SpecialBackgroundTex.loadFromFile("resources\\SpecialBackground.png");

        cardFont.openFromFile("resources\\Metamorphous.ttf");
        specialFont.openFromFile("resources\\Share-Tech-CYR-Bold.otf");
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

    //Занавес для переключения игрока и завершения игры
    Damper damper(damperTex, windowWidth, windowHeight, windowHeight * 4.f, 0.3f);


    std::vector<Card> sovietcards;
    std::vector<Card> germancards;
    std::vector<Card> brittishcards;
    try
    {
        std::ifstream cardsJson;
        cardsJson.open("resources\\cards.json");

        nlohmann::json data = nlohmann::json::parse(cardsJson);
        for (const auto& card : data["common_cards"])
        {
            int type = card["type"];
            if (type == 1)
                sovietcards.push_back(Card(cardTex, cardFont, sf::IntRect({ 0, 0 }, { 450, 620 })));
            if (type == 2)
                germancards.push_back(Card(cardTex, cardFont, sf::IntRect({ 0, 0 }, { 450, 620 })));
            if (type == 3)
                brittishcards.push_back(Card(cardTex, cardFont, sf::IntRect({ 0, 0 }, { 450, 620 })));
            Card& cur = (type == 1 ? sovietcards.back() : (type == 2 ? germancards.back() : brittishcards.back()));
            std::string s = card["description"].get<std::string>();
            cur.setDescription(sf::String::fromUtf8(s.begin(), s.end()));

            cur.cardType = ((card["type"]) % 3);
            //  cur.cardType = card["type"];

              // Доступ к эффектам при выборе "Да" (свайп вправо)

            cur.deltastatesYesChoice.s_moral = (float)(card["effectsYes"]["s_moral"]) / 100.f;
            cur.deltastatesYesChoice.s_ideology = (float)(card["effectsYes"]["s_ideology"]) / 100.f;
            cur.deltastatesYesChoice.s_influence = (float)(card["effectsYes"]["s_influence"]) / 100.f;
            cur.deltastatesYesChoice.s_finance = (float)(card["effectsYes"]["s_finance"]) / 100.f;
            cur.deltastatesYesChoice.s_power = (float)(card["effectsYes"]["s_power"]) / 100.f;
            cur.deltastatesYesChoice.g_finance = (float)(card["effectsYes"]["g_finance"]) / 100.f;
            cur.deltastatesYesChoice.g_influence = (float)(card["effectsYes"]["g_influence"]) / 100.f;
            cur.deltastatesYesChoice.g_moral = (float)(card["effectsYes"]["g_moral"]) / 100.f;
            cur.deltastatesYesChoice.g_power = (float)(card["effectsYes"]["g_power"]) / 100.f;
            cur.deltastatesYesChoice.b_agentNet = (float)(card["effectsYes"]["g_moral"]) / 100.f;
            cur.deltastatesYesChoice.b_greenChance = (float)(card["effectsYes"]["g_moral"]) / 100.f;
            cur.deltastatesYesChoice.b_redChance = (float)(card["effectsYes"]["g_moral"]) / 100.f;
            cur.deltastatesYesChoice.b_yellowChance = (float)(card["effectsYes"]["g_moral"]) / 100.f;

            cur.deltastatesYesChoice.doomsdayClockProgress = (float)(card["effectsYes"]["doomsdayWatch"]) / 100.f;

            cur.deltastatesNoChoice.s_moral = (float)(card["effectsNo"]["s_moral"]) / 100.f;
            cur.deltastatesNoChoice.s_ideology = (float)(card["effectsNo"]["s_ideology"]) / 100.f;
            cur.deltastatesNoChoice.s_influence = (float)(card["effectsNo"]["s_influence"]) / 100.f;
            cur.deltastatesNoChoice.s_finance = (float)(card["effectsNo"]["s_finance"]) / 100.f;
            cur.deltastatesNoChoice.s_power = (float)(card["effectsNo"]["s_power"]) / 100.f;
            cur.deltastatesNoChoice.g_finance = (float)(card["effectsNo"]["g_finance"]) / 100.f;
            cur.deltastatesNoChoice.g_influence = (float)(card["effectsNo"]["g_influence"]) / 100.f;
            cur.deltastatesNoChoice.g_moral = (float)(card["effectsNo"]["g_moral"]) / 100.f;
            cur.deltastatesNoChoice.g_power = (float)(card["effectsNo"]["g_power"]) / 100.f;
            cur.deltastatesNoChoice.b_agentNet = (float)(card["effectsNo"]["g_moral"]) / 100.f;
            cur.deltastatesNoChoice.b_greenChance = (float)(card["effectsNo"]["g_moral"]) / 100.f;
            cur.deltastatesNoChoice.b_redChance = (float)(card["effectsNo"]["g_moral"]) / 100.f;
            cur.deltastatesNoChoice.b_yellowChance = (float)(card["effectsNo"]["g_moral"]) / 100.f;

            cur.deltastatesNoChoice.doomsdayClockProgress = (float)(card["effectsYes"]["doomsdayWatch"]) / 100.f;

        }

    }
    catch (const nlohmann::json::exception& ex)
    {
        std::cerr << "Failed to parse cards.json: " << ex.what() << std::flush;
        return -1;
    }


    CardModifiers currentIvent;
    Card* current = &sovietcards[0];

    float moral = 1.f;
    sf::Clock clock;


    current->moveTo({ (float)(windowWidth / 2), (float)(windowHeight / 2) });
    bool cardSwiped = 0, nextMove = 0;

    TextBox movedescription{ cardFont, 32 };
    movedescription.setPosition({ (float)(windowWidth / 2) - 880, (float)(windowHeight / 2) });

    
/*
    objects.push_back(&buyYellowCard);
    objects.push_back(&buyRedCard);
    objects.push_back(&buyGreenCard);

    buyYellowCard.setVisible(false);
    buyRedCard.setVisible(false);
    buyGreenCard.setVisible(false);
    */
    //*
    sf::Text winnerMessage(specialFont, "", 144);
    winnerMessage.setOutlineColor(sf::Color::Black);
    winnerMessage.setOutlineThickness(6.f);
    winnerMessage.setFillColor({ 255,240,180 });
    winnerMessage.setScale({ 1.5f, 1.5f });

    Winner winner = Winner::None;
    winnerMessage.setPosition({ static_cast<float>(windowWidth / 2), static_cast<float>(windowHeight / 3 * 2) });
    //*/
    SovietInterface sovi;
    GermanInterface gmi;
    BrittishInterface bmi;

    sovi.SetCenter();
    gmi.SetLeft();
    bmi.SetRight();

    Button skipMove({ (float)(40),(float)(windowHeight/2) }, { 60,60 }, { 100,100,100 }, [&]() {});
    objects.push_back(&skipMove);
    window.setVerticalSyncEnabled(true);
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        sf::Vector2f cursorPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

        while (auto eventOpt = window.pollEvent())
        {
            auto& event = eventOpt.value();

            if (event.is<sf::Event::Closed>())
                window.close();

            if (auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2i mousePos = { mouseEvent->position.x, mouseEvent->position.y };

                if (mouseEvent->button == sf::Mouse::Button::Right)
                {
                    if (current->opened)
                    {
                        if (!cardSwiped)
                        {
                            if (mouseEvent->position.x > windowWidth / 2)
                            {
                                current->moveTo({ (float)3000, (float)(windowHeight / 2) });
                                current->swap();
                                gameState.countries.applyOther(current->deltastatesYesChoice, currentIvent);
                                cardSwiped = 1;
                                //  damper.swap();
                            }
                            else
                            {
                                current->moveTo({ (float)-3000, (float)(windowHeight / 2) });
                                current->swap();
                                gameState.countries.applyOther(current->deltastatesNoChoice, currentIvent);
                                cardSwiped = 1;
                                //  damper.swap();
                            }
                        }
                    }
                    else
                        current->swap();

                   bmi.buyYellowCard.click();
                   bmi.buyRedCard.click();
                   bmi.buyGreenCard.click();
                }
                if (cardSwiped && skipMove.containsPoint({ (float)mousex,(float)mousey }))
                    damper.swap();
            }
            if (auto* mouseEvent = event.getIf<sf::Event::MouseMoved>())
            {
                mousex = mouseEvent->position.x;
                mousey = mouseEvent->position.y;
                /*/
                if (mouseEvent->position.x > windowWidth / 2)
                    current->setRotation(10.f);
                else
                    current->setRotation(-10.f);
                //*/
            }
            else if (auto* keyEvent = event.getIf<sf::Event::KeyReleased>())
            {
                if (keyEvent->code == sf::Keyboard::Key::Escape) window.close();
            }
            if (gameState.countries.doomsdayClockProgress >= 1.f)
                gameState.warStarted = true;
        }
        if (mousex > windowWidth / 2)
        {
            current->setRotation(10.f);
            sovi.miliatarPower.hasPoint = (current->deltastatesYesChoice.s_power != 0);
            sovi.moralParam.hasPoint = (current->deltastatesYesChoice.s_moral != 0);
            sovi.influenceParam.hasPoint = (current->deltastatesYesChoice.s_influence != 0);
            sovi.ideologyWin.hasPoint = (current->deltastatesYesChoice.s_ideology != 0);
            sovi.financeParam.hasPoint = (current->deltastatesYesChoice.s_finance != 0);

            gmi.moralParam.hasPoint = (current->deltastatesYesChoice.g_moral != 0);
            gmi.miliatarPower.hasPoint = (current->deltastatesYesChoice.g_power != 0);
            gmi.influenceParam.hasPoint = (current->deltastatesYesChoice.g_influence != 0);
            gmi.financeParam.hasPoint = (current->deltastatesYesChoice.g_finance != 0);

            bmi.greenChance.hasPoint = (current->deltastatesYesChoice.b_greenChance != 0);
            bmi.redChance.hasPoint = (current->deltastatesYesChoice.b_redChance != 0);
            bmi.yellowChance.hasPoint = (current->deltastatesYesChoice.b_yellowChance != 0);
           // bmi.financeParam.hasPoint = (current->deltastatesYesChoice.g_finance != 0);
        }
        else
        {
            current->setRotation(-10.f);
            sovi.miliatarPower.hasPoint = (current->deltastatesNoChoice.s_power != 0);
            sovi.moralParam.hasPoint = (current->deltastatesNoChoice.s_moral != 0);
            sovi.influenceParam.hasPoint = (current->deltastatesNoChoice.s_influence != 0);
            sovi.ideologyWin.hasPoint = (current->deltastatesNoChoice.s_ideology != 0);
            sovi.financeParam.hasPoint = (current->deltastatesNoChoice.s_finance != 0);

            gmi.moralParam.hasPoint = (current->deltastatesNoChoice.g_moral != 0);
            gmi.miliatarPower.hasPoint = (current->deltastatesNoChoice.g_power != 0);
            gmi.influenceParam.hasPoint = (current->deltastatesNoChoice.g_influence != 0);
            gmi.financeParam.hasPoint = (current->deltastatesNoChoice.g_finance != 0);

            bmi.greenChance.hasPoint = (current->deltastatesNoChoice.b_greenChance != 0);
            bmi.redChance.hasPoint = (current->deltastatesNoChoice.b_redChance != 0);
            bmi.yellowChance.hasPoint = (current->deltastatesNoChoice.b_yellowChance != 0);
        }
        if (cardSwiped)
        {
          //  if (damper.state == Damper::AnimationState::Idle)
            if (damper.state == Damper::AnimationState::Closed)
            {
                cardSwiped = 0;
                current->teleport({ 0, 0 });
                if (damper.currentPlayer == 0)
                {
                    sovi.SetCenter();
                    gmi.SetLeft();
                    bmi.SetRight();
                    current = &sovietcards[(int)Random(0, sovietcards.size())];
                }
                else if (damper.currentPlayer == 1)
                {
                    sovi.SetRight();
                    gmi.SetCenter();
                    bmi.SetLeft();
                    current = &germancards[(int)Random(0, germancards.size())];
                }
                if (damper.currentPlayer == 2)
                {
                    sovi.SetLeft();
                    gmi.SetRight();
                    bmi.SetCenter();
                    current = &brittishcards[(int)Random(0, brittishcards.size())];
                }

                current->moveTo({ (float)(windowWidth / 2), (float)(windowHeight / 2) });
            }
        }

        damper.update(deltaTime);
        sovi.update(deltaTime);
        gmi.update(deltaTime);
        bmi.update(deltaTime);
        if (skipMove.containsPoint({ (float)mousex, (float)mousey })) skipMove.hover();
        else skipMove.unhover();
        /*moralParam.update(deltaTime);
        SovietInterface::influenceParam.update(deltaTime);
        financeParam.update(deltaTime);
       
        //specialCardDeck.update(deltaTime, cursorPos.x, cursorPos.y);

        yellowChance.update(deltaTime);
        redChance.update(deltaTime);
        greenChance.update(deltaTime);*/

        window.clear();
        for (sf::Drawable* dr : objects)
            window.draw(*dr);
//*
        movedescription.setPosition({ (float)(windowWidth / 2) - 880, (float)(windowHeight / 2) - 200 });
        std::string s = "Отклонить\nЭффекты хода:\nСоветский союз:\n";
        s += "Идеология: " + std::to_string((int)(current->deltastatesNoChoice.s_ideology * 100)) + "\n";
        s += "Финансы: " + std::to_string((int)(current->deltastatesNoChoice.s_finance * 100)) + "\n";
        s += "Влияние: " + std::to_string((int)(current->deltastatesNoChoice.s_influence * 100)) + "\n";
        s += "Военная мощь: " + std::to_string((int)(current->deltastatesNoChoice.s_power * 100)) + "\n";
        s += "Боевой дух: " + std::to_string((int)(current->deltastatesNoChoice.s_moral * 100)) + "\nГермания:\n";
        s += "Военная мощь: " + std::to_string((int)(current->deltastatesNoChoice.g_power * 100)) + "\n";
        s += "Финансы: " + std::to_string((int)(current->deltastatesNoChoice.g_finance * 100)) + "\n";
        s += "Влияние: " + std::to_string((int)(current->deltastatesNoChoice.g_influence * 100)) + "\n";
        //    s += "Влияние: " + std::to_string(current->deltastatesNoChoice.) + "\n";
        s += "Боевой дух: " + std::to_string((int)(current->deltastatesNoChoice.g_moral * 100)) + "\n";
        s += "Начало войны: " + std::to_string((int)(current->deltastatesNoChoice.doomsdayClockProgress * 100)) + "\nБритания:\n";

        movedescription.setText(sf::String(systemToSfString(s)));

        window.draw(movedescription);

        movedescription.setPosition({ (float)(windowWidth / 2) + 700, (float)(windowHeight / 2) - 200 });
        s = "Принять\nЭффекты хода:\nСоветский союз:\n";
        s += "Идеология: " + std::to_string((int)(current->deltastatesYesChoice.s_ideology * 100)) + "\n";
        s += "Финансы: " + std::to_string((int)(current->deltastatesYesChoice.s_finance * 100)) + "\n";
        s += "Влияние: " + std::to_string((int)(current->deltastatesYesChoice.s_influence * 100)) + "\n";
        s += "Военная мощь: " + std::to_string((int)(current->deltastatesYesChoice.s_power * 100)) + "\n";
        s += "Боевой дух: " + std::to_string((int)(current->deltastatesYesChoice.s_moral * 100)) + "\nГермания:\n";
        s += "Военная мощь: " + std::to_string((int)(current->deltastatesYesChoice.g_power * 100)) + "\n";
        s += "Финансы: " + std::to_string((int)(current->deltastatesYesChoice.g_finance * 100)) + "\n";
        s += "Влияние: " + std::to_string((int)(current->deltastatesYesChoice.g_influence * 100)) + "\n";
        s += "Боевой дух: " + std::to_string((int)(current->deltastatesYesChoice.g_moral * 100)) + "\n";
        s += "Начало войны: " + std::to_string((int)(current->deltastatesNoChoice.doomsdayClockProgress * 100)) + "\nБритания:\n";

        movedescription.setText(sf::String(systemToSfString(s)));
     //   movedescription.setText("123");
        
        window.draw(movedescription);
        //*/
        current->update(deltaTime, 4.f);

        {
            int doomsdayclockSprite = static_cast<int>(gameState.countries.doomsdayClockProgress * 12);
            doomsdayclockSprite = std::clamp(doomsdayclockSprite, 0, 11);
            window.draw(doomClockStates[doomsdayclockSprite]);
        }
        /*
        switch (damper.currentPlayer)
        {
        case 0: { window.draw(specialCardDeckS); break; }
        case 1: { window.draw(specialCardDeckG); break; }
        case 2: { window.draw(specialCardDeckB); break; }
        }*/
        window.draw(bmi);
        window.draw(sovi);
        window.draw(gmi);

        window.draw(*current);


        window.draw(damper);

        //*
        winner = gameState.determineWinner();
        switch (winner)
        {
        case Winner::USSR:
        {
            winnerMessage.setString(U"Все буржуи побиты");

        } break;
        case Winner::Germany:
        {
            winnerMessage.setString(U"Мир услышал ехидное немецкое АХХАХАХАХАХАХ");
        } break;
        case Winner::Britain:
        {
            winnerMessage.setString(U"Бриташки победили");
        } break;
        }
        winnerMessage.setOrigin({ winnerMessage.getLocalBounds().size.x / 2, winnerMessage.getLocalBounds().size.y / 2 });

        if (winner != Winner::None)
        {
          //  damper.lock();
          //  damper.state = Damper::AnimationState::In;
            window.draw(winnerMessage);
        }
        //*/


        window.display();
    }

    return 0;
}
