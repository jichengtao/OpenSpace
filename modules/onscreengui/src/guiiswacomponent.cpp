/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2016                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <modules/onscreengui/include/guiiswacomponent.h>
#include <modules/onscreengui/include/renderproperties.h>

#include <openspace/engine/openspaceengine.h>
#include <openspace/rendering/renderengine.h>
#include <openspace/scripting/scriptengine.h>

#include <openspace/properties/scalarproperty.h>
#include <openspace/properties/optionproperty.h>
#include <openspace/properties/selectionproperty.h>
#include <openspace/properties/stringproperty.h>
#include <openspace/properties/vectorproperty.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/misc/assert.h>

#include <ext/json/json.hpp>
#include <openspace/engine/downloadmanager.h>
#include <modules/iswa/util/iswamanager.h>
#include <ghoul/filesystem/filesystem>


#include <fstream>

#include "imgui.h"

namespace {
    using json = nlohmann::json;
    const std::string _loggerCat = "iSWAComponent";
    const ImVec2 size = ImVec2(350, 500);
}

namespace openspace {
namespace gui {
GuiIswaComponent::GuiIswaComponent()
    :GuiPropertyComponent()
    ,_gmdata(false)
    ,_gmimage(false)
    ,_iondata(false)
{}

void GuiIswaComponent::render() {
    bool gmdatavalue = _gmdata;
    bool gmimagevalue = _gmimage;
    bool iondatavalue = _iondata;

    ImGui::Begin("ISWA", &_isEnabled, size, 0.5f);
    // ImGui::Text("Global Magnetosphere");
    ImGui::Checkbox("Global Magnetosphere From Data", &_gmdata);
    ImGui::Checkbox("Global Magnetosphere From Images", &_gmimage);

    // ImGui::Text("Ionosphere");
    ImGui::Checkbox("Ionosphere From Data", &_iondata);


    ImGui::Spacing();
    static const int addCygnetBufferSize = 256;
    static char addCygnetBuffer[addCygnetBufferSize];
    ImGui::InputText("addCynget", addCygnetBuffer, addCygnetBufferSize);

    if(ImGui::SmallButton("Add Cygnet"))
        OsEng.scriptEngine().queueScript("openspace.iswa.addCygnet("+std::string(addCygnetBuffer)+");");
        // IswaManager::ref().setFit(std::stof(std::string(addCygnetBuffer)));

    if(_gmdata != gmdatavalue){
        if(_gmdata){
            std::string x = "openspace.iswa.addCygnet(-4,'Data','Magnetosphere_Data');";
            std::string y = "openspace.iswa.addCygnet(-5,'Data','Magnetosphere_Data');";
            std::string z = "openspace.iswa.addCygnet(-6,'Data','Magnetosphere_Data');";
            OsEng.scriptEngine().queueScript(x+y+z);
            OsEng.scriptEngine().queueScript("openspace.iswa.clearGroupBuildData('Magnetosphere_Data');");
        }else{
            OsEng.scriptEngine().queueScript("openspace.iswa.removeGroup('Magnetosphere_Data');");
        }
    }

    if(_gmimage != gmimagevalue){
        if(_gmimage){
            std::string x = "openspace.iswa.addCygnet(-4,'Texture','Magnetosphere_Image');";
            std::string y = "openspace.iswa.addCygnet(-5,'Texture','Magnetosphere_Image');";
            std::string z = "openspace.iswa.addCygnet(-6,'Texture','Magnetosphere_Image');";
            OsEng.scriptEngine().queueScript(x+y+z);
        }else{
            OsEng.scriptEngine().queueScript("openspace.iswa.removeGroup('Magnetosphere_Image');");
        }
    }

    if(_iondata != iondatavalue){
        if(_iondata){
            OsEng.scriptEngine().queueScript("openspace.iswa.addCygnet(-10,'Data','Ionosphere');");
            OsEng.scriptEngine().queueScript("openspace.iswa.clearGroupBuildData('Ionosphere');");

        }else{
            OsEng.scriptEngine().queueScript("openspace.iswa.removeGroup('Ionosphere');");
        }
    }

#ifdef OPENSPACE_MODULE_ISWA_ENABLED
    if(ImGui::CollapsingHeader("Cdf files")){
        auto cdfInfo = IswaManager::ref().cdfInformation();

        for(auto group : cdfInfo){
            std::string groupName = group.first;
            if(_cdfOptionsMap.find(groupName) == _cdfOptionsMap.end()){
                _cdfOptionsMap[groupName] = -1;
            }

            if(ImGui::CollapsingHeader(groupName.c_str())){
                int cdfOptionValue = _cdfOptionsMap[groupName];
                auto cdfs = group.second;

                for(int i=0; i<cdfs.size(); i++){
                    ImGui::RadioButton(cdfs[i].name.c_str(), &_cdfOptionsMap[groupName], i);
                }

                int cdfOption = _cdfOptionsMap[groupName];
                if(cdfOptionValue != cdfOption){
                   if(cdfOptionValue >= 0){
                        groupName = cdfs[cdfOptionValue].group;
                        // std::cout << groupName << std::endl;
                        // OsEng.scriptEngine().queueScript("openspace.iswa.removeGroup('"+groupName+"');");
                    }

                    std::string path  = cdfs[cdfOption].path;
                    std::string date  = cdfs[cdfOption].date;
                    groupName = cdfs[cdfOption].group;
                    OsEng.scriptEngine().queueScript("openspace.iswa.addKameleonPlanes('"+groupName+"',"+std::to_string(cdfOption)+");");
                    OsEng.scriptEngine().queueScript("openspace.time.setTime('"+date+"');");
                    OsEng.scriptEngine().queueScript("openspace.time.setDeltaTime(0);");
                }
            }
        }
    }
#endif

    for (const auto& p : _propertiesByOwner) {
        if (ImGui::CollapsingHeader(p.first.c_str())) {
            for (properties::Property* prop : p.second) {
                if (_boolProperties.find(prop) != _boolProperties.end()) {
                    renderBoolProperty(prop, p.first);
                    continue;
                }

                if (_intProperties.find(prop) != _intProperties.end()) {
                    renderIntProperty(prop, p.first);
                    continue;
                }

                if (_floatProperties.find(prop) != _floatProperties.end()) {
                    renderFloatProperty(prop, p.first);
                    continue;
                }

                if (_vec2Properties.find(prop) != _vec2Properties.end()) {
                    renderVec2Property(prop, p.first);
                    continue;
                }

                if (_vec3Properties.find(prop) != _vec3Properties.end()) {
                    renderVec3Property(prop, p.first);
                    continue;
                }

                if (_vec4Properties.find(prop) != _vec4Properties.end()) {
                    renderVec4Property(prop, p.first);
                    continue;
                }

                if (_optionProperties.find(prop) != _optionProperties.end()) {
                    renderOptionProperty(prop, p.first);
                    continue;
                }

                if (_triggerProperties.find(prop) != _triggerProperties.end()) {
                    renderTriggerProperty(prop, p.first);
                    continue;
                }

                if (_selectionProperties.find(prop) != _selectionProperties.end()) {
                    renderSelectionProperty(prop, p.first);
                    continue;
                }

                if (_stringProperties.find(prop) != _stringProperties.end()) {
                    renderStringProperty(prop, p.first);
                    continue;
                }
            }
        }
    }


#ifdef OPENSPACE_MODULE_ISWA_ENABLED
    if (ImGui::CollapsingHeader("iSWA screen space cygntes")) {

        auto map = IswaManager::ref().cygnetInformation();
        for(auto cygnetInfo : map){
            int id = cygnetInfo.first;
            auto info = cygnetInfo.second;

            bool selected = info->selected;
            ImGui::Checkbox(info->name.c_str(), &info->selected);
            ImGui::SameLine();

            if(ImGui::CollapsingHeader(("Description" + std::to_string(id)).c_str())){
                ImGui::TextWrapped(info->description.c_str());
                ImGui::Spacing();
            }

            if(selected != info->selected){
                if(info->selected){
                    OsEng.scriptEngine().queueScript("openspace.iswa.addScreenSpaceCygnet("
                        "{CygnetId = "+std::to_string(id)+" });");
                }else{
                    OsEng.scriptEngine().queueScript("openspace.iswa.removeScreenSpaceCygnet("+std::to_string(id)+");");

                }
            }

        }
    }
#endif
    
    ImGui::End();
}

} // gui
} // openspace