#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>

class TileLayer
{
private:
    std::vector<std::vector<ImVec4>> m_cellColors;
    bool m_isVisible;

public:
    TileLayer(int numRows, int numCols, bool isVisible = true) : m_isVisible(isVisible)
    {
        m_cellColors.resize(numRows, std::vector<ImVec4>(numCols));

    }

    void setTile(int row, int col, const ImVec4& color) {
        if (row >= 0 && row < m_cellColors.size() && col >= 0 && col < m_cellColors[0].size()) {

            m_cellColors[row][col] = color;
        }
    }

    ImVec4 getTile(int row, int col) const
    {
        return m_cellColors[row][col];
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
    std::map<int, TileLayer> m_tileLayers;
    int m_selectedLayer = 1;

    
public:
    Grid(ImVec2 canvasSize, ImVec2 cellSize) : 
        m_canvasSize(canvasSize), 
        m_cellSize(cellSize), 
        m_numRows(static_cast<int>(m_canvasSize.y / m_cellSize.y)),
        m_numCols(static_cast<int>(m_canvasSize.x / m_cellSize.x)),
        m_tileLayers{ {1, TileLayer(m_numRows, m_numCols)}}
    {
    }

    void render(ImDrawList* drawList, int highlightCellX, int highlightCellY, bool showGrid) const {
        ImVec2 windowPos = ImGui::GetCursorScreenPos();

        //Draws only visible layers. New Layers are drawn on Top of Old ones
        for (const auto& layer : m_tileLayers)
        {
            if (layer.second.getVisibility() == true)
            {
                for (int row = 0; row < m_numRows; ++row)
                {
                    for (int col = 0; col < m_numCols; ++col)
                    {
                        ImVec4 cellColor = layer.second.getTile(row, col);
                        float cellX = windowPos.x + col * m_cellSize.x;
                        float cellY = windowPos.y + row * m_cellSize.y;
                        drawList->AddRectFilled(ImVec2(cellX, cellY), ImVec2(cellX + m_cellSize.x, cellY + m_cellSize.y), ImGui::ColorConvertFloat4ToU32(cellColor));

                    }
                }
            }
        }
        if (showGrid) {

            // Render horizontal grid lines
            drawList->AddLine(ImVec2(windowPos.x, windowPos.y), ImVec2(windowPos.x + m_canvasSize.x, windowPos.y), IM_COL32(150, 150, 150, 255));
            for (float y = windowPos.y + m_cellSize.y - 1; y < windowPos.y + m_canvasSize.y; y += m_cellSize.y) {
                drawList->AddLine(ImVec2(windowPos.x, y), ImVec2(windowPos.x + m_canvasSize.x, y), IM_COL32(150, 150, 150, 255));
            }

            // Render vertical grid lines
            drawList->AddLine(ImVec2(windowPos.x, windowPos.y), ImVec2(windowPos.x, windowPos.y + m_canvasSize.y), IM_COL32(150, 150, 150, 255));
            for (float x = windowPos.x + m_cellSize.x - 1; x < windowPos.x + m_canvasSize.x; x += m_cellSize.x) {
                drawList->AddLine(ImVec2(x, windowPos.y), ImVec2(x, windowPos.y + m_canvasSize.y), IM_COL32(150, 150, 150, 255));
            }
        }
               
    }
    void setCellColor(int row, int col, const ImVec4& color) {

        //Mouse draws only on selected layer ID.
        //selected layer ID is first searched in TileLayers to be modified.
        if (m_selectedLayer != -1)
        {
            auto it = m_tileLayers.find(m_selectedLayer);
            if (it != m_tileLayers.end())
            {
                it->second.setTile(row, col, color);
            }
        }
    }

    void drawLayerWindow() {
        ImGui::SetNextWindowSizeConstraints(ImVec2(250, -1), ImVec2(FLT_MAX, -1));
        ImGui::Begin("Layers", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        for (auto it = m_tileLayers.begin(); it != m_tileLayers.end(); ++it) {
            int layerNumber = it->first; 
            bool isSelected = (m_selectedLayer == layerNumber);

            ImGui::Checkbox(("##" + std::to_string(layerNumber)).c_str(), &(it->second.isVisible()));

            ImGui::SameLine();
            if (ImGui::Selectable(("Layer : " + std::to_string(layerNumber)).c_str(), isSelected)) {
                m_selectedLayer = layerNumber;
            }
            
        }
        if (ImGui::Button("Add Layer")) {
            int newLayerNumber = m_tileLayers.size() + 1;
            m_tileLayers.insert({ newLayerNumber, TileLayer(m_numRows, m_numCols) });
            m_selectedLayer = newLayerNumber;
        }

        ImGui::End();
    }
};

int main() 
{
    sf::RenderWindow window(sf::VideoMode(1200, 900), "Tile Editor");

    ImGui::SFML::Init(window);

    ImVec2 canvasSize(800, 640); 
    ImVec2 cellSize(32, 32); 

    Grid grid(canvasSize, cellSize);

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
        grid.render(drawList, highlightCellX, highlightCellY, showGrid);
        grid.drawLayerWindow();
        if (m_mouseButtonPressed)
        {
            windowPos = ImGui::GetCursorScreenPos();
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            if (mousePos.x >= windowPos.x && mousePos.x < windowPos.x + canvasSize.x &&
                mousePos.y >= windowPos.y && mousePos.y < windowPos.y + canvasSize.y) {
                highlightCellX = (mousePos.x - windowPos.x) / cellSize.x;
                highlightCellY = (mousePos.y - windowPos.y) / cellSize.y;

                if(m_leftMouseButtonPressed && ImGui::IsWindowHovered())
                    grid.setCellColor(highlightCellY, highlightCellX, selectedColor);
                else if(m_rightMouseButtonPressed && ImGui::IsWindowHovered())
                    grid.setCellColor(highlightCellY, highlightCellX, {0,0,0,0});

            }
        }
        ImGui::EndChild();
        ImGui::End();

        //EDIT PANEL WINDOW
        ImGui::Begin("Color Palette");
        ImGui::ColorEdit4("Selected Color", reinterpret_cast<float*>(&selectedColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::Checkbox("Show Grid", &showGrid);

        ImGui::End();

        ImGui::SFML::Render(window);

        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
