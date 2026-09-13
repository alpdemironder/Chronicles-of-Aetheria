#pragma once

#include "UIRenderer.hpp"
#include <string>
#include <vector>
#include <functional>

namespace Aetheria {

enum class UpdateCategory {
    All,
    MajorRelease,
    Roadmap,
    SeasonalEvent
};

struct CalendarEvent {
    std::string version;
    std::string title;
    std::string targetDate;
    std::string status;
    Vec4 statusColor{0.3f, 0.85f, 1.0f, 1.0f};
    std::string description;
    std::vector<std::string> highlights;
    std::string techDetails;
    int progressPercent = 100;
    UpdateCategory category = UpdateCategory::MajorRelease;
    int year = 2026;
    int month = 9;
    int day = 13;
};

class UpdateCalendarUI {
public:
    UpdateCalendarUI(UIRenderer* ui);

    void open() { isOpen = true; }
    void close() { isOpen = false; }
    void toggle() { isOpen = !isOpen; }
    bool getIsOpen() const { return isOpen; }

    void render(int screenWidth, int screenHeight,
                int mouseX, int mouseY, bool mouseLeftDown, bool mouseLeftClicked,
                float totalTime);

    void setViewMode(int mode) { viewMode = mode; } // 0: Timeline, 1: Monthly Calendar
    int getViewMode() const { return viewMode; }

    void nextMonth();
    void prevMonth();

private:
    UIRenderer* ui = nullptr;
    bool isOpen = false;
    int viewMode = 0; // 0 = Roadmap Timeline, 1 = Monthly Calendar Grid
    UpdateCategory currentCategory = UpdateCategory::All;
    int selectedEventIdx = 2; // Default to v2.4 (Current Live Build)

    int calendarYear = 2026;
    int calendarMonth = 9; // September

    std::vector<CalendarEvent> events;

    void initEvents();
    std::vector<int> getFilteredIndices() const;

    bool drawButton(float x, float y, float w, float h,
                    const std::string& text, bool active,
                    int mouseX, int mouseY, bool clicked,
                    const Vec4& accent = {0.2f, 0.55f, 0.9f, 1.0f});

    void renderRoadmapTimeline(float panelX, float panelY, float panelW, float panelH,
                               int mouseX, int mouseY, bool clicked, float totalTime);

    void renderCalendarGrid(float panelX, float panelY, float panelW, float panelH,
                            int mouseX, int mouseY, bool clicked, float totalTime);
};

} // namespace Aetheria
