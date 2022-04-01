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
        RedactionPropertiesManager() {
            debug = false;
            loadFields("/workspaces/jpo-cvdp/fieldsToRedact.txt"); // load fields upon construction
        }

        /**
         * @brief Sets the debug flag for the class.
         * 
         * @param b boolean to set
         */
        void setDebug(bool b) {
            debug = b;
        }

        /**
         * @brief Returns the file name that is used to persist the fields to redact.
         * 
         * @return string 
         */
        std::string getFileName() {
            return fileName;
        }

        /**
         * @brief Sets the file name that is used to persist the fields to redact.
         * 
         * @param newFileName 
         */
        void setFilename(std::string newFileName) {
            fileName = newFileName;
        }

        /**
         * @brief Returns a vector of the fields to redact.
         * 
         * @return vector<string> 
         */
        std::vector<std::string> getFields() {
            return fieldsToRedact;
        }
        
        /**
         * @brief Returns the number of fields to redact.
         * 
         * @return int 
         */
        int getNumFields() {
            return fieldsToRedact.size();
        }

        /**
         * @brief Checks if a field is in the list of fields to redact.
         * 
         * @param fieldToCheck 
         * @return boolean indicating whether or not the field is in the list
         */
        bool isField(std::string fieldToCheck) {
            for (std::string field : fieldsToRedact) {
                if (field == fieldToCheck) {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Adds a field to the list of fields to redact.
         * 
         * @param fieldToAdd 
         */
        void addField(std::string fieldToAdd) {
            log("adding field " + fieldToAdd);
            fieldsToRedact.push_back(fieldToAdd);
        }

        /**
         * @brief Prints the fields. For debugging purposes.
         * 
         */
        void printFields() {
            log("printing fields");
            std::cout << "=== Fields to Redact ===" << std::endl;
            for (std::string field: fieldsToRedact) {
                std::cout << field.c_str() << std::endl;
            }
        }

    private:
        bool debug;
        std::vector<std::string> fieldsToRedact;
        std::string fileName;

        /**
         * @brief Logs the message if the debug flag is set to true.
         * 
         * @param message 
         */
        void log(std::string message) {
            if (debug) {
                std::cout << "[RPM] " << message.c_str() << std::endl;
            }
        }

        /**
         * @brief Loads the fields from a file.
         * 
         */
        void loadFields(std::string fileName) {
            log("loading redaction fields");

            std::string line;            
            std::ifstream file(fileName);

            if (!file) {
                std::cout << "ERROR: could not file: " << fileName << std::endl;
                return;
            }

            // for each line, read the line and insert it into fieldsToRedact
            while (getline(file,line)) {
                log("read line: " + line);
                if (line.size() > 0) {
                    fieldsToRedact.push_back(line);
                }
            }
            
            log("redaction fields loaded");
        }
};