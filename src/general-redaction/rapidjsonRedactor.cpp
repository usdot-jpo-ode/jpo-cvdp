#include "../../include/general-redaction/rapidjsonRedactor.hpp"

RapidjsonRedactor::RapidjsonRedactor() {
    indent = 0;
}

void RapidjsonRedactor::redactAllInstancesOfMemberByName(rapidjson::Value& value, std::string member, bool& success) {
    if (value.IsObject()) {
        while (value.HasMember(member.c_str())) {
            value.RemoveMember(member.c_str());
            success = true;
        }
        for (auto& m : value.GetObject()) {
            std::string type = kTypeNames[m.value.GetType()];
            if (type == "Object" || type == "Array") {
                std::string name = m.name.GetString();
                auto& v = value[name.c_str()];
                redactAllInstancesOfMemberByName(v, member, success);
            }
        }
    }
    else if (value.IsArray()) {
        for (auto& m : value.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                redactAllInstancesOfMemberByName(m, member, success);
            }
        }
    }
}

void RapidjsonRedactor::redactMemberByPath(rapidjson::Value& value, std::string path, bool& success) {
    std::string nextPathElement = getTopLevelFromPath(path);
    std::string target = getBottomLevelFromPath(path);

    if (value.IsObject()) {
        if (value.HasMember(nextPathElement.c_str())) {
            // get the type of the next path element
            std::string type = kTypeNames[value[nextPathElement.c_str()].GetType()];
            if (type == "Object" || type == "Array") {
                // if the next path element is an object or array, recurse
                auto& v = value[nextPathElement.c_str()];
                removeTopLevelFromPath(path);
                redactMemberByPath(v, path, success);
            }
            else {
                // if the next path element is the target, remove it
                if (nextPathElement == target) {
                    value.RemoveMember(nextPathElement.c_str());
                    success = true;
                }
            }
        }
        else {
            // if the next path element is not a member of the object, return
            return;
        }
    }
    else if (value.IsArray()) {
        for (auto& m : value.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                redactMemberByPath(m, path, success);
            }
        }
    }
}

void RapidjsonRedactor::searchForMemberByName(rapidjson::Value& value, std::string member, bool& success) {
    if (success) {
        // return if search has already succeeded
        return;
    }
    if (value.IsObject()) {
        if (value.HasMember(member.c_str())) {
            success = true;
        }
        for (auto& m : value.GetObject()) {
            std::string type = kTypeNames[m.value.GetType()];
            if (type == "Object" || type == "Array") {
                std::string name = m.name.GetString();
                auto& v = value[name.c_str()];
                searchForMemberByName(v, member, success);
            }
        }
    }
    else if (value.IsArray()) {
        for (auto& m : value.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                searchForMemberByName(m, member, success);
            }
        }
    }
}

void RapidjsonRedactor::searchForMemberByPath(rapidjson::Value& value, std::string path, bool& success) {
    if (success) {
        // return if search has already succeeded
        return;
    }
    
    std::string nextPathElement = getTopLevelFromPath(path);
    std::string target = getBottomLevelFromPath(path);

    if (value.IsObject()) {
        if (value.HasMember(nextPathElement.c_str())) {
            // get the type of the next path element
            std::string type = kTypeNames[value[nextPathElement.c_str()].GetType()];
            if (type == "Object" || type == "Array") {
                // if the next path element is an object or array, recurse
                auto& v = value[nextPathElement.c_str()];
                removeTopLevelFromPath(path);
                searchForMemberByPath(v, path, success);
            }
            else {
                // if the next path element is the target, return true
                if (nextPathElement == target) {
                    success = true;
                }
            }
        }
        else {
            // if the next path element is not a member of the current object, return
            return;
        }
        for (auto& m : value.GetObject()) {
            std::string type = kTypeNames[m.value.GetType()];
            if (type == "Object" || type == "Array") {
                std::string name = m.name.GetString();
                auto& v = value[name.c_str()];
                searchForMemberByPath(v, path, success);
            }
        }
    }
    else if (value.IsArray()) {
        for (auto& m : value.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                searchForMemberByPath(m, path, success);
            }
        }
    }
}

void RapidjsonRedactor::printValue(rapidjson::Value& valueToPrint) {
    if (valueToPrint.IsObject()) {
        for (auto& m : valueToPrint.GetObject()) {
            std::string name = m.name.GetString();
            std::string type = kTypeNames[m.value.GetType()];
            std::cout << getIndent(indent) << name  << " - " << type << std::endl;
            if (type == "Object" || type == "Array") {
                indent++;
                auto& value = valueToPrint[name.c_str()];
                printValue(value);
                indent--;
            }
        }
    }
    else if (valueToPrint.IsArray()) {
        for (auto& m : valueToPrint.GetArray()) {
            std::string type = kTypeNames[m.GetType()];
            if (type == "Object" || type == "Array") {
                indent++;
                printValue(m);
                indent--;
            }
        }
    }
}

void RapidjsonRedactor::printDocument(rapidjson::Document& document) {
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
std::string RapidjsonRedactor::stringifyDocument(rapidjson::Document& document) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

/**
 * Convert a value into a string.
 */
std::string RapidjsonRedactor::stringifyValue(rapidjson::Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

std::string RapidjsonRedactor::getIndent(int numSpaces) {
    std::string toReturn = "";
    for (int i = 0; i < numSpaces*2; i++) {
        toReturn = toReturn + " ";
    }
    return toReturn;
}

std::string RapidjsonRedactor::getTopLevelFromPath(std::string& path) {
    int firstDot = path.find(".");
    if (firstDot != std::string::npos) {
        return path.substr(0, firstDot);
    }
    return path;
}

void RapidjsonRedactor::removeTopLevelFromPath(std::string& path) {
    int firstDot = path.find(".");
    if (firstDot != std::string::npos) {
        path = path.substr(firstDot + 1);
    }
}

std::string RapidjsonRedactor::getBottomLevelFromPath(std::string& path) {
    int lastDot = path.rfind(".");
    if (lastDot != std::string::npos) {
        return path.substr(lastDot + 1);
    }
    return path;
}