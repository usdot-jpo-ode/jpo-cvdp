#ifndef CVDP_RPM_H
#define CVDP_RPM_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

/**
 * This class is meant to load in a number of specified fields to redact from the partII section of BSM messages. This is used in the BSM Filter.
 */
class RedactionPropertiesManager {

    public:

        /**
         * @brief Construct a new Redaction Properties Manager object with a default path. Upon instantiation, fields are loaded from a file.
         * 
         */
        RedactionPropertiesManager();

        /**
         * @brief Sets the debug flag for the class.
         * 
         * @param b boolean to set
         */
        void setDebug(bool b);

        /**
         * @brief Returns the file name that is used to persist the fields to redact.
         * 
         * @return string 
         */
        std::string getFileName();

        /**
         * @brief Sets the file name that is used to persist the fields to redact.
         * 
         * @param newFileName 
         */
        void setFilename(std::string newFileName);

        /**
         * @brief Returns a vector of the fields to redact.
         * 
         * @return vector<string> 
         */
        std::vector<std::string> getFields();
        
        /**
         * @brief Returns the number of fields to redact.
         * 
         * @return int 
         */
        int getNumFields();

        /**
         * @brief Checks if a field is in the list of fields to redact.
         * 
         * @param fieldToCheck 
         * @return boolean indicating whether or not the field is in the list
         */
        bool isField(std::string fieldToCheck);

        /**
         * @brief Adds a field to the list of fields to redact.
         * 
         * @param fieldToAdd 
         */
        void addField(std::string fieldToAdd);

        /**
         * @brief Prints the fields. For debugging purposes.
         * 
         */
        void printFields();

    private:
        bool debug;
        std::vector<std::string> fieldsToRedact;
        std::string fileName;

        /**
            * @brief Logs the message to a file.
            * 
            * @param message 
            */
        void logToFile(std::string message);

        /**
         * @brief Loads the fields from a file.
         * 
         */
        void loadFields(std::string fileName);

        const char* getEnvironmentVariable(const char* variableName);

        std::string toLowercase(std::string s);

        bool convertStringToBool(std::string s);
};

#endif