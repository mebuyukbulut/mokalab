#include "ContentBrowser.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "imgui_internal.h"

#include "EngineContext.h"
#include "PathResolver.h"
#include "AssetManager.h"
#include "EventDispatcher.h"
#include "Texture.h"
#include "Builtin.h"

#include <iostream>

std::string ContentBrowser::fileTypeString(const ContentType &type)
{
    switch (type)
    {
    case ContentType::Unknown:
        return std::string("Not recognized");
    case ContentType::Image:
        return std::string("Image/Texture");
    case ContentType::Model:
        return std::string("Model");
    case ContentType::Scene:
        return std::string("Scene");
    case ContentType::Material:
        return std::string("Material");
    case ContentType::Directory:
        return std::string("Folder");
    case ContentType::Text:
        return std::string("Text");
    
    default:
        return std::string("Not determined!");
    }
}

ContentType ContentBrowser::determineFileType(const std::filesystem::directory_entry &entry)
{
    if(entry.is_directory())
        return ContentType::Directory;

    std::string ext = entry.path().extension();

    if(ext == ".txt")
        return ContentType::Text;
    else if(ext == ".png" || ext == ".jpg")
        return ContentType::Image;
    else if(ext == ".scn")
        return ContentType::Scene;
    else if(ext == ".obj" || ext == ".fbx")
        return ContentType::Model;
    else if(ext == ".mat")
        return ContentType::Material;
    else
        return ContentType::Unknown;
}

void ContentBrowser::drawDirTreeRecursive(std::shared_ptr<ContentItem> parent)
{
    // if(parent->parent.expired())
    //     std::cout << "root" << std::endl;
        
    for(auto item : parent->children)
    {
        
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        //if (isSelected(entity)) flags |= ImGuiTreeNodeFlags_Selected;
        //if (!(item->isDir)) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (!(item->isDir)) continue;

        bool opened = false;

        opened = ImGui::TreeNodeEx(item.get(), flags, "%s", item->name.c_str());
        // Seçme işlemi
        if (ImGui::IsItemClicked()){
            selectedDir = item; // sadece klasörler gösterildiği için böyle yapabiliriz
        }


        if(item->isDir && opened){
            drawDirTreeRecursive(item);
            ImGui::TreePop();
        }
    }


    // ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    // //if (isSelected(entity)) flags |= ImGuiTreeNodeFlags_Selected;
    // if (parent->children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;

    // bool opened = false;

    // opened = ImGui::TreeNodeEx("123", flags, "%s", item->name.c_str());

    // // Recursive çağrı
    // if (opened) {
    //     for (Transform* transform : entity->transform->getChildren())
    //         drawDirTreeRecursive(transform->owner);
    
    //     ImGui::TreePop();
    // }


    // if (false)//(isLastSelected(entity)) 
    // {
    //     ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(230, 125, 15, 255)); // orange foreground
    //     //opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity->transform->UUID, flags, entity->name.c_str());
    //     opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity->transform->UUID, flags, "%s", entity->name.c_str());
    //     //TreeNodeEx((void*)(uintptr_t)entity->UUID, flags, "%s", entity->name.c_str());
    //     ImGui::PopStyleColor(1);
    // }
    // else {
    //     opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity->transform->UUID, flags, "%s", entity->name.c_str());
    // }



    // // Seçme işlemi
    // if (ImGui::IsItemClicked()){
    //     if (!ImGui::GetIO().KeyCtrl)
    //         deselectAll();
        
    //     select(entity);
    // }


    // // --- BURASI: DRAG SOURCE (Sürüklemeyi Başlat) ---
    // if (ImGui::BeginDragDropSource()) {
    //     // Sürüklenen nesnenin pointer'ını paketle (Payload)
    //     ImGui::SetDragDropPayload("ENTITY_HIERARCHY_NODE", &entity, sizeof(Entity*));

    //     // Sürüklerken farenin yanında ne görünsün?
    //     ImGui::Text("Moving: %s", entity->name.c_str());
    //     ImGui::EndDragDropSource();
    // }

    // // --- BURASI: DRAG TARGET (Üzerine Bırakmayı Kabul Et) ---
    // if (ImGui::BeginDragDropTarget()) {
    //     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HIERARCHY_NODE")) {
    //         // Paketi aç (Sürüklenen Entity'yi al)
    //         Entity* draggedEntity = *(Entity**)payload->Data;

    //         // Kendi kendine veya zaten parent'ı olan birine bırakılmadığından emin ol
    //         if (draggedEntity != entity) {
    //             draggedEntity->transform->setParent(entity->transform.get()); 
    //         }
    //     }
    //     ImGui::EndDragDropTarget();
    // }
}
void ContentBrowser::printTreeRecursive(std::shared_ptr<ContentItem> parent, int level){
    if(parent->parent.expired())
        std::cout << "root" << std::endl;
    //"├── "
    for(auto item : parent->children)
    {
        for(int i = 0; i<level; i++) 
            std::cout << "  ";
        std::cout << " - " << item->name << (item->isDir ? "#" : " " ) << std::endl;
        
        if(item->isDir)
            printTreeRecursive(item, level+1);
    }
    
    //std::cout << dir_entry.path() << (dir_entry.is_directory() ? "#" : " " ) << '\n';
}
void ContentBrowser::dirTreeRecursive(std::shared_ptr<ContentItem> parent)
{

    for (auto const& dir_entry : std::filesystem::directory_iterator{parent->path}){
        auto item = std::make_shared<ContentItem>();
        //dir_entry.
        item->name = dir_entry.path().filename().string();
        item->path = dir_entry;
        item->isDir = dir_entry.is_directory();
        item->type = determineFileType(dir_entry);
        item->parent = parent; 

        if(item->path == selectedDirPath)
            selectedDir = item;

        if(item->isDir)
            dirTreeRecursive(item);

        parent->children.push_back(item);
    } 
}

