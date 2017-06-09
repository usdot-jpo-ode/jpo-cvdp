#include "tool.hpp"
#include "spdlog/spdlog.h"
#include <iomanip>

namespace tool {

Tool::Option::Option( char short_name, const std::string& long_name, const std::string& description, int arg_reqd, const std::string& default_argument ) :
    sname_{ short_name },
    lname_{ long_name },
    desc_{ description },
    arg_reqd_{ arg_reqd },
    arg_{ default_argument },
    set_{ false }
{}

char Tool::Option::shortName() const {
    return sname_;
}

const std::string& Tool::Option::longName() const
{
    return lname_;
}

const std::string& Tool::Option::description() const
{
    return desc_;
}

const std::string& Tool::Option::argument() const
//std::string Tool::Option::argument() const
{
    return arg_;
}

int Tool::Option::argumentAsInt() const
{
    return std::stoi(arg_);   // throws std::invalid_argument or std::out_of_range.
}

double Tool::Option::argumentAsDouble() const
{
    return std::stod(arg_);   // throws std::invalid_argument or std::out_of_range.
}

/**
 * Predicate indicating whether this CLO requires a parameters.
 *
 * This corresponds to getopt hasarg = 1 (required argument).
 */
bool Tool::Option::argReqd() const
{
    return (arg_reqd_ == 1);
}

/**
 * Was this Option set on the user's provided command line.
 */
bool Tool::Option::isSet() const
{
    return set_;
}

/**
 * Predicate indicating whether the arg is non-empty.
 */
bool Tool::Option::hasArg() const
{
    // we only care about the argument if it is set.
    return (isSet() && !arg_.empty());
}

Tool::Option& Tool::Option::set()
{
    set_ = true;
    return *this;
}

/**
 * Set this Option's argument value.
 */
Tool::Option& Tool::Option::set( const std::string& value )
{
    set().set( value.c_str() );
    arg_ = value;
    return *this;           // chaining.
}

/**
 * Set this Option's argument value.
 */
Tool::Option& Tool::Option::set( const char* value )
{
    set();
    //arg_ = std::string{ value };
    arg_ = value;           // convert to string and copy into arg_
    return *this;           // chaining.
}

Tool::Tool(const std::string& name, const std::string& description, bool operandsReqd, std::ostream& os ) :
    option_string{":"},             // empty options with self-generated error suppression.
    name_{name},
    description_{description},
    os_{os},
    operandsReqd{ operandsReqd }
{}

int Tool::run()
{
    // this is the operator() method that must be defined by a child class of this abstract class.
    return operator()();
    //return (*this)();
}

/**
 * Add an Option to the command line. This method is for building the CLI specification for the tool. Also
 * build the option string to use with getopt.
 */
Tool& Tool::addOption(char short_name, const std::string& long_name, const std::string& description, int hasArg, const std::string& defaultArg )
{
    // mirrors the getopt structure.
    options_map.emplace( std::make_pair( short_name, Tool::Option{ short_name, long_name, description, hasArg, defaultArg } ) );

    // gnu option structure build, so getopt_long will work.
    // need an option_string described as follows:
    // This string has some interesting specs:
    //   If the first char is ':' then getopt generated errors are suppressed (this is preferred so we will
    //   have this as default) and return values of '?' indicate invalid option while ':' indicate missing
    //   option argument.

    option_string += short_name;
    if (hasArg > no_argument) {
        option_string += ':';
    }

    // And the option structures themselves:
    // name    : const char *
    // has_arg : { no_argument = 0, required_argument = 1, optional_argument = 2 }; these are const defined in getopt.h
    // *flag   : if nullptr, then getopt_long returns (sets?) the value in the val field.
    // val     : normally this is the character constant used as the short option; however, it can be the default value to load into the variable pointed to by flag or RETURN
    // {"help", 0, nullptr, 'h'},
    option o{ long_name.c_str(), hasArg, nullptr, static_cast<int>( short_name ) };
    options_vector.push_back( o );                      // this must make a copy (hence push_back), so we can use it outside of the scope of this method.

    return *this;       // allow chaining of addOption.
}

Tool& Tool::set( char short_name, const char* argument ) {

    // find the option the user specified on their command.
    auto search = options_map.find( short_name );

    if (search == options_map.end()) {
        // the user specified a non-existent command (this really should be caught by getopt).
        throw std::invalid_argument{ "The option was not found." };

    } else {
        // option is in the CLI specification; mark it as being provided by the executor.
        search->second.set();

        if (argument && search->second.argReqd()) {
            // user also provided an argument and it is expected.
            search->second.set(optarg);
        }
    } 

    return *this;
}

const Tool::Option& Tool::getOption( char short_name ) const
{
    auto search = options_map.find( short_name );
    if (search != options_map.end()) {
        return search->second;
    } else {
        throw std::invalid_argument{ "The option: -" + std::to_string(short_name) + " was not found." };
    }
}

/**
 * Delegated accessor.
 */
const std::string& Tool::optString ( char short_name ) const
{
    return getOption( short_name ).argument();
}

/**
 * Delegated accessor.
 */
int Tool::optInt ( char short_name )
{
    Tool::Option o = getOption( short_name );
    return o.argumentAsInt();
}

/**
 * Delegated accessor.
 */
int Tool::optDouble ( char short_name )
{
    Tool::Option o = getOption( short_name );
    return o.argumentAsDouble();
}

bool Tool::optIsSet( char short_name )
{
    bool r = false;

    // if found the Option is set.
    auto search = options_map.find( short_name );
    if ( search != options_map.end() ) {
        r = search->second.isSet();
    }

    return r;
}

bool Tool::hasOperands() const
{
    return !operands.empty();
}

void Tool::help()
{
    os_ << name_ << "\n" << description_ << "\n";
    for (auto& mapping : options_map) {
        os_ << "\t" << mapping.second << "\n";
    }
}

void Tool::usage()
{
    os_ << "Usage: " << name_ << " [OPTIONS] [OPERANDS]\n";
    for (auto& mapping : options_map) {
        os_ << "\t" << mapping.second << "\n";
    }
}

const std::string& Tool::name() const
{
    return name_;
}

const std::string& Tool::description() const
{
    return description_;
}

bool Tool::parseArgs(int argc, char* argv[])
{
    int opt;

    opterr = 0;                 // turn off error reporting.

    while ((opt = getopt_long(argc, argv, option_string.c_str(), &options_vector[0], nullptr)) != -1)
    {
        /**
         * optstring : if the first character is ':' then error messages are suppressed and '?' is returned
         * when a problem occurs. the optopt is set to the invalid option.
         *
         * Return values put in opt.
         * -1  : end of options.
         * 0   : set the flag variable in the options_vector structure.
         * 1   : optarg points to a plain command line option argument.
         * '?' : invalid option. if the first char of optstring is NOT ':' this can also mean invalid option.
         * ':' : missing option argument.
         * 'x' : option character <x>, e.g., 'a', 'b'
         *
         * "Globals"
         * opterr (int) : when nonzero (default) getopt prints its own error messages.
         *                set to 0 prior to getopt call to suppress all self-generated error messages.
         *                seems to the be the same as starting the optstring with ':'
         * optarg (char*) : the argument for the option if the option accepts a value.
         * optind (int) : current index of argv; when finished remaining args are in argv[optind] thru * argv[argc-1]
         * optopt (int) : when an invalid character is found, this is set to that character
         */
        switch (opt) {
            case 0 :
                os_ << "opt value is 0\n";
                return false;

            case 1 :
                std::cout << "not sure what this does... case 1. optarg = " << optarg << std::endl;
                break;

            case '?' :
                os_ << "CLI Error: -" << (char) optopt << " is an invalid option.\n";
                return false;

            case ':' :
                os_ << "CLI Error: -" << (char) optopt << " is missing a required argument\n";
                return false;

            default :
                set( static_cast<char>(opt), optarg );
        }
    }

    // Because getopt reorders the command line, we have all the operands at the end and they are in
    // sequence. Starting with the argv index optind, updated by getopt_long, till the end are the operands.
    operands = std::vector<std::string>{ argv+optind, argv+argc };
    return true;
}

std::ostream& operator<<(std::ostream& os, const tool::Tool::Option& opt) {
    os << "-" << opt.shortName();
    os << ", --" << std::left << std::setw(15) << opt.longName();
    os << " : " << opt.description() << std::right;
    return os;
}

} // end tool namespace.

