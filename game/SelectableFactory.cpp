// Description:
//   Different kinds of selectable factories.
//
// Copyright (C) 2001 Frank Becker
//

#include "SelectableFactory.hpp"

using namespace std;

hash_map<const string, SelectableFactory*, hash<const string>, equal_to<const string>> SelectableFactory::_sfMap;

bool SelectableFactory::_initialized = false;

SelectableFactory* SelectableFactory::getFactory(const string& name) {
    XTRACE();
    if (!_initialized) {
        _initialized = true;

        new ActionItemFactory();
        new MenuItemFactory();
        new TextItemFactory();
        new ImageItemFactory();
        new BoolFactory();
        new EnumFactory();
        new FloatFactory();
        new LeaderBoardFactory();
        new ResolutionFactory();
    }
    return findHash<const string>(name, _sfMap);
}

void SelectableFactory::cleanup(void) {
    hash_map<const string, SelectableFactory*, hash<const string>>::const_iterator ci;
    for (ci = _sfMap.begin(); ci != _sfMap.end(); ci++) {
        SelectableFactory* sf = ci->second;
        delete sf;
    }
    _sfMap.clear();

    _initialized = false;
}

void SelectableFactory::posToPoint2D(const string& pos, Point2D& point) {
    if (pos == "") {
        point.x = 100;
        point.y = 100;
        return;
    }

    Tokenizer t(pos);
    point.x = (float)atof(t.next().c_str());
    point.y = (float)atof(t.next().c_str());
}

string SelectableFactory::getAttribute(const TiXmlElement* elem, string attr) {
    const char* attrVal = elem->Attribute(attr.c_str());
    if (attrVal) {
        return string(attrVal);
    }

    return (string(""));
}

void SelectableFactory::getBasics(TiXmlElement* elem, Point2D& pos, string& text, string& info, bool& enabled) {
    posToPoint2D(getAttribute(elem, "Position"), pos);
    text = getAttribute(elem, "Text");
    info = getAttribute(elem, "Info");
    enabled = elem->Attribute("Dis") == 0;
}

void SelectableFactory::getBasics(TiXmlElement* elem, Point2D& pos, string& text, string& info, bool& enabled,
                                  bool& centered) {
    getBasics(elem, pos, text, info, enabled);
    centered = elem->Attribute("Center") != 0;
}

//------------------------------------------------------------------------------

ActionItemFactory::ActionItemFactory(void) {
    _sfMap["ActionItem"] = this;
}

ActionItemFactory::~ActionItemFactory() {}

Selectable* ActionItemFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    getBasics(elem, r.min, text, info, enabled);

    string action = getAttribute(elem, "Action");

    string size = getAttribute(elem, "Size");
    if (size != "") {
        float fSize = (float)atof(size.c_str());
        return new ActionSelectable(enabled, r, action, text, info, fSize);
    }

    return new ActionSelectable(enabled, r, action, text, info);
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

MenuItemFactory::MenuItemFactory(void) {
    _sfMap["Menu"] = this;
}

MenuItemFactory::~MenuItemFactory() {}

Selectable* MenuItemFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    getBasics(elem, r.min, text, info, enabled);

    string size = getAttribute(elem, "Size");
    if (size != "") {
        float fSize = (float)atof(size.c_str());
        return new MenuSelectable(node, enabled, r, text, info, fSize);
    }

    return new MenuSelectable(node, enabled, r, text, info);
}

//------------------------------------------------------------------------------

TextItemFactory::TextItemFactory(void) {
    _sfMap["TextItem"] = this;
}

TextItemFactory::~TextItemFactory() {}

Selectable* TextItemFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    bool centered;
    getBasics(elem, r.min, text, info, enabled, centered);

    string size = getAttribute(elem, "Size");
    float fSize = 0.65f;
    if (size != "") {
        fSize = (float)atof(size.c_str());
    }

    return new TextOnlySelectable(enabled, r, text, info, centered, fSize, 1.0, 1.0, 1.0);
}

//------------------------------------------------------------------------------

ImageItemFactory::ImageItemFactory(void) {
    _sfMap["ImageItem"] = this;
}

ImageItemFactory::~ImageItemFactory() {}

Selectable* ImageItemFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    bool centered;
    getBasics(elem, r.min, text, info, enabled, centered);

    string size = getAttribute(elem, "Size");
    float fSize = 0.65f;
    if (size != "") {
        fSize = (float)atof(size.c_str());
    }

    return new ImageOnlySelectable(enabled, r, text, info, centered, fSize, 1.0, 1.0, 1.0);
}

//------------------------------------------------------------------------------

BoolFactory::BoolFactory(void) {
    _sfMap["Bool"] = this;
}

BoolFactory::~BoolFactory() {}

Selectable* BoolFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    getBasics(elem, r.min, text, info, enabled);

    string var = getAttribute(elem, "Variable");

    return new BoolSelectable(enabled, r, text, info, var);
}

//------------------------------------------------------------------------------

EnumFactory::EnumFactory(void) {
    _sfMap["Enum"] = this;
}

EnumFactory::~EnumFactory() {}

Selectable* EnumFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    getBasics(elem, r.min, text, info, enabled);

    string var = getAttribute(elem, "Variable");
    string values = getAttribute(elem, "Values");

    return new EnumSelectable(enabled, r, text, info, var, values);
}

//------------------------------------------------------------------------------

FloatFactory::FloatFactory(void) {
    _sfMap["Float"] = this;
}

FloatFactory::~FloatFactory() {}

Selectable* FloatFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    getBasics(elem, r.min, text, info, enabled);

    string var = getAttribute(elem, "Variable");
    string range = getAttribute(elem, "Range");
    string sliderOffset = getAttribute(elem, "SliderOffset");

    return new FloatSelectable(enabled, r, text, info, var, range, sliderOffset);
}

//------------------------------------------------------------------------------

LeaderBoardFactory::LeaderBoardFactory(void) {
    _sfMap["LeaderBoard"] = this;
}

LeaderBoardFactory::~LeaderBoardFactory() {}

Selectable* LeaderBoardFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    getBasics(elem, r.min, text, info, enabled);

    return new LeaderBoardSelectable(enabled, r, text, info);
}

//------------------------------------------------------------------------------

ResolutionFactory::ResolutionFactory(void) {
    _sfMap["Resolution"] = this;
}

ResolutionFactory::~ResolutionFactory() {}

Selectable* ResolutionFactory::createSelectable(TiXmlNode* node) {
    TiXmlElement* elem = node->ToElement();

    BoundingBox r;
    string text;
    string info;
    bool enabled;
    getBasics(elem, r.min, text, info, enabled);

    return new ResolutionSelectable(enabled, r, text, info);
}

//------------------------------------------------------------------------------
