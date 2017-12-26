/**
 * This is a quick image viewing tool based on SFML that allows you to view
 * images from secure archive that was encrypted using AES
 *
 * The contents of archive is expected to be in following format
 *     <IV>:<Base-64 of Encrypted Zip File>
 *
 * Full compile command: g++ main.cc external/libzippp.cpp external/mine.cc -I/usr/local/lib -lsfml-graphics  -lsfml-window -lsfml-system -lzip -lz -std=c++11 -O3 -o secure-photo-viewer
 *
 * In order to run program you will need to provide AES key in first 
 * param and archive name in second, e.g,
 *    ./secure-photo-viewer <archive> [<key> = ""] [<initial_index> = 0]
 *
 * Keys:
 *      - Right Arrow: Next photo / Re-position when zoomed
 *      - Left Arrow: Prev photo / Re-position when zoomed
 *      - Up Arrow: Re-position when zoomed
 *      - Down Arrow: Re-position when zoomed
 *      - Equal: Zoom In
 *      - Backspace: Zoom Out
 *      - Backslash (\): Reset Zoom
 *      - F: Enter/exit Fullscreen
 *      - Escape: Exit
 *
 * Author: abumusamq (Majid)
 */

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "external/mine.h"
#include "external/libzippp.h"

/**
 * Represents the move factor for positioning when zoomed in
 */
static const float kMoveFactor = 20.0f;

/**
 * Right angle (90')
 */
static const float kRightAngle = 90.0f;

/**
 * Global texture object
 */
sf::Texture texture;

/**
 * Global sprite object
 */
sf::Sprite sprite;

/**
 * Global rotation tracking variable
 */
float currentRotation = 0.0;

/**
 * Represents single item with it's attributes
 */
struct Item {
    sf::Image image;
    std::size_t size;
    std::string name;
};

/**
 * Name of current archive being viewed
 */
std::string archiveName;

/**
 * Current photo index (zero-based)
 */
int currIdx = 0;

/**
 * Items in archive being viewed
 */
std::vector<Item> list;

/**
 * Returns true if subject ends with str
 */
bool endsWith(const std::string& subject, const std::string& str)
{
    if (str.size() > subject.size())
    {
        return false;
    }
    return std::equal(subject.begin() + subject.size() - str.size(), subject.end(), str.begin());
}

/**
 * Unpacks the encrypted archive and returns temporary filename of unencrypted archive
 */
std::string unpack(const std::string& archiveFilename, const std::string& key)
{
    
    std::cout << "Unpacking..." << std::endl;
    
    std::ifstream ifs(archiveFilename.data());
    std::string archiveContents((std::istreambuf_iterator<char>(ifs)),
                                (std::istreambuf_iterator<char>()));
    
    std::string iv(archiveContents.substr(0, 32));
    std::string contents(archiveContents.substr(33));
    
    mine::AES aesManager;
    aesManager.setKey(key);
    
    std::string zip(aesManager.decr(contents, iv, mine::MineCommon::Encoding::Base64, mine::MineCommon::Encoding::Raw));
    
    std::string tempFilename("/tmp/insecure-archive-" + mine::AES::generateRandomKey(128) + ".zip");
    
    std::cout << "Flushing from memory..." << std::endl;
    
    std::ofstream ofs(tempFilename, std::ios::binary);
    ofs.write(zip.data(), zip.size());
    ofs.flush();
    ofs.close();
    return tempFilename;
}

/**
 * Loads the items from insecure (unencrypted) archive and returns the list
 */
std::vector<Item> createList(const std::string& insecureArchive, bool doCleanUp)
{
    std::vector<Item> list;
    std::cout << "Loading..." << std::endl;
    
    libzippp::ZipArchive zf(insecureArchive);
    zf.open(libzippp::ZipArchive::READ_ONLY);
    
    const std::vector<libzippp::ZipEntry> entries = zf.getEntries();
    for (std::vector<libzippp::ZipEntry>::const_iterator entry = entries.begin(); entry != entries.end(); ++entry)
    {
        if ((endsWith(entry->getName(), ".jpg")
             || endsWith(entry->getName(), ".png")
             || endsWith(entry->getName(), ".jpeg")
             || endsWith(entry->getName(), ".gif")
             || endsWith(entry->getName(), ".svg"))
            && (entry->getName().size() > 9
                && entry->getName().substr(0, 9) != "__MACOSX/")
            )
        {
            Item item = {sf::Image(), entry->getSize(), entry->getName()};
            item.image.loadFromMemory(entry->readAsBinary(), item.size);
            list.push_back(item);
        }
    }
    zf.close();
    
    std::cout << list.size() << " images" << std::endl;
    if (doCleanUp)
    {
        std::cout << "Clean up..." << std::endl;
        remove(insecureArchive.c_str());
    }
    return list;
}

/**
 * Navigates to specified index from list
 *
 * This function updates the texture. you may need to load the initial image
 * for the texture before this function will have any affect
 */
