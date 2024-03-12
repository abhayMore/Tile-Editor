#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <unordered_map>
#include <string>

template<>
struct std::hash<std::tuple<int, int, int>> {
    size_t operator()(const std::tuple<int, int, int>  cellCoord) const
    {
        return std::hash<int>()(get<0>(cellCoord)) ^ std::hash<int>()(get<1>(cellCoord)) ^ std::hash<int>()(get<2>(cellCoord));// std::hash<int>()(cellCoord.second.second);
    }
};


class TileLayer
{
private:
    std::unordered_map<std::tuple<int, int, int>, ImVec4> m_cellColors;
    bool m_isVisible;

public:
    TileLayer(bool isVisible = true) : m_isVisible(isVisible)
    {
    }

    void setTile(int pensize, int row, int col, const ImVec4& color) {
        m_cellColors[std::make_tuple(pensize, row, col)] = color;
    }

    ImVec4 getTile(int pensize, int row, int col) const
    {
        auto it = m_cellColors.find(std::make_tuple(pensize, row, col));
        if (it != m_cellColors.end())
            return it->second;
        else
            return ImVec4(0, 0, 0, 0);
    }

    void setVisibility(bool visible) 
    {
        m_isVisible = visible;
    }

    bool getVisibility() const
    {
        return m_isVisible;
    }

    bool& isVisible() 
    {
        return m_isVisible;
    }
};


class Grid
{
private:
    ImVec2 m_canvasSize;
    ImVec2 m_cellSize;
    int m_numRows;
    int m_numCols;
    std::unordered_map<int, TileLayer> m_tileLayers;
    int m_selectedLayer = 1;

public:
    Grid(ImVec2 canvasSize, ImVec2 cellSize) : 
        m_canvasSize(canvasSize), 
        m_cellSize(cellSize), 
        m_numRows(static_cast<int>(m_canvasSize.y / m_cellSize.y)),
        m_numCols(static_cast<int>(m_canvasSize.x / m_cellSize.x)),
        m_tileLayers{ {1, TileLayer()}}
    {
    }

    void render(ImDrawList* drawList, ImVec2 cellSize, int highlightCellX, int highlightCellY, bool showGrid, float gridThickness) {
        ImVec2 windowPos = ImGui::GetCursorScreenPos();

        m_cellSize = cellSize;
        //Draws only visible layers. New Layers are drawn on Top of Old ones
        for (const auto& layer : m_tileLayers)
        {
            if (layer.second.getVisibility() == true)
            {
                for (int row = 0; row < m_numRows; ++row)
                {
                    for (int col = 0; col < m_numCols; ++col)
                    {
                        for (int i = 8; i <= 128; i *= 2)
                        {
                            ImVec4 cellColor = layer.second.getTile(i, row, col);
                            ImVec4 color = { 0,0,0,0 };
                            float cellX = windowPos.x + col * i;
                            float cellY = windowPos.y + row * i;
                            drawList->AddRectFilled(ImVec2(cellX, cellY), ImVec2(cellX + i, cellY + i), ImGui::ColorConvertFloat4ToU32(cellColor));

                        }
                    }
                }
            }
        }
        if (showGrid) {

            // Render horizontal grid lines
            drawList->AddLine(ImVec2(windowPos.x, windowPos.y), ImVec2(windowPos.x + m_canvasSize.x, windowPos.y), IM_COL32(150, 150, 150, 255), gridThickness);
            for (float y = windowPos.y + m_cellSize.y - 1; y < windowPos.y + m_canvasSize.y; y += m_cellSize.y) {
                drawList->AddLine(ImVec2(windowPos.x, y), ImVec2(windowPos.x + m_canvasSize.x, y), IM_COL32(150, 150, 150, 255), gridThickness);
            }

            // Render vertical grid lines
            drawList->AddLine(ImVec2(windowPos.x, windowPos.y), ImVec2(windowPos.x, windowPos.y + m_canvasSize.y), IM_COL32(150, 150, 150, 255), gridThickness);
            for (float x = windowPos.x + m_cellSize.x - 1; x < windowPos.x + m_canvasSize.x; x += m_cellSize.x) {
                drawList->AddLine(ImVec2(x, windowPos.y), ImVec2(x, windowPos.y + m_canvasSize.y), IM_COL32(150, 150, 150, 255), gridThickness);
            }
        }
               
    }
    void setCellColor(int pensize, int row, int col, const ImVec4& color) {

        //Mouse draws only on selected layer ID.
        //selected layer ID is first searched in TileLayers to be modified.
        if (m_selectedLayer != -1)
        {
            auto it = m_tileLayers.find(m_selectedLayer);
            if (it != m_tileLayers.end())
            {
                it->second.setTile(pensize, row, col, color);
            }
        }
    }

