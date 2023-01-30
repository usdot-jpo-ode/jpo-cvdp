#include <string>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };

/**
 * @brief A tool for redacting members of rapidjson::Value objects
 * 
 */
class RapidjsonRedactor {
    public:
        // redaction methods
        
        /**
         * @brief Redacts all instances of a member by name
         * 
         * @param value The rapidjson::Value to redact from
         * @param member The name of the member to redact
         */
        bool redactAllInstancesOfMemberByName(rapidjson::Value& value, std::string member);

        /**
         * @brief Redacts all instances of a member by path
         * 
         * @param value The rapidjson::Value to redact from
         * @param path The path to the member to redact
         */
        bool redactMemberByPath(rapidjson::Value& value, std::string path);

        /**
         * @brief Searches for a member by name
         * 
         * @param value The rapidjson::Value to search
         * @param member The name of the member to search for
         */
        bool searchForMemberByName(rapidjson::Value& value, std::string member);

        /**
         * @brief Searches for a member by path
         * 
         * @param value The rapidjson::Value to search
         * @param path The path to the member to search for
         */
        bool searchForMemberByPath(rapidjson::Value& value, std::string path);

        // utility methods

        /**
         * @brief Gets a rapidjson::Document from a string
         * 
         * @param jsonString The string to convert
         * @return rapidjson::Document The converted rapidjson::Document
         */
        rapidjson::Document getDocumentFromString(std::string jsonString);

        /**
         * @brief Gets a string from a rapidjson::Value
         * 
         * @param value The rapidjson::Value to convert
         * @return std::string The converted string
         */
        std::string stringifyValue(rapidjson::Value& value);
    private:
        // helper methods

        /**
         * @brief Get the Top Level From Path object    
         * 
         * @param path 
         * @return std::string The top level of the path
         */
        std::string getTopLevelFromPath(std::string& path);

        /**
         * @brief Remove the Top Level From Path object
         * 
         * @param path The path to remove the top level from
         * @return std::string The path without the top level
         */
        void removeTopLevelFromPath(std::string& path);

        /**
         * @brief Get the Bottom Level From Path object
         * 
         * @param path The path to get the bottom level from
         * @return std::string The bottom level of the path
         */
        std::string getBottomLevelFromPath(std::string& path);
};
