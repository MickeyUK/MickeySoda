#include "GamepadsWidget.h"
#include "../Hosting.h"
extern Hosting g_hosting;

GamepadsWidget::GamepadsWidget(Hosting& hosting)
    : _hosting(hosting), _gamepads(hosting.getGamepads())
{
}

bool GamepadsWidget::render()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 5));

    static vector<AnimatedGamepadWidget*> agws;

    static ImVec2 cursor;
    static bool isWindowLocked = false;
    static bool isConnectionButtonPressed = false;
    static ImVec2 dummySize = ImVec2(0.0f, 5.0f);

    AppStyle::pushTitle();
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 150), ImVec2(800, 1100));
    ImGui::Begin("Virtual Gamepads", (bool*)0, isWindowLocked ? (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize) : 0);
    AppStyle::pushInput();
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    static ImVec2 size;
    size = ImGui::GetContentRegionAvail();

    //static float indentDistance;

    static bool resetting = false;
    if (IconButton::render(AppIcons::refresh, AppColors::primary, ImVec2(24, 24)))
    {
        if (!resetting)
        {
            resetting = true;
            _hosting.getGamepadClient().resetAll(resetting);
        }
    }
    TitleTooltipWidget::render("Reset gamepad engine", "If all else fails, try this button.\nPress in dire situations.");
    
    ImGui::SameLine();

    if (IconButton::render(AppIcons::padOff, AppColors::primary, ImVec2(24, 24)))
    {
        _hosting.getGamepadClient().disconnectAllGamepads();
    }
    TitleTooltipWidget::render("Disconnect all gamepads", "Disconnects all of the gamepads currently enabled.");

    ImGui::SameLine();
    
    if (IconButton::render(AppIcons::sort, AppColors::primary, ImVec2(24, 24)))
    {
        _hosting.getGamepadClient().sortGamepads();
    }
    TitleTooltipWidget::render("Sort gamepads", "Re-sort all gamepads by index.");

    ImGui::SameLine();

    if (ToggleIconButtonWidget::render(
        AppIcons::lock, AppIcons::unlock, _hosting.isGamepadLock(),
        AppColors::negative, AppColors::positive, ImVec2(24, 24)
    ))
    {
        _hosting.toggleGamepadLock();
    }
    if (_hosting.isGamepadLock())   TitleTooltipWidget::render("Unlock guest inputs", "Guests will be able to control gamepads again.");
    else                            TitleTooltipWidget::render("Lock guest inputs", "Guest inputs will be locked out of gamepads.");

    ImGui::SameLine();

    if (ToggleIconButtonWidget::render(
        AppIcons::buttonLock, AppIcons::buttonLock, _hosting.isGamepadLockButtons(),
        AppColors::negative, AppColors::positive, ImVec2(24, 24)
    ))
    {
        _hosting.toggleGamepadLockButtons();
    }
    if (_hosting.isGamepadLockButtons())
        TitleTooltipWidget::render("Unlock buttons", "Make sure you set it up");
    else
        TitleTooltipWidget::render("Lock buttons", "Make sure you set it up");

    //ImGui::SameLine();

    //cursor = ImGui::GetCursorPos();
    //ImGui::SetCursorPosX(size.x - 25);
    //if (IconButton::render(
    //    AppIcons::move,
    //    isWindowLocked ? AppColors::negative : AppColors::positive,
    //    ImVec2(30, 30)
    //))
    //{
    //    isWindowLocked = !isWindowLocked;
    //}
    //if (isWindowLocked) TitleTooltipWidget::render("Window Locked", "This window cannot move or resize.");
    //else TitleTooltipWidget::render("Window Unlocked", "This window can move and resize.");

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10));

    static int
        xboxCount = (int) MetadataCache::preferences.xboxPuppetCount,
        lastXboxCount = (int) MetadataCache::preferences.xboxPuppetCount,
        ds4Count = (int) MetadataCache::preferences.ds4PuppetCount,
        lastDs4Count = (int) MetadataCache::preferences.ds4PuppetCount;

    ImGui::Image(AppIcons::xinput, ImVec2(26, 26), ImVec2(0, 0), ImVec2(1, 1), AppColors::backgroundIcon);
    ImGui::SameLine();
    if (IntRangeWidget::render("xboxCounter", xboxCount, 0, 32))
    {
        TitleTooltipWidget::render("XBox Puppet Counter", "Set the amount of XBox controllers.\n\n* Warning: disconnect all gamepads before changing this.");
    }
    
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(20, 0));
    ImGui::SameLine();
    
    ImGui::Image(AppIcons::dinput, ImVec2(26, 26), ImVec2(0, 0), ImVec2(1, 1), AppColors::backgroundIcon);
    ImGui::SameLine();
    if (IntRangeWidget::render("ds4Counter", ds4Count, 0, 32))
    {
        TitleTooltipWidget::render("Dualshock Puppet Counter", "Set the amount of DS4 controllers.\n\n* Warning: disconnect all gamepads before changing this.");
    }

    ImGui::Dummy(ImVec2(0, 15));

    for (size_t i = 0; i < _gamepads.size(); ++i)
    {
        while (agws.size() <= i) {
            agws.push_back(new AnimatedGamepadWidget(true));
        }
        AnimatedGamepadWidget* agw = agws[i];
        //AnimatedGamepadWidget agw;
        AGamepad* gi = _gamepads[i];
        static uint32_t userID;
        userID = gi->owner.guest.userID;

        ImGui::BeginChild(
            (string("##Gamepad " ) + to_string(i)).c_str(),
            ImVec2(size.x, 50)
        );

        cursor = ImGui::GetCursorPos();
        
        static int xboxIndex = 0, padIndex = 0;
        xboxIndex = (int)gi->getIndex();
        padIndex = xboxIndex + 1;
        static bool isIndexSuccess = false;
        isIndexSuccess = gi->isConnected() && padIndex > 0 && padIndex <= 4;
        WebSocket& ws = g_hosting.getWebSocket();

        ImGui::BeginGroup();
        {
            static vector<ID3D11ShaderResourceView*> xboxIcons { AppIcons::xbox1, AppIcons::xbox2, AppIcons::xbox3, AppIcons::xbox4 };
            cursor = ImGui::GetCursorPos();

            switch (gi->type())
            {
            case AGamepad::Type::XBOX:
                if (isIndexSuccess)
                {
                    ImGui::Image(
                        AppIcons::xbox,
                        ImVec2(45, 45), ImVec2(0, 0), ImVec2(1, 1), AppColors::white
                    );

                    ImGui::SetCursorPos(cursor);
                    ImGui::Image(
                        xboxIcons[xboxIndex],
                        ImVec2(45, 45), ImVec2(0, 0), ImVec2(1, 1), AppColors::primary
                    );
                    TitleTooltipWidget::render(
                        (string() + "XInput " + to_string(padIndex)).c_str(),
                        (
                            string("This controller is using XInput slot ") + to_string(padIndex) + ".\n\n" +
                            "* Remember:\nYour physical controllers may also occupy XInput slots."
                            ).c_str()
                    );

                }
                else
                {
                    ImGui::Image(
                        AppIcons::xinput,
                        ImVec2(45, 45), ImVec2(0, 0), ImVec2(1, 1), AppColors::backgroundIcon
                    );
                    TitleTooltipWidget::render(
                        (string() + "XBox controller").c_str(),
                        (
                            string("This is an XBox controller out of XInput range.") + "\n\n" +
                            "* It still works, but as a generic controller without XInput features."
                            ).c_str()
                    );
                }
                break;
            case AGamepad::Type::DUALSHOCK:
                ImGui::Image(
                    AppIcons::dinput,
                    ImVec2(45, 45), ImVec2(0, 0), ImVec2(1, 1), AppColors::backgroundIcon
                );
                TitleTooltipWidget::render(
                    (string() + "Dualshock controller").c_str(),
                    (
                        string("This is a Dualshock 4 controller.")).c_str()
                );
                break;
            default:
                break;
            }
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        if (IconButton::render(AppIcons::back, AppColors::primary, ImVec2(24, 24)))
        {
            if (gi->isOwned() && ws.connected())
            {
                MTY_JSON* jmsg = MTY_JSONObjCreate();
                MTY_JSONObjSetString(jmsg, "type", "gamepadstrip");
                MTY_JSONObjSetUInt(jmsg, "id", gi->owner.guest.id);
                MTY_JSONObjSetUInt(jmsg, "userid", gi->owner.guest.userID);
                MTY_JSONObjSetString(jmsg, "username", gi->owner.guest.name.c_str());
                MTY_JSONObjSetUInt(jmsg, "index", i);
                char* finmsg = MTY_JSONSerialize(jmsg);
                ws.handle_message(finmsg);
                MTY_JSONDestroy(&jmsg);
                MTY_Free(finmsg);
            }
            gi->clearOwner();
        }
        TitleTooltipWidget::render("Strip gamepad", "Unlink current user from this gamepad.");

        ImGui::SameLine();

        if (ToggleIconButtonWidget::render(AppIcons::padOn, AppIcons::padOff, gi->isConnected(), ImVec2(24, 24)))
        {
            if (gi->isConnected()) gi->disconnect();
            else gi->connect();

            isConnectionButtonPressed = true;
        }
        if (gi->isConnected()) TitleTooltipWidget::render("Connected gamepad", "Press to \"physically\" disconnect\nthis gamepad (at O.S. level).");
        else                  TitleTooltipWidget::render("Disconnected gamepad", "Press to \"physically\" connect\nthis gamepad (at O.S. level).");
    
        ImGui::SameLine();

        if (gi->isLocked())
        {
            if (IconButton::render(AppIcons::lock, AppColors::negative, ImVec2(24, 24)))
            {
                gi->toggleLocked();
            }
            TitleTooltipWidget::render("Locked gamepad", "Unlock this specific gamepad, allowing inputs and picking.");
        }
        else
        {
            if (IconButton::render(AppIcons::unlock, AppColors::positive, ImVec2(24, 24)))
            {
                gi->toggleLocked();
            }
            TitleTooltipWidget::render("Unlocked gamepad", "Lock this specific gamepad, preventing inputs or picking.");
        }

        ImGui::SameLine();

        if (gi->isLockedButtons())
        {
            if (IconButton::render(AppIcons::buttonLock, AppColors::negative, ImVec2(24, 24)))
            {
                gi->toggleLockedButtons();
            }
            TitleTooltipWidget::render("Unlock buttons", "Make sure you set it up");
        }
        else
        {
            if (IconButton::render(AppIcons::buttonLock, AppColors::positive, ImVec2(24, 24)))
            {
                gi->toggleLockedButtons();
            }
            TitleTooltipWidget::render("Lock buttons", "Make sure you set it up");
        }

        ImGui::SameLine();
        
        static float gamepadLabelWidth;
        gamepadLabelWidth = size.x - 210.0f;
        
        ImGui::BeginChild(
            (string("##name ") + to_string(i)).c_str(),
            ImVec2(gamepadLabelWidth, 35.0f)
        );
        cursor = ImGui::GetCursorPos();

        //ImGui::Dummy(ImVec2(0,8));


        static string name, id;
        if (gi->owner.guest.isValid())
        {
            id = string() + "(# " + to_string(gi->owner.guest.userID) + ")\t";
            name = gi->owner.guest.name;
        }
        else if (_hosting.getGamepadClient().isPuppetMaster && gi->isPuppet)
        {
            id = string() + "(# " + to_string(_hosting.getHost().userID) + ")\t";
            name = _hosting.getHost().name;
        }
        else {
            id = "    ";
            name = "    ";
        }

        AppStyle::pushLabel();
        //ImGui::TextWrapped(id.c_str());
        ImGui::Text(id.c_str());
        AppStyle::pop();

        AppFonts::pushInput();
        AppColors::pushPrimary();
        ImGui::SetNextItemWidth(gamepadLabelWidth);
        ImGui::Text(name.c_str());
        AppColors::pop();
        AppFonts::pop();

        static ImVec2 backupCursor;
        backupCursor = ImGui::GetCursorPos();

        ImGui::SetCursorPos(cursor);
        ImGui::Button(
            (string("##gamepad button") + to_string(i + 1)).c_str(),
            //ImVec2(gamepadLabelWidth, 50.0f)
            ImVec2(gamepadLabelWidth, 35.0f)
        );

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("Gamepad", &i, sizeof(int));

            AppFonts::pushInput();
            AppColors::pushPrimary();
            ImGui::Text("%s", (gi->owner.guest.isValid() ? gi->owner.guest.name.c_str() : "Empty gamepad"));
            AppColors::pop();
            AppFonts::pop();

            AppStyle::pushLabel();
            ImGui::Text("Drop into another Gamepad to swap.");
            AppStyle::pop();

            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Guest"))
            {
                if (payload->DataSize == sizeof(int))
                {
                    int guestIndex = *(const int*)payload->Data;
                    if (guestIndex >= 0 && guestIndex < _hosting.getGuestList().size())
                    {
                        MTY_JSON* jmsg = MTY_JSONObjCreate();
                        bool wsConnected = ws.connected();
                        if (wsConnected)
                        {
                            MTY_JSONObjSetString(jmsg, "type", "gamepadassign");
                            MTY_JSONObjSetUInt(jmsg, "fromid", gi->owner.guest.id);
                            MTY_JSONObjSetUInt(jmsg, "fromuserid", gi->owner.guest.userID);
                            MTY_JSONObjSetString(jmsg, "fromusername", gi->owner.guest.name.c_str());
                            MTY_JSONObjSetUInt(jmsg, "index", i);
                        }
                        gi->owner.guest.copy(_hosting.getGuestList()[guestIndex]);
                        if (wsConnected)
                        {
                            MTY_JSONObjSetUInt(jmsg, "toid", gi->owner.guest.id);
                            MTY_JSONObjSetUInt(jmsg, "touserid", gi->owner.guest.userID);
                            MTY_JSONObjSetString(jmsg, "tousername", gi->owner.guest.name.c_str());
                            char* finmsg = MTY_JSONSerialize(jmsg);
                            ws.handle_message(finmsg);
                            MTY_JSONDestroy(&jmsg);
                            MTY_Free(finmsg);
                        }
                    }
                }
            }
            else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Gamepad"))
            {
                if (payload->DataSize == sizeof(int))
                {
                    int sourceIndex = *(const int*)payload->Data;
                    if (sourceIndex >= 0 && sourceIndex < _gamepads.size())
                    {
                        static GuestDevice backupOwner;
                        backupOwner.copy(_gamepads[i]->owner);
                        
                        _gamepads[i]->copyOwner(_gamepads[sourceIndex]);
                        _gamepads[sourceIndex]->owner.copy(backupOwner);
                        _gamepads[i]->clearState();
                        _gamepads[sourceIndex]->clearState();
                        if (ws.connected())
                        {
                            MTY_JSON* jmsg = MTY_JSONObjCreate();
                            MTY_JSONObjSetString(jmsg, "type", "gamepadmove");
                            MTY_JSONObjSetUInt(jmsg, "fromid", backupOwner.guest.id);
                            MTY_JSONObjSetUInt(jmsg, "fromuserid", backupOwner.guest.userID);
                            MTY_JSONObjSetString(jmsg, "fromusername", backupOwner.guest.name.c_str());
                            MTY_JSONObjSetUInt(jmsg, "toid", _gamepads[i]->owner.guest.id);
                            MTY_JSONObjSetUInt(jmsg, "touserid", _gamepads[i]->owner.guest.userID);
                            MTY_JSONObjSetString(jmsg, "tousername", _gamepads[i]->owner.guest.name.c_str());
                            MTY_JSONObjSetUInt(jmsg, "fromindex", sourceIndex);
                            MTY_JSONObjSetUInt(jmsg, "toindex", i);
                            char* finmsg = MTY_JSONSerialize(jmsg);
                            ws.handle_message(finmsg);
                            MTY_JSONDestroy(&jmsg);
                            MTY_Free(finmsg);
                        }
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::SetCursorPos(backupCursor);

        ImGui::EndChild();
        
        ImGui::SameLine();
        
        ImGui::BeginGroup();
        if (_hosting.getGamepadClient().isPuppetMaster && gi->isPuppet)
        {
            //ImGui::Dummy(ImVec2(0, 8));
            ImGui::Image(AppIcons::puppet, ImVec2(35, 35), ImVec2(0, 0), ImVec2(1, 1), AppColors::primary);
            TitleTooltipWidget::render("Puppet", "This gamepad is under control of Master of Puppets.");
        }
        else
        {
            static int deviceIndices[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            //ImGui::Dummy(ImVec2(0.0f, 12.0f));
            ImGui::SetNextItemWidth(40);
            deviceIndices[i] = gi->owner.deviceID;

            AppFonts::pushTitle();
            if (ImGui::DragInt(
                (string("##DeviceIndex") + to_string(i)).c_str(),
                &deviceIndices[i], 0.1f, -1, 65536
            ))
            {
                gi->owner.deviceID = deviceIndices[i];
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            AppFonts::pop();

            TitleTooltipWidget::render("Device index", "A guest may have multiple gamepads in the same machine.");
        }
        ImGui::EndGroup();
        ImGui::EndChild();

        //ImGui::Dummy(ImVec2(0, 2));

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);

        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(48, 0));
        
        ImGui::SameLine();

        //static AnimatedGamepadWidget agw;
        //AnimatedGamepadWidget::render(gi->getState().Gamepad, 30);
        agw->render(gi->getState().Gamepad, 30);

        ImGui::EndGroup();

        ImGui::Dummy(ImVec2(0, 10));
    }

    ImGui::PopStyleVar();
    AppStyle::pop();
    ImGui::End();
    AppStyle::pop();

    ImGui::PopStyleVar();

    static Stopwatch resizeGamepadsDebouncer = Stopwatch(500);

    if (isConnectionButtonPressed)
    {
        isConnectionButtonPressed = false;
        for (size_t i = 0; i < _gamepads.size(); ++i)
        {
            _gamepads[i]->refreshIndex();
        }
    }

    if (lastXboxCount != xboxCount || lastDs4Count != ds4Count)
    {
        if (xboxCount > 32) xboxCount = 32;
        if (ds4Count > 32) ds4Count = 32;
        lastXboxCount = xboxCount;
        lastDs4Count = ds4Count;
        MetadataCache::preferences.xboxPuppetCount = xboxCount;
        MetadataCache::preferences.ds4PuppetCount = ds4Count;
        _hosting.getGamepadClient().resize(xboxCount, ds4Count);
        resizeGamepadsDebouncer.start();
        resizeGamepadsDebouncer.reset(500);
    }
    if (resizeGamepadsDebouncer.isRunning() && resizeGamepadsDebouncer.isFinished())
    {
        resizeGamepadsDebouncer.stop();
        MetadataCache::savePreferences();
    }

    return true;
}