#include "LogWidget.h"

LogWidget::LogWidget(Hosting& hosting)
    : _commandLog(hosting.getCommandLog()), _messageCount(0)
{
}

bool LogWidget::render()
{
    AppStyle::pushTitle();
    ImGui::SetNextWindowSizeConstraints(ImVec2(150, 150), ImVec2(800, 900));
    ImGui::Begin("Log", (bool*)0);
    AppStyle::pushInput();

    ImVec2 size = ImGui::GetContentRegionAvail();

    if (_commandLog.size() > CHATLOG_COMMAND_LENGTH)
    {
        vector<string>::iterator it = _commandLog.begin();
        _commandLog.erase(it, it + CHATLOG_COMMAND_LENGTH/2);
    }

    ImGui::BeginChild("Log text", ImVec2(size.x, size.y));
    for (size_t i = 0; i < _commandLog.size(); ++i)
    {
        if (_commandLog[i][0] == '@')
        {
            //AppStyle::pushNotice();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.0f, 1.00f));
            ImGui::TextWrapped(_commandLog[i].substr(1).c_str());
            ImGui::PopStyleColor();
            //AppStyle::pop();
        }
        else if (_commandLog[i][0] == '!')
        {
            //AppStyle::pushNegative();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.16f, 0.28f, 1.00f));
            ImGui::TextWrapped(_commandLog[i].substr(1).c_str());
            ImGui::PopStyleColor();
            //AppStyle::pop();
        }
        else
        {
            ImGui::TextWrapped(_commandLog[i].c_str());
        }
    }
    if (_messageCount != _commandLog.size())
    {
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 100)
        {
            ImGui::SetScrollHereY(1.0f);
        }
        _messageCount = _commandLog.size();
    }
    ImGui::EndChild();

    AppStyle::pop();
    ImGui::End();
    AppStyle::pop();

    return true;
}