void ContentBrowser::scan()
{
    selectedDirPath = selectedDir->path;
    selectedDir.reset();
    root->children.clear(); // clear all entities
    
    dirTreeRecursive(root);
    if(!selectedDir)
        selectedDir = root; 

    printTreeRecursive(root);
    // for (auto const& dir_entry : std::filesystem::directory_iterator{root->path}) 
    //     std::cout << dir_entry.path() << (dir_entry.is_directory() ? "#" : " " ) << '\n';

}


void ContentBrowser::breadcrumb(){
    // auto relative = std::filesystem::relative(selectedDir->path, root->path);
    // std::string str = ">";
    // str += relative.string();
    // ImGui::Text(str.c_str());

    
    std::vector<std::shared_ptr<ContentItem>> dirs; 
    dirs.push_back(selectedDir);
    constexpr int MAX_DEPTH = 7;
    int depth = 0; 
    while(dirs.back() != root && !dirs.back()->parent.expired() && depth++ < MAX_DEPTH)
        dirs.push_back(dirs.back()->parent.lock());

    ImGui::Text(">>");ImGui::SameLine();
    for(int i = dirs.size()-1; i>=0; i--){
        std::string bId = dirs[i]->name + "##" + std::to_string(i);
        if(ImGui::Button(bId.c_str())){
            //std::cout << "abc" << selectedDir->path << "\t" << dirs[i]->path << std::endl;
            selectedDir = dirs[i]; 
        }ImGui::SameLine();

        ImGui::Text("/");ImGui::SameLine();

    }
}

void ContentBrowser::drawTree(){
        // HIERARCHY PANEL

    ImGuiTreeNodeFlags rootFlag =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Bullet |
        ImGuiTreeNodeFlags_Leaf;


    if (ImGui::TreeNodeEx("root", rootFlag)){
        if (ImGui::IsItemClicked())
            selectedDir = root; 
        
        ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(55, 55, 55, 255)); // gray background
        
        drawDirTreeRecursive(root);
        // for (const auto& i : _entities) 
        //     if(i->transform->isRoot())
        //         drawHierarchyTreeRecursive(i.get()); // Call recursively to populate each level of children

        ImGui::PopStyleColor(1);

        ImGui::TreePop();  // This is required at the end of the if block
    }
}

