#include "../../include/general-redaction/rapidjsonRedactor.hpp"

RapidjsonRedactor::RapidjsonRedactor() {
    indent = 0;
}

bool RapidjsonRedactor::redactAllInstancesOfMemberByName(rapidjson::Value &value, std::string member) {
    if (value.IsObject()) {
        while (value.HasMember(member.c_str())) {
            value.RemoveMember(member.c_str());
            return true;
        }
        for (auto &m : value.GetObject()) {
            std::string type = kTypeNames[m.value.GetType()];
            if (type == "Object" || type == "Array")
            {
                std::string name = m.name.GetString();
                auto &v = value[name.c_str()];
                bool result = redactAllInstancesOfMemberByName(v, member);
                if (result) {
                    return true;
                }
            }
        }
    }
    else if (value.IsArray()) {
        for (auto &m : value.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                bool result = redactAllInstancesOfMemberByName(m, member);
                if (result) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool RapidjsonRedactor::redactMemberByPath(rapidjson::Value &value, std::string path) {
    std::string nextPathElement = getTopLevelFromPath(path);
    std::string target = getBottomLevelFromPath(path);

    if (value.IsObject()) {
        if (value.HasMember(nextPathElement.c_str())) {
            // get the type of the next path element
            std::string type = kTypeNames[value[nextPathElement.c_str()].GetType()];
            if (type == "Object" || type == "Array") {
                // if the next path element is an object or array, recurse
                auto &v = value[nextPathElement.c_str()];
                removeTopLevelFromPath(path);
                return redactMemberByPath(v, path);
            }
            else {
                // if the next path element is the target, remove it
                if (nextPathElement == target) {
                    value.RemoveMember(nextPathElement.c_str());
                    return true;
                }
            }
        }
        else {
            // if the next path element is not a member of the object, return
            return false;
        }
    }
    else if (value.IsArray()) {
        for (auto &m : value.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                bool result = redactMemberByPath(m, path);
                if (result) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool RapidjsonRedactor::searchForMemberByName(rapidjson::Value &value, std::string member) {
    if (value.IsObject()) {
        if (value.HasMember(member.c_str())) {
            return true;
        }
        for (auto &m : value.GetObject()) {
            std::string type = kTypeNames[m.value.GetType()];
            if (type == "Object" || type == "Array") {
                std::string name = m.name.GetString();
                auto &v = value[name.c_str()];
                bool success = searchForMemberByName(v, member);
                if (success) {
                    return true;
                }
            }
        }
    }
    else if (value.IsArray()) {
        for (auto &m : value.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                bool result = searchForMemberByName(m, member);
                if (result) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool RapidjsonRedactor::searchForMemberByPath(rapidjson::Value &value, std::string path) {
    std::string nextPathElement = getTopLevelFromPath(path);
    std::string target = getBottomLevelFromPath(path);

    if (value.IsObject()) {
        if (value.HasMember(nextPathElement.c_str())) {
            // get the type of the next path element
            std::string type = kTypeNames[value[nextPathElement.c_str()].GetType()];
            if (type == "Object" || type == "Array") {
                // if the next path element is an object or array, recurse
                auto &v = value[nextPathElement.c_str()];
                removeTopLevelFromPath(path);
                return searchForMemberByPath(v, path);
            }
            else {
                // if the next path element is the target, return true
                if (nextPathElement == target) {
                    return true;
                }
            }
        }
        else {
            // if the next path element is not a member of the current object, return
            return false;
        }
        for (auto &m : value.GetObject()) {
            std::string type = kTypeNames[m.value.GetType()];
            if (type == "Object" || type == "Array") {
                std::string name = m.name.GetString();
                auto &v = value[name.c_str()];
                return searchForMemberByPath(v, path);
            }
        }
    }
    else if (value.IsArray()) {
        for (auto &m : value.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                bool result = searchForMemberByPath(m, path);
                if (result) {
                    return true;
                }
            }
        }
    }
    return false;
}

void RapidjsonRedactor::printValue(rapidjson::Value &valueToPrint) {
    if (valueToPrint.IsObject()) {
        for (auto &m : valueToPrint.GetObject()) {
            std::string name = m.name.GetString();
            std::string type = kTypeNames[m.value.GetType()];
            std::cout << getIndent(indent) << name << " - " << type << std::endl;
            if (type == "Object" || type == "Array") {
                indent++;
                auto &value = valueToPrint[name.c_str()];
                printValue(value);
                indent--;
            }
        }
    }
    else if (valueToPrint.IsArray()) {
        for (auto &m : valueToPrint.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                indent++;
                printValue(m);
                indent--;
            }
        }
    }
}

void RapidjsonRedactor::printDocument(rapidjson::Document &document) {
    printValue(document);
}

/**
 * Convert a string into a document.
 */
rapidjson::Document RapidjsonRedactor::getDocumentFromString(std::string jsonString) {
    rapidjson::Document document;
    document.Parse(jsonString.c_str());
    return document;
}

/**
 * Convert a document into a string.
 */
std::string RapidjsonRedactor::stringifyDocument(rapidjson::Document &document) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

/**
 * Convert a value into a string.
 */
std::string RapidjsonRedactor::stringifyValue(rapidjson::Value &value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

std::string RapidjsonRedactor::getIndent(int numSpaces) {
    std::string toReturn = "";
    for (int i = 0; i < numSpaces * 2; i++) {
        toReturn = toReturn + " ";
    }
    return toReturn;
}

std::string RapidjsonRedactor::getTopLevelFromPath(std::string &path) {
    int firstDot = path.find(".");
    if (firstDot != std::string::npos) {
        return path.substr(0, firstDot);
    }
    return path;
}

void RapidjsonRedactor::removeTopLevelFromPath(std::string &path) {
    int firstDot = path.find(".");
    if (firstDot != std::string::npos) {
        path = path.substr(firstDot + 1);
    }
}

std::string RapidjsonRedactor::getBottomLevelFromPath(std::string &path) {
    int lastDot = path.rfind(".");
    if (lastDot != std::string::npos) {
        return path.substr(lastDot + 1);
    }
    return path;
}