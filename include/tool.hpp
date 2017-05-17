#ifndef CTES_TOOL_CPP
#define CTES_TOOL_CPP

#include <getopt.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>

namespace tool {

    /**
     * Encapsulates all the structure needed for a command line tool.
     *
     * - Uses GNU getopt to parse arguments and then stores the results in user-friendly structures.
     * - Users supply an functor (i.e., a std::function<int(void)> instance) that runs the tool using the options.
     *
     * Usage.
     *
     * 1. Instantiate a tool instance.
     * 2. Add addOption to build the specification for the tool's command line.
     * 3. parse the user's provided command line: parseArgs( argc, argv )
     * 4. Run the tool using run() or just ();
     * 5. If it returns true, call the tool's run method.
     */
    class Tool {
        public:

            /**
             * Command line option.
             *
             * Options may or may not have arguments.  Options without arguments are "set" based on their presence on
             * the command line.
             *
             * Internally all values an option can take are represented as a string.
             *
             *  gnu option struction
             *  name    : const char *
             *  has_arg : { no_argument = 0, required_argument = 1, optional_argument = 2 }; these are const defined in getopt.h
             *  *flag   : if nullptr, then getopt_long returns (sets?) the value in the val field.
             *  val     : normally this is the character constant used as the short option; however, it can be the default value to load into the variable pointed to by flag or RETURN
             *
             */
            class Option {
                public:

                    // create a command line option with default value.
                    Option( char short_name, const std::string& long_name, const std::string& description, int arg_reqd, const std::string& default_argument );
                    char shortName() const;
                    const std::string& longName() const;
                    const std::string& description() const;
                    const std::string& argument() const;
                    int argumentAsInt() const;
                    double argumentAsDouble() const;
                    bool argReqd() const;
                    bool isSet() const;
                    bool hasArg() const;
                    Option& set();
                    Option& set( const std::string& value );
                    Option& set( const char* value );

                    friend std::ostream& operator<<(std::ostream& os, const Option& opt);

                private:
                    char sname_;
                    std::string lname_;
                    std::string desc_;
                    int arg_reqd_;
                    std::string arg_;
                    bool set_;   
            };

            Tool(const std::string& name, const std::string& description, bool operandsReqd = true, std::ostream& os = std::cerr );

            Tool& addOption(char short_name, const std::string& long_name, const std::string& description, int hasArg = 0, const std::string& defaultArg = "" );

            bool parseArgs(int argc, char* argv[]);
            Tool& set( char short_name, const char* argument );
            const Option& getOption( char short_name ) const;

            //const std::string& optString ( char short_name );
            const std::string& optString ( char short_name ) const;
            int optInt ( char short_name );
            int optDouble ( char short_name );
            bool optIsSet( char short_name );

            bool hasOperands() const;

            int run();
            virtual int operator() ( void ) = 0;
            void help();
            void usage();

            const std::string& name() const;
            const std::string& description() const;

        protected:  // need visibility of these in child classes implementing the () method.
            std::unordered_map<char, Option> options_map;
            std::vector<std::string> operands;
            std::ostream& os_;

        private:
            std::string option_string;
            std::vector<option> options_vector;

            std::string name_;
            std::string description_;
            bool operandsReqd;
    };

    std::ostream& operator<<(std::ostream& os, const tool::Tool::Option& opt);
};

#endif