void ContentBrowser::drawFolderContent()
{
    if (selectedDir->children.empty()) {
        ImGui::Text("Nothing to shown here.");
        // geri dönüş butonu basılabilir belki 
        return;
    }

    // --- Grid Hesaplamaları ---
    float padding = 16.0f;
    float cellSize = _thumbnailSize + padding;
    float panelWidth = ImGui::GetContentRegionAvail().x;

    int columnCount = static_cast<int>(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    // Table kullanarak ızgara yapısını başlatıyoruz
    if (ImGui::BeginTable("ContentGrid", columnCount)) {
        
        for (const auto& item : selectedDir->children) {
            ImGui::TableNextColumn();
            
            // ID Çakışmasını önlemek için pointer adresini push'luyoruz
            ImGui::PushID(item.get());

            // Resim ve metni bir grup olarak topluyoruz
            ImGui::BeginGroup();

            // 1. İKON / RESİM
            std::string iconType;
            switch (item->type)
            {
            case ContentType::Directory:
                iconType = item->children.empty() ?
                    Builtin::Icon::ContentBrowser::EmptyFolder :
                    Builtin::Icon::ContentBrowser::Folder;
                break;
            case ContentType::Image:
                iconType = Builtin::Icon::ContentBrowser::Texture;
                break;
            case ContentType::Model:
                iconType = Builtin::Icon::ContentBrowser::StaticMesh;
                break;
            case ContentType::Scene:
                iconType = Builtin::Icon::ContentBrowser::Scene;
                break;
            case ContentType::Material:
                iconType = Builtin::Icon::ContentBrowser::Material;
                break;
            default:
                iconType = Builtin::Icon::ContentBrowser::Unknown;
                break;
            }
                
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.0f)); // Olive Green
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

            if (ImGui::ImageButton(
                    "t",
                    (ImTextureID)(intptr_t)ece->assets.get<Texture>(iconType)->getId(),
                    ImVec2(_thumbnailSize, _thumbnailSize)))
            {
            }    

            ImGui::PopStyleColor(3);
            

            // 2. İSİM (Metin)
            // Dosya adını ikonun altına ortalı/sığacak şekilde yazdırıyoruz
            ImGui::PushItemWidth(_thumbnailSize);
            ImGui::SetWindowFontScale(1.0f + (_thumbnailSize-80.f)/48.f*0.2f); // Yazıyı küçültür
            ImGui::TextWrapped("%s", item->name.c_str());
            ImGui::SetWindowFontScale(1.0f); // Eski haline geri getir (ÇOK ÖNEMLİ)
            ImGui::PopItemWidth();

            // ilerisi için : 
            // ImGui::PushFont(ece->fonts.smallFont);
            // ImGui::TextWrapped("%s", item->name.c_str());
            // ImGui::PopFont();


            ImGui::EndGroup(); // Grubu bitir

            // --- İŞLEMLER (Interaction) ---

            // Hover Edilince Tooltip Göster
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Adı: %s", item->name.c_str());
                ImGui::Text("Yol: %s", item->path.string().c_str());
                ImGui::Text("Tip: %s", fileTypeString(item->type).c_str());
                ImGui::EndTooltip();
            }

            // Çift Tıklama Kontrolü (Klasörün İçine Girme)
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (item->isDir) {
                    selectedDir = item; // Seçili klasörü değiştirip içine giriyoruz!
                } else {                    
                    if(item->type == ContentType::Model)
                    {   
                        Event e{
                            EventType::ModelOpened, 
                            std::make_unique<EventData_Text>(item->path.string())
                        };
                        ece->dispatcher.dispatch(e);
                    }
                    else if(item->type == ContentType::Text)
                    {
                        Event e{
                            EventType::OpenTextViewer, 
                            std::make_unique<EventData_Text>(item->path.string())
                        };
                        ece->dispatcher.dispatch(e);
                    }
                    else if(item->type == ContentType::Image)
                    {
                        Event e{
                            EventType::OpenImageViewer, 
                            std::make_unique<EventData_Text>(item->path.string())
                        };
                        ece->dispatcher.dispatch(e);
                    }
                    else if(item->type == ContentType::Scene)
                    {
                        Event e{
                            EventType::LoadScene, 
                            std::make_unique<EventData_Text>(item->path.string())
                        };
                        ece->dispatcher.dispatch(e);
                    }
                }
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void ContentBrowser::draw(){
    if (ImGui::Begin("CONTENT BROWSER")) {
        // menu bar
        if(ImGui::Button("Refresh")) scan(); ImGui::SameLine();
        ImGui::Dummy({60,0}); ImGui::SameLine();
        breadcrumb();
            
        // Boşluk bırak
        // Sağ gruptaki butonların toplam genişlik hesabı
        float styleSpacing = ImGui::GetStyle().ItemSpacing.x;
        float framePadding = ImGui::GetStyle().FramePadding.x * 2.0f;

        float rightGroupWidth = (ImGui::CalcTextSize("Up").x + framePadding) +
                                (ImGui::CalcTextSize("-").x + framePadding) +
                                (160.0f /*+ framePadding*/) + 
                                (ImGui::CalcTextSize("+").x + framePadding) +
                                (styleSpacing * 3.0f);

        // İmleci pencerenin sağ kenarına göre hizala
        float rightPosX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - rightGroupWidth;
        ImGui::SetCursorPosX(rightPosX); 

        // --- Up folder buton 
        if( ImGui::Button("Up") && !(selectedDir->parent.expired()) ){
            selectedDir = selectedDir->parent.lock();
        }

        // --- Üst Araç Çubuğu: Simge Boyutu Ayarı ---
        ImGui::SameLine();
        if( ImGui::Button("-") ){
            _thumbnailSize -= 16; 
            if(_thumbnailSize<32) _thumbnailSize = 32;
        }
        
        ImGui::SameLine();
        ImGui::PushItemWidth(160.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f)); // Çerçeveyi inceltip çizgiye yaklaştırır
        if(ImGui::SliderInt("##ThumbnailSize", &_thumbnailSize,  32.0f, 128.0f, "Icon: %dpx")){
            int step = 16;
            _thumbnailSize = ((_thumbnailSize + step / 2) / step) * step; // En yakın 16'nın katına yuvarlar
        }
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if( ImGui::Button("+") ){
            _thumbnailSize += 16; 
            if(_thumbnailSize>128) _thumbnailSize = 128;
        }

        ImGui::Separator();

        // --- Boyut Hesaplamaları ---
        // Sol panel için sabit veya oransal bir genişlik belirleyebilirsin
        static float leftPanelWidth = 220.0f; 

        // Sınırlandırmalar (Panellerin tamamen kaybolmaması için)
        const float minWidth = 120.0f;
        const float maxWidth = ImGui::GetContentRegionAvail().x - 120.0f;
        const float splitterThickness = 4.0f;


        // --- SOL PANEL: Klasör Ağacı (Tree View) ---
        ImGui::BeginChild("FolderTreePane", ImVec2(leftPanelWidth, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar); {
            drawTree(); 
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // --- SPLITTER (Sürüklenebilir Çizgi) ---
        // separator için button'a renk ataması yaptık
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.369f, 0.243f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.333f, 0.420f, 0.184f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.243f, 0.557f, 0.255f, 1.0f));
        ImGui::Button("##splitter", ImVec2(splitterThickness, -1.0f));
        ImGui::PopStyleColor(3);

        // Sürükleme İşlemi 
        if (ImGui::IsItemActive()) {
            leftPanelWidth += ImGui::GetIO().MouseDelta.x;

            if (leftPanelWidth < minWidth) leftPanelWidth = minWidth;
            if (leftPanelWidth > maxWidth) leftPanelWidth = maxWidth;
        }

        // Fare ayırıcının üzerindeyken imleci sağa-sola büyütme ikonuna çeviriyoruz
        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        ImGui::SameLine();

        // --- SAĞ PANEL: Klasör İçeriği (Grid / List View) ---
        ImGui::BeginChild("FolderContentPane", ImVec2(0.0f, 0.0f), true); {
            drawFolderContent(); 
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void ContentBrowser::init(EngineContext* ece){
    this->ece = ece;

    root = std::make_shared<ContentItem>();

    root->name = "root"; 
    root->path = ece->paths.contentFolder;
    root->type = ContentType::Directory;
    root->isDir = true;
    
    selectedDir = root;
    scan();
}
