#pragma once 

#include <string>
#include <filesystem>
#include <vector>
#include <memory>

class EngineContext;

enum class ContentType{
    Unknown, 
    Image,
    Model,
    Scene,
    Material,
    Directory,
    Text
};

struct ContentItem{
    std::string name{};
    std::filesystem::path path{};
    ContentType type{ContentType::Unknown};
    bool isDir{false};

    std::vector<std::shared_ptr<ContentItem>> children {};
    std::weak_ptr<ContentItem> parent{};
};

class ContentBrowser{

    EngineContext* ece;
    std::shared_ptr<ContentItem> root{};
    std::shared_ptr<ContentItem> selectedDir{};
    std::filesystem::path selectedDirPath{};

    int _thumbnailSize = 64; 

    std::string fileTypeString(const ContentType& type);
    ContentType determineFileType(const std::filesystem::directory_entry& entry);
    void drawDirTreeRecursive(std::shared_ptr<ContentItem> parent);
    void printTreeRecursive(std::shared_ptr<ContentItem> parent, int level = 0);
    void dirTreeRecursive(std::shared_ptr<ContentItem> parent);
    void scan();

    void breadcrumb();
    void drawTree();
    void drawFolderContent();

public:
    void draw();

    void init(EngineContext* ece);

};

// bread crumbs 
// filter 
// cache 
// directory tree 
// directory browser 
// description 
// thumbnail 
// drag/drop
// basic operations for folder & files
// - add 
// - remove 
// - move 
// - remame 
// - duplicate ?
// favorites
// collections 
// search 
// 
// browsing history  <- ->
// up folder button 
// +- for scale slider
// breadcrumbs buttons
// breadcrumsb home button? 
// drawFolderContent içinde dosya yazılar maks 2-3 satır olsun belki default olarak 2 satır ayırabiliriz. 


