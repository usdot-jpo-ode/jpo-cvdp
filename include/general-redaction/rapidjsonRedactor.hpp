#include <string>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };

/**
 * @brief A tool for redacting members of rapidjson::Document and rapidjson::Value objects
 * 
 */
class RapidjsonRedactor {
    public:
        RapidjsonRedactor();

        // redaction methods
        
        /**
         * @brief Redacts all instances of a member by name
         * 
         * @param value The rapidjson::Value to redact from
         * @param member The name of the member to redact
         * @param success Whether or not the redaction was successful
         */
        void redactAllInstancesOfMemberByName(rapidjson::Value& value, std::string member, bool& success);

        /**
         * @brief Redacts all instances of a member by path
         * 
         * @param value The rapidjson::Value to redact from
         * @param path The path to the member to redact
         * @param success Whether or not the redaction was successful
         */
        void redactMemberByPath(rapidjson::Value& value, std::string path, bool& success);

        /**
         * @brief Searches for a member by name
         * 
         * @param value The rapidjson::Value to search
         * @param member The name of the member to search for
         * @param success Whether or not the search was successful
         */
        void searchForMemberByName(rapidjson::Value& value, std::string member, bool& success);

        /**
         * @brief Searches for a member by path
         * 
         * @param value The rapidjson::Value to search
         * @param path The path to the member to search for
         * @param success Whether or not the search was successful
         */
        void searchForMemberByPath(rapidjson::Value& value, std::string path, bool& success);

        // utility methods

        /**
         * @brief Prints a rapidjson::Value to the console
         * 
         * @param valueToPrint The rapidjson::Value to print
         */
        void printValue(rapidjson::Value& valueToPrint);

        /**
         * @brief Prints a rapidjson::Document to the console
         * 
         * @param document The rapidjson::Document to print
         */
        void printDocument(rapidjson::Document& document);

        /**
         * @brief Gets a rapidjson::Document from a string
         * 
         * @param jsonString The string to convert
         * @return rapidjson::Document The converted rapidjson::Document
         */
        rapidjson::Document getDocumentFromString(std::string jsonString);

        /**
         * @brief Gets a rapidjson::Value from a string
         * 
         * @param jsonString The string to convert
         * @return rapidjson::Value The converted rapidjson::Value
         */
        rapidjson::Value getValueFromString(std::string jsonString);

        /**
         * @brief Gets a string from a rapidjson::Document
         * 
         * @param document The rapidjson::Document to convert
         * @return std::string The converted string
         */
        std::string stringifyDocument(rapidjson::Document& document);

        /**
         * @brief Gets a string from a rapidjson::Value
         * 
         * @param value The rapidjson::Value to convert
         * @return std::string The converted string
         */
        std::string stringifyValue(rapidjson::Value& value);
    private:
        int indent;

        // helper methods

        /**
         * @brief Get indent string for printing
         * 
         * @param numSpaces 
         * @return std::string The indent string
         */
        std::string getIndent(int numSpaces);

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
};
