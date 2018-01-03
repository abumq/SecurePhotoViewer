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
#include <map>
#include <utility>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "external/mine.h"
#include "external/libzippp.h"
#include "external/rc.h"

/**
 * Path where images are saved
 */
static const std::string kSavePath = "/Users/mkhan/Downloads/";

/**
 * Download button default color (at normal state)
 */
static const sf::Color kDownloadButtonDefaultColor(255, 255, 255, 75);

/**
 * Download button hover color
 */
static const sf::Color kDownloadButtonHoverColor(255, 255, 255, 200);

/**
 * Represents the move factor for positioning when zoomed in
 */
static const float kMoveFactor = 20.0f;

/**
 * Right angle (90')
 */
static const float kRightAngle = 90.0f;

/**
 * Maximum number of thumbnails to display
 */
static const int kMaximumThumbnails = 20;

/**
 * Size of thumbnail in pixels
 */
static const std::size_t kThumbnailSize = 128;

/**
 * Represents single item with it's attributes
 */
struct Item
{
    /**
     * Raw data (this may take more memory)
     */
    char* data;
    
    /**
     * Image size (file size in bytes)
     */
    std::size_t size;
    
    /**
     * Filename in archive
     */
    std::string name;
    
    /**
     * Image object
     */
    sf::Image image;
    
    Item(char* data_, std::size_t size_, const std::string& name_)
        : data(data_), size(size_), name(name_)
    {
        image.loadFromMemory(data_, size_);
    }
    
    Item(const Item& item)
        : data(item.data), size(item.size), name(item.name)
    {
        image.loadFromMemory(item.data, item.size);
    }
};

struct Viewer
{
    /**
     * Viewer's texture object
     */
    sf::Texture texture;
    
    /**
     * Viewer's sprite object
     */
    sf::Sprite sprite;
    
    /**
     * Viewer's global rotation tracking variable
     */
    float currentRotation;
    
    /**
     * Name of current archive being viewed
     */
    std::string archiveName;
    
    /**
     * Current photo index (zero-based)
     */
    int currentIndex;
    
    /**
     * Items in archive being viewed
     */
    std::vector<Item> list;
};

struct Thumbnail
{
    std::size_t index;
    sf::Sprite sprite;
    std::string name;
};

/**
 * Global viewer object
 */
Viewer viewer;

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
    
    if (archiveContents.find_first_of(":") != 32)
    {
        throw "Invalid encrypted data. Expected <IV>:<B64>";
    }
    
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
    list.reserve(entries.size());
    for (auto& entry : entries)
    {
        if ((endsWith(entry.getName(), ".jpg")
             || endsWith(entry.getName(), ".png")
             || endsWith(entry.getName(), ".jpeg")
             || endsWith(entry.getName(), ".gif")
             || endsWith(entry.getName(), ".svg"))
            && (entry.getName().size() > 9
                && entry.getName().substr(0, 9) != "__MACOSX/")
            )
        {
            list.emplace_back(static_cast<char*>(entry.readAsBinary()),
                              entry.getSize(),
                              entry.getName());
        }
    }
    zf.close();
    list.shrink_to_fit();
    std::cout << list.size() << " images" << std::endl;
    if (doCleanUp)
    {
        std::cout << "Clean up..." << std::endl;
        remove(insecureArchive.c_str());
    }
    return list;
}

/**
 * Reset zoom level to original, the position to 0,0 and rotation
 */
void reset()
{
    viewer.sprite.setPosition(0.0f, 0.0f);
    viewer.sprite.setScale(1.0f, 1.0f);
    if (viewer.currentRotation != 0.0f)
    {
        viewer.sprite.rotate(-viewer.currentRotation);
        viewer.currentRotation = 0.0f;
    }
}

/**
 * Add scale level for sprite
 */
void zoomIn()
{
    viewer.sprite.setScale(viewer.sprite.getScale().x + 0.5f, viewer.sprite.getScale().y + 0.5f);
}

/**
 * Subtract scale level for sprite
 */
void zoomOut()
{
    if (viewer.sprite.getScale().x > 0.5f)
    {
        viewer.sprite.setScale(viewer.sprite.getScale().x - 0.5f, viewer.sprite.getScale().y - 0.5f);
    }
}

/**
 * Moves vertically if zoomed in
 */