void navigateTo(int idx, std::vector<Item>* list)
{
    Item item = list->at(idx);
    
    texture.loadFromImage(item.image);
    sprite.setTextureRect(sf::IntRect(0, 0, (int) item.image.getSize().x, (int) item.image.getSize().y));
    
    sprite.setScale(1.0f, 1.0f);
    sprite.setPosition(0.0f, 0.0f);
    std::cout << "Opening [" << (idx + 1) << " / "
                << list->size() << "] " << item.name << " ("
                << item.size << " bytes)";
    std::cout << " (" << item.image.getSize().x << " x " << item.image.getSize().y << ")" << std::endl;
}

/**
 * Reset zoom level to original, the position to 0,0 and rotation
 */
void reset()
{
    sprite.setPosition(0.0f, 0.0f);
    sprite.setScale(1.0f, 1.0f);
    if (currentRotation != 0.0f)
    {
        sprite.rotate(-currentRotation);
        currentRotation = 0.0f;
    }
}

/**
 * Add scale level for sprite
 */
void zoomIn()
{
    sprite.setScale(sprite.getScale().x + 0.5f, sprite.getScale().y + 0.5f);
}

/**
 * Subtract scale level for sprite
 */
void zoomOut()
{
    if (sprite.getScale().x > 0.5f)
    {
        sprite.setScale(sprite.getScale().x - 0.5f, sprite.getScale().y - 0.5f);
    }
}

/**
 * Moves vertically if zoomed in
 */
bool moveVerticallyIfZoomed(float moveFactor)
{
    if (sprite.getScale().y != 1.0f)
    {
        sprite.move(0.0f, moveFactor);
        return true;
    }
    return false;
}

/**
 * Moves horizontally if zoomed in. If it's moved returns true otherwise false
 */
bool moveHorizontallyIfZoomed(float moveFactor)
{
    if (sprite.getScale().x != 1.0f)
    {
        sprite.move(moveFactor, 0.0f);
        return true;
    }
    return false;
}

/**
 * Get window title based on idx
 */
std::string getWindowTitle()
{
    return std::to_string(currIdx + 1) + " / " + std::to_string(list.size()) + " - " + "Secure Photo - " + archiveName;
}

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <archive> [<key> = \"\"] [<initial_index> = 0]" << std::endl;
        return 1;
    }
    
    bool isFullscreen = false;
    
    archiveName = argv[1];
    
    std::string tempFilename;
    
    if (argc > 2)
    {
        tempFilename = unpack(archiveName, argv[2]);
    } else
    {
        tempFilename = archiveName; // insecure archive
    }
    
    list = createList(tempFilename, argc > 2);
    
    sf::VideoMode winMode = sf::VideoMode::getFullscreenModes().at(0);
    sf::Image winIcon;
    winIcon.loadFromFile("icon.png");
    sf::RenderWindow window(winMode, "Secure Photo [Loading...]");
    window.setIcon(256, 256, winIcon.getPixelsPtr());
    
    if (argc > 3)
    {
        currIdx = std::max(std::min(atoi(argv[3]) - 1, static_cast<int>(list.size() - 1)), 0);
    }
    window.setTitle(getWindowTitle());
    
    sprite.setTexture(texture);
    
    navigateTo(currIdx, &list);
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            bool newPhoto = false;
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                {
                    switch (event.key.code)
                    {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;
                        case sf::Keyboard::F:
                            if (isFullscreen)
                            {
                                window.create(winMode, getWindowTitle());
                                isFullscreen = false;
                            } else
                            {
                                window.create(winMode,
                                              getWindowTitle(),
                                              sf::Style::Default | sf::Style::Fullscreen);
                                isFullscreen = true;
                            }
                            break;
                        case sf::Keyboard::Right:
                            if (!moveHorizontallyIfZoomed(-kMoveFactor))
                            {
                                currIdx++;
                                if (currIdx > static_cast<int>(list.size() - 1))
                                {
                                    currIdx = 0;
                                }
                                navigateTo(currIdx, &list);
                                reset();
                                window.setTitle(getWindowTitle());
                            }
                            break;
                        case sf::Keyboard::Left:
                            if (!moveHorizontallyIfZoomed(kMoveFactor))
                            {
                                currIdx--;
                                if (currIdx < 0)
                                {
                                    currIdx = static_cast<int>(list.size() - 1);
                                }
                                navigateTo(currIdx, &list);
                                reset();
                                window.setTitle(getWindowTitle());
                            }
                            break;
                        case sf::Keyboard::Up:
                            if (!moveVerticallyIfZoomed(kMoveFactor))
                            {
                                currentRotation += kRightAngle;
                                sprite.rotate(kRightAngle);
                            }
                            break;
                        case sf::Keyboard::Down:
                            if (!moveVerticallyIfZoomed(-kMoveFactor))
                            {
                                currentRotation -= kRightAngle;
                                sprite.rotate(-kRightAngle);
                            }
                            break;
                        case sf::Keyboard::Equal:
                            zoomIn();
                            break;
                        case sf::Keyboard::BackSpace:
                            zoomOut();
                            break;
                        case sf::Keyboard::BackSlash:
                            reset();
                            break;
                        default:
                            break;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        
        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.display();
    }
    return 0;
}

