#include <map>
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

        map<string, bool> getRedactionProperties() {
            return redactionProperties;
        }

        int getNumProperties() {
            return redactionProperties.size();
        }

        bool getRedactionProperty(string key) {
            bool toReturn = redactionProperties[key];
            return redactionProperties[key];
        }

        void printProperties() {
            log("printing properties");
            cout << "=== Redaction Properties ===" << endl;
            for (pair<string, bool> pair: redactionProperties) {
                string key = pair.first;
                int value = pair.second;
                cout << key.c_str() << ": " << noboolalpha << value << endl;
            }
        }

    private:
        bool debug;
        map<string, bool> redactionProperties;

        void log(string message) {
            if (debug) {
                cout << "[RPM] " << message.c_str() << endl;
            }
        }

        void loadRedactionProperties() {
            log("loading redaction properties");

            string line;            
            ifstream file("../../redaction.properties");

            while (getline(file, line)) { // TODO: figure out why getline isn't recognized
                log("parsing line: " + line);
                string key = line.substr(0, line.find("="));
                string valueString = line.substr(line.find("="));
                redactionProperties.insert(key, valueString);
            }
            
            log("redaction properties loaded");
        }

};

/**
 * This method is meant to test this class.
 */
int main() {
    RedactionPropertiesManager redactionPropertiesManager;
    cout << "Num properties: " << redactionPropertiesManager.getNumProperties() << endl;
    redactionPropertiesManager.printProperties();
}