    void drawLayerWindow() {
        ImGui::SetNextWindowSizeConstraints(ImVec2(250, -1), ImVec2(FLT_MAX, -1));
        ImGui::Begin("Layers", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        for (auto it = m_tileLayers.begin(); it != m_tileLayers.end(); ++it)
        {
            int layerNumber = it->first; 
            bool isSelected = (m_selectedLayer == layerNumber);

            ImGui::Checkbox(("##" + std::to_string(layerNumber)).c_str(), &(it->second.isVisible()));

            ImGui::SameLine();
            if (ImGui::Selectable(("Layer : " + std::to_string(layerNumber)).c_str(), isSelected)) {
                m_selectedLayer = layerNumber;
            }
            
        }
        if (ImGui::Button("Add"))
        {

            for (int i = 1; i <= m_tileLayers.size() + 1; i++)
            {
                if (m_tileLayers.find(i) == m_tileLayers.end())
                {
                    m_tileLayers.insert({ i, TileLayer() });
                    m_selectedLayer = i;
                    break;
                }
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Delete"))
        {
            auto it = m_tileLayers.find(m_selectedLayer);
            if (it != m_tileLayers.end())
            {
                auto prevIt = (it == m_tileLayers.begin()) ? m_tileLayers.end() : std::prev(it);
                auto layer = m_tileLayers.erase(it);
                if (layer != m_tileLayers.end())
                {
                    m_selectedLayer = layer->first;
                }
                else 
                {
                    if (prevIt != m_tileLayers.end())
                    {
                        m_selectedLayer = prevIt->first;
                    }
                    else
                    {
                        m_selectedLayer = 0;
                    }
                }               
            }
            
        }
        ImGui::End();
    }
};

int main() 
{
    sf::RenderWindow window(sf::VideoMode(1200, 900), "Tile Editor");

    ImGui::SFML::Init(window);

    ImVec2 canvasSize(25 * 32, 20 * 32); 
    int cellSizePixel = 8;
    ImVec2 cellSize(cellSizePixel, cellSizePixel);

    //Cell Size init
    std::string cellSizeLabel[] = {"1x", "2x", "4x", "8x", "16x"};
    float selectedCellSize = 0;


    Grid grid(canvasSize, cellSize);

    //Grid Thickness init
    const char* gridThicknessLabel[] = { "1x", "2x", "3x", "4x" };
    float selectedGridThickness = 0;

    //Pen Size init
    const char* penSizeLabel[] = { "1x", "2x", "3x", "4x" };
    int selectedPenSize = 0;
    ImVec2 penSize(cellSizePixel, cellSizePixel);

    // Default selected color in Color Palette
    ImVec4 selectedColor(1.0f, 0.0f, 0.0f, 1.0f); 
    int highlightCellX = -1; 
    int highlightCellY = -1;

    ImVec2 windowPos;
    bool showGrid = true;
    bool m_mouseButtonPressed = false;
    bool m_leftMouseButtonPressed = false;
    bool m_rightMouseButtonPressed = false;

    sf::Clock deltaTime;
    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) 
        {
            ImGui::SFML::ProcessEvent(event);

            switch (event.type)
            {
            case sf::Event::Closed:
            {
                window.close();
            }
            break;

            case sf::Event::MouseButtonPressed:
            {
                m_mouseButtonPressed = true;
                switch (event.mouseButton.button)
                {
                case sf::Mouse::Left:
                {
                    m_leftMouseButtonPressed = true;
                }
                break;

                case sf::Mouse::Right:
                {
                    m_rightMouseButtonPressed = true;
                }
                break;

                default:
                    break;
                }
            }
            break;

            case sf::Event::MouseButtonReleased:
            {
                m_mouseButtonPressed = false;
                switch (event.mouseButton.button)
                {
                case sf::Mouse::Left:
                {
                    m_leftMouseButtonPressed = false;
                }
                break;

                case sf::Mouse::Right:
                {
                    m_rightMouseButtonPressed = false;              
                }
                break;

                default:
                    break;
                }
            }
            break;

            default:
                break;
            }
        }

        ImGui::SFML::Update(window, deltaTime.restart());

        window.clear(sf::Color(18, 33, 43));

        //GRID WINDOW
        ImGui::Begin("Tile Grid", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::BeginChild("GridChild", ImVec2(canvasSize.x, canvasSize.y), false, ImGuiWindowFlags_NoScrollbar);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        grid.render(drawList, cellSize, highlightCellX, highlightCellY, showGrid, selectedGridThickness + 1);
        grid.drawLayerWindow();

        
        if (m_mouseButtonPressed && showGrid)
        {
            windowPos = ImGui::GetCursorScreenPos();
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
          
            if (mousePos.x >= windowPos.x && mousePos.x < windowPos.x + canvasSize.x &&
                mousePos.y >= windowPos.y && mousePos.y < windowPos.y + canvasSize.y) {
                
                for (int i = -1; i <= -1 + selectedPenSize; ++i)
                {
                    for (int j = -1; j <= -1 + selectedPenSize; ++j)
                    {
                        highlightCellX = static_cast<int>((mousePos.x - windowPos.x) / penSize.x);
                        highlightCellY = static_cast<int>((mousePos.y - windowPos.y) / penSize.y);

                        if (m_leftMouseButtonPressed && ImGui::IsWindowHovered())
                            grid.setCellColor(penSize.x, highlightCellY + i + 1, highlightCellX + j + 1, selectedColor);
                        else if (m_rightMouseButtonPressed && ImGui::IsWindowHovered())
                            grid.setCellColor(penSize.x, highlightCellY + i + 1, highlightCellX + j + 1, { 0,0,0,0 });

                    }
                }
                
            }
        }
        ImGui::EndChild();
        ImGui::End();

        //EDIT PANEL WINDOW
        ImGui::Begin("Edit Panel");
        ImGui::ColorEdit4("Selected Color", reinterpret_cast<float*>(&selectedColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::Checkbox("Show Grid", &showGrid);


        // Cell Size
        ImGui::Text(("Cell Size (x = " + std::to_string(static_cast<int>(cellSize.x)) + ")").c_str());
        for (int i = 0; i < sizeof(cellSizeLabel) / sizeof(cellSizeLabel[0]); ++i) {
            ImGui::Selectable(cellSizeLabel[i].c_str(), selectedCellSize == i, ImGuiSelectableFlags_None, ImVec2(15, 0));
            if (ImGui::IsItemClicked()) {
                selectedCellSize = i;
                selectedPenSize = 0;
                cellSize = ImVec2(cellSizePixel * std::stoi(cellSizeLabel[i], 0), cellSizePixel * std::stoi(cellSizeLabel[i], 0));
                penSize = cellSize;
            }
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();

        }
        ImGui::Spacing();

        // Grid Thickness
        ImGui::Text(("Grid Thickness (x = " + std::to_string(static_cast<int>(1.0f)) + ")").c_str());
        for (int i = 0; i < sizeof(gridThicknessLabel) / sizeof(gridThicknessLabel[0]); ++i) {
            ImGui::Selectable(gridThicknessLabel[i], selectedGridThickness == i, ImGuiSelectableFlags_None, ImVec2(15, 0));
            if (ImGui::IsItemClicked()) {
                selectedGridThickness = i;
            }
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();

        }
        ImGui::Spacing();

        // Pen Size
        ImGui::Text(("Pen Size (x = " + std::to_string(static_cast<int>(penSize.x)) + ")").c_str());
        for (int i = 0; i < sizeof(penSizeLabel) / sizeof(penSizeLabel[0]); ++i) {
            ImGui::Selectable(penSizeLabel[i], selectedPenSize == i, ImGuiSelectableFlags_None, ImVec2(15, 0));
            if (ImGui::IsItemClicked()) {
                selectedPenSize = i;
            }
            ImGui::SameLine();
            ImGui::Spacing(); 
            ImGui::SameLine();

        }

        ImGui::End();

        ImGui::SFML::Render(window);

        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
