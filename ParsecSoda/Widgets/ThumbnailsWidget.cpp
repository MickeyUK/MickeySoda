#include "ThumbnailsWidget.h"

bool ThumbnailsWidget::render(ParsecSession& session, vector<Thumbnail>& _thumbnails)
{
    static bool updating = false;

    ImGui::SetNextWindowSizeConstraints(ImVec2(250, 200), ImVec2(1000, 1000));
    AppStyle::pushTitle();
    ImGui::Begin("Arcade Room Thumbnails");
    AppStyle::pushInput();
    
    static const uint32_t REFRESH_COOLDOWN = MINUTES(1);
    static uint32_t now, next;
    static float fill = 0;
    static Debouncer debouncer = Debouncer(500, [&]() {
        updating = true;
        next = now + REFRESH_COOLDOWN;
        session.fetchArcadeRoomListSync();
        updating = false;
    });

    now = Clock::now();

    if (next <= now)
    {
        next = now + REFRESH_COOLDOWN;
        debouncer.start();
    }
    fill = 1.0f - ((float)(next - now) / REFRESH_COOLDOWN);

    static ImVec2 progressSize = ImVec2(32, 32);
    static ImVec2 cursor;
    cursor = ImGui::GetCursorPos();
    ProgressCircularWidget::render(progressSize.x/2, progressSize.x/4, fill);
    ImGui::SetCursorPos(cursor);
    if (ImGui::Button("###Thumb Refresh Button", progressSize))
    {
        debouncer.start();
    }
    TitleTooltipWidget::render("Next refresh", "Click to force refresh of public room's thumbnails.");

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));

    static const char* thumbPopup = "Edit Thumbnail Name";
    static uint32_t popupIndex = 0;
    static string popupGameid = "";
    static char thumbEditName[256] = "";
    static bool showPopup = false;

    if (!updating)
    {
        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, IM_COL32(0, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_TableRowBg, IM_COL32(0, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, IM_COL32(10, 10, 10, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
        int columns = floor(ImGui::GetWindowWidth() / 275);
        if (columns > 3) columns = 3;
        if (columns < 1) columns = 1;

        if (ImGui::BeginTable("table_thumbs", columns, ImGuiTableFlags_RowBg))
        //if (ImGui::BeginTable("table_thumbs", columns))
        {
            ImGui::TableNextRow();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
            //vector<Thumbnail>::iterator it;
            //static uint32_t index = 0;
            //index = 0;

            for (size_t i=0; i<_thumbnails.size(); i++)
            //for (it = _thumbnails.begin(); it != _thumbnails.end(); ++it)
            {
                ImGui::TableNextColumn();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

                //ImGui::PushID((string("### Thumb Save button") + to_string(index)).c_str());
                ImGui::PushID((string("### Thumb Save button") + to_string(i)).c_str());
                if (IconButton::render(
                    _thumbnails[i].saved ? AppIcons::saveOn : AppIcons::saveOff,
                    _thumbnails[i].saved ? AppColors::positive : AppColors::negative,
                    //(*it).saved ? AppIcons::saveOn : AppIcons::saveOff,
                    //(*it).saved ? AppColors::positive : AppColors::negative,
                    ImVec2(25, 25)
                ))
                {
                    //(*it).saved = !(*it).saved;
                    _thumbnails[i].saved = !_thumbnails[i].saved;
                    static Debouncer saveDebouncer = Debouncer(1000, [&]() {
                        session.saveThumbnails();
                        });
                    saveDebouncer.start();
                }
                ImGui::PopID();
                ImGui::SameLine();
                //ImGui::PushID((string("### Thumb Edit button") + to_string(index)).c_str());
                ImGui::PushID((string("### Thumb Edit button") + to_string(i)).c_str());
                if (IconButton::render(
                    AppIcons::edit,
                    AppColors::primary,
                    ImVec2(25, 25)
                ))
                {
                    //(*it).edit = !(*it).edit;
                    _thumbnails[i].edit = !_thumbnails[i].edit;
                    //if ((*it).edit)
                    if (_thumbnails[i].edit)
                    {
                        //popupIndex = index;
                        popupIndex = i;
                        //popupGameid = (*it).gameId;
                        popupGameid = _thumbnails[i].gameId;
                        showPopup = true;
                        try
                        {
                            strcpy_s(thumbEditName, 256, _thumbnails[popupIndex].name.c_str());
                        }
                        catch (const std::exception&) {}
                    }
                }
                ImGui::PopID();
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
                //ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
                //ImGui::TextWrapped((*it).name.c_str());
                ImGui::TextWrapped(_thumbnails[i].name.c_str());

                //index++;
            }
            ImGui::PopStyleVar();
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        if (showPopup)
        {
            ImGui::OpenPopup(thumbPopup);
        }
        ImGui::SetNextWindowSize(ImVec2(300, 200));
        AppStyle::pushTitle();
        if (ImGui::BeginPopupModal(thumbPopup, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
            static int width = ImGui::GetWindowContentRegionWidth();
            static bool apply, cancel;

            ImGui::Dummy(ImVec2(0, 5));

            ImGui::SetNextItemWidth(width);
            AppStyle::pushInput();
            apply = ImGui::InputText("### Thumb Edit Input", thumbEditName, 256, ImGuiInputTextFlags_EnterReturnsTrue);
            AppStyle::pop();
        
            ImGui::Dummy(ImVec2(0, 20));
            apply = apply || IconButton::render(AppIcons::yes, AppColors::positive, ImVec2(50, 50));
        
            ImGui::SameLine();
            ImGui::SetCursorPosX(width - 40);
            cancel = IconButton::render(AppIcons::no, AppColors::negative, ImVec2(50, 50));

            if (apply || cancel)
            {
                static vector<Thumbnail>::iterator it2;
                for (it2 = _thumbnails.begin(); it2 != _thumbnails.end(); ++it2)
                {
                    if ((*it2).gameId.compare(popupGameid) == 0)
                    {
                        (*it2).edit = false;

                        if (apply)
                        {
                            (*it2).name = thumbEditName;
                            static Debouncer saveDebouncer = Debouncer(1000, [&]() {
                                session.saveThumbnails();
                            });
                            saveDebouncer.start();
                            break;
                        }
                    }
                }

                showPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        AppStyle::pop();

    }

    AppStyle::pop();
    ImGui::End();
    AppStyle::pop();

    return true;
}