bool moveVerticallyIfZoomed(float moveFactor)
{
    if (viewer.sprite.getScale().y != 1.0f)
    {
        viewer.sprite.move(0.0f, moveFactor);
        return true;
    }
    return false;
}

/**
 * Moves horizontally if zoomed in. If it's moved returns true otherwise false
 */
bool moveHorizontallyIfZoomed(float moveFactor)
{
    if (viewer.sprite.getScale().x != 1.0f)
    {
        viewer.sprite.move(moveFactor, 0.0f);
        return true;
    }
    return false;
}

/**
 * Get window title based on idx
 */
std::string getWindowTitle()
{
    return std::to_string(viewer.currentIndex + 1) + " / " + std::to_string(viewer.list.size()) + " - " + "Secure Photo - " + viewer.archiveName;
}

/**
 * Navigates to current index
 */
void navigate()
{
    Item item = viewer.list.at(viewer.currentIndex);
    
    viewer.texture.loadFromImage(item.image);
    viewer.sprite.setTextureRect(sf::IntRect(0, 0, (int) item.image.getSize().x, (int) item.image.getSize().y));
    
    viewer.sprite.setScale(1.0f, 1.0f);
    viewer.sprite.setPosition(0.0f, 0.0f);
    std::cout << "Opening [" << (viewer.currentIndex + 1) << " / "
                << viewer.list.size() << "] " << item.name << " ("
                << item.size << " bytes)";
    std::cout << " (" << item.image.getSize().x << " x " << item.image.getSize().y << ")" << std::endl;
    reset();
}

void next(sf::Window* window)
{
    if (++viewer.currentIndex > static_cast<int>(viewer.list.size() - 1))
    {
        viewer.currentIndex = 0;
    }
    navigate();
    window->setTitle(getWindowTitle());
}

