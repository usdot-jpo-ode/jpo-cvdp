#include "rapidjsonRedactor.hpp"

/**
 * Values with overridden redaction behavior:
 * - angle          (required integer, set to 127)
 * - transmission   (required string, set to "UNAVAILABLE")
 * - wheelBrakes    (required bitstring, set first bit to 1 and all others to 0)
 * - weatherProbe   (optional object, remove if present)
 * - status         (optional object, remove if present)
 * - speedProfile   (optional object, remove if present)
 */
bool RapidjsonRedactor::redactMemberByPath(rapidjson::Value &value, std::string path) {
    std::string nextPathElement = getTopLevelFromPath(path);
    std::string target = getBottomLevelFromPath(path);

    if (value.IsObject()) {
        if (value.HasMember(nextPathElement.c_str())) {
            // get the type of the next path element
            std::string type = kTypeNames[value[nextPathElement.c_str()].GetType()];
            if (type == "Object" || type == "Array") {
                // if the next path element is an object or array, recurse
                auto &nextValue = value[nextPathElement.c_str()];

                // bitstring handling
                if (isBitstring(nextValue)) {

                    // wheelBrakes bitstring handling
                    if (nextPathElement == "wheelBrakes") {
                        if (target == "unavailable") {
                            nextValue["unavailable"] = true;
                        }
                        if (target == "leftFront") {
                            nextValue["leftFront"] = false;
                        }
                        else if (target == "rightFront") {
                            nextValue["rightFront"] = false;
                        }
                        else if (target == "leftRear") {
                            nextValue["leftRear"] = false;
                        }
                        else if (target == "rightRear") {
                            nextValue["rightRear"] = false;
                        }
                        else {
                            return false;
                        }
                        return true;
                    }

                    value.RemoveMember(nextPathElement.c_str());
                    return true;
                }

                // weatherProbe, status & speedProfile object handling
                if (type == "Object") {
                    if (nextPathElement == "weatherProbe" || nextPathElement == "status" || nextPathElement == "speedProfile") {
                        value.RemoveMember(nextPathElement.c_str());
                        return true;
                    }
                }

                removeTopLevelFromPath(path);
                return redactMemberByPath(nextValue, path);
            }
            else {
                // if the next path element is the target, remove it
                if (nextPathElement == target) {

                    // required leaf member handling
                    if (type == "Number" && target == "angle") {
                        value["angle"] = 127;
                        return true;
                    }
                    else if (type == "String" && target == "transmission") {
                        value["transmission"] = "UNAVAILABLE";
                        return true;
                    }

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

/**
 * Convert a string into a document.
 */
rapidjson::Document RapidjsonRedactor::getDocumentFromString(std::string jsonString) {
    rapidjson::Document document;
    document.Parse(jsonString.c_str());
    return document;
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

bool RapidjsonRedactor::isBitstring(rapidjson::Value &value) {
    // check if the value is an object consisting only of booleans
    if (!value.IsObject()) {
        return false;
    }
    for (auto &m : value.GetObject()) {
        std::string type = kTypeNames[m.value.GetType()];
        if (type != "True" && type != "False") {
            return false;
        }
    }
    return true;
}