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
            loadRedactionProperties();
        }

        void setDebug(bool b) {
            debug = b;
        }

        vector<string> getRedactionProperties() {
            return fieldsToRedact;
        }

        int getNumProperties() {
            return fieldsToRedact.size();
        }

        string getRedactionProperty(string property) {
            for (string field : fieldsToRedact) {
                if (field == property) {
                    return field;
                }
            }
            return NULL;
        }

        void printProperties() {
            log("printing properties");
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

        void loadRedactionProperties() {
            log("loading redaction properties");

            string line;            
            ifstream file("../../fieldsToRedact.txt");

            // for each line, read the line and insert it into fieldsToRedact
            while (getline(file,line)) {
                if (line.size() > 0) {
                    fieldsToRedact.push_back(line);
                }
            }
            
            log("redaction properties loaded");
        }

};