void prev(sf::Window* window)
{
    if (--viewer.currentIndex < 0)
    {
        viewer.currentIndex = static_cast<int>(viewer.list.size() - 1);
    }
    navigate();
    window->setTitle(getWindowTitle());
}

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <archive> [<key> = \"\"] [<initial_index> = 0]" << std::endl;
        return 1;
    }
    
    bool isFullscreen = false;
    
    viewer.archiveName = argv[1];
    viewer.currentRotation = 0.0f;
    viewer.currentIndex = 0;
    
    std::string tempFilename;
    
    if (argc > 2)
    {
        try
        {
            tempFilename = unpack(viewer.archiveName, argv[2]);
        }
        catch (const char* e)
        {
            std::cerr << e << std::endl;
            return 1;
        }
    }
    else
    {
        tempFilename = viewer.archiveName; // insecure archive
    }
    
    viewer.list = createList(tempFilename, argc > 2);
    
    const sf::VideoMode winMode = sf::VideoMode::getFullscreenModes().at(0);
    sf::Image winIcon;
    const std::string rawIcon = mine::Base64::decode(kWindowIcon);
    winIcon.loadFromMemory((void*) rawIcon.data(), rawIcon.size());
    sf::RenderWindow window(winMode, "Secure Photo [Loading...]");
    window.setIcon(256, 256, winIcon.getPixelsPtr());
    
    if (argc > 3)
    {
        viewer.currentIndex = std::max(std::min(atoi(argv[3]) - 1, static_cast<int>(viewer.list.size() - 1)), 0);
    }
    
    viewer.sprite.setTexture(viewer.texture);
    
    navigate();
    window.setTitle(getWindowTitle());
    
    // Buttons
    sf::Texture downloadTexture;
    sf::Sprite buttonsSprite(downloadTexture);
    buttonsSprite.setColor(kDownloadButtonDefaultColor);
    const std::string rawDownloadButton = mine::Base64::decode(kDownloadButton);
    downloadTexture.loadFromMemory((void*) rawDownloadButton.data(), rawDownloadButton.size());
    buttonsSprite.setPosition(0, 0);
    buttonsSprite.setTextureRect(sf::IntRect(0, 0, 100, 100));
    
    std::map<std::size_t, Thumbnail> thumbnails;
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            bool newPhoto = false;
            sf::Vector2i pos = sf::Mouse::getPosition(window);
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::MouseMoved:
                {
                    buttonsSprite.setColor(buttonsSprite.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y)
                                            ? kDownloadButtonHoverColor
                                            : kDownloadButtonDefaultColor);
                    break;
                }
                case sf::Event::MouseButtonPressed:
                {
                    switch (event.mouseButton.button)
                    {
                        case sf::Mouse::Button::Left:
                            if (buttonsSprite.getGlobalBounds().contains(pos.x, pos.y))
                            {
                                const Item item = viewer.list.at(viewer.currentIndex);
                                const std::string extension = item.name.substr(item.name.find_last_of("."));
                                const std::string filename = kSavePath + "secure-photo-" + mine::AES::generateRandomKey(128) + extension;
                                std::cout << "Saving... [" << filename << "]" << std::endl;
                                
                                // item.image.saveToFile(savePath) causes problem because of version of libjpeg in local dev
                                // so we manually save the raw data
                                std::ofstream ofs(filename, std::ios::binary);
                                ofs.write(item.data, item.size);
                                ofs.flush();
                                ofs.close();
                            }
                            else
                            {
                                int newIndex = -1;
                                for (auto& thumbnailPair : thumbnails)
                                {
                                    const Thumbnail* const thumbnail = &(thumbnailPair.second);
                                    if (thumbnail->sprite.getGlobalBounds().contains(pos.x, pos.y))
                                    {
                                        newIndex = thumbnail->index;
                                        break;
                                    }
                                }
                                if (newIndex == -1)
                                {
                                    next(&window);
                                }
                                else
                                {
                                    viewer.currentIndex = newIndex;
                                    navigate();
                                    window.setTitle(getWindowTitle());
                                }
                            }
                            break;
                        case sf::Mouse::Button::Right:
                            prev(&window);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case sf::Event::KeyPressed:
                {
                    switch (event.key.code)
                    {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;
                        case sf::Keyboard::F:
                            window.create(winMode,
                                          getWindowTitle(),
                                          isFullscreen
                                            ? sf::Style::Default
                                            : sf::Style::Default | sf::Style::Fullscreen);
                            isFullscreen = !isFullscreen;
                            break;
                        case sf::Keyboard::Right:
                            if (!moveHorizontallyIfZoomed(-kMoveFactor))
                            {
                                next(&window);
                            }
                            break;
                        case sf::Keyboard::Left:
                            if (!moveHorizontallyIfZoomed(kMoveFactor))
                            {
                                prev(&window);
                            }
                            break;
                        case sf::Keyboard::Up:
                            if (!moveVerticallyIfZoomed(kMoveFactor))
                            {
                                viewer.currentRotation += kRightAngle;
                                viewer.sprite.rotate(kRightAngle);
                            }
                            break;
                        case sf::Keyboard::Down:
                            if (!moveVerticallyIfZoomed(-kMoveFactor))
                            {
                                viewer.currentRotation -= kRightAngle;
                                viewer.sprite.rotate(-kRightAngle);
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
        window.draw(viewer.sprite);
        
        // thumbnails
        const std::size_t firstThumbnailIndex = std::max(viewer.currentIndex - (kMaximumThumbnails / 2), 0);
        const std::size_t totalThumbnails = std::min(viewer.list.size(), static_cast<std::size_t>(kMaximumThumbnails));
        for (std::size_t i = firstThumbnailIndex, idx = 0;
             i < std::min(totalThumbnails + firstThumbnailIndex, viewer.list.size());
             ++i, ++idx)
        {
            sf::Texture thumbnailTexture;
            sf::Sprite thumbnailSprite(thumbnailTexture);
            Item item = viewer.list.at(i);
            thumbnailTexture.loadFromImage(item.image);
            thumbnailSprite.setPosition(((window.getSize().x / 2) - ((totalThumbnails / 2) * kThumbnailSize)) + (idx * kThumbnailSize),
                                        window.getSize().y - kThumbnailSize - 10);
            thumbnailSprite.setTextureRect(sf::IntRect(0, 0, kThumbnailSize * 3, kThumbnailSize * 3));
            thumbnailSprite.setScale(0.25, 0.25);
            if (i == viewer.currentIndex)
            {
                thumbnailSprite.setColor(sf::Color(255, 255, 255, 100));
            }
            else
            {
                thumbnailSprite.setColor(sf::Color(255, 255, 255, 200));
            }
            thumbnails[idx] = { i, thumbnailSprite, item.name };
            window.draw(thumbnailSprite);
        }
        
        window.draw(buttonsSprite);
        window.display();
    }
    return 0;
}

