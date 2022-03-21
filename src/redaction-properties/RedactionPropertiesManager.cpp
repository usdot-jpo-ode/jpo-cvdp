#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

/**
 * This class is meant to manage various redaction properties for use in the BSM Filter.
 * @author Daniel Stephenson
 */
class RedactionPropertiesManager {

    public:
        /**
         * @brief Construct a new Redaction Properties Manager object
         * 
         */
        RedactionPropertiesManager() {
            debug = false;
            fileName = "fieldsToRedact.txt";
            loadFields(fileName); // load fields upon construction
        }

        /**
         * @brief Destroy the Redaction Properties Manager object
         * 
         */
        ~RedactionPropertiesManager() {
            saveFields(fileName); // save fields upon destruction
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
        string getFileName() {
            return fileName;
        }

        /**
         * @brief Sets the file name that is used to persist the fields to redact.
         * 
         * @param newFileName 
         */
        void setFilename(string newFileName) {
            fileName = newFileName;
        }

        /**
         * @brief Returns a vector of the fields to redact.
         * 
         * @return vector<string> 
         */
        vector<string> getFields() {
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
        bool isField(string fieldToCheck) {
            for (string field : fieldsToRedact) {
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
        void addField(string fieldToAdd) {
            log("adding field " + fieldToAdd);
            fieldsToRedact.push_back(fieldToAdd);
        }

        /**
         * @brief Removes a field from the list of fields to redact.
         * 
         * @param fieldToRemove 
         */
        void removeField(string fieldToRemove) {
            log("removing field " + fieldToRemove);
            // TODO: implement
            log("WARNING: this method is not implemented yet");
        }

        /**
         * @brief Prints the fields. For debugging purposes.
         * 
         */
        void printFields() {
            log("printing fields");
            cout << "=== Fields to Redact ===" << endl;
            for (string field: fieldsToRedact) {
                cout << field.c_str() << endl;
            }
        }

    private:
        bool debug;
        vector<string> fieldsToRedact;
        string fileName;

        /**
         * @brief Logs the message if the debug flag is set to true.
         * 
         * @param message 
         */
        void log(string message) {
            if (debug) {
                cout << "[RPM] " << message.c_str() << endl;
            }
        }

        /**
         * @brief Loads the fields from a file.
         * 
         */
        void loadFields(string fileName) {
            log("loading redaction fields");

            string line;            
            ifstream file(fileName);

            if (!file) {
                log("could not open the file");
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

        /**
         * @brief Saves the fields to a file.
         * 
         */
        void saveFields(string fileName) {
            log("saving redaction fields");
            // TODO: implement
            log("WARNING: this method is not implemented yet");
        }

};