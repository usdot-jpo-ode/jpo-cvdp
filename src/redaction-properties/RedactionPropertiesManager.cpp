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
        RedactionPropertiesManager() {
            debug = false;
            loadFields(); // load fields upon construction
        }

        ~RedactionPropertiesManager() {
            saveFields(); // save fields upon destruction
        }

        void setDebug(bool b) {
            debug = b;
        }

        /**
         * Returns a vector of the fields to redact.
         */
        vector<string> getFields() {
            return fieldsToRedact;
        }

        /**
         * Returns the number of fields to redact.
         */
        int getNumFields() {
            return fieldsToRedact.size();
        }

        bool isField(string fieldToCheck) {
            for (string field : fieldsToRedact) {
                if (field == fieldToCheck) {
                    return true;
                }
            }
            return false;
        }

        void addField(string fieldToAdd) {
            fieldsToRedact.push_back(fieldToAdd);
        }

        void removeField(string fieldToRemove) {
            // TODO: implement
        }

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

        void log(string message) {
            if (debug) {
                cout << "[RPM] " << message.c_str() << endl;
            }
        }

        void loadFields() {
            log("loading redaction fields");

            string line;            
            ifstream file("fieldsToRedact.txt");

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
            
            log("redaction properties loaded");
        }

        void saveFields() {
            // TODO: implement
        }

};